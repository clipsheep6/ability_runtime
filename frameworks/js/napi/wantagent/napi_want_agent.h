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
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    NativeEngine *engine = nullptr;
    std::unique_ptr<NativeReference> nativeRef = nullptr;
};

struct TriggerReceiveDataWorker {
    std::shared_ptr<WantAgent> wantAgent;
    AAFwk::Want want;
    int resultCode;
    std::string resultData;
    AAFwk::WantParams resultExtras;
    NativeEngine *engine = nullptr;
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
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* Equal(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetWant(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetOperationType(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetBundleName(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetUid(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* Cancel(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* Trigger(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetWantAgent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* NapiGetWant(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* NapiTrigger(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* NapiGetWantAgent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* NapiGetOperationType(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* WrapWantAgent(NativeEngine &engine, const std::shared_ptr<WantAgent> &wantAgent);
    static void UnwrapWantAgent(NativeEngine &engine, NativeValue *jsParam, void** result);

private:
    NativeValue* OnEqual(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetWant(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetOperationType(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetUid(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnCancel(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnTrigger(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetWantAgent(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiGetWant(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiTrigger(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiGetWantAgent(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiGetOperationType(NativeEngine &engine, NativeCallbackInfo &info);
    int32_t UnWrapTriggerInfoParam(NativeEngine &engine, NativeCallbackInfo &info,
        std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
        std::shared_ptr<TriggerCompleteCallBack> &triggerObj);
    int32_t GetTriggerInfo(NativeEngine &engine, NativeValue *param, TriggerInfo &triggerInfo);
    int32_t GetWantAgentParam(NativeEngine &engine, NativeCallbackInfo &info, WantAgentWantsParas &paras);
};

class TriggerCompleteCallBack : public CompletedCallback {
public:
    TriggerCompleteCallBack();
    virtual ~TriggerCompleteCallBack();

public:
    void OnSendFinished(const AAFwk::Want &want, int resultCode, const std::string &resultData,
        const AAFwk::WantParams &resultExtras) override;
    void SetCallbackInfo(NativeEngine &engine, NativeReference *ref);
    void SetWantAgentInstance(const std::shared_ptr<WantAgent> &wantAgent);

private:
    CallbackInfo triggerCompleteInfo_;
};

NativeValue* JsWantAgentInit(NativeEngine *engine, NativeValue *exportObj);
NativeValue* WantAgentFlagsInit(NativeEngine *engine);
NativeValue* WantAgentOperationTypeInit(NativeEngine *engine);
napi_value NapiGetNull(napi_env env);
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_WANT_AGENT_H
