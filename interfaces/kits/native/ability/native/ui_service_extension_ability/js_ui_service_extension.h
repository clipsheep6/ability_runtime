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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_SERVICE_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_SERVICE_EXTENSION_H

#include "ui_service_extension.h"
#include "configuration.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "ability_info.h"
#include "ui_service_extension_context.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "system_ability_status_change_stub.h"
#endif
#include "ui_service_extension.h"
#include "ui_service_stub.h"
#include "freeze_util.h"
#include "time_util.h"
#include "ability_delegator_infos.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class UIServiceExtension;
class Runtime;
class UIServiceExtensionContext;
class JsUIServiceExtension;

class UIServiceExtStub : public AAFwk::UIServiceStub {
public:
    UIServiceExtStub(JsUIServiceExtension* ext) { extension_ = ext; }
    void SetExtension(JsUIServiceExtension* ext) { extension_ = ext; }
    virtual void SendData(OHOS::AAFwk::WantParams &data) override;

protected:
    JsUIServiceExtension* extension_;
};

/**
 * @brief Basic service components.
 */
class JsUIServiceExtension : public UIServiceExtension {
public:
    explicit JsUIServiceExtension(JsRuntime& jsRuntime);
    virtual ~JsUIServiceExtension() override;

    /**
     * @brief Create JsServiceExtension.
     *
     * @param runtime The runtime.
     * @return The JsServiceExtension instance.
     */
    static JsUIServiceExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;


		/**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;
   


	 /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;



	 /**
     * @brief Called when this Service extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
     * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Service extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback) override;


		 /**
     * @brief Called when all abilities connected to this Service extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this Service extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    virtual void OnDisconnect(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo,
        bool &isAsyncCallback) override;

	
   
    /**
     * @brief Called back when Service is started.
     * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
     * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
     * logic.
     *
     * @param want Indicates the want of Service to start.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;
    
    /**
     * @brief Called back when Service is started by intent driver.
     *
     * @param want Indicates the want of intent to handle.
     */
    bool HandleInsightIntent(const AAFwk::Want &want) override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    /**
     * @brief Called when configuration changed, including system configuration and window configuration.
     *
     */
    void ConfigurationUpdated();

    /**
     * @brief Called when extension need dump info.
     *
     * @param params The params from service.
     * @param info The dump info to show.
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    void SendData(OHOS::AAFwk::WantParams &data);
#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called before instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneWillCreated(std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig);

    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneDidCreated();

    /**
     * @brief Called after ability restored.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneRestored();

    void UpdateJsWindowStage(napi_value windowStage);
private:
    bool IsRestorePageStack(const Want &want);
    void RestorePageStack(const Want &want);
    void GetPageStackFromWant(const Want &want, std::string &pageStack);
    void AbilityContinuationOrRecover(const Want &want);

    std::shared_ptr<NativeReference> jsWindowStageObj_;
    int32_t windowMode_ = 0;
#endif
private:
    napi_value CallObjectMethod(const char* name, napi_value const *argv = nullptr, size_t argc = 0);

    void BindContext(napi_env env, napi_value obj);

    void GetSrcPath(std::string &srcPath);

    napi_value CallOnConnect(const AAFwk::Want &want);

    napi_value CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);

    bool CheckPromise(napi_value result);

    bool CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo);

    void ListenWMS();
    
    bool GetInsightIntentExecutorInfo(const Want &want,
        const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
        InsightIntentExecutorInfo &executorInfo);

    bool OnInsightIntentExecuteDone(uint64_t intentId, const AppExecFwk::InsightIntentExecuteResult &result) override;
    std::unique_ptr<NativeReference> CreateAppWindowStage();
    std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CreateADelegatorAbilityProperty();
    void AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName);
    void AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName);
    void DoOnForeground(const Want &want);
    void DoOnForegroundForSceneIsNull(const Want &want,
        const std::shared_ptr< Rosen::WindowStageConfig> windowStageConfig);

    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsObj_;
    std::shared_ptr<AbilityContext> aContext_ = nullptr;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
    sptr<IRemoteObject> callbackProxy_ = nullptr;
    std::unique_ptr<UIServiceExtStub> extensionStub_ = nullptr;

#ifdef SUPPORT_GRAPHICS
protected:
    class JsUIServiceExtensionDisplayListener : public Rosen::DisplayManager::IDisplayListener {
    public:
        explicit JsUIServiceExtensionDisplayListener(const std::weak_ptr<JsUIServiceExtension>& jsUIServiceExtension)
        {
            jsUIServiceExtension_ = jsUIServiceExtension;
        }

        void OnCreate(Rosen::DisplayId displayId) override
        {
            auto sptr = jsUIServiceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnCreate(displayId);
            }
        }

        void OnDestroy(Rosen::DisplayId displayId) override
        {
            auto sptr = jsUIServiceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnDestroy(displayId);
            }
        }

        void OnChange(Rosen::DisplayId displayId) override
        {
            auto sptr = jsUIServiceExtension_.lock();
            if (sptr != nullptr) {
                sptr->OnChange(displayId);
            }
        }

    private:
        std::weak_ptr<JsUIServiceExtension> jsUIServiceExtension_;
    };

    void OnCreate(Rosen::DisplayId displayId);
    void OnDestroy(Rosen::DisplayId displayId);
    void OnChange(Rosen::DisplayId displayId);

private:
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener(sptr<JsUIServiceExtensionDisplayListener> displayListener)
            : tmpDisplayListener_(displayListener) {};
        virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
        virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override {}

    private:
        sptr<JsUIServiceExtensionDisplayListener> tmpDisplayListener_ = nullptr;
    };

    sptr<JsUIServiceExtensionDisplayListener> displayListener_ = nullptr;
#endif
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_SERVICE_EXTENSION_H
