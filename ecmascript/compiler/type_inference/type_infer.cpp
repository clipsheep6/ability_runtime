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

#include "ecmascript/compiler/type_inference/type_infer.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/jspandafile/program_object.h"

namespace panda::ecmascript::kungfu {
void TypeInfer::TraverseCircuit()
{
    size_t gateCount = circuit_->GetGateCount();
    std::vector<bool> inQueue(gateCount, true);
    std::vector<bool> visited(gateCount, false);
    std::queue<GateRef> pendingQueue;
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (auto gate : gateList) {
        pendingQueue.push(gate);
    }

    while (!pendingQueue.empty()) {
        auto curGate = pendingQueue.front();
        inQueue[gateAccessor_.GetId(curGate)] = false;
        pendingQueue.pop();
        auto uses = gateAccessor_.ConstUses(curGate);
        for (auto useIt = uses.begin(); useIt != uses.end(); useIt++) {
            auto gateId = gateAccessor_.GetId(*useIt);
            if (Infer(*useIt) && !inQueue[gateId]) {
                inQueue[gateId] = true;
                pendingQueue.push(*useIt);
            }
        }
    }

    if (IsLogEnabled()) {
        PrintAllByteCodesTypes();
    }

    if (tsManager_->AssertTypes()) {
        Verify();
    }

    if (tsManager_->PrintAnyTypes()) {
        FilterAnyTypeGates();
    }
}

bool TypeInfer::UpdateType(GateRef gate, const GateType type)
{
    auto preType = gateAccessor_.GetGateType(gate);
    if (type != preType) {
        gateAccessor_.SetGateType(gate, type);
        return true;
    }
    return false;
}

bool TypeInfer::UpdateType(GateRef gate, const GlobalTSTypeRef &typeRef)
{
    auto type = GateType(typeRef);
    return UpdateType(gate, type);
}

bool TypeInfer::ShouldInfer(const GateRef gate) const
{
    auto op = gateAccessor_.GetOpCode(gate);
    // handle phi gates
    if (op == OpCode::VALUE_SELECTOR) {
        return true;
    }
    if (op == OpCode::JS_BYTECODE ||
        op == OpCode::CONSTANT ||
        op == OpCode::RETURN) {
        const auto &gateToBytecode = builder_->GetGateToBytecode();
        // handle gates generated by ecma.* bytecodes (not including Jump)
        if (gateToBytecode.find(gate) != gateToBytecode.end()) {
            return !builder_->GetByteCodeInfo(gate).IsJump();
        }
    }
    return false;
}

bool TypeInfer::Infer(GateRef gate)
{
    if (!ShouldInfer(gate)) {
        return false;
    }
    if (gateAccessor_.GetOpCode(gate) == OpCode::VALUE_SELECTOR) {
        return InferPhiGate(gate);
    }
    // infer ecma.* bytecode gates
    EcmaOpcode op = builder_->GetByteCodeOpcode(gate);
    switch (op) {
        case EcmaOpcode::LDNAN:
        case EcmaOpcode::LDINFINITY:
        case EcmaOpcode::SUB2_IMM8_V8:
        case EcmaOpcode::MUL2_IMM8_V8:
        case EcmaOpcode::DIV2_IMM8_V8:
        case EcmaOpcode::MOD2_IMM8_V8:
        case EcmaOpcode::SHL2_IMM8_V8:
        case EcmaOpcode::ASHR2_IMM8_V8:
        case EcmaOpcode::SHR2_IMM8_V8:
        case EcmaOpcode::AND2_IMM8_V8:
        case EcmaOpcode::OR2_IMM8_V8:
        case EcmaOpcode::XOR2_IMM8_V8:
        case EcmaOpcode::TONUMBER_IMM8:
        case EcmaOpcode::TONUMERIC_IMM8:
        case EcmaOpcode::NEG_IMM8:
        case EcmaOpcode::NOT_IMM8:
        case EcmaOpcode::INC_IMM8:
        case EcmaOpcode::DEC_IMM8:
        case EcmaOpcode::EXP_IMM8_V8:
        case EcmaOpcode::STARRAYSPREAD_V8_V8:
        case EcmaOpcode::DEPRECATED_TONUMBER_PREF_V8:
        case EcmaOpcode::DEPRECATED_TONUMERIC_PREF_V8:
        case EcmaOpcode::DEPRECATED_NEG_PREF_V8:
        case EcmaOpcode::DEPRECATED_NOT_PREF_V8:
        case EcmaOpcode::DEPRECATED_INC_PREF_V8:
        case EcmaOpcode::DEPRECATED_DEC_PREF_V8:
            return SetNumberType(gate);
        case EcmaOpcode::LDBIGINT_ID16:
            return SetBigIntType(gate);
        case EcmaOpcode::LDTRUE:
        case EcmaOpcode::LDFALSE:
        case EcmaOpcode::EQ_IMM8_V8:
        case EcmaOpcode::NOTEQ_IMM8_V8:
        case EcmaOpcode::LESS_IMM8_V8:
        case EcmaOpcode::LESSEQ_IMM8_V8:
        case EcmaOpcode::GREATER_IMM8_V8:
        case EcmaOpcode::GREATEREQ_IMM8_V8:
        case EcmaOpcode::ISIN_IMM8_V8:
        case EcmaOpcode::INSTANCEOF_IMM8_V8:
        case EcmaOpcode::STRICTNOTEQ_IMM8_V8:
        case EcmaOpcode::STRICTEQ_IMM8_V8:
        case EcmaOpcode::ISTRUE:
        case EcmaOpcode::ISFALSE:
        case EcmaOpcode::SETOBJECTWITHPROTO_IMM8_V8:
        case EcmaOpcode::SETOBJECTWITHPROTO_IMM16_V8:
        case EcmaOpcode::DELOBJPROP_V8:
        case EcmaOpcode::DEPRECATED_SETOBJECTWITHPROTO_PREF_V8_V8:
        case EcmaOpcode::DEPRECATED_DELOBJPROP_PREF_V8_V8:
            return SetBooleanType(gate);
        case EcmaOpcode::LDUNDEFINED:
            return InferLdUndefined(gate);
        case EcmaOpcode::LDNULL:
            return InferLdNull(gate);
        case EcmaOpcode::LDAI_IMM32:
            return InferLdai(gate);
        case EcmaOpcode::FLDAI_IMM64:
            return InferFLdai(gate);
        case EcmaOpcode::LDSYMBOL:
            return InferLdSymbol(gate);
        case EcmaOpcode::THROW_PREF_NONE:
            return InferThrow(gate);
        case EcmaOpcode::TYPEOF_IMM8:
        case EcmaOpcode::TYPEOF_IMM16:
            return InferTypeOf(gate);
        case EcmaOpcode::ADD2_IMM8_V8:
            return InferAdd2(gate);
        case EcmaOpcode::LDOBJBYINDEX_IMM8_IMM16:
        case EcmaOpcode::LDOBJBYINDEX_IMM16_IMM16:
        case EcmaOpcode::WIDE_LDOBJBYINDEX_PREF_IMM32:
        case EcmaOpcode::DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32:
            return InferLdObjByIndex(gate);
        case EcmaOpcode::STGLOBALVAR_IMM16_ID16:
        case EcmaOpcode::STTOGLOBALRECORD_IMM16_ID16:
        case EcmaOpcode::STCONSTTOGLOBALRECORD_IMM16_ID16:
        case EcmaOpcode::TRYSTGLOBALBYNAME_IMM8_ID16:
        case EcmaOpcode::TRYSTGLOBALBYNAME_IMM16_ID16:
        case EcmaOpcode::DEPRECATED_STCONSTTOGLOBALRECORD_PREF_ID32:
        case EcmaOpcode::DEPRECATED_STLETTOGLOBALRECORD_PREF_ID32:
        case EcmaOpcode::DEPRECATED_STCLASSTOGLOBALRECORD_PREF_ID32:
            return SetStGlobalBcType(gate);
        case EcmaOpcode::LDGLOBALVAR_IMM16_ID16:
            return InferLdGlobalVar(gate);
        case EcmaOpcode::RETURNUNDEFINED:
            return InferReturnUndefined(gate);
        case EcmaOpcode::RETURN:
            return InferReturn(gate);
        case EcmaOpcode::LDOBJBYNAME_IMM8_ID16:
        case EcmaOpcode::LDOBJBYNAME_IMM16_ID16:
        case EcmaOpcode::DEPRECATED_LDOBJBYNAME_PREF_ID32_V8:
            return InferLdObjByName(gate);
        case EcmaOpcode::LDA_STR_ID16:
            return InferLdStr(gate);
        case EcmaOpcode::CALLARG0_IMM8:
        case EcmaOpcode::CALLARG1_IMM8_V8:
        case EcmaOpcode::CALLARGS2_IMM8_V8_V8:
        case EcmaOpcode::CALLARGS3_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLRANGE_PREF_IMM16_V8:
        case EcmaOpcode::CALLTHIS0_IMM8_V8:
        case EcmaOpcode::CALLTHIS1_IMM8_V8_V8:
        case EcmaOpcode::CALLTHIS2_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLTHIS3_IMM8_V8_V8_V8_V8:
        case EcmaOpcode::CALLTHISRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLTHISRANGE_PREF_IMM16_V8:
        case EcmaOpcode::APPLY_IMM8_V8_V8:
            return InferCallFunction(gate);
        case EcmaOpcode::DEPRECATED_CALLARG0_PREF_V8:
        case EcmaOpcode::DEPRECATED_CALLARG1_PREF_V8_V8:
        case EcmaOpcode::DEPRECATED_CALLARGS2_PREF_V8_V8_V8:
        case EcmaOpcode::DEPRECATED_CALLARGS3_PREF_V8_V8_V8_V8:
        case EcmaOpcode::DEPRECATED_CALLSPREAD_PREF_V8_V8_V8:
        case EcmaOpcode::DEPRECATED_CALLRANGE_PREF_IMM16_V8:
        case EcmaOpcode::DEPRECATED_CALLTHISRANGE_PREF_IMM16_V8:
            return InferCallFunction(gate, true);
        case EcmaOpcode::LDOBJBYVALUE_IMM8_V8:
        case EcmaOpcode::LDOBJBYVALUE_IMM16_V8:
        case EcmaOpcode::DEPRECATED_LDOBJBYVALUE_PREF_V8_V8:
            return InferLdObjByValue(gate);
        case EcmaOpcode::GETNEXTPROPNAME_V8:
            return InferGetNextPropName(gate);
        case EcmaOpcode::DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8:
            return InferDefineGetterSetterByValue(gate);
        case EcmaOpcode::NEWOBJRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::NEWOBJRANGE_IMM16_IMM8_V8:
        case EcmaOpcode::WIDE_NEWOBJRANGE_PREF_IMM16_V8:
        case EcmaOpcode::NEWOBJAPPLY_IMM8_V8:
        case EcmaOpcode::NEWOBJAPPLY_IMM16_V8:
            return InferNewObject(gate);
        case EcmaOpcode::SUPERCALLTHISRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8:
        case EcmaOpcode::SUPERCALLARROWRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8:
        case EcmaOpcode::SUPERCALLSPREAD_IMM8_V8:
            return InferSuperCall(gate);
        case EcmaOpcode::TRYLDGLOBALBYNAME_IMM8_ID16:
        case EcmaOpcode::TRYLDGLOBALBYNAME_IMM16_ID16:
            return InferTryLdGlobalByName(gate);
        case EcmaOpcode::LDLEXVAR_IMM4_IMM4:
        case EcmaOpcode::LDLEXVAR_IMM8_IMM8:
        case EcmaOpcode::WIDE_LDLEXVAR_PREF_IMM16_IMM16:
            return InferLdLexVarDyn(gate);
        case EcmaOpcode::STLEXVAR_IMM4_IMM4:
        case EcmaOpcode::STLEXVAR_IMM8_IMM8:
        case EcmaOpcode::WIDE_STLEXVAR_PREF_IMM16_IMM16:
        case EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM4_IMM4_V8:
        case EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM8_IMM8_V8:
        case EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM16_IMM16_V8:
            return InferStLexVarDyn(gate);
        default:
            break;
    }
    return false;
}

bool TypeInfer::InferPhiGate(GateRef gate)
{
    ASSERT(gateAccessor_.GetOpCode(gate) == OpCode::VALUE_SELECTOR);
    CVector<GlobalTSTypeRef> typeList;
    auto ins = gateAccessor_.ConstIns(gate);
    for (auto it =  ins.begin(); it != ins.end(); it++) {
        // assuming that VALUE_SELECTOR is NO_DEPEND and NO_ROOT
        if (gateAccessor_.GetOpCode(*it) == OpCode::MERGE) {
            continue;
        }
        if (gateAccessor_.GetOpCode(*it) == OpCode::LOOP_BEGIN) {
            auto loopInGate = gateAccessor_.GetValueIn(gate);
            auto loopInType = gateAccessor_.GetGateType(loopInGate);
            return UpdateType(gate, loopInType);
        }
        auto valueInType = gateAccessor_.GetGateType(*it);
        if (valueInType.IsAnyType()) {
            return UpdateType(gate, valueInType);
        }
        typeList.emplace_back(valueInType.GetGTRef());
    }
    // deduplicate
    auto deduplicateIndex = std::unique(typeList.begin(), typeList.end());
    typeList.erase(deduplicateIndex, typeList.end());
    if (typeList.size() > 1) {
        auto unionType = tsManager_->GetOrCreateUnionType(typeList);
        return UpdateType(gate, unionType);
    }
    auto type = typeList.at(0);
    return UpdateType(gate, type);
}

bool TypeInfer::SetNumberType(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::SetBigIntType(GateRef gate)
{
    auto bigIntType = GateType::BigIntType();
    return UpdateType(gate, bigIntType);
}

bool TypeInfer::SetBooleanType(GateRef gate)
{
    auto booleanType = GateType::BooleanType();
    return UpdateType(gate, booleanType);
}

bool TypeInfer::InferLdUndefined(GateRef gate)
{
    auto undefinedType = GateType::UndefinedType();
    return UpdateType(gate, undefinedType);
}

bool TypeInfer::InferLdNull(GateRef gate)
{
    auto nullType = GateType::NullType();
    return UpdateType(gate, nullType);
}

bool TypeInfer::InferLdai(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::InferFLdai(GateRef gate)
{
    auto numberType = GateType::NumberType();
    return UpdateType(gate, numberType);
}

bool TypeInfer::InferLdSymbol(GateRef gate)
{
    auto symbolType = GateType::SymbolType();
    return UpdateType(gate, symbolType);
}

bool TypeInfer::InferThrow(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferTypeOf(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferAdd2(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto firInType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    auto secInType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (firInType.IsStringType() || secInType.IsStringType()) {
        return UpdateType(gate, GateType::StringType());
    }
    if (firInType.IsNumberType() && secInType.IsNumberType()) {
        return UpdateType(gate, GateType::NumberType());
    }
    return UpdateType(gate, GateType::AnyType());
}

bool TypeInfer::InferLdObjByIndex(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto inValueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (tsManager_->IsArrayTypeKind(inValueType)) {
        auto type = tsManager_->GetArrayParameterTypeGT(inValueType);
        return UpdateType(gate, type);
    }
    return false;
}

bool TypeInfer::SetStGlobalBcType(GateRef gate)
{
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto inValueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (stringIdToGateType_.find(stringId) != stringIdToGateType_.end()) {
        stringIdToGateType_[stringId] = inValueType;
    } else {
        stringIdToGateType_.emplace(stringId, inValueType);
    }
    return UpdateType(gate, inValueType);
}

bool TypeInfer::InferLdGlobalVar(GateRef gate)
{
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    auto iter = stringIdToGateType_.find(stringId);
    if (iter != stringIdToGateType_.end()) {
        return UpdateType(gate, iter->second);
    }
    return false;
}

bool TypeInfer::InferReturnUndefined(GateRef gate)
{
    auto undefinedType = GateType::UndefinedType();
    return UpdateType(gate, undefinedType);
}

bool TypeInfer::InferReturn(GateRef gate)
{
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 1);
    auto gateType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, gateType);
}

bool TypeInfer::InferLdObjByName(GateRef gate)
{
    // 2: number of value inputs
    ASSERT(gateAccessor_.GetNumValueIn(gate) == 2);
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 1));
    if (!objType.IsAnyType()) {
        if (tsManager_->IsArrayTypeKind(objType)) {
            auto builtinGt = GlobalTSTypeRef(TSModuleTable::BUILTINS_TABLE_ID, TSManager::BUILTIN_ARRAY_ID);
            auto builtinInstanceType = tsManager_->CreateClassInstanceType(builtinGt);
            objType = GateType(builtinInstanceType);
        }
        // If this object has no gt type, we cannot get its internal property type
        if (IsObjectOrClass(objType)) {
            auto index = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
            auto thread = tsManager_->GetEcmaVM()->GetJSThread();
            auto name = ConstantPool::GetStringFromCache(thread, constantPool_.GetTaggedValue(), index);
            auto type = GetPropType(objType, name);
            return UpdateType(gate, type);
        }
    }
    return false;
}

bool TypeInfer::InferNewObject(GateRef gate)
{
    if (gateAccessor_.GetGateType(gate).IsAnyType()) {
        ASSERT(gateAccessor_.GetNumValueIn(gate) > 0);
        auto classType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
        if (tsManager_->IsClassTypeKind(classType)) {
            auto classInstanceType = tsManager_->CreateClassInstanceType(classType);
            return UpdateType(gate, classInstanceType);
        }
    }
    return false;
}

bool TypeInfer::InferLdStr(GateRef gate)
{
    auto stringType = GateType::StringType();
    return UpdateType(gate, stringType);
}

bool TypeInfer::InferCallFunction(GateRef gate, bool isDeprecated)
{
    // first elem is function in old isa
    auto funcIndex = 0;
    if (!isDeprecated) {
        // 2: last two elem is function anc bytecode offset in new isa
        funcIndex = gateAccessor_.GetNumValueIn(gate) - 2;
    }
    auto funcType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, funcIndex));
    if (tsManager_->IsFunctionTypeKind(funcType)) {
        auto returnType = tsManager_->GetFuncReturnValueTypeGT(funcType);
        return UpdateType(gate, returnType);
    }
    return false;
}

