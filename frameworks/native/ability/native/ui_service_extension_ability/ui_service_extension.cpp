/*
 * Copyright (c) s2024 Huawei Device Co., Ltd.
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

#include "ui_service_extension.h"

#include <cstdlib>
#include <regex>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ability_delegator_registry.h"
#include "napi_common_util.h"
#include "runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
// #include "js_ability.h"
#include "ui_service_extension_context.h"
#include "reverse_continuation_scheduler_primary_stage.h"
#include "time_util.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
constexpr char LAUNCHER_ABILITY_NAME[] = "com.ohos.launcher.MainAbility";
constexpr char SHOW_ON_LOCK_SCREEN[] = "ShowOnLockScreen";
}

using namespace OHOS::AppExecFwk;

UIServiceExtension* UIServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new UIServiceExtension();
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "UIServiceExtension Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIServiceExtension::Create(runtime);

        default:
            return new UIServiceExtension();
    }
}

void UIServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "UIExtension begin init");
	abilityInfo_ = record->GetAbilityInfo();
    ExtensionBase<UIServiceExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<UIServiceExtensionContext> UIServiceExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<UIServiceExtensionContext> context =
        ExtensionBase<UIServiceExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UIServiceExtension CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}

std::string UIServiceExtension::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityInfo_ is nullptr");
        return "";
    }
    return abilityInfo_->name;
}

std::string UIServiceExtension::GetModuleName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityInfo_ is nullptr.");
        return "";
    }

    return abilityInfo_->moduleName;
}

sptr<IRemoteObject> UIServiceExtension::GetSessionToken()
{
    return sessionToken_;
}

AppExecFwk::AbilityLifecycleExecutor::LifecycleState UIServiceExtension::GetState()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Called.");
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityLifecycleExecutor_ is nullptr.");
        return AppExecFwk::AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }
    return static_cast<AppExecFwk::AbilityLifecycleExecutor::LifecycleState>(abilityLifecycleExecutor_->GetState());
}

void UIServiceExtension::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
}

void UIServiceExtension::SetStartAbilitySetting(std::shared_ptr<AppExecFwk::AbilityStartSetting> setting)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Called.");
    setting_ = setting;
}

void UIServiceExtension::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Called.");
    launchParam_ = launchParam;
}

bool UIServiceExtension::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityContext_ is null.");
        return false;
    }

    if (launchParam_.launchReason != AAFwk::LaunchReason::LAUNCHREASON_CONTINUATION) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "LaunchReason is %{public}d.", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Get content failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "End.");
    return true;
}


bool UIServiceExtension::ShouldRecoverState(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "AppRecovery not recovery restart.");
        return false;
    }

    if (abilityRecovery_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityRecovery_ is null.");
        return false;
    }

    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityContext_ is null.");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Get content failed.");
        return false;
    }
    return true;
}

bool UIServiceExtension::CheckIsSilentForeground() const
{
    return isSilentForeground_;
}

void UIServiceExtension::SetIsSilentForeground(bool isSilentForeground)
{
    isSilentForeground_ = isSilentForeground;
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Rosen::WindowScene> UIServiceExtension::GetScene()
{
    return scene_;
}

sptr<Rosen::WindowOption> UIServiceExtension::GetWindowOption(const AAFwk::Want &want,
    const std::shared_ptr< Rosen::WindowStageConfig>& windowStageConfig)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(
        AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    TAG_LOGD(AAFwkTag::UIABILITY, "Window mode is %{public}d.", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
    bool showOnLockScreen = false;
    if (abilityInfo_) {
        std::vector<AppExecFwk::CustomizeData> datas = abilityInfo_->metaData.customizeData;
        for (AppExecFwk::CustomizeData data : datas) {
            if (data.name == SHOW_ON_LOCK_SCREEN) {
                showOnLockScreen = true;
            }
        }
    }
    if (showOnLockScreen_ || showOnLockScreen) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED.");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set window type for launcher.");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    option->SetWindowRect(windowStageConfig->rect);
    option->SetWindowStageAttribute(windowStageConfig->windowStageAttribute);
    return option;
}

void UIServiceExtension::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}
#endif
}
}
