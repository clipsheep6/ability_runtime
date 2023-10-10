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

#include "js_child_process.h"

#include "child_process.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> JsChildProcess::Create(const std::unique_ptr<Runtime>& runtime)
{
    return std::make_shared<JsChildProcess>(static_cast<JsRuntime &>(*runtime));
}

JsChildProcess::JsChildProcess(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime) {}
JsChildProcess::~JsChildProcess()
{
    HILOG_DEBUG("JsChildProcess destructor.");
}

void JsChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    HILOG_INFO("JsChildProcess Init called");
    ChildProcess::Init(info);

    std::string srcPath(info->moduleName);
    if (info->srcEntry.empty()) {
        HILOG_ERROR("ChildProcessStartInfo srcEntry is empty");
        return;
    }

    srcPath.append("/").append(info->srcEntry);
    srcPath.erase(srcPath.rfind("."));
    srcPath.append(".abc");
    std::string moduleName(info->moduleName);
    moduleName.append("::").append(info->name);

    HandleScope handleScope(jsRuntime_);
    jsChildProcessObj_ = jsRuntime_.LoadModule(moduleName, srcPath, info->hapPath, info->isEsModule);
    if (jsChildProcessObj_ == nullptr) {
        HILOG_ERROR("Failed to get ChildProcess object");
        return;
    }
}

void JsChildProcess::OnStart()
{
    HILOG_INFO("JsChildProcess OnStart called");
    ChildProcess::OnStart();
    CallObjectMethod("onStart");
}

napi_value JsChildProcess::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    HILOG_DEBUG("JsChildProcess::CallObjectMethod(%{public}s)", name);
    if (jsChildProcessObj_ == nullptr) {
        HILOG_ERROR("ChildProcess.js not found");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsChildProcessObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get ChildProcess object");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from ChildProcess object", name);
        return nullptr;
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    return nullptr;
}
}  // namespace AbilityRuntime
}  // namespace OHOS