bool TypeInfer::InferLdObjByValue(GateRef gate)
{
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));

    // handle array
    if (tsManager_->IsArrayTypeKind(objType)) {
        auto elementType = tsManager_->GetArrayParameterTypeGT(objType);
        return UpdateType(gate, elementType);
    }
    // handle object
    if (IsObjectOrClass(objType)) {
        auto valueGate = gateAccessor_.GetValueIn(gate, 1);
        if (gateAccessor_.GetOpCode(valueGate) == OpCode::CONSTANT) {
            auto value = gateAccessor_.GetBitField(valueGate);
            auto type = GetPropType(objType, value);
            return UpdateType(gate, type);
        }
    }
    return false;
}

bool TypeInfer::InferGetNextPropName(GateRef gate)
{
    auto stringType = GateType::StringType();
    return UpdateType(gate, stringType);
}

bool TypeInfer::InferDefineGetterSetterByValue(GateRef gate)
{
    // 0 : the index of obj
    auto objType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 0));
    return UpdateType(gate, objType);
}

bool TypeInfer::InferSuperCall(GateRef gate)
{
    ArgumentAccessor argAcc(circuit_);
    auto newTarget = argAcc.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
    auto funcType = gateAccessor_.GetGateType(newTarget);
    if (!funcType.IsUndefinedType()) {
        return UpdateType(gate, funcType);
    }
    return false;
}

