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
#ifndef OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H
#define OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H

#include <vector>
#include <string>
#include <unordered_set>

#include "ability_record.h"
#include "system_dialog_scheduler.h"
#ifdef SUPPORT_ERMS
#include "ecological_rule_mgr_service_client.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_ERMS
using namespace OHOS::EcologicalRuleMgrService;
using ErmsCallerInfo = OHOS::EcologicalRuleMgrService::CallerInfo;
#endif
/**
 * @class ImplicitStartProcessor
 * ImplicitStartProcessor.
 */
class ImplicitStartProcessor : public std::enable_shared_from_this<ImplicitStartProcessor> {
public:
    explicit ImplicitStartProcessor() = default;
    virtual ~ImplicitStartProcessor() = default;

    static bool IsImplicitStartAction(const Want &want);

    int ImplicitStartAbility(AbilityRequest &request, int32_t userId);

private:
    int GenerateAbilityRequestByAction(int32_t userId, AbilityRequest &request,
        std::vector<DialogAppInfo> &dialogAppInfos, std::string &deviceType, bool isMoreHapList);

    using StartAbilityClosure = std::function<int32_t()>;
    int CallStartAbilityInner(int32_t userId, const Want &want, const StartAbilityClosure &callBack,
        const AbilityCallType &callType);

    int32_t ImplicitStartAbilityInner(const Want &targetWant, const AbilityRequest &request, int32_t userId);

    bool CheckImplicitStartExtensionIsValid(const AbilityRequest &request,
        const AppExecFwk::ExtensionAbilityInfo &extensionInfo);

    bool FilterAbilityList(const Want &want, std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos, int32_t userId);
    sptr<AppExecFwk::IDefaultApp> GetDefaultAppProxy();

#ifdef SUPPORT_ERMS
    void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId);
#endif

private:
    const static std::vector<std::string> blackList;
    const static std::unordered_set<AppExecFwk::ExtensionAbilityType> extensionWhiteList;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IMPLICIT_START_PROCESSOR_H