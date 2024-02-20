/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef gc_Statistics_h
#define gc_Statistics_h

#include "mozilla/Array.h"
#include "mozilla/Atomics.h"
#include "mozilla/EnumeratedArray.h"
#include "mozilla/IntegerRange.h"
#include "mozilla/Maybe.h"
#include "mozilla/TimeStamp.h"

#include "jspubtd.h"
#include "NamespaceImports.h"

#include "gc/GCEnum.h"
#include "js/AllocPolicy.h"
#include "js/SliceBudget.h"
#include "js/UniquePtr.h"
#include "js/Vector.h"
#include "vm/JSONPrinter.h"

namespace js {
namespace gcstats {

// Phase data is generated by a script. If you need to add phases, edit
// js/src/gc/GenerateStatsPhases.py

#include "gc/StatsPhasesGenerated.h"

// Counts can be incremented with Statistics::count(). They're reset at the end
// of a Major GC.
enum Count {
  COUNT_NEW_CHUNK,
  COUNT_DESTROY_CHUNK,
  COUNT_MINOR_GC,

  // Number of times a 'put' into a storebuffer overflowed, triggering a
  // compaction
  COUNT_STOREBUFFER_OVERFLOW,

  // Number of arenas relocated by compacting GC.
  COUNT_ARENA_RELOCATED,

  COUNT_LIMIT
};

// Stats can be set with Statistics::setStat(). They're not reset automatically.
enum Stat {
  // Number of strings tenured.
  STAT_STRINGS_TENURED,

  // Number of object types pretenured this minor GC.
  STAT_OBJECT_GROUPS_PRETENURED,

  // Number of realms that had nursery strings disabled due to large numbers
  // being tenured.
  STAT_NURSERY_STRING_REALMS_DISABLED,

  // Number of BigInts tenured.
  STAT_BIGINTS_TENURED,

  // Number of realms that had nursery BigInts disabled due to large numbers
  // being tenured.
  STAT_NURSERY_BIGINT_REALMS_DISABLED,

  STAT_LIMIT
};

struct ZoneGCStats {
  /* Number of zones collected in this GC. */
  int collectedZoneCount = 0;

  /* Number of zones that could have been collected in this GC. */
  int collectableZoneCount = 0;

  /* Total number of zones in the Runtime at the start of this GC. */
  int zoneCount = 0;

  /* Number of zones swept in this GC. */
  int sweptZoneCount = 0;

  /* Total number of compartments in all zones collected. */
  int collectedCompartmentCount = 0;

  /* Total number of compartments in the Runtime at the start of this GC. */
  int compartmentCount = 0;

  /* Total number of compartments swept by this GC. */
  int sweptCompartmentCount = 0;

  bool isFullCollection() const {
    return collectedZoneCount == collectableZoneCount;
  }

  ZoneGCStats() = default;
};

struct Trigger {
  size_t amount = 0;
  size_t threshold = 0;
};

#define FOR_EACH_GC_PROFILE_TIME(_)                                 \
  _(BeginCallback, "bgnCB", PhaseKind::GC_BEGIN)                    \
  _(MinorForMajor, "evct4m", PhaseKind::EVICT_NURSERY_FOR_MAJOR_GC) \
  _(WaitBgThread, "waitBG", PhaseKind::WAIT_BACKGROUND_THREAD)      \
  _(Prepare, "prep", PhaseKind::PREPARE)                            \
  _(Mark, "mark", PhaseKind::MARK)                                  \
  _(Sweep, "sweep", PhaseKind::SWEEP)                               \
  _(Compact, "cmpct", PhaseKind::COMPACT)                           \
  _(EndCallback, "endCB", PhaseKind::GC_END)                        \
  _(MinorGC, "minor", PhaseKind::MINOR_GC)                          \
  _(EvictNursery, "evict", PhaseKind::EVICT_NURSERY)                \
  _(Barriers, "brrier", PhaseKind::BARRIER)

const char* ExplainAbortReason(gc::AbortReason reason);
const char* ExplainInvocationKind(JSGCInvocationKind gckind);

/*
 * Struct for collecting timing statistics on a "phase tree". The tree is
 * specified as a limited DAG, but the timings are collected for the whole tree
 * that you would get by expanding out the DAG by duplicating subtrees rooted
 * at nodes with multiple parents.
 *
 * During execution, a child phase can be activated multiple times, and the
 * total time will be accumulated. (So for example, you can start and end
 * PhaseKind::MARK_ROOTS multiple times before completing the parent phase.)
 *
 * Incremental GC is represented by recording separate timing results for each
 * slice within the overall GC.
 */
struct Statistics {
  template <typename T, size_t Length>
  using Array = mozilla::Array<T, Length>;

