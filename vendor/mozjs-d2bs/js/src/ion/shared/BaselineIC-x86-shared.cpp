/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=99:
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ion/BaselineJIT.h"
#include "ion/BaselineIC.h"
#include "ion/BaselineCompiler.h"
#include "ion/BaselineHelpers.h"
#include "ion/IonLinker.h"

using namespace js;
using namespace js::ion;

bool
ICCompare_Double::Compiler::generateStubCode(MacroAssembler &masm)
{
    Label failure, isNaN;
    masm.ensureDouble(R0, FloatReg0, &failure);
    masm.ensureDouble(R1, FloatReg1, &failure);

    Register dest = R0.scratchReg();

    Assembler::DoubleCondition cond = JSOpToDoubleCondition(op);
    masm.compareDouble(cond, FloatReg0, FloatReg1);
    masm.setCC(Assembler::ConditionFromDoubleCondition(cond), dest);
    masm.movzxbl(dest, dest);

    // Check for NaN.
    masm.j(Assembler::Parity, &isNaN);

    masm.tagValue(JSVAL_TYPE_BOOLEAN, dest, R0);
    EmitReturnFromIC(masm);

    Assembler::NaNCond nanCond = Assembler::NaNCondFromDoubleCondition(cond);
    JS_ASSERT(nanCond == Assembler::NaN_IsTrue || nanCond == Assembler::NaN_IsFalse);

    masm.bind(&isNaN);
    masm.moveValue(BooleanValue(nanCond == Assembler::NaN_IsTrue), R0);
    EmitReturnFromIC(masm);

    // Failure case - jump to next stub
    masm.bind(&failure);
    EmitStubGuardFailure(masm);
    return true;
}
