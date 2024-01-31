/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BaselineJIT.h"
#include "BaselineCompiler.h"
#include "BaselineHelpers.h"
#include "BaselineIC.h"
#include "IonLinker.h"
#include "IonSpewer.h"
#include "VMFunctions.h"
#include "IonFrames-inl.h"

#include "builtin/Eval.h"

#include "jsinterpinlines.h"

namespace js {
namespace ion {

#ifdef DEBUG
void
FallbackICSpew(JSContext *cx, ICFallbackStub *stub, const char *fmt, ...)
{
    if (IonSpewEnabled(IonSpew_BaselineICFallback)) {
        RootedScript script(cx, GetTopIonJSScript(cx));
        jsbytecode *pc = stub->icEntry()->pc(script);

        char fmtbuf[100];
        va_list args;
        va_start(args, fmt);
        vsnprintf(fmtbuf, 100, fmt, args);
        va_end(args);

        IonSpew(IonSpew_BaselineICFallback,
                "Fallback hit for (%s:%d) (pc=%d,line=%d,uses=%d,stubs=%d): %s",
                script->filename(),
                script->lineno,
                (int) (pc - script->code),
                PCToLineNumber(script, pc),
                script->getUseCount(),
                (int) stub->numOptimizedStubs(),
                fmtbuf);
    }
}

void
TypeFallbackICSpew(JSContext *cx, ICTypeMonitor_Fallback *stub, const char *fmt, ...)
{
    if (IonSpewEnabled(IonSpew_BaselineICFallback)) {
        RootedScript script(cx, GetTopIonJSScript(cx));
        jsbytecode *pc = stub->icEntry()->pc(script);

        char fmtbuf[100];
        va_list args;
        va_start(args, fmt);
        vsnprintf(fmtbuf, 100, fmt, args);
        va_end(args);

        IonSpew(IonSpew_BaselineICFallback,
                "Type monitor fallback hit for (%s:%d) (pc=%d,line=%d,uses=%d,stubs=%d): %s",
                script->filename(),
                script->lineno,
                (int) (pc - script->code),
                PCToLineNumber(script, pc),
                script->getUseCount(),
                (int) stub->numOptimizedMonitorStubs(),
                fmtbuf);
    }
}

#else
#define FallbackICSpew(...)
#define TypeFallbackICSpew(...)
#endif


ICFallbackStub *
ICEntry::fallbackStub() const
{
    return firstStub()->getChainFallback();
}


ICStubConstIterator &
ICStubConstIterator::operator++()
{
    JS_ASSERT(currentStub_ != NULL);
    currentStub_ = currentStub_->next();
    return *this;
}


ICStubIterator::ICStubIterator(ICFallbackStub *fallbackStub, bool end)
  : icEntry_(fallbackStub->icEntry()),
    fallbackStub_(fallbackStub),
    previousStub_(NULL),
    currentStub_(end ? fallbackStub : icEntry_->firstStub()),
    unlinked_(false)
{ }

ICStubIterator &
ICStubIterator::operator++()
{
    JS_ASSERT(currentStub_->next() != NULL);
    if (!unlinked_)
        previousStub_ = currentStub_;
    currentStub_ = currentStub_->next();
    unlinked_ = false;
    return *this;
}

void
ICStubIterator::unlink(Zone *zone)
{
    JS_ASSERT(currentStub_->next() != NULL);
    JS_ASSERT(currentStub_ != fallbackStub_);
    JS_ASSERT(!unlinked_);
    fallbackStub_->unlinkStub(zone, previousStub_, currentStub_);

    // Mark the current iterator position as unlinked, so operator++ works properly.
    unlinked_ = true;
}


void
ICStub::markCode(JSTracer *trc, const char *name)
{
    IonCode *stubIonCode = ionCode();
    MarkIonCodeUnbarriered(trc, &stubIonCode, name);
}

void
ICStub::updateCode(IonCode *code)
{
    // Write barrier on the old code.
#ifdef JSGC_INCREMENTAL
    IonCode::writeBarrierPre(ionCode());
#endif
    stubCode_ = code->raw();
}

/* static */ void
ICStub::trace(JSTracer *trc)
{
    markCode(trc, "baseline-stub-ioncode");

    // If the stub is a monitored fallback stub, then mark the monitor ICs hanging
    // off of that stub.  We don't need to worry about the regular monitored stubs,
    // because the regular monitored stubs will always have a monitored fallback stub
    // that references the same stub chain.
    if (isMonitoredFallback()) {
        ICTypeMonitor_Fallback *lastMonStub = toMonitoredFallbackStub()->fallbackMonitorStub();
        for (ICStubConstIterator iter = lastMonStub->firstMonitorStub(); !iter.atEnd(); iter++) {
            JS_ASSERT_IF(iter->next() == NULL, *iter == lastMonStub);
            iter->markCode(trc, "baseline-monitor-stub-ioncode");
        }
    }

    if (isUpdated()) {
        for (ICStubConstIterator iter = toUpdatedStub()->firstUpdateStub(); !iter.atEnd(); iter++) {
            JS_ASSERT_IF(iter->next() == NULL, iter->isTypeUpdate_Fallback());
            iter->markCode(trc, "baseline-update-stub-ioncode");
        }
    }

    switch (kind()) {
      case ICStub::Call_Scripted: {
        ICCall_Scripted *callStub = toCall_Scripted();
        MarkScript(trc, &callStub->calleeScript(), "baseline-callscripted-callee");
        break;
      }
      case ICStub::Call_Native: {
        ICCall_Native *callStub = toCall_Native();
        MarkObject(trc, &callStub->callee(), "baseline-callnative-callee");
        break;
      }
      case ICStub::GetElem_Native: {
        ICGetElem_Native *getElemStub = toGetElem_Native();
        MarkShape(trc, &getElemStub->shape(), "baseline-getelem-native-shape");
        gc::MarkValue(trc, &getElemStub->idval(), "baseline-getelem-native-idval");
        break;
      }
      case ICStub::GetElem_NativePrototype: {
        ICGetElem_NativePrototype *getElemStub = toGetElem_NativePrototype();
        MarkShape(trc, &getElemStub->shape(), "baseline-getelem-nativeproto-shape");
        gc::MarkValue(trc, &getElemStub->idval(), "baseline-getelem-nativeproto-idval");
        MarkObject(trc, &getElemStub->holder(), "baseline-getelem-nativeproto-holder");
        MarkShape(trc, &getElemStub->holderShape(), "baseline-getelem-nativeproto-holdershape");
        break;
      }
      case ICStub::GetElem_Dense: {
        ICGetElem_Dense *getElemStub = toGetElem_Dense();
        MarkShape(trc, &getElemStub->shape(), "baseline-getelem-dense-shape");
        break;
      }
      case ICStub::GetElem_TypedArray: {
        ICGetElem_TypedArray *getElemStub = toGetElem_TypedArray();
        MarkShape(trc, &getElemStub->shape(), "baseline-getelem-typedarray-shape");
        break;
      }
      case ICStub::SetElem_Dense: {
        ICSetElem_Dense *setElemStub = toSetElem_Dense();
        MarkShape(trc, &setElemStub->shape(), "baseline-getelem-dense-shape");
        MarkTypeObject(trc, &setElemStub->type(), "baseline-setelem-dense-type");
        break;
      }
      case ICStub::SetElem_DenseAdd: {
        ICSetElem_DenseAdd *setElemStub = toSetElem_DenseAdd();
        MarkTypeObject(trc, &setElemStub->type(), "baseline-setelem-denseadd-type");

        JS_STATIC_ASSERT(ICSetElem_DenseAdd::MAX_PROTO_CHAIN_DEPTH == 4);

        switch (setElemStub->protoChainDepth()) {
          case 0: setElemStub->toImpl<0>()->traceShapes(trc); break;
          case 1: setElemStub->toImpl<1>()->traceShapes(trc); break;
          case 2: setElemStub->toImpl<2>()->traceShapes(trc); break;
          case 3: setElemStub->toImpl<3>()->traceShapes(trc); break;
          case 4: setElemStub->toImpl<4>()->traceShapes(trc); break;
          default: JS_NOT_REACHED("Invalid proto stub.");
        }
        break;
      }
      case ICStub::SetElem_TypedArray: {
        ICSetElem_TypedArray *setElemStub = toSetElem_TypedArray();
        MarkShape(trc, &setElemStub->shape(), "baseline-setelem-typedarray-shape");
        break;
      }
      case ICStub::TypeMonitor_SingleObject: {
        ICTypeMonitor_SingleObject *monitorStub = toTypeMonitor_SingleObject();
        MarkObject(trc, &monitorStub->object(), "baseline-monitor-singleobject");
        break;
      }
      case ICStub::TypeMonitor_TypeObject: {
        ICTypeMonitor_TypeObject *monitorStub = toTypeMonitor_TypeObject();
        MarkTypeObject(trc, &monitorStub->type(), "baseline-monitor-typeobject");
        break;
      }
      case ICStub::TypeUpdate_SingleObject: {
        ICTypeUpdate_SingleObject *updateStub = toTypeUpdate_SingleObject();
        MarkObject(trc, &updateStub->object(), "baseline-update-singleobject");
        break;
      }
      case ICStub::TypeUpdate_TypeObject: {
        ICTypeUpdate_TypeObject *updateStub = toTypeUpdate_TypeObject();
        MarkTypeObject(trc, &updateStub->type(), "baseline-update-typeobject");
        break;
      }
      case ICStub::Profiler_PushFunction: {
        ICProfiler_PushFunction *pushFunStub = toProfiler_PushFunction();
        MarkScript(trc, &pushFunStub->script(), "baseline-profilerpushfunction-stub-script");
        break;
      }
      case ICStub::GetName_Global: {
        ICGetName_Global *globalStub = toGetName_Global();
        MarkShape(trc, &globalStub->shape(), "baseline-global-stub-shape");
        break;
      }
      case ICStub::GetName_Scope0:
        static_cast<ICGetName_Scope<0>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope1:
        static_cast<ICGetName_Scope<1>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope2:
        static_cast<ICGetName_Scope<2>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope3:
        static_cast<ICGetName_Scope<3>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope4:
        static_cast<ICGetName_Scope<4>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope5:
        static_cast<ICGetName_Scope<5>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetName_Scope6:
        static_cast<ICGetName_Scope<6>*>(this)->traceScopes(trc);
        break;
      case ICStub::GetIntrinsic_Constant: {
        ICGetIntrinsic_Constant *constantStub = toGetIntrinsic_Constant();
        gc::MarkValue(trc, &constantStub->value(), "baseline-getintrinsic-constant-value");
        break;
      }
      case ICStub::GetProp_String: {
        ICGetProp_String *propStub = toGetProp_String();
        MarkShape(trc, &propStub->stringProtoShape(), "baseline-getpropstring-stub-shape");
        break;
      }
      case ICStub::GetProp_Native: {
        ICGetProp_Native *propStub = toGetProp_Native();
        MarkShape(trc, &propStub->shape(), "baseline-getpropnative-stub-shape");
        break;
      }
      case ICStub::GetProp_NativePrototype: {
        ICGetProp_NativePrototype *propStub = toGetProp_NativePrototype();
        MarkShape(trc, &propStub->shape(), "baseline-getpropnativeproto-stub-shape");
        MarkObject(trc, &propStub->holder(), "baseline-getpropnativeproto-stub-holder");
        MarkShape(trc, &propStub->holderShape(), "baseline-getpropnativeproto-stub-holdershape");
        break;
      }
      case ICStub::GetProp_CallScripted: {
        ICGetProp_CallScripted *callStub = toGetProp_CallScripted();
        MarkShape(trc, &callStub->shape(), "baseline-getpropcallscripted-stub-shape");
        MarkObject(trc, &callStub->holder(), "baseline-getpropcallscripted-stub-holder");
        MarkShape(trc, &callStub->holderShape(), "baseline-getpropcallscripted-stub-holdershape");
        MarkObject(trc, &callStub->getter(), "baseline-getpropcallscripted-stub-getter");
        break;
      }
      case ICStub::SetProp_Native: {
        ICSetProp_Native *propStub = toSetProp_Native();
        MarkShape(trc, &propStub->shape(), "baseline-setpropnative-stub-shape");
        MarkTypeObject(trc, &propStub->type(), "baseline-setpropnative-stub-type");
        break;
      }
      case ICStub::SetProp_NativeAdd: {
        ICSetProp_NativeAdd *propStub = toSetProp_NativeAdd();
        MarkTypeObject(trc, &propStub->type(), "baseline-setpropnativeadd-stub-type");
        MarkShape(trc, &propStub->newShape(), "baseline-setpropnativeadd-stub-newshape");
        JS_STATIC_ASSERT(ICSetProp_NativeAdd::MAX_PROTO_CHAIN_DEPTH == 4);
        switch (propStub->protoChainDepth()) {
          case 0: propStub->toImpl<0>()->traceShapes(trc); break;
          case 1: propStub->toImpl<1>()->traceShapes(trc); break;
          case 2: propStub->toImpl<2>()->traceShapes(trc); break;
          case 3: propStub->toImpl<3>()->traceShapes(trc); break;
          case 4: propStub->toImpl<4>()->traceShapes(trc); break;
          default: JS_NOT_REACHED("Invalid proto stub.");
        }
        break;
      }
      case ICStub::SetProp_CallScripted: {
        ICSetProp_CallScripted *callStub = toSetProp_CallScripted();
        MarkShape(trc, &callStub->shape(), "baseline-setpropcallscripted-stub-shape");
        MarkObject(trc, &callStub->holder(), "baseline-setpropcallscripted-stub-holder");
        MarkShape(trc, &callStub->holderShape(), "baseline-setpropcallscripted-stub-holdershape");
        MarkObject(trc, &callStub->setter(), "baseline-setpropcallscripted-stub-setter");
        break;
      }
      default:
        break;
    }
}

void
ICFallbackStub::unlinkStub(Zone *zone, ICStub *prev, ICStub *stub)
{
    JS_ASSERT(stub->next());

    // If stub is the last optimized stub, update lastStubPtrAddr.
    if (stub->next() == this) {
        JS_ASSERT(lastStubPtrAddr_ == stub->addressOfNext());
        if (prev)
            lastStubPtrAddr_ = prev->addressOfNext();
        else
            lastStubPtrAddr_ = icEntry()->addressOfFirstStub();
        *lastStubPtrAddr_ = this;
    } else {
        if (prev) {
            JS_ASSERT(prev->next() == stub);
            prev->setNext(stub->next());
        } else {
            JS_ASSERT(icEntry()->firstStub() == stub);
            icEntry()->setFirstStub(stub->next());
        }
    }

    JS_ASSERT(numOptimizedStubs_ > 0);
    numOptimizedStubs_--;

    if (zone->needsBarrier()) {
        // We are removing edges from ICStub to gcthings. Perform one final trace
        // of the stub for incremental GC, as it must know about those edges.
        stub->trace(zone->barrierTracer());
    }

    if (ICStub::CanMakeCalls(stub->kind()) && stub->isMonitored()) {
        // This stub can make calls so we can return to it if it's on the stack.
        // We just have to reset its firstMonitorStub_ field to avoid a stale
        // pointer when purgeOptimizedStubs destroys all optimized monitor
        // stubs (unlinked stubs won't be updated).
        ICTypeMonitor_Fallback *monitorFallback = toMonitoredFallbackStub()->fallbackMonitorStub();
        stub->toMonitoredStub()->resetFirstMonitorStub(monitorFallback);
    }

#ifdef DEBUG
    // Poison stub code to ensure we don't call this stub again. However, if this
    // stub can make calls, a pointer to it may be stored in a stub frame on the
    // stack, so we can't touch the stubCode_ or GC will crash when marking this
    // pointer.
    if (!ICStub::CanMakeCalls(stub->kind()))
        stub->stubCode_ = (uint8_t *)0xbad;
#endif
}

void
ICFallbackStub::unlinkStubsWithKind(JSContext *cx, ICStub::Kind kind)
{
    for (ICStubIterator iter = beginChain(); !iter.atEnd(); iter++) {
        if (iter->kind() == kind)
            iter.unlink(cx->zone());
    }
}

void
ICTypeMonitor_Fallback::resetMonitorStubChain(Zone *zone)
{
    if (zone->needsBarrier()) {
        // We are removing edges from monitored stubs to gcthings (IonCode).
        // Perform one final trace of all monitor stubs for incremental GC,
        // as it must know about those edges.
        this->trace(zone->barrierTracer());
    }

    firstMonitorStub_ = this;
    numOptimizedMonitorStubs_ = 0;

    if (hasFallbackStub_) {
        lastMonitorStubPtrAddr_ = NULL;

        // Reset firstMonitorStub_ field of all monitored stubs.
        for (ICStubConstIterator iter = mainFallbackStub_->beginChainConst();
             !iter.atEnd(); iter++)
        {
            if (!iter->isMonitored())
                continue;
            iter->toMonitoredStub()->resetFirstMonitorStub(this);
        }
    } else {
        icEntry_->setFirstStub(this);
        lastMonitorStubPtrAddr_ = icEntry_->addressOfFirstStub();
    }
}

ICMonitoredStub::ICMonitoredStub(Kind kind, IonCode *stubCode, ICStub *firstMonitorStub)
  : ICStub(kind, ICStub::Monitored, stubCode),
    firstMonitorStub_(firstMonitorStub)
{
    // If the first monitored stub is a ICTypeMonitor_Fallback stub, then
    // double check that _its_ firstMonitorStub is the same as this one.
    JS_ASSERT_IF(firstMonitorStub_->isTypeMonitor_Fallback(),
                 firstMonitorStub_->toTypeMonitor_Fallback()->firstMonitorStub() ==
                    firstMonitorStub_);
}

bool
ICMonitoredFallbackStub::initMonitoringChain(JSContext *cx, ICStubSpace *space)
{
    JS_ASSERT(fallbackMonitorStub_ == NULL);

    ICTypeMonitor_Fallback::Compiler compiler(cx, this);
    ICTypeMonitor_Fallback *stub = compiler.getStub(space);
    if (!stub)
        return false;
    fallbackMonitorStub_ = stub;
    return true;
}

bool
ICMonitoredFallbackStub::addMonitorStubForValue(JSContext *cx, HandleScript script, HandleValue val)
{
    return fallbackMonitorStub_->addMonitorStubForValue(cx, script, val);
}

bool
ICUpdatedStub::initUpdatingChain(JSContext *cx, ICStubSpace *space)
{
    JS_ASSERT(firstUpdateStub_ == NULL);

    ICTypeUpdate_Fallback::Compiler compiler(cx);
    ICTypeUpdate_Fallback *stub = compiler.getStub(space);
    if (!stub)
        return false;

    firstUpdateStub_ = stub;
    return true;
}

IonCode *
ICStubCompiler::getStubCode()
{
    IonCompartment *ion = cx->compartment->ionCompartment();

    // Check for existing cached stubcode.
    uint32_t stubKey = getKey();
    IonCode *stubCode = ion->getStubCode(stubKey);
    if (stubCode)
        return stubCode;

    // Compile new stubcode.
    MacroAssembler masm;
#ifdef JS_CPU_ARM
    masm.setSecondScratchReg(BaselineSecondScratchReg);
#endif

    AutoFlushCache afc("ICStubCompiler::getStubCode", cx->runtime->ionRuntime());
    if (!generateStubCode(masm))
        return NULL;
    Linker linker(masm);
    Rooted<IonCode *> newStubCode(cx, linker.newCode(cx, JSC::BASELINE_CODE));
    if (!newStubCode)
        return NULL;

    // After generating code, run postGenerateStubCode()
    if (!postGenerateStubCode(masm, newStubCode))
        return NULL;

    // All barriers are emitted off-by-default, enable them if needed.
    if (cx->zone()->needsBarrier())
        newStubCode->togglePreBarriers(true);

    // Cache newly compiled stubcode.
    if (!ion->putStubCode(stubKey, newStubCode))
        return NULL;

    JS_ASSERT(entersStubFrame_ == ICStub::CanMakeCalls(kind));

    return newStubCode;
}

bool
ICStubCompiler::tailCallVM(const VMFunction &fun, MacroAssembler &masm)
{
    IonCompartment *ion = cx->compartment->ionCompartment();
    IonCode *code = ion->getVMWrapper(fun);
    if (!code)
        return false;

    uint32_t argSize = fun.explicitStackSlots() * sizeof(void *);
    EmitTailCallVM(code, masm, argSize);
    return true;
}

bool
ICStubCompiler::callVM(const VMFunction &fun, MacroAssembler &masm)
{
    IonCompartment *ion = cx->compartment->ionCompartment();
    IonCode *code = ion->getVMWrapper(fun);
    if (!code)
        return false;

    EmitCallVM(code, masm);
    return true;
}

bool
ICStubCompiler::callTypeUpdateIC(MacroAssembler &masm, uint32_t objectOffset)
{
    IonCompartment *ion = cx->compartment->ionCompartment();
    IonCode *code = ion->getVMWrapper(DoTypeUpdateFallbackInfo);
    if (!code)
        return false;

    EmitCallTypeUpdateIC(masm, code, objectOffset);
    return true;
}

void
ICStubCompiler::enterStubFrame(MacroAssembler &masm, Register scratch)
{
    EmitEnterStubFrame(masm, scratch);
#ifdef DEBUG
    entersStubFrame_ = true;
#endif
}

void
ICStubCompiler::leaveStubFrame(MacroAssembler &masm, bool calledIntoIon)
{
    JS_ASSERT(entersStubFrame_);
    EmitLeaveStubFrame(masm, calledIntoIon);
}
 
void
ICStubCompiler::guardProfilingEnabled(MacroAssembler &masm, Register scratch, Label *skip)
{
    // This should only be called from the following stubs.
    JS_ASSERT(kind == ICStub::Call_Scripted || kind == ICStub::Call_AnyScripted ||
              kind == ICStub::Call_Native   || kind == ICStub::GetProp_CallScripted ||
              kind == ICStub::SetProp_CallScripted);

    // Guard on bit in frame that indicates if the SPS frame was pushed in the first
    // place.  This code is expected to be called from within a stub that has already
    // entered a stub frame.
    JS_ASSERT(entersStubFrame_);
    masm.loadPtr(Address(BaselineFrameReg, 0), scratch);
    masm.branchTest32(Assembler::Zero,
                      Address(scratch, BaselineFrame::reverseOffsetOfFlags()),
                      Imm32(BaselineFrame::HAS_PUSHED_SPS_FRAME),
                      skip);

    // Check if profiling is enabled
    uint32_t *enabledAddr = cx->runtime->spsProfiler.addressOfEnabled();
    masm.branch32(Assembler::Equal, AbsoluteAddress(enabledAddr), Imm32(0), skip);
}

//
// UseCount_Fallback
//
static bool
IsTopFrameConstructing(JSContext *cx)
{
    IonFrameIterator iter(cx);
    JS_ASSERT(iter.type() == IonFrame_Exit);

    ++iter;
    JS_ASSERT(iter.type() == IonFrame_BaselineStub);

    ++iter;
    JS_ASSERT(iter.isBaselineJS());

    return iter.isConstructing();
}

static bool
EnsureCanEnterIon(JSContext *cx, ICUseCount_Fallback *stub, BaselineFrame *frame,
                  HandleScript script, jsbytecode *pc, void **jitcodePtr)
{
    JS_ASSERT(jitcodePtr);
    JS_ASSERT(!*jitcodePtr);

    bool isLoopEntry = (JSOp(*pc) == JSOP_LOOPENTRY);

    bool isConstructing = IsTopFrameConstructing(cx);
    MethodStatus stat;
    if (isLoopEntry) {
        IonSpew(IonSpew_BaselineOSR, "  Compile at loop entry!");
        stat = CanEnterAtBranch(cx, script, frame, pc, isConstructing);
    } else if (frame->isFunctionFrame()) {
        IonSpew(IonSpew_BaselineOSR, "  Compile function from top for later entry!");
        stat = CompileFunctionForBaseline(cx, script, frame, isConstructing);
    } else {
        return true;
    }

    if (stat == Method_Error) {
        IonSpew(IonSpew_BaselineOSR, "  Compile with Ion errored!");
        return false;
    }

    if (stat == Method_CantCompile)
        IonSpew(IonSpew_BaselineOSR, "  Can't compile with Ion!");
    else if (stat == Method_Skipped)
        IonSpew(IonSpew_BaselineOSR, "  Skipped compile with Ion!");
    else if (stat == Method_Compiled)
        IonSpew(IonSpew_BaselineOSR, "  Compiled with Ion!");
    else
        JS_NOT_REACHED("Invalid MethodStatus!");

    // Failed to compile.  Reset use count and return.
    if (stat != Method_Compiled) {
        // TODO: If stat == Method_CantCompile, insert stub that just skips the useCount
        // entirely, instead of resetting it.
        if (stat == Method_CantCompile ||
            (script->hasIonScript() && script->ion->bailoutExpected()))
        {
            IonSpew(IonSpew_BaselineOSR, "  Reset UseCount cantCompile=%s bailoutExpected=%s!",
                    stat == Method_CantCompile ? "yes" : "no",
                    (script->hasIonScript() && script->ion->bailoutExpected()) ? "yes" : "no");
            script->resetUseCount();
        }
        return true;
    }

    if (isLoopEntry) {
        IonSpew(IonSpew_BaselineOSR, "  OSR possible!");
        IonScript *ion = script->ionScript();
        *jitcodePtr = ion->method()->raw() + ion->osrEntryOffset();
    }

    return true;
}

//
// The following data is kept in a temporary heap-allocated buffer, stored in
// IonRuntime (high memory addresses at top, low at bottom):
//
//            +=================================+  --      <---- High Address
//            |                                 |   |
//            |     ...Locals/Stack...          |   |
//            |                                 |   |
//            +---------------------------------+   |
//            |                                 |   |
//            |     ...StackFrame...            |   |-- Fake StackFrame
//            |                                 |   |
//     +----> +---------------------------------+   |
//     |      |                                 |   |
//     |      |     ...Args/This...             |   |
//     |      |                                 |   |
//     |      +=================================+  --
//     |      |     Padding(Maybe Empty)        |
//     |      +=================================+  --
//     +------|-- stackFrame                    |   |-- IonOsrTempData
//            |   jitcode                       |   |
//            +=================================+  --      <---- Low Address
//
// A pointer to the IonOsrTempData is returned.

struct IonOsrTempData
{
    void *jitcode;
    uint8_t *stackFrame;
};

static IonOsrTempData *
PrepareOsrTempData(JSContext *cx, ICUseCount_Fallback *stub, BaselineFrame *frame,
                   HandleScript script, jsbytecode *pc, void *jitcode)
{
    // Calculate the (numLocals + numStackVals), and the number  of formal args.
    size_t numLocalsAndStackVals = frame->numValueSlots();
    size_t numFormalArgs = frame->isFunctionFrame() ? frame->numFormalArgs() : 0;

    // Calculate the amount of space to allocate:
    //      StackFrame space:
    //          (sizeof(Value) * (numLocals + numStackVals))
    //        + sizeof(StackFrame)
    //        + (sizeof(Value) * (numFormalArgs + 1))   // +1 for ThisV
    //
    //      IonOsrTempData space:
    //          sizeof(IonOsrTempData)

    size_t stackFrameSpace = (sizeof(Value) * numLocalsAndStackVals) + sizeof(StackFrame)
                           + (sizeof(Value) * (numFormalArgs + 1));
    size_t ionOsrTempDataSpace = sizeof(IonOsrTempData);

    size_t totalSpace = AlignBytes(stackFrameSpace, sizeof(Value)) +
                        AlignBytes(ionOsrTempDataSpace, sizeof(Value));

    IonOsrTempData *info = (IonOsrTempData *)cx->runtime->getIonRuntime(cx)->allocateOsrTempData(totalSpace);
    if (!info)
        return NULL;

    memset(info, 0, totalSpace);

    info->jitcode = jitcode;

    uint8_t *stackFrameStart = (uint8_t *)info + AlignBytes(ionOsrTempDataSpace, sizeof(Value));
    info->stackFrame = stackFrameStart + (numFormalArgs * sizeof(Value)) + sizeof(Value);

    //
    // Initialize the fake StackFrame.
    //

    // Copy formal args and thisv.
    memcpy(stackFrameStart, frame->formals() - 1, (numFormalArgs + 1) * sizeof(Value));

    // Initialize ScopeChain, Exec, and Flags fields in StackFrame struct.
    uint8_t *stackFrame = info->stackFrame;
    *((JSObject **) (stackFrame + StackFrame::offsetOfScopeChain())) = frame->scopeChain();
    if (frame->isFunctionFrame()) {
        // Store the function in exec field, and StackFrame::FUNCTION for flags.
        *((JSFunction **) (stackFrame + StackFrame::offsetOfExec())) = frame->fun();
        *((uint32_t *) (stackFrame + StackFrame::offsetOfFlags())) = StackFrame::FUNCTION;
    } else {
        *((RawScript *) (stackFrame + StackFrame::offsetOfExec())) = frame->script();
        *((uint32_t *) (stackFrame + StackFrame::offsetOfFlags())) = 0;
    }

    // Do locals and stack values.  Note that in the fake StackFrame, these go from
    // low to high addresses, while on the C stack, they go from high to low addresses.
    // So we can't use memcpy on this, but must copy the values in reverse order.
    Value *stackFrameLocalsStart = (Value *) (stackFrame + sizeof(StackFrame));
    for (size_t i = 0; i < numLocalsAndStackVals; i++)
        stackFrameLocalsStart[i] = *(frame->valueSlot(i));

    IonSpew(IonSpew_BaselineOSR, "Allocated IonOsrTempData at %p", (void *) info);
    IonSpew(IonSpew_BaselineOSR, "Jitcode is %p", info->jitcode);

    // All done.
    return info;
}

static bool
DoUseCountFallback(JSContext *cx, ICUseCount_Fallback *stub, BaselineFrame *frame,
                   IonOsrTempData **infoPtr)
{
    JS_ASSERT(infoPtr);
    *infoPtr = NULL;

    // A TI OOM will disable TI and Ion.
    if (!ion::IsEnabled(cx))
        return true;

    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    bool isLoopEntry = JSOp(*pc) == JSOP_LOOPENTRY;

    FallbackICSpew(cx, stub, "UseCount(%d)", isLoopEntry ? int(pc - script->code) : int(-1));

    if (!script->canIonCompile()) {
        // TODO: ASSERT that ion-compilation-disabled checker stub doesn't exist.
        // TODO: Clear all optimized stubs.
        // TODO: Add a ion-compilation-disabled checker IC stub
        script->resetUseCount();
        return true;
    }

    JS_ASSERT(!script->isIonCompilingOffThread());

    // If Ion script exists, but PC is not at a loop entry, then Ion will be entered for
    // this script at an appropriate LOOPENTRY or the next time this function is called.
    if (script->hasIonScript() && !isLoopEntry) {
        IonSpew(IonSpew_BaselineOSR, "IonScript exists, but not at loop entry!");
        // TODO: ASSERT that a ion-script-already-exists checker stub doesn't exist.
        // TODO: Clear all optimized stubs.
        // TODO: Add a ion-script-already-exists checker stub.
        return true;
    }

    // Ensure that Ion-compiled code is available.
    IonSpew(IonSpew_BaselineOSR,
            "UseCount for %s:%d reached %d at pc %p, trying to switch to Ion!",
            script->filename(), script->lineno, (int) script->getUseCount(), (void *) pc);
    void *jitcode = NULL;
    if (!EnsureCanEnterIon(cx, stub, frame, script, pc, &jitcode))
        return false;

    // Jitcode should only be set here if not at loop entry.
    JS_ASSERT_IF(!isLoopEntry, !jitcode);
    if (!jitcode)
        return true;

    // Prepare the temporary heap copy of the fake StackFrame and actual args list.
    IonSpew(IonSpew_BaselineOSR, "Got jitcode.  Preparing for OSR into ion.");
    IonOsrTempData *info = PrepareOsrTempData(cx, stub, frame, script, pc, jitcode);
    if (!info)
        return false;
    *infoPtr = info;

    return true;
}

typedef bool (*DoUseCountFallbackFn)(JSContext *, ICUseCount_Fallback *, BaselineFrame *frame,
                                     IonOsrTempData **infoPtr);
static const VMFunction DoUseCountFallbackInfo =
    FunctionInfo<DoUseCountFallbackFn>(DoUseCountFallback);

bool
ICUseCount_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    // enterStubFrame is going to clobber the BaselineFrameReg, save it in R0.scratchReg()
    // first.
    masm.movePtr(BaselineFrameReg, R0.scratchReg());