bool TypeInfer::InferTryLdGlobalByName(GateRef gate)
{
    // todo by hongtao, should consider function of .d.ts
    auto byteCodeInfo = builder_->GetByteCodeInfo(gate);
    ASSERT(byteCodeInfo.inputs.size() == 1);
    auto stringId = std::get<StringId>(byteCodeInfo.inputs[0]).GetId();
    auto iter = stringIdToGateType_.find(stringId);
    if (iter != stringIdToGateType_.end()) {
        return UpdateType(gate, iter->second);
    }
    return false;
}

bool TypeInfer::InferLdLexVarDyn(GateRef gate)
{
    auto level = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
    auto slot = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 1));
    auto type = lexEnvManager_->GetLexEnvElementType(methodId_, level, slot);
    return UpdateType(gate, type);
}

bool TypeInfer::InferStLexVarDyn(GateRef gate)
{
    auto level = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 0));
    auto slot = gateAccessor_.GetBitField(gateAccessor_.GetValueIn(gate, 1));
    auto type = lexEnvManager_->GetLexEnvElementType(methodId_, level, slot);
    if (type.IsAnyType()) {
        auto valueType = gateAccessor_.GetGateType(gateAccessor_.GetValueIn(gate, 2));
        if (!valueType.IsAnyType()) {
            lexEnvManager_->SetLexEnvElementType(methodId_, level, slot, valueType);
            return true;
        }
    }
    return false;
}

