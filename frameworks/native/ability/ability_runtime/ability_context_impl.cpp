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

#include "ability_context_impl.h"

#include <native_engine/native_engine.h>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "authorization_result.h"
#include "hitrace_meter.h"
#include "connection_manager.h"
#include "dialog_request_callback_impl.h"
#include "hilog_wrapper.h"
#include "permission_list_state.h"
#include "remote_object_wrapper.h"
#include "request_constants.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

using OHOS::Security::AccessToken::AccessTokenKit;
using OHOS::Security::AccessToken::PermissionListState;
using OHOS::Security::AccessToken::TypePermissionOper;

namespace OHOS {
namespace AbilityRuntime {
const size_t AbilityContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AbilityContext"));
const std::string GRANT_ABILITY_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string GRANT_ABILITY_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string STATE_KEY = "ohos.user.grant.permission.state";
const std::string TOKEN_KEY = "ohos.ability.params.token";
const std::string CALLBACK_KEY = "ohos.ability.params.callback";

std::mutex AbilityContextImpl::mutex_;
std::map<int, PermissionRequestTask> AbilityContextImpl::permissionRequestCallbacks;

struct RequestResult {
    int32_t resultCode {0};
    RequestDialogResultTask task;
};

Global::Resource::DeviceType AbilityContextImpl::GetDeviceType() const
{
    return (stageContext_ != nullptr) ? stageContext_->GetDeviceType() : Global::Resource::DeviceType::DEVICE_PHONE;
}

std::string AbilityContextImpl::GetBaseDir() const
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}

std::string AbilityContextImpl::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContextImpl::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContextImpl::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContextImpl::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

std::string AbilityContextImpl::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContextImpl::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string AbilityContextImpl::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

bool AbilityContextImpl::IsUpdatingConfigurations()
{
    return stageContext_ ? stageContext_->IsUpdatingConfigurations() : false;
}

bool AbilityContextImpl::PrintDrawnCompleted()
{
    return stageContext_ ? stageContext_->PrintDrawnCompleted() : false;
}

void AbilityContextImpl::SwitchArea(int mode)
{
    HILOG_INFO("AbilityContextImpl::SwitchArea to %{public}d.", mode);
    if (stageContext_ != nullptr) {
        stageContext_->SwitchArea(mode);
    }
}

int AbilityContextImpl::GetArea()
{
    HILOG_DEBUG("AbilityContextImpl::GetArea.");
    if (stageContext_ == nullptr) {
        HILOG_ERROR("AbilityContextImpl::stageContext is nullptr.");
        return ContextImpl::EL_DEFAULT;
    }
    return stageContext_->GetArea();
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want& want, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityAsCaller(const AAFwk::Want &want, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start calling StartAbilityAsCaller.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbilityAsCaller. End calling StartAbilityAsCaller. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityWithAccount(const AAFwk::Want& want, int accountId, int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbilityWithAccount. Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    HILOG_INFO("AbilityContextImpl::StartAbilityWithAccount. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions,
    int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityContextImpl::StartAbility. Start calling StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityAsCaller(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
    int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityContextImpl::StartAbilityAsCaller. Start calling StartAbilityAsCaller.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want,
        startOptions, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbilityAsCaller. End calling StartAbilityAsCaller. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityWithAccount(
    const AAFwk::Want& want, int accountId, const AAFwk::StartOptions& startOptions, int requestCode)
{
    HILOG_INFO("%{public}s called, bundleName=%{public}s, abilityName=%{public}s, accountId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    HILOG_INFO("AbilityContextImpl::StartAbilityWithAccount. End calling StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want& want, int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResult.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("%{public}s. End calling StartAbilityForResult. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, const int accountId, int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("%{public}s called, accountId:%{private}d", __func__, accountId);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode, accountId);
    HILOG_INFO("%{public}s end. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want& want, const AAFwk::StartOptions& startOptions,
    int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResult.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    HILOG_INFO("%{public}s. End calling StartAbilityForResult. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResultWithAccount(
    const AAFwk::Want& want, int accountId, const AAFwk::StartOptions& startOptions,
    int requestCode, RuntimeTask&& task)
{
    HILOG_DEBUG("%{public}s. Start calling StartAbilityForResultWithAccount.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, startOptions, token_, requestCode, accountId);
    HILOG_INFO("%{public}s. End calling StartAbilityForResultWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode AbilityContextImpl::StartServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    HILOG_INFO("%{public}s begin. bundleName=%{public}s, abilityName=%{public}s, accountId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::StartServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContextImpl::StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    HILOG_INFO("%{public}s begin. bundleName=%{public}s, abilityName=%{public}s, accountId=%{public}d",
        __func__, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::StopServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContextImpl::TerminateAbilityWithResult(const AAFwk::Want& want, int resultCode)
{
    HILOG_DEBUG("%{public}s. Start calling TerminateAbilityWithResult.", __func__);
    isTerminating_ = true;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode, &want);
    HILOG_INFO("%{public}s. End calling TerminateAbilityWithResult. ret=%{public}d", __func__, err);
    return err;
}

void AbilityContextImpl::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want& resultData)
{
    HILOG_DEBUG("%{public}s. Start calling OnAbilityResult.", __func__);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_INFO("%{public}s. End calling OnAbilityResult.", __func__);
}

void AbilityContextImpl::OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want& resultData)
{
    HILOG_DEBUG("%{public}s. Start calling OnAbilityResult.", __func__);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_INFO("%{public}s. End calling OnAbilityResult.", __func__);
}

ErrCode AbilityContextImpl::ConnectAbility(const AAFwk::Want& want, const sptr<AbilityConnectCallback>& connectCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ConnectAbility begin, name:%{public}s.", abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("AbilityContextImpl::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode AbilityContextImpl::ConnectAbilityWithAccount(const AAFwk::Want& want, int accountId,
    const sptr<AbilityConnectCallback>& connectCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    HILOG_INFO("AbilityContextImpl::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

void AbilityContextImpl::DisconnectAbility(const AAFwk::Want& want,
    const sptr<AbilityConnectCallback>& connectCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("DisconnectAbility begin, caller:%{public}s.",
        abilityInfo_ == nullptr ? "" : abilityInfo_->name.c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
}

std::string AbilityContextImpl::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContextImpl::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::string AbilityContextImpl::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContextImpl::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContextImpl::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

std::shared_ptr<Context> AbilityContextImpl::CreateBundleContext(const std::string& bundleName)
{
    return stageContext_ ? stageContext_->CreateBundleContext(bundleName) : nullptr;
}

std::shared_ptr<Context> AbilityContextImpl::CreateModuleContext(const std::string& moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<Context> AbilityContextImpl::CreateModuleContext(const std::string& bundleName,
    const std::string& moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(bundleName, moduleName) : nullptr;
}

void AbilityContextImpl::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContextImpl::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContextImpl::SetStageContext(const std::shared_ptr<AbilityRuntime::Context>& stageContext)
{
    stageContext_ = stageContext;
}

void AbilityContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& config)
{
    config_ = config;
}

std::shared_ptr<AppExecFwk::Configuration> AbilityContextImpl::GetConfiguration() const
{
    return config_;
}

void AbilityContextImpl::MinimizeAbility(bool fromUser)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->MinimizeAbility(token_, fromUser);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::MinimizeAbility is failed %{public}d", err);
    }
}

ErrCode AbilityContextImpl::TerminateSelf()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    isTerminating_ = true;
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::TerminateSelf is failed %{public}d", err);
    }
    return err;
}

ErrCode AbilityContextImpl::CloseAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
    isTerminating_ = true;
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->CloseAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("CloseAbility failed: %{public}d", err);
    }
    return err;
}

sptr<IRemoteObject> AbilityContextImpl::GetToken()
{
    return token_;
}

void AbilityContextImpl::RequestPermissionsFromUser(NativeEngine& engine, const std::vector<std::string>& permissions,
    int requestCode, PermissionRequestTask&& task)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (permissions.empty()) {
        HILOG_ERROR("%{public}s. The params are invalid.", __func__);
        return;
    }

    std::vector<PermissionListState> permList;
    for (const auto& permission : permissions) {
        HILOG_DEBUG("%{public}s. permission: %{public}s.", __func__, permission.c_str());
        PermissionListState permState;
        permState.permissionName = permission;
        permState.state = -1;
        permList.emplace_back(permState);
    }
    HILOG_DEBUG("%{public}s. permList size: %{public}zu, permissions size: %{public}zu.",
        __func__, permList.size(), permissions.size());

    auto ret = AccessTokenKit::GetSelfPermissionsState(permList);
    if (permList.size() != permissions.size()) {
        HILOG_ERROR("%{public}s. Returned permList size: %{public}zu.", __func__, permList.size());
        return;
    }

    std::vector<int> permissionsState;
    for (const auto& permState : permList) {
        HILOG_DEBUG("%{public}s. permissions: %{public}s. permissionsState: %{public}u",
            __func__, permState.permissionName.c_str(), permState.state);
        permissionsState.emplace_back(permState.state);
    }
    HILOG_DEBUG("%{public}s. permissions size: %{public}zu. permissionsState size: %{public}zu",
        __func__, permissions.size(), permissionsState.size());

    if (ret == TypePermissionOper::DYNAMIC_OPER) {
        StartGrantExtension(engine, permissions, permissionsState, requestCode, std::move(task));
    } else {
        HILOG_DEBUG("%{public}s. No dynamic popup required.", __func__);
        if (task) {
            task(permissions, permissionsState);
        }
    }
}

void AbilityContextImpl::StartGrantExtension(NativeEngine& engine, const std::vector<std::string>& permissions,
    const std::vector<int>& permissionsState, int requestCode, PermissionRequestTask&& task)
{
    AAFwk::Want want;
    want.SetElementName(GRANT_ABILITY_BUNDLE_NAME, GRANT_ABILITY_ABILITY_NAME);
    want.SetParam(PERMISSION_KEY, permissions);
    want.SetParam(STATE_KEY, permissionsState);
    want.SetParam(TOKEN_KEY, token_);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        permissionRequestCallbacks.insert(make_pair(requestCode, std::move(task)));
    }
    auto resultTask =
        [&engine, requestCode](const std::vector<std::string>& permissions, const std::vector<int>& grantResults) {
        auto retCB = new ResultCallback();
        retCB->permissions_ = permissions;
        retCB->grantResults_ = grantResults;
        retCB->requestCode_ = requestCode;

        auto loop = engine.GetUVLoop();
        if (loop == nullptr) {
            HILOG_ERROR("StartGrantExtension, fail to get uv loop.");
            return;
        }
        auto work = new uv_work_t;
        work->data = static_cast<void*>(retCB);
        int rev = uv_queue_work(
            loop,
            work,
            [](uv_work_t* work) {},
            ResultCallbackJSThreadWorker);
        if (rev != 0) {
            if (retCB != nullptr) {
                delete retCB;
                retCB = nullptr;
            }
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        }
    };

    sptr<IRemoteObject> remoteObject = new AuthorizationResult(std::move(resultTask));
    want.SetParam(CALLBACK_KEY, remoteObject);

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, -1);
    HILOG_DEBUG("%{public}s. End calling StartExtension. ret=%{public}d", __func__, err);
}

void AbilityContextImpl::ResultCallbackJSThreadWorker(uv_work_t* work, int status)
{
    HILOG_DEBUG("ResultCallbackJSThreadWorker is called.");
    if (work == nullptr) {
        HILOG_ERROR("ResultCallbackJSThreadWorker, uv_queue_work input work is nullptr");
        return;
    }
    ResultCallback* retCB = static_cast<ResultCallback*>(work->data);
    if (retCB == nullptr) {
        HILOG_ERROR("ResultCallbackJSThreadWorker, retCB is nullptr");
        delete work;
        work = nullptr;
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto requestCode = retCB->requestCode_;
    auto iter = permissionRequestCallbacks.find(requestCode);
    if (iter != permissionRequestCallbacks.end() && iter->second) {
        auto task = iter->second;
        if (task) {
            HILOG_DEBUG("%{public}s. calling js task.", __func__);
            task(retCB->permissions_, retCB->grantResults_);
        }
        permissionRequestCallbacks.erase(iter);
    }

    delete retCB;
    retCB = nullptr;
    delete work;
    work = nullptr;
}

ErrCode AbilityContextImpl::RestoreWindowStage(NativeEngine& engine, NativeValue* contentStorage)
{
    HILOG_INFO("%{public}s begin.", __func__);
    contentStorage_ = std::unique_ptr<NativeReference>(engine.CreateReference(contentStorage, 1));
    return ERR_OK;
}

ErrCode AbilityContextImpl::StartAbilityByCall(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack>& callback)
{
    if (localCallContainer_ == nullptr) {
        localCallContainer_ = new (std::nothrow) LocalCallContainer();
        if (localCallContainer_ == nullptr) {
            HILOG_ERROR("%{public}s failed, localCallContainer_ is nullptr.", __func__);
            return ERR_INVALID_VALUE;
        }
    }
    return localCallContainer_->StartAbilityByCallInner(want, callback, token_);
}

ErrCode AbilityContextImpl::ReleaseCall(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("AbilityContextImpl::Release begin.");
    if (localCallContainer_ == nullptr) {
        HILOG_ERROR("%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("AbilityContextImpl::Release end.");
    return localCallContainer_->ReleaseCall(callback);
}

void AbilityContextImpl::RegisterAbilityCallback(std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback)
{
    HILOG_INFO("%{public}s called.", __func__);
    abilityCallback_ = abilityCallback;
}

ErrCode AbilityContextImpl::RequestDialogService(NativeEngine &engine,
    AAFwk::Want &want, RequestDialogResultTask &&task)
{
    want.SetParam(RequestConstants::REQUEST_TOKEN_KEY, token_);

    auto resultTask =
        [&engine, outTask = std::move(task)](int32_t resultCode) {
        auto retData = new RequestResult();
        retData->resultCode = resultCode;
        retData->task = std::move(outTask);

        auto loop = engine.GetUVLoop();
        if (loop == nullptr) {
            HILOG_ERROR("RequestDialogService, fail to get uv loop.");
            return;
        }
        auto work = new uv_work_t;
        work->data = static_cast<void*>(retData);
        int rev = uv_queue_work(
            loop,
            work,
            [](uv_work_t* work) {},
            RequestDialogResultJSThreadWorker);
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

    auto err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, -1);
    HILOG_DEBUG("RequestDialogService ret=%{public}d", static_cast<int32_t>(err));
    return err;
}

void AbilityContextImpl::RequestDialogResultJSThreadWorker(uv_work_t* work, int status)
{
    HILOG_DEBUG("RequestDialogResultJSThreadWorker is called.");
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
        retCB->task(retCB->resultCode);
    }

    delete retCB;
    retCB = nullptr;
    delete work;
    work = nullptr;
}

ErrCode AbilityContextImpl::GetMissionId(int32_t &missionId)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (missionId_ != -1) {
        missionId = missionId_;
        return ERR_OK;
    }

    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->GetMissionIdByToken(token_, missionId);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::GetMissionId is failed %{public}d", err);
    } else {
        missionId_ = missionId;
        HILOG_DEBUG("%{public}s success, missionId is %{public}d.", __func__, missionId_);
    }
    return err;
}

#ifdef SUPPORT_GRAPHICS
ErrCode AbilityContextImpl::SetMissionLabel(const std::string& label)
{
    HILOG_INFO("%{public}s begin. label = %{public}s", __func__, label.c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionLabel(token_, label);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::SetMissionLabel is failed %{public}d", err);
    } else {
        HILOG_INFO("AbilityContextImpl::SetMissionLabel success.");
        auto abilityCallback = abilityCallback_.lock();
        if (abilityCallback) {
            abilityCallback->SetMissionLabel(label);
        }
    }
    return err;
}

ErrCode AbilityContextImpl::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->SetMissionIcon(token_, icon);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::SetMissionIcon is failed %{public}d", err);
    } else {
        HILOG_INFO("AbilityContextImpl::SetMissionIcon success.");
        auto abilityCallback = abilityCallback_.lock();
        if (abilityCallback) {
            abilityCallback->SetMissionIcon(icon);
        }
    }
    return err;
}

int AbilityContextImpl::GetCurrentWindowMode()
{
    HILOG_INFO("%{public}s called.", __func__);
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        return AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    }
    return abilityCallback->GetCurrentWindowMode();
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
