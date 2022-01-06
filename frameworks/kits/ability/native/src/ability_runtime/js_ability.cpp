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

#include "ability_runtime/js_ability.h"

#include "ability_runtime/js_ability_context.h"
#include "ability_runtime/js_window_stage.h"
#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";

Ability* JsAbility::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsAbility(static_cast<JsRuntime&>(*runtime));
}

JsAbility::JsAbility(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsAbility::~JsAbility() = default;

void JsAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    Ability::Init(abilityInfo, application, handler, token);

    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    std::string srcPath(abilityInfo->package);
    srcPath.append("/assets/js/");
    if (!abilityInfo->srcPath.empty()) {
        srcPath.append(abilityInfo->srcPath);
    }
    srcPath.append("/").append(abilityInfo->name).append(".abc");

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsAbilityObj_ = jsRuntime_.LoadModule(moduleName, srcPath);

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    auto context = GetAbilityContext();
    NativeValue* contextObj = CreateJsAbilityContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.AbilityContext", &contextObj, 1);
    contextObj = shellContextRef->Get();

    context->Bind(jsRuntime_, shellContextRef.release());
    obj->SetProperty("context", contextObj);
}

void JsAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue* jsWant = reinterpret_cast<NativeValue*>(napiWant);

    obj->SetProperty("launchWant", jsWant);
    obj->SetProperty("lastRequestWant", jsWant);

    NativeValue* argv[] = {
        jsWant,
        CreateJsLaunchParam(nativeEngine, GetLaunchParam()),
    };
    CallObjectMethod("onCreate", argv, ArraySize(argv));
}

void JsAbility::OnStop()
{
    Ability::OnStop();

    CallObjectMethod("onDestroy");
}

void JsAbility::OnSceneCreated()
{
    Ability::OnSceneCreated();
    HILOG_INFO("OnSceneCreated");
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue* argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));
}

void JsAbility::OnSceneRestored()
{
    Ability::OnSceneRestored();
    HILOG_INFO("OnSceneRestored");
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue* argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));
}

void JsAbility::onSceneDestroyed()
{
    Ability::onSceneDestroyed();

    CallObjectMethod("onWindowStageDestroy");
}

void JsAbility::OnForeground(const Want &want)
{
    Ability::OnForeground(want);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue* jsWant = reinterpret_cast<NativeValue*>(napiWant);

    obj->SetProperty("lastRequestWant", jsWant);

    CallObjectMethod("onForeground", &jsWant, 1);
}

void JsAbility::OnBackground()
{
    Ability::OnBackground();

    CallObjectMethod("onBackground");
}

void JsAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Ability::OnAbilityResult(requestCode, resultCode, resultData);
    std::shared_ptr<AbilityRuntime::AbilityContext> context = GetAbilityContext();
    if (context == nullptr) {
        HILOG_WARN("JsAbility not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsAbility::OnRequestPermissionsFromUserResult(int requestCode, const std::vector<std::string> &permissions,
    const std::vector<int> &grantResults)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void JsAbility::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsAbility::CallObjectMethod(%{public}s", name);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}

std::unique_ptr<NativeReference> JsAbility::CreateAppWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();
    NativeValue* jsWindowStage = CreateJsWindowStage(engine, GetScene());
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return jsRuntime_.LoadSystemModule("application.WindowStage", &jsWindowStage, 1);
}

void JsAbility::DoOnForeground(const Want& want)
{
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            HILOG_ERROR("Ability::OnForeground error. abilityContext_ or sceneListener_ is nullptr!");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        if (scene_ == nullptr) {
            HILOG_ERROR("%{public}s error. failed to create WindowScene instance!", __func__);
            return;
        }
        auto option = GetWindowOption(want);
        Rosen::WMError ret = scene_->Init(Rosen::WindowScene::DEFAULT_DISPLAY_ID, abilityContext_, sceneListener_,
            option);
        if (ret != Rosen::WMError::WM_OK) {
            HILOG_ERROR("%{public}s error. failed to init window scene!", __func__);
            return;
        }

        // multi-instance ability continuation
        HILOG_INFO("lauch reason = %{public}d, contentStorage = %{public}p", launchParam_.launchReason,
            abilityContext_->GetContentStorage());
        if (IsRestoredInContinuation()) {
            std::string pageStack = AAFwk::String::Unbox(
                AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME)));
            HandleScope handleScope(jsRuntime_);
            auto& engine = jsRuntime_.GetNativeEngine();
            scene_->GetMainWindow()->SetUIContent(abilityContext_, pageStack, &engine,
                static_cast<NativeValue*>(abilityContext_->GetContentStorage()), true);
            OnSceneRestored();
            NotityContinuationResult(want, true);
        } else {
            OnSceneCreated();
        }
    }
    HILOG_INFO("%{public}s begin scene_->GoForeground.", __func__);
    scene_->GoForeground();
    HILOG_INFO("%{public}s end scene_->GoForeground.", __func__);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
