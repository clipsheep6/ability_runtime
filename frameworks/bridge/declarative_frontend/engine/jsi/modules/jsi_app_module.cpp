/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "frameworks/bridge/declarative_frontend/engine/jsi/modules/jsi_app_module.h"

#include "base/log/log.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"
#include "frameworks/bridge/js_frontend/engine/common/js_constants.h"

namespace OHOS::Ace::Framework {

shared_ptr<JsValue> AppGetInfo(const shared_ptr<JsRuntime>& runtime, const shared_ptr<JsValue>& thisObj,
    const std::vector<shared_ptr<JsValue>>& argv, int32_t argc)
{
    auto instance = static_cast<JsiDeclarativeEngineInstance*>(runtime->GetEmbedderData());
    if (instance == nullptr) {
        LOGE("get jsi engine instance failed");
        return runtime->NewNull();
    }
    auto delegate = instance->GetDelegate();
    if (!delegate) {
        LOGE("get frontend delegate failed");
        return runtime->NewNull();
    }

    shared_ptr<JsValue> appInfo = runtime->NewObject();
    auto appId = delegate->GetAppID();
    auto appName = delegate->GetAppName();
    auto versionName = delegate->GetVersionName();
    auto versionCode = delegate->GetVersionCode();
    appInfo->SetProperty(runtime, "appID", runtime->NewString(appId));
    appInfo->SetProperty(runtime, "appName", runtime->NewString(appName));
    appInfo->SetProperty(runtime, "versionName", runtime->NewString(versionName));
    appInfo->SetProperty(runtime, "versionCode", runtime->NewNumber(versionCode));
    return appInfo;
}

shared_ptr<JsValue> AppTerminate(const shared_ptr<JsRuntime>& runtime, const shared_ptr<JsValue>& thisObj,
    const std::vector<shared_ptr<JsValue>>& argv, int32_t argc)
{
    auto instance = static_cast<JsiDeclarativeEngineInstance*>(runtime->GetEmbedderData());
    if (instance == nullptr) {
        LOGE("get jsi engine instance failed");
        return runtime->NewNull();
    }
    auto delegate = instance->GetDelegate();
    if (!delegate) {
        LOGE("get frontend delegate failed");
        return runtime->NewNull();
    }
    auto pipelineContext = instance->GetPipelineContext(runtime);
    if (!pipelineContext) {
        LOGE("get frontend pipelineContext failed");
        return runtime->NewNull();
    }
    auto uiTaskExecutor = delegate->GetUiTask();
    WeakPtr<PipelineContext> pipelineContextWeak(pipelineContext);
    uiTaskExecutor.PostTask([pipelineContextWeak]() mutable {
        auto pipelineContext = pipelineContextWeak.Upgrade();
        if (pipelineContext) {
            pipelineContext->Finish();
        }
    });
    return runtime->NewNull();
}

void InitAppModule(const shared_ptr<JsRuntime>& runtime, shared_ptr<JsValue>& moduleObj)
{
    moduleObj->SetProperty(runtime, APP_GET_INFO, runtime->NewFunction(AppGetInfo));
    moduleObj->SetProperty(runtime, APP_TERMINATE, runtime->NewFunction(AppTerminate));
}

} // namespace OHOS::Ace::Framework