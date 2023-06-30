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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_CONTAINER_WINDOW_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_CONTAINER_WINDOW_MANAGER_H

#include <functional>

#include "base/memory/ace_type.h"
#include "core/components/common/layout/constants.h"

namespace OHOS::Ace {

using WindowCallback = std::function<void(void)>;
using WindowModeCallback = std::function<WindowMode(void)>;
using WindowSetMaximizeModeCallback = std::function<void(MaximizeMode)>;
using WindowGetMaximizeModeCallback = std::function<MaximizeMode(void)>;

class WindowManager : public virtual AceType {
    DECLARE_ACE_TYPE(WindowManager, AceType);

public:
    WindowManager() = default;
    ~WindowManager() override = default;

    void SetAppIconId(int32_t id)
    {
        appIconId_ = id;
    }

    int32_t GetAppIconId() const
    {
        return appIconId_;
    }

    void SetAppLabelId(int32_t id)
    {
        appLabelId_ = id;
    }

    int32_t GetAppLabelId() const
    {
        return appLabelId_;
    }

    void SetWindowMinimizeCallBack(WindowCallback&& callback)
    {
        windowMinimizeCallback_ = std::move(callback);
    }

    void SetWindowMaximizeCallBack(WindowCallback&& callback)
    {
        windowMaximizeCallback_ = std::move(callback);
    }

    void SetWindowMaximizeFloatingCallBack(WindowCallback&& callback)
    {
        windowMaximizeFloatingCallback_ = std::move(callback);
    }

    void SetWindowRecoverCallBack(WindowCallback&& callback)
    {
        windowRecoverCallback_ = std::move(callback);
    }

    void SetWindowCloseCallBack(WindowCallback&& callback)
    {
        windowCloseCallback_ = std::move(callback);
    }

    void SetWindowSplitPrimaryCallBack(WindowCallback&& callback)
    {
        windowSplitPrimaryCallback_ = std::move(callback);
    }

    void SetWindowSplitSecondaryCallBack(WindowCallback&& callback)
    {
        windowSplitSecondaryCallback_ = std::move(callback);
    }

    void SetWindowGetModeCallBack(WindowModeCallback&& callback)
    {
        windowGetModeCallback_ = std::move(callback);
    }

    void SetWindowStartMoveCallBack(WindowCallback&& callback)
    {
        windowStartMoveCallback_ = std::move(callback);
    }

    void SetWindowSetMaximizeModeCallBack(WindowSetMaximizeModeCallback&& callback)
    {
        windowSetMaximizeModeCallback_ = std::move(callback);
    }

    void SetWindowGetMaximizeModeCallBack(WindowGetMaximizeModeCallback&& callback)
    {
        windowGetMaximizeModeCallback_ = std::move(callback);
    }

    void WindowMinimize() const
    {
        if (windowMinimizeCallback_) {
            windowMinimizeCallback_();
        }
    }

    void WindowMaximize(bool supportFloatingMaximize = false)
    {
        if (supportFloatingMaximize && windowMaximizeFloatingCallback_) {
            windowMaximizeFloatingCallback_();
            maximizeMode_ = GetWindowMaximizeMode();
        }
        if (!supportFloatingMaximize && windowMaximizeCallback_) {
            windowMaximizeCallback_();
        }
    }

    void WindowRecover()
    {
        if (windowRecoverCallback_) {
            windowRecoverCallback_();
            maximizeMode_ = MaximizeMode::MODE_RECOVER;
        }
    }

    void FireWindowSplitCallBack(bool isPrimary = true) const
    {
        if (isPrimary && windowSplitPrimaryCallback_) {
            windowSplitPrimaryCallback_();
        }
        if (!isPrimary && windowSplitSecondaryCallback_) {
            windowSplitSecondaryCallback_();
        }
    }

    void WindowClose() const
    {
        if (windowCloseCallback_) {
            windowCloseCallback_();
        }
    }

    void WindowStartMove() const
    {
        if (windowStartMoveCallback_) {
            windowStartMoveCallback_();
        }
    }

    WindowMode GetWindowMode() const
    {
        if (windowGetModeCallback_) {
            return windowGetModeCallback_();
        }
        return WindowMode::WINDOW_MODE_UNDEFINED;
    }
     
    void SetWindowMaximizeMode(MaximizeMode mode)
    {
        if (windowSetMaximizeModeCallback_) {
            windowSetMaximizeModeCallback_(mode);
        }
    }

    MaximizeMode GetWindowMaximizeMode() const
    {
        if (windowGetMaximizeModeCallback_) {
            return windowGetMaximizeModeCallback_();
        }
        return MaximizeMode::MODE_RECOVER;
    }

    MaximizeMode GetCurrentWindowMaximizeMode() const
    {
        return maximizeMode_;
    }
    
    void SetCurrentWindowMaximizeMode(MaximizeMode mode)
    {
        maximizeMode_ = mode;
    }

private:
    int32_t appLabelId_ = 0;
    int32_t appIconId_ = 0;
    MaximizeMode maximizeMode_ = MaximizeMode::MODE_RECOVER;
    WindowCallback windowMinimizeCallback_;
    WindowCallback windowRecoverCallback_;
    WindowCallback windowCloseCallback_;
    WindowCallback windowSplitPrimaryCallback_;
    WindowCallback windowSplitSecondaryCallback_;
    WindowCallback windowStartMoveCallback_;
    WindowCallback windowMaximizeCallback_;
    WindowCallback windowMaximizeFloatingCallback_;
    WindowSetMaximizeModeCallback windowSetMaximizeModeCallback_;
    WindowGetMaximizeModeCallback windowGetMaximizeModeCallback_;
    WindowModeCallback windowGetModeCallback_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_PIPELINE_CONTAINER_WINDOW_MANAGER_H