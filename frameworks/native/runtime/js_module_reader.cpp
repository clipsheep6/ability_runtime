/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "file_path_utils.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "js_runtime_utils.h"
#include "singleton.h"
#include "system_ability_definition.h"

using namespace OHOS::AbilityBase;

namespace OHOS {
namespace AbilityRuntime {
using IBundleMgr = AppExecFwk::IBundleMgr;

JsModuleReader::JsModuleReader(const std::string& bundleName, const std::string& hapPath, bool isFormRender)
    : JsModuleSearcher(bundleName), isFormRender_(isFormRender)
{
    if (!hapPath.empty() && hapPath.find(std::string(ABS_DATA_CODE_PATH)) != 0) {
        isSystemPath_ = true;
    } else {
        isSystemPath_ = false;
    }
}

bool JsModuleReader::operator()(const std::string& inputPath, uint8_t **buff, size_t *buffSize) const
{
    HILOG_INFO("JsModuleReader operator start: %{private}s", inputPath.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
        HILOG_ERROR("Invalid param");
        return false;
    }

    auto realHapPath = GetAppHspPath(inputPath);
    if (realHapPath.empty()) {
        HILOG_ERROR("realHapPath is empty");
        return false;
    }

    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(realHapPath, newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("realHapPath %{private}s GetExtractor failed", realHapPath.c_str());
        return false;
    }

    auto data = extractor->GetSafeData(MERGE_ABC_PATH);
    if (!data) {
        HILOG_ERROR("get mergeAbc fileBuffer failed");
        return false;
    }

    *buff = data->GetDataPtr();
    *buffSize = data->GetDataLen();
    return true;
}

std::string JsModuleReader::GetAppHspPath(const std::string& inputPath) const
{
    if (isFormRender_) {
        return GetFormAppHspPath(inputPath);
    }
    return GetCommonAppHspPath(inputPath);
}

std::string JsModuleReader::GetFormAppHspPath(const std::string& inputPath) const
{
    std::string realHapPath;
    std::string suffix = std::string(SHARED_FILE_SUFFIX);
    realHapPath.append("/data/bundles/")
        .append(bundleName_).append("/")
        .append(GetModuleName(inputPath))
        .append(SHARED_FILE_SUFFIX);

    HILOG_INFO("realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        HILOG_ERROR("failed to obtain realHapPath");
        return realHapPath;
    }
    return realHapPath;
}

std::string JsModuleReader::GetModuleName(const std::string& inputPath) const
{
    return inputPath.substr(inputPath.find_last_of("/") + 1);
}

std::string JsModuleReader::GetCommonAppHspPath(const std::string& inputPath) const
{
    std::string suffix = std::string(SHARED_FILE_SUFFIX);
    std::string realHapPath = GetPresetAppHapPath(inputPath, bundleName_);
    if ((realHapPath.find(ABS_DATA_CODE_PATH) == 0) || (realHapPath == inputPath)) {
        realHapPath = std::string(ABS_CODE_PATH) + inputPath + suffix;
    }

    HILOG_INFO("realHapPath: %{private}s", realHapPath.c_str());
    if (realHapPath.empty() ||
        realHapPath.length() < suffix.length() ||
        realHapPath.compare(realHapPath.length() - suffix.length(), suffix.length(), suffix) != 0) {
        HILOG_ERROR("failed to obtain realHapPath");
        return realHapPath;
    }
    return realHapPath;
}

std::string JsModuleReader::GetPresetAppHapPath(const std::string& inputPath, const std::string& bundleName)
{
    std::string presetAppHapPath = inputPath;
    std::string moduleName = inputPath.substr(inputPath.find_last_of("/") + 1);
    if (moduleName.empty()) {
        HILOG_ERROR("Failed to obtain moduleName.");
        return presetAppHapPath;
    }
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return presetAppHapPath;
    }
    if (inputPath.find_first_of("/") == inputPath.find_last_of("/")) {
        AppExecFwk::BundleInfo bundleInfo;
        auto getInfoResult = bundleMgrHelper->GetBundleInfoForSelf(static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::
            GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
        if (getInfoResult != 0 || bundleInfo.hapModuleInfos.empty()) {
            HILOG_ERROR("GetBundleInfoForSelf failed.");
            return presetAppHapPath;
        }
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleName == moduleName) {
                presetAppHapPath = hapModuleInfo.hapPath;
                break;
            }
        }
    } else {
        std::vector<AppExecFwk::BaseSharedBundleInfo> baseSharedBundleInfos;
        if (bundleMgrHelper->GetBaseSharedBundleInfos(bundleName, baseSharedBundleInfos) != 0) {
            HILOG_ERROR("GetBaseSharedBundleInfos failed.");
            return presetAppHapPath;
        }
        std::string tmpPath = inputPath.substr(inputPath.find_first_of("/") + 1);
        const std::string sharedBundleName = tmpPath.substr(0, tmpPath.find_first_of("/"));
        for (const auto &info : baseSharedBundleInfos) {
            if ((info.bundleName == sharedBundleName) && (info.moduleName == moduleName)) {
                presetAppHapPath = info.hapPath;
                break;
            }
        }
    }
    return presetAppHapPath;
}

void JsModuleReader::GetHapPathList(const std::string &bundleName, std::vector<std::string> &hapList)
{
    auto systemAbilityManagerClient = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManagerClient) {
        HILOG_ERROR("fail to get system ability mgr.");
        return;
    }
    auto remoteObject = systemAbilityManagerClient->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!remoteObject) {
        HILOG_ERROR("fail to get bundle manager proxy.");
        return;
    }
    auto bundleMgrProxy = iface_cast<IBundleMgr>(remoteObject);
    AppExecFwk::BundleInfo bundleInfo;
    auto getInfoResult = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::
        GET_BUNDLE_INFO_WITH_HAP_MODULE), bundleInfo);
    if (getInfoResult != 0 || bundleInfo.hapModuleInfos.empty()) {
        HILOG_ERROR("GetBundleInfoForSelf failed.");
        return;
    }
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        hapList.emplace_back(hapModuleInfo.hapPath);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS