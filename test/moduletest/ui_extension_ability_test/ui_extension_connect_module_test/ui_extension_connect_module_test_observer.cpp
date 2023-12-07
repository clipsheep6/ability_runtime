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

#include "ui_extension_connect_module_test_observer.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TARGET_BUNDLE_NAME = "com.ohos.uiextensionprovider";
} // namespace

void UIExtensionConnectModuleTestObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("bundleName: %{public}s, abilityState: %{public}d", processData.bundleName.c_str(), processData.state);
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == TARGET_BUNDLE_NAME) {
        processCreated_ = true;
        observerCondation_.notify_one();
    }
}

void UIExtensionConnectModuleTestObserver::OnProcessStateChanged(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("bundleName: %{public}s, abilityState: %{public}d", processData.bundleName.c_str(), processData.state);
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == TARGET_BUNDLE_NAME) {
        if (processData.state == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            processForegrounded_ = true;
            observerCondation_.notify_one();
        } else if (processData.state == AppExecFwk::AppProcessState::APP_STATE_BACKGROUND) {
            processBackgrounded_ = true;
            observerCondation_.notify_one();
        }
    }
}

void UIExtensionConnectModuleTestObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("bundleName: %{public}s, abilityState: %{public}d", processData.bundleName.c_str(), processData.state);
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == TARGET_BUNDLE_NAME) {
        processDied_ = true;
        observerCondation_.notify_one();
    }
}

void UIExtensionConnectModuleTestObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    HILOG_INFO("bundleName: %{public}s, abilityState: %{public}d", abilityStateData.bundleName.c_str(),
        abilityStateData.abilityState);
}

void UIExtensionConnectModuleTestObserver::OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    HILOG_INFO("bundleName: %{public}s, abilityState: %{public}d", abilityStateData.bundleName.c_str(),
        abilityStateData.abilityState);
}
} // namespace AAFwk
} // namespace OHOS
