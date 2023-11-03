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
#include "js_utils.h"
#include "js_worker.h"
#include "ohos_loop_handler.h"
#include "systemcapability.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint8_t SYSCAP_MAX_SIZE = 64;

napi_value CanIUse(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        HILOG_ERROR("get syscap failed since env or callback info is nullptr.");
        return nullptr;
    }
    napi_value undefined = CreateJsUndefined(env);

    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != 1) {
        HILOG_ERROR("Get syscap failed with invalid parameter.");
        return undefined;
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_string) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return undefined;
    }

    char syscap[SYSCAP_MAX_SIZE] = { 0 };

    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[0], syscap, sizeof(syscap), &strLen);

    bool ret = HasSystemCapability(syscap);
    return CreateJsValue(env, ret);
}

void InitSyscap(napi_env env, napi_value globalObject)
{
    const char *moduleName = "JsRuntime";
    BindNativeFunction(env, globalObject, "canIUse", moduleName, CanIUse);
}
} // namespace

OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

OHOSJsEnvironmentImpl::OHOSJsEnvironmentImpl(const std::shared_ptr<AppExecFwk::EventRunner>& eventRunner)
{
    HILOG_INFO("called");
    if (eventRunner != nullptr) {
        HILOG_DEBUG("Create event handler.");
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(eventRunner);
    }
}

OHOSJsEnvironmentImpl::~OHOSJsEnvironmentImpl()
{
    HILOG_DEBUG("called");
}

void OHOSJsEnvironmentImpl::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    HILOG_DEBUG("called");
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(task, name, delayTime);
    }
}

void OHOSJsEnvironmentImpl::PostSyncTask(const std::function<void()>& task, const std::string& name)
{
    HILOG_DEBUG("Post sync task");
    if (eventHandler_ != nullptr) {
        eventHandler_->PostSyncTask(task, name);
    }
}

void OHOSJsEnvironmentImpl::RemoveTask(const std::string& name)
{
    HILOG_DEBUG("called");
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(name);
    }
}

void OHOSJsEnvironmentImpl::InitTimerModule(NativeEngine* engine)
{
    HILOG_DEBUG("Init timer.");
    CHECK_POINTER(engine);
    auto ret = JsSysModule::Timer::RegisterTime(reinterpret_cast<napi_env>(engine));
    if (!ret) {
        HILOG_ERROR("Register timer failed");
    }
}

void OHOSJsEnvironmentImpl::InitConsoleModule(NativeEngine* engine)
{
    HILOG_DEBUG("called");
    JsSysModule::Console::InitConsoleModule(reinterpret_cast<napi_env>(engine));
}

bool OHOSJsEnvironmentImpl::InitLoop(NativeEngine* engine)
{
    HILOG_DEBUG("called");
    CHECK_POINTER_AND_RETURN(engine, false);
    auto uvLoop = engine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        HILOG_ERROR("Failed to get backend fd from uv loop");
        return false;
    }
    uv_run(uvLoop, UV_RUN_NOWAIT);

    if (eventHandler_ != nullptr) {
        uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
        eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<OHOSLoopHandler>(uvLoop), "uvLoopTask");
    }

    return true;
}

void OHOSJsEnvironmentImpl::DeInitLoop(NativeEngine* engine)
{
    CHECK_POINTER(engine);
    auto uvLoop = engine->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);
}

void OHOSJsEnvironmentImpl::InitWorkerModule(NativeEngine* engine, std::shared_ptr<JsEnv::WorkerInfo> workerInfo)
{
    HILOG_DEBUG("called");
    CHECK_POINTER(engine);
    engine->SetInitWorkerFunc(InitWorkerFunc);
    engine->SetOffWorkerFunc(OffWorkerFunc);
    engine->SetGetAssetFunc(AssetHelper(workerInfo));

    engine->SetGetContainerScopeIdFunc(GetContainerId);
    engine->SetInitContainerScopeFunc(UpdateContainerScope);
    engine->SetFinishContainerScopeFunc(RestoreContainerScope);
}

void OHOSJsEnvironmentImpl::InitSyscapModule(NativeEngine* engine)
{
    HILOG_DEBUG("called");
    CHECK_POINTER(engine);
    napi_env env = reinterpret_cast<napi_env>(engine);
    CHECK_POINTER(env);

    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    CHECK_POINTER(globalObj);
    InitSyscap(env, globalObj);
}
} // namespace AbilityRuntime
} // namespace OHOS
