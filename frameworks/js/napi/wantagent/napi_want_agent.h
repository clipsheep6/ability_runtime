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

struct AsyncGetWantAgentCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {nullptr};
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    WantAgentConstant::OperationType operationType;
    int32_t requestCode = -1;
    std::vector<WantAgentConstant::Flags> wantAgentFlags;
    std::shared_ptr<AAFwk::WantParams> extraInfo;
    std::shared_ptr<AbilityRuntime::ApplicationContext> context;
    std::shared_ptr<WantAgent> wantAgent;
};

struct CallbackInfo {
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    napi_env env = nullptr;
    napi_ref ref = nullptr;
};

struct TriggerReceiveDataWorker {
    napi_env env;
    napi_ref ref = nullptr;
    std::shared_ptr<WantAgent> wantAgent;
    AAFwk::Want want;
    int resultCode;
    std::string resultData;
    AAFwk::WantParams resultExtras;
};

struct WantAgentWantsParas {
    std::vector<std::shared_ptr<AAFwk::Want>> &wants;
    int32_t &operationType;
    int32_t &requestCode;
    std::vector<WantAgentConstant::Flags> &wantAgentFlags;
    AAFwk::WantParams &extraInfo;
};

struct AsyncGetOperationTypeCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[2] = {nullptr};
    std::shared_ptr<WantAgent> wantAgent;
    int32_t operationType;
    bool newInterface = false;
    int32_t errorCode = 0;
};

class JsWantAgent final {
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
    static NativeValue* NapiGetWant(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* NapiTrigger(NativeEngine *engine, NativeCallbackInfo *info);

private:
    NativeValue* OnEqual(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetWant(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetOperationType(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetBundleName(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetUid(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnCancel(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnTrigger(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiGetWant(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnNapiTrigger(NativeEngine &engine, NativeCallbackInfo &info);
    int32_t UnWrapTriggerInfoParam(NativeEngine &engine, NativeCallbackInfo &info,
    std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
    std::shared_ptr<TriggerCompleteCallBack> &triggerObj);
    int32_t GetTriggerInfo(NativeEngine &engine, NativeValue *param, TriggerInfo &triggerInfo);
};

class TriggerCompleteCallBack : public CompletedCallback {
public:
    TriggerCompleteCallBack();
    virtual ~TriggerCompleteCallBack();

public:
    void OnSendFinished(const AAFwk::Want &want, int resultCode, const std::string &resultData,
        const AAFwk::WantParams &resultExtras) override;
    void SetCallbackInfo(const napi_env &env, const napi_ref &ref);
    void SetWantAgentInstance(const std::shared_ptr<WantAgent> &wantAgent);

private:
    CallbackInfo triggerCompleteInfo_;
};
NativeValue* JsWantAgentInit(NativeEngine* engine, NativeValue* exportObj);
napi_value WantAgentInit(napi_env env, napi_value exports);

void SetNamedPropertyByInteger(napi_env env, napi_value dstObj, int32_t objName, const std::string &propName);
napi_value WantAgentFlagsInit(napi_env env, napi_value exports);
napi_value WantAgentOperationTypeInit(napi_env env, napi_value exports);

napi_value NAPI_GetWantAgent(napi_env env, napi_callback_info info);
napi_value NAPI_GetOperationType(napi_env env, napi_callback_info info);
napi_value GetCallbackErrorResult(napi_env env, int errCode);
napi_value NapiGetNull(napi_env env);
napi_value JSParaError(const napi_env &env, const bool bCallback);
napi_value GetOperationType(napi_env env, napi_callback_info info);

static std::unique_ptr<std::map<AsyncGetWantAgentCallbackInfo *, const int32_t>,
    std::function<void(std::map<AsyncGetWantAgentCallbackInfo *, const int32_t> *)>>
    g_WantAgentMap(new std::map<AsyncGetWantAgentCallbackInfo *, const int32_t>,
        [](std::map<AsyncGetWantAgentCallbackInfo *, const int32_t> *map) {
            if (map == nullptr) {
                return;
            }
            for (auto &item : *map) {
                if (item.first != nullptr) {
                    delete item.first;
                }
            }
            map->clear();
            delete map;
        });
static std::recursive_mutex g_mutex;
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_WANT_AGENT_H
