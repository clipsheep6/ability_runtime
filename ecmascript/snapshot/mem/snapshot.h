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

#ifndef ECMASCRIPT_SNAPSHOT_MEM_SNAPSHOT_H
#define ECMASCRIPT_SNAPSHOT_MEM_SNAPSHOT_H

#include "ecmascript/compiler/aot_file/aot_version.h"
#include "ecmascript/common.h"
#include "ecmascript/mem/c_string.h"
#include "ecmascript/snapshot/mem/encode_bit.h"
#include "ecmascript/snapshot/mem/snapshot_processor.h"

#include "libpandabase/macros.h"

namespace panda::ecmascript {
class Program;
class EcmaVM;
class JSPandaFile;

class PUBLIC_API Snapshot {
public:
    explicit Snapshot(EcmaVM *vm) : vm_(vm) {}
    ~Snapshot() = default;

    void Serialize(const CString &fileName = "./snapshot");
    void Serialize(TaggedObject *objectHeader, const JSPandaFile *jsPandaFile, const CString &fileName = "./snapshot");
    void Serialize(uintptr_t startAddr, size_t size, const CString &fileName = "./snapshot");
    void SerializeBuiltins(const CString &fileName = "./snapshot");
    bool Deserialize(SnapshotType type, const CString &snapshotFile, bool isBuiltins = false);

protected:
    struct SnapShotHeader : public base::FileHeaderBase {
    public:
        explicit SnapShotHeader(const VersionType &lastVersion) : base::FileHeaderBase(lastVersion) {}

        bool Verify(const VersionType &lastVersion) const
        {
            return VerifyVersion("snapshot file", lastVersion, AOTFileVersion::AI_STRICT_MATCH);
        }

        uint32_t oldSpaceObjSize {0};
        uint32_t nonMovableObjSize {0};
        uint32_t machineCodeObjSize {0};
        uint32_t snapshotObjSize {0};
        uint32_t hugeObjSize {0};
        uint32_t stringSize {0};
        uint32_t pandaFileBegin {0};
        uint32_t rootObjectSize {0};
    };

    virtual const base::FileHeaderBase::VersionType &GetLastVersion() const
    {
        return AOTFileVersion::AI_VERSION;
    }

private:
    size_t AlignUpPageSize(size_t spaceSize);
    void WriteToFile(std::fstream &writer, const JSPandaFile *jsPandaFile, size_t size, SnapshotProcessor &processor);

    NO_MOVE_SEMANTIC(Snapshot);
    NO_COPY_SEMANTIC(Snapshot);

    EcmaVM *vm_;
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_SNAPSHOT_MEM_SNAPSHOT_H
