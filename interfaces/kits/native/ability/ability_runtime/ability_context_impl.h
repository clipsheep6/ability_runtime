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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H

#include "ability_context.h"

#include <uv.h>

#include "context_impl.h"
#include "configuration.h"
#include "local_call_container.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityContextImpl : public AbilityContext {
public:
    AbilityContextImpl() = default;
    virtual ~AbilityContextImpl() = default;

    Global::Resource::DeviceType GetDeviceType() const override;
    std::string GetBaseDir() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetResourceDir() override;
    std::string GetNativeLibraryPath() override;
    std::string GetFilesDir() override;
    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    std::string GetDatabaseDir() override;
    std::string GetGroupDir(std::string groupId) override;
    std::string GetPreferencesDir() override;
    std::string GetDistributedFilesDir() override;
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;
    void SwitchArea(int mode) override;
    int GetArea() override;
    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;

    std::string GetBundleCodePath() const override;
    ErrCode StartAbility(const AAFwk::Want &want, int requestCode) override;
    ErrCode StartAbilityWithAccount(const AAFwk::Want &want, int accountId, int requestCode) override;
    ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode) override;
    ErrCode StartAbilityAsCaller(const AAFwk::Want &want, int requestCode) override;
    ErrCode StartAbilityAsCaller(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode) override;
    ErrCode StartAbilityWithAccount(
        const AAFwk::Want &want, int accountId, const AAFwk::StartOptions &startOptions, int requestCode) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResultWithAccount(
        const AAFwk::Want &want, int accountId, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResultWithAccount(const AAFwk::Want &want, int accountId,
        const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task) override;
    ErrCode StartAbilityForResult(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
        int requestCode, RuntimeTask &&task) override;
    ErrCode StartServiceExtensionAbility(const Want &want, int32_t accountId = -1) override;
    ErrCode StopServiceExtensionAbility(const Want& want, int32_t accountId = -1) override;
    ErrCode TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode) override;
    void OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData) override;
    ErrCode ConnectAbility(const AAFwk::Want &want,
                        const sptr<AbilityConnectCallback> &connectCallback) override;
    ErrCode ConnectAbilityWithAccount(const AAFwk::Want &want, int accountId,
                        const sptr<AbilityConnectCallback> &connectCallback) override;
    void DisconnectAbility(const AAFwk::Want &want,
                           const sptr<AbilityConnectCallback> &connectCallback) override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const override;
    void MinimizeAbility(bool fromUser = false) override;

    ErrCode OnBackPressedCallBack(bool &needMoveToBackground) override;

    ErrCode MoveAbilityToBackground() override;

    ErrCode TerminateSelf() override;

    ErrCode CloseAbility() override;

    sptr<IRemoteObject> GetToken() override;

    ErrCode RestoreWindowStage(napi_env env, napi_value contentStorage) override;

    void SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext);

    /**
     * @brief Set the Ability Info object
     *
     * set ability info to ability context
     */
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Attachs ability's token.
     *
     * @param token The token represents ability.
     */
    void SetToken(const sptr<IRemoteObject> &token) override
    {
        token_ = token;
    }

    /**
     * @brief Get ContentStorage.
     *
     * @return Returns the ContentStorage.
     */
    std::unique_ptr<NativeReference>& GetContentStorage() override
    {
        return contentStorage_;
    }

    /**
     * @brief Get LocalCallContainer.
     *
     * @return Returns the LocalCallContainer.
     */
    std::shared_ptr<LocalCallContainer> GetLocalCallContainer() override
    {
        return localCallContainer_;
    }

    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config) override;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    /**
     * call function by callback objectS
     *
     * @param want Request info for ability.
     * @param callback Indicates the callback object.
     * @param accountId Indicates the account to start.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode StartAbilityByCall(const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback,
        int32_t accountId = DEFAULT_INVAL_VALUE) override;

    /**
     * caller release by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) override;

    /**
     * clear failed call connection by callback object
     *
     * @param callback Indicates the callback object.
     *
     * @return void.
     */
    void ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) override;

    /**
     * register ability callback
     *
     * @param abilityCallback Indicates the abilityCallback object.
     */
    void RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback) override;

    bool IsTerminating() override
    {
        return isTerminating_;
    }

    void SetWeakSessionToken(const wptr<IRemoteObject>& sessionToken) override;

    void SetTerminating(bool state) override
    {
        isTerminating_ = state;
    }

    ErrCode RequestDialogService(napi_env env, AAFwk::Want &want, RequestDialogResultTask &&task) override;

    ErrCode ReportDrawnCompleted() override;

    ErrCode GetMissionId(int32_t &missionId) override;
    
    /**
     * @brief Set mission continue state of this ability.
     *
     * @param state the mission continuation state of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionContinueState(const AAFwk::ContinueState &state) override;

    ErrCode StartAbilityByType(const std::string &type,
        AAFwk::WantParams &wantParam, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks) override;

    ErrCode RequestModalUIExtension(const Want &want) override;

#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Set mission label of this ability.
     *
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionLabel(const std::string &label) override;

    /**
     * @brief Set mission icon of this ability.
     *
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    /**
     * @brief get current window mode.
     *
     * @return Returns the current window mode.
     */
    int GetCurrentWindowMode() override;

    /**
     * @brief Get window rectangle of this ability.
     *
     * @param the left position of window rectangle.
     * @param the top position of window rectangle.
     * @param the width position of window rectangle.
     * @param the height position of window rectangle.
     */
    void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height) override;

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    Ace::UIContent* GetUIContent() override;

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    ErrCode CreateModalUIExtensionWithApp(const Want &want) override;
    void EraseUIExtension(int32_t sessionId) override;
    bool IsUIExtensionExist(const AAFwk::Want &want);
#endif

private:
    sptr<IRemoteObject> token_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<AbilityRuntime::Context> stageContext_ = nullptr;
    std::map<int, RuntimeTask> resultCallbacks_;
    std::unique_ptr<NativeReference> contentStorage_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> config_ = nullptr;
    std::shared_ptr<LocalCallContainer> localCallContainer_ = nullptr;
    std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback_;
    bool isTerminating_ = false;
    int32_t missionId_ = -1;
    std::mutex sessionTokenMutex_;
    wptr<IRemoteObject> sessionToken_;
    std::mutex uiExtensionMutex_;
    std::map<int32_t, Want> uiExtensionMap_;

    static void RequestDialogResultJSThreadWorker(uv_work_t* work, int status);
    void OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData);
    sptr<IRemoteObject> GetSessionToken();
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
