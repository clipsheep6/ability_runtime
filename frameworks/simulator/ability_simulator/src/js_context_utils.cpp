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

#include "js_context_utils.h"

#include <iostream>
#include "hilog_wrapper.h"
#include "js_application_context_utils.h"
#include "js_runtime_utils.h"

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

    static NativeValue* GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetTempDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDistributedFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info);

    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);

protected:
    std::weak_ptr<Context> context_;
};

void JsBaseContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsBaseContext::Finalizer is called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

NativeValue* JsBaseContext::CreateBundleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::SwitchArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetTempDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetDistributedFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    return nullptr;
}

NativeValue* JsBaseContext::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("GetApplicationContext start");

    NativeValue* value = JsApplicationContextUtils::CreateJsApplicationContext(engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    return contextObj;
}
} // namespace

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
