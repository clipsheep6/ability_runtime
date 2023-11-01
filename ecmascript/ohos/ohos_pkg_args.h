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

#ifndef ECMASCRIPT_OHOS_OHOS_PKG_ARGS_H
#define ECMASCRIPT_OHOS_OHOS_PKG_ARGS_H

#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ecmascript/ecma_vm.h"
#include "ecmascript/base/json_parser.h"
#include "ecmascript/compiler/aot_compiler_preprocessor.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/log.h"
#include "ecmascript/log_wrapper.h"
#include "ecmascript/mem/c_string.h"
#include "ecmascript/module/module_path_helper.h"
#include "ecmascript/ohos/white_list_helper.h"
#include "ecmascript/pgo_profiler/pgo_utils.h"
#include "ecmascript/platform/file.h"

namespace panda::ecmascript::kungfu {
class OhosPkgArgs {
public:
    constexpr static const char *const KEY_BUNDLE_NAME = "bundleName";
    constexpr static const char *const KEY_MODULE_NAME = "moduleName";
    constexpr static const char *const KEY_PKG_PATH = "pkgPath";
    constexpr static const char *const KEY_FILE_NAME = "abcName";
    constexpr static const char *const KEY_ABC_OFFSET = "abcOffset";
    constexpr static const char *const KEY_ABC_SIZE = "abcSize";
    constexpr static const char *const KEY_PGO_DIR = "pgoDir";

    OhosPkgArgs() = default;

    static bool ParseArgs(AotCompilerPreprocessor &preProcessor, CompilationOptions &cOptions)
    {
        ASSERT(preProcessor.runtimeOptions_.IsTargetCompilerMode());
        std::shared_ptr<OhosPkgArgs> pkgArgs = std::make_shared<OhosPkgArgs>();
        if (!preProcessor.runtimeOptions_.GetCompilerPkgJsonInfo().empty()) {
            if (!pkgArgs->ParseFromJson(preProcessor.vm_, preProcessor.runtimeOptions_.GetCompilerPkgJsonInfo())) {
                return false;
            }
            LOG_COMPILER(INFO) << "Parse main pkg info success.";
            preProcessor.mainPkgName_ = pkgArgs->GetFullName();
            preProcessor.pkgsArgs_[preProcessor.mainPkgName_] = pkgArgs;
            if (!ParseProfilerPath(pkgArgs, preProcessor, cOptions)) {
                return false;
            }
        }
        if (preProcessor.runtimeOptions_.GetCompilerEnableExternalPkg() &&
            !preProcessor.runtimeOptions_.GetCompilerExternalPkgJsonInfo().empty()) {
            OhosPkgArgs::ParseListFromJson(preProcessor.vm_,
                                           preProcessor.runtimeOptions_.GetCompilerExternalPkgJsonInfo(),
                                           preProcessor.pkgsArgs_);
        }
        for (const auto &pkgInfo : preProcessor.pkgsArgs_) {
            preProcessor.pandaFileNames_.emplace_back(pkgInfo.first);
            pkgInfo.second->Dump();
        }
        return true;
    }

    bool GetJSPandaFile(const JSRuntimeOptions &runtimeOptions, std::shared_ptr<JSPandaFile> &pf) const
    {
        std::string hapPath;
        uint32_t offset {};
        uint32_t size {};
        if (Valid()) {
            hapPath = GetPath();
            offset = GetOffset();
            size = GetSize();
        } else {
            // for legacy params
            hapPath = runtimeOptions.GetHapPath();
            offset = runtimeOptions.GetHapAbcOffset();
            size = runtimeOptions.GetHapAbcSize();
        }
        if (size == 0) {
            LOG_ECMA(ERROR) << "buffer is empty in target compiler mode!";
            return false;
        }
        std::string realPath;
        if (!RealPath(hapPath, realPath, false)) {
            LOG_ECMA(ERROR) << "realpath for hap path failed!";
            return false;
        }
        MemMap fileMapMem = FileMap(realPath.c_str(), FILE_RDONLY, PAGE_PROT_READ);
        if (fileMapMem.GetOriginAddr() == nullptr) {
            LOG_ECMA(ERROR) << "File mmap failed";
            return false;
        }
        uint8_t *buffer = reinterpret_cast<uint8_t *>(fileMapMem.GetOriginAddr()) + offset;
        JSPandaFileManager *jsPandaFileManager = JSPandaFileManager::GetInstance();
        pf = jsPandaFileManager->OpenJSPandaFileFromBuffer(buffer, size, GetFullName().c_str());
        FileUnMap(fileMapMem);
        fileMapMem.Reset();
        return true;
    }

