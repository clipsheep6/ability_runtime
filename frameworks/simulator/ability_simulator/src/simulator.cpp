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

#include "simulator.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "hilog_wrapper.h"
#include "js_console_log.h"
#include "js_module_searcher.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "native_engine/impl/ark/ark_native_engine.h"

extern const char _binary_jsMockSystemPlugin_abc_start[];
extern const char _binary_jsMockSystemPlugin_abc_end[];

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_ARK_PROPERTIES = -1;
constexpr size_t DEFAULT_GC_THREAD_NUM = 7;
constexpr size_t DEFAULT_LONG_PAUSE_TIME = 40;

int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
{
    HILOG_INFO("ArkLog: %{public}s", message);
    return 0;
}

class SimulatorImpl : public Simulator {
public:
    SimulatorImpl() = default;
    ~SimulatorImpl();

    bool Initialize(const Options& options);

    int64_t StartAbility(const std::string& abilityName, TerminateCallback callback) override;
    void TerminateAbility(const int64_t abilityId) override;

    int64_t CreateForm(const std::string& formName, FormUpdateCallback callback) override;
    void RequestUpdateForm(const int64_t formId) override;
    void DestroyForm(const int64_t formId) override;

private:
    bool OnInit() const;
    void Run();

    Options options_;
    std::thread thread_;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
    std::unique_ptr<NativeEngine> nativeEngine_;

    int64_t currentId_ = 0;
    std::unordered_map<int64_t, std::shared_ptr<NativeReference>> abilities_;
};

template <class T>
class ResultWaiter final {
public:
    T WaitForResult()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&] { return !waiting_; });
        return result_;
    }

    void NotifyResult(T result)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        waiting_ = false;
        result_ = result;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool waiting_ = true;
    T result_ = false;
};

SimulatorImpl::~SimulatorImpl()
{
    if (nativeEngine_) {
        uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
        if (uvLoop != nullptr) {
            uv_work_t work;
            uv_queue_work(uvLoop, &work, [](uv_work_t*) {}, [](uv_work_t* work, int32_t status) {
                HILOG_INFO("Simulator stop uv loop");
                uv_stop(work->loop);
            });
        }
    }

    if (thread_.joinable()) {
        HILOG_INFO("Simulator Waiting for thread stopped");
        thread_.join();
        HILOG_INFO("Simulator thread stopped");
    }
}

bool SimulatorImpl::Initialize(const Options& options)
{
    if (nativeEngine_) {
        HILOG_INFO("Simulator is already initialized");
        return true;
    }

    ResultWaiter<bool> waiter;

    options_ = options;
    thread_ = std::thread([&] {
        bool initResult = OnInit();
        if (!initResult) {
            waiter.NotifyResult(false);
            return;
        }

        uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
        if (uvLoop == nullptr) {
            waiter.NotifyResult(false);
            return;
        }

        uv_timer_t timerReq;
        uv_timer_init(uvLoop, &timerReq);
        timerReq.data = &waiter;
        uv_timer_start(&timerReq, [](uv_timer_t* timerReq) {
            auto watier = static_cast<ResultWaiter<bool>*>(timerReq->data);
            watier->NotifyResult(true);
            timerReq->data = nullptr;
        }, 0, 0);

        Run();
    });

    bool result = waiter.WaitForResult();
    if (!result && thread_.joinable()) {
        thread_.join();
    }

    return result;
}

void CallObjectMethod(NativeEngine& engine, NativeValue* value, const char *name, NativeValue *const *argv, size_t argc)
{
    HandleScope handleScope(engine);

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("%{public}s, Failed to get Ability object", __func__);
        return;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    engine.CallFunction(value, methodOnCreate, argv, argc);
}

int64_t SimulatorImpl::StartAbility(const std::string& abilitySrcPath, TerminateCallback callback)
{
    uv_work_t work;

    ResultWaiter<int64_t> waiter;

    work.data = new std::function<void()>([abilitySrcPath, this, &waiter] () {
        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
        NativeValue* exports = nativeEngine_->CreateObject();
        globalObj->SetProperty("exports", exports);

        if (nativeEngine_->RunScriptPath(abilitySrcPath.c_str()) == nullptr) {
            HILOG_ERROR("Failed to run script: %{public}s", abilitySrcPath.c_str());
            waiter.NotifyResult(-1);
            return;
        }

        NativeObject* exportsObj = ConvertNativeValueTo<NativeObject>(globalObj->GetProperty("exports"));
        if (exportsObj == nullptr) {
            HILOG_ERROR("Failed to get exports objcect: %{public}s", abilitySrcPath.c_str());
            waiter.NotifyResult(-1);
            return;
        }

        NativeValue* exportObj = exportsObj->GetProperty("default");
        if (exportObj == nullptr) {
            HILOG_ERROR("Failed to get default objcect: %{public}s", abilitySrcPath.c_str());
            waiter.NotifyResult(-1);
            return;
        }

        NativeValue* instanceValue = nativeEngine_->CreateInstance(exportObj, nullptr, 0);
        if (instanceValue == nullptr) {
            HILOG_ERROR("Failed to create object instance");
            waiter.NotifyResult(-1);
            return;
        }

        CallObjectMethod(*nativeEngine_, instanceValue, "onCreate", nullptr, 0);
        CallObjectMethod(*nativeEngine_, instanceValue, "onWindowStageCreate", nullptr, 0);
        CallObjectMethod(*nativeEngine_, instanceValue, "onForeground", nullptr, 0);

        int64_t id = ++currentId_;
        abilities_.emplace(id, nativeEngine_->CreateReference(instanceValue, 1));

        waiter.NotifyResult(id);
    });

    uv_queue_work(nativeEngine_->GetUVLoop(), &work, [](uv_work_t*) {}, [](uv_work_t* work, int32_t status) {
        auto func = static_cast<std::function<void()>*>(work->data);
        (*func)();
        delete func;
    });

    return waiter.WaitForResult();
}

