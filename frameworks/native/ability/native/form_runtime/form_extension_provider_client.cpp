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

#include "form_runtime/form_extension_provider_client.h"

#include "string_ex.h"

#include "appexecfwk_errors.h"
#include "event_handler.h"
#include "event_runner.h"
#include "form_caller_mgr.h"
#include "form_extension.h"
#include "form_mgr_errors.h"
#include "form_supply_proxy.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

int FormExtensionProviderClient::AcquireProviderFormInfo(const AppExecFwk::FormJsInfo &formJsInfo, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_ERROR("IFormSupply is nullptr.");
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    Want connectWant(want);
    connectWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    connectWant.SetParam(Constants::FORM_CONNECT_ID, want.GetIntParam(Constants::FORM_CONNECT_ID, 0));
    connectWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    connectWant.SetParam(Constants::PROVIDER_FLAG, true);
    connectWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formJsInfo.formId));

    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_WARN("Permission denied.");
        FormProviderInfo formProviderInfo;
        connectWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_PERMISSION_DENY);
        return FormProviderClient::HandleAcquire(formProviderInfo, connectWant, callerToken);
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> acquireProviderInfoFunc = [client = sptr<FormExtensionProviderClient>(this),
        formJsInfo, want, callerToken]() {
        client->AcquireFormExtensionProviderInfo(formJsInfo, want, callerToken);
    };
    mainHandler->PostSyncTask(acquireProviderInfoFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::AcquireFormExtensionProviderInfo(const AppExecFwk::FormJsInfo &formJsInfo,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    Want connectWant(want);
    connectWant.SetParam(Constants::ACQUIRE_TYPE, want.GetIntParam(Constants::ACQUIRE_TYPE, 0));
    connectWant.SetParam(Constants::FORM_CONNECT_ID, want.GetIntParam(Constants::FORM_CONNECT_ID, 0));
    connectWant.SetParam(Constants::FORM_SUPPLY_INFO, want.GetStringParam(Constants::FORM_SUPPLY_INFO));
    connectWant.SetParam(Constants::PROVIDER_FLAG, true);
    connectWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formJsInfo.formId));

    FormProviderInfo formProviderInfo;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        connectWant.SetParam(Constants::PROVIDER_FLAG, ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY);
    } else {
        Want createWant(want);
        createWant.SetParam(Constants::PARAM_FORM_IDENTITY_KEY, std::to_string(formJsInfo.formId));
        createWant.RemoveParam(Constants::FORM_CONNECT_ID);
        createWant.RemoveParam(Constants::ACQUIRE_TYPE);
        createWant.RemoveParam(Constants::FORM_SUPPLY_INFO);
        createWant.RemoveParam(Constants::PARAM_FORM_HOST_TOKEN);
        createWant.RemoveParam(Constants::FORM_COMP_ID);
        createWant.RemoveParam(Constants::FORM_DENSITY);
        createWant.RemoveParam(Constants::FORM_PROCESS_ON_ADD_SURFACE);
        createWant.RemoveParam(Constants::FORM_ALLOW_UPDATE);
        createWant.SetElement(want.GetElement());
        if (!createWant.HasParameter(Constants::LAUNCH_REASON_KEY)) {
            createWant.SetParam(Constants::LAUNCH_REASON_KEY, Constants::FORM_DEFAULT);
        }
        if (!createWant.HasParameter(Constants::PARAM_FORM_CUSTOMIZE_KEY)) {
            std::vector<std::string> customizeData;
            createWant.SetParam(Constants::PARAM_FORM_CUSTOMIZE_KEY, customizeData);
        }

        formProviderInfo = ownerFormExtension->OnCreate(createWant);
        HILOG_DEBUG("FormJsInfo.formId: %{public}s, data: %{public}s, proxies size: %{public}zu.",
            createWant.GetStringParam(Constants::PARAM_FORM_IDENTITY_KEY).c_str(),
            formProviderInfo.GetFormDataString().c_str(),
            formProviderInfo.GetFormProxies().size());
    }

    if (connectWant.HasParameter(Constants::PARAM_FORM_HOST_TOKEN)) {
        FormProviderClient::HandleRemoteAcquire(formJsInfo, formProviderInfo, connectWant, AsObject());
    }
    int error = FormProviderClient::HandleAcquire(formProviderInfo, connectWant, callerToken);
    if (error != ERR_OK) {
        HILOG_ERROR("HandleAcquire failed with %{public}d.", error);
        HandleResultCode(error, connectWant, callerToken);
    }
    HILOG_DEBUG("called end.");
}

