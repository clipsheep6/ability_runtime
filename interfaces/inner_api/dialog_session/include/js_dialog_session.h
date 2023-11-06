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

#ifndef OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_H
#define OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_H

#include "js_dialog_session_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class JsDialogSession {
public:
    static napi_value GetDialogSessionInfo(napi_env env, napi_callback_info info);
    static napi_value SendDialogResult(napi_env env, napi_callback_info info);
protected:
    virtual napi_value OnGetDialogSessionInfo(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnSendDialogResult(napi_env env, NapiCallbackInfo& info);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DIALOG_SESSION_H