void SimulatorImpl::TerminateAbility(const int64_t abilityId)
{
    uv_work_t work;

    ResultWaiter<bool> waiter;

    work.data = new std::function<void()>([abilityId, this, &waiter] () {
        auto it = abilities_.find(abilityId);
        if (it == abilities_.end()) {
            waiter.NotifyResult(false);
            return;
        }

        std::shared_ptr<NativeReference> ref = it->second;
        abilities_.erase(it);

        auto instanceValue = ref->Get();
        if (instanceValue == nullptr) {
            waiter.NotifyResult(false);
            return;
        }

        CallObjectMethod(*nativeEngine_, instanceValue, "onBackground", nullptr, 0);
        CallObjectMethod(*nativeEngine_, instanceValue, "onWindowStageDestroy", nullptr, 0);
        CallObjectMethod(*nativeEngine_, instanceValue, "onDestroy", nullptr, 0);

        waiter.NotifyResult(true);
    });

    uv_queue_work(nativeEngine_->GetUVLoop(), &work, [](uv_work_t*) {}, [](uv_work_t* work, int32_t status) {
        auto func = static_cast<std::function<void()>*>(work->data);
        (*func)();
        delete func;
    });

    waiter.WaitForResult();
}

int64_t SimulatorImpl::CreateForm(const std::string& formSrcPath, FormUpdateCallback callback)
{
    return -1;
}

void SimulatorImpl::RequestUpdateForm(const int64_t formId)
{
}

void SimulatorImpl::DestroyForm(const int64_t formId)
{
}

bool SimulatorImpl::OnInit() const
{
    panda::RuntimeOption pandaOption;
    pandaOption.SetArkProperties(DEFAULT_ARK_PROPERTIES);
    pandaOption.SetGcThreadNum(DEFAULT_GC_THREAD_NUM);
    pandaOption.SetLongPauseTime(DEFAULT_LONG_PAUSE_TIME);
    pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
    pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
    pandaOption.SetLogBufPrint(PrintVmLog);
    pandaOption.SetEnableAsmInterpreter(true);
    pandaOption.SetAsmOpcodeDisableRange("");
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        return false;
    }

    panda::JSNApi::SetHostResolvePathTracker(vm_, JsModuleSearcher(""));
    auto nativeEngine = std::make_unique<ArkNativeEngine>(vm_, nullptr);

    HandleScope handleScope(*nativeEngine);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    InitConsoleLogModule(*nativeEngine, *globalObj);
    InitTimerModule(*nativeEngine, *globalObj);

    globalObj->SetProperty("group", nativeEngine->CreateObject());

    uintptr_t bufferStart = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_start);
    uintptr_t bufferEnd = reinterpret_cast<uintptr_t>(_binary_jsMockSystemPlugin_abc_end);
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(bufferStart);
    size_t size = bufferEnd - bufferStart;

    panda::JSNApi::Execute(vm_, buffer, size, "_GLOBAL::func_main_0");

    NativeValue* mockRequireNapi = globalObj->GetProperty("requireNapi");
    globalObj->SetProperty("mockRequireNapi", mockRequireNapi);
    
    BindNativeFunction(*nativeEngine, *globalObj, "requireNapi", [](NativeEngine* engine, NativeCallbackInfo* info) {
        NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
        NativeValue* requireNapi = globalObj->GetProperty("requireNapiPreview");

        NativeValue* result = engine->CallFunction(engine->CreateUndefined(), requireNapi, info->argv, info->argc);
        if (result->TypeOf() != NATIVE_UNDEFINED) {
            return result;
        }

        NativeValue* mockRequireNapi = globalObj->GetProperty("mockRequireNapi");
        return engine->CallFunction(engine->CreateUndefined(), mockRequireNapi, info->argv, info->argc);
    });

    nativeEngine_ = std::move(nativeEngine);
    return true;
}

void SimulatorImpl::Run()
{
    uv_loop_t* uvLoop = nativeEngine_->GetUVLoop();
    if (uvLoop != nullptr) {
        HILOG_INFO("Simulator start uv loop");
        uv_run(uvLoop, UV_RUN_DEFAULT);
        HILOG_INFO("Simulator uv loop stopped");
    }

    abilities_.clear();
    nativeEngine_.reset();
    panda::JSNApi::DestroyJSVM(vm_);
    vm_ = nullptr;
}
}

std::unique_ptr<Simulator> Simulator::Create(const Options& options)
{
    auto simulator = std::make_unique<SimulatorImpl>();
    if (simulator->Initialize(options)) {
        return simulator;
    }
    return nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS