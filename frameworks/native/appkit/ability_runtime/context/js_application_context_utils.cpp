/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_hap_module_info_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
const char* MD_NAME = "JsApplicationContextUtils";

class JsApplicationContextUtils {
public:
    explicit JsApplicationContextUtils(std::weak_ptr<ApplicationContext> &&applicationContext)
        : applicationContext_(std::move(applicationContext))
    {
    }
    virtual ~JsApplicationContextUtils() = default;
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue* RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* UnregisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* RegisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* UnregisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* On(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* Off(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* SwitchArea(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetArea(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info);

    NativeValue* OnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);

    NativeValue* OnRegisterEnvironmentCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregisterEnvironmentCallback(NativeEngine &engine, NativeCallbackInfo &info);

    NativeValue* OnOn(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnOff(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnOnAbilityLifecycle(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnOffAbilityLifecycle(NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId);
    NativeValue* OnOnEnvironment(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnOffEnvironment(NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId);

    NativeValue* OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnKillProcessBySelf(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetProcessRunningInformation(NativeEngine &engine, NativeCallbackInfo &info);

    static NativeValue* GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetTempDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetApplicationContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* KillProcessBySelf(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetProcessRunningInformation(NativeEngine *engine, NativeCallbackInfo *info);

    void KeepApplicationContext(std::shared_ptr<ApplicationContext> applicationContext)
    {
        keepApplicationContext_ = applicationContext;
    }

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    NativeValue* OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetArea(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);
    std::shared_ptr<ApplicationContext> keepApplicationContext_;
    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
    std::shared_ptr<JsEnvironmentCallback> envCallback_;
};

NativeValue *JsApplicationContextUtils::CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateBundleContext(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string bundleName;
    if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
        HILOG_ERROR("Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        HILOG_ERROR("bundleContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, bundleContext, nullptr, nullptr, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue *JsApplicationContextUtils::SwitchArea(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::SwitchArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSwitchArea(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }

    int mode = 0;
    if (!ConvertFromJsValue(engine, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return engine.CreateUndefined();
    }

    applicationContext->SwitchArea(mode);

    NativeValue *thisVar = info.thisVar;
    NativeObject *object = ConvertNativeValueTo<NativeObject>(thisVar);
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


NativeValue* JsApplicationContextUtils::CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateModuleContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    if (!ConvertFromJsValue(engine, info.argv[1], moduleName)) {
        HILOG_INFO("Parse inner module name.");
        if (!ConvertFromJsValue(engine, info.argv[0], moduleName)) {
            HILOG_ERROR("Parse moduleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        moduleContext = applicationContext->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("Parse bundleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        HILOG_INFO("Parse outer module name.");
        moduleContext = applicationContext->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, moduleContext, nullptr, nullptr, true);
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
            HILOG_INFO("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsApplicationContextUtils::GetArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetArea(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    int area = applicationContext->GetArea();
    return engine.CreateNumber(area);
}

NativeValue *JsApplicationContextUtils::GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetCacheDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetTempDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetTempDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDistributedFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDistributedFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDatabaseDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetPreferencesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetBundleCodeDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::KillProcessBySelf(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnKillProcessBySelf(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnKillProcessBySelf(NativeEngine &engine, NativeCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("kill self process");
    AsyncTask::CompleteCallback complete =
        [applicationContext = applicationContext_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = applicationContext.lock();
            if (!context) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                    "applicationContext if already released."));
                return;
            }
            context->KillProcessBySelf();
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue* lastParam = (info.argc = ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSAppManager::OnkillProcessBySelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::GetProcessRunningInformation(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetProcessRunningInformation(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetProcessRunningInformation(NativeEngine &engine, NativeCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("Get Process Info");
    auto complete = [applicationContext = applicationContext_](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        AppExecFwk::RunningProcessInfo processInfo;
        auto ret = context->GetProcessRunningInformation(processInfo);
        if (ret == 0) {
            NativeValue* objValue = engine.CreateObject();
            NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
            object->SetProperty("processName", CreateJsValue(engine, processInfo.processName_));
            object->SetProperty("pid", CreateJsValue(engine, processInfo.pid_));
            object->SetProperty("uid", CreateJsValue(engine, processInfo.uid_));
            object->SetProperty("bundleNames", CreateNativeArray(engine, processInfo.bundleNames));
            object->SetProperty("state", CreateJsValue(engine, processInfo.state_));
            object->SetProperty("isContinuousTask", CreateJsValue(engine, processInfo.isContinuousTask));
            object->SetProperty("isKeepAlive", CreateJsValue(engine, processInfo.isKeepAlive));
            object->SetProperty("isFocused", CreateJsValue(engine, processInfo.isFocused));
            task.ResolveWithNoError(engine, objValue);
        } else {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Get process infos failed."));
        }
    };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSAppManager::OnGetProcessRunningInformation",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

void JsApplicationContextUtils::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JsApplicationContextUtils::Finalizer is called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

NativeValue *JsApplicationContextUtils::RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnregisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnRegisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return engine.CreateUndefined();
    }

    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return engine.CreateNumber(callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    int32_t callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    keepApplicationContext_->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnUnregisterAbilityLifecycleCallback is called");
    int32_t errCode = 0;
    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("OnUnregisterAbilityLifecycleCallback, Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    AsyncTask::CompleteCallback complete =
        [&applicationContext = keepApplicationContext_, callbackWeak, callbackId, errCode](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            auto callback = callbackWeak.lock();
            if (applicationContext == nullptr || callback == nullptr) {
                HILOG_ERROR("applicationContext or callback nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "applicationContext or callback nullptr"));
                return;
            }

            HILOG_DEBUG("OnUnregisterAbilityLifecycleCallback begin");
            if (!callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::RegisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterEnvironmentCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterEnvironmentCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnregisterEnvironmentCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnRegisterEnvironmentCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnRegisterEnvironmentCallback is called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return engine.CreateUndefined();
    }

    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return engine.CreateNumber(envCallback_->Register(info.argv[0]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(&engine);
    int32_t callbackId = envCallback_->Register(info.argv[INDEX_ZERO]);
    keepApplicationContext_->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("OnRegisterEnvironmentCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterEnvironmentCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnUnregisterEnvironmentCallback is called");
    int32_t errCode = 0;
    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("OnUnregisterEnvironmentCallback, Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    AsyncTask::CompleteCallback complete =
        [&applicationContext = keepApplicationContext_, envCallbackWeak, callbackId, errCode](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            auto env_callback = envCallbackWeak.lock();
            if (applicationContext == nullptr || env_callback == nullptr) {
                HILOG_ERROR("applicationContext or env_callback nullptr");
                task.Reject(engine,
                    CreateJsError(engine, ERROR_CODE_ONE, "applicationContext or env_callback nullptr"));
                return;
            }

            HILOG_DEBUG("OnUnregisterEnvironmentCallback begin");
            if (!env_callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterEnvironmentCallback", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::On(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOn(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::Off(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOff(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnOn(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnOn is called");

    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(engine, info);
    }
    if (type == "environment") {
        return OnOnEnvironment(engine, info);
    }
    HILOG_ERROR("on function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOff(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_INFO("OnOff is called");

    if (keepApplicationContext_ == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    int32_t callbackId = -1;
    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    } else {
        napi_get_value_int32(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[1]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(engine, info, callbackId);
    }
    if (type == "environment") {
        return OnOffEnvironment(engine, info, callbackId);
    }
    HILOG_ERROR("off function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOnAbilityLifecycle(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnOnAbilityLifecycle is called");
    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return engine.CreateNumber(callback_->Register(info.argv[1]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    int32_t callbackId = callback_->Register(info.argv[1]);
    keepApplicationContext_->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnOnAbilityLifecycle is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnOffAbilityLifecycle(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_INFO("OnOffAbilityLifecycle is called");
    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    AsyncTask::CompleteCallback complete =
        [&applicationContext = keepApplicationContext_, callbackWeak, callbackId](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            auto callback = callbackWeak.lock();
            if (applicationContext == nullptr || callback == nullptr) {
                HILOG_ERROR("applicationContext or callback nullptr");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "applicationContext or callback nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffAbilityLifecycle begin");
            if (!callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::OnOnEnvironment(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnOnEnvironment is called");
    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return engine.CreateNumber(envCallback_->Register(info.argv[1]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(&engine);
    int32_t callbackId = envCallback_->Register(info.argv[1]);
    keepApplicationContext_->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("OnOnEnvironment is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnOffEnvironment(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_DEBUG("OnOffEnvironment is called");
    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    AsyncTask::CompleteCallback complete =
        [&applicationContext = keepApplicationContext_, envCallbackWeak, callbackId](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            auto env_callback = envCallbackWeak.lock();
            if (applicationContext == nullptr || env_callback == nullptr) {
                HILOG_ERROR("applicationContext or env_callback nullptr");
                task.Reject(engine,
                    CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                        "applicationContext or env_callback nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffEnvironment begin");
            if (!env_callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnOffEnvironment", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsApplicationContextUtils::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("GetApplicationContext start");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsApplicationContext(engine, applicationContext, nullptr, nullptr, true);
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
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr);
    return contextObj;
}
}  // namespace

NativeValue *CreateJsApplicationContext(NativeEngine &engine, std::shared_ptr<ApplicationContext> applicationContext,
    DetachCallback detach, AttachCallback attach, bool keepApplicationContext)
{
    HILOG_DEBUG("CreateJsApplicationContext start");
    NativeValue* objValue;
    if (detach == nullptr || attach == nullptr) {
        objValue = engine.CreateObject();
    } else {
        objValue = engine.CreateNBObject(detach, attach);
    }
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        return objValue;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    if (keepApplicationContext) {
        jsApplicationContextUtils->KeepApplicationContext(applicationContext);
    }
    SetNamedNativePointer(engine, *object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
    }

    BindNativeProperty(*object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(engine, *object, "registerAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "unregisterAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "registerEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::RegisterEnvironmentCallback);
    BindNativeFunction(engine, *object, "unregisterEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterEnvironmentCallback);
    BindNativeFunction(engine, *object, "createBundleContext", MD_NAME, JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(engine, *object, "switchArea", MD_NAME, JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(engine, *object, "getArea", MD_NAME, JsApplicationContextUtils::GetArea);
    BindNativeFunction(engine, *object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(engine, *object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(engine, *object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(engine, *object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(engine, *object, "killProcessesBySelf", MD_NAME,
        JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(engine, *object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetProcessRunningInformation);

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
