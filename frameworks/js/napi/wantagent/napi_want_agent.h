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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_WANT_AGENT_H
#define OHOS_ABILITY_RUNTIME_NAPI_WANT_AGENT_H

#include <map>
#include <memory>
#include <mutex>
#include <uv.h>

#include "ability.h"
#include "completed_callback.h"
#include "context/application_context.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "trigger_info.h"
#include "want.h"
#include "want_agent.h"
#include "want_agent_constant.h"
#include "want_params.h"

namespace OHOS {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime::WantAgent;

const uint8_t NUMBER_OF_PARAMETERS_ZERO = 0;
const uint8_t NUMBER_OF_PARAMETERS_ONE = 1;
const uint8_t NUMBER_OF_PARAMETERS_TWO = 2;
const uint8_t NUMBER_OF_PARAMETERS_THREE = 3;
const uint8_t NUMBER_OF_PARAMETERS_FOUR = 4;
const uint8_t NUMBER_OF_PARAMETERS_FIVE = 5;
const uint8_t NUMBER_OF_PARAMETERS_SIX = 6;
const uint8_t NUMBER_OF_PARAMETERS_SEVEN = 7;
const uint8_t NUMBER_OF_PARAMETERS_EIGHT = 8;
const uint8_t NUMBER_OF_PARAMETERS_NINE = 9;

class TriggerCompleteCallBack;

struct CallbackInfo {
    WantAgent* wantAgent = nullptr;
    napi_env env = nullptr;
    std::unique_ptr<NativeReference> nativeRef = nullptr;
};

struct TriggerReceiveDataWorker {
    WantAgent* wantAgent;
    AAFwk::Want want;
    int resultCode;
    std::string resultData;
    AAFwk::WantParams resultExtras;
    napi_env env = nullptr;
    std::unique_ptr<NativeReference> nativeRef = nullptr;
};

struct WantAgentWantsParas {
    std::vector<std::shared_ptr<AAFwk::Want>> wants = {};
    int32_t operationType = -1;
    int32_t requestCode = -1;
    std::vector<WantAgentConstant::Flags> wantAgentFlags = {};
    AAFwk::WantParams extraInfo = {};
};

class JsWantAgent : public std::enable_shared_from_this<JsWantAgent> {
public:
    JsWantAgent() = default;
    ~JsWantAgent() = default;
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value Equal(napi_env env, napi_callback_info info);
    static napi_value GetWant(napi_env env, napi_callback_info info);
    static napi_value GetOperationType(napi_env env, napi_callback_info info);
    static napi_value GetBundleName(napi_env env, napi_callback_info info);
    static napi_value GetUid(napi_env env, napi_callback_info info);
    static napi_value Cancel(napi_env env, napi_callback_info info);
    static napi_value Trigger(napi_env env, napi_callback_info info);
    static napi_value GetWantAgent(napi_env env, napi_callback_info info);
    static napi_value NapiGetWant(napi_env env, napi_callback_info info);
    static napi_value NapiTrigger(napi_env env, napi_callback_info info);
    static napi_value NapiGetWantAgent(napi_env env, napi_callback_info info);
    static napi_value NapiGetOperationType(napi_env env, napi_callback_info info);
    static napi_value WrapWantAgent(napi_env env, WantAgent *wantAgent);
    static void UnwrapWantAgent(napi_env env, napi_value jsParam, void** result);

private:
    napi_value OnEqual(napi_env env, napi_callback_info info);
    napi_value OnGetWant(napi_env env, napi_callback_info info);
    napi_value OnGetOperationType(napi_env env, napi_callback_info info);
    napi_value OnGetBundleName(napi_env env, napi_callback_info info);
    napi_value OnGetUid(napi_env env, napi_callback_info info);
    napi_value OnCancel(napi_env env, napi_callback_info info);
    napi_value OnTrigger(napi_env env, napi_callback_info info);
    napi_value OnGetWantAgent(napi_env env, napi_callback_info info);
    napi_value OnNapiGetWant(napi_env env, napi_callback_info info);
    napi_value OnNapiTrigger(napi_env env, napi_callback_info info);
    napi_value OnNapiGetWantAgent(napi_env env, napi_callback_info info);
    napi_value OnNapiGetOperationType(napi_env env, napi_callback_info info);
    int32_t UnWrapTriggerInfoParam(napi_env env, napi_callback_info info,
        std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
        std::shared_ptr<TriggerCompleteCallBack> &triggerObj);
    int32_t GetTriggerInfo(napi_env env, napi_value param, TriggerInfo &triggerInfo);
    int32_t GetWantAgentParam(napi_env env, napi_callback_info info, WantAgentWantsParas &paras);
};

class TriggerCompleteCallBack : public CompletedCallback {
public:
    TriggerCompleteCallBack();
    virtual ~TriggerCompleteCallBack();

public:
    void OnSendFinished(const AAFwk::Want &want, int resultCode, const std::string &resultData,
        const AAFwk::WantParams &resultExtras) override;
    void SetCallbackInfo(napi_env env, NativeReference* ref);
    void SetWantAgentInstance(WantAgent* wantAgent);

private:
    CallbackInfo triggerCompleteInfo_;
};

napi_value JsWantAgentInit(napi_env env, napi_value exportObj);
napi_value WantAgentFlagsInit(napi_env env);
napi_value WantAgentOperationTypeInit(napi_env env);
napi_value NapiGetNull(napi_env env);
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_WANT_AGENT_H
