/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_action_extension.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
JsActionExtension *JsActionExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new JsActionExtension(runtime);
}

JsActionExtension::JsActionExtension(const std::unique_ptr<Runtime> &runtime)
{
    jsUIExtensionBase_ = std::make_shared<JsUIExtensionBase>(runtime);
}

JsActionExtension::~JsActionExtension()
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
}

void JsActionExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    ActionExtension::Init(record, application, handler, token);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->SetAbilityInfo(abilityInfo_);
    jsUIExtensionBase_->SetContext(GetContext());
    auto extensionCommon = jsUIExtensionBase_->Init(record, application, handler, token);
    SetExtensionCommon(extensionCommon);
}

void JsActionExtension::OnStart(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    Extension::OnStart(want);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnStart(want);
}

void JsActionExtension::OnStop()
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    ActionExtension::OnStop();

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnStop();
}

void JsActionExtension::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "begin. persistentId: %{private}d, winCmd: %{public}d",
        sessionInfo->persistentId, winCmd);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "sessionInfo is nullptr.");
        return;
    }
    Extension::OnCommandWindow(want, sessionInfo, winCmd);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnCommandWindow(want, sessionInfo, winCmd);
}

void JsActionExtension::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::ACTION_EXT, "begin restart = %{public}s, startId = %{public}d.",
        restart ? "true" : "false", startId);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnCommand(want, restart, startId);
}

void JsActionExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnForeground(want, sessionInfo);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnForeground(want, sessionInfo);
}

void JsActionExtension::OnBackground()
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnBackground();
    Extension::OnBackground();
}

void JsActionExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    Extension::OnConfigurationUpdated(configuration);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->OnConfigurationUpdated(configuration);
}

void JsActionExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "called.");
    Extension::Dump(params, info);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is nullptr");
        return;
    }
    jsUIExtensionBase_->Dump(params, info);
}

void JsActionExtension::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ACTION_EXT, "OnAbilityResult called.");
    Extension::OnAbilityResult(requestCode, resultCode, resultData);

    if (jsUIExtensionBase_ == nullptr) {
        TAG_LOGE(AAFwkTag::ACTION_EXT, "jsUIExtensionBase_ is null");
        return;
    }
    jsUIExtensionBase_->OnAbilityResult(requestCode, resultCode, resultData);
}
} // namespace AbilityRuntime
} // namespace OHOS
