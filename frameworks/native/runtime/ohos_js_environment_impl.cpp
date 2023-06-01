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

#include "ohos_js_environment_impl.h"
#include "commonlibrary/ets_utils/js_sys_module/console/console.h"
#include "commonlibrary/ets_utils/js_sys_module/timer/timer.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_utils.h"

#include "js_worker.h"
namespace OHOS {
namespace AbilityRuntime {
OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

OHOSJsEnvironmentImpl::~OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::RemoveTask(const std::string& name)
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::InitTimerModule(NativeEngine* engine)
{
    HILOG_DEBUG("Init timer.");
    CHECK_POINTER(engine);

    HandleScope handleScope(*engine);
    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
    CHECK_POINTER(globalObj);

    JsSysModule::Timer::RegisterTime(reinterpret_cast<napi_env>(engine));
}

void OHOSJsEnvironmentImpl::InitConsoleModule(NativeEngine *engine)
{
    HILOG_DEBUG("called");
    JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(engine));
}

void OHOSJsEnvironmentImpl::InitWorkerModule(NativeEngine& engine, std::shared_ptr<JsEnv::WorkerInfo> workerInfo)
{
    HILOG_DEBUG("called");
    engine.SetInitWorkerFunc(InitWorkerFunc);
    engine.SetOffWorkerFunc(OffWorkerFunc);
    engine.SetGetAssetFunc(AssetHelper(workerInfo));

    engine.SetGetContainerScopeIdFunc(GetContainerId);
    engine.SetInitContainerScopeFunc(UpdateContainerScope);
    engine.SetFinishContainerScopeFunc(RestoreContainerScope);
}

void OHOSJsEnvironmentImpl::InitSyscapModule()
{
    HILOG_DEBUG("called");
}
} // namespace AbilityRuntime
} // namespace OHOS