    static bool ParseListFromJson(EcmaVM *vm, const std::string &jsonInfo,
                                  std::map<std::string, std::shared_ptr<OhosPkgArgs>> &argsMap)
    {
        LocalScope scope(vm);
        ObjectFactory *factory = vm->GetFactory();
        ecmascript::base::Utf8JsonParser parser(vm->GetJSThread());

        JSHandle<JSTaggedValue> handleMsg(factory->NewFromASCII(jsonInfo.c_str()));
        JSHandle<EcmaString> handleStr(JSTaggedValue::ToString(vm->GetAssociatedJSThread(), handleMsg));  // JSON Object
        JSHandle<JSTaggedValue> result = parser.Parse(*handleStr);
        JSTaggedValue resultValue(static_cast<JSTaggedType>(result->GetRawData()));
        if (!resultValue.IsArray(vm->GetJSThread())) {
            LOG_COMPILER(ERROR) << "Pkg list info parse failed. result is not an array. jsonData: " << jsonInfo.c_str();
            return false;
        }
        JSHandle<JSArray> valueHandle(vm->GetJSThread(), resultValue);
        JSHandle<TaggedArray> elements(vm->GetJSThread(), valueHandle->GetElements());
        for (uint32_t i = 0; i < elements->GetLength(); i++) {
            JSHandle<JSTaggedValue> entry(vm->GetJSThread(), elements->Get(i));
            if (entry->IsHole()) {
                continue;
            }
            std::shared_ptr<OhosPkgArgs> pkgInfo = std::make_shared<OhosPkgArgs>();
            JSTaggedValue entryValue(static_cast<JSTaggedType>(entry->GetRawData()));
            JSHandle<JSObject> entryHandle(vm->GetJSThread(), entryValue);
            if (!pkgInfo->ParseFromJsObject(vm, entryHandle)) {
                LOG_COMPILER(ERROR) << "Pkg list entry info parse failed. jsonData: " << jsonInfo.c_str();
                return false;
            }
            argsMap[pkgInfo->GetFullName()] = pkgInfo;
        }
        return true;
    }

    bool ParseFromJson(EcmaVM *vm, const std::string &jsonInfo)
    {
        LocalScope scope(vm);
        ObjectFactory *factory = vm->GetFactory();
        ecmascript::base::Utf8JsonParser parser(vm->GetJSThread());

        JSHandle<JSTaggedValue> handleMsg(factory->NewFromASCII(jsonInfo.c_str()));
        JSHandle<EcmaString> handleStr(JSTaggedValue::ToString(vm->GetAssociatedJSThread(), handleMsg));  // JSON Object
        JSHandle<JSTaggedValue> result = parser.Parse(*handleStr);
        JSTaggedValue resultValue(static_cast<JSTaggedType>(result->GetRawData()));
        if (!resultValue.IsECMAObject()) {
            LOG_COMPILER(ERROR) << "Pkg info parse failed. result is not an object. jsonData: " << jsonInfo.c_str();
            return false;
        }
        JSHandle<JSObject> valueHandle(vm->GetJSThread(), resultValue);
        return ParseFromJsObject(vm, valueHandle);
    }

    bool ParseFromJsObject(EcmaVM *vm, JSHandle<JSObject> &valueHandle)
    {
        LocalScope scope(vm);
        JSHandle<TaggedArray> nameList(JSObject::EnumerableOwnNames(vm->GetJSThread(), valueHandle));
        for (uint32_t i = 0; i < nameList->GetLength(); i++) {
            JSHandle<JSTaggedValue> key(vm->GetJSThread(), nameList->Get(i));
            JSHandle<JSTaggedValue> value = JSObject::GetProperty(vm->GetJSThread(), valueHandle, key).GetValue();
            if (!key->IsString() || !value->IsString()) {
                LOG_COMPILER(ERROR) << "Pkg info parse from js object failed. key and value must be string type.";
                return false;
            }
            UpdateProperty(ConvertToString(*JSTaggedValue::ToString(vm->GetJSThread(), key)).c_str(),
                           ConvertToString(*JSTaggedValue::ToString(vm->GetJSThread(), value)).c_str());
        }
        return Valid();
    }

    void UpdateProperty(const char *key, const char *value)
    {
        if (strcmp(key, KEY_BUNDLE_NAME) == 0) {
            bundleName_ = value;
        } else if (strcmp(key, KEY_MODULE_NAME) == 0) {
            moduleName_ = value;
        } else if (strcmp(key, KEY_PKG_PATH) == 0) {
            pkgPath_ = value;
        } else if (strcmp(key, KEY_FILE_NAME) == 0) {
            abcName_ = value;
        } else if (strcmp(key, KEY_ABC_OFFSET) == 0) {
            char *str = nullptr;
            abcOffset_ = static_cast<uint32_t>(strtol(value, &str, 0));
        } else if (strcmp(key, KEY_ABC_SIZE) == 0) {
            char *str = nullptr;
            abcSize_ = static_cast<uint32_t>(strtol(value, &str, 0));
        } else if (strcmp(key, KEY_PGO_DIR) == 0) {
            pgoDir_ = value;
        } else {
            LOG_COMPILER(ERROR) << "Unknown keyword when parse pkg info. key: " << key << ", value: " << value;
        }
    }

    bool Valid() const
    {
        if (!base::StringHelper::EndsWith(abcName_, ".abc")) {
            LOG_COMPILER(ERROR) << KEY_FILE_NAME << " must be abc file, but now is: " << abcName_;
            return false;
        }
        return !bundleName_.empty() && !moduleName_.empty() && !pkgPath_.empty() && (abcOffset_ != INVALID_VALUE) &&
               (abcSize_ != INVALID_VALUE);
    }

