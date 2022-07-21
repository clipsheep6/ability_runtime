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

#include "ability_manager_service.h"
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
        if (!bundleInfos[i].isKeepAlive) {
            needEraseIndexSet.insert(i);
            continue;
        }
        for (auto hapModuleInfo : bundleInfos[i].hapModuleInfos) {
            std::string mainElement;
            if (!hapModuleInfo.isModuleJson) {
                // old application model
                mainElement = hapModuleInfo.mainAbility;
                if (mainElement.empty()) {
                    continue;
                }

                std::string uriStr;
                bool getDataAbilityUri = DelayedSingleton<AbilityManagerService>::GetInstance()->GetDataAbilityUri(
                    hapModuleInfo.abilityInfos, mainElement, uriStr);
                if (getDataAbilityUri) {
                    // dataability, need use AcquireDataAbility
                    Uri uri(uriStr);
                    DelayedSingleton<AbilityManagerService>::GetInstance()->AcquireDataAbility(uri, true, nullptr);
                    needEraseIndexSet.insert(i);
                    continue;
                }
            } else {
                // new application model
                mainElement = hapModuleInfo.mainElementName;
                if (mainElement.empty()) {
                    continue;
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
                HILOG_INFO("%{public}s, %{public}s is page ability", __func__, mainElement.c_str());
                continue;
            }

            needEraseIndexSet.insert(i);
            // startAbility
            Want want;
            want.SetElementName(hapModuleInfo.bundleName, mainElement);
            DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, USER_ID_NO_HEAD,
                DEFAULT_INVAL_VALUE);
        }
    }

    // delete item which process has been started.
    for (auto iter = needEraseIndexSet.rbegin(); iter != needEraseIndexSet.rend(); iter++) {
        bundleInfos.erase(bundleInfos.begin() + *iter);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
