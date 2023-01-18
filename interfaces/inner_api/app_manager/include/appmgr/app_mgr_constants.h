/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H

namespace OHOS {
namespace AppExecFwk {
namespace Constants {
const std::string APP_MGR_SERVICE_NAME = "AppMgrService";
}  // namespace Constants

enum class ApplicationState {
    APP_STATE_CREATE = 0,
    APP_STATE_READY,
    APP_STATE_FOREGROUND,
    APP_STATE_FOCUS,
    APP_STATE_BACKGROUND,
    APP_STATE_TERMINATED,
    APP_STATE_END,
};

enum class AbilityState {
    ABILITY_STATE_CREATE = 0,
    ABILITY_STATE_READY,
    ABILITY_STATE_FOREGROUND,
    ABILITY_STATE_FOCUS,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_TERMINATED,
    ABILITY_STATE_END,
    ABILITY_STATE_CONNECTED,
    ABILITY_STATE_DISCONNECTED,
};

enum class ExtensionState {
    EXTENSION_STATE_CREATE = 0,
    EXTENSION_STATE_READY,
    EXTENSION_STATE_CONNECTED,
    EXTENSION_STATE_DISCONNECTED,
    EXTENSION_STATE_TERMINATED,
};

enum AppMgrResultCode {
    RESULT_OK = 0,
    ERROR_SERVICE_NOT_READY,
    ERROR_SERVICE_NOT_CONNECTED,
    ERROR_KILL_APPLICATION
};

enum class ProcessChangeReason {
    REASON_NONE = 0,
    REASON_REMOTE_DIED,
    REASON_APP_TERMINATED,
    REASON_APP_TERMINATED_TIMEOUT,
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_CONSTANTS_H
