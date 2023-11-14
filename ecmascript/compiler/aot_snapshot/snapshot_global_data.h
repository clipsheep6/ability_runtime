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
#ifndef ECMASCRIPT_COMPILER_AOT_SNAPSHOT_SNAPSHOT_GLOBAL_DATA_H
#define ECMASCRIPT_COMPILER_AOT_SNAPSHOT_SNAPSHOT_GLOBAL_DATA_H

#include "ecmascript/ecma_vm.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/compiler/aot_snapshot/aot_snapshot_constants.h"

namespace panda::ecmascript::kungfu {
class SnapshotGlobalData;
/*
 * The information that needs to be revised before saving the 'ai' file is recorded in SnapshotReviseData.
 * Currently, the revised information includes the entry index of each method in the 'an' file.
 */
class ReviseData {
public:
    struct ItemData {
        uint32_t dataIdx_;
        uint32_t cpArrayIdx_;
        int32_t constpoolIdx_;
    };

    ReviseData() = default;
    virtual ~ReviseData() = default;

    void Record(ItemData data)
    {
        data_.emplace_back(data);
    }

    void Resolve(JSThread *thread, const SnapshotGlobalData *globalData,
                 const CMap<std::pair<std::string, uint32_t>, uint32_t> &methodToEntryIndexMap);

protected:
    JSHandle<ConstantPool> GetConstantPoolFromSnapshotData(JSThread *thread, const SnapshotGlobalData *globalData,
                                                           uint32_t dataIdx, uint32_t cpArrayIdx);
    std::vector<ItemData> data_;
};

class SnapshotReviseInfo {
public:
    SnapshotReviseInfo() = default;
    ~SnapshotReviseInfo() = default;

    void Record(ReviseData::ItemData data)
    {
        reviseData_.Record(data);
    }

    void ResolveData(JSThread *thread, const SnapshotGlobalData *globalData,
                     const CMap<std::pair<std::string, uint32_t>, uint32_t> &methodToEntryIndexMap)
    {
        reviseData_.Resolve(thread, globalData, methodToEntryIndexMap);
    }

private:
    ReviseData reviseData_ {};
};

class SnapshotGlobalData {
public:
    static constexpr uint32_t CP_ARRAY_OFFSET = 1;

    SnapshotGlobalData() = default;
    ~SnapshotGlobalData() = default;

    void Iterate(const RootVisitor &v)
    {
        v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&data_)));
        v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&curSnapshotCpArray_)));
        v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&hclassInfo_)));
        v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&arrayInfo_)));
        v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&constantIndexInfo_)));
    }

    void SetData(JSTaggedValue data)
    {
        data_ = data;
    }

    JSTaggedValue GetData() const
    {
        return data_;
    }

    uint32_t GetCurDataIdx() const
    {
        return curDataIdx_;
    }

    JSTaggedValue GetCurSnapshotCpArray() const
    {
        return curSnapshotCpArray_;
    }

    void AddSnapshotCpArrayToData(JSThread *thread, CString fileName, JSHandle<TaggedArray> snapshotCpArray);

    CString GetFileNameByDataIdx(uint32_t dataIdx) const;

    void RecordReviseData(ReviseData::ItemData data)
    {
        reviseInfo_.Record(data);
    }

    void ResolveSnapshotData(JSThread *thread,
                             const CMap<std::pair<std::string, uint32_t>, uint32_t> &methodToEntryIndexMap)
    {
        reviseInfo_.ResolveData(thread, this, methodToEntryIndexMap);
    }

    void RecordCpArrIdx(int32_t constantPoolId, uint32_t cpArrIdx)
    {
        dataIdxToCpArrIdxMap_[curDataIdx_][constantPoolId] = cpArrIdx;
    }

    uint32_t GetCpArrIdxByConstanPoolId(int32_t constantPoolId)
    {
        return GetCpIdToCpArrIdxMap().at(constantPoolId);
    }

    const CUnorderedMap<int32_t, uint32_t>& GetCpIdToCpArrIdxMap()
    {
        return dataIdxToCpArrIdxMap_.at(curDataIdx_);
    }

    JSTaggedValue GetHClassInfo()
    {
        return hclassInfo_;
    }

    JSTaggedValue GetArrayInfo()
    {
        return arrayInfo_;
    }

    JSTaggedValue GetConstantIndexInfo()
    {
        return constantIndexInfo_;
    }

    void StoreHClassInfo(JSHandle<TaggedArray> info)
    {
        hclassInfo_ = info.GetTaggedValue();
    }

    void StoreArrayInfo(JSHandle<TaggedArray> info)
    {
        arrayInfo_ = info.GetTaggedValue();
    }

    void StoreConstantIndexInfo(JSHandle<TaggedArray> info)
    {
        constantIndexInfo_ = info.GetTaggedValue();
    }

private:
    using CpIdToCpArrIdxMap = CUnorderedMap<int32_t, uint32_t>;

    bool isFirstData_ {true};
    uint32_t curDataIdx_ {0};
    JSTaggedValue data_ {JSTaggedValue::Hole()};
    JSTaggedValue curSnapshotCpArray_ {JSTaggedValue::Hole()};
    CUnorderedMap<uint32_t, CpIdToCpArrIdxMap> dataIdxToCpArrIdxMap_;
    CUnorderedMap<uint32_t, CString> dataIdxToFileNameMap_ {};

    SnapshotReviseInfo reviseInfo_;
    JSTaggedValue hclassInfo_ {JSTaggedValue::Hole()};
    JSTaggedValue arrayInfo_ {JSTaggedValue::Hole()};
    JSTaggedValue constantIndexInfo_ {JSTaggedValue::Hole()};
};
}  // panda::ecmascript::kungfu
#endif // ECMASCRIPT_COMPILER_AOT_SNAPSHOT_SNAPSHOT_GLOBAL_DATA_H