void TypeInfer::PrintAllByteCodesTypes() const
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);

    const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
    const MethodLiteral *methodLiteral = builder_->GetMethod();
    const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodLiteral->GetMethodId());

    auto &pcToBCOffset = builder_->GetPcToBCOffset();
    auto &bytecodeToGate = builder_->GetBytecodeToGate();

    LOG_COMPILER(INFO) << "print bytecode types:";
    LOG_COMPILER(INFO) << ".function " + functionName + "() {";
    for (auto it = pcToBCOffset.begin(); std::next(it) != pcToBCOffset.end(); it++) {  // ignore last element
        const uint8_t *pc = it->first;
        BytecodeInstruction inst(pc);
        auto findIt = bytecodeToGate.find(pc);
        if (findIt != bytecodeToGate.end()) {
            GateRef gate = bytecodeToGate.at(pc);
            GateType type = gateAccessor_.GetGateType(gate);
            if (!tsManager_->IsPrimitiveTypeKind(type)) {
                GlobalTSTypeRef gt = type.GetGTRef();
                LOG_COMPILER(INFO) << "    " << inst << ", type: " + tsManager_->GetTypeStr(type)
                                   << ", [moduleId: " + std::to_string(gt.GetModuleId())
                                   << ", [localId: " + std::to_string(gt.GetLocalId()) + "]";
            } else {
                    LOG_COMPILER(INFO) << "    " << inst << ", type: " + tsManager_->GetTypeStr(type);
            }
        }
    }
    LOG_COMPILER(INFO) << "}";
}