    // Push a stub frame so that we can perform a non-tail call.
    enterStubFrame(masm, R1.scratchReg());

    Label noCompiledCode;
    // Call DoUseCountFallback to compile/check-for Ion-compiled function
    {
        // Push IonOsrTempData pointer storage
        masm.subPtr(Imm32(sizeof(void *)), BaselineStackReg);
        masm.push(BaselineStackReg);

        // Push IonJSFrameLayout pointer.
        masm.loadBaselineFramePtr(R0.scratchReg(), R0.scratchReg());
        masm.push(R0.scratchReg());

        // Push stub pointer.
        masm.push(BaselineStubReg);

        if (!callVM(DoUseCountFallbackInfo, masm))
            return false;

        // Pop IonOsrTempData pointer.
        masm.pop(R0.scratchReg());

        leaveStubFrame(masm);

        // If no IonCode was found, then skip just exit the IC.
        masm.branchPtr(Assembler::Equal, R0.scratchReg(), ImmWord((void*) NULL), &noCompiledCode);
    }

    // Get a scratch register.
    GeneralRegisterSet regs(availableGeneralRegs(0));
    Register osrDataReg = R0.scratchReg();
    regs.take(osrDataReg);
    regs.takeUnchecked(OsrFrameReg);

    Register scratchReg = regs.takeAny();

    // At this point, stack looks like:
    //  +-> [...Calling-Frame...]
    //  |   [...Actual-Args/ThisV/ArgCount/Callee...]
    //  |   [Descriptor]
    //  |   [Return-Addr]
    //  +---[Saved-FramePtr]            <-- BaselineFrameReg points here.
    //      [...Baseline-Frame...]

    // Restore the stack pointer to point to the saved frame pointer.
    masm.movePtr(BaselineFrameReg, BaselineStackReg);

    // Discard saved frame pointer, so that the return address is on top of
    // the stack.
    masm.pop(scratchReg);

    // Jump into Ion.
    masm.loadPtr(Address(osrDataReg, offsetof(IonOsrTempData, jitcode)), scratchReg);
    masm.loadPtr(Address(osrDataReg, offsetof(IonOsrTempData, stackFrame)), OsrFrameReg);
    masm.jump(scratchReg);

    // No jitcode available, do nothing.
    masm.bind(&noCompiledCode);
    EmitReturnFromIC(masm);
    return true;
}

//
// ICProfile_Fallback
//

static bool
DoProfilerFallback(JSContext *cx, BaselineFrame *frame, ICProfiler_Fallback *stub)
{
    RootedScript script(cx, frame->script());
    RootedFunction func(cx, frame->maybeFun());
    mozilla::DebugOnly<ICEntry *> icEntry = stub->icEntry();

    FallbackICSpew(cx, stub, "Profiler");

    SPSProfiler *profiler = &cx->runtime->spsProfiler;

    // Manually enter SPS this time.
    JS_ASSERT(profiler->enabled());
    if (!cx->runtime->spsProfiler.enter(cx, script, func))
        return false;
    frame->setPushedSPSFrame();

    // Unlink any existing PushFunction stub (which may hold stale 'const char *' to
    // the profile string.
    JS_ASSERT_IF(icEntry->firstStub() != stub,
                 icEntry->firstStub()->isProfiler_PushFunction() &&
                 icEntry->firstStub()->next() == stub);
    stub->unlinkStubsWithKind(cx, ICStub::Profiler_PushFunction);
    JS_ASSERT(icEntry->firstStub() == stub);

    // Generate the string to use to identify this stack frame.
    const char *string = profiler->profileString(cx, script, func);
    if (string == NULL)
        return false;

    IonSpew(IonSpew_BaselineIC, "  Generating Profiler_PushFunction stub for %s:%d",
            script->filename(), script->lineno);

    // Create a new optimized stub.
    ICProfiler_PushFunction::Compiler compiler(cx, string, script);
    ICStub *optStub = compiler.getStub(compiler.getStubSpace(script));
    if (!optStub)
        return false;
    stub->addNewStub(optStub);

    return true;
}

typedef bool (*DoProfilerFallbackFn)(JSContext *, BaselineFrame *frame, ICProfiler_Fallback *);
static const VMFunction DoProfilerFallbackInfo =
    FunctionInfo<DoProfilerFallbackFn>(DoProfilerFallback);

bool
ICProfiler_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.push(BaselineStubReg);         // Push stub.
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg()); // Push frame.

    return tailCallVM(DoProfilerFallbackInfo, masm);
}

bool
ICProfiler_PushFunction::Compiler::generateStubCode(MacroAssembler &masm)
{

    Register scratch = R0.scratchReg();
    Register scratch2 = R1.scratchReg();

    // Profiling should be enabled if we ever reach here.
#ifdef DEBUG
    Label spsEnabled;
    uint32_t *enabledAddr = cx->runtime->spsProfiler.addressOfEnabled();
    masm.branch32(Assembler::NotEqual, AbsoluteAddress(enabledAddr), Imm32(0), &spsEnabled);
    masm.breakpoint();
    masm.bind(&spsEnabled);
#endif

    // Push SPS entry.
    masm.spsPushFrame(&cx->runtime->spsProfiler,
                      Address(BaselineStubReg, ICProfiler_PushFunction::offsetOfStr()),
                      Address(BaselineStubReg, ICProfiler_PushFunction::offsetOfScript()),
                      scratch,
                      scratch2);

    // Mark frame as having profiler entry pushed.
    Address flagsOffset(BaselineFrameReg, BaselineFrame::reverseOffsetOfFlags());
    masm.or32(Imm32(BaselineFrame::HAS_PUSHED_SPS_FRAME), flagsOffset);

    EmitReturnFromIC(masm);

    return true;
}

//
// TypeMonitor_Fallback
//

bool
ICTypeMonitor_Fallback::addMonitorStubForValue(JSContext *cx, HandleScript script, HandleValue val)
{
    bool wasDetachedMonitorChain = lastMonitorStubPtrAddr_ == NULL;
    JS_ASSERT_IF(wasDetachedMonitorChain, numOptimizedMonitorStubs_ == 0);

    if (numOptimizedMonitorStubs_ >= MAX_OPTIMIZED_STUBS) {
        // TODO: if the TypeSet becomes unknown or has the AnyObject type,
        // replace stubs with a single stub to handle these.
        return true;
    }

    if (val.isPrimitive()) {
        JS_ASSERT(!val.isMagic());
        JSValueType type = val.isDouble() ? JSVAL_TYPE_DOUBLE : val.extractNonDoubleType();

        // Check for existing TypeMonitor stub.
        ICTypeMonitor_PrimitiveSet *existingStub = NULL;
        for (ICStubConstIterator iter = firstMonitorStub(); !iter.atEnd(); iter++) {
            if (iter->isTypeMonitor_PrimitiveSet()) {
                existingStub = iter->toTypeMonitor_PrimitiveSet();
                if (existingStub->containsType(type))
                    return true;
            }
        }

        ICTypeMonitor_PrimitiveSet::Compiler compiler(cx, existingStub, type);
        ICStub *stub = existingStub ? compiler.updateStub()
                                    : compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        IonSpew(IonSpew_BaselineIC, "  %s TypeMonitor stub %p for primitive type %d",
                existingStub ? "Modified existing" : "Created new", stub, type);

        if (!existingStub) {
            JS_ASSERT(!hasStub(TypeMonitor_PrimitiveSet));
            addOptimizedMonitorStub(stub);
        }

    } else if (val.toObject().hasSingletonType()) {
        RootedObject obj(cx, &val.toObject());

        // Check for existing TypeMonitor stub.
        for (ICStubConstIterator iter = firstMonitorStub(); !iter.atEnd(); iter++) {
            if (iter->isTypeMonitor_SingleObject() &&
                iter->toTypeMonitor_SingleObject()->object() == obj)
            {
                return true;
            }
        }

        ICTypeMonitor_SingleObject::Compiler compiler(cx, obj);
        ICStub *stub = compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        IonSpew(IonSpew_BaselineIC, "  Added TypeMonitor stub %p for singleton %p",
                stub, obj.get());

        addOptimizedMonitorStub(stub);

    } else {
        RootedTypeObject type(cx, val.toObject().type());

        // Check for existing TypeMonitor stub.
        for (ICStubConstIterator iter = firstMonitorStub(); !iter.atEnd(); iter++) {
            if (iter->isTypeMonitor_TypeObject() &&
                iter->toTypeMonitor_TypeObject()->type() == type)
            {
                return true;
            }
        }

        ICTypeMonitor_TypeObject::Compiler compiler(cx, type);
        ICStub *stub = compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        IonSpew(IonSpew_BaselineIC, "  Added TypeMonitor stub %p for TypeObject %p",
                stub, type.get());

        addOptimizedMonitorStub(stub);
    }

    bool firstMonitorStubAdded = wasDetachedMonitorChain && (numOptimizedMonitorStubs_ > 0);

    if (firstMonitorStubAdded) {
        // Was an empty monitor chain before, but a new stub was added.  This is the
        // only time that any main stubs' firstMonitorStub fields need to be updated to
        // refer to the newly added monitor stub.
        ICStub *firstStub = mainFallbackStub_->icEntry()->firstStub();
        for (ICStubConstIterator iter = firstStub; !iter.atEnd(); iter++) {
            // Non-monitored stubs are used if the result has always the same type,
            // e.g. a StringLength stub will always return int32.
            if (!iter->isMonitored())
                continue;

            // Since we just added the first optimized monitoring stub, any
            // existing main stub's |firstMonitorStub| MUST be pointing to the fallback
            // monitor stub (i.e. this stub).
            JS_ASSERT(iter->toMonitoredStub()->firstMonitorStub() == this);
            iter->toMonitoredStub()->updateFirstMonitorStub(firstMonitorStub_);
        }
    }

    return true;
}

static bool
DoTypeMonitorFallback(JSContext *cx, BaselineFrame *frame, ICTypeMonitor_Fallback *stub,
                      HandleValue value, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    TypeFallbackICSpew(cx, stub, "TypeMonitor");

    uint32_t argument;
    if (stub->monitorsThis()) {
        JS_ASSERT(pc == script->code);
        types::TypeScript::SetThis(cx, script, value);
    } else if (stub->monitorsArgument(&argument)) {
        JS_ASSERT(pc == script->code);
        types::TypeScript::SetArgument(cx, script, argument, value);
    } else {
        types::TypeScript::Monitor(cx, script, pc, value);
    }

    if (!stub->addMonitorStubForValue(cx, script, value))
        return false;

    // Copy input value to res.
    res.set(value);
    return true;
}

typedef bool (*DoTypeMonitorFallbackFn)(JSContext *, BaselineFrame *, ICTypeMonitor_Fallback *,
                                        HandleValue, MutableHandleValue);
static const VMFunction DoTypeMonitorFallbackInfo =
    FunctionInfo<DoTypeMonitorFallbackFn>(DoTypeMonitorFallback);

bool
ICTypeMonitor_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoTypeMonitorFallbackInfo, masm);
}

bool
ICTypeMonitor_PrimitiveSet::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label success;
    if ((flags_ & TypeToFlag(JSVAL_TYPE_INT32)) && !(flags_ & TypeToFlag(JSVAL_TYPE_DOUBLE)))
        masm.branchTestInt32(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_DOUBLE))
        masm.branchTestNumber(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_UNDEFINED))
        masm.branchTestUndefined(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_BOOLEAN))
        masm.branchTestBoolean(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_STRING))
        masm.branchTestString(Assembler::Equal, R0, &success);

    // Currently, we will never generate primitive stub checks for object.  However,
    // when we do get to the point where we want to collapse our monitor chains of
    // objects and singletons down (when they get too long) to a generic "any object"
    // in coordination with the typeset doing the same thing, this will need to
    // be re-enabled.
    /*
    if (flags_ & TypeToFlag(JSVAL_TYPE_OBJECT))
        masm.branchTestObject(Assembler::Equal, R0, &success);
    */
    JS_ASSERT(!(flags_ & TypeToFlag(JSVAL_TYPE_OBJECT)));

    if (flags_ & TypeToFlag(JSVAL_TYPE_NULL))
        masm.branchTestNull(Assembler::Equal, R0, &success);

    EmitStubGuardFailure(masm);

    masm.bind(&success);
    EmitReturnFromIC(masm);
    return true;
}

bool
ICTypeMonitor_SingleObject::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Guard on the object's identity.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    Address expectedObject(BaselineStubReg, ICTypeMonitor_SingleObject::offsetOfObject());
    masm.branchPtr(Assembler::NotEqual, expectedObject, obj, &failure);

    EmitReturnFromIC(masm);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICTypeMonitor_TypeObject::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Guard on the object's TypeObject.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(obj, JSObject::offsetOfType()), R1.scratchReg());

    Address expectedType(BaselineStubReg, ICTypeMonitor_TypeObject::offsetOfType());
    masm.branchPtr(Assembler::NotEqual, expectedType, R1.scratchReg(), &failure);

    EmitReturnFromIC(masm);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICUpdatedStub::addUpdateStubForValue(JSContext *cx, HandleScript script, HandleObject obj,
                                     RawId id, HandleValue val)
{
    if (numOptimizedStubs_ >= MAX_OPTIMIZED_STUBS) {
        // TODO: if the TypeSet becomes unknown or has the AnyObject type,
        // replace stubs with a single stub to handle these.
        return true;
    }

    types::EnsureTrackPropertyTypes(cx, obj, id);

    if (val.isPrimitive()) {
        JSValueType type = val.isDouble() ? JSVAL_TYPE_DOUBLE : val.extractNonDoubleType();

        // Check for existing TypeUpdate stub.
        ICTypeUpdate_PrimitiveSet *existingStub = NULL;
        for (ICStubConstIterator iter = firstUpdateStub_; !iter.atEnd(); iter++) {
            if (iter->isTypeUpdate_PrimitiveSet()) {
                existingStub = iter->toTypeUpdate_PrimitiveSet();
                if (existingStub->containsType(type))
                    return true;
            }
        }

        ICTypeUpdate_PrimitiveSet::Compiler compiler(cx, existingStub, type);
        ICStub *stub = existingStub ? compiler.updateStub()
                                    : compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;
        if (!existingStub) {
            JS_ASSERT(!hasTypeUpdateStub(TypeUpdate_PrimitiveSet));
            addOptimizedUpdateStub(stub);
        }

        IonSpew(IonSpew_BaselineIC, "  %s TypeUpdate stub %p for primitive type %d",
                existingStub ? "Modified existing" : "Created new", stub, type);

    } else if (val.toObject().hasSingletonType()) {
        RootedObject obj(cx, &val.toObject());

        // Check for existing TypeUpdate stub.
        for (ICStubConstIterator iter = firstUpdateStub_; !iter.atEnd(); iter++) {
            if (iter->isTypeUpdate_SingleObject() &&
                iter->toTypeUpdate_SingleObject()->object() == obj)
            {
                return true;
            }
        }

        ICTypeUpdate_SingleObject::Compiler compiler(cx, obj);
        ICStub *stub = compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        IonSpew(IonSpew_BaselineIC, "  Added TypeUpdate stub %p for singleton %p", stub, obj.get());

        addOptimizedUpdateStub(stub);

    } else {
        RootedTypeObject type(cx, val.toObject().type());

        // Check for existing TypeUpdate stub.
        for (ICStubConstIterator iter = firstUpdateStub_; !iter.atEnd(); iter++) {
            if (iter->isTypeUpdate_TypeObject() &&
                iter->toTypeUpdate_TypeObject()->type() == type)
            {
                return true;
            }
        }

        ICTypeUpdate_TypeObject::Compiler compiler(cx, type);
        ICStub *stub = compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        IonSpew(IonSpew_BaselineIC, "  Added TypeUpdate stub %p for TypeObject %p",
                stub, type.get());

        addOptimizedUpdateStub(stub);
    }

    return true;
}

//
// TypeUpdate_Fallback
//
static bool
DoTypeUpdateFallback(JSContext *cx, BaselineFrame *frame, ICUpdatedStub *stub, HandleValue objval,
                     HandleValue value)
{
    FallbackICSpew(cx, stub->getChainFallback(), "TypeUpdate(%s)",
                   ICStub::KindString(stub->kind()));

    RootedScript script(cx, frame->script());
    RootedObject obj(cx, &objval.toObject());
    RootedId id(cx);

    switch(stub->kind()) {
      case ICStub::SetElem_Dense:
      case ICStub::SetElem_DenseAdd: {
        JS_ASSERT(obj->isNative());
        id = JSID_VOID;
        types::AddTypePropertyId(cx, obj, id, value);
        break;
      }
      case ICStub::SetProp_Native:
      case ICStub::SetProp_NativeAdd: {
        JS_ASSERT(obj->isNative());
        jsbytecode *pc = stub->getChainFallback()->icEntry()->pc(script);
        id = NameToId(script->getName(pc));
        types::AddTypePropertyId(cx, obj, id, value);
        break;
      }
      default:
        JS_NOT_REACHED("Invalid stub");
        return false;
    }

    return stub->addUpdateStubForValue(cx, script, obj, id, value);
}

typedef bool (*DoTypeUpdateFallbackFn)(JSContext *, BaselineFrame *, ICUpdatedStub *, HandleValue,
                                       HandleValue);
const VMFunction DoTypeUpdateFallbackInfo =
    FunctionInfo<DoTypeUpdateFallbackFn>(DoTypeUpdateFallback);

bool
ICTypeUpdate_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    // Just store false into R1.scratchReg() and return.
    masm.move32(Imm32(0), R1.scratchReg());
    EmitReturnFromIC(masm);
    return true;
}

bool
ICTypeUpdate_PrimitiveSet::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label success;
    if ((flags_ & TypeToFlag(JSVAL_TYPE_INT32)) && !(flags_ & TypeToFlag(JSVAL_TYPE_DOUBLE)))
        masm.branchTestInt32(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_DOUBLE))
        masm.branchTestNumber(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_UNDEFINED))
        masm.branchTestUndefined(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_BOOLEAN))
        masm.branchTestBoolean(Assembler::Equal, R0, &success);

    if (flags_ & TypeToFlag(JSVAL_TYPE_STRING))
        masm.branchTestString(Assembler::Equal, R0, &success);

    // Currently, we will never generate primitive stub checks for object.  However,
    // when we do get to the point where we want to collapse our monitor chains of
    // objects and singletons down (when they get too long) to a generic "any object"
    // in coordination with the typeset doing the same thing, this will need to
    // be re-enabled.
    /*
    if (flags_ & TypeToFlag(JSVAL_TYPE_OBJECT))
        masm.branchTestObject(Assembler::Equal, R0, &success);
    */
    JS_ASSERT(!(flags_ & TypeToFlag(JSVAL_TYPE_OBJECT)));

    if (flags_ & TypeToFlag(JSVAL_TYPE_NULL))
        masm.branchTestNull(Assembler::Equal, R0, &success);

    EmitStubGuardFailure(masm);

    // Type matches, load true into R1.scratchReg() and return.
    masm.bind(&success);
    masm.mov(Imm32(1), R1.scratchReg());
    EmitReturnFromIC(masm);

    return true;
}

bool
ICTypeUpdate_SingleObject::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Guard on the object's identity.
    Register obj = masm.extractObject(R0, R1.scratchReg());
    Address expectedObject(BaselineStubReg, ICTypeUpdate_SingleObject::offsetOfObject());
    masm.branchPtr(Assembler::NotEqual, expectedObject, obj, &failure);

    // Identity matches, load true into R1.scratchReg() and return.
    masm.mov(Imm32(1), R1.scratchReg());
    EmitReturnFromIC(masm);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICTypeUpdate_TypeObject::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Guard on the object's TypeObject.
    Register obj = masm.extractObject(R0, R1.scratchReg());
    masm.loadPtr(Address(obj, JSObject::offsetOfType()), R1.scratchReg());

    Address expectedType(BaselineStubReg, ICTypeUpdate_TypeObject::offsetOfType());
    masm.branchPtr(Assembler::NotEqual, expectedType, R1.scratchReg(), &failure);

    // Type matches, load true into R1.scratchReg() and return.
    masm.mov(Imm32(1), R1.scratchReg());
    EmitReturnFromIC(masm);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// This_Fallback
