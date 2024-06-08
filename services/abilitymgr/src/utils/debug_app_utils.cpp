/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "debug_app_utils.h"

#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "parameters.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
// Developer mode param
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr const char* DEBUG_APP = "debugApp";

void DebugAppUtils::SetDebugAppByWaitingDebugFlag(
    const Want &want, Want &requestWant, const std::string &bundleName, bool isDebugApp)
{
    if (!isDebugApp || !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not meeting the set debugging conditions.");
        return;
    }

    if (IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->IsWaitingDebugApp(bundleName))) {
        (const_cast<Want &>(want)).SetParam(DEBUG_APP, true);
        requestWant.SetParam(DEBUG_APP, true);
        IN_PROCESS_CALL_WITHOUT_RET(
            DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->ClearNonPersistWaitingDebugFlag());
    }
}
}  // namespace AAFwk
}  // namespace OHOS
