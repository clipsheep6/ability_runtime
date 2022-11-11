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
#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H

#include <functional>

#include "bundle_mgr_interface.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
enum class DialogType {
    DIALOG_ANR = 0,
    DIALOG_TIPS,
    DIALOG_SELECTOR,
};
enum class DialogAlign {
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
    int32_t window_width = 0;
    int32_t window_height = 0;
    int32_t window_offsetX = 0;
    int32_t window_offsetY = 0;
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
class SystemDialogScheduler : public DelayedSingleton<SystemDialogScheduler> {
public:

    explicit SystemDialogScheduler() = default;
    virtual ~SystemDialogScheduler() = default;

    bool GetANRDialogWant(int userId, int pid, AAFwk::Want &want);
    Want GetSelectorDialogWant(const std::vector<DialogAppInfo> &dialogAppInfos, Want &targetWant);
    Want GetTipsDialogWant();

    void SetDeviceType(const std::string &deviceType)
    {
        deviceType_ = deviceType;
    }

private:
    const std::string GetAnrParams(const DialogPosition position, const std::string &appName) const;
    const std::string GetSelectorParams(const std::vector<DialogAppInfo> &infos) const;
    const std::string GetDialogPositionParams(const DialogPosition position) const;
    
    void InitDialogPosition(DialogType type, DialogPosition &position) const;
    void GetDialogPositionAndSize(DialogType type, DialogPosition &position, int lineNums = 0) const;
    void DialogPositionAdaptive(DialogPosition &position, int lineNums) const;

    sptr<AppExecFwk::IBundleMgr> GetBundleManager();

    void GetAppNameFromResource(int32_t labelId,
        const std::string &bundleName, int32_t userId, std::string &appName);

private:
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::string deviceType_ = {};
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SYSTEM_DIALOG_SCHEDULER_H
