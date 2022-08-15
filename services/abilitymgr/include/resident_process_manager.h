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

#ifndef OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H

#include "app_scheduler.h"
#include "bundle_info.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ResidentProcessManager
 * ResidentProcessManager
 */
class ResidentProcessManager : public std::enable_shared_from_this<ResidentProcessManager> {
    DECLARE_DELAYED_SINGLETON(ResidentProcessManager)
public:
    void StartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    void StartResidentProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos);
private:
    bool CheckMainElement(const AppExecFwk::HapModuleInfo &hapModuleInfo, const std::string &processName,
        std::string &mainElement, std::set<uint32_t> &needEraseIndexSet, size_t bundleInfoIndex);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H
