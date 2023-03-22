/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "application_data_manager.h"

#include "app_recovery.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ApplicationDataManager::ApplicationDataManager() {}

ApplicationDataManager::~ApplicationDataManager() {}

ApplicationDataManager &ApplicationDataManager::GetInstance()
{
    static ApplicationDataManager manager;
    return manager;
}

void ApplicationDataManager::AddErrorObserver(const std::shared_ptr<IErrorObserver> &observer)
{
    HILOG_DEBUG("Add error observer come.");
    errorObserver_ = observer;
}

bool ApplicationDataManager::NotifyUnhandledException(const std::string &errMsg)
{
    HILOG_DEBUG("Notify error observer come.");
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

void ApplicationDataManager::RemoveErrorObserver()
{
    HILOG_DEBUG("Remove error observer come.");
    errorObserver_ = nullptr;
}

bool ApplicationDataManager::NotifyExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    HILOG_DEBUG("Notify Exception error observer come.");
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}
}  // namespace AppExecFwk
}  // namespace OHOS
