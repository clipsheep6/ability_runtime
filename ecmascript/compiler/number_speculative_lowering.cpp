/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ecmascript/compiler/gate_meta_data.h"
#include "ecmascript/compiler/number_gate_info.h"
#include "ecmascript/compiler/type.h"
#include "ecmascript/compiler/type_mcr_lowering.h"
#include "ecmascript/compiler/builtins_lowering.h"
#include "ecmascript/compiler/new_object_stub_builder.h"
#include "ecmascript/compiler/number_speculative_lowering.h"
#include "ecmascript/deoptimizer/deoptimizer.h"
#include "ecmascript/js_arraybuffer.h"
#include "ecmascript/js_locale.h"
#include "ecmascript/js_native_pointer.h"

namespace panda::ecmascript::kungfu {

void NumberSpeculativeLowering::Run()
{
    std::vector<GateRef> gateList;
    acc_.GetAllGates(gateList);
    for (auto gate : gateList) {
        if (acc_.GetOpCode(gate) != OpCode::INDEX_CHECK) {
            VisitGate(gate);
        } else {
            checkedGates_.push_back(gate);
        }
    }
    for (auto check : checkedGates_) {
        VisitIndexCheck(check);
    }
}

void NumberSpeculativeLowering::VisitGate(GateRef gate)
{
    OpCode op = acc_.GetOpCode(gate);
    switch (op) {
        case OpCode::TYPED_BINARY_OP: {
            VisitTypedBinaryOp(gate);
            break;
        }
        case OpCode::TYPED_UNARY_OP: {
            VisitTypedUnaryOp(gate);
            break;
        }
        case OpCode::TYPED_CONDITION_JUMP: {
            VisitTypedConditionJump(gate);
            break;
        }
        case OpCode::VALUE_SELECTOR: {
            VisitPhi(gate);
            break;
        }
        case OpCode::CONSTANT: {
            VisitConstant(gate);
            break;
        }
        case OpCode::TYPED_CALL_BUILTIN: {
            VisitCallBuiltins(gate);
            break;
        }
        case OpCode::LOAD_ELEMENT: {
            VisitLoadElement(gate);
            break;
        }
        case OpCode::LOAD_ARRAY_LENGTH:
        case OpCode::LOAD_TYPED_ARRAY_LENGTH: {
            VisitLoadArrayLength(gate);
            break;
        }
        default:
            break;
    }
}

void NumberSpeculativeLowering::VisitTypedBinaryOp(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    if (acc_.HasNumberType(gate)) {
        VisitNumberBinaryOp(gate);
    } else {
        [[maybe_unused]] GateRef left = acc_.GetValueIn(gate, 0);
        [[maybe_unused]] GateRef right = acc_.GetValueIn(gate, 1);
        ASSERT(acc_.IsConstantUndefined(left) || acc_.IsConstantUndefined(right));
        ASSERT(acc_.GetTypedBinaryOp(gate) == TypedBinOp::TYPED_STRICTEQ);
        VisitUndefinedStrictEq(gate);
    }
}

void NumberSpeculativeLowering::VisitNumberBinaryOp(GateRef gate)
{
    TypedBinOp Op = acc_.GetTypedBinaryOp(gate);
    switch (Op) {
        case TypedBinOp::TYPED_ADD: {
            VisitNumberCalculate<TypedBinOp::TYPED_ADD>(gate);
            break;
        }
        case TypedBinOp::TYPED_SUB: {
            VisitNumberCalculate<TypedBinOp::TYPED_SUB>(gate);
            break;
        }
        case TypedBinOp::TYPED_MUL: {
            VisitNumberCalculate<TypedBinOp::TYPED_MUL>(gate);
            break;
        }
        case TypedBinOp::TYPED_LESS: {
            VisitNumberCompare<TypedBinOp::TYPED_LESS>(gate);
            break;
        }
        case TypedBinOp::TYPED_LESSEQ: {
            VisitNumberCompare<TypedBinOp::TYPED_LESSEQ>(gate);
            break;
        }
        case TypedBinOp::TYPED_GREATER: {
            VisitNumberCompare<TypedBinOp::TYPED_GREATER>(gate);
            break;
        }
        case TypedBinOp::TYPED_GREATEREQ: {
            VisitNumberCompare<TypedBinOp::TYPED_GREATEREQ>(gate);
            break;
        }
        case TypedBinOp::TYPED_EQ: {
            VisitNumberCompare<TypedBinOp::TYPED_EQ>(gate);
            break;
        }
        case TypedBinOp::TYPED_NOTEQ: {
            VisitNumberCompare<TypedBinOp::TYPED_NOTEQ>(gate);
            break;
        }
        case TypedBinOp::TYPED_STRICTEQ: {
            VisitNumberCompare<TypedBinOp::TYPED_STRICTEQ>(gate);
            break;
        }
        case TypedBinOp::TYPED_SHL: {
            VisitNumberShift<TypedBinOp::TYPED_SHL>(gate);
            break;
        }
        case TypedBinOp::TYPED_SHR: {
            VisitNumberShift<TypedBinOp::TYPED_SHR>(gate);
            break;
        }
        case TypedBinOp::TYPED_ASHR: {
            VisitNumberShift<TypedBinOp::TYPED_ASHR>(gate);
            break;
        }
        case TypedBinOp::TYPED_AND: {
            VisitNumberLogical<TypedBinOp::TYPED_AND>(gate);
            break;
        }
        case TypedBinOp::TYPED_OR: {
            VisitNumberLogical<TypedBinOp::TYPED_OR>(gate);
            break;
        }
        case TypedBinOp::TYPED_XOR: {
            VisitNumberLogical<TypedBinOp::TYPED_XOR>(gate);
            break;
        }
        case TypedBinOp::TYPED_DIV: {
            VisitNumberDiv(gate);
            break;
        }
        case TypedBinOp::TYPED_MOD: {
            VisitNumberMod<TypedBinOp::TYPED_MOD>(gate);
            break;
        }
        default:
            break;
    }
}

void NumberSpeculativeLowering::VisitTypedUnaryOp(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    TypedUnOp Op = acc_.GetTypedUnAccessor(gate).GetTypedUnOp();
    switch (Op) {
        case TypedUnOp::TYPED_INC: {
            VisitNumberMonocular<TypedUnOp::TYPED_INC>(gate);
            return;
        }
        case TypedUnOp::TYPED_DEC: {
            VisitNumberMonocular<TypedUnOp::TYPED_DEC>(gate);
            return;
        }
        case TypedUnOp::TYPED_NEG: {
            VisitNumberMonocular<TypedUnOp::TYPED_NEG>(gate);
            return;
        }
        case TypedUnOp::TYPED_ISFALSE: {
            VisitIsTrueOrFalse(gate, false);
            return;
        }
        case TypedUnOp::TYPED_ISTRUE: {
            VisitIsTrueOrFalse(gate, true);
            return;
        }
        case TypedUnOp::TYPED_NOT: {
            VisitNumberNot(gate);
            return;
        }
        default:
            break;
    }
}

void NumberSpeculativeLowering::VisitTypedConditionJump(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    GateType type = acc_.GetTypedJumpAccessor(gate).GetTypeValue();
    if (type.IsBooleanType()) {
        VisitBooleanJump(gate);
    } else {
        UNREACHABLE();
    }
}

template<TypedBinOp Op>
void NumberSpeculativeLowering::VisitNumberCalculate(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType gateType = acc_.GetGateType(gate);
    PGOSampleType sampleType = acc_.GetTypedBinaryType(gate);
    if (sampleType.IsNumber()) {
        if (sampleType.IsInt()) {
            gateType = GateType::IntType();
        } else {
            gateType = GateType::DoubleType();
        }
    }
    GateRef result = Circuit::NullGate();
    if (gateType.IsIntType()) {
        result = CalculateInts<Op>(left, right);    // int op int
        UpdateRange(result, GetRange(gate));
        acc_.SetMachineType(gate, MachineType::I32);
    } else {
        result = CalculateDoubles<Op>(left, right); // float op float
        acc_.SetMachineType(gate, MachineType::F64);
    }
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

template<TypedBinOp Op>
void NumberSpeculativeLowering::VisitNumberCompare(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType leftType = acc_.GetLeftType(gate);
    GateType rightType = acc_.GetRightType(gate);
    PGOSampleType sampleType = acc_.GetTypedBinaryType(gate);
    if (sampleType.IsNumber()) {
        if (sampleType.IsInt()) {
            leftType = GateType::IntType();
            rightType = GateType::IntType();
        } else {
            leftType = GateType::NumberType();
            rightType = GateType::NumberType();
        }
    }
    GateRef result = Circuit::NullGate();
    if (leftType.IsIntType() && rightType.IsIntType()) {
        result = CompareInts<Op>(left, right);  // int op int
    } else {
        result = CompareDoubles<Op>(left, right);   // float op float
    }
    acc_.SetMachineType(gate, MachineType::I1);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

template<TypedBinOp Op>
void NumberSpeculativeLowering::VisitNumberShift(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateRef result = ShiftInts<Op>(left, right);  // int op int
    UpdateRange(result, GetRange(gate));
    acc_.SetMachineType(gate, MachineType::I32);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

template<TypedBinOp Op>
void NumberSpeculativeLowering::VisitNumberLogical(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateRef result = LogicalInts<Op>(left, right);  // int op int
    UpdateRange(result, GetRange(gate));
    acc_.SetMachineType(gate, MachineType::I32);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void NumberSpeculativeLowering::VisitNumberDiv(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType gateType = acc_.GetGateType(gate);
    PGOSampleType sampleType = acc_.GetTypedBinaryType(gate);
    if (sampleType.IsNumber()) {
        if (sampleType.IsInt()) {
            gateType = GateType::IntType();
        } else {
            gateType = GateType::DoubleType();
        }
    }
    GateRef result = Circuit::NullGate();
    if (gateType.IsIntType()) {
        result = builder_.Int32DivWithCheck(left, right);
        acc_.SetMachineType(gate, MachineType::I32);
    } else {
        builder_.Float64CheckRightIsZero(right);
        result = builder_.BinaryArithmetic(circuit_->Fdiv(), MachineType::F64, left, right);
        acc_.SetMachineType(gate, MachineType::F64);
    }
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

template<TypedBinOp Op>
void NumberSpeculativeLowering::VisitNumberMod(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType gateType = acc_.GetGateType(gate);
    PGOSampleType sampleType = acc_.GetTypedBinaryType(gate);
    if (sampleType.IsNumber()) {
        if (sampleType.IsInt()) {
            gateType = GateType::IntType();
        } else {
            gateType = GateType::DoubleType();
        }
    }
    GateRef result = Circuit::NullGate();
    if (gateType.IsIntType()) {
        builder_.Int32CheckRightIsZero(right);
        result = CalculateInts<Op>(left, right);
        UpdateRange(result, GetRange(gate));
        acc_.SetMachineType(gate, MachineType::I32);
    } else {
        GateRef glue = acc_.GetGlueFromArgList();
        result = builder_.CallNGCRuntime(glue, RTSTUB_ID(FloatMod),
            Gate::InvalidGateRef, {left, right}, Circuit::NullGate());
        acc_.SetMachineType(gate, MachineType::F64);
    }
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

template<TypedUnOp Op>
void NumberSpeculativeLowering::VisitNumberMonocular(GateRef gate)
{
    TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
    GateType type = accessor.GetTypeValue();
    ASSERT(type.IsNumberType());
    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef result = Circuit::NullGate();
    if (type.IsIntType()) {
        if (Op == TypedUnOp::TYPED_NEG) {
            builder_.ValueCheckNegOverflow(value);
        }
        result = MonocularInt<Op>(value);
        UpdateRange(result, GetRange(gate));
        acc_.SetMachineType(gate, MachineType::I32);
    } else {
        result = MonocularDouble<Op>(value);
        acc_.SetMachineType(gate, MachineType::F64);
    }
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void NumberSpeculativeLowering::VisitNumberNot(GateRef gate)
{
    ASSERT(TypedUnaryAccessor(acc_.TryGetValue(gate)).GetTypeValue().IsNumberType());
    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef result = builder_.Int32Not(value);
    UpdateRange(result, GetRange(gate));
    acc_.SetMachineType(gate, MachineType::I32);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void NumberSpeculativeLowering::VisitIsTrueOrFalse(GateRef gate, bool flag)
{
    GateRef value = acc_.GetValueIn(gate, 0);
    GateRef result = Circuit::NullGate();
    if (!flag) {
        result = builder_.BoolNot(value);
    } else {
        result = value;
    }
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void NumberSpeculativeLowering::VisitBooleanJump(GateRef gate)
{
    TypedJumpOp jumpOp = acc_.GetTypedJumpAccessor(gate).GetTypedJumpOp();
    ASSERT((jumpOp == TypedJumpOp::TYPED_JEQZ) || (jumpOp == TypedJumpOp::TYPED_JNEZ));
    GateRef condition = acc_.GetValueIn(gate, 0);
    if (jumpOp == TypedJumpOp::TYPED_JEQZ) {
        condition = builder_.BoolNot(condition);
    }
    GateRef ifBranch = builder_.Branch(acc_.GetState(gate), condition);
    acc_.ReplaceGate(gate, ifBranch, acc_.GetDep(gate), Circuit::NullGate());
}

void NumberSpeculativeLowering::VisitUndefinedStrictEq(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateRef result = builder_.Equal(left, right);
    acc_.SetMachineType(gate, MachineType::I1);
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), result);
}

void NumberSpeculativeLowering::VisitCallBuiltins(GateRef gate)
{
    auto valuesIn = acc_.GetNumValueIn(gate);
    auto idGate = acc_.GetValueIn(gate, valuesIn - 1);
    auto id = static_cast<BuiltinsStubCSigns::ID>(acc_.GetConstantValue(idGate));
    if (id != BUILTINS_STUB_ID(SQRT)) {
        return;
    }

    BuiltinLowering lowering(circuit_);
    lowering.LowerTypedSqrt(gate);
}

void NumberSpeculativeLowering::VisitConstant(GateRef gate)
{
    TypeInfo output = GetOutputType(gate);
    switch (output) {
        case TypeInfo::INT32: {
            int value = acc_.GetInt32FromConstant(gate);
            GateRef constGate = GetConstInt32(value);
            acc_.UpdateAllUses(gate, constGate);
            break;
        }
        case TypeInfo::FLOAT64: {
            double value = acc_.GetFloat64FromConstant(gate);
            acc_.UpdateAllUses(gate, builder_.Double(value));
            break;
        }
        default:
            break;
    }
}

void NumberSpeculativeLowering::VisitPhi(GateRef gate)
{
    TypeInfo output = GetOutputType(gate);
    switch (output) {
        case TypeInfo::INT1: {
            acc_.SetGateType(gate, GateType::NJSValue());
            acc_.SetMachineType(gate, MachineType::I1);
            break;
        }
        case TypeInfo::INT32: {
            acc_.SetGateType(gate, GateType::NJSValue());
            acc_.SetMachineType(gate, MachineType::I32);
            break;
        }
        case TypeInfo::FLOAT64: {
            acc_.SetGateType(gate, GateType::NJSValue());
            acc_.SetMachineType(gate, MachineType::F64);
            break;
        }
        default:
            break;
    }
}

void NumberSpeculativeLowering::VisitLoadArrayLength(GateRef gate)
{
    acc_.SetGateType(gate, GateType::NJSValue());
    acc_.SetMachineType(gate, MachineType::I32);
}

void NumberSpeculativeLowering::VisitLoadElement(GateRef gate)
{
    auto op = acc_.GetTypedLoadOp(gate);
    switch (op) {
        case TypedLoadOp::INT32ARRAY_LOAD_ELEMENT:
            acc_.SetMachineType(gate, MachineType::I32);
            break;
        case TypedLoadOp::FLOAT32ARRAY_LOAD_ELEMENT:
            acc_.SetMachineType(gate, MachineType::F64);
            break;
        case TypedLoadOp::FLOAT64ARRAY_LOAD_ELEMENT:
            acc_.SetMachineType(gate, MachineType::F64);
            break;
        default:
            break;
    }
    acc_.SetGateType(gate, GateType::NJSValue());
}

void NumberSpeculativeLowering::VisitIndexCheck(GateRef gate)
{
    auto type = acc_.GetParamGateType(gate);
    if (!tsManager_->IsArrayTypeKind(type)) {
        // return checked index value
        acc_.SetGateType(gate, GateType::NJSValue());
        acc_.SetMachineType(gate, MachineType::I32);
        return;
    }
    Environment env(gate, circuit_, &builder_);
    GateRef index = acc_.GetValueIn(gate, 1);
    if (!noCheck_) {
        GateRef length = acc_.GetValueIn(gate, 0);
        RangeInfo indexRange = GetRange(index);
        if (indexRange.GetMin() < 0) {
            builder_.NegativeIndexCheck(index);
        }
        builder_.LargeIndexCheck(index, length);
        // return checked index value
        acc_.SetGateType(gate, GateType::NJSValue());
        acc_.SetMachineType(gate, MachineType::I32);
    }
    acc_.ReplaceGate(gate, builder_.GetState(), builder_.GetDepend(), index);
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::CalculateInts(GateRef left, GateRef right)
{
    GateRef res = Circuit::NullGate();
    RangeInfo leftRange = GetRange(left);
    RangeInfo rightRange = GetRange(right);
    switch (Op) {
        case TypedBinOp::TYPED_ADD: {
            if (!leftRange.MaybeAddOverflowOrUnderflow(rightRange)) {
                return builder_.Int32Add(left, right, GateType::NJSValue());
            }
            res = builder_.AddWithOverflow(left, right);
            break;
        }
        case TypedBinOp::TYPED_SUB: {
            if (!leftRange.MaybeSubOverflowOrUnderflow(rightRange)) {
                return builder_.Int32Sub(left, right, GateType::NJSValue());
            }
            res = builder_.SubWithOverflow(left, right);
            break;
        }
        case TypedBinOp::TYPED_MUL:
            res = builder_.MulWithOverflow(left, right);
            break;
        case TypedBinOp::TYPED_MOD: {
            return builder_.BinaryArithmetic(circuit_->Smod(), MachineType::I32, left, right);
            break;
        }
        default:
            break;
    }
    // DeoptCheckForOverFlow
    builder_.OverflowCheck(res);
    return builder_.ExtractValue(MachineType::I32, res, GetConstInt32(0));
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::CalculateDoubles(GateRef left, GateRef right)
{
    GateRef res = Circuit::NullGate();
    switch (Op) {
        case TypedBinOp::TYPED_ADD:
            res = builder_.DoubleAdd(left, right, GateType::NJSValue());
            break;
        case TypedBinOp::TYPED_SUB:
            res = builder_.DoubleSub(left, right, GateType::NJSValue());
            break;
        case TypedBinOp::TYPED_MUL:
            res = builder_.DoubleMul(left, right, GateType::NJSValue());
            break;
        default:
            break;
    }
    return res;
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::CompareInts(GateRef left, GateRef right)
{
    GateRef condition = Circuit::NullGate();
    switch (Op) {
        case TypedBinOp::TYPED_LESS:
            condition = builder_.Int32LessThan(left, right);
            break;
        case TypedBinOp::TYPED_LESSEQ:
            condition = builder_.Int32LessThanOrEqual(left, right);
            break;
        case TypedBinOp::TYPED_GREATER:
            condition = builder_.Int32GreaterThan(left, right);
            break;
        case TypedBinOp::TYPED_GREATEREQ:
            condition = builder_.Int32GreaterThanOrEqual(left, right);
            break;
        case TypedBinOp::TYPED_EQ:
        case TypedBinOp::TYPED_STRICTEQ:
            condition = builder_.Int32Equal(left, right);
            break;
        case TypedBinOp::TYPED_NOTEQ:
            condition = builder_.Int32NotEqual(left, right);
            break;
        default:
            break;
    }
    return condition;
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::CompareDoubles(GateRef left, GateRef right)
{
    GateRef condition = Circuit::NullGate();
    switch (Op) {
        case TypedBinOp::TYPED_LESS:
            condition = builder_.DoubleLessThan(left, right);
            break;
        case TypedBinOp::TYPED_LESSEQ:
            condition = builder_.DoubleLessThanOrEqual(left, right);
            break;
        case TypedBinOp::TYPED_GREATER:
            condition = builder_.DoubleGreaterThan(left, right);
            break;
        case TypedBinOp::TYPED_GREATEREQ:
            condition = builder_.DoubleGreaterThanOrEqual(left, right);
            break;
        case TypedBinOp::TYPED_EQ:
            condition = builder_.DoubleEqual(left, right);
            break;
        case TypedBinOp::TYPED_NOTEQ:
            condition = builder_.DoubleNotEqual(left, right);
            break;
        case TypedBinOp::TYPED_STRICTEQ: {
            GateRef leftNotNan = builder_.BoolNot(builder_.DoubleIsNAN(left));
            GateRef rightNotNan = builder_.BoolNot(builder_.DoubleIsNAN(right));
            GateRef doubleEqual = builder_.DoubleEqual(left, right);
            condition = builder_.BoolAnd(builder_.BoolAnd(leftNotNan, rightNotNan), doubleEqual);
            break;
        }
        default:
            break;
    }
    return condition;
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::ShiftInts(GateRef left, GateRef right)
{
    GateRef value = Circuit::NullGate();
    GateRef bitmask = GetConstInt32(0x1f); // 0x1f: bit mask of shift value
    GateRef shift = builder_.Int32And(right, bitmask, GateType::NJSValue());
    switch (Op) {
        case TypedBinOp::TYPED_SHL: {
            value = builder_.Int32LSL(left, shift, GateType::NJSValue());
            break;
        }
        case TypedBinOp::TYPED_SHR: {
            value = builder_.Int32LSR(left, shift, GateType::NJSValue());
            RangeInfo leftRange = GetRange(left);
            RangeInfo rightRange = GetRange(right);
            if (!leftRange.MaybeShrOverflow(rightRange)) {
                return value;
            }
            builder_.Int32UnsignedUpperBoundCheck(value, builder_.Int32(INT32_MAX));
            break;
        }
        case TypedBinOp::TYPED_ASHR: {
            value = builder_.Int32ASR(left, shift, GateType::NJSValue());
            break;
        }
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
            break;
    }
    return value;
}

template<TypedBinOp Op>
GateRef NumberSpeculativeLowering::LogicalInts(GateRef left, GateRef right)
{
    GateRef value = Circuit::NullGate();
    switch (Op) {
        case TypedBinOp::TYPED_AND: {
            value = builder_.Int32And(left, right, GateType::NJSValue());
            break;
        }
        case TypedBinOp::TYPED_OR: {
            value = builder_.Int32Or(left, right, GateType::NJSValue());
            break;
        }
        case TypedBinOp::TYPED_XOR: {
            value = builder_.Int32Xor(left, right, GateType::NJSValue());
            break;
        }
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
            break;
    }
    return value;
}

template<TypedUnOp Op>
GateRef NumberSpeculativeLowering::MonocularInt(GateRef value)
{
    GateRef res = Circuit::NullGate();
    switch (Op) {
        case TypedUnOp::TYPED_INC:
            res = CalculateInts<TypedBinOp::TYPED_ADD>(value, GetConstInt32(1));
            break;
        case TypedUnOp::TYPED_DEC:
            res = CalculateInts<TypedBinOp::TYPED_SUB>(value, GetConstInt32(1));
            break;
        case TypedUnOp::TYPED_NEG:
            res = builder_.Int32Sub(GetConstInt32(0), value, GateType::NJSValue());
            break;
        default:
            break;
    }
    return res;
}

template<TypedUnOp Op>
GateRef NumberSpeculativeLowering::MonocularDouble(GateRef value)
{
    GateRef res = Circuit::NullGate();
    switch (Op) {
        case TypedUnOp::TYPED_INC:
            res = builder_.DoubleAdd(value, builder_.Double(1));
            break;
        case TypedUnOp::TYPED_DEC:
            res = builder_.DoubleSub(value, builder_.Double(1));
            break;
        case TypedUnOp::TYPED_NEG:
            res = builder_.DoubleSub(builder_.Double(0), value);
            break;
        default:
            break;
    }
    return res;
}

void NumberSpeculativeLowering::UpdateRange(GateRef gate, const RangeInfo& range)
{
    auto id = acc_.GetId(gate);
    rangeInfos_.resize(id + 1, RangeInfo::ANY());
    rangeInfos_[id] = range;
}

RangeInfo NumberSpeculativeLowering::GetRange(GateRef gate) const
{
    ASSERT(!rangeInfos_[acc_.GetId(gate)].IsNone());
    return rangeInfos_[acc_.GetId(gate)];
}

GateRef NumberSpeculativeLowering::GetConstInt32(int32_t v)
{
    auto val = builder_.Int32(v);
    UpdateRange(val, RangeInfo(v, v));
    return val;
}
}  // namespace panda::ecmascript
