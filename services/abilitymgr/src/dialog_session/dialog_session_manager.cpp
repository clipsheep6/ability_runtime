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

#include "dialog_session_manager.h"

// #include <random>
// #include <string>
// #include <chrono>
// #include "ability_record.h"
// #include "ability_util.h"
// #include "hilog_wrapper.h"
// #include "int_wrapper.h"
// #include "string_wrapper.h"
// #include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {
DialogSessionManager &DialogSessionManager::GetInstance()
{
    static DialogSessionManager dialogSessionManager;
    return dialogSessionManager;
}


}  // namespace AAFwk
}  // namespace OHOS
