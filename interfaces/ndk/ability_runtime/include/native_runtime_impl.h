/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ABILITY_ABILITY_RUNTIME_NATIVE_RUNTIME_IMPL_H
#define ABILITY_ABILITY_RUNTIME_NATIVE_RUNTIME_IMPL_H

#include "js_runtime.h"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "native_err_code.h"
using Options = OHOS::AbilityRuntime::Runtime::Options;
namespace OHOS {
namespace AbilityRuntime {
class NativeRuntimeImpl {
public:
    NativeRuntimeImpl(const NativeRuntimeImpl&) = delete;
    NativeRuntimeImpl& operator=(const NativeRuntimeImpl&) = delete;
    static NativeRuntimeImpl& GetNativeRuntimeImpl();
    int32_t CreateJsEnv(const Options& options, std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    int32_t RemoveJsEnv(napi_env env);
    int32_t Init(const Options& options, napi_env env);
    
private:
    NativeRuntimeImpl();
    ~NativeRuntimeImpl();
    int32_t AddEnv(napi_env env, std::shared_ptr<JsEnv::JsEnvironment> jsEnv);
    panda::ecmascript::EcmaVM* GetEcmaVm(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv) const;
    std::shared_ptr<JsEnv::JsEnvironment> GetJsEnv(napi_env env);
    void LoadAotFile(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void InitConsoleModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj,
        const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void InitTimerModule(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate,
        const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void SetRequestAotCallback(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    bool InitLoop(const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    void InitWorkerModule(const Options& options, const std::shared_ptr<JsEnv::JsEnvironment>& jsEnv);
    std::unordered_map<napi_env, std::shared_ptr<JsEnv::JsEnvironment>> envMap_;
    std::unordered_set<pid_t> threadIds_;
    bool preloaded_ = false;
    std::mutex envMutex_;
};
}
}
#endif // ABILITY_ABILITY_RUNTIME_NATIVE_RUNTIME_IMPL_H