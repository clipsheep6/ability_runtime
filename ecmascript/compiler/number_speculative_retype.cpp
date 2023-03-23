/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ecmascript/compiler/number_speculative_retype.h"
#include "ecmascript/compiler/circuit_builder-inl.h"
#include "ecmascript/compiler/gate_meta_data.h"
#include "ecmascript/compiler/number_gate_info.h"
#include "ecmascript/compiler/type.h"

namespace panda::ecmascript::kungfu {
GateRef NumberSpeculativeRetype::SetOutputType(GateRef gate, GateType pgoType)
{
    TypeInfo& type = typeInfos_[acc_.GetId(gate)];
    TypeInfo old = type;
    if (pgoType.IsIntType()) {
        type = TypeInfo::INT32;
    } else if (pgoType.IsDoubleType()) {
        type = TypeInfo::FLOAT64;
    } else if (pgoType.IsBooleanType()) {
        type = TypeInfo::INT1;
    } else {
        type = TypeInfo::TAGGED;
    }
    return old == type ? Circuit::NullGate() : gate;
}

GateRef NumberSpeculativeRetype::VisitGate(GateRef gate)
{
    OpCode op = acc_.GetOpCode(gate);
    switch (op) {
        case OpCode::TYPED_BINARY_OP:
            return VisitTypedBinaryOp(gate);
        case OpCode::TYPED_UNARY_OP:
            return VisitTypedUnaryOp(gate);
        case OpCode::INT32_OVERFLOW_CHECK:
            return VisitOverflowCheck(gate);
        case OpCode::INDEX_CHECK:
            return VisitIndexCheck(gate);
        case OpCode::LOAD_ELEMENT:
            return VisitLoadElement(gate);
        case OpCode::STORE_ELEMENT:
            return VisitStoreElement(gate);
        case OpCode::VALUE_SELECTOR:
            return VisitPhi(gate);
        case OpCode::CONSTANT:
            return VisitConstant(gate);
        default:
            return VisitOthers(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitTypedBinaryOp(GateRef gate)
{
    GateType leftType = acc_.GetLeftType(gate);
    GateType rightType = acc_.GetRightType(gate);
    if (leftType.IsNumberType() && rightType.IsNumberType()) {
        return VisitNumberBinaryOp(gate);
    } else {
        [[maybe_unused]] GateRef left = acc_.GetValueIn(gate, 0);
        [[maybe_unused]] GateRef right = acc_.GetValueIn(gate, 1);
        ASSERT((acc_.IsConstantUndefined(left)) || (acc_.IsConstantUndefined(right)));
        ASSERT(acc_.GetTypedBinaryOp(gate) == TypedBinOp::TYPED_STRICTEQ);
        return VisitUndefinedStrictEq(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitUndefinedStrictEq(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::BooleanType());
    }
    if (IsConvert()) {
        GateRef left = acc_.GetValueIn(gate, 0);
        GateRef right = acc_.GetValueIn(gate, 1);
        acc_.ReplaceValueIn(gate, ConvertToTagged(left), 0);
        acc_.ReplaceValueIn(gate, ConvertToTagged(right), 1);
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitConstant(GateRef gate)
{
    if (IsRetype()) {
        GateType gateType = acc_.GetGateType(gate);
        if (acc_.IsConstantNumber(gate)) {
            return SetOutputType(gate, gateType);
        } else {
            return SetOutputType(gate, GateType::AnyType());
        }
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitPhi(GateRef gate)
{
    size_t valueNum = acc_.GetNumValueIn(gate);
    if (IsRetype()) {
        TypeInfo tempType = TypeInfo::NONE;
        for (size_t i = 0; i < valueNum; ++i) {
            GateRef input = acc_.GetValueIn(gate, i);
            TypeInfo inputInfo = typeInfos_[acc_.GetId(input)];
            if (tempType == TypeInfo::NONE) {
                tempType = inputInfo;
            } else if ((tempType != inputInfo) && (inputInfo != TypeInfo::NONE)) {
                tempType = TypeInfo::TAGGED;
                break;
            }
        }
        TypeInfo& typeInfo = typeInfos_[acc_.GetId(gate)];
        if (typeInfo != tempType) {
            typeInfo = tempType;
            return gate;
        }
    }

    if (IsConvert()) {
        TypeInfo output = typeInfos_[acc_.GetId(gate)];
        if (output == TypeInfo::TAGGED) {
            return VisitOthers(gate);
        }
    }

    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitNumberBinaryOp(GateRef gate)
{
    TypedBinOp op = acc_.GetTypedBinaryOp(gate);
    switch (op) {
        case TypedBinOp::TYPED_ADD:
        case TypedBinOp::TYPED_SUB:
        case TypedBinOp::TYPED_MUL: {
            return VisitNumberCalculate(gate);
        }
        case TypedBinOp::TYPED_LESS:
        case TypedBinOp::TYPED_LESSEQ:
        case TypedBinOp::TYPED_GREATER:
        case TypedBinOp::TYPED_GREATEREQ:
        case TypedBinOp::TYPED_EQ:
        case TypedBinOp::TYPED_NOTEQ: {
            return VisitNumberCompare(gate);
        }
        case TypedBinOp::TYPED_SHL:
        case TypedBinOp::TYPED_SHR:
        case TypedBinOp::TYPED_ASHR: {
            return VisitNumberShift(gate);
        }
        default:
            return VisitNumberRelated(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitTypedUnaryOp(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
    TypedUnOp Op = accessor.GetTypedUnOp();
    switch (Op) {
        case TypedUnOp::TYPED_INC:
            return VisitTypedInc(gate);
        case TypedUnOp::TYPED_NOT:
            return VisitTypedNot(gate);
        case TypedUnOp::TYPED_JEQZ:
            return VisitTypedJeqz(gate);
        default:
            return VisitNumberRelated(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitNumberCalculate(GateRef gate)
{
    GateType gateType = acc_.GetGateType(gate);
    if (IsRetype()) {
        GateType resType = gateType.IsIntType() ? GateType::IntType() : GateType::DoubleType();
        return SetOutputType(gate, resType);
    } else if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        if (gateType.IsIntType()) {
            ConvertForIntOperator(gate);
        } else {
            ConvertForDoubleOperator(gate);
        }
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitNumberCompare(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::BooleanType());
    }
    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateType leftType = acc_.GetLeftType(gate);
        GateType rightType = acc_.GetRightType(gate);
        if (leftType.IsIntType() && rightType.IsIntType()) {
            ConvertForIntOperator(gate);
        } else {
            ConvertForDoubleOperator(gate);
        }
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitNumberShift(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::IntType());
    }
    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        ConvertForIntOperator(gate);
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitTypedInc(GateRef gate)
{
    TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
    GateType type = accessor.GetTypeValue();
    if (type.IsIntType()) {
        return VisitIntInc(gate);
    } else if (type.IsNumberType()) {
        return VisitDoubleInc(gate);
    } else {
        UNREACHABLE();
    }
}

GateRef NumberSpeculativeRetype::VisitIntInc(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::IntType());
    }

    if (IsConvert()) {
        GateRef value = acc_.GetValueIn(gate, 0);
        acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(value, GateType::IntType()), 0);
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitDoubleInc(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::DoubleType());
    }

    if (IsConvert()) {
        TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
        GateRef value = acc_.GetValueIn(gate, 0);
        acc_.ReplaceValueIn(gate, CheckAndConvertToFloat64(value, accessor.GetTypeValue()), 0);
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitTypedNot(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
    GateType type = accessor.GetTypeValue();
    if (type.IsIntType()) {
        return VisitIntNot(gate);
    } else {
        return VisitNumberRelated(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitTypedJeqz(GateRef gate)
{
    Environment env(gate, circuit_, &builder_);
    TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
    GateType type = accessor.GetTypeValue();
    if (type.IsBooleanType()) {
        return VisitBooleanJeqz(gate);
    } else {
        return VisitNumberRelated(gate);
    }
}

GateRef NumberSpeculativeRetype::VisitIntNot(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::IntType());
    }
    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateRef value = acc_.GetValueIn(gate, 0);
        TypedUnaryAccessor accessor(acc_.TryGetValue(gate));
        GateType valueType = accessor.GetTypeValue();
        acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(value, valueType), 0);
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitBooleanJeqz(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }
    if (IsConvert()) {
        GateRef value = acc_.GetValueIn(gate, 0);
        acc_.ReplaceValueIn(gate, CheckAndConvertToBool(value, GateType::BooleanType()), 0);
    }
    return Circuit::NullGate();
}

void NumberSpeculativeRetype::ConvertForIntOperator(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType leftType = acc_.GetLeftType(gate);
    GateType rightType = acc_.GetRightType(gate);
    acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(left, leftType), 0);
    acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(right, rightType), 1);
}

void NumberSpeculativeRetype::ConvertForDoubleOperator(GateRef gate)
{
    GateRef left = acc_.GetValueIn(gate, 0);
    GateRef right = acc_.GetValueIn(gate, 1);
    GateType leftType = acc_.GetLeftType(gate);
    GateType rightType = acc_.GetRightType(gate);
    acc_.ReplaceValueIn(gate, CheckAndConvertToFloat64(left, leftType), 0);
    acc_.ReplaceValueIn(gate, CheckAndConvertToFloat64(right, rightType), 1);
}

GateRef NumberSpeculativeRetype::VisitNumberRelated(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::NumberType());
    }
    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        size_t valueNum = acc_.GetNumValueIn(gate);
        for (size_t i = 0; i < valueNum; ++i) {
            GateRef input = acc_.GetValueIn(gate, i);
            GateType inputType = acc_.GetGateType(input);
            if (inputType.IsNumberType() || inputType.IsBooleanType()) {
                acc_.ReplaceValueIn(gate, CheckAndConvertToTagged(input, inputType), i);
            }
        }
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitOthers(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }
    if (IsConvert()) {
        size_t valueNum = acc_.GetNumValueIn(gate);
        for (size_t i = 0; i < valueNum; ++i) {
            GateRef input = acc_.GetValueIn(gate, i);
            acc_.ReplaceValueIn(gate, ConvertToTagged(input), i);
        }
    }
    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::CheckAndConvertToBool(GateRef gate, [[maybe_unused]]GateType gateType)
{
    TypeInfo output = typeInfos_[acc_.GetId(gate)];
    switch (output) {
        case TypeInfo::INT1:
            return gate;
        case TypeInfo::TAGGED: {
            ASSERT(gateType.IsBooleanType());
            return builder_.ConvertTaggedBooleanToBool(gate);
        }
        default: {
            UNREACHABLE();
            return Circuit::NullGate();
        }
    }
}

GateRef NumberSpeculativeRetype::CheckAndConvertToInt32(GateRef gate, GateType gateType)
{
    TypeInfo output = typeInfos_[acc_.GetId(gate)];
    switch (output) {
        case TypeInfo::INT32:
            return gate;
        case TypeInfo::FLOAT64:
            return builder_.ConvertFloat64ToInt32(gate);
        case TypeInfo::TAGGED: {
            ASSERT(gateType.IsNumberType());
            if (gateType.IsIntType()) {
                return builder_.CheckTaggedIntAndConvertToInt32(gate);
            } else if (gateType.IsDoubleType()) {
                return builder_.CheckTaggedDoubleAndConvertToInt32(gate);
            } else {
                return builder_.CheckTaggedNumberAndConvertToInt32(gate);
            }
        }
        default: {
            UNREACHABLE();
            return Circuit::NullGate();
        }
    }
}

GateRef NumberSpeculativeRetype::CheckAndConvertToFloat64(GateRef gate, GateType gateType)
{
    TypeInfo output = typeInfos_[acc_.GetId(gate)];
    switch (output) {
        case TypeInfo::INT32:
            return builder_.ConvertInt32ToFloat64(gate);
        case TypeInfo::FLOAT64:
            return gate;
        case TypeInfo::TAGGED: {
            ASSERT(gateType.IsNumberType());
            if (gateType.IsIntType()) {
                return builder_.CheckTaggedIntAndConvertToFloat64(gate);
            } else if (gateType.IsDoubleType()) {
                return builder_.CheckTaggedDoubleAndConvertToFloat64(gate);
            } else {
                return builder_.CheckTaggedNumberAndConvertToFloat64(gate);
            }
        }
        default: {
            UNREACHABLE();
            return Circuit::NullGate();
        }
    }
}

GateRef NumberSpeculativeRetype::CheckAndConvertToTagged(GateRef gate, GateType gateType)
{
    TypeInfo output = typeInfos_[acc_.GetId(gate)];
    switch (output) {
        case TypeInfo::INT1:
            return builder_.ConvertBoolToTaggedBoolean(gate);
        case TypeInfo::INT32:
            return builder_.ConvertInt32ToTaggedInt(gate);
        case TypeInfo::FLOAT64:
            return builder_.ConvertFloat64ToTaggedDouble(gate);
        case TypeInfo::TAGGED: {
            ASSERT(gateType.IsNumberType() || gateType.IsBooleanType());
            builder_.TryPrimitiveTypeCheck(gateType, gate);
            return gate;
        }
        default:
            UNREACHABLE();
            return Circuit::NullGate();
    }
}

GateRef NumberSpeculativeRetype::ConvertToTagged(GateRef gate)
{
    TypeInfo output = typeInfos_[acc_.GetId(gate)];
    switch (output) {
        case TypeInfo::INT1:
            return builder_.ConvertBoolToTaggedBoolean(gate);
        case TypeInfo::INT32:
            return builder_.ConvertInt32ToTaggedInt(gate);
        case TypeInfo::FLOAT64:
            return builder_.ConvertFloat64ToTaggedDouble(gate);
        case TypeInfo::TAGGED: {
            return gate;
        }
        default:
            UNREACHABLE();
            return Circuit::NullGate();
    }
}

GateRef NumberSpeculativeRetype::VisitOverflowCheck(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }

    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateRef value = acc_.GetValueIn(gate, 0);
        ASSERT(acc_.GetGateType(value).IsIntType());
        acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(value, GateType::IntType()), 0);
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }

    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitIndexCheck(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }

    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateRef receiver = acc_.GetValueIn(gate, 0);
        GateRef index = acc_.GetValueIn(gate, 1);
        GateType receiverType = acc_.GetGateType(receiver);
        GateType indexType = acc_.GetGateType(index);
        if (receiverType.IsNumberType()) {
            // IndexCheck receive length at first value input.
            acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(receiver, receiverType), 0);
        }
        if (indexType.IsNumberType()) {
            acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(index, indexType), 1);
        }
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }

    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitLoadElement(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }

    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateRef index = acc_.GetValueIn(gate, 1);
        GateType indexType = acc_.GetGateType(index);
        if (indexType.IsNumberType()) {
            acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(index, indexType), 1);
        }
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }

    return Circuit::NullGate();
}

GateRef NumberSpeculativeRetype::VisitStoreElement(GateRef gate)
{
    if (IsRetype()) {
        return SetOutputType(gate, GateType::AnyType());
    }

    if (IsConvert()) {
        Environment env(gate, circuit_, &builder_);
        GateRef index = acc_.GetValueIn(gate, 1);
        GateType indexType = acc_.GetGateType(index);
        GateRef value = acc_.GetValueIn(gate, 2);
        if (indexType.IsNumberType()) {
            acc_.ReplaceValueIn(gate, CheckAndConvertToInt32(index, indexType), 1);
        }
        acc_.ReplaceValueIn(gate, ConvertToTagged(value), 2);   // 2: index of value to be stored.
        acc_.ReplaceStateIn(gate, builder_.GetState());
        acc_.ReplaceDependIn(gate, builder_.GetDepend());
    }

    return Circuit::NullGate();
}

void NumberSpeculativeRetype::Run()
{
    // visit gate in RPO, propagate use infos and
    // reset the machine type of number operator gate and related phi,
    // if some tagged phi is used as native value, change it to native phi.
    state_ = State::Retype;
    VisitGraph();
    state_ = State::Convert;
    VisitGraph();
}

}  // namespace panda::ecmascript::kungfu