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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_BASE_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_BASE_H

#include "ability_handler.h"
#include "ability_local_record.h"
#include "configuration.h"
#include "js_extension_common.h"
#include "native_engine/native_engine.h"
#include "ohos_application.h"
#include "session_info.h"
#include "ui_extension_context.h"
#include "ui_extension_window_command.h"
#include "want.h"
#include "window.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
using Want = OHOS::AAFwk::Want;
class JsRuntime;
/**
 * @brief Js ui extension base.
 */
class JsUIExtensionBase {
public:
    explicit JsUIExtensionBase(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsUIExtensionBase();

    /**
     * @brief Init the ui extension.
     *
     * @param record the ui extension record.
     * @param application the application info.
     * @param handler the ui extension handler.
     * @param token the remote token.
     * @return js extension common object.
     */
    std::shared_ptr<JsExtensionCommon> Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token);

    /**
     * @brief Called when this ui extension is started. You must override this function if you want to perform some
     *        initialization operations during ui extension startup.
     *
     * This function can be called only once in the entire lifecycle of an ui extension.
     *
     * @param Want Indicates the {@link Want} structure containing startup information about the ui extension.
     */
    void OnStart(const AAFwk::Want &want);

    /**
     * @brief Called back when ui extension is started.
     *
     * This method can be called only by ui extension. You can use the StartAbility(Want) method to start
     * ui extension. Then the system calls back the current method to use the transferred want parameter to
     * execute its own logic.
     *
     * @param want Indicates the want of ui extension to start.
     * @param restart Indicates the startup mode. The value true indicates that ui extension is restarted after
     * being destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the ui extension has been started. The startId is incremented
     * by 1 every time the ui extension is started. For example, if the ui extension has been started for six
     * times, the value of startId is 6.
     */
    void OnCommand(const AAFwk::Want &want, bool restart, int32_t startId);

    void OnCommandWindow(
        const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd);

    /**
     * @brief Called when this ui extension enters the <b>STATE_STOP</b> state.
     *
     * The ui extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop();

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration);

    /**
     * @brief Called when this extension enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    void OnForeground(const Want &want);

    /**
     * @brief Called when this extension enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground();

    /**
     * @brief Called when ui extension need dump info.
     *
     * @param params The params from ui extension.
     * @param info The dump info to show.
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int32_t) is called to start an extension ability
     * and the result is returned.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     */
    void OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData);

    /**
     * @brief Set ability info.
     */
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Set ui extension context.
     */
    void SetContext(const std::shared_ptr<UIExtensionContext> &context);

private:
    void BindContext(napi_env env, napi_value obj);
    napi_value CallObjectMethod(const char *name, napi_value const *argv = nullptr, size_t argc = 0);
    void ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);
    void BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);
    void DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);
    bool CallJsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        const sptr<Rosen::Window> &uiWindow, const sptr<IRemoteObject> &sessionToken);

    JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_;
    std::map<sptr<IRemoteObject>, sptr<Rosen::Window>> uiWindowMap_;
    std::set<sptr<IRemoteObject>> foregroundWindows_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<NativeReference>> contentSessions_;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_;
    std::shared_ptr<UIExtensionContext> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_BASE_H
