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

#include "ui_service_extension.h"

#include <cstdlib>
#include <regex>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ability_delegator_registry.h"
#include "napi_common_util.h"
#include "runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ui_service_extension_context.h"
#include "time_util.h"


namespace OHOS {
namespace AbilityRuntime {
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

#ifdef SUPPORT_GRAPHICS
sptr<Rosen::WindowOption> UIServiceExtension::GetWindowOption(const AAFwk::Want &want,
    const std::shared_ptr< Rosen::ExtensionWindowConfig>& extensionWindowConfig, const sptr<AAFwk::SessionInfo>& sessionInfo)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(
        AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    TAG_LOGI(AAFwkTag::UIABILITY, "Window mode is %{public}d.", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
    bool showOnLockScreen = false;
    if (abilityInfo_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set window type for launcher.");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }

    if (extensionWindowConfig->windowAttribute == Rosen::ExtensionWindowAttribute::SUB_WINDOW) {
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
        option->SetParentId(sessionInfo->hostWindowId);
        option->SetExtensionTag(true);
        option->SetSubWindowTitle(extensionWindowConfig->subWindowOptions.title);
        option->SetSubWindowDecorEnable(extensionWindowConfig->subWindowOptions.decorEnabled);
        if(extensionWindowConfig->subWindowOptions.isModal) {
            option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_IS_MODAL);
        }
    } else if (extensionWindowConfig->windowAttribute == Rosen::ExtensionWindowAttribute::SYSTEM_WINDOW) {
        if (Rosen::JS_TO_NATIVE_WINDOW_TYPE_MAP.count(static_cast<Rosen::ApiWindowType>(extensionWindowConfig->systemWindowOptions.windowType)) != 0) {
            Rosen::WindowType winType = Rosen::WindowType::SYSTEM_WINDOW_BASE;
            winType = Rosen::JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<Rosen::ApiWindowType>(extensionWindowConfig->systemWindowOptions.windowType));
            if (Rosen::WindowHelper::IsSystemWindow(winType)){
                option->SetWindowType(winType);
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    }
    option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowRect(extensionWindowConfig->windowRect);
    return option;
}
#endif
}
}
