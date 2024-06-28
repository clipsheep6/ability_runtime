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

#ifndef OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H

#include "js_auto_fill_extension_util.h"
#include "js_runtime_utils.h"
#include "session_info.h"
#include "view_data.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
class JsSaveRequestCallback {
public:
    JsSaveRequestCallback(const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow);
    virtual ~JsSaveRequestCallback() = default;

    static napi_value CreateJsSaveRequestCallback(napi_env env,
        const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow);
    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value SaveRequestSuccess(napi_env env, napi_callback_info info);
    static napi_value SaveRequestFailed(napi_env env, napi_callback_info info);

private:
    napi_value OnSaveRequestSuccess(napi_env env, NapiCallbackInfo &info);
    napi_value OnSaveRequestFailed(napi_env env, NapiCallbackInfo &info);
    void SendResultCodeAndViewData(const JsAutoFillExtensionUtil::AutoFillResultCode &resultCode);

    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H
