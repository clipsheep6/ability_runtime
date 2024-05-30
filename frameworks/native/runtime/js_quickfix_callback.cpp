/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_quickfix_callback.h"

#include "file_path_utils.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "bundle_constants.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
    constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";

    constexpr char WIDGET_ABC_PATH[] = "/ets/widgets.abc";
}

std::string JsQuickfixCallback::getModuleName(const std::string &baseFileName)
{
    std::string moduleName;
    if (type_ == AbilityRuntime::RuntimeType::RUNTIME_APP) {
        int baseFileNameLen = static_cast<int>(baseFileName.length());
        int prefixLen = strlen(BUNDLE_INSTALL_PATH);
        int suffixLen = strlen(MERGE_ABC_PATH);
        int moduleLen = baseFileNameLen - prefixLen - suffixLen;
        if (moduleLen < 0) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "invalid baseFileName!");
            return "";
        }
        moduleName = baseFileName.substr(prefixLen, moduleLen);
    } else if (type_ == AbilityRuntime::RuntimeType::RUNTIME_FORM) {
        std::string fullPath = patchVersion_ + "/";
        size_t patchPos = baseFileName.find(fullPath);
        if (patchPos == std::string::npos) {
            HILOG_ERROR("invalid patchPos!");
            return "";
        }
        size_t endPos = baseFileName.find(WIDGET_ABC_PATH);
        if (endPos == std::string::npos) {
            HILOG_ERROR("invalid endPos!");
            return "";
        }
        moduleName = baseFileName.substr(patchPos + fullPath.size(), endPos - patchPos - fullPath.size());
        HILOG_DEBUG("moduleName:%{public}s", moduleName.c_str());
    } else {
        HILOG_ERROR("not support runtime_type");
        return "";
    }
    return moduleName;
}

bool JsQuickfixCallback::operator()(std::string baseFileName, std::string &patchFileName,
                                    void **patchBuffer, size_t &patchSize)
{
    TAG_LOGD(AAFwkTag::JSRUNTIME, "baseFileName: %{private}s", baseFileName.c_str());
    auto position = baseFileName.find(".abc");
    if (position == std::string::npos) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "invalid baseFileName!");
        return false;
    }
    std::string moduleName = getModuleName(baseFileName);
    TAG_LOGD(AAFwkTag::JSRUNTIME, "moduleName: %{private}s", moduleName.c_str());
    if (moduleName.empty()) {
        return false;
    }
    
    auto it = moduleAndHqfPath_.find(moduleName);
    if (it == moduleAndHqfPath_.end()) {
        return false;
    }

    std::string resolvedHqfFile;
    std::string hqfFile = it->second;
    if (type_ == AbilityRuntime::RuntimeType::RUNTIME_APP) {
        resolvedHqfFile = AbilityBase::GetLoadPath(hqfFile);
    } else if (type_ == AbilityRuntime::RuntimeType::RUNTIME_FORM) {
        resolvedHqfFile = hqfFile;
    }

    TAG_LOGD(AAFwkTag::JSRUNTIME, "hqfFile: %{private}s, resolvedHqfFile: %{private}s", hqfFile.c_str(),
        resolvedHqfFile.c_str());
    
    if (!JsRuntime::GetFileBuffer(resolvedHqfFile, patchFileName, newpatchBuffer_)) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "GetFileBuffer failed");
        return false;
    }
    *patchBuffer = newpatchBuffer_.data();
    TAG_LOGD(AAFwkTag::JSRUNTIME, "patchFileName: %{private}s", patchFileName.c_str());
    patchSize = newpatchBuffer_.size();
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
