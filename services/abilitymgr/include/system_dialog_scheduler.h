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
#ifndef OHOS_AAFWK_SYSTEM_DIALOG_SCHEDULER_H
#define OHOS_AAFWK_SYSTEM_DIALOG_SCHEDULER_H

#include <functional>

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AAFwk {
enum DialogType {
    DIALOG_ANR = 0,
    DIALOG_TIPS,
    DIALOG_SELECTOR,
};
enum DialogAlign {
    TOP = 0,
    CENTER,
    BOTTOM,
    LEFT,
    RIGHT,
};
struct DialogPosition {
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    int32_t width = 0;
    int32_t height = 0; 
    int32_t width_narrow = 0;
    int32_t height_narrow = 0;
    bool wideScreen = true;
    DialogAlign align = DialogAlign::CENTER;
};
struct DialogAppInfo {
    int32_t iconId = 0;
    int32_t labelId = 0;
    std::string bundleName = {};
    std::string abilityName = {};
};
/**
 * @class SystemDialogScheduler
 * SystemDialogScheduler.
 */
class SystemDialogScheduler {
public:
    using DialogCallback = std::function<void(int32_t id, const std::string& event, const std::string& param)>;
    using Closure = std::function<void()>;

    explicit SystemDialogScheduler(const std::string &deviceType);
    virtual ~SystemDialogScheduler() = default;

    int32_t ShowANRDialog(const std::string &appName, const Closure &callBack);

    void GetAppNameFromResource(int32_t labelId,
        const std::string &bundleName, int32_t userId, std::string &appName);

private:
    void InitDialogPosition(DialogType type, DialogPosition &position) const;
    void GetDialogPositionAndSize(DialogType type, DialogPosition &position, int lineNums = 0) const;
    
    void ScheduleShowDialog(const std::string &name, const DialogPosition &position,
        const std::string &params, DialogCallback callback) const;

    sptr<AppExecFwk::IBundleMgr> GetBundleManager();

private:
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::string deviceType_ = {};
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_SYSTEM_DIALOG_SCHEDULER_H