  template <typename IndexType, IndexType SizeAsEnumValue, typename ValueType>
  using EnumeratedArray =
      mozilla::EnumeratedArray<IndexType, SizeAsEnumValue, ValueType>;

  using TimeDuration = mozilla::TimeDuration;
  using TimeStamp = mozilla::TimeStamp;

  // Create a convenient type for referring to tables of phase times.
  using PhaseTimeTable = EnumeratedArray<Phase, Phase::LIMIT, TimeDuration>;

  static MOZ_MUST_USE bool initialize();

  explicit Statistics(gc::GCRuntime* gc);
  ~Statistics();

  Statistics(const Statistics&) = delete;
  Statistics& operator=(const Statistics&) = delete;

  void beginPhase(PhaseKind phaseKind);
  void endPhase(PhaseKind phaseKind);
  void recordParallelPhase(PhaseKind phaseKind, TimeDuration duration);

  // Occasionally, we may be in the middle of something that is tracked by
  // this class, and we need to do something unusual (eg evict the nursery)
  // that doesn't normally nest within the current phase. Suspend the
  // currently tracked phase stack, at which time the caller is free to do
  // other tracked operations.
  //
  // This also happens internally with the PhaseKind::MUTATOR "phase". While in
  // this phase, any beginPhase will automatically suspend the non-GC phase,
  // until that inner stack is complete, at which time it will automatically
  // resume the non-GC phase. Explicit suspensions do not get auto-resumed.
  void suspendPhases(PhaseKind suspension = PhaseKind::EXPLICIT_SUSPENSION);

  // Resume a suspended stack of phases.
  void resumePhases();

  void beginSlice(const ZoneGCStats& zoneStats, JSGCInvocationKind gckind,
                  SliceBudget budget, JS::GCReason reason);
  void endSlice();

  MOZ_MUST_USE bool startTimingMutator();
  MOZ_MUST_USE bool stopTimingMutator(double& mutator_ms, double& gc_ms);

  // Note when we sweep a zone or compartment.
  void sweptZone() { ++zoneStats.sweptZoneCount; }
  void sweptCompartment() { ++zoneStats.sweptCompartmentCount; }

  void reset(gc::AbortReason reason) {
    MOZ_ASSERT(reason != gc::AbortReason::None);
    if (!aborted) {
      slices_.back().resetReason = reason;
    }
  }

  void measureInitialHeapSize();
  void adoptHeapSizeDuringIncrementalGC(Zone* mergedZone);

  void nonincremental(gc::AbortReason reason) {
    MOZ_ASSERT(reason != gc::AbortReason::None);
    nonincrementalReason_ = reason;
    writeLogMessage("Non-incremental reason: %s", nonincrementalReason());
  }

  bool nonincremental() const {
    return nonincrementalReason_ != gc::AbortReason::None;
  }

  const char* nonincrementalReason() const {
    return ExplainAbortReason(nonincrementalReason_);
  }

  void count(Count s) { counts[s]++; }

  uint32_t getCount(Count s) const { return uint32_t(counts[s]); }

  void setStat(Stat s, uint32_t value) { stats[s] = value; }

  uint32_t getStat(Stat s) const { return stats[s]; }

  void recordTrigger(size_t amount, size_t threshold) {
    recordedTrigger = mozilla::Some(Trigger{amount, threshold});
  }
  bool hasTrigger() const { return recordedTrigger.isSome(); }

  void noteNurseryAlloc() { allocsSinceMinorGC.nursery++; }

  // tenured allocs don't include nursery evictions.
  void setAllocsSinceMinorGCTenured(uint32_t allocs) {
    allocsSinceMinorGC.tenured = allocs;
  }

  uint32_t allocsSinceMinorGCNursery() { return allocsSinceMinorGC.nursery; }

  uint32_t allocsSinceMinorGCTenured() { return allocsSinceMinorGC.tenured; }

  uint32_t* addressOfAllocsSinceMinorGCNursery() {
    return &allocsSinceMinorGC.nursery;
  }

  void beginNurseryCollection(JS::GCReason reason);
  void endNurseryCollection(JS::GCReason reason);

  TimeStamp beginSCC();
  void endSCC(unsigned scc, TimeStamp start);

  UniqueChars formatCompactSliceMessage() const;
  UniqueChars formatCompactSummaryMessage() const;
  UniqueChars formatDetailedMessage() const;

  JS::GCSliceCallback setSliceCallback(JS::GCSliceCallback callback);
  JS::GCNurseryCollectionCallback setNurseryCollectionCallback(
      JS::GCNurseryCollectionCallback callback);

