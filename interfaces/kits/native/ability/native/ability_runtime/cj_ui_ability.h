/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_UI_ABILITY_H
#define OHOS_ABILITY_RUNTIME_CJ_UI_ABILITY_H

#include "ability_delegator_infos.h"
#include "freeze_util.h"
#include "ui_ability.h"
#ifdef SUPPORT_GRAPHICS
#include "window_stage_impl.h"
#endif

namespace OHOS {

namespace AbilityRuntime {
class CJRuntime;
class CJAbilityObject;
struct InsightIntentExecutorInfo;
using AbilityHandler = AppExecFwk::AbilityHandler;
using AbilityInfo = AppExecFwk::AbilityInfo;
using OHOSApplication = AppExecFwk::OHOSApplication;
using Want = AppExecFwk::Want;
using AbilityStartSetting = AppExecFwk::AbilityStartSetting;
using Configuration = AppExecFwk::Configuration;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecutorAsyncCallback = AppExecFwk::InsightIntentExecutorAsyncCallback;

class CJUIAbility : public UIAbility {
public:
    /**
     * @brief Create a JsUIAbility instance through the singleton pattern
     * @param runtime The runtime of the ability
     * @return Returns the JsUIability Instance point
     */
    static UIAbility *Create(const std::unique_ptr<Runtime> &runtime);

    explicit CJUIAbility(CJRuntime &cjRuntime);
    ~CJUIAbility() override;

    /**
     * @brief Init the UIability
     * @param abilityInfo Indicate the Ability information
     * @param ohosAppObject Indicates the main process
     * @param handler the UIability EventHandler object
     * @param token the remote token
     */
    void Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
        const std::shared_ptr<OHOSApplication> application,
        std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    /**
     * @brief OnStart,Start JsUIability
     * @param want Indicates the {@link Want} structure containing startup information about the ability
     * @param sessionInfo Indicates the sessionInfo
     */
    void OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    void OnStop() override;

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     * @param callbackInfo Indicates the lifecycle transaction callback information
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    void OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback) override;

    /**
     * @brief The callback of OnStop.
     */
    void OnStopCallback() override;

    /**
     * @brief Prepare user data of local Ability.
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns 0;
     * otherwise, it returns errcode.
     */
    int32_t OnContinue(WantParams &wantParams) override;

    /**
     * @brief Update configuration
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const Configuration &configuration) override;

    /**
     * @brief Update Contextconfiguration
     */
    void UpdateContextConfiguration() override;

    /**
     * @brief Called when the system configuration is updated.
     * @param level Indicates the memory trim level, which shows the current memory usage status.
     */
    void OnMemoryLevel(int level) override;

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     * @param want Indicates the new Want containing information about the ability.
     */
    void OnNewWant(const Want &want) override;

    /**
     * @brief Prepare user data of local Ability.
     * @param reason the reason why framework invoke this function
     * @param wantParams Indicates the user data to be saved.
     * @return result code defined in abilityConstants
     */
    int32_t OnSaveState(int32_t reason, WantParams &wantParams) override;

    /**
     * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an ability and the
     * result is returned. This method is called only on Page abilities. You can start a new ability to perform some
     * calculations and use setResult (int,ohos.aafwk.content.Want) to return the calculation result. Then the system
     * calls back the current method to use the returned data to execute its own logic.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param resultCode Indicates the result code returned after the ability is started. You can define the result code
     * to identify an error.
     * @param want Indicates the data returned after the ability is started. You can define the data returned. The
     * value can be null.
     */
    void OnAbilityResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief request a remote object of callee from this ability.
     * @return Returns the remote object of callee.
     */
    sptr<IRemoteObject> CallRequest() override;

    /**
     * @brief dump ability info
     * @param params dump params that indicate different dump targets
     * @param info dump ability info
     */
    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    std::shared_ptr<CJAbilityObject> GetCJAbility();

    /**
     * @brief Callback when the ability is shared.You can override this function to implement your own sharing logic.
     * @param wantParams Indicates the user data to be saved.
     * @return the result of OnShare
     */
    int32_t OnShare(WantParams &wantParams) override;

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
public:
    /**
     * @brief Called after instantiating WindowScene.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneCreated() override;

    /**
     * @brief Called after ability stoped.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneDestroyed() ;

    /**
     * @brief Called after ability restored.
     * You can override this function to implement your own processing logic.
     */
    void OnSceneRestored() override;

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    void OnForeground(const Want &want) override;

    /**
     * @brief Call "onForeground" js function barely.
     *
     * @param want Want
     */
    void CallOnForegroundFunc(const Want &want) override;

    /**
     * @brief Request focus for current window, can be override.
     *
     * @param want Want
     */
    void RequestFocus(const Want &want) override;

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    void OnBackground() override;

    /**
     * Called when back press is dispatched.
     * Return true if ability will be moved to background; return false if will be terminated
     */
    bool OnBackPress() override;

    /**
     * @brief Called when ability prepare terminate.
     * @return Return true if ability need to stop terminating; return false if ability need to terminate.
     */
    bool OnPrepareTerminate() override;

    /**
     * @brief Get CJRuntime
     * @return Returns the current CJRuntime
     */
    const CJRuntime &GetCJRuntime();

    /**
     * @brief Execute insight intention when an ability is in foreground, schedule it to foreground repeatly.
     *
     * @param want Want.
     * @param executeParam insight intention execute param.
     * @param callback insight intention async callback.
     */
    void ExecuteInsightIntentRepeateForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) override;

    /**
     * @brief Execute insight intention when an ability didn't started or in background, schedule it to foreground.
     *
     * @param want Want.
     * @param executeParam insight intention execute param.
     * @param callback insight intention async callback.
     */
    void ExecuteInsightIntentMoveToForeground(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) override;

protected:
    void DoOnForeground(const Want &want) override;
    void ContinuationRestore(const Want &want) override;

private:
    bool IsRestorePageStack(const Want &want);
    void RestorePageStack(const Want &want);
    void GetPageStackFromWant(const Want &want, std::string &pageStack);
    void AbilityContinuationOrRecover(const Want &want);
    inline bool GetInsightIntentExecutorInfo(const Want &want,
        const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
        InsightIntentExecutorInfo& executeInfo);

    sptr<Rosen::CJWindowStageImpl> cjWindowStage_;
    int32_t windowMode_ = 0;
#endif
#endif
private:
    std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CreateADelegatorAbilityProperty();
    sptr<IRemoteObject> SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj);
    void SetAbilityContext(const std::shared_ptr<AbilityInfo> &abilityInfo);
    void InitSceneDoOnForeground(std::shared_ptr<Rosen::WindowScene> scene, const Want &want);
    void AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;
    void AddLifecycleEventAfterCall(FreezeUtil::TimeoutState state, const std::string &methodName) const;

    CJRuntime &cjRuntime_;
    std::shared_ptr<CJAbilityObject> cjAbilityObj_;
    sptr<IRemoteObject> remoteCallee_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CJ_UI_ABILITY_H