//

static bool
DoThisFallback(JSContext *cx, ICThis_Fallback *stub, HandleValue thisv, MutableHandleValue ret)
{
    FallbackICSpew(cx, stub, "This");

    ret.set(thisv);
    bool modified;
    if (!BoxNonStrictThis(cx, ret, &modified))
        return false;
    return true;
}

typedef bool (*DoThisFallbackFn)(JSContext *, ICThis_Fallback *, HandleValue, MutableHandleValue);
static const VMFunction DoThisFallbackInfo = FunctionInfo<DoThisFallbackFn>(DoThisFallback);

bool
ICThis_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);

    return tailCallVM(DoThisFallbackInfo, masm);
}

//
// NewArray_Fallback
//

static bool
DoNewArray(JSContext *cx, ICNewArray_Fallback *stub, uint32_t length,
           HandleTypeObject type, MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "NewArray");

    RawObject obj = NewInitArray(cx, length, type);
    if (!obj)
        return false;

    res.setObject(*obj);
    return true;
}

typedef bool(*DoNewArrayFn)(JSContext *, ICNewArray_Fallback *, uint32_t, HandleTypeObject,
                            MutableHandleValue);
static const VMFunction DoNewArrayInfo = FunctionInfo<DoNewArrayFn>(DoNewArray);

bool
ICNewArray_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.push(R1.scratchReg()); // type
    masm.push(R0.scratchReg()); // length
    masm.push(BaselineStubReg); // stub.

    return tailCallVM(DoNewArrayInfo, masm);
}

//
// NewObject_Fallback
//

static bool
DoNewObject(JSContext *cx, ICNewObject_Fallback *stub, HandleObject templateObject,
            MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "NewObject");

    RawObject obj = NewInitObject(cx, templateObject);
    if (!obj)
        return false;

    res.setObject(*obj);
    return true;
}

typedef bool(*DoNewObjectFn)(JSContext *, ICNewObject_Fallback *, HandleObject,
                             MutableHandleValue);
static const VMFunction DoNewObjectInfo = FunctionInfo<DoNewObjectFn>(DoNewObject);

bool
ICNewObject_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.push(R0.scratchReg()); // template
    masm.push(BaselineStubReg); // stub.

    return tailCallVM(DoNewObjectInfo, masm);
}

//
// Compare_Fallback
//

static bool
DoCompareFallback(JSContext *cx, BaselineFrame *frame, ICCompare_Fallback *stub, HandleValue lhs,
                  HandleValue rhs, MutableHandleValue ret)
{
    jsbytecode *pc = stub->icEntry()->pc(frame->script());
    JSOp op = JSOp(*pc);

    FallbackICSpew(cx, stub, "Compare(%s)", js_CodeName[op]);

    // Case operations in a CONDSWITCH are performing strict equality.
    if (op == JSOP_CASE)
        op = JSOP_STRICTEQ;

    // Don't pass lhs/rhs directly, we need the original values when
    // generating stubs.
    RootedValue lhsCopy(cx, lhs);
    RootedValue rhsCopy(cx, rhs);

    // Perform the compare operation.
    JSBool out;
    switch(op) {
      case JSOP_LT:
        if (!LessThan(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_LE:
        if (!LessThanOrEqual(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_GT:
        if (!GreaterThan(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_GE:
        if (!GreaterThanOrEqual(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_EQ:
        if (!LooselyEqual<true>(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_NE:
        if (!LooselyEqual<false>(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_STRICTEQ:
        if (!StrictlyEqual<true>(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      case JSOP_STRICTNE:
        if (!StrictlyEqual<false>(cx, &lhsCopy, &rhsCopy, &out))
            return false;
        break;
      default:
        JS_ASSERT(!"Unhandled baseline compare op");
        return false;
    }

    ret.setBoolean(out);

    // Check to see if a new stub should be generated.
    if (stub->numOptimizedStubs() >= ICCompare_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    RawScript script = frame->script();

    // Try to generate new stubs.
    if (lhs.isInt32() && rhs.isInt32()) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Int32, Int32) stub", js_CodeName[op]);
        ICCompare_Int32::Compiler compiler(cx, op);
        ICStub *int32Stub = compiler.getStub(compiler.getStubSpace(script));
        if (!int32Stub)
            return false;

        stub->addNewStub(int32Stub);
        return true;
    }

    if (lhs.isNumber() && rhs.isNumber()) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Number, Number) stub", js_CodeName[op]);

        // Unlink int32 stubs, it's faster to always use the double stub.
        stub->unlinkStubsWithKind(cx, ICStub::Compare_Int32);

        ICCompare_Double::Compiler compiler(cx, op);
        ICStub *doubleStub = compiler.getStub(compiler.getStubSpace(script));
        if (!doubleStub)
            return false;

        stub->addNewStub(doubleStub);
        return true;
    }

    if ((lhs.isNumber() && rhs.isUndefined()) ||
        (lhs.isUndefined() && rhs.isNumber()))
    {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(%s, %s) stub", js_CodeName[op],
                    rhs.isUndefined() ? "Number" : "Undefined",
                    rhs.isUndefined() ? "Undefined" : "Number");
        ICCompare_NumberWithUndefined::Compiler compiler(cx, op, lhs.isUndefined());
        ICStub *doubleStub = compiler.getStub(compiler.getStubSpace(script));
        if (!stub)
            return false;

        stub->addNewStub(doubleStub);
        return true;
    }

    if (lhs.isBoolean() && rhs.isBoolean()) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Boolean, Boolean) stub", js_CodeName[op]);
        ICCompare_Boolean::Compiler compiler(cx, op);
        ICStub *booleanStub = compiler.getStub(compiler.getStubSpace(script));
        if (!booleanStub)
            return false;

        stub->addNewStub(booleanStub);
        return true;
    }

    if ((lhs.isBoolean() && rhs.isInt32()) || (lhs.isInt32() && rhs.isBoolean())) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(%s, %s) stub", js_CodeName[op],
                    rhs.isInt32() ? "Boolean" : "Int32",
                    rhs.isInt32() ? "Int32" : "Boolean");
        ICCompare_Int32WithBoolean::Compiler compiler(cx, op, lhs.isInt32());
        ICStub *optStub = compiler.getStub(compiler.getStubSpace(script));
        if (!optStub)
            return false;

        stub->addNewStub(optStub);
        return true;
    }

    if (IsEqualityOp(op)) {
        if (lhs.isString() && rhs.isString() && !stub->hasStub(ICStub::Compare_String)) {
            IonSpew(IonSpew_BaselineIC, "  Generating %s(String, String) stub", js_CodeName[op]);
            ICCompare_String::Compiler compiler(cx, op);
            ICStub *stringStub = compiler.getStub(compiler.getStubSpace(script));
            if (!stringStub)
                return false;

            stub->addNewStub(stringStub);
            return true;
        }

        if (lhs.isObject() && rhs.isObject()) {
            JS_ASSERT(!stub->hasStub(ICStub::Compare_Object));
            IonSpew(IonSpew_BaselineIC, "  Generating %s(Object, Object) stub", js_CodeName[op]);
            ICCompare_Object::Compiler compiler(cx, op);
            ICStub *objectStub = compiler.getStub(compiler.getStubSpace(script));
            if (!objectStub)
                return false;

            stub->addNewStub(objectStub);
            return true;
        }

        if ((lhs.isObject() || lhs.isNull() || lhs.isUndefined()) &&
            (rhs.isObject() || rhs.isNull() || rhs.isUndefined()) &&
            !stub->hasStub(ICStub::Compare_ObjectWithUndefined))
        {
            IonSpew(IonSpew_BaselineIC, "  Generating %s(Obj/Null/Undef, Obj/Null/Undef) stub",
                    js_CodeName[op]);
            bool lhsIsUndefined = lhs.isNull() || lhs.isUndefined();
            bool compareWithNull = lhs.isNull() || rhs.isNull();
            ICCompare_ObjectWithUndefined::Compiler compiler(cx, op,
                                                             lhsIsUndefined, compareWithNull);
            ICStub *objectStub = compiler.getStub(compiler.getStubSpace(script));
            if (!objectStub)
                return false;

            stub->addNewStub(objectStub);
            return true;
        }
    }

    return true;
}

typedef bool (*DoCompareFallbackFn)(JSContext *, BaselineFrame *, ICCompare_Fallback *,
                                    HandleValue, HandleValue, MutableHandleValue);
static const VMFunction DoCompareFallbackInfo =
    FunctionInfo<DoCompareFallbackFn>(DoCompareFallback, PopValues(2));

bool
ICCompare_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoCompareFallbackInfo, masm);
}

//
// Compare_String
//

bool
ICCompare_String::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);
    masm.branchTestString(Assembler::NotEqual, R1, &failure);

    JS_ASSERT(IsEqualityOp(op));

    Register left = masm.extractString(R0, ExtractTemp0);
    Register right = masm.extractString(R1, ExtractTemp1);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();
    // x86 doesn't have the luxury of a second scratch.
    Register scratchReg2;
    if (regs.empty()) {
        scratchReg2 = BaselineStubReg;
        masm.push(BaselineStubReg);
    } else {
        scratchReg2 = regs.takeAny();
    }
    JS_ASSERT(scratchReg2 != scratchReg);

    Label inlineCompareFailed;
    masm.compareStrings(op, left, right, scratchReg2, scratchReg, &inlineCompareFailed);
    masm.tagValue(JSVAL_TYPE_BOOLEAN, scratchReg2, R0);
    if (scratchReg2 == BaselineStubReg)
        masm.pop(BaselineStubReg);
    EmitReturnFromIC(masm);

    masm.bind(&inlineCompareFailed);
    if (scratchReg2 == BaselineStubReg)
        masm.pop(BaselineStubReg);
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Compare_Boolean
//

bool
ICCompare_Boolean::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestBoolean(Assembler::NotEqual, R0, &failure);
    masm.branchTestBoolean(Assembler::NotEqual, R1, &failure);

    Register left = masm.extractInt32(R0, ExtractTemp0);
    Register right = masm.extractInt32(R1, ExtractTemp1);

    // Compare payload regs of R0 and R1.
    Assembler::Condition cond = JSOpToCondition(op, /* signed = */true);
    masm.cmp32(left, right);
    masm.emitSet(cond, left);

    // Box the result and return
    masm.tagValue(JSVAL_TYPE_BOOLEAN, left, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Compare_NumberWithUndefined
//

bool
ICCompare_NumberWithUndefined::Compiler::generateStubCode(MacroAssembler &masm)
{
    ValueOperand numberOperand, undefinedOperand;
    if (lhsIsUndefined) {
        numberOperand = R1;
        undefinedOperand = R0;
    } else {
        numberOperand = R0;
        undefinedOperand = R1;
    }

    Label failure;
    masm.branchTestNumber(Assembler::NotEqual, numberOperand, &failure);
    masm.branchTestUndefined(Assembler::NotEqual, undefinedOperand, &failure);

    // Comparing a number with undefined will always be true for NE/STRICTNE,
    // and always be false for other compare ops.
    masm.moveValue(BooleanValue(op == JSOP_NE || op == JSOP_STRICTNE), R0);

    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Compare_Object
//

bool
ICCompare_Object::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestObject(Assembler::NotEqual, R1, &failure);

    JS_ASSERT(IsEqualityOp(op));

    Register left = masm.extractObject(R0, ExtractTemp0);
    Register right = masm.extractObject(R1, ExtractTemp1);

    Label ifTrue;
    masm.branchPtr(JSOpToCondition(op, /* signed = */true), left, right, &ifTrue);

    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    masm.bind(&ifTrue);
    masm.moveValue(BooleanValue(true), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Compare_ObjectWithUndefined
//

bool
ICCompare_ObjectWithUndefined::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(IsEqualityOp(op));

    ValueOperand objectOperand, undefinedOperand;
    if (lhsIsUndefined) {
        objectOperand = R1;
        undefinedOperand = R0;
    } else {
        objectOperand = R0;
        undefinedOperand = R1;
    }

    Label failure;
    if (compareWithNull)
        masm.branchTestNull(Assembler::NotEqual, undefinedOperand, &failure);
    else
        masm.branchTestUndefined(Assembler::NotEqual, undefinedOperand, &failure);

    Label notObject;
    masm.branchTestObject(Assembler::NotEqual, objectOperand, &notObject);

    if (op == JSOP_STRICTEQ || op == JSOP_STRICTNE) {
        // obj !== undefined for all objects.
        masm.moveValue(BooleanValue(op == JSOP_STRICTNE), R0);
        EmitReturnFromIC(masm);
    } else {
        // obj != undefined only where !obj->getClass()->emulatesUndefined()
        Label emulatesUndefined;
        Register obj = masm.extractObject(objectOperand, ExtractTemp0);
        masm.loadPtr(Address(obj, JSObject::offsetOfType()), obj);
        masm.loadPtr(Address(obj, offsetof(types::TypeObject, clasp)), obj);
        masm.branchTest32(Assembler::NonZero,
                          Address(obj, Class::offsetOfFlags()),
                          Imm32(JSCLASS_EMULATES_UNDEFINED),
                          &emulatesUndefined);
        masm.moveValue(BooleanValue(op == JSOP_NE), R0);
        EmitReturnFromIC(masm);
        masm.bind(&emulatesUndefined);
        masm.moveValue(BooleanValue(op == JSOP_EQ), R0);
        EmitReturnFromIC(masm);
    }

    masm.bind(&notObject);

    // Also support null == null or undefined == undefined comparisons.
    if (compareWithNull)
        masm.branchTestNull(Assembler::NotEqual, objectOperand, &failure);
    else
        masm.branchTestUndefined(Assembler::NotEqual, objectOperand, &failure);

    masm.moveValue(BooleanValue(op == JSOP_STRICTEQ || op == JSOP_EQ), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Compare_Int32WithBoolean
//

bool
ICCompare_Int32WithBoolean::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    ValueOperand int32Val;
    ValueOperand boolVal;
    if (lhsIsInt32_) {
        int32Val = R0;
        boolVal = R1;
    } else {
        boolVal = R0;
        int32Val = R1;
    }
    masm.branchTestBoolean(Assembler::NotEqual, boolVal, &failure);
    masm.branchTestInt32(Assembler::NotEqual, int32Val, &failure);

    if (op_ == JSOP_STRICTEQ || op_ == JSOP_STRICTNE) {
        // Ints and booleans are never strictly equal, always strictly not equal.
        masm.moveValue(BooleanValue(op_ == JSOP_STRICTNE), R0);
        EmitReturnFromIC(masm);
    } else {
        Register boolReg = masm.extractBoolean(boolVal, ExtractTemp0);
        Register int32Reg = masm.extractInt32(int32Val, ExtractTemp1);

        // Compare payload regs of R0 and R1.
        Assembler::Condition cond = JSOpToCondition(op_, /* signed = */true);
        masm.cmp32(lhsIsInt32_ ? int32Reg : boolReg,
                   lhsIsInt32_ ? boolReg : int32Reg);
        masm.emitSet(cond, R0.scratchReg());

        // Box the result and return
        masm.tagValue(JSVAL_TYPE_BOOLEAN, R0.scratchReg(), R0);
        EmitReturnFromIC(masm);
    }

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToBool_Fallback
//

static bool
DoToBoolFallback(JSContext *cx, BaselineFrame *frame, ICToBool_Fallback *stub, HandleValue arg,
                 MutableHandleValue ret)
{
    FallbackICSpew(cx, stub, "ToBool");

    bool cond = ToBoolean(arg);
    ret.setBoolean(cond);

    // Check to see if a new stub should be generated.
    if (stub->numOptimizedStubs() >= ICToBool_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    JS_ASSERT(!arg.isBoolean());

    RawScript script = frame->script();

    // Try to generate new stubs.
    if (arg.isInt32()) {
        IonSpew(IonSpew_BaselineIC, "  Generating ToBool(Int32) stub.");
        ICToBool_Int32::Compiler compiler(cx);
        ICStub *int32Stub = compiler.getStub(compiler.getStubSpace(script));
        if (!int32Stub)
            return false;

        stub->addNewStub(int32Stub);
        return true;
    }

    if (arg.isDouble()) {
        IonSpew(IonSpew_BaselineIC, "  Generating ToBool(Double) stub.");
        ICToBool_Double::Compiler compiler(cx);
        ICStub *doubleStub = compiler.getStub(compiler.getStubSpace(script));
        if (!doubleStub)
            return false;

        stub->addNewStub(doubleStub);
        return true;
    }

    if (arg.isString()) {
        IonSpew(IonSpew_BaselineIC, "  Generating ToBool(String) stub");
        ICToBool_String::Compiler compiler(cx);
        ICStub *stringStub = compiler.getStub(compiler.getStubSpace(script));
        if (!stringStub)
            return false;

        stub->addNewStub(stringStub);
        return true;
    }

    if (arg.isNull() || arg.isUndefined()) {
        ICToBool_NullUndefined::Compiler compiler(cx);
        ICStub *nilStub = compiler.getStub(compiler.getStubSpace(script));
        if (!nilStub)
            return false;

        stub->addNewStub(nilStub);
        return true;
    }

    if (arg.isObject()) {
        IonSpew(IonSpew_BaselineIC, "  Generating ToBool(Object) stub.");
        ICToBool_Object::Compiler compiler(cx);
        ICStub *objStub = compiler.getStub(compiler.getStubSpace(script));
        if (!objStub)
            return false;

        stub->addNewStub(objStub);
        return true;
    }

    return true;
}

typedef bool (*pf)(JSContext *, BaselineFrame *, ICToBool_Fallback *, HandleValue,
                   MutableHandleValue);
static const VMFunction fun = FunctionInfo<pf>(DoToBoolFallback);

bool
ICToBool_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Push arguments.
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(fun, masm);
}

//
// ToBool_Int32
//

bool
ICToBool_Int32::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestInt32(Assembler::NotEqual, R0, &failure);

    Label ifFalse;
    Assembler::Condition cond = masm.testInt32Truthy(false, R0);
    masm.j(cond, &ifFalse);

    masm.moveValue(BooleanValue(true), R0);
    EmitReturnFromIC(masm);

    masm.bind(&ifFalse);
    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToBool_String
//

bool
ICToBool_String::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);

    Label ifFalse;
    Assembler::Condition cond = masm.testStringTruthy(false, R0);
    masm.j(cond, &ifFalse);

    masm.moveValue(BooleanValue(true), R0);
    EmitReturnFromIC(masm);

    masm.bind(&ifFalse);
    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToBool_NullUndefined
//

bool
ICToBool_NullUndefined::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure, ifFalse;
    masm.branchTestNull(Assembler::Equal, R0, &ifFalse);
    masm.branchTestUndefined(Assembler::NotEqual, R0, &failure);

    masm.bind(&ifFalse);
    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToBool_Double
//

bool
ICToBool_Double::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure, ifTrue;
    masm.branchTestDouble(Assembler::NotEqual, R0, &failure);
    masm.unboxDouble(R0, FloatReg0);
    Assembler::Condition cond = masm.testDoubleTruthy(true, FloatReg0);
    masm.j(cond, &ifTrue);

    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    masm.bind(&ifTrue);
    masm.moveValue(BooleanValue(true), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToBool_Object
//

bool
ICToBool_Object::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure, ifFalse, slowPath;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    Register objReg = masm.extractObject(R0, ExtractTemp0);
    Register scratch = R1.scratchReg();
    Assembler::Condition cond = masm.branchTestObjectTruthy(false, objReg, scratch, &slowPath);
    masm.j(cond, &ifFalse);

    // If object doesn't emulate undefined, it evaulates to true.
    masm.moveValue(BooleanValue(true), R0);
    EmitReturnFromIC(masm);

    masm.bind(&ifFalse);
    masm.moveValue(BooleanValue(false), R0);
    EmitReturnFromIC(masm);

    masm.bind(&slowPath);
    masm.setupUnalignedABICall(1, scratch);
    masm.passABIArg(objReg);
    masm.callWithABI(JS_FUNC_TO_DATA_PTR(void *, ObjectEmulatesUndefined));
    masm.xor32(Imm32(1), ReturnReg);
    masm.tagValue(JSVAL_TYPE_BOOLEAN, ReturnReg, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// ToNumber_Fallback
//

static bool
DoToNumberFallback(JSContext *cx, ICToNumber_Fallback *stub, HandleValue arg, MutableHandleValue ret)
{
    FallbackICSpew(cx, stub, "ToNumber");
    ret.set(arg);
    return ToNumber(cx, ret.address());
}

typedef bool (*DoToNumberFallbackFn)(JSContext *, ICToNumber_Fallback *, HandleValue, MutableHandleValue);
static const VMFunction DoToNumberFallbackInfo =
    FunctionInfo<DoToNumberFallbackFn>(DoToNumberFallback, PopValues(1));

bool
ICToNumber_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);

    // Push arguments.
    masm.pushValue(R0);
    masm.push(BaselineStubReg);

    return tailCallVM(DoToNumberFallbackInfo, masm);
}

//
// BinaryArith_Fallback
//

// Disable PGO (see bug 851490).
#if defined(_MSC_VER)
# pragma optimize("g", off)
#endif
static bool
DoBinaryArithFallback(JSContext *cx, BaselineFrame *frame, ICBinaryArith_Fallback *stub,
                      HandleValue lhs, HandleValue rhs, MutableHandleValue ret)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "BinaryArith(%s,%d,%d)", js_CodeName[op],
            int(lhs.isDouble() ? JSVAL_TYPE_DOUBLE : lhs.extractNonDoubleType()),
            int(rhs.isDouble() ? JSVAL_TYPE_DOUBLE : rhs.extractNonDoubleType()));

    // Don't pass lhs/rhs directly, we need the original values when
    // generating stubs.
    RootedValue lhsCopy(cx, lhs);
    RootedValue rhsCopy(cx, rhs);

    // Perform the compare operation.
    switch(op) {
      case JSOP_ADD:
        // Do an add.
        if (!AddValues(cx, script, pc, &lhsCopy, &rhsCopy, ret.address()))
            return false;
        break;
      case JSOP_SUB:
        if (!SubValues(cx, script, pc, &lhsCopy, &rhsCopy, ret.address()))
            return false;
        break;
      case JSOP_MUL:
        if (!MulValues(cx, script, pc, &lhsCopy, &rhsCopy, ret.address()))
            return false;
        break;
      case JSOP_DIV:
        if (!DivValues(cx, script, pc, &lhsCopy, &rhsCopy, ret.address()))
            return false;
        break;
      case JSOP_MOD:
        if (!ModValues(cx, script, pc, &lhsCopy, &rhsCopy, ret.address()))
            return false;
        break;
      case JSOP_BITOR: {
        int32_t result;
        if (!BitOr(cx, lhs, rhs, &result))
            return false;
        ret.setInt32(result);
        break;
      }
      case JSOP_BITXOR: {
        int32_t result;
        if (!BitXor(cx, lhs, rhs, &result))
            return false;
        ret.setInt32(result);
        break;
      }
      case JSOP_BITAND: {
        int32_t result;
        if (!BitAnd(cx, lhs, rhs, &result))
            return false;
        ret.setInt32(result);
        break;
      }
      case JSOP_LSH: {
        int32_t result;
        if (!BitLsh(cx, lhs, rhs, &result))
            return false;
        ret.setInt32(result);
        break;
      }
      case JSOP_RSH: {
        int32_t result;
        if (!BitRsh(cx, lhs, rhs, &result))
            return false;
        ret.setInt32(result);
        break;
      }
      case JSOP_URSH: {
        if (!UrshOperation(cx, script, pc, lhs, rhs, ret.address()))
            return false;
        break;
      }
      default:
        JS_NOT_REACHED("Unhandled baseline arith op");
        return false;
    }

    // Check to see if a new stub should be generated.
    if (stub->numOptimizedStubs() >= ICBinaryArith_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    // Handle string concat.
    if (op == JSOP_ADD) {
        if (lhs.isString() && rhs.isString()) {
            IonSpew(IonSpew_BaselineIC, "  Generating %s(String, String) stub", js_CodeName[op]);
            JS_ASSERT(ret.isString());
            ICBinaryArith_StringConcat::Compiler compiler(cx);
            ICStub *strcatStub = compiler.getStub(compiler.getStubSpace(script));
            if (!strcatStub)
                return false;
            stub->addNewStub(strcatStub);
            return true;
        }

        if ((lhs.isString() && rhs.isObject()) || (lhs.isObject() && rhs.isString())) {
            IonSpew(IonSpew_BaselineIC, "  Generating %s(%s, %s) stub", js_CodeName[op],
                    lhs.isString() ? "String" : "Object",
                    lhs.isString() ? "Object" : "String");
            JS_ASSERT(ret.isString());
            ICBinaryArith_StringObjectConcat::Compiler compiler(cx, lhs.isString());
            ICStub *strcatStub = compiler.getStub(compiler.getStubSpace(script));
            if (!strcatStub)
                return false;
            stub->addNewStub(strcatStub);
            return true;
        }
    }

    if (((lhs.isBoolean() && (rhs.isBoolean() || rhs.isInt32())) ||
         (rhs.isBoolean() && (lhs.isBoolean() || lhs.isInt32()))) &&
        (op == JSOP_ADD || op == JSOP_SUB || op == JSOP_BITOR || op == JSOP_BITAND ||
         op == JSOP_BITXOR))
    {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(%s, %s) stub", js_CodeName[op],
                lhs.isBoolean() ? "Boolean" : "Int32", rhs.isBoolean() ? "Boolean" : "Int32");
        ICBinaryArith_BooleanWithInt32::Compiler compiler(cx, op, lhs.isBoolean(), rhs.isBoolean());
        ICStub *arithStub = compiler.getStub(compiler.getStubSpace(script));
        if (!arithStub)
            return false;
        stub->addNewStub(arithStub);
        return true;
    }

    // Handle only int32 or double.
    if (!lhs.isNumber() || !rhs.isNumber())
        return true;

    JS_ASSERT(ret.isNumber());

    if (lhs.isDouble() || rhs.isDouble() || ret.isDouble()) {
        switch (op) {
          case JSOP_ADD:
          case JSOP_SUB:
          case JSOP_MUL:
          case JSOP_DIV:
          case JSOP_MOD: {
            // Unlink int32 stubs, it's faster to always use the double stub.
            stub->unlinkStubsWithKind(cx, ICStub::BinaryArith_Int32);
            IonSpew(IonSpew_BaselineIC, "  Generating %s(Double, Double) stub", js_CodeName[op]);

            ICBinaryArith_Double::Compiler compiler(cx, op);
            ICStub *doubleStub = compiler.getStub(compiler.getStubSpace(script));
            if (!doubleStub)
                return false;
            stub->addNewStub(doubleStub);
            return true;
          }
          default:
            break;
        }
    }

    // TODO: unlink previous !allowDouble stub.
    if (lhs.isInt32() && rhs.isInt32()) {
        bool allowDouble = ret.isDouble();
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Int32, Int32) stub", js_CodeName[op]);
        ICBinaryArith_Int32::Compiler compilerInt32(cx, op, allowDouble);
        ICStub *int32Stub = compilerInt32.getStub(compilerInt32.getStubSpace(script));
        if (!int32Stub)
            return false;
        stub->addNewStub(int32Stub);
        return true;
    }

    // Handle Double <BITOP> Int32 or Int32 <BITOP> Double case.
    if (((lhs.isDouble() && rhs.isInt32()) || (lhs.isInt32() && rhs.isDouble())) &&
        ret.isInt32())
    {
        switch(op) {
          case JSOP_BITOR:
          case JSOP_BITXOR:
          case JSOP_BITAND: {
            IonSpew(IonSpew_BaselineIC, "  Generating %s(%s, %s) stub", js_CodeName[op],
                        lhs.isDouble() ? "Double" : "Int32",
                        lhs.isDouble() ? "Int32" : "Double");
            ICBinaryArith_DoubleWithInt32::Compiler compiler(cx, op, lhs.isDouble());
            ICStub *optStub = compiler.getStub(compiler.getStubSpace(script));
            if (!optStub)
                return false;
            stub->addNewStub(optStub);
            return true;
          }
          default:
            break;
        }
    }

    return true;
}
#if defined(_MSC_VER)
# pragma optimize("g", on)
#endif

typedef bool (*DoBinaryArithFallbackFn)(JSContext *, BaselineFrame *, ICBinaryArith_Fallback *,
                                        HandleValue, HandleValue, MutableHandleValue);
static const VMFunction DoBinaryArithFallbackInfo =
    FunctionInfo<DoBinaryArithFallbackFn>(DoBinaryArithFallback, PopValues(2));

bool
ICBinaryArith_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoBinaryArithFallbackInfo, masm);
}

static bool
DoConcatStrings(JSContext *cx, HandleValue lhs, HandleValue rhs, MutableHandleValue res)
{
    JS_ASSERT(lhs.isString());
    JS_ASSERT(rhs.isString());
    RawString lstr = lhs.toString();
    RawString rstr = rhs.toString();
    RawString result = ConcatStrings<NoGC>(cx, lstr, rstr);
    if (result) {
        res.set(StringValue(result));
        return true;
    }

    RootedString rootedl(cx, lstr), rootedr(cx, rstr);
    result = ConcatStrings<CanGC>(cx, rootedl, rootedr);
    if (!result)
        return false;

    res.set(StringValue(result));
    return true;
}

typedef bool (*DoConcatStringsFn)(JSContext *, HandleValue, HandleValue, MutableHandleValue);
static const VMFunction DoConcatStringsInfo = FunctionInfo<DoConcatStringsFn>(DoConcatStrings);

bool
ICBinaryArith_StringConcat::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);
    masm.branchTestString(Assembler::NotEqual, R1, &failure);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R1);
    masm.pushValue(R0);
    if (!tailCallVM(DoConcatStringsInfo, masm))
        return false;

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