int FormExtensionProviderClient::NotifyFormDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionDeleteFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionDelete(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionDeleteFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionDelete(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    int errorCode = ERR_OK;
    auto hostToken = want.GetRemoteObject(Constants::PARAM_FORM_HOST_TOKEN);
    if (hostToken != nullptr) {
        HILOG_DEBUG("remove provider caller.");
        FormCallerMgr::GetInstance().RemoveFormProviderCaller(formId, hostToken);
        HandleResultCode(errorCode, want, callerToken);
        return;
    }
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnDestroy(formId);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("called end.");
}

int FormExtensionProviderClient::NotifyFormsDelete(const std::vector<int64_t> &formIds, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionsDeleteFunc = [client = sptr<FormExtensionProviderClient>(this),
        formIds, want, callerToken]() {
        client->NotifyFormExtensionsDelete(formIds, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionsDeleteFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionsDelete(const std::vector<int64_t> &formIds,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        for (int64_t formId : formIds) {
            ownerFormExtension->OnDestroy(formId);
        }
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_DEBUG("called end.");
}

int FormExtensionProviderClient::NotifyFormUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionUpdateFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionUpdate(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionUpdateFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionUpdate(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnUpdate(formId);
    }

    if (want.HasParameter(Constants::FORM_CONNECT_ID)) {
        HandleResultCode(errorCode, want, callerToken);
    }
    HILOG_DEBUG("called end.");
}

int FormExtensionProviderClient::EventNotify(const std::vector<int64_t> &formIds, const int32_t formVisibleType,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> eventNotifyExtensionFunc = [client = sptr<FormExtensionProviderClient>(this),
        formIds, formVisibleType, want, callerToken]() {
        client->EventNotifyExtension(formIds, formVisibleType, want, callerToken);
    };
    mainHandler->PostSyncTask(eventNotifyExtensionFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::EventNotifyExtension(const std::vector<int64_t> &formIds,
    const int32_t formVisibleType, const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        std::map<int64_t, int32_t> formEventsMap;
        for (const auto &formId : formIds) {
            formEventsMap.insert(std::make_pair(formId, formVisibleType));
        }
        ownerFormExtension->OnVisibilityChange(formEventsMap);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("called end.");
}

int FormExtensionProviderClient::NotifyFormCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionCastTempFormFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, want, callerToken]() {
        client->NotifyFormExtensionCastTempForm(formId, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionCastTempFormFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionCastTempForm(const int64_t formId, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnCastToNormal(formId);
    }

    HandleResultCode(errorCode, want, callerToken);
    HILOG_INFO("called end.");
}

int FormExtensionProviderClient::FireFormEvent(const int64_t formId, const std::string &message,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> fireFormExtensionEventFunc = [client = sptr<FormExtensionProviderClient>(this),
        formId, message, want, callerToken]() {
        client->FireFormExtensionEvent(formId, message, want, callerToken);
    };
    mainHandler->PostSyncTask(fireFormExtensionEventFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::FireFormExtensionEvent(const int64_t formId, const std::string &message,
    const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    int errorCode = ERR_OK;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        errorCode = ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    } else {
        ownerFormExtension->OnEvent(formId, message);
    }

    if (want.HasParameter(Constants::FORM_CONNECT_ID)) {
        HandleResultCode(errorCode, want, callerToken);
    }
    HILOG_INFO("called end.");
}

int FormExtensionProviderClient::AcquireState(const Want &wantArg, const std::string &provider, const Want &want,
                                              const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    std::pair<int, int> errorCode = CheckParam(want, callerToken);
    if (errorCode.first != ERR_OK) {
        HILOG_ERROR("CheckParam failed with %{public}d.", errorCode.first);
        return errorCode.second;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    std::function<void()> notifyFormExtensionAcquireStateFunc = [client = sptr<FormExtensionProviderClient>(this),
        wantArg, provider, want, callerToken]() {
        client->NotifyFormExtensionAcquireState(wantArg, provider, want, callerToken);
    };
    mainHandler->PostSyncTask(notifyFormExtensionAcquireStateFunc);
    return ERR_OK;
}

void FormExtensionProviderClient::NotifyFormExtensionAcquireState(const Want &wantArg, const std::string &provider,
                                                                  const Want &want,
                                                                  const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("called.");
    FormState state = FormState::UNKNOWN;
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
    } else {
        state = ownerFormExtension->OnAcquireFormState(wantArg);
    }
    HandleAcquireStateResult(state, provider, wantArg, want, callerToken);
    HILOG_INFO("called end.");
}

void FormExtensionProviderClient::SetOwner(const std::shared_ptr<FormExtension> formExtension)
{
    if (formExtension == nullptr) {
        HILOG_ERROR("FormExtension is nullptr.");
        return;
    }

    std::lock_guard<std::mutex> lock(formExtensionMutex_);
    ownerFormExtension_ = formExtension;
}

void FormExtensionProviderClient::ClearOwner(const std::shared_ptr<FormExtension> formExtension)
{
    if (formExtension == nullptr) {
        HILOG_ERROR("FormExtension is nullptr.");
        return;
    }

    std::lock_guard<std::mutex> lock(formExtensionMutex_);
    std::shared_ptr<FormExtension> ownerFormExtension = ownerFormExtension_.lock();
    if (formExtension == ownerFormExtension) {
        ownerFormExtension_.reset();
    }
}

std::shared_ptr<FormExtension> FormExtensionProviderClient::GetOwner()
{
    std::shared_ptr<FormExtension> owner = nullptr;
    std::lock_guard<std::mutex> lock(formExtensionMutex_);
    owner = ownerFormExtension_.lock();
    return owner;
}

int FormExtensionProviderClient::HandleResultCode(int errorCode, const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    int disconnectErrorCode = FormProviderClient::HandleDisconnect(want, callerToken);
    if (errorCode != ERR_OK) {
        // If errorCode is not ERR_OK return errorCode.
        return errorCode;
    } else {
        // If errorCode is ERR_OK return disconnectErrorCode.
        if (disconnectErrorCode != ERR_OK) {
            HILOG_ERROR("disconnect error: %{public}d.", disconnectErrorCode);
        }
        return disconnectErrorCode;
    }
}

std::pair<ErrCode, ErrCode> FormExtensionProviderClient::CheckParam(const Want &want,
    const sptr<IRemoteObject> &callerToken)
{
    if (IsCallBySelfBundle()) {
        return std::pair<ErrCode, ErrCode>(ERR_OK, ERR_OK);
    }
    sptr<IFormSupply> formSupplyClient = iface_cast<IFormSupply>(callerToken);
    if (formSupplyClient == nullptr) {
        HILOG_ERROR("IFormSupply is nullptr.");
        return std::pair<ErrCode, ErrCode>(ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED,
            ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED);
    }

    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_ERROR("Permission denied.");
        int errorCode = HandleResultCode(ERR_APPEXECFWK_FORM_PERMISSION_DENY, want, callerToken);
        return std::pair<ErrCode, ErrCode>(ERR_APPEXECFWK_FORM_PERMISSION_DENY, errorCode);
    }
    return std::pair<ErrCode, ErrCode>(ERR_OK, ERR_OK);
}

int32_t FormExtensionProviderClient::AcquireShareFormData(int64_t formId, const std::string &remoteDeviceId,
    const sptr<IRemoteObject> &formSupplyCallback, int64_t requestCode)
{
    HILOG_DEBUG("called.");

    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_ERROR("Permission denied.");
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    auto formCall = iface_cast<IFormSupply>(formSupplyCallback);
    if (formCall == nullptr) {
        HILOG_ERROR("IFormSupply is nullptr.");
        return ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY;
    }

    AAFwk::WantParams wantParams;
    bool result = false;
    auto taskProc = [client = sptr<FormExtensionProviderClient>(this), formId, &wantParams, &result]() {
        result = client->AcquireFormExtensionProviderShareFormInfo(formId, wantParams);
    };
    mainHandler->PostSyncTask(taskProc);
    formCall->OnShareAcquire(formId, remoteDeviceId, wantParams, requestCode, result);

    return ERR_OK;
}

bool FormExtensionProviderClient::AcquireFormExtensionProviderShareFormInfo(
    int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("called.");
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        return false;
    }

    return ownerFormExtension->OnShare(formId, wantParams);
}

int32_t FormExtensionProviderClient::AcquireFormData(int64_t formId, const sptr<IRemoteObject> &formSupplyCallback,
    int64_t requestCode)
{
    HILOG_DEBUG("called.");

    if (!FormProviderClient::CheckIsSystemApp()) {
        HILOG_ERROR("Permission denied.");
        return ERR_APPEXECFWK_FORM_PERMISSION_DENY;
    }

    std::shared_ptr<EventHandler> mainHandler = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    auto formCall = iface_cast<IFormSupply>(formSupplyCallback);
    if (formCall == nullptr) {
        HILOG_ERROR("error, callback is nullptr.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    AAFwk::WantParams wantParams;
    bool result = false;
    auto taskProc = [client = wptr<FormExtensionProviderClient>(this), formId, &wantParams, &result]() {
        result = client->FormExtensionProviderAcquireFormData(formId, wantParams);
    };
    mainHandler->PostSyncTask(taskProc);
    formCall->OnAcquireDataResult(wantParams, requestCode);

    return ERR_OK;
}

bool FormExtensionProviderClient::FormExtensionProviderAcquireFormData(
    int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("called.");
    std::shared_ptr<FormExtension> ownerFormExtension = GetOwner();
    if (ownerFormExtension == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        return false;
    }

    return ownerFormExtension->OnAcquireData(formId, wantParams);
}
} // namespace AbilityRuntime
} // namespace OHOS
