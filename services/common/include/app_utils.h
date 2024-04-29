/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include <string>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
template<typename T>
class DeviceConfiguration {
public:
    bool isLoaded = false;
    T value;
};

class AppUtils {
public:
    static AppUtils &GetInstance();
    bool IsLauncher(const std::string &bundleName) const;
    bool IsLauncherAbility(const std::string &abilityName) const;
    bool IsInheritWindowSplitScreenMode();
    bool IsSupportAncoApp();
    int32_t GetTimeoutUnitTimeRatio();
    bool IsSelectorDialogDefaultPossion();
    bool IsStartSpecifiedProcess();
    bool IsUseMultiRenderProcess();
    bool IsLimitMaximumOfRenderProcess();
    bool IsGrantPersistUriPermission();
    bool IsStartOptionsWithAnimation();
    bool IsMultiProcessModel();
    bool IsStartOptionsWithProcessOptions();
    bool EnableMoveUIAbilityToBackgroundApi();
    bool IsLaunchEmbededUIAbility();

private:
    AppUtils();
    ~AppUtils();
    volatile bool isSceneBoard_ = false;
    volatile DeviceConfiguration<bool> isInheritWindowSplitScreenMode_ = {false, true};
    volatile DeviceConfiguration<bool> isSupportAncoApp_ = {false, false};
    volatile DeviceConfiguration<int32_t> timeoutUnitTimeRatio_ = {false, 1};
    volatile DeviceConfiguration<bool> isSelectorDialogDefaultPossion_ = {false, true};
    volatile DeviceConfiguration<bool> isStartSpecifiedProcess_ = {false, false};
    volatile DeviceConfiguration<bool> isUseMultiRenderProcess_ = {false, true};
    volatile DeviceConfiguration<bool> isLimitMaximumOfRenderProcess_ = {false, true};
    volatile DeviceConfiguration<bool> isGrantPersistUriPermission_ = {false, false};
    volatile DeviceConfiguration<bool> isStartOptionsWithAnimation_ = {false, false};
    volatile DeviceConfiguration<bool> isMultiProcessModel_ = {false, false};
    volatile DeviceConfiguration<bool> isStartOptionsWithProcessOptions_ = {false, false};
    volatile DeviceConfiguration<bool> enableMoveUIAbilityToBackgroundApi_ = {false, true};
    volatile DeviceConfiguration<bool> isLaunchEmbededUIAbility_ = {false, false};
    DISALLOW_COPY_AND_MOVE(AppUtils);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