static RawString
ConvertObjectToStringForConcat(JSContext *cx, HandleValue obj)
{
    JS_ASSERT(obj.isObject());
    RootedValue rootedObj(cx, obj);
    if (!ToPrimitive(cx, &rootedObj))
        return NULL;
    return ToString<CanGC>(cx, rootedObj);
}

static bool
DoConcatStringObject(JSContext *cx, bool lhsIsString, HandleValue lhs, HandleValue rhs,
                     MutableHandleValue res)
{
    RawString lstr = NULL;
    RawString rstr = NULL;
    if (lhsIsString) {
        // Convert rhs first.
        JS_ASSERT(lhs.isString() && rhs.isObject());
        rstr = ConvertObjectToStringForConcat(cx, rhs);
        if (!rstr)
            return false;

        // lhs is already string.
        lstr = lhs.toString();
    } else {
        JS_ASSERT(rhs.isString() && lhs.isObject());
        // Convert lhs first.
        lstr = ConvertObjectToStringForConcat(cx, lhs);
        if (!lstr)
            return false;

        // rhs is already string.
        rstr = rhs.toString();
    }

    JSString *str = ConcatStrings<NoGC>(cx, lstr, rstr);
    if (!str) {
        RootedString nlstr(cx, lstr), nrstr(cx, rstr);
        str = ConcatStrings<CanGC>(cx, nlstr, nrstr);
        if (!str)
            return false;
    }

    // Technically, we need to call TypeScript::MonitorString for this PC, however
    // it was called when this stub was attached so it's OK.

    res.setString(str);
    return true;
}

typedef bool (*DoConcatStringObjectFn)(JSContext *, bool lhsIsString, HandleValue, HandleValue,
                                       MutableHandleValue);
static const VMFunction DoConcatStringObjectInfo =
    FunctionInfo<DoConcatStringObjectFn>(DoConcatStringObject, PopValues(2));

bool
ICBinaryArith_StringObjectConcat::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    if (lhsIsString_) {
        masm.branchTestString(Assembler::NotEqual, R0, &failure);
        masm.branchTestObject(Assembler::NotEqual, R1, &failure);
    } else {
        masm.branchTestObject(Assembler::NotEqual, R0, &failure);
        masm.branchTestString(Assembler::NotEqual, R1, &failure);
    }

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Sync for the decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(Imm32(lhsIsString_));
    if (!tailCallVM(DoConcatStringObjectInfo, masm))
        return false;

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICBinaryArith_Double::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.ensureDouble(R0, FloatReg0, &failure);
    masm.ensureDouble(R1, FloatReg1, &failure);

    switch (op) {
      case JSOP_ADD:
        masm.addDouble(FloatReg1, FloatReg0);
        break;
      case JSOP_SUB:
        masm.subDouble(FloatReg1, FloatReg0);
        break;
      case JSOP_MUL:
        masm.mulDouble(FloatReg1, FloatReg0);
        break;
      case JSOP_DIV:
        masm.divDouble(FloatReg1, FloatReg0);
        break;
      case JSOP_MOD:
        masm.setupUnalignedABICall(2, R0.scratchReg());
        masm.passABIArg(FloatReg0);
        masm.passABIArg(FloatReg1);
        masm.callWithABI(JS_FUNC_TO_DATA_PTR(void *, NumberMod), MacroAssembler::DOUBLE);
        JS_ASSERT(ReturnFloatReg == FloatReg0);
        break;
      default:
        JS_NOT_REACHED("Unexpected op");
        return false;
    }

    masm.boxDouble(FloatReg0, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICBinaryArith_BooleanWithInt32::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    if (lhsIsBool_)
        masm.branchTestBoolean(Assembler::NotEqual, R0, &failure);
    else
        masm.branchTestInt32(Assembler::NotEqual, R0, &failure);

    if (rhsIsBool_)
        masm.branchTestBoolean(Assembler::NotEqual, R1, &failure);
    else
        masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    Register lhsReg = lhsIsBool_ ? masm.extractBoolean(R0, ExtractTemp0)
                                 : masm.extractInt32(R0, ExtractTemp0);
    Register rhsReg = rhsIsBool_ ? masm.extractBoolean(R1, ExtractTemp1)
                                 : masm.extractInt32(R1, ExtractTemp1);

    JS_ASSERT(op_ == JSOP_ADD || op_ == JSOP_SUB ||
              op_ == JSOP_BITOR || op_ == JSOP_BITXOR || op_ == JSOP_BITAND);

    switch(op_) {
      case JSOP_ADD: {
        Label fixOverflow;

        masm.add32(rhsReg, lhsReg);
        masm.j(Assembler::Overflow, &fixOverflow);
        masm.tagValue(JSVAL_TYPE_INT32, lhsReg, R0);
        EmitReturnFromIC(masm);

        masm.bind(&fixOverflow);
        masm.sub32(rhsReg, lhsReg);
        masm.jump(&failure);
        break;
      }
      case JSOP_SUB: {
        Label fixOverflow;

        masm.sub32(rhsReg, lhsReg);
        masm.j(Assembler::Overflow, &fixOverflow);
        masm.tagValue(JSVAL_TYPE_INT32, lhsReg, R0);
        EmitReturnFromIC(masm);

        masm.bind(&fixOverflow);
        masm.add32(rhsReg, lhsReg);
        masm.jump(&failure);
        break;
      }
      case JSOP_BITOR: {
        masm.orPtr(rhsReg, lhsReg);
        masm.tagValue(JSVAL_TYPE_INT32, lhsReg, R0);
        EmitReturnFromIC(masm);
        break;
      }
      case JSOP_BITXOR: {
        masm.xorPtr(rhsReg, lhsReg);
        masm.tagValue(JSVAL_TYPE_INT32, lhsReg, R0);
        EmitReturnFromIC(masm);
        break;
      }
      case JSOP_BITAND: {
        masm.andPtr(rhsReg, lhsReg);
        masm.tagValue(JSVAL_TYPE_INT32, lhsReg, R0);
        EmitReturnFromIC(masm);
        break;
      }
      default:
       JS_NOT_REACHED("Unhandled op for BinaryArith_BooleanWithInt32.");
       return false;
    }

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICBinaryArith_DoubleWithInt32::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(op == JSOP_BITOR || op == JSOP_BITAND || op == JSOP_BITXOR);

    Label failure;
    Register intReg;
    Register scratchReg;
    if (lhsIsDouble_) {
        masm.branchTestDouble(Assembler::NotEqual, R0, &failure);
        masm.branchTestInt32(Assembler::NotEqual, R1, &failure);
        intReg = masm.extractInt32(R1, ExtractTemp0);
        masm.unboxDouble(R0, FloatReg0);
        scratchReg = R0.scratchReg();
    } else {
        masm.branchTestInt32(Assembler::NotEqual, R0, &failure);
        masm.branchTestDouble(Assembler::NotEqual, R1, &failure);
        intReg = masm.extractInt32(R0, ExtractTemp0);
        masm.unboxDouble(R1, FloatReg0);
        scratchReg = R1.scratchReg();
    }

    // Truncate the double to an int32.
    {
        Label doneTruncate;
        Label truncateABICall;
        masm.branchTruncateDouble(FloatReg0, scratchReg, &truncateABICall);
        masm.jump(&doneTruncate);

        masm.bind(&truncateABICall);
        masm.push(intReg);
        masm.setupUnalignedABICall(1, scratchReg);
        masm.passABIArg(FloatReg0);
        masm.callWithABI(JS_FUNC_TO_DATA_PTR(void *, js::ToInt32));
        masm.storeCallResult(scratchReg);
        masm.pop(intReg);

        masm.bind(&doneTruncate);
    }

    Register intReg2 = scratchReg;
    // All handled ops commute, so no need to worry about ordering.
    switch(op) {
      case JSOP_BITOR:
        masm.orPtr(intReg, intReg2);
        break;
      case JSOP_BITXOR:
        masm.xorPtr(intReg, intReg2);
        break;
      case JSOP_BITAND:
        masm.andPtr(intReg, intReg2);
        break;
      default:
       JS_NOT_REACHED("Unhandled op for BinaryArith_DoubleWithInt32.");
       return false;
    }
    masm.tagValue(JSVAL_TYPE_INT32, intReg2, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// UnaryArith_Fallback
//

// Disable PGO (see bug 851490).
#if defined(_MSC_VER)
# pragma optimize("g", off)
#endif
static bool
DoUnaryArithFallback(JSContext *cx, BaselineFrame *frame, ICUnaryArith_Fallback *stub,
                     HandleValue val, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "UnaryArith(%s)", js_CodeName[op]);

    switch (op) {
      case JSOP_BITNOT: {
        int32_t result;
        if (!BitNot(cx, val, &result))
            return false;
        res.setInt32(result);
        break;
      }
      case JSOP_NEG:
        if (!NegOperation(cx, script, pc, val, res))
            return false;
        break;
      default:
        JS_NOT_REACHED("Unexpected op");
        return false;
    }

    if (stub->numOptimizedStubs() >= ICUnaryArith_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard/replace stubs.
        return true;
    }

    if (val.isInt32() && res.isInt32()) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Int32 => Int32) stub", js_CodeName[op]);
        ICUnaryArith_Int32::Compiler compiler(cx, op);
        ICStub *int32Stub = compiler.getStub(compiler.getStubSpace(script));
        if (!int32Stub)
            return false;
        stub->addNewStub(int32Stub);
        return true;
    }

    if (val.isNumber() && res.isNumber() && op == JSOP_NEG) {
        IonSpew(IonSpew_BaselineIC, "  Generating %s(Number => Number) stub", js_CodeName[op]);
        // Unlink int32 stubs, the double stub handles both cases and TI specializes for both.
        stub->unlinkStubsWithKind(cx, ICStub::UnaryArith_Int32);

        ICUnaryArith_Double::Compiler compiler(cx, op);
        ICStub *doubleStub = compiler.getStub(compiler.getStubSpace(script));
        if (!doubleStub)
            return false;
        stub->addNewStub(doubleStub);
        return true;
    }

    return true;
}
#if defined(_MSC_VER)
# pragma optimize("g", on)
#endif

typedef bool (*DoUnaryArithFallbackFn)(JSContext *, BaselineFrame *, ICUnaryArith_Fallback *,
                                       HandleValue, MutableHandleValue);
static const VMFunction DoUnaryArithFallbackInfo =
    FunctionInfo<DoUnaryArithFallbackFn>(DoUnaryArithFallback, PopValues(1));

bool
ICUnaryArith_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);

    // Push arguments.
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoUnaryArithFallbackInfo, masm);
}

bool
ICUnaryArith_Double::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.ensureDouble(R0, FloatReg0, &failure);

    JS_ASSERT(op == JSOP_NEG);
    masm.negateDouble(FloatReg0);
    masm.boxDouble(FloatReg0, R0);

    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// GetElem_Fallback
//

static void GetFixedOrDynamicSlotOffset(HandleObject obj, uint32_t slot,
                                        bool *isFixed, uint32_t *offset)
{
    JS_ASSERT(isFixed);
    JS_ASSERT(offset);
    *isFixed = obj->isFixedSlot(slot);
    *offset = *isFixed ? JSObject::getFixedSlotOffset(slot)
                       : obj->dynamicSlotIndex(slot) * sizeof(Value);
}

// Look up a property's shape on an object, being careful never to do any effectful
// operations.  This procedure not yielding a shape should not be taken as a lack of
// existence of the property on the object.
static bool
EffectlesslyLookupProperty(JSContext *cx, HandleObject obj, HandlePropertyName name,
                           MutableHandleObject holder, MutableHandleShape shape)
{
    if (obj->hasIdempotentProtoChain()) {
        if (!JSObject::lookupProperty(cx, obj, name, holder, shape))
            return false;
    } else if (obj->isNative()) {
        shape.set(obj->nativeLookup(cx, NameToId(name)));
        if (shape)
            holder.set(obj);
    } else {
        shape.set(NULL);
        holder.set(NULL);
    }
    return true;
}

static bool
IsCacheableProtoChain(JSObject *obj, JSObject *holder)
{
    JS_ASSERT(obj->isNative());

    // Don't handle objects which require a prototype guard. This should
    // be uncommon so handling it is likely not worth the complexity.
    if (obj->hasUncacheableProto())
        return false;

    while (obj != holder) {
        // We cannot assume that we find the holder object on the prototype
        // chain and must check for null proto. The prototype chain can be
        // altered during the lookupProperty call.
        JSObject *proto = obj->getProto();
        if (!proto || !proto->isNative())
            return false;

        if (proto->hasUncacheableProto())
            return false;

        obj = proto;
    }
    return true;
}

static bool
IsCacheableGetPropReadSlot(JSObject *obj, JSObject *holder, Shape *shape)
{
    if (!shape || !IsCacheableProtoChain(obj, holder))
        return false;

    if (!shape->hasSlot() || !shape->hasDefaultGetter())
        return false;

    return true;
}

static bool
IsCacheableGetPropCallScripted(JSObject *obj, JSObject *holder, Shape *shape)
{
    // Currently we only optimize getter calls for getters bound on prototypes.
    if (obj == holder)
        return false;

    if (!shape || !IsCacheableProtoChain(obj, holder))
        return false;

    if (shape->hasSlot() || shape->hasDefaultGetter())
        return false;

    if (!shape->hasGetterValue() || !shape->getterObject()->isFunction())
        return false;

    JSFunction *func = shape->getterObject()->toFunction();
    if (!func->hasScript())
        return false;

    JSScript *script = func->nonLazyScript();
    return script->hasIonScript() || script->hasBaselineScript();
}

static bool
IsCacheableSetPropWriteSlot(JSObject *obj, Shape *oldShape, JSObject *holder, Shape *shape)
{
    if (!shape)
        return false;

    // Object shape must not have changed during the property set.
    if (obj->lastProperty() != oldShape)
        return false;

    // Currently we only optimize direct writes.
    if (obj != holder)
        return false;

    if (!shape->hasSlot() || !shape->hasDefaultSetter() || !shape->writable())
        return false;

    return true;
}

static bool
IsCacheableSetPropAddSlot(JSContext *cx, HandleObject obj, HandleShape oldShape, uint32_t oldSlots,
                          HandleId id, HandleObject holder, HandleShape shape,
                          size_t *protoChainDepth)
{
    if (!shape)
        return false;

    // Property must be set directly on object, and be last added property of object.
    if (obj != holder || shape != obj->lastProperty())
        return false;

    // Object must be extensible, oldShape must be immediate parent of curShape.
    if (!obj->isExtensible() || obj->lastProperty()->previous() != oldShape)
        return false;

    // Basic shape checks.
    if (shape->inDictionary() || !shape->hasSlot() || !shape->hasDefaultSetter() ||
        !shape->writable())
    {
        return false;
    }

    // If object has a non-default resolve hook, don't inline
    if (obj->getClass()->resolve != JS_ResolveStub)
        return false;

    size_t chainDepth = 0;
    // walk up the object prototype chain and ensure that all prototypes
    // are native, and that all prototypes have setter defined on the property
    for (JSObject *proto = obj->getProto(); proto; proto = proto->getProto()) {
        chainDepth++;
        // if prototype is non-native, don't optimize
        if (!proto->isNative())
            return false;

        // if prototype defines this property in a non-plain way, don't optimize
        Shape *protoShape = proto->nativeLookup(cx, id);
        if (protoShape && !protoShape->hasDefaultSetter())
            return false;

        // Otherise, if there's no such property, watch out for a resolve hook that would need
        // to be invoked and thus prevent inlining of property addition.
        if (proto->getClass()->resolve != JS_ResolveStub)
             return false;
    }

    // Only add a IC entry if the dynamic slots didn't change when the shapes
    // changed.  Need to ensure that a shape change for a subsequent object
    // won't involve reallocating the slot array.
    if (obj->numDynamicSlots() != oldSlots)
        return false;

    *protoChainDepth = chainDepth;
    return true;
}

static bool
IsCacheableSetPropCallScripted(JSObject *obj, JSObject *holder, Shape *shape)
{
    // Currently we only optimize setter calls for setters bound on prototypes.
    if (obj == holder)
        return false;

    if (!shape || !IsCacheableProtoChain(obj, holder))
        return false;

    if (shape->hasSlot() || shape->hasDefaultSetter())
        return false;

    if (!shape->hasSetterValue() || !shape->setterObject()->isFunction())
        return false;

    JSFunction *func = shape->setterObject()->toFunction();
    if (!func->hasScript())
        return false;

    JSScript *script = func->nonLazyScript();
    return script->hasIonScript() || script->hasBaselineScript();
}

static bool
TypedArrayGetElemStubExists(ICGetElem_Fallback *stub, HandleObject obj)
{
    for (ICStubConstIterator iter = stub->beginChainConst(); !iter.atEnd(); iter++) {
        if (!iter->isGetElem_TypedArray())
            continue;
        if (obj->lastProperty() == iter->toGetElem_TypedArray()->shape())
            return true;
    }
    return false;
}


static bool TryAttachNativeGetElemStub(JSContext *cx, HandleScript script,
                                       ICGetElem_Fallback *stub, HandleObject obj,
                                       HandleValue key)
{
    RootedId id(cx);
    RootedValue idval(cx);
    if (!FetchElementId(cx, obj, key, &id, &idval))
        return false;

    uint32_t dummy;
    if (!JSID_IS_ATOM(id) || JSID_TO_ATOM(id)->isIndex(&dummy))
        return true;

    RootedPropertyName propName(cx, JSID_TO_ATOM(id)->asPropertyName());

    RootedShape shape(cx);
    RootedObject holder(cx);
    if (!EffectlesslyLookupProperty(cx, obj, propName, &holder, &shape))
        return false;

    if (!IsCacheableGetPropReadSlot(obj, holder, shape))
        return true;

    bool isFixedSlot;
    uint32_t offset;
    GetFixedOrDynamicSlotOffset(holder, shape->slot(), &isFixedSlot, &offset);

    ICStub *monitorStub = stub->fallbackMonitorStub()->firstMonitorStub();
    ICStub::Kind kind = (obj == holder) ? ICStub::GetElem_Native : ICStub::GetElem_NativePrototype;

    IonSpew(IonSpew_BaselineIC, "  Generating GetElem(Native %s) stub (obj=%p, shape=%p, holder=%p, holderShape=%p)",
                (obj == holder) ? "direct" : "prototype",
                obj.get(), obj->lastProperty(), holder.get(), holder->lastProperty());

    ICGetElemNativeCompiler compiler(cx, kind, monitorStub, obj, holder, key,
                                     isFixedSlot, offset);
    ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
    if (!newStub)
        return false;

    stub->addNewStub(newStub);
    return true;
}

static bool
TryAttachGetElemStub(JSContext *cx, HandleScript script, ICGetElem_Fallback *stub,
                     HandleValue lhs, HandleValue rhs, HandleValue res)
{
    // Check for String[i] => Char accesses.
    if (lhs.isString() && rhs.isInt32() && res.isString() &&
        !stub->hasStub(ICStub::GetElem_String))
    {
        IonSpew(IonSpew_BaselineIC, "  Generating GetElem(String[Int32]) stub");
        ICGetElem_String::Compiler compiler(cx);
        ICStub *stringStub = compiler.getStub(compiler.getStubSpace(script));
        if (!stringStub)
            return false;

        stub->addNewStub(stringStub);
        return true;
    }

    // Otherwise, GetElem is only optimized on objects.
    if (!lhs.isObject())
        return true;
    RootedObject obj(cx, &lhs.toObject());

    if (obj->isNative()) {
        // Check for NativeObject[int] dense accesses.
        if (rhs.isInt32()) {
            IonSpew(IonSpew_BaselineIC, "  Generating GetElem(Native[Int32] dense) stub");
            ICGetElem_Dense::Compiler compiler(cx, stub->fallbackMonitorStub()->firstMonitorStub(),
                                               obj->lastProperty());
            ICStub *denseStub = compiler.getStub(compiler.getStubSpace(script));
            if (!denseStub)
                return false;

            stub->addNewStub(denseStub);
            return true;
        }

        // Check for NativeObject[id] shape-optimizable accesses.
        if (rhs.isString()) {
            if (!TryAttachNativeGetElemStub(cx, script, stub, obj, rhs))
                return false;
        }
    }

    // Check for TypedArray[int] => Number accesses.
    if (obj->isTypedArray() && rhs.isInt32() && res.isNumber() &&
        !TypedArrayGetElemStubExists(stub, obj))
    {
        IonSpew(IonSpew_BaselineIC, "  Generating GetElem(TypedArray[Int32]) stub");
        ICGetElem_TypedArray::Compiler compiler(cx, obj->lastProperty(), TypedArray::type(obj));
        ICStub *typedArrayStub = compiler.getStub(compiler.getStubSpace(script));
        if (!typedArrayStub)
            return false;

        stub->addNewStub(typedArrayStub);
        return true;
    }

    return true;
}

static bool
DoGetElemFallback(JSContext *cx, BaselineFrame *frame, ICGetElem_Fallback *stub, HandleValue lhs,
                  HandleValue rhs, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "GetElem(%s)", js_CodeName[op]);

    JS_ASSERT(op == JSOP_GETELEM || op == JSOP_CALLELEM);

    // Don't pass lhs directly, we need it when generating stubs.
    RootedValue lhsCopy(cx, lhs);

    bool isOptimizedArgs = false;
    if (lhs.isMagic(JS_OPTIMIZED_ARGUMENTS)) {
        // Handle optimized arguments[i] access.
        if (!GetElemOptimizedArguments(cx, frame, &lhsCopy, rhs, res, &isOptimizedArgs))
            return false;
        if (isOptimizedArgs)
            types::TypeScript::Monitor(cx, script, pc, res);
    }

    if (!isOptimizedArgs) {
        if (!GetElementOperation(cx, op, &lhsCopy, rhs, res))
            return false;
        types::TypeScript::Monitor(cx, script, pc, res);
    }

    // Add a type monitor stub for the resulting value.
    if (!stub->addMonitorStubForValue(cx, script, res))
        return false;

    if (stub->numOptimizedStubs() >= ICGetElem_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    // Try to attach an optimized stub.
    if (!TryAttachGetElemStub(cx, script, stub, lhs, rhs, res))
        return false;

    return true;
}

typedef bool (*DoGetElemFallbackFn)(JSContext *, BaselineFrame *, ICGetElem_Fallback *,
                                    HandleValue, HandleValue, MutableHandleValue);
static const VMFunction DoGetElemFallbackInfo =
    FunctionInfo<DoGetElemFallbackFn>(DoGetElemFallback, PopValues(2));

bool
ICGetElem_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Restore the tail call register.
    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoGetElemFallbackInfo, masm);
}

//
// GetElem_Native
//

