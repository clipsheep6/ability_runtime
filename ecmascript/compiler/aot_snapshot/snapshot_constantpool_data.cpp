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

#include "ecmascript/compiler/aot_snapshot/snapshot_constantpool_data.h"

#include "ecmascript/compiler/pgo_type/pgo_type_location.h"
#include "ecmascript/compiler/pgo_type/pgo_type_manager.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/ts_types/global_type_info.h"
#include "ecmascript/global_env_constants-inl.h"

namespace panda::ecmascript::kungfu {
uint64_t BaseSnapshotInfo::GetItemKey(uint32_t constantPoolId, uint32_t constantPoolIdx)
{
    uint64_t result = constantPoolId;
    result = result << CONSTPOOL_MASK;
    result |= constantPoolIdx;
    return result;
}

void BaseSnapshotInfo::Record(ItemData &data)
{
    ItemKey key = GetItemKey(data.constantPoolId_, data.constantPoolIdx_);
    info_.emplace(key, data);
}

void BaseSnapshotInfo::CollectLiteralInfo(EcmaVM *vm, JSHandle<TaggedArray> array, uint32_t constantPoolIndex,
                                          JSHandle<ConstantPool> snapshotConstantPool,
                                          const std::set<uint32_t> &skippedMethods,
                                          JSHandle<JSTaggedValue> ihc, JSHandle<JSTaggedValue> chc)
{
    JSThread *thread = vm->GetJSThread();
    ObjectFactory *factory = vm->GetFactory();
    JSMutableHandle<JSTaggedValue> valueHandle(thread, JSTaggedValue::Undefined());
    uint32_t len = array->GetLength();
    std::vector<int> methodOffsetVec;
    for (uint32_t i = 0; i < len; i++) {
        valueHandle.Update(array->Get(i));
        if (valueHandle->IsJSFunction()) {
            auto methodOffset = JSHandle<JSFunction>(valueHandle)->GetCallTarget()->GetMethodId().GetOffset();
            if (skippedMethods.find(methodOffset) != skippedMethods.end()) {
                methodOffsetVec.emplace_back(AOTLiteralInfo::NO_FUNC_ENTRY_VALUE);
            } else {
                methodOffsetVec.emplace_back(methodOffset);
            }
        }
    }

    uint32_t methodSize = methodOffsetVec.size();
    JSHandle<AOTLiteralInfo> aotLiteralInfo = factory->NewAOTLiteralInfo(methodSize);
    for (uint32_t i = 0; i < methodSize; ++i) {
        auto methodOffset = methodOffsetVec[i];
        aotLiteralInfo->SetObjectToCache(thread, i, JSTaggedValue(methodOffset));
    }

    if (!ihc->IsUndefined()) {
        aotLiteralInfo->SetIhc(ihc.GetTaggedValue());
    }

    if (!chc->IsUndefined()) {
        aotLiteralInfo->SetChc(chc.GetTaggedValue());
    }

    snapshotConstantPool->SetObjectToCache(thread, constantPoolIndex, aotLiteralInfo.GetTaggedValue());
}

void StringSnapshotInfo::StoreDataToGlobalData(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                               SnapshotGlobalData &globalData, const std::set<uint32_t>&)
{
    JSThread *thread = vm->GetJSThread();
    for (auto item : info_) {
        const ItemData &data = item.second;
        JSTaggedValue cp = thread->GetCurrentEcmaContext()->FindConstpool(jsPandaFile, data.constantPoolId_);
        JSTaggedValue str = ConstantPool::GetStringFromCache(thread, cp, data.constantPoolIdx_);

        uint32_t snapshotCpArrIdx = globalData.GetCpArrIdxByConstanPoolId(data.constantPoolId_);
        JSHandle<TaggedArray> snapshotCpArr(thread, globalData.GetCurSnapshotCpArray());
        JSHandle<ConstantPool> snapshotCp(thread, snapshotCpArr->Get(snapshotCpArrIdx));
        snapshotCp->SetObjectToCache(thread, data.constantPoolIdx_, str);
    }
}

void MethodSnapshotInfo::StoreDataToGlobalData(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                               SnapshotGlobalData &globalData,
                                               const std::set<uint32_t> &skippedMethods)
{
    JSThread *thread = vm->GetJSThread();
    ObjectFactory *factory = vm->GetFactory();
    PGOTypeManager *ptManager = thread->GetCurrentEcmaContext()->GetPTManager();
    for (auto item : info_) {
        const ItemData &data = item.second;
        JSHandle<ConstantPool> cp(thread,
            thread->GetCurrentEcmaContext()->FindConstpool(jsPandaFile, data.constantPoolId_));
        uint32_t methodOffset = cp->GetEntityId(data.constantPoolIdx_).GetOffset();

        uint32_t snapshotCpArrIdx = globalData.GetCpArrIdxByConstanPoolId(data.constantPoolId_);
        JSHandle<TaggedArray> snapshotCpArr(thread, globalData.GetCurSnapshotCpArray());
        JSHandle<ConstantPool> snapshotCp(thread, snapshotCpArr->Get(snapshotCpArrIdx));

        PGOTypeLocation loc(jsPandaFile, data.methodOffset_, data.bcIndex_);
        ProfileType pt = ptManager->GetRootIdByLocation(loc);
        JSHandle<JSTaggedValue> ihc = JSHandle<JSTaggedValue>(thread, ptManager->QueryHClass(pt, pt));
        JSHandle<AOTLiteralInfo> aotLiteralInfo = factory->NewAOTLiteralInfo(1); // 1: only one method
        int initValue = static_cast<int>(AOTLiteralInfo::NO_FUNC_ENTRY_VALUE);
        aotLiteralInfo->SetObjectToCache(thread, 0, JSTaggedValue(initValue));
        if (!ihc->IsUndefined()) {
            aotLiteralInfo->SetIhc(ihc.GetTaggedValue());
        }
        if (skippedMethods.find(methodOffset) == skippedMethods.end()) {
            aotLiteralInfo->SetObjectToCache(thread, 0, JSTaggedValue(methodOffset));
            snapshotCp->SetObjectToCache(thread, data.constantPoolIdx_, aotLiteralInfo.GetTaggedValue());
            globalData.RecordReviseData(
                ReviseData::ItemData {globalData.GetCurDataIdx(), snapshotCpArrIdx, data.constantPoolIdx_});
        }
    }
}

void ClassLiteralSnapshotInfo::StoreDataToGlobalData(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                                     SnapshotGlobalData &globalData,
                                                     const std::set<uint32_t> &skippedMethods)
{
    JSThread *thread = vm->GetJSThread();
    PGOTypeManager *ptManager = thread->GetCurrentEcmaContext()->GetPTManager();
    for (auto item : info_) {
        const ItemData &data = item.second;
        JSHandle<ConstantPool> cp(thread,
            thread->GetCurrentEcmaContext()->FindConstpool(jsPandaFile, data.constantPoolId_));
        auto literalObj = ConstantPool::GetClassLiteralFromCache(thread, cp, data.constantPoolIdx_, data.recordName_);
        JSHandle<ClassLiteral> classLiteral(thread, literalObj);
        JSHandle<TaggedArray> arrayHandle(thread, classLiteral->GetArray());

        uint32_t snapshotCpArrIdx = globalData.GetCpArrIdxByConstanPoolId(data.constantPoolId_);
        JSHandle<TaggedArray> snapshotCpArr(thread, globalData.GetCurSnapshotCpArray());
        JSHandle<ConstantPool> snapshotCp(thread, snapshotCpArr->Get(snapshotCpArrIdx));

        PGOTypeLocation loc(jsPandaFile, data.methodOffset_, data.bcIndex_);
        PGOTypeLocation ctorLoc = loc.ChangeType(PGOTypeLocation::Type::CONSTRUCTOR);
        ProfileType pt = ptManager->GetRootIdByLocation(loc);
        ProfileType ctorPt = ptManager->GetRootIdByLocation(ctorLoc);
        JSHandle<JSTaggedValue> ihc = JSHandle<JSTaggedValue>(thread, ptManager->QueryHClass(pt, pt));
        JSHandle<JSTaggedValue> chc = JSHandle<JSTaggedValue>(thread, ptManager->QueryHClass(ctorPt, ctorPt));

        CollectLiteralInfo(vm, arrayHandle, data.constantPoolIdx_, snapshotCp, skippedMethods, ihc, chc);
        globalData.RecordReviseData(
            ReviseData::ItemData {globalData.GetCurDataIdx(), snapshotCpArrIdx, data.constantPoolIdx_});
    }
}


void ObjectLiteralSnapshotInfo::StoreDataToGlobalData(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                                      SnapshotGlobalData &globalData,
                                                      const std::set<uint32_t> &skippedMethods)
{
    JSThread *thread = vm->GetJSThread();
    PGOTypeManager *ptManager = thread->GetCurrentEcmaContext()->GetPTManager();
    for (auto item : info_) {
        const ItemData &data = item.second;
        JSHandle<ConstantPool> cp(thread,
            thread->GetCurrentEcmaContext()->FindConstpool(jsPandaFile, data.constantPoolId_));
        panda_file::File::EntityId id = cp->GetEntityId(data.constantPoolIdx_);
        JSMutableHandle<TaggedArray> elements(thread, JSTaggedValue::Undefined());
        JSMutableHandle<TaggedArray> properties(thread, JSTaggedValue::Undefined());
        LiteralDataExtractor::ExtractObjectDatas(thread, jsPandaFile, id, elements,
                                                 properties, cp, data.recordName_);

        uint32_t snapshotCpArrIdx = globalData.GetCpArrIdxByConstanPoolId(data.constantPoolId_);
        JSHandle<TaggedArray> snapshotCpArr(thread, globalData.GetCurSnapshotCpArray());
        JSHandle<ConstantPool> snapshotCp(thread, snapshotCpArr->Get(snapshotCpArrIdx));

        PGOTypeLocation loc(jsPandaFile, data.methodOffset_, data.bcIndex_);
        PGOTypeLocation ctorLoc = loc.ChangeType(PGOTypeLocation::Type::CONSTRUCTOR);
        ProfileType pt = ptManager->GetRootIdByLocation(loc);
        ProfileType ctorPt = ptManager->GetRootIdByLocation(ctorLoc);
        JSHandle<JSTaggedValue> ihc = JSHandle<JSTaggedValue>(thread, ptManager->QueryHClass(pt, pt));
        JSHandle<JSTaggedValue> chc = JSHandle<JSTaggedValue>(thread, ptManager->QueryHClass(ctorPt, ctorPt));

        CollectLiteralInfo(vm, properties, data.constantPoolIdx_, snapshotCp, skippedMethods, ihc, chc);
        globalData.RecordReviseData(
            ReviseData::ItemData {globalData.GetCurDataIdx(), snapshotCpArrIdx, data.constantPoolIdx_});
    }
}

void ArrayLiteralSnapshotInfo::StoreDataToGlobalData(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                                     SnapshotGlobalData &globalData,
                                                     const std::set<uint32_t> &skippedMethods)
{
    JSThread *thread = vm->GetJSThread();
    for (auto item : info_) {
        const ItemData &data = item.second;
        JSHandle<ConstantPool> cp(thread,
            thread->GetCurrentEcmaContext()->FindConstpool(jsPandaFile, data.constantPoolId_));
        panda_file::File::EntityId id = cp->GetEntityId(data.constantPoolIdx_);
        JSHandle<TaggedArray> literal = LiteralDataExtractor::GetDatasIgnoreType(
            thread, jsPandaFile, id, cp, data.recordName_);

        uint32_t snapshotCpArrIdx = globalData.GetCpArrIdxByConstanPoolId(data.constantPoolId_);
        JSHandle<TaggedArray> snapshotCpArr(thread, globalData.GetCurSnapshotCpArray());
        JSHandle<ConstantPool> snapshotCp(thread, snapshotCpArr->Get(snapshotCpArrIdx));
        JSHandle<JSTaggedValue> ihc = thread->GlobalConstants()->GetHandledUndefined();
        JSHandle<JSTaggedValue> chc = thread->GlobalConstants()->GetHandledUndefined();
        CollectLiteralInfo(vm, literal, data.constantPoolIdx_, snapshotCp, skippedMethods, ihc, chc);
        globalData.RecordReviseData(
            ReviseData::ItemData {globalData.GetCurDataIdx(), snapshotCpArrIdx, data.constantPoolIdx_});
    }
}

void SnapshotConstantPoolData::Record(const BytecodeInstruction &bcIns, int32_t bcIndex,
                                      const CString &recordName, const MethodLiteral *method)
{
    BytecodeInstruction::Opcode opcode = static_cast<BytecodeInstruction::Opcode>(bcIns.GetOpcode());
    uint32_t methodOffset = method->GetMethodId().GetOffset();
    panda_file::IndexAccessor indexAccessor(*jsPandaFile_->GetPandaFile(),
                                            panda_file::File::EntityId(methodOffset));
    uint32_t constantPoolId = static_cast<uint32_t>(indexAccessor.GetHeaderIndex());

    switch (opcode) {
        case BytecodeInstruction::Opcode::LDA_STR_ID16:
        case BytecodeInstruction::Opcode::STOWNBYNAME_IMM8_ID16_V8:
        case BytecodeInstruction::Opcode::STOWNBYNAME_IMM16_ID16_V8:
        case BytecodeInstruction::Opcode::CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8:
        case BytecodeInstruction::Opcode::CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8:
        case BytecodeInstruction::Opcode::STCONSTTOGLOBALRECORD_IMM16_ID16:
        case BytecodeInstruction::Opcode::TRYLDGLOBALBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::TRYLDGLOBALBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::TRYSTGLOBALBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::TRYSTGLOBALBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::STTOGLOBALRECORD_IMM16_ID16:
        case BytecodeInstruction::Opcode::STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8:
        case BytecodeInstruction::Opcode::STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8:
        case BytecodeInstruction::Opcode::LDTHISBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::LDTHISBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::STTHISBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::STTHISBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::LDGLOBALVAR_IMM16_ID16:
        case BytecodeInstruction::Opcode::LDOBJBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::LDOBJBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::STOBJBYNAME_IMM8_ID16_V8:
        case BytecodeInstruction::Opcode::STOBJBYNAME_IMM16_ID16_V8:
        case BytecodeInstruction::Opcode::LDSUPERBYNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::LDSUPERBYNAME_IMM16_ID16:
        case BytecodeInstruction::Opcode::STSUPERBYNAME_IMM8_ID16_V8:
        case BytecodeInstruction::Opcode::STSUPERBYNAME_IMM16_ID16_V8:
        case BytecodeInstruction::Opcode::STGLOBALVAR_IMM16_ID16:
        case BytecodeInstruction::Opcode::LDBIGINT_ID16: {
            auto constantPoolIdx = bcIns.GetId().AsRawValue();
            BaseSnapshotInfo::ItemData itemData = {recordName, constantPoolId, constantPoolIdx, methodOffset, bcIndex};
            RecordInfo(Type::STRING, itemData);
            break;
        }
        case BytecodeInstruction::Opcode::DEFINEFUNC_IMM8_ID16_IMM8:
        case BytecodeInstruction::Opcode::DEFINEFUNC_IMM16_ID16_IMM8:
        case BytecodeInstruction::Opcode::DEFINEMETHOD_IMM8_ID16_IMM8:
        case BytecodeInstruction::Opcode::DEFINEMETHOD_IMM16_ID16_IMM8: {
            auto constantPoolIdx = bcIns.GetId().AsRawValue();
            BaseSnapshotInfo::ItemData itemData = {recordName, constantPoolId, constantPoolIdx, methodOffset, bcIndex};
            RecordInfo(Type::METHOD, itemData);
            break;
        }
        case BytecodeInstruction::Opcode::CREATEOBJECTWITHBUFFER_IMM8_ID16:
        case BytecodeInstruction::Opcode::CREATEOBJECTWITHBUFFER_IMM16_ID16: {
            auto constantPoolIdx = bcIns.GetId().AsRawValue();
            BaseSnapshotInfo::ItemData itemData = {recordName, constantPoolId, constantPoolIdx, methodOffset, bcIndex};
            RecordInfo(Type::OBJECT_LITERAL, itemData);
            break;
        }
        case BytecodeInstruction::Opcode::CREATEARRAYWITHBUFFER_IMM8_ID16:
        case BytecodeInstruction::Opcode::CREATEARRAYWITHBUFFER_IMM16_ID16: {
            auto constantPoolIdx = bcIns.GetId().AsRawValue();
            BaseSnapshotInfo::ItemData itemData = {recordName, constantPoolId, constantPoolIdx, methodOffset, bcIndex};
            RecordInfo(Type::ARRAY_LITERAL, itemData);
            break;
        }
        case BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8: {
            auto methodCPIdx = (bcIns.GetId <BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 0>()).AsRawValue();
            BaseSnapshotInfo::ItemData methodItemData = {recordName, constantPoolId,
                methodCPIdx, methodOffset, bcIndex};
            RecordInfo(Type::METHOD, methodItemData);

            auto literalCPIdx = (bcIns.GetId <BytecodeInstruction::Format::IMM8_ID16_ID16_IMM16_V8, 1>()).AsRawValue();
            BaseSnapshotInfo::ItemData literalItemData = {recordName, constantPoolId,
                literalCPIdx, methodOffset, bcIndex};
            RecordInfo(Type::CLASS_LITERAL, literalItemData);
            break;
        }
        case BytecodeInstruction::Opcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8: {
            auto methodCPIdx = (bcIns.GetId <BytecodeInstruction::Format::IMM16_ID16_ID16_IMM16_V8, 0>()).AsRawValue();
            BaseSnapshotInfo::ItemData methodItemData = {recordName, constantPoolId,
                methodCPIdx, methodOffset, bcIndex};
            RecordInfo(Type::METHOD, methodItemData);

            auto literalCPIdx = (bcIns.GetId <BytecodeInstruction::Format::IMM16_ID16_ID16_IMM16_V8, 1>()).AsRawValue();
            BaseSnapshotInfo::ItemData literalItemData = {recordName, constantPoolId,
                literalCPIdx, methodOffset, bcIndex};
            RecordInfo(Type::CLASS_LITERAL, literalItemData);
            break;
        }
        default:
            break;
    }
}

void SnapshotConstantPoolData::StoreDataToGlobalData(SnapshotGlobalData &snapshotData,
                                                     const std::set<uint32_t> &skippedMethods) const
{
    for (auto &info : infos_) {
        info->StoreDataToGlobalData(vm_, jsPandaFile_, snapshotData, skippedMethods);
    }
}
}  // namespace panda::ecmascript
