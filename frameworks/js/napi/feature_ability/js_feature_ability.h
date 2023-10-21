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

#ifndef OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_H
#define OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_H

#include "ability.h"
#include "distribute_req_param.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "want.h"
#include "uri.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

class JsFeatureAbility final {
public:
    JsFeatureAbility() = default;
    ~JsFeatureAbility() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value CreateJsFeatureAbility(napi_env env);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value FinishWithResult(napi_env env, napi_callback_info info);
    static napi_value GetDeviceList(napi_env env, napi_callback_info info);
    static napi_value CallAbility(napi_env env, napi_callback_info info);
    static napi_value ContinueAbility(napi_env env, napi_callback_info info);
    static napi_value SubscribeAbilityEvent(napi_env env, napi_callback_info info);
    static napi_value UnsubscribeAbilityEvent(napi_env env, napi_callback_info info);
    static napi_value SendMsg(napi_env env, napi_callback_info info);
    static napi_value SubscribeMsg(napi_env env, napi_callback_info info);
    static napi_value UnsubscribeMsg(napi_env env, napi_callback_info info);
private:
    Ability* GetAbility(napi_env env);
    Want GetWant(DistributeReqParam &requestParam);
    bool CheckThenGetDeepLinkUri(const DistributeReqParam &requestParam, Uri &uri);
    bool UnWrapRequestParams(napi_env env, napi_value param, DistributeReqParam &requestParam);
    static napi_value CreateJsResult(napi_env env, int32_t errCode, const std::string &message);
    void GetExtraParams(const DistributeReqParam &requestParam, const Want &want);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnFinishWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDeviceList(napi_env env, const NapiCallbackInfo& info);
    napi_value OnCallAbility(napi_env env, const NapiCallbackInfo& info);
    napi_value OnContinueAbility(napi_env env, const NapiCallbackInfo& info);
    napi_value OnSubscribeAbilityEvent(napi_env env, const NapiCallbackInfo& info);
    napi_value OnUnsubscribeAbilityEvent(napi_env env, const NapiCallbackInfo& info);
    napi_value OnSendMsg(napi_env env, const NapiCallbackInfo& info);
    napi_value OnSubscribeMsg(napi_env env, const NapiCallbackInfo& info);
    napi_value OnUnsubscribeMsg(napi_env env, const NapiCallbackInfo& info);

    int requestCode_ = 0;
};

napi_value JsFeatureAbilityInit(napi_env env, napi_value exports);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_FEATURE_ABILITY_H
