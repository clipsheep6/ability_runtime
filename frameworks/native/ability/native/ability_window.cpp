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

#include "ability_window.h"
#include "ability.h"
#include "ability_handler.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AbilityWindow::AbilityWindow()
{}

AbilityWindow::~AbilityWindow()
{}

/**
 * @brief Init the AbilityWindow object.
 *
 * @param handler The EventHandler of the Ability the AbilityWindow belong.
 */
void AbilityWindow::Init(std::shared_ptr<AbilityHandler>& handler, std::shared_ptr<Ability> ability)
{
    HILOG_DEBUG("%{public}s come.", __func__);
    handler_ = handler;
    ability_ = std::weak_ptr<IAbilityEvent>(ability);
    windowScene_ = std::make_shared<Rosen::WindowScene>();
}

bool AbilityWindow::InitWindow(Rosen::WindowType winType,
    std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext,
    sptr<Rosen::IWindowLifeCycle> &listener, int32_t displayId, sptr<Rosen::WindowOption> option,
    bool isPrivacy)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (windowScene_ == nullptr) {
        windowScene_ = std::make_shared<Rosen::WindowScene>();
    }
    auto ret = windowScene_->Init(displayId, abilityContext, listener, option);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("%{public}s error. failed to init window scene!", __func__);
        return false;
    }

    auto window = windowScene_->GetMainWindow();
    if (!window) {
        HILOG_ERROR("%{public}s window is nullptr.", __func__);
        return false;
    }

    ret = window->SetWindowType(winType);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("Set window type error, errcode = %{public}d", ret);
        return false;
    }
    winType_ = winType;
    if (isPrivacy) {
        window->SetSystemPrivacyMode(true);
    }

    isWindowAttached = true;
    HILOG_DEBUG("%{public}s end.", __func__);
    return true;
}

/**
 * @brief Called when this ability is background.
 *
 */
void AbilityWindow::OnPostAbilityBackground(uint32_t sceneFlag)
{
    HILOG_DEBUG("AbilityWindow::OnPostAbilityBackground called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityBackground window not attached.");
        return;
    }

    if (windowScene_) {
        HILOG_DEBUG("%{public}s begin windowScene_->GoBackground, sceneFlag:%{public}d.", __func__, sceneFlag);
        windowScene_->GoBackground(sceneFlag);
        HILOG_DEBUG("%{public}s end windowScene_->GoBackground.", __func__);
    }

    HILOG_DEBUG("AbilityWindow::OnPostAbilityBackground end.");
}

/**
 * @brief Called when this ability is foreground.
 *
 */
void AbilityWindow::OnPostAbilityForeground(uint32_t sceneFlag)
{
    HILOG_DEBUG("AbilityWindow::OnPostAbilityForeground called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityForeground window not attached.");
        return;
    }

    if (windowScene_) {
        HILOG_DEBUG("%{public}s begin windowScene_->GoForeground, sceneFlag:%{public}d.", __func__, sceneFlag);
        windowScene_->GoForeground(sceneFlag);
        HILOG_DEBUG("%{public}s end windowScene_->GoForeground.", __func__);
    }

    HILOG_DEBUG("AbilityWindow::OnPostAbilityForeground end.");
}

/**
 * @brief Called when this ability is stopped.
 *
 */
void AbilityWindow::OnPostAbilityStop()
{
    HILOG_DEBUG("AbilityWindow::OnPostAbilityStop called.");
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::OnPostAbilityStop window not attached.");
        return;
    }

    if (windowScene_) {
        windowScene_->GoDestroy();
    }
    isWindowAttached = false;
    HILOG_DEBUG("AbilityWindow::OnPostAbilityStop end.");
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a Window object pointer.
 */
const sptr<Rosen::Window> AbilityWindow::GetWindow()
{
    if (!isWindowAttached) {
        HILOG_ERROR("AbilityWindow::GetWindow window not attached.");
        return nullptr;
    }
    return windowScene_ ? windowScene_->GetMainWindow() : nullptr;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityWindow::SetMissionLabel(const std::string &label)
{
    HILOG_DEBUG("%{public}s start", __func__);
    auto window = GetWindow();
    if (!window) {
        HILOG_ERROR("get window failed.");
        return -1;
    }

    auto ret = window->SetAPPWindowLabel(label);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("SetAPPWindowLabel failed, errCode:%{public}d.", ret);
        return -1;
    }

    return ERR_OK;
}

ErrCode AbilityWindow::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    HILOG_DEBUG("%{public}s start", __func__);
    auto window = GetWindow();
    if (!window) {
        HILOG_ERROR("get window failed, will not set mission icon.");
        return -1;
    }

    auto ret = window->SetAPPWindowIcon(icon);
    if (ret != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("SetAPPWindowIcon failed, errCode:%{public}d.", ret);
        return -1;
    }

    return ERR_OK;
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
