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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H

#include "native_engine/native_engine.h"
#include "js_free_install_observer.h"
#include "js_ui_extension.h"
#include "session_info.h"
#include "window.h"


namespace OHOS {
namespace AbilityRuntime {
using RuntimeTask = std::function<void(int, const AAFwk::Want&, bool)>;

class UISessionAbilityResultListener : public AbilityResultListener
{
public:
    UISessionAbilityResultListener() = default;
    virtual ~UISessionAbilityResultListener() = default;
    virtual void OnAbilityResult(int requestCode, int resultCode, const Want &resultData);
    void OnAbilityResultInner(int requestCode, int resultCode, const Want &resultData);
    void SaveResultCallbacks(int requestCode, RuntimeTask&& task);
private:
    std::map<int, RuntimeTask> resultCallbacks_;
};

class JsUIExtensionContentSession {
private:
    class CallbackWrapper;
public:
    JsUIExtensionContentSession(NativeEngine& engine, sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context>& context,
        std::shared_ptr<AbilityResultListeners>& abilityResultListeners);
    JsUIExtensionContentSession(NativeEngine& engine, sptr<AAFwk::SessionInfo> sessionInfo,
        sptr<Rosen::Window> uiWindow);
    virtual ~JsUIExtensionContentSession() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateJsUIExtensionContentSession(NativeEngine& engine,
        sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
        std::weak_ptr<AbilityRuntime::Context> context,
        std::shared_ptr<AbilityResultListeners>& abilityResultListeners);
    static NativeValue* CreateJsUIExtensionContentSession(NativeEngine& engine,
        sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow);

    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SendData(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetReceiveDataCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* LoadContent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowBackgroundColor(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowPrivacyMode(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSendData(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetReceiveDataCallback(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetWindowBackgroundColor(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetWindowPrivacyMode(NativeEngine& engine, NativeCallbackInfo& info);

    static void CallReceiveDataCallback(NativeEngine& engine, std::weak_ptr<CallbackWrapper> weakCallback,
        const AAFwk::WantParams& wantParams);
    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want);
    static NativeValue* WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want);
    static NativeValue* WrapWant(NativeEngine& engine, const AAFwk::Want& want);
    static bool UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want);
    void AddFreeInstallObserver(NativeEngine& engine, const AAFwk::Want &want, NativeValue* callback,
        bool isAbilityResult = false);
private:
    NativeEngine& engine_;
    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
    std::weak_ptr<AbilityRuntime::Context> context_;
    std::shared_ptr<CallbackWrapper> receiveDataCallback_;
    bool isRegistered = false;
    std::shared_ptr<UISessionAbilityResultListener> listener_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;
};

class JsUIExtensionContentSession::CallbackWrapper {
public:
    void ResetCallback(std::shared_ptr<NativeReference> callback)
    {
        callback_ = callback;
    }
    std::shared_ptr<NativeReference> GetCallback() const
    {
        return callback_;
    }
private:
    std::shared_ptr<NativeReference> callback_;
};

class JsUIExtensionContentSession::CallbackWrapper {
public:
    void ResetCallback(std::shared_ptr<NativeReference> callback)
    {
        callback_ = callback;
    }
    std::shared_ptr<NativeReference> GetCallback() const
    {
        return callback_;
    }
private:
    std::shared_ptr<NativeReference> callback_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
