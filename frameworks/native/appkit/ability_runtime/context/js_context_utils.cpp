/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_context_utils.h"

#include "ability_runtime_error_util.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_application_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_hap_module_info_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "tokenid_kit.h"
#include "application_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";

class JsBaseContext {
public:
    explicit JsBaseContext(std::weak_ptr<Context>&& context) : context_(std::move(context)) {}
    virtual ~JsBaseContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateBundleContext(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SwitchArea(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetArea(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info);

    NativeValue* OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetDistributedFilesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info);

    static NativeValue* GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetTempDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDistributedFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    std::weak_ptr<Context> context_;

private:
    NativeValue* OnCreateBundleContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSwitchArea(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetArea(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info);
    bool CheckCallerIsSystemApp();
};

void JsBaseContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsBaseContext::Finalizer is called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

NativeValue* JsBaseContext::CreateBundleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateBundleContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::SwitchArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::SwitchArea is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnSwitchArea(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnSwitchArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }

    int mode = 0;
    if (!ConvertFromJsValue(engine, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return engine.CreateUndefined();
    }

    context->SwitchArea(mode);

    NativeValue* thisVar = info.thisVar;
    NativeObject* object = ConvertNativeValueTo<NativeObject>(thisVar);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return engine.CreateUndefined();
    }
    BindNativeProperty(*object, "cacheDir", GetCacheDir);
    BindNativeProperty(*object, "tempDir", GetTempDir);
    BindNativeProperty(*object, "filesDir", GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", GetBundleCodeDir);
    return engine.CreateUndefined();
}

NativeValue* JsBaseContext::CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::CreateModuleContext is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateModuleContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::shared_ptr<Context> moduleContext = nullptr;
    std::string moduleName;

    if (!ConvertFromJsValue(engine, info.argv[1], moduleName)) {
        HILOG_INFO("Parse inner module name.");
        if (!ConvertFromJsValue(engine, info.argv[0], moduleName)) {
            HILOG_ERROR("Parse moduleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        moduleContext = context->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("Parse bundleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        if (!CheckCallerIsSystemApp()) {
            HILOG_ERROR("This application is not system-app, can not use system-api");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return engine.CreateUndefined();
        }
        HILOG_DEBUG("Parse outer module name.");
        moduleContext = context->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnCreateModuleContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsBaseContext::GetArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetArea is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetArea(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    int area = context->GetArea();
    return engine.CreateNumber(area);
}

NativeValue* JsBaseContext::GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetCacheDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetTempDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetTempDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetFilesDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetDistributedFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetDistributedFilesDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDistributedFilesDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetDistributedFilesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetDistributedFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetDatabaseDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetPreferencesDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsBaseContext::GetBundleCodeDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::OnCreateBundleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        HILOG_ERROR("This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string bundleName;
    if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
        HILOG_ERROR("Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto bundleContext = context->CreateBundleContext(bundleName);
    if (!bundleContext) {
        HILOG_ERROR("bundleContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnCreateBundleContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnCreateBundleContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsBaseContext::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("GetApplicationContext start");
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        HILOG_WARN("applicationContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = JsApplicationContextUtils::CreateJsApplicationContext(engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachApplicationContext,
        workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr);
    return contextObj;
}

bool JsBaseContext::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}
} // namespace

NativeValue* AttachBaseContext(NativeEngine* engine, void* value, void* hint)
{
    HILOG_DEBUG("AttachBaseContext");
    if (value == nullptr || engine == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<Context>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue* object = CreateJsBaseContext(*engine, ptr, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.Context", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("AttachBaseContext, invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_WARN("AttachBaseContext, invalid nObject.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr base context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr);
    return contextObj;
}

NativeValue* AttachApplicationContext(NativeEngine* engine, void* value, void* hint)
{
    HILOG_DEBUG("AttachApplicationContext");
    if (value == nullptr || engine == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ApplicationContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue* object = JsApplicationContextUtils::CreateJsApplicationContext(*engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.ApplicationContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_WARN("invalid nObject.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachApplicationContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        }, nullptr);
    return contextObj;
}

NativeValue* CreateJsBaseContext(NativeEngine& engine, std::shared_ptr<Context> context, bool keepContext)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_WARN("invalid object.");
        return objValue;
    }
    auto jsContext = std::make_unique<JsBaseContext>(context);
    SetNamedNativePointer(engine, *object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        object->SetProperty("currentHapModuleInfo", CreateJsHapModuleInfo(engine, *hapModuleInfo));
    }
    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
    }

    BindNativeProperty(*object, "cacheDir", JsBaseContext::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsBaseContext::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsBaseContext::GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", JsBaseContext::GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", JsBaseContext::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsBaseContext::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsBaseContext::GetBundleCodeDir);
    BindNativeProperty(*object, "area", JsBaseContext::GetArea);
    const char *moduleName = "JsBaseContext";
    BindNativeFunction(engine, *object, "createBundleContext", moduleName, JsBaseContext::CreateBundleContext);
    BindNativeFunction(engine, *object, "getApplicationContext", moduleName, JsBaseContext::GetApplicationContext);
    BindNativeFunction(engine, *object, "switchArea", moduleName, JsBaseContext::SwitchArea);
    BindNativeFunction(engine, *object, "getArea", moduleName, JsBaseContext::GetArea);
    BindNativeFunction(engine, *object, "createModuleContext", moduleName, JsBaseContext::CreateModuleContext);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
