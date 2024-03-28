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

#include "resident_process_manager.h"

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "exit_resident_process_info"
#include "user_controller.h"

namespace OHOS {
namespace AAFwk {
ResidentProcessManager::ResidentProcessManager()
{}

ResidentProcessManager::~ResidentProcessManager()
{}

void ResidentProcessManager::StartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    DelayedSingleton<AppScheduler>::GetInstance()->StartupResidentProcess(bundleInfos);
}

void ResidentProcessManager::StartResidentProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    std::set<uint32_t> needEraseIndexSet;

    for (size_t i = 0; i < bundleInfos.size(); i++) {
        std::string processName = bundleInfos[i].applicationInfo.process;
        if (!bundleInfos[i].isKeepAlive || processName.empty()) {
            needEraseIndexSet.insert(i);
            continue;
        }
        for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
            std::string mainElement;
            if (!CheckMainElement(hapModuleInfo, processName, mainElement, needEraseIndexSet, i)) {
                continue;
            }

            needEraseIndexSet.insert(i);
            // startAbility
            Want want;
            want.SetElementName(hapModuleInfo.bundleName, mainElement);
            HILOG_INFO("Start resident ability, mainElement: %{public}s", mainElement.c_str());
            DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, USER_ID_NO_HEAD,
                DEFAULT_INVAL_VALUE);
        }
    }

    // delete item which process has been started.
    for (auto iter = needEraseIndexSet.rbegin(); iter != needEraseIndexSet.rend(); iter++) {
        bundleInfos.erase(bundleInfos.begin() + *iter);
    }
}

bool ResidentProcessManager::CheckMainElement(const AppExecFwk::HapModuleInfo &hapModuleInfo,
    const std::string &processName, std::string &mainElement,
    std::set<uint32_t> &needEraseIndexSet, size_t bundleInfoIndex)
{
    if (!hapModuleInfo.isModuleJson) {
        // old application model
        mainElement = hapModuleInfo.mainAbility;
        if (mainElement.empty()) {
            return false;
        }

        // old application model, use ability 'process'
        bool isAbilityKeepAlive = false;
        for (auto abilityInfo : hapModuleInfo.abilityInfos) {
            if (abilityInfo.process != processName || abilityInfo.name != mainElement) {
                continue;
            }
            isAbilityKeepAlive = true;
        }
        if (!isAbilityKeepAlive) {
            return false;
        }

        std::string uriStr;
        bool getDataAbilityUri = DelayedSingleton<AbilityManagerService>::GetInstance()->GetDataAbilityUri(
            hapModuleInfo.abilityInfos, mainElement, uriStr);
        if (getDataAbilityUri) {
            // dataability, need use AcquireDataAbility
            HILOG_INFO("Start resident dataability, mainElement: %{public}s, uri: %{public}s",
                mainElement.c_str(), uriStr.c_str());
            Uri uri(uriStr);
            DelayedSingleton<AbilityManagerService>::GetInstance()->AcquireDataAbility(uri, true, nullptr);
            needEraseIndexSet.insert(bundleInfoIndex);
            return false;
        }
    } else {
        // new application model
        mainElement = hapModuleInfo.mainElementName;
        if (mainElement.empty()) {
            return false;
        }

        // new application model, user model 'process'
        if (hapModuleInfo.process != processName) {
            return false;
        }
    }

    // ability need to start, but need to filt page ability
    bool mainElementIsPageAbility = false;
    for (auto abilityInfo : hapModuleInfo.abilityInfos) {
        if (abilityInfo.name == mainElement && abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
            mainElementIsPageAbility = true;
            break;
        }
    }
    if (mainElementIsPageAbility) {
        HILOG_DEBUG("%{public}s is page ability", mainElement.c_str());
        return false;
    }

    return true;
}

void ResidentProcessManager::SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    taskHandler_ = taskHandler;
}

int32_t ResidentProcessManager::HandleMemorySizeSufficent()
{
    if (ExitResidentProcessInfo::GetInstance()->IsMemorySizeSufficent()) {
        HILOG_INFO("memory size is sufficent");
        return ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    ExitResidentProcessInfo::GetInstance()->SetCurrentMemorySizeState(MemorySizeState::MEMORY_SIZE_SUFFICENT);
    auto StartExitKeepAliveProcessTask = [residentProcessManagerWeak = weak_from_this()]() {
        auto residentProcessManager = residentProcessManagerWeak.lock();
        if (residentProcessManager == nullptr) {
            return;
        }
        std::vector<std::string> exitBundleNames;
        ExitResidentProcessInfo::GetInstance()->GetExitResidentBundleNames(exitBundleNames);
        std::vector<AppExecFwk::BundleInfo> bundleInfos;
        for (const std::string& bundleName:exitBundleNames) {   
            if (!IN_PROCESS_CALL(bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, 
                bundleInfo, U0_USER_ID))) {
                HILOG_ERROR("Failed to get bundle info from %{public}s.", bundleName.c_str());
                continue;
            }
            if (!bundleInfo.isKeepAlive) {
                HILOG_ERROR("Not a resident application.");
                continue;
            }
            bundleInfos.emplace_back(bundleInfo);  
        }
        ExitResidentProcessInfo::GetInstance()->ClearExitResidentBundleNames();
        residentProcessManager->StartResidentProcessWithMainElement(bundleInfos);

        if (!bundleInfos.empty()) {
            residentProcessManager->StartResidentProcess(bundleInfos);
        }
    };
    taskHandler_->SubmitTask(StartExitKeepAliveProcessTask, "startexitkeepaliveprocess");
    return ERR_OK;
}

int32_t ResidentProcessManager::HandleMemorySizeInSufficent()
{
    if (!ExitResidentProcessInfo::GetInstance()->IsMemorySizeSufficent()) {
        HILOG_INFO("memory size is insufficent");
        return ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED;
    }
    ExitResidentProcessInfo::GetInstance()->SetCurrentMemorySizeState(MemorySizeState::MEMORY_SIZE_INSUFFICENT);
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
