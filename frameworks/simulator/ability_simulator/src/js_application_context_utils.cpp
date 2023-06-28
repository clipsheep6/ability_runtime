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
#include "js_runtime_utils.h"
#include "js_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
const char* MD_NAME = "JsApplicationContextUtils";
}  // namespace

NativeValue *JsApplicationContextUtils::CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::SwitchArea(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSwitchArea(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info)
{
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
    return nullptr;
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    int area = context->GetArea();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetCacheDir();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetFilesDir();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetDistributedFilesDir();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetDatabaseDir();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetPreferencesDir();
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
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::KillProcessBySelf(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::GetRunningProcessInformation(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

void JsApplicationContextUtils::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

NativeValue *JsApplicationContextUtils::RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::RegisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterEnvironmentCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::On(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsApplicationContextUtils::Off(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue* JsApplicationContextUtils::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    NativeValue* value = CreateJsApplicationContext(engine, context_.lock());
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    return contextObj;
}

NativeValue* JsApplicationContextUtils::CreateJsApplicationContext(
    NativeEngine &engine, const std::shared_ptr<Context>& context)
{
    HILOG_DEBUG("CreateJsApplicationContext start");

    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        return objValue;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(context);
    SetNamedNativePointer(engine, *object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    BindNativeApplicationContext(engine, object);

    return objValue;
}

void JsApplicationContextUtils::BindNativeApplicationContext(NativeEngine &engine, NativeObject* object)
{
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
    BindNativeFunction(engine, *object, "killAllProcesses", MD_NAME, JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(engine, *object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(engine, *object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
