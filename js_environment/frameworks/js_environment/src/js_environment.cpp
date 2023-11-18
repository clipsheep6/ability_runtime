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

#include "js_environment.h"

#include "js_env_logger.h"
#include "js_environment_impl.h"
#include "native_engine/native_engine.h"
#include "uncaught_exception_callback.h"
#include "commonlibrary/ets_utils/js_sys_module/console/console.h"

namespace OHOS {
namespace JsEnv {

static panda::DFXJSNApi::ProfilerType ConvertProfilerType(JsEnvironment::PROFILERTYPE type)
{
    if (type == JsEnvironment::PROFILERTYPE::PROFILERTYPE_CPU) {
        return panda::DFXJSNApi::ProfilerType::CPU_PROFILER;
    } else {
        return panda::DFXJSNApi::ProfilerType::HEAP_PROFILER;
    }
}

JsEnvironment::JsEnvironment(std::unique_ptr<JsEnvironmentImpl> impl) : impl_(std::move(impl))
{
    JSENV_LOG_I("Js environment costructor.");
}

JsEnvironment::~JsEnvironment()
{
    JSENV_LOG_I("Js environment destructor.");

    if (engine_ != nullptr) {
        delete engine_;
        engine_ = nullptr;
    }

    if (vm_ != nullptr) {
        panda::JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

bool JsEnvironment::Initialize(const panda::RuntimeOption& pandaOption, void* jsEngine)
{
    JSENV_LOG_D("Js environment initialize.");
    vm_ = panda::JSNApi::CreateJSVM(pandaOption);
    if (vm_ == nullptr) {
        JSENV_LOG_E("Create vm failed.");
        return false;
    }

    engine_ = new NativeEngine(vm_, jsEngine);
    return true;
}

void JsEnvironment::InitTimerModule()
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid native engine.");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitTimerModule(engine_);
    }
}

void JsEnvironment::InitWorkerModule(std::shared_ptr<WorkerInfo> workerInfo)
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid native engine.");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitWorkerModule(engine_, workerInfo);
    }
}

void JsEnvironment::InitSyscapModule()
{
    if (impl_ != nullptr) {
        impl_->InitSyscapModule();
    }
}

void JsEnvironment::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    if (impl_ != nullptr) {
        impl_->PostTask(task, name, delayTime);
    }
}

void JsEnvironment::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    if (impl_ != nullptr) {
        impl_->PostSyncTask(task, name);
    }
}

void JsEnvironment::RemoveTask(const std::string& name)
{
    if (impl_ != nullptr) {
        impl_->RemoveTask(name);
    }
}

void JsEnvironment::InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj)
{
    sourceMapOperator_ = operatorObj;
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return;
    }

    auto translateBySourceMapFunc = [&](const std::string& rawStack) {
        return sourceMapOperator_->TranslateBySourceMap(rawStack);
    };
    engine_->RegisterTranslateBySourceMap(translateBySourceMapFunc);

    auto translateUrlBySourceMapFunc = [&](std::string& url, int& line, int& column) {
        return sourceMapOperator_->TranslateUrlPositionBySourceMap(url, line, column);
    };
    engine_->RegisterSourceMapTranslateCallback(translateUrlBySourceMapFunc);
}

void JsEnvironment::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo)
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return;
    }

    engine_->RegisterNapiUncaughtExceptionHandler(NapiUncaughtExceptionCallback(uncaughtExceptionInfo.uncaughtTask,
        sourceMapOperator_, reinterpret_cast<napi_env>(engine_)));
}

bool JsEnvironment::LoadScript(const std::string& path, std::vector<uint8_t>* buffer, bool isBundle)
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return false;
    }

    if (buffer == nullptr) {
        return engine_->RunScriptPath(path.c_str());
    }

    return engine_->RunScriptBuffer(path.c_str(), *buffer, isBundle) != nullptr;
}

bool JsEnvironment::StartDebugger(const char* libraryPath, bool needBreakPoint, uint32_t instanceId, bool isDebug)
{
    if (vm_ == nullptr) {
        JSENV_LOG_E("Invalid vm.");
        return false;
    }

    panda::JSNApi::DebugOption debugOption = {libraryPath, needBreakPoint};
    auto debuggerPostTask = [weak = weak_from_this()](std::function<void()>&& task) {
        auto jsEnv = weak.lock();
        if (jsEnv == nullptr) {
            JSENV_LOG_E("JsEnv is invalid.");
            return;
        }
        jsEnv->PostTask(task, "JsEnvironment:StartDebugger");
    };
    return panda::JSNApi::StartDebugger(vm_, debugOption, instanceId, debuggerPostTask);
}

void JsEnvironment::StopDebugger()
{
    if (vm_ == nullptr) {
        JSENV_LOG_E("Invalid vm.");
        return;
    }

    (void)panda::JSNApi::StopDebugger(vm_);
}

void JsEnvironment::InitConsoleModule()
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return;
    }

    if (impl_ != nullptr) {
        impl_->InitConsoleModule(engine_);
    }
}

bool JsEnvironment::InitLoop()
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return false;
    }

    if (impl_ != nullptr) {
        impl_->InitLoop(engine_);
    }
    return true;
}

void JsEnvironment::DeInitLoop()
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return;
    }

    if (impl_ != nullptr) {
        impl_->DeInitLoop(engine_);
    }
}

bool JsEnvironment::LoadScript(const std::string& path, uint8_t* buffer, size_t len, bool isBundle)
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid Native Engine.");
        return false;
    }

    return engine_->RunScriptBuffer(path, buffer, len, isBundle);
}

void JsEnvironment::StartProfiler(const char* libraryPath, uint32_t instanceId, PROFILERTYPE profiler,
    int32_t interval, bool isDebug)
{
    if (vm_ == nullptr) {
        JSENV_LOG_E("Invalid vm.");
        return;
    }

    auto debuggerPostTask = [weak = weak_from_this()](std::function<void()>&& task) {
        auto jsEnv = weak.lock();
        if (jsEnv == nullptr) {
            JSENV_LOG_E("JsEnv is invalid.");
            return;
        }
        jsEnv->PostTask(task, "JsEnvironment::StartProfiler");
    };

    panda::DFXJSNApi::ProfilerOption option;
    option.libraryPath = libraryPath;
    option.profilerType = ConvertProfilerType(profiler);
    option.interval = interval;

    panda::DFXJSNApi::StartProfiler(vm_, option, instanceId, debuggerPostTask);
}

void JsEnvironment::ReInitJsEnvImpl(std::unique_ptr<JsEnvironmentImpl> impl)
{
    JSENV_LOG_I("ReInit jsenv impl.");
    impl_ = std::move(impl);
}

void JsEnvironment::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate)
{
    if (engine_ == nullptr) {
        JSENV_LOG_E("Invalid native engine.");
        return;
    }

    engine_->SetModuleLoadChecker(moduleCheckerDelegate);
}

void JsEnvironment::SetRequestAotCallback(const RequestAotCallback& cb)
{
    if (vm_ == nullptr) {
        JSENV_LOG_E("Invalid vm.");
        return;
    }

    panda::JSNApi::SetRequestAotCallback(vm_, cb);
}

void JsEnvironment::SetDeviceDisconnectCallback(const std::function<bool()> &cb)
{
    if (vm_ == nullptr) {
        JSENV_LOG_E("Invalid vm.");
        return;
    }
    panda::JSNApi::SetDeviceDisconnectCallback(vm_, std::move(cb));
}
} // namespace JsEnv
} // namespace OHOS
