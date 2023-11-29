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

#ifndef OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_UTILS_H

#include <map>
#include <string>
#include <vector>

#include "js_dialog_session.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "dialog_session_record.h"

namespace OHOS {
namespace AppExecFwk {
napi_value WrapArrayDialogAbilityInfoToJS(napi_env env, const std::vector<DialogAbilityInfo> &value);
napi_value WrapDialogSessionInfo(napi_env env, const AAFwk::DialogSessionInfo &dialogSessionInfo);
napi_value WrapDialogAbilityInfo(napi_env env, const AAFwk::DialogAbilityInfo &dialogAbilityInfo);
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_UTILS_H
