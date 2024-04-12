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

#ifndef OHOS_ABILITY_RUNTIME_OHOS_JS_ENVIRONMENT_IMPL_H
#define OHOS_ABILITY_RUNTIME_OHOS_JS_ENVIRONMENT_IMPL_H

#include "js_environment_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class OHOSJsEnvironmentImpl : public JsEnv::JsEnvironmentImpl {
public:
    OHOSJsEnvironmentImpl();
    explicit OHOSJsEnvironmentImpl(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner);
    ~OHOSJsEnvironmentImpl() override;

    void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime) override;

    void PostSyncTask(const std::function<void()>& task, const std::string& name) override;

    void RemoveTask(const std::string& name) override;

    void InitTimerModule(NativeEngine* engine) override;

    void InitConsoleModule(NativeEngine* engine) override;

    bool InitLoop(NativeEngine* engine, bool isStage = true) override;

    void DeInitLoop(NativeEngine* engine) override;

    void InitWorkerModule(NativeEngine* engine, std::shared_ptr<JsEnv::WorkerInfo> workerInfo) override;

    void InitSyscapModule() override;

private:
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_OHOS_JS_ENVIRONMENT_IMPL_H
