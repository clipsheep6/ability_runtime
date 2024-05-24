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
#include "napi_common_util.h"
#include "runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ui_service_extension_context.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
constexpr char LAUNCHER_ABILITY_NAME[] = "com.ohos.launcher.MainAbility";
constexpr char SHOW_ON_LOCK_SCREEN[] = "ShowOnLockScreen";
}

using namespace OHOS::AppExecFwk;

void UIServiceExtension::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
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