  TimeDuration clearMaxGCPauseAccumulator();
  TimeDuration getMaxGCPauseSinceClear();

  PhaseKind currentPhaseKind() const;

  static const size_t MAX_SUSPENDED_PHASES = MAX_PHASE_NESTING * 3;

  struct SliceData {
    SliceData(SliceBudget budget, mozilla::Maybe<Trigger> trigger,
              JS::GCReason reason, TimeStamp start, size_t startFaults,
              gc::State initialState);

    SliceBudget budget;
    JS::GCReason reason = JS::GCReason::NO_REASON;
    mozilla::Maybe<Trigger> trigger;
    gc::State initialState = gc::State::NotActive;
    gc::State finalState = gc::State::NotActive;
    gc::AbortReason resetReason = gc::AbortReason::None;
    TimeStamp start;
    TimeStamp end;
    size_t startFaults = 0;
    size_t endFaults = 0;
    PhaseTimeTable phaseTimes;
    PhaseTimeTable maxParallelTimes;

    TimeDuration duration() const { return end - start; }
    bool wasReset() const { return resetReason != gc::AbortReason::None; }
  };

  typedef Vector<SliceData, 8, SystemAllocPolicy> SliceDataVector;

  const SliceDataVector& slices() const { return slices_; }

  TimeStamp start() const { return slices_[0].start; }

  TimeStamp end() const { return slices_.back().end; }

  // Occasionally print header lines for profiling information.
  void maybePrintProfileHeaders();

  // Print header line for profile times.
  void printProfileHeader();

  // Print total profile times on shutdown.
  void printTotalProfileTimes();

  enum JSONUse { TELEMETRY, PROFILER };

  // Return JSON for a whole major GC.  If use == PROFILER then
  // detailed per-slice data and some other fields will be included.
  UniqueChars renderJsonMessage(uint64_t timestamp, JSONUse use) const;

  // Return JSON for the timings of just the given slice.
  UniqueChars renderJsonSlice(size_t sliceNum) const;

  // Return JSON for the previous nursery collection.
  UniqueChars renderNurseryJson() const;

#ifdef DEBUG
  // Print a logging message.
  void writeLogMessage(const char* fmt, ...);
#else
  void writeLogMessage(const char* fmt, ...){};
#endif

 private:
  gc::GCRuntime* const gc;

  /* File used for MOZ_GCTIMER output. */
  FILE* gcTimerFile;

  /* File used for JS_GC_DEBUG output. */
  FILE* gcDebugFile;

  ZoneGCStats zoneStats;

  JSGCInvocationKind gckind;

  gc::AbortReason nonincrementalReason_;

  SliceDataVector slices_;

  /* Most recent time when the given phase started. */
  EnumeratedArray<Phase, Phase::LIMIT, TimeStamp> phaseStartTimes;

#ifdef DEBUG
  /* Most recent time when the given phase ended. */
  EnumeratedArray<Phase, Phase::LIMIT, TimeStamp> phaseEndTimes;
#endif

  /* Bookkeeping for GC timings when timingMutator is true */
  TimeStamp timedGCStart;
  TimeDuration timedGCTime;

  /* Total time in a given phase for this GC. */
  PhaseTimeTable phaseTimes;

  /* Number of events of this type for this GC. */
  EnumeratedArray<Count, COUNT_LIMIT,
                  mozilla::Atomic<uint32_t, mozilla::ReleaseAcquire>>
      counts;

  /* Other GC statistics. */
  EnumeratedArray<Stat, STAT_LIMIT, uint32_t> stats;

  /*
   * These events cannot be kept in the above array, we need to take their
   * address.
   */
  struct {
    uint32_t nursery;
    uint32_t tenured;
  } allocsSinceMinorGC;

  /* Total GC heap size before and after the GC ran. */
  size_t preTotalHeapBytes;
  size_t postTotalHeapBytes;

  /* GC heap size for collected zones before GC ran. */
  size_t preCollectedHeapBytes;

  /*
   * If a GC slice was triggered by exceeding some threshold, record the
   * threshold and the value that exceeded it. This happens before the slice
   * starts so this is recorded here first and then transferred to SliceData.
   */
  mozilla::Maybe<Trigger> recordedTrigger;

  /* GC numbers as of the beginning of the collection. */
  uint64_t startingMinorGCNumber;
  uint64_t startingMajorGCNumber;
  uint64_t startingSliceNumber;

  /* Records the maximum GC pause in an API-controlled interval. */
  mutable TimeDuration maxPauseInInterval;

  /* Phases that are currently on stack. */
  Vector<Phase, MAX_PHASE_NESTING, SystemAllocPolicy> phaseStack;

