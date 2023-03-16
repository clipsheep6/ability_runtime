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
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS {
namespace JsEnv {
JsEnvironment::JsEnvironment(std::shared_ptr<JsEnvironmentImpl> impl) : impl_(impl)
{
    JSENV_LOG_D("Js environment costructor.");
}

JsEnvironment::~JsEnvironment()
{
    JSENV_LOG_D("Js environment destructor.");

    if (engine_ != nullptr) {
        engine_->DeleteEngine();
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

    engine_ = new ArkNativeEngine(vm_, jsEngine);
    return true;
}

void JsEnvironment::StartDebuggger(bool needBreakPoint)
{
}

void JsEnvironment::StopDebugger()
{
}

void JsEnvironment::InitTimerModule()
{
    if (impl_ != nullptr) {
        impl_->InitTimerModule();
    }
}

void JsEnvironment::InitConsoleLogModule()
{
    if (impl_ != nullptr) {
        impl_->InitConsoleLogModule();
    }
}

void JsEnvironment::InitWorkerModule()
{
    if (impl_ != nullptr) {
        impl_->InitWorkerModule();
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

void JsEnvironment::RemoveTask(const std::string& name)
{
    if (impl_ != nullptr) {
        impl_->RemoveTask(name);
    }
}
} // namespace JsEnv
} // namespace OHOS
