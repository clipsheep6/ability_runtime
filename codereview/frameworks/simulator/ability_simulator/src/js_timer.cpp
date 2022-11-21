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

#include "js_timer.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>

#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif

#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
class JsTimer;

std::atomic<uint32_t> g_callbackId(1);
std::mutex g_mutex;
std::unordered_map<uint32_t, std::shared_ptr<JsTimer>> g_timerTable;

class JsTimer final {
public:
    JsTimer(NativeEngine& nativeEngine, const std::shared_ptr<NativeReference>& jsFunction, uint32_t id)
        : nativeEngine_(nativeEngine), jsFunction_(jsFunction), id_(id)
    {
        uv_timer_init(nativeEngine.GetUVLoop(), &timerReq_);
        timerReq_.data = this;
    }

    ~JsTimer()
    {
        uv_timer_stop(&timerReq_);
    }

    void Start(int64_t timeout, int64_t repeat)
    {
        uv_timer_start(&timerReq_, [](uv_timer_t* timerReq) {
            auto me = static_cast<JsTimer*>(timerReq->data);
            me->OnTimeout();
        }, timeout, repeat);
    }

    void OnTimeout()
    {
#ifdef SUPPORT_GRAPHICS
        // call js function
        ContainerScope containerScope(containerScopeId_);
#endif
        HandleScope handleScope(nativeEngine_);

        std::vector<NativeValue*> args;
        args.reserve(jsArgs_.size());
        for (auto arg : jsArgs_) {
            args.emplace_back(arg->Get());
        }
        nativeEngine_.CallFunction(nativeEngine_.CreateUndefined(), jsFunction_->Get(), args.data(), args.size());

        if (uv_timer_get_repeat(&timerReq_) == 0) {
            std::lock_guard<std::mutex> lock(g_mutex);
            g_timerTable.erase(id_);
        }
    }

    void PushArgs(const std::shared_ptr<NativeReference>& ref)
    {
        jsArgs_.emplace_back(ref);
    }

private:
    NativeEngine& nativeEngine_;
    std::shared_ptr<NativeReference> jsFunction_;
    std::vector<std::shared_ptr<NativeReference>> jsArgs_;
    uv_timer_t timerReq_;
    uint32_t id_ = 0;
#ifdef SUPPORT_GRAPHICS
    int32_t containerScopeId_ = ContainerScope::CurrentId();
#endif
};

NativeValue* StartTimeoutOrInterval(NativeEngine* engine, NativeCallbackInfo* info, bool isInterval)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Start timeout or interval failed with engine or callback info is nullptr.");
        return nullptr;
    }

    // parameter check, must have at least 2 params
    if (info->argc < 2 || info->argv[0]->TypeOf() != NATIVE_FUNCTION || info->argv[1]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Set callback timer failed with invalid parameter.");
        return engine->CreateUndefined();
    }

    // parse parameter
    std::shared_ptr<NativeReference> jsFunction(engine->CreateReference(info->argv[0], 1));
    int64_t delayTime = *ConvertNativeValueTo<NativeNumber>(info->argv[1]);
    uint32_t callbackId = g_callbackId.fetch_add(1, std::memory_order_relaxed);

    auto task = std::make_shared<JsTimer>(*engine, jsFunction, callbackId);
    for (size_t index = 2; index < info->argc; ++index) {
        task->PushArgs(std::shared_ptr<NativeReference>(engine->CreateReference(info->argv[index], 1)));
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

    return engine->CreateNumber(callbackId);
}

NativeValue* StartTimeout(NativeEngine* engine, NativeCallbackInfo* info)
{
    return StartTimeoutOrInterval(engine, info, false);
}

NativeValue* StartInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    return StartTimeoutOrInterval(engine, info, true);
}

NativeValue* StopTimeoutOrInterval(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("Stop timeout or interval failed with engine or callback info is nullptr.");
        return nullptr;
    }

    // parameter check, must have at least 1 param
    if (info->argc < 1 || info->argv[0]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("Clear callback timer failed with invalid parameter.");
        return engine->CreateUndefined();
    }

    uint32_t callbackId = *ConvertNativeValueTo<NativeNumber>(info->argv[0]);
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_timerTable.erase(callbackId);
    }
    return engine->CreateUndefined();
}
}

void InitTimerModule(NativeEngine& engine, NativeObject& globalObject)
{
    const char *moduleName = "AsJsTimer";
    BindNativeFunction(engine, globalObject, "setTimeout", moduleName, StartTimeout);
    BindNativeFunction(engine, globalObject, "setInterval", moduleName, StartInterval);
    BindNativeFunction(engine, globalObject, "clearTimeout", moduleName, StopTimeoutOrInterval);
    BindNativeFunction(engine, globalObject, "clearInterval", moduleName, StopTimeoutOrInterval);
}
} // namespace AbilityRuntime
} // namespace OHOS