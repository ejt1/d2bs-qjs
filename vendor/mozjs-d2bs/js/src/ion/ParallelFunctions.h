/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jsion_parallel_functions_h__
#define jsion_parallel_functions_h__

#include "vm/ThreadPool.h"
#include "vm/ForkJoin.h"
#include "gc/Heap.h"

namespace js {
namespace ion {

ForkJoinSlice *ParForkJoinSlice();
JSObject *ParNewGCThing(gc::AllocKind allocKind);
bool ParWriteGuard(ForkJoinSlice *context, JSObject *object);
void ParBailout(uint32_t id);
bool ParCheckOverRecursed(ForkJoinSlice *slice);
bool ParCheckInterrupt(ForkJoinSlice *context);

void ParDumpValue(Value *v);

// We pass the arguments to ParPush in a structure because, in code
// gen, it is convenient to store them on the stack to avoid
// constraining the reg alloc for the slow path.
struct ParPushArgs {
    JSObject *object;
    Value value;
};

// Extends the given object with the given value (like `Array.push`).
// Returns NULL on failure or else `args->object`, which is convenient
// during code generation.
JSObject* ParPush(ParPushArgs *args);

// Extends the given array with `length` new holes.  Returns NULL on
// failure or else `array`, which is convenient during code
// generation.
JSObject *ParExtendArray(ForkJoinSlice *slice, JSObject *array, uint32_t length);

enum ParCompareResult {
    ParCompareNe = false,
    ParCompareEq = true,
    ParCompareUnknown = 2
};
ParCompareResult ParCompareStrings(JSString *str1, JSString *str2);

void ParallelAbort(JSScript *script);

void TraceLIR(uint32_t bblock, uint32_t lir, uint32_t execModeInt,
              const char *lirOpName, const char *mirOpName,
              JSScript *script, jsbytecode *pc);

void ParCallToUncompiledScript(JSFunction *func);

} // namespace ion
} // namespace js

#endif // jsion_parallel_functions_h__