  /*
   * Certain phases can interrupt the phase stack, eg callback phases. When
   * this happens, we move the suspended phases over to a sepearate list,
   * terminated by a dummy PhaseKind::SUSPENSION phase (so that we can nest
   * suspensions by suspending multiple stacks with a PhaseKind::SUSPENSION in
   * between).
   */
  Vector<Phase, MAX_SUSPENDED_PHASES, SystemAllocPolicy> suspendedPhases;

  /* Sweep times for SCCs of compartments. */
  Vector<TimeDuration, 0, SystemAllocPolicy> sccTimes;

  TimeDuration timeSinceLastGC;

  JS::GCSliceCallback sliceCallback;
  JS::GCNurseryCollectionCallback nurseryCollectionCallback;

  /*
   * True if we saw an OOM while allocating slices or we saw an impossible
   * timestamp. The statistics for this GC will be invalid.
   */
  bool aborted;

  /* Profiling data. */

  enum class ProfileKey {
    Total,
#define DEFINE_TIME_KEY(name, text, phase) name,
    FOR_EACH_GC_PROFILE_TIME(DEFINE_TIME_KEY)
#undef DEFINE_TIME_KEY
        KeyCount
  };

  using ProfileDurations =
      EnumeratedArray<ProfileKey, ProfileKey::KeyCount, TimeDuration>;

  TimeDuration profileThreshold_;
  bool enableProfiling_;
  ProfileDurations totalTimes_;
  uint64_t sliceCount_;

  JSContext* context();

  Phase currentPhase() const;
  Phase lookupChildPhase(PhaseKind phaseKind) const;

  void beginGC(JSGCInvocationKind kind, const TimeStamp& currentTime);
  void endGC();

  void sendGCTelemetry();
  void sendSliceTelemetry(const SliceData& slice);

  void recordPhaseBegin(Phase phase);
  void recordPhaseEnd(Phase phase);

  void gcDuration(TimeDuration* total, TimeDuration* maxPause) const;
  void sccDurations(TimeDuration* total, TimeDuration* maxPause) const;
  void printStats();

  void reportLongestPhaseInMajorGC(PhaseKind longest, int telemetryId);

  UniqueChars formatCompactSlicePhaseTimes(
      const PhaseTimeTable& phaseTimes) const;

  UniqueChars formatDetailedDescription() const;
  UniqueChars formatDetailedSliceDescription(unsigned i,
                                             const SliceData& slice) const;
  UniqueChars formatDetailedPhaseTimes(const PhaseTimeTable& phaseTimes) const;
  UniqueChars formatDetailedTotals() const;

  void formatJsonDescription(uint64_t timestamp, JSONPrinter&, JSONUse) const;
  void formatJsonSliceDescription(unsigned i, const SliceData& slice,
                                  JSONPrinter&) const;
  void formatJsonPhaseTimes(const PhaseTimeTable& phaseTimes,
                            JSONPrinter&) const;
  void formatJsonSlice(size_t sliceNum, JSONPrinter&) const;

  double computeMMU(TimeDuration resolution) const;

  void printSliceProfile();
  static void printProfileTimes(const ProfileDurations& times);
};

struct MOZ_RAII AutoGCSlice {
  AutoGCSlice(Statistics& stats, const ZoneGCStats& zoneStats,
              JSGCInvocationKind gckind, SliceBudget budget,
              JS::GCReason reason)
      : stats(stats) {
    stats.beginSlice(zoneStats, gckind, budget, reason);
  }
  ~AutoGCSlice() { stats.endSlice(); }

  Statistics& stats;
};

struct MOZ_RAII AutoPhase {
  AutoPhase(Statistics& stats, PhaseKind phaseKind)
      : stats(stats), phaseKind(phaseKind), enabled(true) {
    stats.beginPhase(phaseKind);
  }

  AutoPhase(Statistics& stats, bool condition, PhaseKind phaseKind)
      : stats(stats), phaseKind(phaseKind), enabled(condition) {
    if (enabled) {
      stats.beginPhase(phaseKind);
    }
  }

  ~AutoPhase() {
    if (enabled) {
      stats.endPhase(phaseKind);
    }
  }

  Statistics& stats;
  PhaseKind phaseKind;
  bool enabled;
};

struct MOZ_RAII AutoSCC {
  AutoSCC(Statistics& stats, unsigned scc) : stats(stats), scc(scc) {
    start = stats.beginSCC();
  }
  ~AutoSCC() { stats.endSCC(scc, start); }

  Statistics& stats;
  unsigned scc;
  mozilla::TimeStamp start;
};

} /* namespace gcstats */
} /* namespace js */

#endif /* gc_Statistics_h */
