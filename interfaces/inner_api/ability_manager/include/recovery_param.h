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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECOVERY_PARAM_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECOVERY_PARAM_H

namespace OHOS {
namespace AppExecFwk {
/**
 * @enum OnSaveResult
 * OnSaveResult defines the reason of Save data mode.
 */
enum OnSaveResult {
    ALL_AGREE = 0,
    CONTINUATION_REJECT,
    CONTINUATION_MISMATCH,
    RECOVERY_AGREE,
    RECOVERY_REJECT,
    ALL_REJECT
};

/**
 * @enum StateType
 * StateType defines the reason of state type.
 */
enum StateType {
    CONTINUATION = 0,
    APP_RECOVERY,
};

enum RestartFlag {
    ALWAYS_RESTART = 0,
    RESTART_WHEN_JS_CRASH = 0x0001,
    RESTART_WHEN_APP_FREEZE = 0x0002,
    NO_RESTART = 0xFFFF,
};

enum SaveOccasionFlag {
    NO_SAVE = 0,
    SAVE_WHEN_ERROR = 1,
    SAVE_WHEN_BACKGROUND = 2,
    SAVE_ALL = 0xFF,
};

enum SaveModeFlag {
    SAVE_WITH_FILE = 1,
    SAVE_WITH_SHARED_MEMORY = 2,
};

enum StateReason {
    DEVELOPER_REQUEST,
    LIFECYCLE,
    CPP_CRASH,
    JS_ERROR,
    APP_FREEZE,
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECOVERY_PARAM_H