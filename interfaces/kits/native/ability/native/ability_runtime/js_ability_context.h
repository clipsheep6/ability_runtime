/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>

#include "ability_connect_callback.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/ability/ability_runtime/ability_context.h"
#include "js_free_install_observer.h"
#include "js_runtime.h"
#include "event_handler.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
class JsEmbeddableUIAbilityContext;
class JsAbilityContext final {
public:
    explicit JsAbilityContext(const std::shared_ptr<AbilityContext>& context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartRecentAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResultWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value RestoreWindowStage(napi_env env, napi_callback_info info);
    static napi_value RequestDialogService(napi_env env, napi_callback_info info);
    static napi_value IsTerminating(napi_env env, napi_callback_info info);
    static napi_value ReportDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value SetMissionContinueState(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByType(napi_env env, napi_callback_info info);
    static napi_value RequestModalUIExtension(napi_env env, napi_callback_info info);
    static napi_value ShowAbility(napi_env env, napi_callback_info info);
    static napi_value HideAbility(napi_env env, napi_callback_info info);
    static napi_value OpenAtomicService(napi_env env, napi_callback_info info);
    static napi_value MoveAbilityToBackground(napi_env env, napi_callback_info info);

    static void ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

    std::shared_ptr<AbilityContext> GetAbilityContext()
    {
        return context_.lock();
    }

#ifdef SUPPORT_GRAPHICS
public:
    static napi_value SetMissionLabel(napi_env env, napi_callback_info info);
    static napi_value SetMissionIcon(napi_env env, napi_callback_info info);

private:
    napi_value OnSetMissionLabel(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionIcon(napi_env env, NapiCallbackInfo& info);
#endif

private:
    static void ClearFailedCallConnection(
        const std::weak_ptr<AbilityContext>& abilityContext, const std::shared_ptr<CallerCallBack> &callback);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false);
    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartRecentAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    napi_value OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestDialogService(napi_env env, NapiCallbackInfo& info);
    napi_value OnIsTerminating(napi_env env, NapiCallbackInfo& info);
    napi_value OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbilityByType(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestModalUIExtension(napi_env env, NapiCallbackInfo& info);
    napi_value OnShowAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnHideAbility(napi_env env, NapiCallbackInfo& info);
    napi_value ChangeAbilityVisibility(napi_env env, NapiCallbackInfo& info, bool isShow);
    napi_value OnOpenAtomicService(napi_env env, NapiCallbackInfo& info);
    napi_value OpenAtomicServiceInner(napi_env env, NapiCallbackInfo& info, AAFwk::Want &want,
        AAFwk::StartOptions &options);
    napi_value OnMoveAbilityToBackground(napi_env env, NapiCallbackInfo& info);

    static bool UnWrapWant(napi_env env, napi_value argv, AAFwk::Want& want);
    static napi_value WrapWant(napi_env env, const AAFwk::Want& want);
    static bool UnWrapAbilityResult(napi_env env, napi_value argv, int& resultCode, AAFwk::Want& want);
    static napi_value WrapAbilityResult(napi_env env, const int& resultCode, const AAFwk::Want& want);
    void InheritWindowMode(AAFwk::Want &want);
    static napi_value WrapRequestDialogResult(napi_env env, int32_t resultCode, const AAFwk::Want& want);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
        bool isAbilityResult = false);
    bool CheckStartAbilityByCallParams(napi_env env, NapiCallbackInfo& info, AAFwk::Want &want,
        int32_t &userId, napi_value &lastParam);

    std::weak_ptr<AbilityContext> context_;
    int curRequestCode_ = 0;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
    friend class JsEmbeddableUIAbilityContext;
};

napi_value CreateJsAbilityContext(napi_env env, std::shared_ptr<AbilityContext> context);

struct ConnectCallback {
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

class JSAbilityConnection : public AbilityConnectCallback {
public:
    explicit JSAbilityConnection(napi_env env);
    ~JSAbilityConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
private:
    napi_value ConvertElement(const AppExecFwk::ElementName &element);
    napi_env env_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct KeyCompare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_CONTEXT_H
