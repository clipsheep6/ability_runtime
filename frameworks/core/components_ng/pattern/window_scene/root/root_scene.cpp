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

#include "core/components_ng/pattern/window_scene/root/root_scene.h"

#include "dm/display_manager.h"
#include "session/container/include/session_stage.h"
#include "wm_common.h"

namespace OHOS::Ace::NG {

extern "C" ACE_EXPORT void* OHOS_ACE_CreateRootScene()
{
    LOGI("Ace lib loaded, CreateRootScene.");
    return new RootScene();
}

void RootScene::LoadContent(const std::string& contentUrl, NativeEngine* engine, NativeValue* storage,
    AbilityRuntime::Context* context)
{
    CHECK_NULL_VOID(context);
    uiContent_ = UIContent::Create(context, engine);
    CHECK_NULL_VOID(uiContent_);
    uiContent_->Initialize(this, contentUrl, storage);

    auto defaultDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHECK_NULL_VOID(defaultDisplay);
    auto deviceWidth = defaultDisplay->GetWidth();
    auto deviceHeight = defaultDisplay->GetHeight();
    auto deviceDensity = defaultDisplay->GetVirtualPixelRatio();
    LOGI("LoadContent: deviceWidth: %{public}d, deviceHeight: %{public}d, deviceDensity: %{public}f", deviceWidth,
        deviceHeight, deviceDensity);

    ViewportConfig config;
    config.SetPosition(0, 0);
    config.SetSize(deviceWidth, deviceHeight);
    config.SetDensity(deviceDensity);
    UpdateViewportConfig(config, Rosen::WindowSizeChangeReason::UNDEFINED);
}

} // namespace OHOS::Ace::NG
