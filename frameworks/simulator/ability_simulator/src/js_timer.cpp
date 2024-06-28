/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_timer.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
class JsTimer;

std::atomic<uint32_t> g_callbackId(1);
std::mutex g_mutex;
std::unordered_map<uint32_t, std::shared_ptr<JsTimer>> g_timerTable;

class JsTimer final {
public:
    JsTimer(napi_env env, const std::shared_ptr<NativeReference> &jsFunction, uint32_t id)
        : env_(env), jsFunction_(jsFunction), id_(id)
    {
        uv_loop_s* loop = nullptr;
        napi_get_uv_event_loop(env_, &loop);
        if (loop == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY_SIM, "loop == nullptr.");
            return;
        }
        uv_timer_init(loop, &timerReq_);
        timerReq_.data = this;
    }

    ~JsTimer()
    {
        uv_timer_stop(&timerReq_);
    }

    void Start(int64_t timeout, int64_t repeat)
    {
        uv_timer_start(&timerReq_, [](uv_timer_t *timerReq) {
            auto me = static_cast<JsTimer*>(timerReq->data);
            me->OnTimeout();
        }, timeout, repeat);
    }

    void OnTimeout()
    {
        std::vector<napi_value> args;
        args.reserve(jsArgs_.size());
        for (auto arg : jsArgs_) {
            args.emplace_back(arg->GetNapiValue());
        }
        napi_value res = nullptr;
        napi_call_function(env_, CreateJsUndefined(env_),
            jsFunction_->GetNapiValue(), args.size(), args.data(), &res);

        if (uv_timer_get_repeat(&timerReq_) == 0) {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_timerTable.erase(id_);
        }
    }

    void PushArgs(const std::shared_ptr<NativeReference> &ref)
    {
        jsArgs_.emplace_back(ref);
    }

private:
    napi_env env_;
    std::shared_ptr<NativeReference> jsFunction_;
    std::vector<std::shared_ptr<NativeReference>> jsArgs_;
    uv_timer_t timerReq_;
    uint32_t id_ = 0;
};

napi_value StartTimeoutOrInterval(napi_env env, napi_callback_info info, bool isInterval)
{
    if (env == nullptr || info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Start timeout or interval failed with env or callback info is nullptr.");
        return nullptr;
    }
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    // parameter check, must have at least 2 params
    if (argc < 2 ||!CheckTypeForNapiValue(env, argv[0], napi_function)
        || !CheckTypeForNapiValue(env, argv[1], napi_number)) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Set callback timer failed with invalid parameter.");
        return CreateJsUndefined(env);
    }

    // parse parameter
    napi_ref ref = nullptr;
    napi_create_reference(env, argv[0], 1, &ref);
    std::shared_ptr<NativeReference> jsFunction(reinterpret_cast<NativeReference*>(ref));
    int64_t delayTime = 0;
    napi_get_value_int64(env, argv[1], &delayTime);
    uint32_t callbackId = g_callbackId.fetch_add(1, std::memory_order_relaxed);

    auto task = std::make_shared<JsTimer>(env, jsFunction, callbackId);
    for (size_t index = 2; index < argc; ++index) {
        napi_ref taskRef = nullptr;
        napi_create_reference(env, argv[index], 1, &taskRef);
        task->PushArgs(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(taskRef)));
    }

    // if setInterval is called, interval must not be zero for repeat, so set to 1ms
    int64_t interval = 0;
    if (isInterval) {
        interval = delayTime > 0 ? delayTime : 1;
    }
    task->Start(delayTime, interval);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_timerTable.emplace(callbackId, task);
    }

    return CreateJsValue(env, callbackId);
}

napi_value StartTimeout(napi_env env, napi_callback_info info)
{
    return StartTimeoutOrInterval(env, info, false);
}

napi_value StartInterval(napi_env env, napi_callback_info info)
{
    return StartTimeoutOrInterval(env, info, true);
}

napi_value StopTimeoutOrInterval(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Stop timeout or interval failed with env or callback info is nullptr.");
        return nullptr;
    }
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    // parameter check, must have at least 1 param
    if (argc < 1 || !CheckTypeForNapiValue(env, argv[0], napi_number)) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Clear callback timer failed with invalid parameter.");
        return CreateJsUndefined(env);
    }
    uint32_t callbackId = 0;
    napi_get_value_uint32(env, argv[0], &callbackId);
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_timerTable.erase(callbackId);
    }
    return CreateJsUndefined(env);
}
}

void InitTimer(napi_env env, napi_value globalObject)
{
    const char *moduleName = "AsJsTimer";
    BindNativeFunction(env, globalObject, "setTimeout", moduleName, StartTimeout);
    BindNativeFunction(env, globalObject, "setInterval", moduleName, StartInterval);
    BindNativeFunction(env, globalObject, "clearTimeout", moduleName, StopTimeoutOrInterval);
    BindNativeFunction(env, globalObject, "clearInterval", moduleName, StopTimeoutOrInterval);
}
} // namespace AbilityRuntime
} // namespace OHOS