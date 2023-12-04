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

#include "js_application_context_utils.h"

#include <map>
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_converter.h"
#include "js_runtime_utils.h"
#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
const char *MD_NAME = "JsApplicationContextUtils";
} // namespace

napi_value JsApplicationContextUtils::CreateBundleContext(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::SwitchArea(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSwitchArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSwitchArea(napi_env env, NapiCallbackInfo &info)
{
    napi_value object = info.thisVar;
    if (object == nullptr) {
        HILOG_ERROR("object is null");
        return CreateJsUndefined(env);
    }
    BindNativeProperty(env, object, "cacheDir", GetCacheDir);
    BindNativeProperty(env, object, "tempDir", GetTempDir);
    BindNativeProperty(env, object, "filesDir", GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", GetBundleCodeDir);
    return CreateJsUndefined(env);
}


napi_value JsApplicationContextUtils::CreateModuleContext(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::CreateModuleResourceManager(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::GetTempDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetTempDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetTempDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetArea(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetArea(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    int area = context->GetArea();
    return CreateJsValue(env, area);
}

napi_value JsApplicationContextUtils::GetCacheDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetCacheDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCacheDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetFilesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetFilesDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetDistributedFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetDistributedFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetDatabaseDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDatabaseDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env,
        info, JsApplicationContextUtils, OnGetPreferencesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetPreferencesDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env,
        info, JsApplicationContextUtils, OnGetBundleCodeDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo &info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::KillProcessBySelf(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::GetRunningProcessInformation(napi_env env, napi_callback_info info)
{
    return nullptr;
}

void JsApplicationContextUtils::Finalizer(napi_env env, void *data, void *hint)
{
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

napi_value JsApplicationContextUtils::RegisterAbilityLifecycleCallback(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::RegisterEnvironmentCallback(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::UnregisterEnvironmentCallback(
    napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::On(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::Off(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsApplicationContextUtils::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetApplicationContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetApplicationContext(napi_env env, NapiCallbackInfo &info)
{
    napi_value value = CreateJsApplicationContext(env, context_.lock());
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        return CreateJsUndefined(env);
    }
    return systemModule->GetNapiValue();
}

napi_value JsApplicationContextUtils::CreateJsApplicationContext(
    napi_env env, const std::shared_ptr<Context> &context)
{
    HILOG_DEBUG("called");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        return object;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(context);
    SetNamedNativePointer(env, object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }

    BindNativeApplicationContext(env, object);

    return object;
}

void JsApplicationContextUtils::BindNativeApplicationContext(napi_env env, napi_value object)
{
    BindNativeProperty(env, object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(env, object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(env, object, "registerAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "unregisterAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "registerEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::RegisterEnvironmentCallback);
    BindNativeFunction(env, object, "unregisterEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterEnvironmentCallback);
    BindNativeFunction(env, object, "createBundleContext", MD_NAME, JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(env, object, "switchArea", MD_NAME, JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(env, object, "getArea", MD_NAME, JsApplicationContextUtils::GetArea);
    BindNativeFunction(env, object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(env, object, "createModuleResourceManager", MD_NAME,
        JsApplicationContextUtils::CreateModuleResourceManager);
    BindNativeFunction(env, object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(env, object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(env, object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(env, object, "killAllProcesses", MD_NAME, JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(env, object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
}
} // namespace AbilityRuntime
} // namespace OHOS
