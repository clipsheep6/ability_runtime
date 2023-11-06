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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H

#include "auto_fill_extension.h"
#include "configuration.h"
#include "session_info.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillExtension;
class JsRuntime;
class JsUIExtensionBase;
/**
 * @brief Basic auto fill extension components.
 */
class JsAutoFillExtension : public AutoFillExtension, public std::enable_shared_from_this<JsAutoFillExtension> {
public:
    explicit JsAutoFillExtension(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsAutoFillExtension() override;

    /**
     * @brief Create JsAutoFillExtension.
     *
     * @param runtime The runtime.
     * @return The JsAutoFillExtension instance.
     */
    static JsAutoFillExtension *Create(const std::unique_ptr<Runtime> &runtime);

    /**
     * @brief Init the auto fill extension.
     *
     * @param record the auto fill extension record.
     * @param application the application info.
     * @param handler the auto fill extension handler.
     * @param token the remote token.
     */
    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this action extension is started. You must override this function if you want to perform some
     *        initialization operations during action extension startup.
     *
     * This function can be called only once in the entire lifecycle of an action extension.
     *
     * @param Want Indicates the {@link Want} structure containing startup information about the action extension.
     */
    void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called back when action extension is started.
     *
     * This method can be called only by action extension. You can use the StartAbility(Want) method to start
     * action extension. Then the system calls back the current method to use the transferred want parameter to
     * execute its own logic.
     *
     * @param want Indicates the want of action extension to start.
     * @param restart Indicates the startup mode. The value true indicates that action extension is restarted after
     * being destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the action extension has been started. The startId is incremented
     * by 1 every time the action extension is started. For example, if the action extension has been started for six
     * times, the value of startId is 6.
     */
    void OnCommand(const AAFwk::Want &want, bool restart, int32_t startId) override;

    void OnCommandWindow(
        const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    /**
     * @brief Called when this action extension enters the <b>STATE_STOP</b> state.
     *
     * The action extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop() override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration &configuration) override;

    /**
     * @brief Called when this extension enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    void OnForeground(const Want &want) override;

    /**
     * @brief Called when this extension enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The extension in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground() override;

    /**
     * @brief onNewPasswordRequest, check session and callback new password needed.
     *
     * @param session the session data passed in from UIContext.
     * @param viewData the view data is used to store the new genepassword .
     * @param callback used to store status code and message.
     */
   // void onNewPasswordRequest(napi_value, ViewData* viewData, std::shared_ptr<IFillRequestCallback> callback);
    /**
     * @brief Called when action extension need dump info.
     *
     * @param params The params from action extension.
     * @param info The dump info to show.
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

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
    void OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData) override;
private:
    std::shared_ptr<JsUIExtensionBase> jsUIExtensionBase_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_H