void TypeInfer::Verify() const
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (const auto &gate : gateList) {
        auto op = gateAccessor_.GetOpCode(gate);
        if (op == OpCode::JS_BYTECODE) {
            TypeCheck(gate);
        }
    }
}

/*
 * Let v be a variable in one ts-file and t be a type. To check whether the type of v is t after
 * type inferenece, one should declare a function named "AssertType(value:any, type:string):void"
 * in ts-file and call it with arguments v and t, where t is the expected type string.
 * The following interface performs such a check at compile time.
 */
void TypeInfer::TypeCheck(GateRef gate) const
{
    auto info = builder_->GetByteCodeInfo(gate);
    if (!info.IsBc(EcmaOpcode::CALLARGS2_IMM8_V8_V8)) {
        return;
    }
    auto func = gateAccessor_.GetValueIn(gate, 2);
    auto funcInfo = builder_->GetByteCodeInfo(func);
    if (!funcInfo.IsBc(EcmaOpcode::TRYLDGLOBALBYNAME_IMM8_ID16) &&
        !funcInfo.IsBc(EcmaOpcode::TRYLDGLOBALBYNAME_IMM16_ID16)) {
        return;
    }
    auto funcName = gateAccessor_.GetValueIn(func, 0);
    auto thread = tsManager_->GetEcmaVM()->GetJSThread();
    ConstantPool::GetStringFromCache(thread, constantPool_.GetTaggedValue(), gateAccessor_.GetBitField(funcName));
    auto funcNameString = constantPool_->GetStdStringByIdx(gateAccessor_.GetBitField(funcName));
    if (funcNameString ==  "AssertType") {
        GateRef expectedGate = gateAccessor_.GetValueIn(gateAccessor_.GetValueIn(gate, 1), 0);
        ConstantPool::GetStringFromCache(thread, constantPool_.GetTaggedValue(),
                                         gateAccessor_.GetBitField(expectedGate));
        auto expectedTypeStr = constantPool_->GetStdStringByIdx(gateAccessor_.GetBitField(expectedGate));
        GateRef valueGate = gateAccessor_.GetValueIn(gate, 0);
        auto type = gateAccessor_.GetGateType(valueGate);
        if (expectedTypeStr != tsManager_->GetTypeStr(type)) {
            const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
            const MethodLiteral *methodLiteral = builder_->GetMethod();
            EntityId methodId = builder_->GetMethod()->GetMethodId();
            DebugInfoExtractor *debugExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
            const std::string &sourceFileName = debugExtractor->GetSourceFile(methodId);
            const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodId);

            std::string log = CollectGateTypeLogInfo(valueGate, debugExtractor, "[TypeAssertion] ");
            log += "[TypeAssertion] but expected type: " + expectedTypeStr + "\n";

            LOG_COMPILER(ERROR) << "[TypeAssertion] [" << sourceFileName << ":" << functionName << "] begin:";
            LOG_COMPILER(FATAL) << log << " [TypeAssertion] end";
        }
    }
}

