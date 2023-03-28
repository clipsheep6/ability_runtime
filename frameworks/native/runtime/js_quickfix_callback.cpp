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

#include "js_quickfix_callback.h"

#include "file_path_utils.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {

constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
bool JsQuickfixCallback::operator()(std::string baseFileName, std::string &patchFileName,
                                    void **patchBuffer, size_t &patchSize) 
{
    // 1.baseFileName = /data/storage/el1/bundle/${moduleName}/ets/modules.abc.
    HILOG_DEBUG("zhuhan====baseFileName: %{public}s", baseFileName.c_str());
    int baseFileNameLen = baseFileName.length();
    int prefixLen = strlen(BUNDLE_INSTALL_PATH);
    int suffixLen = strlen(MERGE_ABC_PATH);
    std::string moduleName = baseFileName.substr(prefixLen, baseFileNameLen - prefixLen - suffixLen);

    HILOG_DEBUG("zhuhan====moduleName: %{public}s", moduleName.c_str());

    // 2.根据moudleName获取到patchFileName，补丁路径。hqfInfos：moudelName， hqfFile
    auto it = moduleAndHqfPath_.find(moduleName);
    if(it == moduleAndHqfPath_.end()) {
        HILOG_ERROR("zhuhan====moduleName: %{public}s", moduleName.c_str());
        return false;
    }
    std::string hqfFile = it->second;

    HILOG_DEBUG("zhuhan====hqfFile: %{public}s", hqfFile.c_str());
    std::string resolvedHqfFile(AbilityBase::GetLoadPath(hqfFile));
    HILOG_DEBUG("zhuhan====hqfFile: %{public}s", resolvedHqfFile.c_str());
    
    if (!JsRuntime::GetFileBuffer(resolvedHqfFile, patchFileName, newpatchBuffer_)) {
        HILOG_ERROR("zhuhan====GetFileBuffer failed");
        return false;
    }
    *patchBuffer = newpatchBuffer_.data();
    HILOG_DEBUG("zhuhan====patchFileName: %{public}s", patchFileName.c_str());
    patchSize = newpatchBuffer_.size();
    HILOG_DEBUG("zhuhan====");
    return true;
}


} // namespace AbilityRuntime
} // namespace OHOS
