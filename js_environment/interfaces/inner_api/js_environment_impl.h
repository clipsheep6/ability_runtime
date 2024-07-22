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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H
#define OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H

#include <string>
#include "event_handler.h"
#include "native_engine/native_engine.h"

#include "native_engine/native_engine.h"

#include "data_protect.h"

namespace OHOS {
namespace JsEnv {
struct WorkerInfo {
    panda::panda_file::StringPacProtect codePath;
    bool isDebugVersion = false;
    bool isBundle = true;
    std::string packagePathStr;
    std::vector<std::string> assetBasePathStr;
    panda::panda_file::StringPacProtect hapPath;
    panda::panda_file::BoolPacProtect isStageModel = panda::panda_file::BoolPacProtect(true);
    std::string moduleName;
    panda::panda_file::DataProtect apiTargetVersion = panda::panda_file::DataProtect();
};

class JsEnvironmentImpl {
public:
    JsEnvironmentImpl() {}
    virtual ~JsEnvironmentImpl() {}

    virtual void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime) = 0;

    virtual void PostSyncTask(const std::function<void()>& task, const std::string& name) = 0;

    virtual void RemoveTask(const std::string& name) = 0;

    virtual void InitTimerModule(NativeEngine* engine) = 0;

    virtual void InitConsoleModule(NativeEngine* engine) = 0;

    virtual bool InitLoop(NativeEngine* engine, bool isStage = true) = 0;

    virtual void DeInitLoop(NativeEngine* engine) = 0;

    virtual void InitWorkerModule(NativeEngine* engine, std::shared_ptr<WorkerInfo> workerInfo) = 0;

    virtual void InitSyscapModule() = 0;
};
} // namespace JsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_JS_ENVIRONMENT_JS_ENVIRONMENT_IMPL_H