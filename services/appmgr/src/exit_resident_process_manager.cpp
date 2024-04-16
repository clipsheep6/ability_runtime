/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
}
ExitResidentProcessManager::~ExitResidentProcessInfo() {}

ExitResidentProcessManager::ExitResidentProcessInfo() {}

ExitResidentProcessManager &ExitResidentProcessManager::GetInstance()
{
    static ExitResidentProcessManager instance;
    return instance;
}

bool ExitResidentProcessManager::IsMemorySizeSufficent() const
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    return currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICENT;
}

bool ExitResidentProcessManager::RecordExitResidentBundleName(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICENT) {
        return false;
    }
    exitResidentBundleNames_.emplace_back(bundleName);
    return true;
}

int32_t ExitResidentProcessManager::HandleMemorySizeInSufficent()
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ != MemorySizeState::MEMORY_SIZE_SUFFICENT) {
        TAG_LOGE(AAFwkTag::APPMGR, "memory size is insufficent");
        return ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_INSUFFICENT;
    return ERR_OK;
}

int32_t ExitResidentProcessManager::HandleMemorySizeSufficent(std::vector<std::string>& bundleNames)
{
    std::lock_guard<ffrt::mutex> lock(mutexLock_);
    if (currentMemorySizeState_ == MemorySizeState::MEMORY_SIZE_SUFFICENT) {
        TAG_LOGE(AAFwkTag::APPMGR, "memory size is sufficent");
        return ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    bundleNames = exitResidentBundleNames_;
    exitResidentBundleNames_.clear();
    return ERR_OK;
}

void ExitResidentProcessManager::QueryExitBundleInfos(const std::vector<std::string>& exitBundleNames,
    std::vector<AppExecFwk::BundleInfo>& exitBundleInfos)
{
    AppExecFwk::BundleInfo bundleInfo;
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bms);
    for (const std::string& bundleName:exitBundleNames) {
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
            bundleInfo, U0_USER_ID))) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get bundle info from %{public}s.", bundleName.c_str());
            continue;
        }
        if (!bundleInfo.isKeepAlive) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not a resident application.");
            continue;
        }
        exitBundleInfos.emplace_back(bundleInfo);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