bool
ICGetElemNativeCompiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    Label failurePopR1;
    bool popR1 = false;

    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    Address idValAddr(BaselineStubReg, ICGetElemNativeStub::offsetOfIdval());
    masm.branchTestValue(Assembler::NotEqual, idValAddr, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox object.
    Register objReg = masm.extractObject(R0, ExtractTemp0);

    // Check object shape.
    masm.loadPtr(Address(objReg, JSObject::offsetOfShape()), scratchReg);
    Address shapeAddr(BaselineStubReg, ICGetElemNativeStub::offsetOfShape());
    masm.branchPtr(Assembler::NotEqual, shapeAddr, scratchReg, &failure);

    Register holderReg;
    if (obj_ == holder_) {
        holderReg = objReg;
    } else {
        // Shape guard holder.
        if (regs.empty()) {
            masm.push(R1.scratchReg());
            popR1 = true;
            holderReg = R1.scratchReg();
        } else {
            holderReg = regs.takeAny();
        }
        masm.loadPtr(Address(BaselineStubReg, ICGetElem_NativePrototype::offsetOfHolder()),
                     holderReg);
        masm.loadPtr(Address(BaselineStubReg, ICGetElem_NativePrototype::offsetOfHolderShape()),
                     scratchReg);
        masm.branchTestObjShape(Assembler::NotEqual, holderReg, scratchReg,
                                popR1 ? &failurePopR1 : &failure);
    }

    // Load from object.
    if (!isFixedSlot_)
        masm.loadPtr(Address(holderReg, JSObject::offsetOfSlots()), holderReg);

    masm.load32(Address(BaselineStubReg, ICGetElem_Native::offsetOfOffset()), scratchReg);
    masm.loadValue(BaseIndex(holderReg, scratchReg, TimesOne), R0);

    if (popR1)
        masm.pop(R1.scratchReg());
    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    if (popR1) {
        masm.bind(&failurePopR1);
        masm.pop(R1.scratchReg());
    }
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// GetElem_String
//

bool
ICGetElem_String::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox string in R0.
    Register str = masm.extractString(R0, ExtractTemp0);

    // Load string lengthAndFlags
    Address lengthAndFlagsAddr(str, JSString::offsetOfLengthAndFlags());
    masm.loadPtr(lengthAndFlagsAddr, scratchReg);

    // Check for non-linear strings.
    masm.branchTest32(Assembler::Zero, scratchReg, Imm32(JSString::FLAGS_MASK), &failure);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Extract length and bounds check.
    masm.rshiftPtr(Imm32(JSString::LENGTH_SHIFT), scratchReg);
    masm.branch32(Assembler::BelowOrEqual, scratchReg, key, &failure);

    // Get char code.
    Address charsAddr(str, JSString::offsetOfChars());
    masm.loadPtr(charsAddr, scratchReg);
    masm.load16ZeroExtend(BaseIndex(scratchReg, key, TimesTwo, 0), scratchReg);

    // Check if char code >= UNIT_STATIC_LIMIT.
    masm.branch32(Assembler::AboveOrEqual, scratchReg, Imm32(StaticStrings::UNIT_STATIC_LIMIT),
                  &failure);

    // Load static string.
    masm.movePtr(ImmWord(&cx->compartment->rt->staticStrings.unitStaticTable), str);
    masm.loadPtr(BaseIndex(str, scratchReg, ScalePointer), str);

    // Return.
    masm.tagValue(JSVAL_TYPE_STRING, str, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// GetElem_Dense
//

bool
ICGetElem_Dense::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox R0 and shape guard.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICGetElem_Dense::offsetOfShape()), scratchReg);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratchReg, &failure);

    // Load obj->elements.
    masm.loadPtr(Address(obj, JSObject::offsetOfElements()), scratchReg);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Bounds check.
    Address initLength(scratchReg, ObjectElements::offsetOfInitializedLength());
    masm.branch32(Assembler::BelowOrEqual, initLength, key, &failure);

    // Hole check and load value.
    BaseIndex element(scratchReg, key, TimesEight);
    masm.branchTestMagic(Assembler::Equal, element, &failure);
    masm.loadValue(element, R0);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// GetElem_TypedArray
//

bool
ICGetElem_TypedArray::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox R0 and shape guard.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICGetElem_TypedArray::offsetOfShape()), scratchReg);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratchReg, &failure);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Bounds check.
    masm.unboxInt32(Address(obj, TypedArray::lengthOffset()), scratchReg);
    masm.branch32(Assembler::BelowOrEqual, scratchReg, key, &failure);

    // Load the elements vector.
    masm.loadPtr(Address(obj, TypedArray::dataOffset()), scratchReg);

    // Load the value.
    BaseIndex source(scratchReg, key, ScaleFromElemWidth(TypedArray::slotWidth(type_)));
    masm.loadFromTypedArray(type_, source, R0, false, scratchReg, &failure);

    // Todo: Allow loading doubles from uint32 arrays, but this requires monitoring.
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// SetElem_Fallback
//

static bool
DenseSetElemStubExists(JSContext *cx, ICStub::Kind kind, ICSetElem_Fallback *stub, HandleObject obj)
{
    JS_ASSERT(kind == ICStub::SetElem_Dense || kind == ICStub::SetElem_DenseAdd);

    for (ICStubConstIterator iter = stub->beginChainConst(); !iter.atEnd(); iter++) {
        if (kind == ICStub::SetElem_Dense && iter->isSetElem_Dense()) {
            ICSetElem_Dense *dense = iter->toSetElem_Dense();
            if (obj->lastProperty() == dense->shape() && obj->getType(cx) == dense->type())
                return true;
        }

        if (kind == ICStub::SetElem_DenseAdd && iter->isSetElem_DenseAdd()) {
            ICSetElem_DenseAdd *dense = iter->toSetElem_DenseAdd();
            if (obj->lastProperty() == dense->toImplUnchecked<0>()->shape(0) &&
                obj->getType(cx) == dense->type())
            {
                return true;
            }
        }
    }
    return false;
}

static bool
TypedArraySetElemStubExists(ICSetElem_Fallback *stub, HandleObject obj, bool expectOOB)
{
    for (ICStubConstIterator iter = stub->beginChainConst(); !iter.atEnd(); iter++) {
        if (!iter->isSetElem_TypedArray())
            continue;
        ICSetElem_TypedArray *taStub = iter->toSetElem_TypedArray();
        if (obj->lastProperty() == taStub->shape() && taStub->expectOutOfBounds() == expectOOB)
            return true;
    }
    return false;
}

static bool
RemoveExistingTypedArraySetElemStub(JSContext *cx, ICSetElem_Fallback *stub, HandleObject obj)
{
    for (ICStubIterator iter = stub->beginChain(); !iter.atEnd(); iter++) {
        if (!iter->isSetElem_TypedArray())
            continue;

        if (obj->lastProperty() != iter->toSetElem_TypedArray()->shape())
            continue;

        // TypedArraySetElem stubs are only removed using this procedure if
        // being replaced with one that expects out of bounds index.
        JS_ASSERT(!iter->toSetElem_TypedArray()->expectOutOfBounds());
        iter.unlink(cx->zone());
        return true;
    }
    return false;
}

static bool
CanOptimizeDenseSetElem(JSContext *cx, HandleObject obj, uint32_t index,
                        HandleShape oldShape, uint32_t oldCapacity, uint32_t oldInitLength,
                        bool *isAddingCaseOut, size_t *protoDepthOut)
{
    uint32_t initLength = obj->getDenseInitializedLength();
    uint32_t capacity = obj->getDenseCapacity();

    *isAddingCaseOut = false;
    *protoDepthOut = 0;

    // Some initial sanity checks.
    if (initLength < oldInitLength || capacity < oldCapacity)
        return false;

    RootedShape shape(cx, obj->lastProperty());

    // Cannot optimize if the shape changed.
    if (oldShape != shape)
        return false;

    // Cannot optimize if the capacity changed.
    if (oldCapacity != capacity)
        return false;

    // Cannot optimize if the index doesn't fit within the new initialized length.
    if (index >= initLength)
        return false;

    // Cannot optimize if the value at position after the set is a hole.
    if (!obj->containsDenseElement(index))
        return false;

    // At this point, if we know that the initLength did not change, then
    // an optimized set is possible.
    if (oldInitLength == initLength)
        return true;

    // If it did change, ensure that it changed specifically by incrementing by 1
    // to accomodate this particular indexed set.
    if (oldInitLength + 1 != initLength)
        return false;
    if (index != oldInitLength)
        return false;

    // The checks are not complete.  The object may have a setter definition,
    // either directly, or via a prototype, or via the target object for a prototype
    // which is a proxy, that handles a particular integer write.
    // Scan the prototype and shape chain to make sure that this is not the case.
    RootedObject curObj(cx, obj);
    while (curObj) {
        // Ensure object is native.
        if (!curObj->isNative())
            return false;

        // Ensure all indexed properties are stored in dense elements.
        if (curObj->isIndexed())
            return false;

        curObj = curObj->getProto();
        if (curObj)
            ++*protoDepthOut;
    }

    if (*protoDepthOut > ICSetElem_DenseAdd::MAX_PROTO_CHAIN_DEPTH)
        return false;

    *isAddingCaseOut = true;

    return true;
}

static bool
DoSetElemFallback(JSContext *cx, BaselineFrame *frame, ICSetElem_Fallback *stub, Value *stack,
                  HandleValue objv, HandleValue index, HandleValue rhs)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "SetElem(%s)", js_CodeName[JSOp(*pc)]);

    JS_ASSERT(op == JSOP_SETELEM ||
              op == JSOP_ENUMELEM ||
              op == JSOP_INITELEM ||
              op == JSOP_INITELEM_ARRAY);

    RootedObject obj(cx, ToObject(cx, objv));
    if (!obj)
        return false;

    RootedShape oldShape(cx, obj->lastProperty());

    // Check the old capacity
    uint32_t oldCapacity = 0;
    uint32_t oldInitLength = 0;
    if (obj->isNative() && index.isInt32() && index.toInt32() >= 0) {
        oldCapacity = obj->getDenseCapacity();
        oldInitLength = obj->getDenseInitializedLength();
    }

    if (op == JSOP_INITELEM) {
        RootedValue nindex(cx, index);
        if (!InitElemOperation(cx, obj, &nindex, rhs))
            return false;
    } else if (op == JSOP_INITELEM_ARRAY) {
        JS_ASSERT(uint32_t(index.toInt32()) == GET_UINT24(pc));
        if (!InitArrayElemOperation(cx, pc, obj, index.toInt32(), rhs))
            return false;
    } else {
        if (!SetObjectElement(cx, obj, index, rhs, script->strict, script, pc))
            return false;
    }

    // Overwrite the object on the stack (pushed for the decompiler) with the rhs.
    JS_ASSERT(stack[2] == objv);
    stack[2] = rhs;

    if (stub->numOptimizedStubs() >= ICSetElem_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    // Try to generate new stubs.
    if (obj->isNative() &&
        index.isInt32() && index.toInt32() >= 0 &&
        !rhs.isMagic(JS_ELEMENTS_HOLE))
    {
        JS_ASSERT(!obj->isTypedArray());

        bool addingCase;
        size_t protoDepth;

        if (CanOptimizeDenseSetElem(cx, obj, index.toInt32(), oldShape, oldCapacity, oldInitLength,
                                    &addingCase, &protoDepth))
        {
            RootedTypeObject type(cx, obj->getType(cx));
            RootedShape shape(cx, obj->lastProperty());

            if (addingCase && !DenseSetElemStubExists(cx, ICStub::SetElem_DenseAdd, stub, obj)) {
                IonSpew(IonSpew_BaselineIC,
                        "  Generating SetElem_DenseAdd stub "
                        "(shape=%p, type=%p, protoDepth=%u)",
                        obj->lastProperty(), type.get(), protoDepth);
                ICSetElemDenseAddCompiler compiler(cx, obj, protoDepth);
                ICUpdatedStub *denseStub = compiler.getStub(compiler.getStubSpace(script));
                if (!denseStub)
                    return false;
                if (!denseStub->addUpdateStubForValue(cx, script, obj, JSID_VOID, rhs))
                    return false;

                stub->addNewStub(denseStub);
            } else if (!addingCase &&
                       !DenseSetElemStubExists(cx, ICStub::SetElem_Dense, stub, obj))
            {
                IonSpew(IonSpew_BaselineIC,
                        "  Generating SetElem_Dense stub (shape=%p, type=%p)",
                        obj->lastProperty(), type.get());
                ICSetElem_Dense::Compiler compiler(cx, shape, type);
                ICUpdatedStub *denseStub = compiler.getStub(compiler.getStubSpace(script));
                if (!denseStub)
                    return false;
                if (!denseStub->addUpdateStubForValue(cx, script, obj, JSID_VOID, rhs))
                    return false;

                stub->addNewStub(denseStub);
            }
        }

        return true;
    }

    if (obj->isTypedArray() && index.isInt32() && rhs.isNumber()) {
        uint32_t len = TypedArray::length(obj);
        int32_t idx = index.toInt32();
        bool expectOutOfBounds = (idx < 0) || (static_cast<uint32_t>(idx) >= len);

        if (!TypedArraySetElemStubExists(stub, obj, expectOutOfBounds)) {
            // Remove any existing TypedArraySetElemStub that doesn't handle out-of-bounds
            if (expectOutOfBounds)
                RemoveExistingTypedArraySetElemStub(cx, stub, obj);

            IonSpew(IonSpew_BaselineIC,
                    "  Generating SetElem_TypedArray stub (shape=%p, type=%u, oob=%s)",
                    obj->lastProperty(), TypedArray::type(obj),
                    expectOutOfBounds ? "yes" : "no");
            ICSetElem_TypedArray::Compiler compiler(cx, obj->lastProperty(), TypedArray::type(obj),
                                                    expectOutOfBounds);
            ICStub *typedArrayStub = compiler.getStub(compiler.getStubSpace(script));
            if (!typedArrayStub)
                return false;

            stub->addNewStub(typedArrayStub);
            return true;
        }
    }

    return true;
}

typedef bool (*DoSetElemFallbackFn)(JSContext *, BaselineFrame *, ICSetElem_Fallback *, Value *,
                                    HandleValue, HandleValue, HandleValue);
static const VMFunction DoSetElemFallbackInfo =
    FunctionInfo<DoSetElemFallbackFn>(DoSetElemFallback, PopValues(2));

bool
ICSetElem_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    EmitRestoreTailCallReg(masm);

    // State: R0: object, R1: index, stack: rhs.
    // For the decompiler, the stack has to be: object, index, rhs,
    // so we push the index, then overwrite the rhs Value with R0
    // and push the rhs value.
    masm.pushValue(R1);
    masm.loadValue(Address(BaselineStackReg, sizeof(Value)), R1);
    masm.storeValue(R0, Address(BaselineStackReg, sizeof(Value)));
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1); // RHS

    // Push index. On x86 and ARM two push instructions are emitted so use a
    // separate register to store the old stack pointer.
    masm.mov(BaselineStackReg, R1.scratchReg());
    masm.pushValue(Address(R1.scratchReg(), 2 * sizeof(Value)));
    masm.pushValue(R0); // Object.

    // Push pointer to stack values, so that the stub can overwrite the object
    // (pushed for the decompiler) with the rhs.
    masm.computeEffectiveAddress(Address(BaselineStackReg, 3 * sizeof(Value)), R0.scratchReg());
    masm.push(R0.scratchReg());

    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoSetElemFallbackInfo, masm);
}

//
// SetElem_Dense
//

bool
ICSetElem_Dense::Compiler::generateStubCode(MacroAssembler &masm)
{
    // R0 = object
    // R1 = key
    // Stack = { ... rhs-value, <return-addr>? }
    Label failure;
    Label failureUnstow;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox R0 and guard on its shape.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetElem_Dense::offsetOfShape()), scratchReg);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratchReg, &failure);

    // Stow both R0 and R1 (object and key)
    // But R0 and R1 still hold their values.
    EmitStowICValues(masm, 2);

    // We may need to free up some registers.
    regs = availableGeneralRegs(0);
    regs.take(R0);

    // Guard that the type object matches.
    Register typeReg = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICSetElem_Dense::offsetOfType()), typeReg);
    masm.branchPtr(Assembler::NotEqual, Address(obj, JSObject::offsetOfType()), typeReg,
                   &failureUnstow);
    regs.add(typeReg);

    // Stack is now: { ..., rhs-value, object-value, key-value, maybe?-RET-ADDR }
    // Load rhs-value in to R0
    masm.loadValue(Address(BaselineStackReg, 2 * sizeof(Value) + ICStackValueOffset), R0);

    // Call the type-update stub.
    if (!callTypeUpdateIC(masm, sizeof(Value)))
        return false;

    // Unstow R0 and R1 (object and key)
    EmitUnstowICValues(masm, 2);

    // Reset register set.
    regs = availableGeneralRegs(2);
    scratchReg = regs.takeAny();

    // Load obj->elements in scratchReg.
    masm.loadPtr(Address(obj, JSObject::offsetOfElements()), scratchReg);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Bounds check.
    Address initLength(scratchReg, ObjectElements::offsetOfInitializedLength());
    masm.branch32(Assembler::BelowOrEqual, initLength, key, &failure);

    // Hole check.
    BaseIndex element(scratchReg, key, TimesEight);
    masm.branchTestMagic(Assembler::Equal, element, &failure);

    // Convert int32 values to double if convertDoubleElements is set. In this
    // case the heap typeset is guaranteed to contain both int32 and double, so
    // it's okay to store a double.
    Label convertDoubles, convertDoublesDone;
    Address elementsFlags(scratchReg, ObjectElements::offsetOfFlags());
    masm.branchTest32(Assembler::NonZero, elementsFlags,
                      Imm32(ObjectElements::CONVERT_DOUBLE_ELEMENTS),
                      &convertDoubles);
    masm.bind(&convertDoublesDone);

    // It's safe to overwrite R0 now.
    Address valueAddr(BaselineStackReg, ICStackValueOffset);
    masm.loadValue(valueAddr, R0);
    EmitPreBarrier(masm, element, MIRType_Value);
    masm.storeValue(R0, element);
    EmitReturnFromIC(masm);

    // Convert to double and jump back.
    masm.bind(&convertDoubles);
    masm.convertInt32ValueToDouble(valueAddr, R0.scratchReg(), &convertDoublesDone);
    masm.jump(&convertDoublesDone);

    // Failure case - fail but first unstow R0 and R1
    masm.bind(&failureUnstow);
    EmitUnstowICValues(masm, 2);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

static bool
GetProtoShapes(JSObject *obj, size_t protoChainDepth, AutoShapeVector *shapes)
{
    JS_ASSERT(shapes->length() == 1);
    JSObject *curProto = obj->getProto();
    for (size_t i = 0; i < protoChainDepth; i++) {
        if (!shapes->append(curProto->lastProperty()))
            return false;
        curProto = curProto->getProto();
    }
    JS_ASSERT(!curProto);
    return true;
}

//
// SetElem_DenseAdd
//

ICUpdatedStub *
ICSetElemDenseAddCompiler::getStub(ICStubSpace *space)
{
    AutoShapeVector shapes(cx);
    if (!shapes.append(obj_->lastProperty()))
        return NULL;

    if (!GetProtoShapes(obj_, protoChainDepth_, &shapes))
        return NULL;

    JS_STATIC_ASSERT(ICSetElem_DenseAdd::MAX_PROTO_CHAIN_DEPTH == 4);

    ICUpdatedStub *stub = NULL;
    switch (protoChainDepth_) {
      case 0: stub = getStubSpecific<0>(space, &shapes); break;
      case 1: stub = getStubSpecific<1>(space, &shapes); break;
      case 2: stub = getStubSpecific<2>(space, &shapes); break;
      case 3: stub = getStubSpecific<3>(space, &shapes); break;
      case 4: stub = getStubSpecific<4>(space, &shapes); break;
      default: JS_NOT_REACHED("ProtoChainDepth too high.");
    }
    if (!stub || !stub->initUpdatingChain(cx, space))
        return NULL;
    return stub;
}

bool
ICSetElemDenseAddCompiler::generateStubCode(MacroAssembler &masm)
{
    // R0 = object
    // R1 = key
    // Stack = { ... rhs-value, <return-addr>? }
    Label failure;
    Label failureUnstow;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox R0 and guard on its shape.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetElem_DenseAddImpl<0>::offsetOfShape(0)),
                 scratchReg);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratchReg, &failure);

    // Stow both R0 and R1 (object and key)
    // But R0 and R1 still hold their values.
    EmitStowICValues(masm, 2);

    // We may need to free up some registers.
    regs = availableGeneralRegs(0);
    regs.take(R0);

    // Guard that the type object matches.
    Register typeReg = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICSetElem_DenseAdd::offsetOfType()), typeReg);
    masm.branchPtr(Assembler::NotEqual, Address(obj, JSObject::offsetOfType()), typeReg,
                   &failureUnstow);
    regs.add(typeReg);

    // Shape guard objects on the proto chain.
    scratchReg = regs.takeAny();
    Register protoReg = regs.takeAny();
    for (size_t i = 0; i < protoChainDepth_; i++) {
        masm.loadObjProto(i == 0 ? obj : protoReg, protoReg);
        masm.loadPtr(Address(BaselineStubReg, ICSetElem_DenseAddImpl<0>::offsetOfShape(i + 1)),
                     scratchReg);
        masm.branchTestObjShape(Assembler::NotEqual, protoReg, scratchReg, &failureUnstow);
    }
    regs.add(protoReg);
    regs.add(scratchReg);

    // Stack is now: { ..., rhs-value, object-value, key-value, maybe?-RET-ADDR }
    // Load rhs-value in to R0
    masm.loadValue(Address(BaselineStackReg, 2 * sizeof(Value) + ICStackValueOffset), R0);

    // Call the type-update stub.
    if (!callTypeUpdateIC(masm, sizeof(Value)))
        return false;

    // Unstow R0 and R1 (object and key)
    EmitUnstowICValues(masm, 2);

    // Reset register set.
    regs = availableGeneralRegs(2);
    scratchReg = regs.takeAny();

    // Load obj->elements in scratchReg.
    masm.loadPtr(Address(obj, JSObject::offsetOfElements()), scratchReg);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Bounds check (key == initLength)
    Address initLength(scratchReg, ObjectElements::offsetOfInitializedLength());
    masm.branch32(Assembler::NotEqual, initLength, key, &failure);

    // Capacity check.
    Address capacity(scratchReg, ObjectElements::offsetOfCapacity());
    masm.branch32(Assembler::BelowOrEqual, capacity, key, &failure);

    // Increment initLength before write.
    masm.add32(Imm32(1), initLength);

    // If length is now <= key, increment length before write.
    Label skipIncrementLength;
    Address length(scratchReg, ObjectElements::offsetOfLength());
    masm.branch32(Assembler::Above, length, key, &skipIncrementLength);
    masm.add32(Imm32(1), length);
    masm.bind(&skipIncrementLength);

    // Convert int32 values to double if convertDoubleElements is set. In this
    // case the heap typeset is guaranteed to contain both int32 and double, so
    // it's okay to store a double.
    Label convertDoubles, convertDoublesDone;
    Address elementsFlags(scratchReg, ObjectElements::offsetOfFlags());
    masm.branchTest32(Assembler::NonZero, elementsFlags,
                      Imm32(ObjectElements::CONVERT_DOUBLE_ELEMENTS),
                      &convertDoubles);
    masm.bind(&convertDoublesDone);

    // Write the value.  No need for write barrier since we're not overwriting an old value.
    // It's safe to overwrite R0 now.
    BaseIndex element(scratchReg, key, TimesEight);
    Address valueAddr(BaselineStackReg, ICStackValueOffset);
    masm.loadValue(valueAddr, R0);
    masm.storeValue(R0, element);
    EmitReturnFromIC(masm);

    // Convert to double and jump back.
    masm.bind(&convertDoubles);
    masm.convertInt32ValueToDouble(valueAddr, R0.scratchReg(), &convertDoublesDone);
    masm.jump(&convertDoublesDone);

    // Failure case - fail but first unstow R0 and R1
    masm.bind(&failureUnstow);
    EmitUnstowICValues(masm, 2);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// SetElem_TypedArray
//

bool
ICSetElem_TypedArray::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);
    masm.branchTestInt32(Assembler::NotEqual, R1, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratchReg = regs.takeAny();

    // Unbox R0 and shape guard.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetElem_TypedArray::offsetOfShape()), scratchReg);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratchReg, &failure);

    // Unbox key.
    Register key = masm.extractInt32(R1, ExtractTemp1);

    // Bounds check.
    Label oobWrite;
    masm.unboxInt32(Address(obj, TypedArray::lengthOffset()), scratchReg);
    masm.branch32(Assembler::BelowOrEqual, scratchReg, key,
                  expectOutOfBounds_ ? &oobWrite : &failure);

    // Load the elements vector.
    masm.loadPtr(Address(obj, TypedArray::dataOffset()), scratchReg);

    BaseIndex dest(scratchReg, key, ScaleFromElemWidth(TypedArray::slotWidth(type_)));
    Address value(BaselineStackReg, ICStackValueOffset);

    // We need a second scratch register. It's okay to clobber the type tag of
    // R0 or R1, as long as it's restored before jumping to the next stub.
    regs = availableGeneralRegs(0);
    regs.takeUnchecked(obj);
    regs.takeUnchecked(key);
    regs.take(scratchReg);
    Register secondScratch = regs.takeAny();

    if (type_ == TypedArray::TYPE_FLOAT32 || type_ == TypedArray::TYPE_FLOAT64) {
        masm.ensureDouble(value, FloatReg0, &failure);
        masm.storeToTypedFloatArray(type_, FloatReg0, dest);
        EmitReturnFromIC(masm);
    } else if (type_ == TypedArray::TYPE_UINT8_CLAMPED) {
        Label notInt32;
        masm.branchTestInt32(Assembler::NotEqual, value, &notInt32);
        masm.unboxInt32(value, secondScratch);
        masm.clampIntToUint8(secondScratch, secondScratch);

        Label clamped;
        masm.bind(&clamped);
        masm.storeToTypedIntArray(type_, secondScratch, dest);
        EmitReturnFromIC(masm);

        // If the value is a double, clamp to uint8 and jump back.
        // Else, jump to failure.
        masm.bind(&notInt32);
        masm.branchTestDouble(Assembler::NotEqual, value, &failure);
        masm.unboxDouble(value, FloatReg0);
        masm.clampDoubleToUint8(FloatReg0, secondScratch);
        masm.jump(&clamped);
    } else {
        Label notInt32;
        masm.branchTestInt32(Assembler::NotEqual, value, &notInt32);
        masm.unboxInt32(value, secondScratch);

        Label isInt32;
        masm.bind(&isInt32);
        masm.storeToTypedIntArray(type_, secondScratch, dest);
        EmitReturnFromIC(masm);

        // If the value is a double, truncate and jump back.
        // Else, jump to failure.
        Label failureRestoreRegs;
        masm.bind(&notInt32);
        masm.branchTestDouble(Assembler::NotEqual, value, &failure);
        masm.unboxDouble(value, FloatReg0);
        masm.branchTruncateDouble(FloatReg0, secondScratch, &failureRestoreRegs);
        masm.jump(&isInt32);

        // Writing to secondScratch may have clobbered R0 or R1, restore them
        // first.
        masm.bind(&failureRestoreRegs);
        masm.tagValue(JSVAL_TYPE_OBJECT, obj, R0);
        masm.tagValue(JSVAL_TYPE_INT32, key, R1);
    }

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);

    if (expectOutOfBounds_) {
        masm.bind(&oobWrite);
        EmitReturnFromIC(masm);
    }
    return true;
}

//
// In_Fallback
//

static bool
DoInFallback(JSContext *cx, ICIn_Fallback *stub, HandleValue key, HandleValue objValue,
             MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "In");

    if (!objValue.isObject()) {
        js_ReportValueError(cx, JSMSG_IN_NOT_OBJECT, -1, objValue, NullPtr());
        return false;
    }

    RootedObject obj(cx, &objValue.toObject());

    JSBool cond = false;
    if (!OperatorIn(cx, key, obj, &cond))
        return false;

    res.setBoolean(cond);
    return true;
}

typedef bool (*DoInFallbackFn)(JSContext *, ICIn_Fallback *, HandleValue, HandleValue,
                               MutableHandleValue);
static const VMFunction DoInFallbackInfo =
    FunctionInfo<DoInFallbackFn>(DoInFallback, PopValues(2));

bool
ICIn_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    // Sync for the decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);

    return tailCallVM(DoInFallbackInfo, masm);
}

// Attach an optimized stub for a GETGNAME/CALLGNAME op.
static bool
TryAttachGlobalNameStub(JSContext *cx, HandleScript script, ICGetName_Fallback *stub,
                        HandleObject global, HandlePropertyName name)
{
    JS_ASSERT(global->isGlobal());

    RootedId id(cx, NameToId(name));

    // The property must be found, and it must be found as a normal data property.
    RootedShape shape(cx, global->nativeLookup(cx, id));
    if (!shape || !shape->hasDefaultGetter() || !shape->hasSlot())
        return true;

    JS_ASSERT(shape->slot() >= global->numFixedSlots());
    uint32_t slot = shape->slot() - global->numFixedSlots();

    // TODO: if there's a previous stub discard it, or just update its Shape + slot?

    ICStub *monitorStub = stub->fallbackMonitorStub()->firstMonitorStub();
    IonSpew(IonSpew_BaselineIC, "  Generating GetName(GlobalName) stub");
    ICGetName_Global::Compiler compiler(cx, monitorStub, global->lastProperty(), slot);
    ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
    if (!newStub)
        return false;

    stub->addNewStub(newStub);
    return true;
}

static bool
TryAttachScopeNameStub(JSContext *cx, HandleScript script, ICGetName_Fallback *stub,
                       HandleObject initialScopeChain, HandlePropertyName name)
{
    AutoShapeVector shapes(cx);
    RootedId id(cx, NameToId(name));
    RootedObject scopeChain(cx, initialScopeChain);

    Shape *shape = NULL;
    while (scopeChain) {
        if (!shapes.append(scopeChain->lastProperty()))
            return false;

        if (scopeChain->isGlobal()) {
            shape = scopeChain->nativeLookup(cx, id);
            if (shape)
                break;
            return true;
        }

        if (!scopeChain->isScope() || scopeChain->isWith())
            return true;

        // Check for an 'own' property on the scope. There is no need to
        // check the prototype as non-with scopes do not inherit properties
        // from any prototype.
        shape = scopeChain->nativeLookup(cx, id);
        if (shape)
            break;

        scopeChain = scopeChain->enclosingScope();
    }

    if (!IsCacheableGetPropReadSlot(scopeChain, scopeChain, shape))
        return true;

    bool isFixedSlot;
    uint32_t offset;
    GetFixedOrDynamicSlotOffset(scopeChain, shape->slot(), &isFixedSlot, &offset);

    ICStub *monitorStub = stub->fallbackMonitorStub()->firstMonitorStub();
    ICStub *newStub;

    switch (shapes.length()) {
      case 1: {
        ICGetName_Scope<0>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 2: {
        ICGetName_Scope<1>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 3: {
        ICGetName_Scope<2>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 4: {
        ICGetName_Scope<3>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 5: {
        ICGetName_Scope<4>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 6: {
        ICGetName_Scope<5>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      case 7: {
        ICGetName_Scope<6>::Compiler compiler(cx, monitorStub, &shapes, isFixedSlot, offset);
        newStub = compiler.getStub(compiler.getStubSpace(script));
        break;
      }
      default:
        return true;
    }

    if (!newStub)
        return false;

    stub->addNewStub(newStub);
    return true;
}

static bool
DoGetNameFallback(JSContext *cx, BaselineFrame *frame, ICGetName_Fallback *stub,
                  HandleObject scopeChain, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    mozilla::DebugOnly<JSOp> op = JSOp(*pc);
    FallbackICSpew(cx, stub, "GetName(%s)", js_CodeName[JSOp(*pc)]);

    JS_ASSERT(op == JSOP_NAME || op == JSOP_CALLNAME || op == JSOP_GETGNAME || op == JSOP_CALLGNAME);

    RootedPropertyName name(cx, script->getName(pc));

    if (JSOp(pc[JSOP_GETGNAME_LENGTH]) == JSOP_TYPEOF) {
        if (!GetScopeNameForTypeOf(cx, scopeChain, name, res))
            return false;
    } else {
        if (!GetScopeName(cx, scopeChain, name, res))
            return false;
    }

    types::TypeScript::Monitor(cx, script, pc, res);

    // Add a type monitor stub for the resulting value.
    if (!stub->addMonitorStubForValue(cx, script, res))
        return false;

    // Attach new stub.
    if (stub->numOptimizedStubs() >= ICGetName_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with generic stub.
        return true;
    }

    if (js_CodeSpec[*pc].format & JOF_GNAME) {
        if (!TryAttachGlobalNameStub(cx, script, stub, scopeChain, name))
            return false;
    } else {
        if (!TryAttachScopeNameStub(cx, script, stub, scopeChain, name))
            return false;
    }

    return true;
}

typedef bool (*DoGetNameFallbackFn)(JSContext *, BaselineFrame *, ICGetName_Fallback *,
                                    HandleObject, MutableHandleValue);
static const VMFunction DoGetNameFallbackInfo = FunctionInfo<DoGetNameFallbackFn>(DoGetNameFallback);

bool
ICGetName_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    EmitRestoreTailCallReg(masm);

    masm.push(R0.scratchReg());
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoGetNameFallbackInfo, masm);
}

bool
ICGetName_Global::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    Register obj = R0.scratchReg();
    Register scratch = R1.scratchReg();

    // Shape guard.
    masm.loadPtr(Address(BaselineStubReg, ICGetName_Global::offsetOfShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, obj, scratch, &failure);

    // Load dynamic slot.
    masm.loadPtr(Address(obj, JSObject::offsetOfSlots()), obj);
    masm.load32(Address(BaselineStubReg, ICGetName_Global::offsetOfSlot()), scratch);
    masm.loadValue(BaseIndex(obj, scratch, TimesEight), R0);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

template <size_t NumHops>
bool
ICGetName_Scope<NumHops>::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register obj = R0.scratchReg();
    Register walker = regs.takeAny();
    Register scratch = regs.takeAny();

    // Use a local to silence Clang tautological-compare warning if NumHops is 0.
    size_t numHops = NumHops;

    for (size_t index = 0; index < NumHops + 1; index++) {
        Register scope = index ? walker : obj;

        // Shape guard.
        masm.loadPtr(Address(BaselineStubReg, ICGetName_Scope::offsetOfShape(index)), scratch);
        masm.branchTestObjShape(Assembler::NotEqual, scope, scratch, &failure);

        if (index < numHops)
            masm.extractObject(Address(scope, ScopeObject::offsetOfEnclosingScope()), walker);
    }

    Register scope = NumHops ? walker : obj;

    if (!isFixedSlot_) {
        masm.loadPtr(Address(scope, JSObject::offsetOfSlots()), walker);
        scope = walker;
    }

    masm.load32(Address(BaselineStubReg, ICGetName_Scope::offsetOfOffset()), scratch);
    masm.loadValue(BaseIndex(scope, scratch, TimesOne), R0);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// BindName_Fallback
//

static bool
DoBindNameFallback(JSContext *cx, BaselineFrame *frame, ICBindName_Fallback *stub,
                   HandleObject scopeChain, MutableHandleValue res)
{
    jsbytecode *pc = stub->icEntry()->pc(frame->script());
    mozilla::DebugOnly<JSOp> op = JSOp(*pc);
    FallbackICSpew(cx, stub, "BindName(%s)", js_CodeName[JSOp(*pc)]);

    JS_ASSERT(op == JSOP_BINDNAME);

    RootedPropertyName name(cx, frame->script()->getName(pc));

    RootedObject scope(cx);
    if (!LookupNameWithGlobalDefault(cx, name, scopeChain, &scope))
        return false;

    res.setObject(*scope);
    return true;
}

typedef bool (*DoBindNameFallbackFn)(JSContext *, BaselineFrame *, ICBindName_Fallback *,
                                     HandleObject, MutableHandleValue);
static const VMFunction DoBindNameFallbackInfo =
    FunctionInfo<DoBindNameFallbackFn>(DoBindNameFallback);

bool
ICBindName_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    EmitRestoreTailCallReg(masm);

    masm.push(R0.scratchReg());
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoBindNameFallbackInfo, masm);
}

//
// GetIntrinsic_Fallback
//

static bool
DoGetIntrinsicFallback(JSContext *cx, BaselineFrame *frame, ICGetIntrinsic_Fallback *stub,
                       MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    mozilla::DebugOnly<JSOp> op = JSOp(*pc);
    FallbackICSpew(cx, stub, "GetIntrinsic(%s)", js_CodeName[JSOp(*pc)]);

    JS_ASSERT(op == JSOP_GETINTRINSIC || op == JSOP_CALLINTRINSIC);

    if (!GetIntrinsicOperation(cx, script, pc, res))
        return false;

    // An intrinsic operation will always produce the same result, so only
    // needs to be monitored once. Attach a stub to load the resulting constant
    // directly.

    types::TypeScript::Monitor(cx, script, pc, res);

    IonSpew(IonSpew_BaselineIC, "  Generating GetIntrinsic optimized stub");
    ICGetIntrinsic_Constant::Compiler compiler(cx, res);
    ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
    if (!newStub)
        return false;

    stub->addNewStub(newStub);
    return true;
}

typedef bool (*DoGetIntrinsicFallbackFn)(JSContext *, BaselineFrame *, ICGetIntrinsic_Fallback *,
                                         MutableHandleValue);
static const VMFunction DoGetIntrinsicFallbackInfo =
    FunctionInfo<DoGetIntrinsicFallbackFn>(DoGetIntrinsicFallback);

bool
ICGetIntrinsic_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoGetIntrinsicFallbackInfo, masm);
}

bool
ICGetIntrinsic_Constant::Compiler::generateStubCode(MacroAssembler &masm)
{
    masm.loadValue(Address(BaselineStubReg, ICGetIntrinsic_Constant::offsetOfValue()), R0);

    EmitReturnFromIC(masm);
    return true;
}

//
// GetProp_Fallback
//

static bool
TryAttachLengthStub(JSContext *cx, HandleScript script, ICGetProp_Fallback *stub, HandleValue val,
                    HandleValue res, bool *attached)
{
    JS_ASSERT(!*attached);

    if (val.isString()) {
        JS_ASSERT(res.isInt32());
        IonSpew(IonSpew_BaselineIC, "  Generating GetProp(String.length) stub");
        ICGetProp_StringLength::Compiler compiler(cx);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        *attached = true;
        stub->addNewStub(newStub);
        return true;
    }

    if (!val.isObject())
        return true;

    RootedObject obj(cx, &val.toObject());

    if (obj->isArray() && res.isInt32()) {
        IonSpew(IonSpew_BaselineIC, "  Generating GetProp(Array.length) stub");
        ICGetProp_ArrayLength::Compiler compiler(cx);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        *attached = true;
        stub->addNewStub(newStub);
        return true;
    }
    if (obj->isTypedArray()) {
        JS_ASSERT(res.isInt32());
        IonSpew(IonSpew_BaselineIC, "  Generating GetProp(TypedArray.length) stub");
        ICGetProp_TypedArrayLength::Compiler compiler(cx);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        *attached = true;
        stub->addNewStub(newStub);
        return true;
    }

    return true;
}

static bool
TryAttachNativeGetPropStub(JSContext *cx, HandleScript script, jsbytecode *pc,
                           ICGetProp_Fallback *stub, HandlePropertyName name,
                           HandleValue val, HandleValue res, bool *attached)
{
    JS_ASSERT(!*attached);

    if (!val.isObject())
        return true;

    RootedObject obj(cx, &val.toObject());
    if (!obj->isNative())
        return true;

    RootedShape shape(cx);
    RootedObject holder(cx);
    if (!EffectlesslyLookupProperty(cx, obj, name, &holder, &shape))
        return false;

    ICStub *monitorStub = stub->fallbackMonitorStub()->firstMonitorStub();
    if (IsCacheableGetPropReadSlot(obj, holder, shape)) {
        bool isFixedSlot;
        uint32_t offset;
        GetFixedOrDynamicSlotOffset(holder, shape->slot(), &isFixedSlot, &offset);

        ICStub::Kind kind = (obj == holder) ? ICStub::GetProp_Native
                                            : ICStub::GetProp_NativePrototype;

        IonSpew(IonSpew_BaselineIC, "  Generating GetProp(Native %s) stub",
                    (obj == holder) ? "direct" : "prototype");
        ICGetPropNativeCompiler compiler(cx, kind, monitorStub, obj, holder, isFixedSlot, offset);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        stub->addNewStub(newStub);
        *attached = true;
        return true;
    }

    // Only handle CallGetter stubs on prototypes for now, not direct getters.
    if (IsCacheableGetPropCallScripted(obj, holder, shape)) {
        RootedFunction callee(cx, shape->getterObject()->toFunction());
        JS_ASSERT(obj != holder);
        JS_ASSERT(callee->hasScript());

        IonSpew(IonSpew_BaselineIC, "  Generating GetProp(Native %s Getter %s:%d) stub",
                    (obj == holder) ? "direct" : "prototype",
                    callee->nonLazyScript()->filename(), callee->nonLazyScript()->lineno);

        ICGetProp_CallScripted::Compiler compiler(cx, monitorStub, obj, holder, callee,
                                                  pc - script->code);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        stub->addNewStub(newStub);
        *attached = true;
        return true;
    }

    return true;
}

static bool
TryAttachStringGetPropStub(JSContext *cx, HandleScript script, ICGetProp_Fallback *stub,
                           HandlePropertyName name, HandleValue val, HandleValue res,
                           bool *attached)
{
    JS_ASSERT(!*attached);
    JS_ASSERT(val.isString());

    RootedObject stringProto(cx, script->global().getOrCreateStringPrototype(cx));
    if (!stringProto)
        return false;

    // For now, only look for properties directly set on String.prototype
    RootedId propId(cx, NameToId(name));
    RootedShape shape(cx, stringProto->nativeLookup(cx, propId));
    if (!shape || !shape->hasSlot() || !shape->hasDefaultGetter())
        return true;

    bool isFixedSlot;
    uint32_t offset;
    GetFixedOrDynamicSlotOffset(stringProto, shape->slot(), &isFixedSlot, &offset);

    ICStub *monitorStub = stub->fallbackMonitorStub()->firstMonitorStub();

    IonSpew(IonSpew_BaselineIC, "  Generating GetProp(String.ID from prototype) stub");
    ICGetProp_String::Compiler compiler(cx, monitorStub, stringProto, isFixedSlot, offset);
    ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
    if (!newStub)
        return false;

    stub->addNewStub(newStub);
    *attached = true;
    return true;
}

static bool
DoGetPropFallback(JSContext *cx, BaselineFrame *frame, ICGetProp_Fallback *stub,
                  MutableHandleValue val, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "GetProp(%s)", js_CodeName[op]);

    JS_ASSERT(op == JSOP_GETPROP || op == JSOP_CALLPROP || op == JSOP_LENGTH || op == JSOP_GETXPROP);

    RootedPropertyName name(cx, script->getName(pc));
    RootedId id(cx, NameToId(name));

    if (op == JSOP_LENGTH && val.isMagic(JS_OPTIMIZED_ARGUMENTS)) {
        // Handle arguments.length access.
        if (IsOptimizedArguments(frame, val.address())) {
            // TODO: attach optimized stub.
            res.setInt32(frame->numActualArgs());
            types::TypeScript::Monitor(cx, script, pc, res);
            return true;
        }
    }

    RootedObject obj(cx, ToObjectFromStack(cx, val));
    if (!obj)
        return false;

    if (obj->getOps()->getProperty) {
        if (!JSObject::getGeneric(cx, obj, obj, id, res))
            return false;
    } else {
        if (!GetPropertyHelper(cx, obj, id, 0, res))
            return false;
    }

#if JS_HAS_NO_SUCH_METHOD
    // Handle objects with __noSuchMethod__.
    if (op == JSOP_CALLPROP && JS_UNLIKELY(res.isPrimitive()) && val.isObject()) {
        if (!OnUnknownMethod(cx, obj, IdToValue(id), res))
            return false;
    }
#endif

    types::TypeScript::Monitor(cx, script, pc, res);

    // Add a type monitor stub for the resulting value.
    if (!stub->addMonitorStubForValue(cx, script, res))
        return false;

    if (stub->numOptimizedStubs() >= ICGetProp_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with generic getprop stub.
        return true;
    }

    bool attached = false;

    if (op == JSOP_LENGTH) {
        if (!TryAttachLengthStub(cx, script, stub, val, res, &attached))
            return false;
        if (attached)
            return true;
    }

    if (!TryAttachNativeGetPropStub(cx, script, pc, stub, name, val, res, &attached))
        return false;
    if (attached)
        return true;

    if (val.isString()) {
        if (!TryAttachStringGetPropStub(cx, script, stub, name, val, res, &attached))
            return false;
        if (attached)
            return true;
    }

    JS_ASSERT(!attached);
    stub->noteUnoptimizableAccess();

    return true;
}

typedef bool (*DoGetPropFallbackFn)(JSContext *, BaselineFrame *, ICGetProp_Fallback *,
                                    MutableHandleValue, MutableHandleValue);
static const VMFunction DoGetPropFallbackInfo =
    FunctionInfo<DoGetPropFallbackFn>(DoGetPropFallback, PopValues(1));

bool
ICGetProp_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);

    // Push arguments.
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoGetPropFallbackInfo, masm);
}

bool
ICGetProp_ArrayLength::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    Register scratch = R1.scratchReg();

    // Unbox R0 and guard it's an array.
    Register obj = masm.extractObject(R0, ExtractTemp0);
    masm.branchTestObjClass(Assembler::NotEqual, obj, scratch, &ArrayClass, &failure);

    // Load obj->elements->length.
    masm.loadPtr(Address(obj, JSObject::offsetOfElements()), scratch);
    masm.load32(Address(scratch, ObjectElements::offsetOfLength()), scratch);

    // Guard length fits in an int32.
    masm.branchTest32(Assembler::Signed, scratch, scratch, &failure);

    masm.tagValue(JSVAL_TYPE_INT32, scratch, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICGetProp_TypedArrayLength::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    Register scratch = R1.scratchReg();

    // Unbox R0.
    Register obj = masm.extractObject(R0, ExtractTemp0);

    // Implement the negated version of JSObject::isTypedArray predicate.
    masm.loadObjClass(obj, scratch);
    masm.branchPtr(Assembler::Below, scratch, ImmWord(&TypedArray::classes[0]), &failure);
    masm.branchPtr(Assembler::AboveOrEqual, scratch,
                   ImmWord(&TypedArray::classes[TypedArray::TYPE_MAX]), &failure);

    // Load length from fixed slot.
    masm.loadValue(Address(obj, TypedArray::lengthOffset()), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICGetProp_StringLength::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);

    // Unbox string and load its length.
    Register string = masm.extractString(R0, ExtractTemp0);
    masm.loadStringLength(string, string);

    masm.tagValue(JSVAL_TYPE_INT32, string, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICGetProp_String::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    masm.branchTestString(Assembler::NotEqual, R0, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register holderReg = regs.takeAny();
    Register scratchReg = regs.takeAny();

    // Verify the shape of |String.prototype|
    masm.movePtr(ImmGCPtr(stringPrototype_.get()), holderReg);

    Address shapeAddr(BaselineStubReg, ICGetProp_String::offsetOfStringProtoShape());
    masm.loadPtr(Address(holderReg, JSObject::offsetOfShape()), scratchReg);
    masm.branchPtr(Assembler::NotEqual, shapeAddr, scratchReg, &failure);

    if (!isFixedSlot_)
        masm.loadPtr(Address(holderReg, JSObject::offsetOfSlots()), holderReg);

    masm.load32(Address(BaselineStubReg, ICGetPropNativeStub::offsetOfOffset()), scratchReg);
    masm.loadValue(BaseIndex(holderReg, scratchReg, TimesOne), R0);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICGetPropNativeCompiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    GeneralRegisterSet regs(availableGeneralRegs(1));

    // Guard input is an object.
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    Register scratch = regs.takeAny();

    // Unbox and shape guard.
    Register objReg = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICGetPropNativeStub::offsetOfShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, objReg, scratch, &failure);

    Register holderReg;
    if (obj_ == holder_) {
        holderReg = objReg;
    } else {
        // Shape guard holder.
        holderReg = regs.takeAny();
        masm.loadPtr(Address(BaselineStubReg, ICGetProp_NativePrototype::offsetOfHolder()),
                     holderReg);
        masm.loadPtr(Address(BaselineStubReg, ICGetProp_NativePrototype::offsetOfHolderShape()),
                     scratch);
        masm.branchTestObjShape(Assembler::NotEqual, holderReg, scratch, &failure);
    }

    if (!isFixedSlot_)
        masm.loadPtr(Address(holderReg, JSObject::offsetOfSlots()), holderReg);

    masm.load32(Address(BaselineStubReg, ICGetPropNativeStub::offsetOfOffset()), scratch);
    masm.loadValue(BaseIndex(holderReg, scratch, TimesOne), R0);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICGetProp_CallScripted::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    Label failureLeaveStubFrame;
    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register scratch;
    if (regs.has(BaselineTailCallReg)) {
        regs.take(BaselineTailCallReg);
        scratch = regs.takeAny();
        regs.add(BaselineTailCallReg);
    } else {
        scratch = regs.takeAny();
    }

    // Guard input is an object.
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Unbox and shape guard.
    Register objReg = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICGetProp_CallScripted::offsetOfShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, objReg, scratch, &failure);

    Register holderReg = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICGetProp_CallScripted::offsetOfHolder()), holderReg);
    masm.loadPtr(Address(BaselineStubReg, ICGetProp_CallScripted::offsetOfHolderShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, holderReg, scratch, &failure);
    regs.add(holderReg);

    // Push a stub frame so that we can perform a non-tail call.
    enterStubFrame(masm, scratch);

    // Load callee function and code.  To ensure that |code| doesn't end up being
    // ArgumentsRectifierReg, if it's available we assign it to |callee| instead.
    Register callee;
    if (regs.has(ArgumentsRectifierReg)) {
        callee = ArgumentsRectifierReg;
        regs.take(callee);
    } else {
        callee = regs.takeAny();
    }
    Register code = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICGetProp_CallScripted::offsetOfGetter()), callee);
    masm.loadPtr(Address(callee, JSFunction::offsetOfNativeOrScript()), code);
    masm.loadBaselineOrIonCode(code, scratch, &failureLeaveStubFrame);
    masm.loadPtr(Address(code, IonCode::offsetOfCode()), code);

    // Getter is called with 0 arguments, just |obj| as thisv.
    // Note that we use Push, not push, so that callIon will align the stack
    // properly on ARM.
    masm.Push(R0);
    EmitCreateStubFrameDescriptor(masm, scratch);
    masm.Push(Imm32(0));  // ActualArgc is 0
    masm.Push(callee);
    masm.Push(scratch);

    // Handle arguments underflow.
    Label noUnderflow;
    masm.load16ZeroExtend(Address(callee, offsetof(JSFunction, nargs)), scratch);
    masm.branch32(Assembler::Equal, scratch, Imm32(0), &noUnderflow);
    {
        // Call the arguments rectifier.
        JS_ASSERT(ArgumentsRectifierReg != code);

        IonCode *argumentsRectifier =
            cx->compartment->ionCompartment()->getArgumentsRectifier(SequentialExecution);

        masm.movePtr(ImmGCPtr(argumentsRectifier), code);
        masm.loadPtr(Address(code, IonCode::offsetOfCode()), code);
        masm.mov(Imm32(0), ArgumentsRectifierReg);
    }

    masm.bind(&noUnderflow);

    // If needed, update SPS Profiler frame entry.  At this point, callee and scratch can
    // be clobbered.
    {
        Label skipProfilerUpdate;

        // Need to avoid using ArgumentsRectifierReg and code register.
        GeneralRegisterSet availRegs = availableGeneralRegs(0);
        availRegs.take(ArgumentsRectifierReg);
        availRegs.take(code);
        Register scratch = availRegs.takeAny();
        Register pcIdx = availRegs.takeAny();

        // Check if profiling is enabled.
        guardProfilingEnabled(masm, scratch, &skipProfilerUpdate);

        // Update profiling entry before leaving function.
        masm.load32(Address(BaselineStubReg, ICGetProp_CallScripted::offsetOfPCOffset()), pcIdx);
        masm.spsUpdatePCIdx(&cx->runtime->spsProfiler, pcIdx, scratch);

        masm.bind(&skipProfilerUpdate);
    }
    masm.callIon(code);

    leaveStubFrame(masm, true);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Leave stub frame and go to next stub.
    masm.bind(&failureLeaveStubFrame);
    leaveStubFrame(masm, false);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// SetProp_Fallback