void TypeInfer::FilterAnyTypeGates() const
{
    const JSPandaFile *jsPandaFile = builder_->GetJSPandaFile();
    const MethodLiteral *methodLiteral = builder_->GetMethod();
    EntityId methodId = methodLiteral->GetMethodId();

    DebugInfoExtractor *debugExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
    const std::string &sourceFileName = debugExtractor->GetSourceFile(methodId);
    const std::string functionName = methodLiteral->ParseFunctionName(jsPandaFile, methodId);

    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    std::string log;
    for (const auto &gate : gateList) {
        GateType type = gateAccessor_.GetGateType(gate);
        if (ShouldInfer(gate) && type.IsAnyType()) {
            log += CollectGateTypeLogInfo(gate, debugExtractor, "[TypeFilter] ");
        }
    }

    LOG_COMPILER(INFO) << "[TypeFilter] [" << sourceFileName << ":" << functionName << "] begin:";
    LOG_COMPILER(INFO) << log << "[TypeFilter] end";
}

std::string TypeInfer::CollectGateTypeLogInfo(GateRef gate, DebugInfoExtractor *debugExtractor,
                                              const std::string &logPreFix) const
{
    std::string log(logPreFix);
    log += "gate id: "+ std::to_string(gateAccessor_.GetId(gate)) + ", ";
    OpCode op = gateAccessor_.GetOpCode(gate);
    log += "op: " + op.Str() + ", ";
    if (op != OpCode::VALUE_SELECTOR) {
    // handle ByteCode gate: print gate id, bytecode and line number in source code.
        log += "bytecode: " + builder_->GetBytecodeStr(gate) + ", ";
        GateType type = gateAccessor_.GetGateType(gate);
        log += "type: " + tsManager_->GetTypeStr(type) + ", ";
        if (!tsManager_->IsPrimitiveTypeKind(type)) {
            GlobalTSTypeRef gt = type.GetGTRef();
            log += "[moduleId: " + std::to_string(gt.GetModuleId()) + ", ";
            log += "localId: " + std::to_string(gt.GetLocalId()) + "], ";
        }

        int32_t lineNumber = 0;
        auto callbackLineFunc = [&lineNumber](int32_t line) -> bool {
            lineNumber = line + 1;
            return true;
        };

        const auto &gateToBytecode = builder_->GetGateToBytecode();
        const uint8_t *pc = gateToBytecode.at(gate).second;
        const MethodLiteral *methodLiteral = builder_->GetMethod();

        uint32_t offset = pc - methodLiteral->GetBytecodeArray();
        debugExtractor->MatchLineWithOffset(callbackLineFunc, methodLiteral->GetMethodId(), offset);

        log += "at line: " + std::to_string(lineNumber);
    } else {
    // handle phi gate: print gate id and input gates id list.
        log += "phi gate, ins: ";
        auto ins = gateAccessor_.ConstIns(gate);
        for (auto it =  ins.begin(); it != ins.end(); it++) {
            log += std::to_string(gateAccessor_.GetId(*it)) + " ";
        }
    }

    log += "\n compiler: ";
    return log;
}
}  // namespace panda::ecmascript
