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

#include "application_anr_listener.h"

#include <sys/time.h>
#include "singleton.h"

#include "app_mgr_client.h"
#include "fault_data.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
ApplicationAnrListener::ApplicationAnrListener() {}

ApplicationAnrListener::~ApplicationAnrListener() {}

void ApplicationAnrListener::OnAnr(int32_t pid) const
{
    AppExecFwk::AppFaultDataBySA faultData;
    faultData.faultType = AppExecFwk::FaultDataType::APP_FREEZE;
    faultData.pid = pid;
    faultData.errorObject.message = "User input does not respond!";
    faultData.errorObject.stack = "";
    faultData.errorObject.name = AppExecFwk::AppFreezeType::APP_INPUT_BLOCK;
    faultData.waitSaveState = false;
    faultData.notifyApp = false;
    faultData.forceExit = false;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultData);
}
}  // namespace AAFwk
}  // namespace OHOS