//

// Attach an optimized stub for a SETPROP/SETGNAME/SETNAME op.
static bool
TryAttachSetPropStub(JSContext *cx, HandleScript script, jsbytecode *pc, ICSetProp_Fallback *stub,
                     HandleObject obj, HandleShape oldShape, uint32_t oldSlots,
                     HandlePropertyName name, HandleId id, HandleValue rhs, bool *attached)
{
    JS_ASSERT(!*attached);

    if (!obj->isNative() || obj->watched())
        return true;

    RootedShape shape(cx);
    RootedObject holder(cx);
    if (!EffectlesslyLookupProperty(cx, obj, name, &holder, &shape))
        return false;

    size_t chainDepth;
    if (IsCacheableSetPropAddSlot(cx, obj, oldShape, oldSlots, id, holder, shape, &chainDepth)) {
        // Don't attach if proto chain depth is too high.
        if (chainDepth > ICSetProp_NativeAdd::MAX_PROTO_CHAIN_DEPTH)
            return true;

        bool isFixedSlot;
        uint32_t offset;
        GetFixedOrDynamicSlotOffset(obj, shape->slot(), &isFixedSlot, &offset);

        IonSpew(IonSpew_BaselineIC, "  Generating SetProp(NativeObject.ADD) stub");
        ICSetPropNativeAddCompiler compiler(cx, obj, oldShape, chainDepth, isFixedSlot, offset);
        ICUpdatedStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;
        if (!newStub->addUpdateStubForValue(cx, script, obj, id, rhs))
            return false;

        stub->addNewStub(newStub);
        *attached = true;
        return true;
    }

    if (IsCacheableSetPropWriteSlot(obj, oldShape, holder, shape)) {
        bool isFixedSlot;
        uint32_t offset;
        GetFixedOrDynamicSlotOffset(obj, shape->slot(), &isFixedSlot, &offset);

        IonSpew(IonSpew_BaselineIC, "  Generating SetProp(NativeObject.PROP) stub");
        ICSetProp_Native::Compiler compiler(cx, obj, isFixedSlot, offset);
        ICUpdatedStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;
        if (!newStub->addUpdateStubForValue(cx, script, obj, id, rhs))
            return false;

        stub->addNewStub(newStub);
        *attached = true;
        return true;
    }

    if (IsCacheableSetPropCallScripted(obj, holder, shape)) {
        RootedFunction callee(cx, shape->setterObject()->toFunction());
        JS_ASSERT(obj != holder);
        JS_ASSERT(callee->hasScript());

        IonSpew(IonSpew_BaselineIC, "  Generating SetProp(Native %s Setter %s:%d) stub",
                    (obj == holder) ? "direct" : "prototype",
                    callee->nonLazyScript()->filename(), callee->nonLazyScript()->lineno);

        ICSetProp_CallScripted::Compiler compiler(cx, obj, holder, callee, pc - script->code);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        stub->addNewStub(newStub);
        *attached = true;
        return true;
    }

    return true;
}

static bool
DoSetPropFallback(JSContext *cx, BaselineFrame *frame, ICSetProp_Fallback *stub, HandleValue lhs,
                  HandleValue rhs, MutableHandleValue res)
{
    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "SetProp(%s)", js_CodeName[op]);

    JS_ASSERT(op == JSOP_SETPROP || op == JSOP_SETNAME || op == JSOP_SETGNAME || op == JSOP_INITPROP);

    RootedPropertyName name(cx, script->getName(pc));
    RootedId id(cx, NameToId(name));

    RootedObject obj(cx, ToObjectFromStack(cx, lhs));
    if (!obj)
        return false;
    RootedShape oldShape(cx, obj->lastProperty());
    uint32_t oldSlots = obj->numDynamicSlots();

    if (op == JSOP_INITPROP && name != cx->names().proto) {
        JS_ASSERT(obj->isObject());
        if (!DefineNativeProperty(cx, obj, id, rhs, NULL, NULL, JSPROP_ENUMERATE, 0, 0, 0))
            return false;
    } else if (op == JSOP_SETNAME || op == JSOP_SETGNAME) {
        if (!SetNameOperation(cx, script, pc, obj, rhs))
            return false;
    } else if (script->strict) {
        if (!js::SetProperty<true>(cx, obj, id, rhs))
            return false;
    } else {
        if (!js::SetProperty<false>(cx, obj, id, rhs))
            return false;
    }

    // Leave the RHS on the stack.
    res.set(rhs);

    if (stub->numOptimizedStubs() >= ICSetProp_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with generic setprop stub.
        return true;
    }

    bool attached = false;
    if (!TryAttachSetPropStub(cx, script, pc, stub, obj, oldShape, oldSlots, name, id, rhs,
         &attached))
    {
        return false;
    }
    if (attached)
        return true;

    JS_ASSERT(!attached);
    stub->noteUnoptimizableAccess();

    return true;
}

typedef bool (*DoSetPropFallbackFn)(JSContext *, BaselineFrame *, ICSetProp_Fallback *,
                                    HandleValue, HandleValue, MutableHandleValue);
static const VMFunction DoSetPropFallbackInfo =
    FunctionInfo<DoSetPropFallbackFn>(DoSetPropFallback, PopValues(2));

bool
ICSetProp_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    EmitRestoreTailCallReg(masm);

    // Ensure stack is fully synced for the expression decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    // Push arguments.
    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoSetPropFallbackInfo, masm);
}

bool
ICSetProp_Native::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;

    // Guard input is an object.
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratch = regs.takeAny();

    // Unbox and shape guard.
    Register objReg = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_Native::offsetOfShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, objReg, scratch, &failure);

    // Guard that the type object matches.
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_Native::offsetOfType()), scratch);
    masm.branchPtr(Assembler::NotEqual, Address(objReg, JSObject::offsetOfType()), scratch,
                   &failure);

    // Stow both R0 and R1 (object and value).
    EmitStowICValues(masm, 2);

    // Type update stub expects the value to check in R0.
    masm.moveValue(R1, R0);

    // Call the type-update stub.
    if (!callTypeUpdateIC(masm, sizeof(Value)))
        return false;

    // Unstow R0 and R1 (object and key)
    EmitUnstowICValues(masm, 2);

    if (!isFixedSlot_)
        masm.loadPtr(Address(objReg, JSObject::offsetOfSlots()), objReg);

    // Perform the store.
    masm.load32(Address(BaselineStubReg, ICSetProp_Native::offsetOfOffset()), scratch);
    EmitPreBarrier(masm, BaseIndex(objReg, scratch, TimesOne), MIRType_Value);
    masm.storeValue(R1, BaseIndex(objReg, scratch, TimesOne));

    // The RHS has to be in R0.
    masm.moveValue(R1, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

ICUpdatedStub *
ICSetPropNativeAddCompiler::getStub(ICStubSpace *space)
{
    AutoShapeVector shapes(cx);
    if (!shapes.append(oldShape_))
        return NULL;

    if (!GetProtoShapes(obj_, protoChainDepth_, &shapes))
        return NULL;

    JS_STATIC_ASSERT(ICSetProp_NativeAdd::MAX_PROTO_CHAIN_DEPTH == 4);

    ICUpdatedStub *stub = NULL;
    switch(protoChainDepth_) {
      case 0: stub = getStubSpecific<0>(space, &shapes); break;
      case 1: stub = getStubSpecific<1>(space, &shapes); break;
      case 2: stub = getStubSpecific<2>(space, &shapes); break;
      case 3: stub = getStubSpecific<3>(space, &shapes); break;
      case 4: stub = getStubSpecific<4>(space, &shapes); break;
      default: JS_NOT_REACHED("ProtoChainDepth too high.");
    }
    if (!stub || !stub->initUpdatingChain(cx, space))
        return NULL;
    return stub;
}

bool
ICSetPropNativeAddCompiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    Label failureUnstow;

    // Guard input is an object.
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    GeneralRegisterSet regs(availableGeneralRegs(2));
    Register scratch = regs.takeAny();

    // Unbox and guard against old shape.
    Register objReg = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_NativeAddImpl<0>::offsetOfShape(0)), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, objReg, scratch, &failure);

    // Guard that the type object matches.
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_NativeAdd::offsetOfType()), scratch);
    masm.branchPtr(Assembler::NotEqual, Address(objReg, JSObject::offsetOfType()), scratch,
                   &failure);

    // Stow both R0 and R1 (object and value).
    EmitStowICValues(masm, 2);

    regs = availableGeneralRegs(1);
    scratch = regs.takeAny();
    Register protoReg = regs.takeAny();
    // Check the proto chain.
    for (size_t i = 0; i < protoChainDepth_; i++) {
        masm.loadObjProto(i == 0 ? objReg : protoReg, protoReg);
        masm.loadPtr(Address(BaselineStubReg, ICSetProp_NativeAddImpl<0>::offsetOfShape(i + 1)),
                     scratch);
        masm.branchTestObjShape(Assembler::NotEqual, protoReg, scratch, &failureUnstow);
    }

    // Shape and type checks succeeded, ok to proceed.

    // Load RHS into R0 for TypeUpdate check.
    // Stack is currently: [..., ObjValue, RHSValue, MaybeReturnAddr? ]
    masm.loadValue(Address(BaselineStackReg, ICStackValueOffset), R0);

    // Call the type-update stub.
    if (!callTypeUpdateIC(masm, sizeof(Value)))
        return false;

    // Unstow R0 and R1 (object and key)
    EmitUnstowICValues(masm, 2);
    regs = availableGeneralRegs(2);
    scratch = regs.takeAny();

    // Changing object shape.  Write the object's new shape.
    Address shapeAddr(objReg, JSObject::offsetOfShape());
    EmitPreBarrier(masm, shapeAddr, MIRType_Shape);
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_NativeAdd::offsetOfNewShape()), scratch);
    masm.storePtr(scratch, shapeAddr);

    if (!isFixedSlot_)
        masm.loadPtr(Address(objReg, JSObject::offsetOfSlots()), objReg);

    // Perform the store.  No write barrier required since this is a new
    // initialization.
    masm.load32(Address(BaselineStubReg, ICSetProp_NativeAdd::offsetOfOffset()), scratch);
    masm.storeValue(R1, BaseIndex(objReg, scratch, TimesOne));

    // The RHS has to be in R0.
    masm.moveValue(R1, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failureUnstow);
    EmitUnstowICValues(masm, 2);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICSetProp_CallScripted::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    Label failureUnstow;
    Label failureLeaveStubFrame;

    // Guard input is an object.
    masm.branchTestObject(Assembler::NotEqual, R0, &failure);

    // Stow R0 and R1 to free up registers.
    EmitStowICValues(masm, 2);

    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register scratch;
    if (regs.has(BaselineTailCallReg)) {
        regs.take(BaselineTailCallReg);
        scratch = regs.takeAny();
        regs.add(BaselineTailCallReg);
    } else {
        scratch = regs.takeAny();
    }

    // Unbox and shape guard.
    Register objReg = masm.extractObject(R0, ExtractTemp0);
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_CallScripted::offsetOfShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, objReg, scratch, &failureUnstow);

    Register holderReg = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_CallScripted::offsetOfHolder()), holderReg);
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_CallScripted::offsetOfHolderShape()), scratch);
    masm.branchTestObjShape(Assembler::NotEqual, holderReg, scratch, &failureUnstow);
    regs.add(holderReg);

    // Push a stub frame so that we can perform a non-tail call.
    enterStubFrame(masm, scratch);

    // Load callee function and code.  To ensure that |code| doesn't end up being
    // ArgumentsRectifierReg, if it's available we assign it to |callee| instead.
    Register callee;
    if (regs.has(ArgumentsRectifierReg)) {
        callee = ArgumentsRectifierReg;
        regs.take(callee);
    } else {
        callee = regs.takeAny();
    }
    Register code = regs.takeAny();
    masm.loadPtr(Address(BaselineStubReg, ICSetProp_CallScripted::offsetOfSetter()), callee);
    masm.loadPtr(Address(callee, JSFunction::offsetOfNativeOrScript()), code);
    masm.loadBaselineOrIonCode(code, scratch, &failureLeaveStubFrame);
    masm.loadPtr(Address(code, IonCode::offsetOfCode()), code);

    // Setter is called with the new value as the only argument, and |obj| as thisv.
    // Note that we use Push, not push, so that callIon will align the stack
    // properly on ARM.

    // To Push R1, read it off of the stowed values on stack.
    // Stack: [ ..., R0, R1, ..STUBFRAME-HEADER.. ]
    masm.movePtr(BaselineStackReg, scratch);
    masm.PushValue(Address(scratch, STUB_FRAME_SIZE));
    masm.Push(R0);
    EmitCreateStubFrameDescriptor(masm, scratch);
    masm.Push(Imm32(1));  // ActualArgc is 1
    masm.Push(callee);
    masm.Push(scratch);

    // Handle arguments underflow.
    Label noUnderflow;
    masm.load16ZeroExtend(Address(callee, offsetof(JSFunction, nargs)), scratch);
    masm.branch32(Assembler::BelowOrEqual, scratch, Imm32(1), &noUnderflow);
    {
        // Call the arguments rectifier.
        JS_ASSERT(ArgumentsRectifierReg != code);

        IonCode *argumentsRectifier =
            cx->compartment->ionCompartment()->getArgumentsRectifier(SequentialExecution);

        masm.movePtr(ImmGCPtr(argumentsRectifier), code);
        masm.loadPtr(Address(code, IonCode::offsetOfCode()), code);
        masm.mov(Imm32(1), ArgumentsRectifierReg);
    }

    masm.bind(&noUnderflow);

    // If needed, update SPS Profiler frame entry.  At this point, callee and scratch can
    // be clobbered.
    {
        Label skipProfilerUpdate;

        // Need to avoid using ArgumentsRectifierReg and code register.
        GeneralRegisterSet availRegs = availableGeneralRegs(0);
        availRegs.take(ArgumentsRectifierReg);
        availRegs.take(code);
        Register scratch = availRegs.takeAny();
        Register pcIdx = availRegs.takeAny();

        // Check if profiling is enabled.
        guardProfilingEnabled(masm, scratch, &skipProfilerUpdate);

        // Update profiling entry before leaving function.
        masm.load32(Address(BaselineStubReg, ICSetProp_CallScripted::offsetOfPCOffset()), pcIdx);
        masm.spsUpdatePCIdx(&cx->runtime->spsProfiler, pcIdx, scratch);

        masm.bind(&skipProfilerUpdate);
    }
    masm.callIon(code);

    leaveStubFrame(masm, true);
    // Do not care about return value from function. The original RHS should be returned
    // as the result of this operation.
    EmitUnstowICValues(masm, 2);
    masm.moveValue(R1, R0);
    EmitReturnFromIC(masm);

    // Leave stub frame and go to next stub.
    masm.bind(&failureLeaveStubFrame);
    leaveStubFrame(masm, false);

    // Unstow R0 and R1
    masm.bind(&failureUnstow);
    EmitUnstowICValues(masm, 2);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// Call_Fallback
//

static bool
TryAttachCallStub(JSContext *cx, ICCall_Fallback *stub, HandleScript script, jsbytecode *pc,
                  JSOp op, uint32_t argc, Value *vp, bool constructing, bool useNewType)
{
    if (useNewType || op == JSOP_EVAL)
        return true;

    if (stub->numOptimizedStubs() >= ICCall_Fallback::MAX_OPTIMIZED_STUBS) {
        // TODO: Discard all stubs in this IC and replace with inert megamorphic stub.
        // But for now we just bail.
        return true;
    }

    RootedValue callee(cx, vp[0]);
    RootedValue thisv(cx, vp[1]);

    if (!callee.isObject())
        return true;

    RootedObject obj(cx, &callee.toObject());
    if (!obj->isFunction())
        return true;

    RootedFunction fun(cx, obj->toFunction());
    if (fun->hasScript()) {
        RootedScript calleeScript(cx, fun->nonLazyScript());
        if (!calleeScript->hasBaselineScript() && !calleeScript->hasIonScript())
            return true;

        if (calleeScript->shouldCloneAtCallsite)
            return true;

        // Check if this stub chain has already generalized scripted calls.
        if (stub->scriptedStubsAreGeneralized()) {
            IonSpew(IonSpew_BaselineIC, "  Chain already has generalized scripted call stub!");
            return true;
        }

        if (stub->scriptedStubCount() >= ICCall_Fallback::MAX_SCRIPTED_STUBS) {
            // Create a Call_AnyScripted stub.
            IonSpew(IonSpew_BaselineIC, "  Generating Call_AnyScripted stub (cons=%s)", 
                    constructing ? "yes" : "no");

            ICCallScriptedCompiler compiler(cx, stub->fallbackMonitorStub()->firstMonitorStub(),
                                            constructing, pc - script->code);
            ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
            if (!newStub)
                return false;

            // Before adding new stub, unlink all previous Call_Scripted.
            stub->unlinkStubsWithKind(cx, ICStub::Call_Scripted);

            // Add new generalized stub.
            stub->addNewStub(newStub);
            return true;
        }

        IonSpew(IonSpew_BaselineIC,
                "  Generating Call_Scripted stub (fun=%p, %s:%d, cons=%s)",
                fun.get(), fun->nonLazyScript()->filename(), fun->nonLazyScript()->lineno,
                constructing ? "yes" : "no");
        ICCallScriptedCompiler compiler(cx, stub->fallbackMonitorStub()->firstMonitorStub(),
                                        calleeScript, constructing, pc - script->code);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        stub->addNewStub(newStub);
        return true;
    }

    if (fun->isNative() && (!constructing || (constructing && fun->isNativeConstructor()))) {

        // Generalied native call stubs are not here yet!
        JS_ASSERT(!stub->nativeStubsAreGeneralized());

        if (stub->nativeStubCount() >= ICCall_Fallback::MAX_NATIVE_STUBS) {
            IonSpew(IonSpew_BaselineIC, "  Too many Call_Native stubs. TODO: add Call_AnyNative!");
            return true;
        }

        IonSpew(IonSpew_BaselineIC, "  Generating Call_Native stub (fun=%p, cons=%s)", fun.get(),
                constructing ? "yes" : "no");
        ICCall_Native::Compiler compiler(cx, stub->fallbackMonitorStub()->firstMonitorStub(),
                                         fun, constructing, pc - script->code);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(script));
        if (!newStub)
            return false;

        stub->addNewStub(newStub);
        return true;
    }

    return true;
}

static bool
MaybeCloneFunctionAtCallsite(JSContext *cx, MutableHandleValue callee, HandleScript script,
                             jsbytecode *pc)
{
    RootedFunction fun(cx);
    if (!IsFunctionObject(callee, fun.address()))
        return true;

    if (!fun->hasScript() || !fun->nonLazyScript()->shouldCloneAtCallsite)
        return true;

    if (!cx->typeInferenceEnabled())
        return true;

    fun = CloneFunctionAtCallsite(cx, fun, script, pc);
    if (!fun)
        return false;

    callee.setObject(*fun);
    return true;
}

static bool
DoCallFallback(JSContext *cx, BaselineFrame *frame, ICCall_Fallback *stub, uint32_t argc,
               Value *vp, MutableHandleValue res)
{
    // Ensure vp array is rooted - we may GC in here.
    AutoArrayRooter vpRoot(cx, argc + 2, vp);

    RootedScript script(cx, frame->script());
    jsbytecode *pc = stub->icEntry()->pc(script);
    JSOp op = JSOp(*pc);
    FallbackICSpew(cx, stub, "Call(%s)", js_CodeName[op]);

    JS_ASSERT(argc == GET_ARGC(pc));

    RootedValue callee(cx, vp[0]);
    RootedValue thisv(cx, vp[1]);

    Value *args = vp + 2;

    // Handle funapply with JSOP_ARGUMENTS
    if (op == JSOP_FUNAPPLY && argc == 2 && args[1].isMagic(JS_OPTIMIZED_ARGUMENTS)) {
        if (!GuardFunApplyArgumentsOptimization(cx, frame, callee, args, argc))
            return false;
    }

    // Compute construcing and useNewType flags.
    bool constructing = (op == JSOP_NEW);
    bool newType = false;
    if (cx->typeInferenceEnabled())
        newType = types::UseNewType(cx, script, pc);

    // Try attaching a call stub.
    if (!TryAttachCallStub(cx, stub, script, pc, op, argc, vp, constructing, newType))
        return false;

    // Maybe update PC in profiler entry before leaving this script by call.
    if (cx->runtime->spsProfiler.enabled() && frame->hasPushedSPSFrame())
        cx->runtime->spsProfiler.updatePC(script, pc);

    if (!MaybeCloneFunctionAtCallsite(cx, &callee, script, pc))
        return false;

    if (op == JSOP_NEW) {
        if (!InvokeConstructor(cx, callee, argc, args, res.address()))
            return false;
    } else if (op == JSOP_EVAL && IsBuiltinEvalForScope(frame->scopeChain(), callee)) {
        if (!DirectEval(cx, CallArgsFromVp(argc, vp)))
            return false;
        res.set(vp[0]);
    } else {
        JS_ASSERT(op == JSOP_CALL || op == JSOP_FUNCALL || op == JSOP_FUNAPPLY || op == JSOP_EVAL);
        if (!Invoke(cx, thisv, callee, argc, args, res.address()))
            return false;
    }

    types::TypeScript::Monitor(cx, script, pc, res);

    // Attach a new TypeMonitor stub for this value.
    ICTypeMonitor_Fallback *typeMonFbStub = stub->fallbackMonitorStub();
    if (!typeMonFbStub->addMonitorStubForValue(cx, script, res))
        return false;
    // Add a type monitor stub for the resulting value.
    if (!stub->addMonitorStubForValue(cx, script, res))
        return false;

    return true;
}

void
ICCallStubCompiler::pushCallArguments(MacroAssembler &masm, GeneralRegisterSet regs, Register argcReg)
{
    JS_ASSERT(!regs.has(argcReg));

    // Push the callee and |this| too.
    Register count = regs.takeAny();
    masm.mov(argcReg, count);
    masm.add32(Imm32(2), count);

    // argPtr initially points to the last argument.
    Register argPtr = regs.takeAny();
    masm.mov(BaselineStackReg, argPtr);

    // Skip 4 pointers pushed on top of the arguments: the frame descriptor,
    // return address, old frame pointer and stub reg.
    masm.addPtr(Imm32(STUB_FRAME_SIZE), argPtr);

    // Push all values, starting at the last one.
    Label loop, done;
    masm.bind(&loop);
    masm.branchTest32(Assembler::Zero, count, count, &done);
    {
        masm.pushValue(Address(argPtr, 0));
        masm.addPtr(Imm32(sizeof(Value)), argPtr);

        masm.sub32(Imm32(1), count);
        masm.jump(&loop);
    }
    masm.bind(&done);
}

typedef bool (*DoCallFallbackFn)(JSContext *, BaselineFrame *, ICCall_Fallback *,
                                 uint32_t, Value *, MutableHandleValue);
static const VMFunction DoCallFallbackInfo = FunctionInfo<DoCallFallbackFn>(DoCallFallback);

bool
ICCall_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(R0 == JSReturnOperand);

    // Push a stub frame so that we can perform a non-tail call.
    enterStubFrame(masm, R1.scratchReg());

    // Values are on the stack left-to-right. Calling convention wants them
    // right-to-left so duplicate them on the stack in reverse order.
    // |this| and callee are pushed last.

    GeneralRegisterSet regs(availableGeneralRegs(0));
    regs.take(R0.scratchReg()); // argc.

    pushCallArguments(masm, regs, R0.scratchReg());

    masm.push(BaselineStackReg);
    masm.push(R0.scratchReg());
    masm.push(BaselineStubReg);

    // Load previous frame pointer, push BaselineFrame *.
    masm.loadPtr(Address(BaselineFrameReg, 0), R0.scratchReg());
    masm.pushBaselineFramePtr(R0.scratchReg(), R0.scratchReg());

    if (!callVM(DoCallFallbackInfo, masm))
        return false;

    leaveStubFrame(masm);
    EmitReturnFromIC(masm);

    // The following asmcode is only used when an Ion inlined frame bails out into
    // baseline jitcode.  The return address pushed onto the reconstructed baseline stack
    // points here.
    returnOffset_ = masm.currentOffset();

    // Load passed-in ThisV into R1 just in case it's needed.  Need to do this before
    // we leave the stub frame since that info will be lost.
    // Current stack:  [...., ThisV, ActualArgc, CalleeToken, Descriptor ]
    masm.loadValue(Address(BaselineStackReg, 3 * sizeof(size_t)), R1);

    leaveStubFrame(masm, true);

    // R1 and R0 are taken.
    regs = availableGeneralRegs(2);
    Register scratch = regs.takeAny();

    // If this is a |constructing| call, if the callee returns a non-object, we replace it with
    // the |this| object passed in.
    JS_ASSERT(JSReturnOperand == R0);
    Label skipThisReplace;
    masm.load16ZeroExtend(Address(BaselineStubReg, ICStub::offsetOfExtra()), scratch);
    masm.branchTest32(Assembler::Zero, scratch, Imm32(ICCall_Fallback::CONSTRUCTING_FLAG),
                      &skipThisReplace);
    masm.branchTestObject(Assembler::Equal, JSReturnOperand, &skipThisReplace);
    masm.moveValue(R1, R0);
#ifdef DEBUG
    masm.branchTestObject(Assembler::Equal, JSReturnOperand, &skipThisReplace);
    masm.breakpoint();
#endif
    masm.bind(&skipThisReplace);

    // At this point, BaselineStubReg points to the ICCall_Fallback stub, which is NOT
    // a MonitoredStub, but rather a MonitoredFallbackStub.  To use EmitEnterTypeMonitorIC,
    // first load the ICTypeMonitor_Fallback stub into BaselineStubReg.  Then, use
    // EmitEnterTypeMonitorIC with a custom struct offset.
    masm.loadPtr(Address(BaselineStubReg, ICMonitoredFallbackStub::offsetOfFallbackMonitorStub()),
                 BaselineStubReg);
    EmitEnterTypeMonitorIC(masm, ICTypeMonitor_Fallback::offsetOfFirstMonitorStub());

    return true;
}

