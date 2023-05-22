/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "form_extension_context.h"

#include "ability_manager_client.h"
#include "appexecfwk_errors.h"
#include "form_mgr.h"
#include "form_mgr_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t FormExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("FormExtensionContext"));

int FormExtensionContext::UpdateForm(const int64_t formId, const AppExecFwk::FormProviderData &formProviderData)
{
    HILOG_DEBUG("Update form, formId: %{public}" PRId64 ".", formId);
    if (formId <= 0) {
        HILOG_ERROR("FormId can't be negative or zero.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // check fms recover status
    if (AppExecFwk::FormMgr::GetRecoverStatus() == AppExecFwk::Constants::IN_RECOVERING) {
        HILOG_ERROR("Update failed, form is in recover status, can't do action on it.");
        return ERR_APPEXECFWK_FORM_IN_RECOVER;
    }

    // check formProviderData
    if (formProviderData.GetDataString().empty()) {
        HILOG_ERROR("Form data is empty.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    // update form request to fms
    return AppExecFwk::FormMgr::GetInstance().UpdateForm(formId, formProviderData);
}

ErrCode FormExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HILOG_DEBUG("Start ability.");
    // route to FMS
    ErrCode err = AppExecFwk::FormMgr::GetInstance().StartAbility(want, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Start ability failed with %{public}d.", err);
    }
    return err;
}

AppExecFwk::AbilityType FormExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("Ability info is invalid.");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<AppExecFwk::AbilityInfo> FormExtensionContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

void FormExtensionContext::SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("Ability info is invalid.");
        return;
    }
    abilityInfo_ = abilityInfo;
}
} // namespace AbilityRuntime
} // namespace OHOS
