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
class AppUtils {
public:
    static AppUtils &GetInstance();
    bool IsLauncher(const std::string &bundleName) const;
    bool IsLauncherAbility(const std::string &abilityName) const;
    bool JudgePCDevice() const;
    bool IsInheritWindowSplitScreenMode() const;
    bool IsSupportAncoApp() const;
    int32_t GetTimeoutUnitTimeRatio() const;
    bool IsSelectorDialogDefaultPossion() const;
    bool IsStartSpecifiedProcess() const;
    bool IsUseMultiRenderProcess() const;
    bool IsLimitMaximumOfRenderProcess() const;
    bool IsGrantPersistUriPermission() const;
    bool IsStartOptionsWithAnimation() const;
    bool IsMultiProcessModel() const;
    bool IsStartOptionsWithProcessOption() const;
    bool EnableMoveUIAbilityToBackgroundApi() const;

private:
    AppUtils();
    ~AppUtils();
    volatile bool isSceneBoard_ = false;
    volatile bool isPcDevice_ = false;
    volatile bool isInheritWindowSplitScreenMode_ = true;
    volatile bool isSupportAncoApp_ = false;
    volatile int32_t timeoutUnitTimeRatio_ = 1;
    volatile bool isSelectorDialogDefaultPossion_ = true;
    volatile bool isStartSpecifiedProcess_ = false;
    volatile bool isUseMultiRenderProcess_ = true;
    volatile bool isLimitMaximumOfRenderProcess_ = true;
    volatile bool isGrantPersistUriPermission_ = false;
    volatile bool isStartOptionsWithAnimation_ = false;
    volatile bool isMultiProcessModel_ = false;
    volatile bool isStartOptionsWithProcessOption_ = false;
    volatile bool enableMoveUIAbilityToBackgroundApi_ = true;
    DISALLOW_COPY_AND_MOVE(AppUtils);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H
