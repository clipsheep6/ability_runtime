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

#include "ability_runtime/cj_ability_context.h"

#include "cj_common_ffi.h"
#include "hilog_tag_wrapper.h"
#include "cj_ability_connect_callback_object.h"

namespace OHOS {
namespace AbilityRuntime {

std::shared_ptr<AbilityRuntime::AbilityContext> CJAbilityContext::GetAbilityContext()
{
    return context_;
}

sptr<IRemoteObject> CJAbilityContext::GetToken()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }
    return context_->GetToken();
}

std::string CJAbilityContext::GetPreferencesDir()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return "";
    }
    return context_->GetPreferencesDir();
}

std::string CJAbilityContext::GetDatabaseDir()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return "";
    }
    return context_->GetDatabaseDir();
}

std::string CJAbilityContext::GetBundleName()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return "";
    }
    return context_->GetBundleName();
}

int32_t CJAbilityContext::GetArea()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->GetArea();
}

std::shared_ptr<AppExecFwk::AbilityInfo> CJAbilityContext::GetAbilityInfo()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }
    return context_->GetAbilityInfo();
}

std::shared_ptr<AppExecFwk::HapModuleInfo> CJAbilityContext::GetHapModuleInfo()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }
    return context_->GetHapModuleInfo();
}

std::shared_ptr<AppExecFwk::Configuration> CJAbilityContext::GetConfiguration()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return nullptr;
    }
    return context_->GetConfiguration();
}

int32_t CJAbilityContext::StartAbility(const AAFwk::Want& want)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    // -1 is default accountId which is the same as js.
    return context_->StartAbility(want, -1);
}

int32_t CJAbilityContext::StartAbility(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbility(want, startOptions, -1);
}

int32_t CJAbilityContext::StartAbilityWithAccount(const AAFwk::Want& want, int32_t accountId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityWithAccount(want, accountId, -1);
}

int32_t CJAbilityContext::StartAbilityWithAccount(
    const AAFwk::Want& want, int32_t accountId, const AAFwk::StartOptions& startOptions)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityWithAccount(want, accountId, startOptions, -1);
}

int32_t CJAbilityContext::StartServiceExtensionAbility(const Want& want)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartServiceExtensionAbility(want);
}

int32_t CJAbilityContext::StartServiceExtensionAbilityWithAccount(const Want& want, int32_t accountId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartServiceExtensionAbility(want, accountId);
}

int32_t CJAbilityContext::StopServiceExtensionAbility(const Want& want)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StopServiceExtensionAbility(want);
}

int32_t CJAbilityContext::StopServiceExtensionAbilityWithAccount(const Want& want, int32_t accountId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StopServiceExtensionAbility(want, accountId);
}

int32_t CJAbilityContext::TerminateSelf()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->TerminateSelf();
}

int32_t CJAbilityContext::TerminateSelfWithResult(const AAFwk::Want& want, int32_t resultCode)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->TerminateAbilityWithResult(want, resultCode);
}

std::tuple<int32_t, bool> CJAbilityContext::IsTerminating()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return std::make_tuple(ERR_INVALID_INSTANCE_CODE, false);
    }
    return std::make_tuple(SUCCESS_CODE, context_->IsTerminating());
}

bool CJAbilityContext::ConnectAbility(const AAFwk::Want& want, int64_t connectionId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto connection = new CJAbilityConnectCallback(connectionId);
    return context_->ConnectAbility(want, connection);
}

int32_t CJAbilityContext::ConnectAbilityWithAccount(
    const AAFwk::Want& want, int32_t accountId, int64_t connectionId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto connection = new CJAbilityConnectCallback(connectionId);
    return context_->ConnectAbilityWithAccount(want, accountId, connection);
}

void CJAbilityContext::DisconnectAbility(const AAFwk::Want& want, int64_t connectionId)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return;
    }
    auto connection = new CJAbilityConnectCallback(connectionId);
    context_->ConnectAbility(want, connection);
}

int32_t CJAbilityContext::StartAbilityForResult(const AAFwk::Want& want, int32_t requestCode, RuntimeTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityForResult(want, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResult(
    const AAFwk::Want& want, const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, int32_t accountId, int32_t requestCode, RuntimeTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityForResultWithAccount(want, accountId, requestCode, std::move(task));
}

int32_t CJAbilityContext::StartAbilityForResultWithAccount(const AAFwk::Want& want, int32_t accountId,
    const AAFwk::StartOptions& startOptions, int32_t requestCode, RuntimeTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->StartAbilityForResultWithAccount(want, accountId, startOptions, requestCode, std::move(task));
}

int32_t CJAbilityContext::RequestPermissionsFromUser(
    AppExecFwk::Ability* ability, std::vector<std::string>& permissions, PermissionRequestTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return SUCCESS_CODE;
}

#ifdef SUPPORT_GRAPHICS
int32_t CJAbilityContext::SetMissionLabel(const std::string& label)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->SetMissionLabel(label);
}

int32_t CJAbilityContext::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->SetMissionIcon(icon);
}
#endif

void CJAbilityContext::InheritWindowMode(AAFwk::Want& want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "called");
#ifdef SUPPORT_GRAPHICS
    // Only split mode need inherit.
    auto windowMode = context_->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "end, window mode is %{public}d", windowMode);
#endif
}

int32_t CJAbilityContext::RequestDialogService(AAFwk::Want& want, RequestDialogResultTask&& task)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return context_->RequestDialogService(want, std::move(task));
}

} // namespace AbilityRuntime
} // namespace OHOS
