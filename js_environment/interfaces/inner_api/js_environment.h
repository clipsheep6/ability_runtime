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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H
#define OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H

#include <memory>
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "js_environment_impl.h"
#include "native_engine/native_engine.h"
#include "source_map_operator.h"
#include "uncaught_exception_callback.h"

namespace OHOS {
namespace JsEnv {
class JsEnvironmentImpl;
class JsEnvironment final : public std::enable_shared_from_this<JsEnvironment> {
public:
    JsEnvironment() {}
    explicit JsEnvironment(std::unique_ptr<JsEnvironmentImpl> impl);
    ~JsEnvironment();

    enum class PROFILERTYPE {
        PROFILERTYPE_CPU,
        PROFILERTYPE_HEAP
    };

    bool Initialize(const panda::RuntimeOption& pandaOption, void* jsEngine);

    NativeEngine* GetNativeEngine() const
    {
        return engine_;
    }

    panda::ecmascript::EcmaVM* GetVM() const
    {
        return vm_;
    }

    void InitTimerModule();

    void InitWorkerModule(std::shared_ptr<WorkerInfo> workerInfo);

    void InitSourceMap(const std::shared_ptr<JsEnv::SourceMapOperator> operatorObj);

    void InitSyscapModule();

    void PostTask(const std::function<void()>& task, const std::string& name = "", int64_t delayTime = 0);

    void PostSyncTask(const std::function<void()>& task, const std::string& name = "");

    void RemoveTask(const std::string& name);

    void RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo);
    bool LoadScript(const std::string& path, std::vector<uint8_t>* buffer = nullptr, bool isBundle = false);

    bool StartDebugger(const char* libraryPath, bool needBreakPoint, uint32_t instanceId);

    void StopDebugger();

    void InitConsoleModule();

    bool InitLoop();

    void DeInitLoop();

    bool LoadScript(const std::string& path, uint8_t *buffer, size_t len, bool isBundle);

    void StartProfiler(const char* libraryPath, uint32_t instanceId, PROFILERTYPE profiler, int32_t interval,
        const DebuggerPostTask &debuggerPostTask = {});

    void ReInitJsEnvImpl(std::unique_ptr<JsEnvironmentImpl> impl);

    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate);

private:
    std::unique_ptr<JsEnvironmentImpl> impl_ = nullptr;
    NativeEngine* engine_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
    std::shared_ptr<SourceMapOperator> sourceMapOperator_ = nullptr;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H
