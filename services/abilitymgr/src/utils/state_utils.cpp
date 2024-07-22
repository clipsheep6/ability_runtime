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

#include "utils/state_utils.h"

namespace OHOS {
namespace AAFwk {
std::string StateUtils::StateToStrMap(const AbilityState &state)
{
    switch (state) {
        case INITIAL: return "INITIAL";
        case INACTIVE: return "INACTIVE";
        case ACTIVE: return "ACTIVE";
        case INACTIVATING: return "INACTIVATING";
        case ACTIVATING: return "ACTIVATING";
        case TERMINATING: return "TERMINATING";
        case FOREGROUND: return "FOREGROUND";
        case BACKGROUND: return "BACKGROUND";
        case FOREGROUNDING: return "FOREGROUNDING";
        case BACKGROUNDING: return "BACKGROUNDING";
        case FOREGROUND_FAILED: return "FOREGROUND_FAILED";
        case FOREGROUND_INVALID_MODE: return "FOREGROUND_INVALID_MODE";
        case FOREGROUND_WINDOW_FREEZED: return "FOREGROUND_WINDOW_FREEZED";
        case FOREGROUND_DO_NOTHING: return "FOREGROUND_DO_NOTHING";
        case BACKGROUND_FAILED: return "BACKGROUND_FAILED";
        default: return "INVALIDSTATE";
    }
}

std::string StateUtils::AppStateToStrMap(const AppState &state)
{
    switch (state) {
        case AppState::BEGIN: return "BEGIN";
        case AppState::READY: return "READY";
        case AppState::FOREGROUND: return "FOREGROUND";
        case AppState::BACKGROUND: return "BACKGROUND";
        case AppState::SUSPENDED: return "SUSPENDED";
        case AppState::TERMINATED: return "TERMINATED";
        case AppState::END: return "END";
        case AppState::FOCUS: return "FOCUS";
        default: return "INVALIDSTATE";
    }
}

int StateUtils::ConvertStateMap(const AbilityLifeCycleState &state)
{
    switch (state) {
        case ABILITY_STATE_INITIAL: return INITIAL;
        case ABILITY_STATE_INACTIVE: return INACTIVE;
        case ABILITY_STATE_ACTIVE: return ACTIVE;
        case ABILITY_STATE_FOREGROUND_NEW: return FOREGROUND;
        case ABILITY_STATE_BACKGROUND_NEW: return BACKGROUND;
        case ABILITY_STATE_FOREGROUND_FAILED: return FOREGROUND_FAILED;
        case ABILITY_STATE_INVALID_WINDOW_MODE: return FOREGROUND_INVALID_MODE;
        case ABILITY_STATE_WINDOW_FREEZED: return FOREGROUND_WINDOW_FREEZED;
        case ABILITY_STATE_DO_NOTHING: return FOREGROUND_DO_NOTHING;
        case ABILITY_STATE_BACKGROUND_FAILED: return BACKGROUND_FAILED;
        default: return DEFAULT_INVAL_VALUE;
    }
}
}  // namespace AAFwk
}  // namespace OHOS

