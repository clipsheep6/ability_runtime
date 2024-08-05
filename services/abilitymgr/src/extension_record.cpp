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

#include "extension_record.h"

#include "ability_manager_service.h"
#include "ability_util.h"

namespace OHOS {
namespace AbilityRuntime {
ExtensionRecord::ExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord)
    : abilityRecord_(abilityRecord)
{}

ExtensionRecord::~ExtensionRecord() = default;

sptr<IRemoteObject> ExtensionRecord::GetCallToken() const
{
    CHECK_POINTER_AND_RETURN(abilityRecord_, nullptr);
    auto sessionInfo = abilityRecord_->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    return sessionInfo->callerToken;
}

sptr<IRemoteObject> ExtensionRecord::GetRootCallerToken() const
{
    return rootCallerToken_;
}

void ExtensionRecord::SetRootCallerToken(sptr<IRemoteObject> &rootCallerToken)
{
    rootCallerToken_ = rootCallerToken;
}

sptr<IRemoteObject> ExtensionRecord::GetFocusedCallerToken() const
{
    return focusedCallerToken_;
}

void ExtensionRecord::SetFocusedCallerToken(sptr<IRemoteObject> &focusedCallerToken)
{
    focusedCallerToken_ = focusedCallerToken;
}

void ExtensionRecord::UnloadUIExtensionAbility()
{
    auto ret = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->UnregisterApplicationStateObserver(
        preLoadUIExtStateObserver_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Unregister application state observer error.");
    }
    auto result = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance()->UnloadUIExtensionAbility(
        abilityRecord_, hostBundleName_);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Unload UIExtension error.");
    }
}

int32_t ExtensionRecord::RegisterStateObserver(const std::string &hostBundleName)
{
    preLoadUIExtStateObserver_ = sptr<AAFwk::PreLoadUIExtStateObserver>::MakeSptr(weak_from_this());
    auto ret = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->RegisterApplicationStateObserver(
            preLoadUIExtStateObserver_, {hostBundleName}));
    return ret;
}

bool ExtensionRecord::ContinueToGetCallerToken()
{
    return false;
}

void ExtensionRecord::Update(const AAFwk::AbilityRequest &abilityRequest)
{
}
} // namespace AbilityRuntime
} // namespace OHOS