bool
ICCall_Fallback::Compiler::postGenerateStubCode(MacroAssembler &masm, Handle<IonCode *> code)
{
    CodeOffsetLabel offset(returnOffset_);
    offset.fixup(&masm);
    cx->compartment->ionCompartment()->initBaselineCallReturnAddr(code->raw() + offset.offset());
    return true;
}

typedef bool (*CreateThisFn)(JSContext *cx, HandleObject callee, MutableHandleValue rval);
static const VMFunction CreateThisInfo = FunctionInfo<CreateThisFn>(CreateThis);

bool
ICCallScriptedCompiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    GeneralRegisterSet regs(availableGeneralRegs(0));
    bool canUseTailCallReg = regs.has(BaselineTailCallReg);

    Register argcReg = R0.scratchReg();
    JS_ASSERT(argcReg != ArgumentsRectifierReg);

    regs.take(argcReg);
    regs.take(ArgumentsRectifierReg);
    if (regs.has(BaselineTailCallReg))
        regs.take(BaselineTailCallReg);

    // Load the callee in R1.
    // Stack Layout: [ ..., CalleeVal, ThisVal, Arg0Val, ..., ArgNVal, +ICStackValueOffset+ ]
    BaseIndex calleeSlot(BaselineStackReg, argcReg, TimesEight, ICStackValueOffset + sizeof(Value));
    masm.loadValue(calleeSlot, R1);
    regs.take(R1);

    // Ensure callee is an object.
    masm.branchTestObject(Assembler::NotEqual, R1, &failure);

    // Ensure callee is a function.
    Register callee = masm.extractObject(R1, ExtractTemp0);
    masm.branchTestObjClass(Assembler::NotEqual, callee, regs.getAny(), &FunctionClass, &failure);

    // If calling a specific script, check if the script matches.  Otherwise, ensure that
    // callee function is scripted.  Leave calleeScript in |callee| reg.
    if (calleeScript_) {
        JS_ASSERT(kind == ICStub::Call_Scripted);

        // Callee is a function.  Check if script matches.
        masm.loadPtr(Address(callee, JSFunction::offsetOfNativeOrScript()), callee);
        Address expectedScript(BaselineStubReg, ICCall_Scripted::offsetOfCalleeScript());
        masm.branchPtr(Assembler::NotEqual, expectedScript, callee, &failure);
    } else {
        masm.branchIfFunctionHasNoScript(callee, &failure);
        masm.loadPtr(Address(callee, JSFunction::offsetOfNativeOrScript()), callee);
    }

    // Load IonScript or BaselineScript.
    masm.loadBaselineOrIonCode(callee, regs.getAny(), &failure);

    // Load the start of the target IonCode.
    Register code;
    if (!isConstructing_) {
        code = regs.takeAny();
        masm.loadPtr(Address(callee, IonCode::offsetOfCode()), code);
    }

    // We no longer need R1.
    regs.add(R1);

    // Push a stub frame so that we can perform a non-tail call.
    enterStubFrame(masm, regs.getAny());
    if (canUseTailCallReg)
        regs.add(BaselineTailCallReg);

    Label failureLeaveStubFrame;

    if (isConstructing_) {
        // Save argc before call.
        masm.push(argcReg);

        // Stack now looks like:
        //      [..., Callee, ThisV, Arg0V, ..., ArgNV, StubFrameHeader, ArgC ]
        BaseIndex calleeSlot2(BaselineStackReg, argcReg, TimesEight,
                               sizeof(Value) + STUB_FRAME_SIZE + sizeof(size_t));
        masm.loadValue(calleeSlot2, R1);
        masm.push(masm.extractObject(R1, ExtractTemp0));
        if (!callVM(CreateThisInfo, masm))
            return false;

        // Return of CreateThis must be an object.
#ifdef DEBUG
        Label createdThisIsObject;
        masm.branchTestObject(Assembler::Equal, JSReturnOperand, &createdThisIsObject);
        masm.breakpoint();
        masm.bind(&createdThisIsObject);
#endif

        // Reset the register set from here on in.
        JS_ASSERT(JSReturnOperand == R0);
        regs = availableGeneralRegs(0);
        regs.take(R0);
        regs.take(ArgumentsRectifierReg);
        argcReg = regs.takeAny();

        // Restore saved argc so we can use it to calculate the address to save
        // the resulting this object to.
        masm.pop(argcReg);

        // Save "this" value back into pushed arguments on stack.  R0 can be clobbered after that.
        // Stack now looks like:
        //      [..., Callee, ThisV, Arg0V, ..., ArgNV, StubFrameHeader ]
        BaseIndex thisSlot(BaselineStackReg, argcReg, TimesEight, STUB_FRAME_SIZE);
        masm.storeValue(R0, thisSlot);

        // Restore the stub register from the baseline stub frame.
        masm.loadPtr(Address(BaselineStackReg, STUB_FRAME_SAVED_STUB_OFFSET), BaselineStubReg);

        // Reload callee script. Note that a GC triggered by CreateThis may
        // have destroyed the callee BaselineScript and IonScript. CreateThis is
        // safely repeatable though, so in this case we just leave the stub frame
        // and jump to the next stub.

        // Just need to load the script now.
        BaseIndex calleeSlot3(BaselineStackReg, argcReg, TimesEight,
                               sizeof(Value) + STUB_FRAME_SIZE);
        masm.loadValue(calleeSlot3, R0);
        callee = masm.extractObject(R0, ExtractTemp0);
        regs.add(R0);
        regs.takeUnchecked(callee);
        masm.loadPtr(Address(callee, JSFunction::offsetOfNativeOrScript()), callee);
        Register loadScratch = ArgumentsRectifierReg;
        masm.loadBaselineOrIonCode(callee, loadScratch, &failureLeaveStubFrame);
        // Release callee register, but don't add ExtractTemp0 back into the pool
        // ExtractTemp0 is used later, and if it's allocated to some other register at that
        // point, it will get clobbered when used.
        if (callee != ExtractTemp0)
            regs.add(callee);

        // Load the start of the target IonCode.
        code = regs.takeAny();
        masm.loadPtr(Address(callee, IonCode::offsetOfCode()), code);
        if (canUseTailCallReg)
            regs.addUnchecked(BaselineTailCallReg);
    }
    Register scratch = regs.takeAny();

    // Values are on the stack left-to-right. Calling convention wants them
    // right-to-left so duplicate them on the stack in reverse order.
    // |this| and callee are pushed last.
    pushCallArguments(masm, regs, argcReg);

    // The callee is on top of the stack. Pop and unbox it.
    ValueOperand val = regs.takeAnyValue();
    masm.popValue(val);
    callee = masm.extractObject(val, ExtractTemp0);

    EmitCreateStubFrameDescriptor(masm, scratch);

    // Note that we use Push, not push, so that callIon will align the stack
    // properly on ARM.
    masm.Push(argcReg);
    masm.Push(callee);
    masm.Push(scratch);

    // Handle arguments underflow.
    Label noUnderflow;
    masm.load16ZeroExtend(Address(callee, offsetof(JSFunction, nargs)), callee);
    masm.branch32(Assembler::AboveOrEqual, argcReg, callee, &noUnderflow);
    {
        // Call the arguments rectifier.
        JS_ASSERT(ArgumentsRectifierReg != code);
        JS_ASSERT(ArgumentsRectifierReg != argcReg);

        IonCode *argumentsRectifier =
            cx->compartment->ionCompartment()->getArgumentsRectifier(SequentialExecution);

        masm.movePtr(ImmGCPtr(argumentsRectifier), code);
        masm.loadPtr(Address(code, IonCode::offsetOfCode()), code);
        masm.mov(argcReg, ArgumentsRectifierReg);
    }

    masm.bind(&noUnderflow);

    // If needed, update SPS Profiler frame entry before and after call.
    {
        Label skipProfilerUpdate;

        // Need to avoid using ArgumentsRectifierReg and code register.
        GeneralRegisterSet availRegs = availableGeneralRegs(0);
        availRegs.take(ArgumentsRectifierReg);
        availRegs.take(code);
        Register scratch = availRegs.takeAny();
        Register pcIdx = availRegs.takeAny();

        // Check if profiling is enabled.
        guardProfilingEnabled(masm, scratch, &skipProfilerUpdate);

        // Update profiling entry before leaving function.
        JS_ASSERT(kind == ICStub::Call_Scripted || kind == ICStub::Call_AnyScripted);
        if (kind == ICStub::Call_Scripted)
            masm.load32(Address(BaselineStubReg, ICCall_Scripted::offsetOfPCOffset()), pcIdx);
        else
            masm.load32(Address(BaselineStubReg, ICCall_AnyScripted::offsetOfPCOffset()), pcIdx);
        masm.spsUpdatePCIdx(&cx->runtime->spsProfiler, pcIdx, scratch);

        masm.bind(&skipProfilerUpdate);
    }
    // Do call
    masm.callIon(code);

    // If this is a constructing call, and the callee returns a non-object, replace it with
    // the |this| object passed in.
    if (isConstructing_) {
        Label skipThisReplace;
        masm.branchTestObject(Assembler::Equal, JSReturnOperand, &skipThisReplace);

        // Current stack: [ ARGVALS..., ThisVal, ActualArgc, Callee, Descriptor ]
        masm.loadValue(Address(BaselineStackReg, 3*sizeof(size_t)), JSReturnOperand);
#ifdef DEBUG
        masm.branchTestObject(Assembler::Equal, JSReturnOperand, &skipThisReplace);
        masm.breakpoint();
#endif
        masm.bind(&skipThisReplace);
    }

    leaveStubFrame(masm, true);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Leave stub frame and restore argc for the next stub.
    masm.bind(&failureLeaveStubFrame);
    leaveStubFrame(masm, false);
    if (argcReg != R0.scratchReg())
        masm.mov(argcReg, R0.scratchReg());

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICCall_Native::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;
    GeneralRegisterSet regs(availableGeneralRegs(0));

    Register argcReg = R0.scratchReg();
    regs.take(argcReg);
    regs.takeUnchecked(BaselineTailCallReg);

    // Load the callee in R1.
    BaseIndex calleeSlot(BaselineStackReg, argcReg, TimesEight, ICStackValueOffset + sizeof(Value));
    masm.loadValue(calleeSlot, R1);
    regs.take(R1);

    masm.branchTestObject(Assembler::NotEqual, R1, &failure);

    // Ensure callee matches this stub's callee.
    Register callee = masm.extractObject(R1, ExtractTemp0);
    Address expectedCallee(BaselineStubReg, ICCall_Native::offsetOfCallee());
    masm.branchPtr(Assembler::NotEqual, expectedCallee, callee, &failure);

    regs.add(R1);
    regs.takeUnchecked(callee);

    // Push a stub frame so that we can perform a non-tail call.
    // Note that this leaves the return address in TailCallReg.
    enterStubFrame(masm, regs.getAny());

    // Values are on the stack left-to-right. Calling convention wants them
    // right-to-left so duplicate them on the stack in reverse order.
    // |this| and callee are pushed last.
    pushCallArguments(masm, regs, argcReg);

    if (isConstructing_) {
        // Stack looks like: [ ..., Arg0Val, ThisVal, CalleeVal ]
        // Replace ThisVal with MagicValue(JS_IS_CONSTRUCTING)
        masm.storeValue(MagicValue(JS_IS_CONSTRUCTING), Address(BaselineStackReg, sizeof(Value)));
    }

    masm.checkStackAlignment();

    // Native functions have the signature:
    //
    //    bool (*)(JSContext *, unsigned, Value *vp)
    //
    // Where vp[0] is space for callee/return value, vp[1] is |this|, and vp[2] onward
    // are the function arguments.

    // Initialize vp.
    Register vpReg = regs.takeAny();
    masm.movePtr(StackPointer, vpReg);

    // Construct a native exit frame.
    masm.push(argcReg);

    Register scratch = regs.takeAny();
    EmitCreateStubFrameDescriptor(masm, scratch);
    masm.push(scratch);
    masm.push(BaselineTailCallReg);
    masm.enterFakeExitFrame();

    // If needed, update SPS Profiler frame entry.  At this point, BaselineTailCallReg
    // and scratch can be clobbered.
    {
        Label skipProfilerUpdate;
        Register pcIdx = BaselineTailCallReg;
        guardProfilingEnabled(masm, scratch, &skipProfilerUpdate);

        masm.load32(Address(BaselineStubReg, ICCall_Native::offsetOfPCOffset()), pcIdx);
        masm.spsUpdatePCIdx(&cx->runtime->spsProfiler, pcIdx, scratch);

        masm.bind(&skipProfilerUpdate);
    }
    // Execute call.
    masm.setupUnalignedABICall(3, scratch);
    masm.loadJSContext(scratch);
    masm.passABIArg(scratch);
    masm.passABIArg(argcReg);
    masm.passABIArg(vpReg);
    masm.callWithABI(Address(callee, JSFunction::offsetOfNativeOrScript()));

    // Test for failure.
    Label success, exception;
    masm.branchTest32(Assembler::Zero, ReturnReg, ReturnReg, &exception);

    // Load the return value into R0.
    masm.loadValue(Address(StackPointer, IonNativeExitFrameLayout::offsetOfResult()), R0);

    leaveStubFrame(masm);

    // Enter type monitor IC to type-check result.
    EmitEnterTypeMonitorIC(masm);

    // Handle exception case.
    masm.bind(&exception);
    masm.handleException();

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

bool
ICTableSwitch::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label isInt32, notInt32, outOfRange;
    Register scratch = R1.scratchReg();

    masm.branchTestInt32(Assembler::NotEqual, R0, &notInt32);

    Register key = masm.extractInt32(R0, ExtractTemp0);

    masm.bind(&isInt32);

    masm.load32(Address(BaselineStubReg, offsetof(ICTableSwitch, min_)), scratch);
    masm.sub32(scratch, key);
    masm.branch32(Assembler::BelowOrEqual,
                  Address(BaselineStubReg, offsetof(ICTableSwitch, length_)), key, &outOfRange);

    masm.loadPtr(Address(BaselineStubReg, offsetof(ICTableSwitch, table_)), scratch);
    masm.loadPtr(BaseIndex(scratch, key, ScalePointer), scratch);

    EmitChangeICReturnAddress(masm, scratch);
    EmitReturnFromIC(masm);

    masm.bind(&notInt32);

    masm.branchTestDouble(Assembler::NotEqual, R0, &outOfRange);
    masm.unboxDouble(R0, FloatReg0);

    // N.B. -0 === 0, so convert -0 to a 0 int32.
    masm.convertDoubleToInt32(FloatReg0, key, &outOfRange, /* negativeZeroCheck = */ false);
    masm.jump(&isInt32);

    masm.bind(&outOfRange);

    masm.loadPtr(Address(BaselineStubReg, offsetof(ICTableSwitch, defaultTarget_)), scratch);

    EmitChangeICReturnAddress(masm, scratch);
    EmitReturnFromIC(masm);
    return true;
}

ICStub *
ICTableSwitch::Compiler::getStub(ICStubSpace *space)
{
    IonCode *code = getStubCode();
    if (!code)
        return NULL;

    jsbytecode *pc = pc_;
    pc += JUMP_OFFSET_LEN;
    int32_t low = GET_JUMP_OFFSET(pc);
    pc += JUMP_OFFSET_LEN;
    int32_t high = GET_JUMP_OFFSET(pc);
    int32_t length = high - low + 1;
    pc += JUMP_OFFSET_LEN;

    void **table = (void**) space->alloc(sizeof(void*) * length);
    if (!table)
        return NULL;

    jsbytecode *defaultpc = pc_ + GET_JUMP_OFFSET(pc_);

    for (int32_t i = 0; i < length; i++) {
        int32_t off = GET_JUMP_OFFSET(pc);
        if (off)
            table[i] = pc_ + off;
        else
            table[i] = defaultpc;
        pc += JUMP_OFFSET_LEN;
    }

    return ICTableSwitch::New(space, code, table, low, length, defaultpc);
}

void
ICTableSwitch::fixupJumpTable(HandleScript script, BaselineScript *baseline)
{
    defaultTarget_ = baseline->nativeCodeForPC(script, (jsbytecode *) defaultTarget_);

    for (int32_t i = 0; i < length_; i++)
        table_[i] = baseline->nativeCodeForPC(script, (jsbytecode *) table_[i]);
}

//
// IteratorNew_Fallback
//

static bool
DoIteratorNewFallback(JSContext *cx, BaselineFrame *frame, ICIteratorNew_Fallback *stub,
                      HandleValue value, MutableHandleValue res)
{
    jsbytecode *pc = stub->icEntry()->pc(frame->script());
    FallbackICSpew(cx, stub, "IteratorNew");

    uint8_t flags = GET_UINT8(pc);
    res.set(value);
    return ValueToIterator(cx, flags, res);
}

typedef bool (*DoIteratorNewFallbackFn)(JSContext *, BaselineFrame *, ICIteratorNew_Fallback *,
                                        HandleValue, MutableHandleValue);
static const VMFunction DoIteratorNewFallbackInfo =
    FunctionInfo<DoIteratorNewFallbackFn>(DoIteratorNewFallback, PopValues(1));

bool
ICIteratorNew_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    // Sync stack for the decompiler.
    masm.pushValue(R0);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoIteratorNewFallbackInfo, masm);
}

//
// IteratorMore_Fallback
//

static bool
DoIteratorMoreFallback(JSContext *cx, BaselineFrame *frame, ICIteratorMore_Fallback *stub,
                       HandleValue iterValue, MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "IteratorMore");

    bool cond;
    if (!IteratorMore(cx, &iterValue.toObject(), &cond, res))
        return false;
    res.setBoolean(cond);

    if (iterValue.toObject().isPropertyIterator() && !stub->hasStub(ICStub::IteratorMore_Native)) {
        ICIteratorMore_Native::Compiler compiler(cx);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(frame->script()));
        if (!newStub)
            return false;
        stub->addNewStub(newStub);
    }

    return true;
}

typedef bool (*DoIteratorMoreFallbackFn)(JSContext *, BaselineFrame *, ICIteratorMore_Fallback *,
                                         HandleValue, MutableHandleValue);
static const VMFunction DoIteratorMoreFallbackInfo =
    FunctionInfo<DoIteratorMoreFallbackFn>(DoIteratorMoreFallback);

bool
ICIteratorMore_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoIteratorMoreFallbackInfo, masm);
}

//
// IteratorMore_Native
//

bool
ICIteratorMore_Native::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;

    Register obj = masm.extractObject(R0, ExtractTemp0);

    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register nativeIterator = regs.takeAny();
    Register scratch = regs.takeAny();

    masm.branchTestObjClass(Assembler::NotEqual, obj, scratch,
                            &PropertyIteratorObject::class_, &failure);
    masm.loadObjPrivate(obj, JSObject::ITER_CLASS_NFIXED_SLOTS, nativeIterator);

    masm.branchTest32(Assembler::NonZero, Address(nativeIterator, offsetof(NativeIterator, flags)),
                      Imm32(JSITER_FOREACH), &failure);

    // Set output to true if props_cursor < props_end.
    masm.loadPtr(Address(nativeIterator, offsetof(NativeIterator, props_end)), scratch);
    masm.cmpPtr(Address(nativeIterator, offsetof(NativeIterator, props_cursor)), scratch);
    masm.emitSet(Assembler::LessThan, scratch);

    masm.tagValue(JSVAL_TYPE_BOOLEAN, scratch, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// IteratorNext_Fallback
//

static bool
DoIteratorNextFallback(JSContext *cx, BaselineFrame *frame, ICIteratorNext_Fallback *stub,
                       HandleValue iterValue, MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "IteratorNext");

    RootedObject iteratorObject(cx, &iterValue.toObject());
    if (!IteratorNext(cx, iteratorObject, res))
        return false;

    if (iteratorObject->isPropertyIterator() && !stub->hasStub(ICStub::IteratorNext_Native)) {
        ICIteratorNext_Native::Compiler compiler(cx);
        ICStub *newStub = compiler.getStub(compiler.getStubSpace(frame->script()));
        if (!newStub)
            return false;
        stub->addNewStub(newStub);
    }

    return true;
}

typedef bool (*DoIteratorNextFallbackFn)(JSContext *, BaselineFrame *, ICIteratorNext_Fallback *,
                                         HandleValue, MutableHandleValue);
static const VMFunction DoIteratorNextFallbackInfo =
    FunctionInfo<DoIteratorNextFallbackFn>(DoIteratorNextFallback);

bool
ICIteratorNext_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoIteratorNextFallbackInfo, masm);
}

//
// IteratorNext_Native
//

bool
ICIteratorNext_Native::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure;

    Register obj = masm.extractObject(R0, ExtractTemp0);

    GeneralRegisterSet regs(availableGeneralRegs(1));
    Register nativeIterator = regs.takeAny();
    Register scratch = regs.takeAny();

    masm.branchTestObjClass(Assembler::NotEqual, obj, scratch,
                            &PropertyIteratorObject::class_, &failure);
    masm.loadObjPrivate(obj, JSObject::ITER_CLASS_NFIXED_SLOTS, nativeIterator);

    masm.branchTest32(Assembler::NonZero, Address(nativeIterator, offsetof(NativeIterator, flags)),
                      Imm32(JSITER_FOREACH), &failure);

    // Get cursor, next string.
    masm.loadPtr(Address(nativeIterator, offsetof(NativeIterator, props_cursor)), scratch);
    masm.loadPtr(Address(scratch, 0), scratch);

    // Increase the cursor.
    masm.addPtr(Imm32(sizeof(JSString *)),
                Address(nativeIterator, offsetof(NativeIterator, props_cursor)));

    masm.tagValue(JSVAL_TYPE_STRING, scratch, R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

//
// IteratorClose_Fallback
//

static bool
DoIteratorCloseFallback(JSContext *cx, ICIteratorClose_Fallback *stub, HandleValue iterValue)
{
    FallbackICSpew(cx, stub, "IteratorClose");

    RootedObject iteratorObject(cx, &iterValue.toObject());
    return CloseIterator(cx, iteratorObject);
}

typedef bool (*DoIteratorCloseFallbackFn)(JSContext *, ICIteratorClose_Fallback *, HandleValue);
static const VMFunction DoIteratorCloseFallbackInfo =
    FunctionInfo<DoIteratorCloseFallbackFn>(DoIteratorCloseFallback);

bool
ICIteratorClose_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);

    return tailCallVM(DoIteratorCloseFallbackInfo, masm);
}

//
// InstanceOf_Fallback
//

static bool
DoInstanceOfFallback(JSContext *cx, ICInstanceOf_Fallback *stub,
                     HandleValue lhs, HandleValue rhs,
                     MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "InstanceOf");

    if (!rhs.isObject()) {
        js_ReportValueError(cx, JSMSG_BAD_INSTANCEOF_RHS, -1, rhs, NullPtr());
        return false;
    }

    RootedObject obj(cx, &rhs.toObject());

    JSBool cond = false;
    if (!HasInstance(cx, obj, lhs, &cond))
        return false;

    res.setBoolean(cond);
    return true;
}

typedef bool (*DoInstanceOfFallbackFn)(JSContext *, ICInstanceOf_Fallback *, HandleValue, HandleValue,
                                       MutableHandleValue);
static const VMFunction DoInstanceOfFallbackInfo =
    FunctionInfo<DoInstanceOfFallbackFn>(DoInstanceOfFallback, PopValues(2));

bool
ICInstanceOf_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    // Sync stack for the decompiler.
    masm.pushValue(R0);
    masm.pushValue(R1);

    masm.pushValue(R1);
    masm.pushValue(R0);
    masm.push(BaselineStubReg);

    return tailCallVM(DoInstanceOfFallbackInfo, masm);
}

//
// TypeOf_Fallback
//

static bool
DoTypeOfFallback(JSContext *cx, BaselineFrame *frame, ICTypeOf_Fallback *stub, HandleValue val,
                 MutableHandleValue res)
{
    FallbackICSpew(cx, stub, "TypeOf");
    JSType type = JS_TypeOfValue(cx, val);
    RootedString string(cx, TypeName(type, cx));

    res.setString(string);

    JS_ASSERT(type != JSTYPE_NULL);
    if (type != JSTYPE_OBJECT && type != JSTYPE_FUNCTION) {
        // Create a new TypeOf stub.
        IonSpew(IonSpew_BaselineIC, "  Generating TypeOf stub for JSType (%d)", (int) type);
        ICTypeOf_Typed::Compiler compiler(cx, type, string);
        ICStub *typeOfStub = compiler.getStub(compiler.getStubSpace(frame->script()));
        if (!typeOfStub)
            return false;
        stub->addNewStub(typeOfStub);
    }

    return true;
}

typedef bool (*DoTypeOfFallbackFn)(JSContext *, BaselineFrame *frame, ICTypeOf_Fallback *,
                                   HandleValue, MutableHandleValue);
static const VMFunction DoTypeOfFallbackInfo =
    FunctionInfo<DoTypeOfFallbackFn>(DoTypeOfFallback);

bool
ICTypeOf_Fallback::Compiler::generateStubCode(MacroAssembler &masm)
{
    EmitRestoreTailCallReg(masm);

    masm.pushValue(R0);
    masm.push(BaselineStubReg);
    masm.pushBaselineFramePtr(BaselineFrameReg, R0.scratchReg());

    return tailCallVM(DoTypeOfFallbackInfo, masm);
}

bool
ICTypeOf_Typed::Compiler::generateStubCode(MacroAssembler &masm)
{
    JS_ASSERT(type_ != JSTYPE_NULL);
    JS_ASSERT(type_ != JSTYPE_FUNCTION);
    JS_ASSERT(type_ != JSTYPE_OBJECT);

    Label failure;
    switch(type_) {
      case JSTYPE_VOID:
        masm.branchTestUndefined(Assembler::NotEqual, R0, &failure);
        break;

      case JSTYPE_STRING:
        masm.branchTestString(Assembler::NotEqual, R0, &failure);
        break;

      case JSTYPE_NUMBER:
        masm.branchTestNumber(Assembler::NotEqual, R0, &failure);
        break;

      case JSTYPE_BOOLEAN:
        masm.branchTestBoolean(Assembler::NotEqual, R0, &failure);
        break;

      default:
        JS_NOT_REACHED("Unexpected type");
    }

    masm.movePtr(ImmGCPtr(typeString_), R0.scratchReg());
    masm.tagValue(JSVAL_TYPE_STRING, R0.scratchReg(), R0);
    EmitReturnFromIC(masm);

    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}

} // namespace ion
} // namespace js
