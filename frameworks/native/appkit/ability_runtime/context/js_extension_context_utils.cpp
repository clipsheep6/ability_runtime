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

#include "ability_runtime_error_util.h"
#include "js_extension_context_utils.h"
#include "js_application_context_utils.h"
#include "js_data_struct_converter.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"
#include "application_context_manager.h"
#include "js_context_utils.h"
namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char EXTENSION_BASE_CONTEXT_NAME[] = "__extension_base_context_ptr__";
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ONE = 1;
class JsExtensionBaseContext {
public:
    explicit JsExtensionBaseContext(std::weak_ptr<ExtensionContext>&& extensionContext) : extensionContext_(std::move(extensionContext)) {}
    virtual ~JsExtensionBaseContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value SwitchArea(napi_env env, napi_callback_info info);

    napi_value OnGetCacheDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetTempDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetResourceDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetGroupDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info);

    static napi_value GetCacheDir(napi_env env, napi_callback_info info);
    static napi_value GetTempDir(napi_env env, napi_callback_info info);
    static napi_value GetResourceDir(napi_env env, napi_callback_info info);
    static napi_value GetFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDistributedFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDatabaseDir(napi_env env, napi_callback_info info);
    static napi_value GetPreferencesDir(napi_env env, napi_callback_info info);
    static napi_value GetGroupDir(napi_env env, napi_callback_info info);
    static napi_value GetBundleCodeDir(napi_env env, napi_callback_info info);

protected:
    std::weak_ptr<ExtensionContext> extensionContext_;

private:
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnSwitchArea(napi_env env, NapiCallbackInfo& info);
};

// 参数 hint
void JsExtensionBaseContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("called");
    std::unique_ptr<JsExtensionBaseContext>(static_cast<JsExtensionBaseContext*>(data));
}

napi_value JsExtensionBaseContext::SwitchArea(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnSwitchArea, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnSwitchArea(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return CreateJsUndefined(env);
    }

    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_WARN("extensionContext is already released");
        return CreateJsUndefined(env);
    }

    int mode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Context> context = extensionContext;
    context->SwitchArea(mode);

    napi_value object = info.thisVar;
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Check type failed");
        return CreateJsUndefined(env);
    }
    BindNativeProperty(env, object, "cacheDir", GetCacheDir);
    BindNativeProperty(env, object, "tempDir", GetTempDir);
    BindNativeProperty(env, object, "resourceDir", GetResourceDir);
    BindNativeProperty(env, object, "filesDir", GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", GetBundleCodeDir);
    return CreateJsUndefined(env);
}

napi_value JsExtensionBaseContext::GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetCacheDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetCacheDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string cacheDir = extensionContext->GetCacheDir();
    return CreateJsValue(env, cacheDir);
}

napi_value JsExtensionBaseContext::GetTempDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetTempDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetTempDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string tempDir = extensionContext->GetTempDir();
    return CreateJsValue(env, tempDir);
}

napi_value JsExtensionBaseContext::GetResourceDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetResourceDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetResourceDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string resourceDir = extensionContext->GetResourceDir();
    return CreateJsValue(env, resourceDir);
}

napi_value JsExtensionBaseContext::GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetFilesDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetFilesDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string filesDir = extensionContext->GetFilesDir();
    return CreateJsValue(env, filesDir);
}

napi_value JsExtensionBaseContext::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetDistributedFilesDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string distributedFilesDir = extensionContext->GetDistributedFilesDir();
    return CreateJsValue(env, distributedFilesDir);
}

napi_value JsExtensionBaseContext::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetDatabaseDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetDatabaseDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string databaseDir = extensionContext->GetDatabaseDir();
    return CreateJsValue(env, databaseDir);
}

napi_value JsExtensionBaseContext::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetPreferencesDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetPreferencesDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string preferencesDir = extensionContext->GetPreferencesDir();
    return CreateJsValue(env, preferencesDir);
}

napi_value JsExtensionBaseContext::GetGroupDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetGroupDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetGroupDir(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string groupId;
    if (!ConvertFromJsValue(env, info.argv[0], groupId)) {
        HILOG_ERROR("Parse groupId failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    HILOG_DEBUG("Get Group Dir");
    auto complete = [context = extensionContext_, groupId]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        auto completeContext = context.lock();
        if (!completeContext) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "completeContext if already released."));
            return;
        }
        std::string path = completeContext->GetGroupDir(groupId);
        task.ResolveWithNoError(env, CreateJsValue(env, path));
    };

    napi_value lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsExtensionBaseContext::OnGetGroupDir",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsExtensionBaseContext::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetBundleCodeDir, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo &info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    std::string bundleCodeDir = extensionContext->GetBundleCodeDir();
    return CreateJsValue(env, bundleCodeDir);
}

napi_value JsExtensionBaseContext::GetApplicationContext(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsExtensionBaseContext, OnGetApplicationContext, EXTENSION_BASE_CONTEXT_NAME);
}

napi_value JsExtensionBaseContext::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    auto extensionContext = extensionContext_.lock();
    if (!extensionContext) {
        HILOG_WARN("extensionContext is already released");
        // 是否需要throw
        return CreateJsUndefined(env);
    }

    // 此处入口调用过来的extension是有独立沙箱
    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        HILOG_INFO("extensionContext is already released");
        return CreateJsUndefined(env);
    }
    if (!applicationContext->GetApplicationInfoUpdateFlag()) {
        std::shared_ptr<NativeReference> applicationContextObj =
            ApplicationContextManager::GetApplicationContextManager().GetGlobalObject(env);
        if (applicationContextObj != nullptr) {
            napi_value objValue = applicationContextObj->GetNapiValue();
            return objValue;
        }
    }

    napi_value value = JsApplicationContextUtils::CreateJsApplicationContext(env, extensionContext);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, object, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr, nullptr);
    napi_ref ref = nullptr;
    napi_create_reference(env, object, 1, &ref);
    ApplicationContextManager::GetApplicationContextManager()
        .AddGlobalObject(env, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    applicationContext->SetApplicationInfoUpdateFlag(false);
    return object;
}
} // namespace

napi_value CreateJsExtensionBaseContext(napi_env env, const std::shared_ptr<ExtensionContext> extensionContext)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        return nullptr;
    }

    BindNativeProperty(env, object, "cacheDir", JsExtensionBaseContext::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsExtensionBaseContext::GetTempDir);
    BindNativeProperty(env, object, "resourceDir", JsExtensionBaseContext::GetResourceDir);
    BindNativeProperty(env, object, "filesDir", JsExtensionBaseContext::GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", JsExtensionBaseContext::GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsExtensionBaseContext::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsExtensionBaseContext::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsExtensionBaseContext::GetBundleCodeDir);
    const char *moduleName = "JsExtensionBaseContext";
    BindNativeFunction(env, object, "getApplicationContext", moduleName, JsExtensionBaseContext::GetApplicationContext);
    BindNativeFunction(env, object, "switchArea", moduleName, JsExtensionBaseContext::SwitchArea);
    BindNativeFunction(env, object, "getGroupDir", moduleName, JsExtensionBaseContext::GetGroupDir);
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS
