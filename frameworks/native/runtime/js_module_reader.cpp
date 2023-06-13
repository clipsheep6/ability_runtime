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

#include "js_module_reader.h"

#include "bundle_info.h"
#include "bundle_mgr_proxy.h"
#include "file_path_utils.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "js_runtime_utils.h"
#include "system_ability_definition.h"

using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {
using IBundleMgr = AppExecFwk::IBundleMgr;

JsModuleReader::JsModuleReader(const std::string& bundleName, const std::string& hapPath) : JsModuleSearcher(bundleName)
{
    if (!hapPath.empty() && hapPath.find(std::string(SYS_ABS_CODE_PATH)) == 0) {
        isSystemPath_ = true;
    } else {
        isSystemPath_ = false;
    }
}

std::vector<uint8_t> JsModuleReader::operator()(const std::string& inputPath) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<uint8_t> buffer;
    if (inputPath.empty()) {
        HILOG_ERROR("inputPath is empty");
        return buffer;
    }
    std::string realHapPath;
    std::string suffix = std::string(SHARED_FILE_SUFFIX);
    if (isSystemPath_) {
        realHapPath = GetPresetAppHapPath(inputPath);
    } else {
        realHapPath = std::string(ABS_CODE_PATH) + inputPath + suffix;
    }
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        HILOG_ERROR("failed to obtain realHapPath");
        return buffer;
    }
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(realHapPath, newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("realHapPath %{private}s GetExtractor failed", realHapPath.c_str());
        return buffer;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    size_t len = 0;
    if (!extractor->ExtractToBufByName(MERGE_ABC_PATH, dataPtr, len)) {
        HILOG_ERROR("get mergeAbc fileBuffer failed");
        return buffer;
    }
    buffer.assign(dataPtr.get(), dataPtr.get() + len);
    return buffer;
}

std::string JsModuleReader::GetPresetAppHapPath(const std::string& inputPath) const
{
    std::string presetAppHapPath;
    std::string moudleName = inputPath.substr(inputPath.find_last_of("/") + 1);
    if (moudleName.empty()) {
        HILOG_ERROR("failed to obtain moudleName.");
        return presetAppHapPath;
    }
    auto systemAbilityManagerClient = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManagerClient) {
        HILOG_ERROR("fail to get system ability mgr.");
        return presetAppHapPath;
    }
    auto remoteObject = systemAbilityManagerClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOG_ERROR("fail to get bundle manager proxy.");
        return presetAppHapPath;
    }
    auto bundleMgrProxy = iface_cast<IBundleMgr>(remoteObject);
    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::
        GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0 || bundleInfo.hapModuleInfos.size() == 0) {
        HILOG_ERROR("GetBundleInfoForSelf failed.");
        return presetAppHapPath;
    }
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        if (hapModuleInfo.moduleName == moudleName) {
            presetAppHapPath = hapModuleInfo.hapPath;
            break;
        }
    }
    return presetAppHapPath;
}
} // namespace AbilityRuntime
} // namespace OHOS