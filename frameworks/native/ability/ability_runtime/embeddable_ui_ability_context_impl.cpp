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

#include "embeddable_ui_ability_context_impl.h"

#include <native_engine/native_engine.h>

#include "ability_manager_client.h"
#include "hitrace_meter.h"
#include "connection_manager.h"
#include "dialog_request_callback_impl.h"
#include "hilog_wrapper.h"
#include "remote_object_wrapper.h"
#include "request_constants.h"
#include "scene_board_judgement.h"
#include "session/host/include/zidl/session_interface.h"
#include "session_info.h"
#include "string_wrapper.h"
#include "ui_content.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t AbilityContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AbilityContext")); //TODO: 没地方使用.
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";

struct RequestResult {
    int32_t resultCode {0};
    AAFwk::Want resultWant;
    RequestDialogResultTask task;
};

Global::Resource::DeviceType EmUIAbilityContextImpl::GetDeviceType() const
{
    return (stageContext_ != nullptr) ? stageContext_->GetDeviceType() : Global::Resource::DeviceType::DEVICE_PHONE;
}

std::string EmUIAbilityContextImpl::GetBaseDir() const
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}

std::string EmUIAbilityContextImpl::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string EmUIAbilityContextImpl::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string EmUIAbilityContextImpl::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

int32_t EmUIAbilityContextImpl::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return stageContext_ ?
        stageContext_->GetSystemDatabaseDir(groupId, checkExist, databaseDir) : ERR_INVALID_VALUE;
}

std::string EmUIAbilityContextImpl::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

int EmUIAbilityContextImpl::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    return stageContext_ ?
        stageContext_->GetSystemPreferencesDir(groupId, checkExist, preferencesDir) : ERR_INVALID_VALUE;
}

std::string EmUIAbilityContextImpl::GetGroupDir(std::string groupId)
{
    return stageContext_ ? stageContext_->GetGroupDir(groupId) : "";
}

std::string EmUIAbilityContextImpl::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string EmUIAbilityContextImpl::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string EmUIAbilityContextImpl::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

bool EmUIAbilityContextImpl::IsUpdatingConfigurations()
{
    return stageContext_ ? stageContext_->IsUpdatingConfigurations() : false;
}

bool EmUIAbilityContextImpl::PrintDrawnCompleted()
{
    return stageContext_ ? stageContext_->PrintDrawnCompleted() : false;
}

void EmUIAbilityContextImpl::SwitchArea(int mode)
{
    HILOG_INFO("mode:%{public}d.", mode);
    if (stageContext_ != nullptr) {
        stageContext_->SwitchArea(mode);
    }
}

int EmUIAbilityContextImpl::GetArea()
{
    HILOG_DEBUG("GetArea");
    if (stageContext_ == nullptr) {
        HILOG_ERROR("stageContext is nullptr.");
        return ContextImpl::EL_DEFAULT;
    }
    return stageContext_->GetArea();
}