    void Dump() const
    {
        LOG_COMPILER(INFO) << "PkgInfo: "
                           << KEY_BUNDLE_NAME << ": " << bundleName_ << ", "
                           << KEY_MODULE_NAME << ": " << moduleName_ << ", "
                           << KEY_PKG_PATH << ": " << pkgPath_ << ", "
                           << KEY_ABC_OFFSET << ": " << std::hex << abcOffset_ << ", "
                           << KEY_ABC_SIZE << ": " << abcSize_ << ", "
                           << KEY_PGO_DIR << ": " << pgoDir_;
    }

    const std::string &GetBundleName() const
    {
        return bundleName_;
    }

    const std::string &GetModuleName() const
    {
        return moduleName_;
    }

    const std::string &GetPath() const
    {
        return pkgPath_;
    }

    std::string GetFullName() const
    {
        return pkgPath_ + GetPathSeparator() + moduleName_ + GetPathSeparator() + abcName_;
    }

    uint32_t GetOffset() const
    {
        return abcOffset_;
    }

    uint32_t GetSize() const
    {
        return abcSize_;
    }

    const std::string &GetPgoDir() const
    {
        return pgoDir_;
    }

    void SetPgoDir(const std::string &pgoDir)
    {
        pgoDir_ = pgoDir;
    }

    void GetPgoPaths(std::string &pgoPaths, bool &needMerge) const
    {
        pgoPaths.clear();
        needMerge = false;
        // 1. use target aps when app in white list
        if (WhiteListHelper::GetInstance()->IsEnable(bundleName_, moduleName_)) {
            pgoPaths = GetTargetApPaths();
        }
        if (!pgoPaths.empty()) {
            needMerge = true;
            return;
        }

        // 2. use baseline ap if there's no runtime ap
        auto baselineAp = pgoDir_ + '/' + pgo::ApNameUtils::GetOhosPkgApName(moduleName_);
        if (FileExist(baselineAp.c_str())) {
            pgoPaths = baselineAp;
        }
    }

    std::string GetRuntimeApPath() const
    {
        auto runtimeAp = pgoDir_ + '/' + pgo::ApNameUtils::GetRuntimeApName(moduleName_);
        if (!FileExist(runtimeAp.c_str())) {
            return "";
        }
        return runtimeAp;
    }

    std::string GetMergedApPathWithoutCheck() const
    {
        return pgoDir_ + '/' + pgo::ApNameUtils::GetMergedApName(moduleName_);
    }

    std::string GetMergedApPath() const
    {
        auto mergedAp = GetMergedApPathWithoutCheck();
        if (!FileExist(mergedAp.c_str())) {
            return "";
        }
        return mergedAp;
    }

private:
    static bool ParseProfilerPath(std::shared_ptr<OhosPkgArgs> &pkgArgs, AotCompilerPreprocessor &preProcessor,
                                  CompilationOptions &cOptions)
    {
        if (!preProcessor.runtimeOptions_.IsPartialCompilerMode()) {
            return true;
        }
        if (pkgArgs->GetPgoDir().empty() && !cOptions.profilerIn_.empty()) {
            // try get pgo dir from --compiler-pgo-profiler-path
            arg_list_t pandaFileNames = base::StringHelper::SplitString(cOptions.profilerIn_, GetFileDelimiter());
            ASSERT(!pandaFileNames.empty());
            // just parse the first ap's dir
            pkgArgs->SetPgoDir(ResolveDirPath(pandaFileNames.at(0)));
        }
        // reset profilerIn from pgo dir
        pkgArgs->GetPgoPaths(cOptions.profilerIn_, cOptions.needMerge_);
        if (cOptions.profilerIn_.empty()) {
            LOG_COMPILER(ERROR) << "No available ap files found in " << pkgArgs->GetPgoDir();
            return false;
        }
        return true;
    }

    /*
    * Before: xxx/xxx
    * After:  xxx
    */
    static std::string ResolveDirPath(const std::string &fileName)
    {
        // find last '/', '\\'
        auto foundPos = fileName.find_last_of("/\\");
        if (foundPos == std::string::npos) {
            return "";
        }
        return fileName.substr(0, foundPos);
    }

    std::string GetTargetApPaths() const
    {
        // handle merged ap
        std::string pgoPaths = GetMergedApPath();

        // handle runtime ap
        auto runtimeAp = GetRuntimeApPath();
        if (!runtimeAp.empty()) {
            if (!pgoPaths.empty()) {
                pgoPaths += GetFileDelimiter();
            }
            pgoPaths += runtimeAp;
        }
        return pgoPaths;
    }

    static constexpr uint32_t INVALID_VALUE = std::numeric_limits<uint32_t>::max();
    std::string bundleName_;
    std::string moduleName_;
    std::string pkgPath_;
    std::string abcName_;
    std::string pgoDir_;
    uint32_t abcOffset_ {INVALID_VALUE};
    uint32_t abcSize_ {INVALID_VALUE};
};
}  // namespace panda::ecmascript::kungfu
#endif