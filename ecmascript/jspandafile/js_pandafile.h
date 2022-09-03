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

#ifndef ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H
#define ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H

#include "ecmascript/common.h"
#include "ecmascript/js_function.h"
#include "ecmascript/jspandafile/constpool_value.h"
#include "ecmascript/jspandafile/method_literal.h"
#include "ecmascript/mem/c_containers.h"

#include "libpandafile/file.h"
namespace panda {
namespace ecmascript {
class JSPandaFile {
public:
    struct JSRecordInfo {
        uint32_t mainMethodIndex {0};
        bool isCjs {false};
        int moduleRecordIdx {-1};
    };
    static constexpr char ENTRY_FUNCTION_NAME[] = "func_main_0";
    static constexpr char ENTRY_MAIN_FUNCTION[] = "_GLOBAL::func_main_0";
    static constexpr char PATCH_ENTRY_FUNCTION[] = "_GLOBAL::patch_main_0";

    static constexpr char MODULE_CLASS[] = "L_ESModuleRecord;";
    static constexpr char TS_TYPES_CLASS[] = "L_ESTypeInfoRecord;";
    static constexpr char COMMONJS_CLASS[] = "L_CommonJsRecord;";
    static constexpr char TYPE_FLAG[] = "typeFlag";
    static constexpr char TYPE_SUMMARY_INDEX[] = "typeSummaryIndex";

    static constexpr char IS_COMMON_JS[] = "isCommonjs";
    static constexpr char MODULE_RECORD_IDX[] = "moduleRecordIdx";
    static constexpr char MODULE_DEFAULE_ETS[] = "ets/";
    static constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
    static constexpr char MERGE_ABC_PATH[] = "/data/storage/el1/bundle/entry/ets/modules.abc";
    static constexpr char NODE_MODULES[] = "node_modules";
    static constexpr char NODE_MODULES_ZERO[] = "node_modules/0/";
    static constexpr char NODE_MODULES_ONE[] = "node_modules/1/";

    JSPandaFile(const panda_file::File *pf, const CString &descriptor);
    ~JSPandaFile();

    const CString &GetJSPandaFileDesc() const
    {
        return desc_;
    }

    const panda_file::File *GetPandaFile() const
    {
        return pf_;
    }

    MethodLiteral* GetMethodLiterals() const
    {
        return methodLiterals_;
    }

    void SetMethodLiteralToMap(MethodLiteral *methodLiteral)
    {
        if (methodLiteral != nullptr) {
            methodLiteralMap_.emplace(methodLiteral->GetMethodId().GetOffset(), methodLiteral);
        }
    }

    const CUnorderedMap<uint32_t, MethodLiteral *> &GetMethodLiteralMap() const
    {
        return methodLiteralMap_;
    }

    uint32_t GetNumMethods() const
    {
        return numMethods_;
    }

    uint32_t GetConstpoolIndex() const
    {
        return constpoolIndex_;
    }

    uint32_t GetMainMethodIndex(const CString &recordName = ENTRY_FUNCTION_NAME) const
    {
        auto info = jsRecordInfo_.find(recordName);
        if (info != jsRecordInfo_.end()) {
            return info->second.mainMethodIndex;
        }
        return 0;
    }

    const CUnorderedMap<uint32_t, uint64_t> &GetConstpoolMap() const
    {
        return constpoolMap_;
    }

    uint32_t PUBLIC_API GetOrInsertConstantPool(ConstPoolType type, uint32_t offset);

    void UpdateMainMethodIndex(uint32_t mainMethodIndex, const CString &recordName = ENTRY_FUNCTION_NAME)
    {
        auto info = jsRecordInfo_.find(recordName);
        if (info != jsRecordInfo_.end()) {
            info->second.mainMethodIndex = mainMethodIndex;
        }
    }

    MethodLiteral* PUBLIC_API FindMethodLiteral(uint32_t offset) const;

    int GetModuleRecordIdx(const CString &recordName = ENTRY_FUNCTION_NAME) const
    {
        auto info = jsRecordInfo_.find(recordName);
        if (info != jsRecordInfo_.end()) {
            return info->second.moduleRecordIdx;
        }
        // The array subscript will not have a negative number, and returning -1 means the search failed
        return -1;
    }

    Span<const uint32_t> GetClasses() const
    {
        return pf_->GetClasses();
    }

    bool PUBLIC_API IsModule(const CString &recordName = ENTRY_FUNCTION_NAME) const;

    bool IsCjs(const CString &recordName = ENTRY_FUNCTION_NAME) const;

    bool IsBundle() const
    {
        return isBundle_;
    }

    bool HasTSTypes() const
    {
        return hasTSTypes_;
    }

    void SetLoadedAOTStatus(bool status)
    {
        isLoadedAOT_ = status;
    }

    bool IsLoadedAOT() const
    {
        return isLoadedAOT_;
    }

    uint32_t GetTypeSummaryIndex() const
    {
        return typeSummaryIndex_;
    }

    uint32_t GetFileUniqId() const
    {
        return static_cast<uint32_t>(GetPandaFile()->GetUniqId());
    }

    bool HasRecord(const CString &recordName) const
    {
        auto info = jsRecordInfo_.find(recordName);
        if (info != jsRecordInfo_.end()) {
            return true;
        }
        return false;
    }
    void checkIsBundle();
    CString FindrecordName(const CString &record) const;
    static std::string ParseOhmUrl(std::string fileName);
private:
    void InitializeUnMergedPF();
    void InitializeMergedPF();
    uint32_t constpoolIndex_ {0};
    CUnorderedMap<uint32_t, uint64_t> constpoolMap_;
    uint32_t numMethods_ {0};
    MethodLiteral *methodLiterals_ {nullptr};
    CUnorderedMap<uint32_t, MethodLiteral *> methodLiteralMap_;
    const panda_file::File *pf_ {nullptr};
    CString desc_;
    bool hasTSTypes_ {false};
    bool isLoadedAOT_ {false};
    uint32_t typeSummaryIndex_ {0};

    // marge abc
    bool isBundle_ {true}; // isBundle means app compile mode is JSBundle
    CUnorderedMap<CString, JSRecordInfo> jsRecordInfo_;
};
}  // namespace ecmascript
}  // namespace panda
#endif // ECMASCRIPT_JSPANDAFILE_JS_PANDAFILE_H
