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
#include "ecmascript/napi/include/jsnapi.h"
#include "js_environment_impl.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace JsEnv {
class JsEnvironmentImpl;
class JsEnvironment final {
public:
    JsEnvironment() {}
    explicit JsEnvironment(std::shared_ptr<JsEnvironmentImpl> impl);
    ~JsEnvironment();

    bool Initialize(const panda::RuntimeOption& pandaOption, void* jsEngine);

    NativeEngine* GetNativeEngine() const
    {
        return engine_;
    }

    panda::ecmascript::EcmaVM* GetVM() const
    {
        return vm_;
    }

    void StartDebuggger(bool needBreakPoint);

    void StopDebugger();

    void InitTimerModule();

    void InitConsoleLogModule();

    void InitWorkerModule();

    void InitSyscapModule();

    void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime);

    void RemoveTask(const std::string& name);
private:
    std::shared_ptr<JsEnvironmentImpl> impl_ = nullptr;
    NativeEngine* engine_ = nullptr;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_H
