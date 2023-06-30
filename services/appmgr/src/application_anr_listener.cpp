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

#include "application_anr_listener.h"

#include <sys/time.h>
#include "singleton.h"

#include "app_mgr_client.h"
#include "fault_data.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}
uint64_t ApplicationAnrListener::lastTime_ = 0;
ApplicationAnrListener::ApplicationAnrListener() {}

ApplicationAnrListener::~ApplicationAnrListener() {}

void ApplicationAnrListener::OnAnr(int32_t pid) const
{
    const uint64_t intervalTime = 3000; // ms
    if (GetMilliseconds() - lastTime_ < intervalTime) {
        return;
    }
    lastTime_ = GetMilliseconds();
    AppFaultDataBySA faultData;
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.pid = pid;
    faultData.errorObject.message = "User input does not respond!";
    faultData.errorObject.stack = "";
    faultData.errorObject.name = "APP_INPUT_BLOCK";
    faultData.waitSaveState = true;
    faultData.notifyApp = false;
    faultData.forceExit = true;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultData);
}
}  // namespace AAFwk
}  // namespace OHOS