ErrCode EmUIAbilityContextImpl::StartAbility(const AAFwk::Want& want, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("StartAbility");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbility. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityAsCaller(const AAFwk::Want &want, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("StartAbilityAsCaller");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want, token_, nullptr, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbilityAsCaller. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityWithAccount(const AAFwk::Want& want, int accountId, int requestCode)
{
    HILOG_DEBUG("StartAbilityWithAccount");
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbilityWithAccount. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbility(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions,
    int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("StartAbility");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbility. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityAsCaller(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
    int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("StartAbilityAsCaller");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want,
        startOptions, token_, nullptr, requestCode);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbilityAsCaller. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityWithAccount(
    const AAFwk::Want& want, int accountId, const AAFwk::StartOptions& startOptions, int requestCode)
{
    HILOG_DEBUG("name:%{public}s %{public}s, accountId=%{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbilityWithAccount. ret=%{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityForResult(const AAFwk::Want& want, int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("StartAbilityForResult");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, -1);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        HILOG_ERROR("StartAbilityForResult. ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, const int accountId, int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("accountId:%{private}d", accountId);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        HILOG_ERROR("StartAbilityForResultWithAccount. ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityForResult(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions,
    int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("StartAbilityForResult");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        HILOG_ERROR("StartAbilityForResult. ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, int accountId, const AAFwk::StartOptions& startOptions,
    int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("StartAbilityForResultWithAccount");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        HILOG_ERROR("StartAbilityForResultWithAccount. ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StartServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    HILOG_INFO("name:%{public}s %{public}s, accountId=%{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        HILOG_ERROR("StartServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    HILOG_INFO("name:%{public}s %{public}s, accountId=%{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        HILOG_ERROR("StopServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::TerminateAbilityWithResult(const AAFwk::Want& want, int resultCode)
{
    HILOG_DEBUG("TerminateAbilityWithResult");
    isTerminating_ = true;

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sessionToken = sessionToken_.promote();
        if (sessionToken == nullptr) {
            return ERR_INVALID_VALUE;
        }
        sptr<AAFwk::SessionInfo> info = new AAFwk::SessionInfo();
        info->want = want;
        info->resultCode = resultCode;
        auto ifaceSessionToken = iface_cast<Rosen::ISession>(sessionToken);
        auto err = ifaceSessionToken->TerminateSession(info);
        HILOG_INFO("TerminateAbilityWithResult. ret=%{public}d", err);
        return static_cast<int32_t>(err);
    } else {
        ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode, &want);
        HILOG_INFO("TerminateAbilityWithResult. ret=%{public}d", err);
        return err;
    }
}

void EmUIAbilityContextImpl::SetWeakSessionToken(const wptr<IRemoteObject>& sessionToken)
{
    HILOG_DEBUG("Start calling SetWeakSessionToken.");
    sessionToken_ = sessionToken;
}

void EmUIAbilityContextImpl::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want& resultData)
{
    HILOG_DEBUG("Start calling OnAbilityResult.");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_INFO("OnAbilityResult");
}

void EmUIAbilityContextImpl::OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want& resultData)
{
    HILOG_DEBUG("Start calling OnAbilityResult.");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_INFO("OnAbilityResult");
}

ErrCode EmUIAbilityContextImpl::ConnectAbility(const AAFwk::Want& want, const sptr<AbilityConnectCallback>& connectCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ConnectAbility begin, name:%{public}s.", abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("ConnectAbility ret:%{public}d", ret);
    }
    return ret;
}

ErrCode EmUIAbilityContextImpl::ConnectAbilityWithAccount(const AAFwk::Want& want, int accountId,
    const sptr<AbilityConnectCallback>& connectCallback)
{
    HILOG_DEBUG("ConnectAbilityWithAccount");
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("ConnectAbilityWithAccount ret:%{public}d", ret);
    }
    return ret;
}

void EmUIAbilityContextImpl::DisconnectAbility(const AAFwk::Want& want,
    const sptr<AbilityConnectCallback>& connectCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("DisconnectAbility begin, caller:%{public}s.",
        abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("error, ret=%{public}d", ret);
    }
}

std::string EmUIAbilityContextImpl::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> EmUIAbilityContextImpl::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::string EmUIAbilityContextImpl::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> EmUIAbilityContextImpl::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> EmUIAbilityContextImpl::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

std::shared_ptr<Context> EmUIAbilityContextImpl::CreateBundleContext(const std::string& bundleName)
{
    return stageContext_ ? stageContext_->CreateBundleContext(bundleName) : nullptr;
}

std::shared_ptr<Context> EmUIAbilityContextImpl::CreateModuleContext(const std::string& moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<Context> EmUIAbilityContextImpl::CreateModuleContext(const std::string& bundleName,
    const std::string& moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(bundleName, moduleName) : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> EmUIAbilityContextImpl::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleResourceManager(bundleName, moduleName) : nullptr;
}

void EmUIAbilityContextImpl::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> EmUIAbilityContextImpl::GetAbilityInfo() const
{
    return abilityInfo_;
}

void EmUIAbilityContextImpl::SetStageContext(const std::shared_ptr<AbilityRuntime::Context>& stageContext)
{
    stageContext_ = stageContext;
}

void EmUIAbilityContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& config)
{
    config_ = config;
}

std::shared_ptr<AppExecFwk::Configuration> EmUIAbilityContextImpl::GetConfiguration() const
{
    return config_;
}

void EmUIAbilityContextImpl::MinimizeAbility(bool fromUser)
{
    HILOG_DEBUG("call");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->MinimizeAbility(token_, fromUser);
    if (err != ERR_OK) {
        HILOG_ERROR("MinimizeAbility is failed %{public}d", err);
    }
}

ErrCode EmUIAbilityContextImpl::OnBackPressedCallBack(bool &needMoveToBackground)
{
    HILOG_DEBUG("call");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        HILOG_ERROR("abilityCallback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    needMoveToBackground = abilityCallback->OnBackPress();
    return ERR_OK;
}

ErrCode EmUIAbilityContextImpl::MoveAbilityToBackground()
{
    HILOG_DEBUG("call");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->MoveAbilityToBackground(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("MoveAbilityToBackground failed: %{public}d", err);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::TerminateSelf()
{
    HILOG_DEBUG("TerminateSelf");
    isTerminating_ = true;
    auto sessionToken = sessionToken_.promote();
    if (sessionToken == nullptr) {
        HILOG_WARN("sessionToken is null");
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken) {
        HILOG_INFO("TerminateSelf. SCB");
        AAFwk::Want resultWant;
        sptr<AAFwk::SessionInfo> info = new AAFwk::SessionInfo();
        info->want = resultWant;
        info->resultCode = -1;
        auto ifaceSessionToken = iface_cast<Rosen::ISession>(sessionToken);
        auto err = ifaceSessionToken->TerminateSession(info);
        return static_cast<int32_t>(err);
    } else {
        AAFwk::Want resultWant;
        ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, &resultWant);
        if (err != ERR_OK) {
            HILOG_ERROR("EmUIAbilityContextImpl::TerminateSelf is failed %{public}d", err);
        }
        return err;
    }
}

ErrCode EmUIAbilityContextImpl::CloseAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("CloseAbility");
    isTerminating_ = true;
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->CloseAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("CloseAbility failed: %{public}d", err);
    }
    return err;
}

sptr<IRemoteObject> EmUIAbilityContextImpl::GetToken()
{
    return token_;
}

ErrCode EmUIAbilityContextImpl::RestoreWindowStage(napi_env env, napi_value contentStorage)
{
    HILOG_INFO("call");
    napi_ref value = nullptr;
    napi_create_reference(env, contentStorage, 1, &value);
    contentStorage_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(value));
    return ERR_OK;
}

ErrCode EmUIAbilityContextImpl::StartAbilityByCall(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack>& callback, int32_t accountId)
{
    if (localCallContainer_ == nullptr) {
        localCallContainer_ = std::make_shared<LocalCallContainer>();
        if (localCallContainer_ == nullptr) {
            HILOG_ERROR("localCallContainer_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }
    return localCallContainer_->StartAbilityByCallInner(want, callback, token_, accountId);
}

ErrCode EmUIAbilityContextImpl::ReleaseCall(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("Release begin.");
    if (localCallContainer_ == nullptr) {
        HILOG_ERROR("localCallContainer_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("Release end.");
    return localCallContainer_->ReleaseCall(callback);
}

void EmUIAbilityContextImpl::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("Clear begin.");
    if (localCallContainer_ == nullptr) {
        HILOG_ERROR("localCallContainer_ is nullptr.");
        return;
    }
    localCallContainer_->ClearFailedCallConnection(callback);
    HILOG_DEBUG("Clear end.");
}

void EmUIAbilityContextImpl::RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback)
{
    HILOG_INFO("call");
    abilityCallback_ = abilityCallback;
}

ErrCode EmUIAbilityContextImpl::RequestDialogService(napi_env env, AAFwk::Want &want, RequestDialogResultTask &&task)
{
    want.SetParam(RequestConstants::REQUEST_TOKEN_KEY, token_);
    int32_t left, top, width, height;
    GetWindowRect(left, top, width, height);
    want.SetParam(RequestConstants::WINDOW_RECTANGLE_LEFT_KEY, left);
    want.SetParam(RequestConstants::WINDOW_RECTANGLE_TOP_KEY, top);
    want.SetParam(RequestConstants::WINDOW_RECTANGLE_WIDTH_KEY, width);
    want.SetParam(RequestConstants::WINDOW_RECTANGLE_HEIGHT_KEY, height);
    auto resultTask =
        [env, outTask = std::move(task)](int32_t resultCode, const AAFwk::Want &resultWant) {
        auto retData = new RequestResult();
        retData->resultCode = resultCode;
        retData->resultWant = resultWant;
        retData->task = std::move(outTask);

        uv_loop_s* loop = nullptr;
        napi_get_uv_event_loop(env, &loop);
        if (loop == nullptr) {
            HILOG_ERROR("RequestDialogService, fail to get uv loop.");
            return;
        }
        auto work = new uv_work_t;
        work->data = static_cast<void*>(retData);
        int rev = uv_queue_work_with_qos(
            loop,
            work,
            [](uv_work_t* work) {},
            RequestDialogResultJSThreadWorker,
            uv_qos_user_initiated);
        if (rev != 0) {
            delete retData;
            retData = nullptr;
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        }
    };

    sptr<IRemoteObject> remoteObject = new DialogRequestCallbackImpl(std::move(resultTask));
    want.SetParam(RequestConstants::REQUEST_CALLBACK_KEY, remoteObject);

    auto err = AAFwk::AbilityManagerClient::GetInstance()->RequestDialogService(want, token_);
    HILOG_DEBUG("RequestDialogService ret=%{public}d", static_cast<int32_t>(err));
    return err;
}

ErrCode EmUIAbilityContextImpl::ReportDrawnCompleted()
{
    HILOG_DEBUG("called.");
    return AAFwk::AbilityManagerClient::GetInstance()->ReportDrawnCompleted(token_);
}

void EmUIAbilityContextImpl::RequestDialogResultJSThreadWorker(uv_work_t* work, int status)
{
    HILOG_DEBUG("RequestDialogResultJSThreadWorker");
    if (work == nullptr) {
        HILOG_ERROR("RequestDialogResultJSThreadWorker, uv_queue_work input work is nullptr");
        return;
    }
    RequestResult* retCB = static_cast<RequestResult*>(work->data);
    if (retCB == nullptr) {
        HILOG_ERROR("RequestDialogResultJSThreadWorker, retCB is nullptr");
        delete work;
        work = nullptr;
        return;
    }

    if (retCB->task) {
        retCB->task(retCB->resultCode, retCB->resultWant);
    }

    delete retCB;
    retCB = nullptr;
    delete work;
    work = nullptr;
}

ErrCode EmUIAbilityContextImpl::GetMissionId(int32_t &missionId)
{
    HILOG_DEBUG("GetMissionId");
    if (missionId_ != -1) {
        missionId = missionId_;
        return ERR_OK;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->GetMissionIdByToken(token_, missionId);
    if (err != ERR_OK) {
        HILOG_ERROR("GetMissionId is failed %{public}d", err);
    } else {
        missionId_ = missionId;
        HILOG_DEBUG("missionId is %{public}d.", missionId_);
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::SetMissionContinueState(const AAFwk::ContinueState &state)
{
    HILOG_DEBUG("SetMissionContinueState: %{public}d", state);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionContinueState(token_, state);
    if (err != ERR_OK) {
        HILOG_ERROR("SetMissionContinueState failed: %{public}d", err);
    }
    return err;
}

void EmUIAbilityContextImpl::GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
{
    HILOG_DEBUG("call");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback) {
        abilityCallback->GetWindowRect(left, top, width, height);
    }
}

void EmUIAbilityContextImpl::SetHalfScreenMode(const std::shared_ptr<AAFwk::Want> &want)
{
    if (want == nullptr) {
        HILOG_ERROR("The want is null.")
        return;
    }
    //TODO: get half screen flag in want
    isHalfScreenMode_ = false; // temp
}

bool EmUIAbilityContextImpl::GetHalfScreenMode()
{
    HILOG_DEBUG("GetHalfScreenMode");
    return isHalfScreenMode_;
}

#ifdef SUPPORT_GRAPHICS
ErrCode EmUIAbilityContextImpl::SetMissionLabel(const std::string& label)
{
    HILOG_DEBUG("call label:%{public}s", label.c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionLabel(token_, label);
    if (err != ERR_OK) {
        HILOG_ERROR("SetMissionLabel is failed %{public}d", err);
    } else {
        auto abilityCallback = abilityCallback_.lock();
        if (abilityCallback) {
            abilityCallback->SetMissionLabel(label);
        }
    }
    return err;
}

ErrCode EmUIAbilityContextImpl::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    HILOG_DEBUG("call");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionIcon(token_, icon);
    if (err != ERR_OK) {
        HILOG_ERROR("SetMissionIcon is failed %{public}d", err);
    } else {
        auto abilityCallback = abilityCallback_.lock();
        if (abilityCallback) {
            abilityCallback->SetMissionIcon(icon);
        }
    }
    return err;
}

int EmUIAbilityContextImpl::GetCurrentWindowMode()
{
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        return AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    }
    return abilityCallback->GetCurrentWindowMode();
}

Ace::UIContent* EmUIAbilityContextImpl::GetUIContent()
{
    HILOG_DEBUG("call");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        return nullptr;
    }

    return abilityCallback->GetUIContent();
}

ErrCode EmUIAbilityContextImpl::StartAbilityByType(const std::string &type,
    AAFwk::WantParams &wantParams, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks)
{
    HILOG_DEBUG("call");
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        HILOG_ERROR("uiContent is nullptr");
        return ERR_INVALID_VALUE;
    }
    wantParams.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParams);
    Ace::ModalUIExtensionCallbacks callback;
    callback.onError = std::bind(&JsUIExtensionCallback::OnError, uiExtensionCallbacks, std::placeholders::_1);
    callback.onRelease = std::bind(&JsUIExtensionCallback::OnRelease, uiExtensionCallbacks, std::placeholders::_1);
    Ace::ModalUIExtensionConfig config;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("CreateModalUIExtension is failed");
        return ERR_INVALID_VALUE;
    }
    uiExtensionCallbacks->SetUIContent(uiContent);
    uiExtensionCallbacks->SetSessionId(sessionId);
    return ERR_OK;
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS