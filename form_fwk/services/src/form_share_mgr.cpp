/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_info.h"
#include "form_share_mgr.h"
#include "form_ams_helper.h"
#include "form_bms_helper.h"
#include "form_data_mgr.h"
#include "form_mgr_errors.h"
#include "form_host_interface.h"
#include "form_provider_interface.h"
#include "form_share_connection.h"
#include "form_util.h"
#include "form_constants.h"
#include "form_event_handler.h"
#include "form_supply_callback.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
const int FORM_SHARE_INFO_DELAY_TIMER = 50000;
const int FORM_PACKAGE_FREE_INSTALL_TIMER = 40000;

static bool GetAbilityInfoByAction(const std::string action, AppExecFwk::AbilityInfo &abilityInfo)
{
    HILOG_ERROR("%{public}s called.", __func__);
    if (action.empty()) {
        HILOG_ERROR("%{public}s input parasm error.", __func__);
        return false;
    }

    Want wantAction;
    wantAction.SetAction(action);
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t userId = FormUtil::GetCurrentAccountId();
    auto iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (!IN_PROCESS_CALL(iBundleMgr->ImplicitQueryInfoByPriority(wantAction,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId, abilityInfo, extensionAbilityInfo))) {
        HILOG_ERROR("%{public}s Failed to ImplicitQueryInfoByPriority for publishing form, c%{public}d, u%{public}d",
            __func__, callingUid, userId);
        return false;
    }

    if (abilityInfo.name.empty() || abilityInfo.bundleName.empty()) {
        HILOG_ERROR("%{public}sQuery highest priority ability failed, no form host ability found.", __func__);
        return false;
    }

    HILOG_ERROR("%{public}s end.", __func__);
    return true;
}

FormShareMgr::FormShareMgr()
{
    HILOG_DEBUG("FormShareMgr is created");
};

FormShareMgr::~FormShareMgr()
{
    HILOG_DEBUG("FormShareMgr is destroyed");
};

int FormShareMgr::ShareForm(const int64_t formId, const std::string &deviceId, const sptr<IRemoteObject> &callerToken,
    const int64_t requestCode)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    FormRecord formRecord;
    bool isFormRecExist = FormDataMgr::GetInstance().GetFormRecord(formId, formRecord);
    if (!isFormRecExist) {
        HILOG_ERROR("form share info get formRecord failed.");
        return ERR_APPEXECFWK_FORM_GET_INFO_FAILED;
    }

    {
        std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
        requestMap_.emplace(requestCode, callerToken);
    }

    sptr<FormShareConnection> formShareConnection = new FormShareConnection(formId, formRecord.bundleName,
        formRecord.abilityName, deviceId, requestCode);
    Want want;
    want.SetElementName(formRecord.bundleName, formRecord.abilityName);
    want.AddFlags(Want::FLAG_ABILITY_FORM_ENABLED);
    ErrCode errorCode = FormAmsHelper::GetInstance().ConnectServiceAbility(want, formShareConnection);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s fail, ConnectServiceAbility failed.", __func__);
        std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
        requestMap_.erase(requestCode);
        return ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED;
    }

    return ERR_OK;
}

int FormShareMgr::RecvFormShareInfoFromRemote(const FormShareInfo &info)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    if (eventHandler_ == nullptr) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return ERR_APPEXECFWK_FORM_COMMON_CODE;
    }

    auto task = [this, info]() { this->HandleRecvFormShareInfoFromRemoteTask(info); };
    eventHandler_->PostTask(task);

    return ERR_OK;
}

int FormShareMgr::HandleRecvFormShareInfoFromRemoteTask(const FormShareInfo &info)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    if (!CheckFormShareInfo(info)) {
        HILOG_ERROR("form share info check failed.");
        return ERR_APPEXECFWK_FORM_SHARE_INFO_CHECK_FAILED;
    }

    auto formShareInfoKey = makeFormShareInfoKey(info);
    {
        std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
        if (shareInfo_.find(formShareInfoKey) != shareInfo_.end()) {
            HILOG_ERROR("form is beging shared.");
            return ERR_APPEXECFWK_FORM_SHARING;
        }

        shareInfo_.emplace(formShareInfoKey, info);

        auto eventId = FormEventHandler::GetEventId();
        eventMap_.emplace(eventId, formShareInfoKey);
        eventHandler_->SendEvent(MSG::FORM_SHARE_INFO_DELAY_MSG, eventId, FORM_SHARE_INFO_DELAY_TIMER);
    }

    if (!IsExistFormPackage(info.bundleName, info.moduleName)) {
        std::shared_ptr<FormFreeInstallOperator> freeInstallOperator =
            std::make_shared<FormFreeInstallOperator>(formShareInfoKey, eventHandler_);
        auto eventId = FormEventHandler::GetEventId();
        HILOG_DEBUG("free install operator send event, eventId:%{public}" PRId64 ", key: %{public}s",
            eventId, formShareInfoKey.c_str());
        eventHandler_->SendEvent(MSG::FORM_PACKAGE_FREE_INSTALL_DELAY_MSG, eventId, FORM_PACKAGE_FREE_INSTALL_TIMER);

        {
            std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
            freeInstallOperatorMap_.emplace(eventId, freeInstallOperator);
        }

        auto ret = freeInstallOperator->StartFreeInstall(info.bundleName, info.abilityName);
        if (ret != ERR_OK) {
            HILOG_ERROR("free install failed.");
            RemoveFormShareInfo(formShareInfoKey);
            FinishFreeInstallTask(freeInstallOperator);
            return ERR_APPEXECFWK_FORM_FREE_INSTALL_FAILED;
        }
        return ret;
    }

    StartFormUser(info);

    return ERR_OK;
}

bool FormShareMgr::CheckFormShareInfo(const FormShareInfo &info)
{
    return !(info.bundleName.empty() ||
            info.moduleName.empty() ||
            info.abilityName.empty() ||
            info.formName.empty() ||
            info.deviceName.empty() ||
            info.deviceId.empty() ||
            (info.dimensionId <= 0));
}

std::string FormShareMgr::makeFormShareInfoKey(const FormShareInfo &info)
{
    return (info.bundleName + info.moduleName + info.abilityName + info.formName);
}

std::string FormShareMgr::makeFormShareInfoKey(const FormItemInfo &info)
{
    return (info.GetProviderBundleName() + info.GetModuleName() + info.GetAbilityName() + info.GetFormName());
}

void FormShareMgr::StartFormUser(const FormShareInfo &info)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AppExecFwk::AbilityInfo abilityInfo;
    if (!GetAbilityInfoByAction(Want::ACTION_SHARE_FORM, abilityInfo)) {
        HILOG_ERROR("%{public}s GetAbilityInfoByAction failed", __func__);
        return;
    }
    Want want;
    want.SetElementName(abilityInfo.bundleName, abilityInfo.name);
    want.SetAction(Want::ACTION_SHARE_FORM);
    want.SetParam(Constants::PARAM_SHARE_FORM_BUNDLENAME_KEY, info.bundleName);
    want.SetParam(Constants::PARAM_SHARE_FORM_MOUDLENAME_KEY, info.moduleName);
    want.SetParam(Constants::PARAM_SHARE_FORM_ABILITYNAME_KEY, info.abilityName);
    want.SetParam(Constants::PARAM_SHARE_FORM_NAME_KEY, info.formName);
    want.SetParam(Constants::PARAM_SHARE_FORM_TEMP_FLAG_KEY, info.formTempFlag);
    want.SetParam(Constants::PARAM_SHARE_FORM_DIMENSION_ID_KEY, info.dimensionId);

    auto errorCode = FormAmsHelper::GetInstance().StartAbility(want);
    if (errorCode != ERR_OK) {
        HILOG_ERROR("%{public}s failed.", __func__);
        RemoveFormShareInfo(makeFormShareInfoKey(info));
    }
}

bool FormShareMgr::IsExistFormPackage(const std::string &bundleName, const std::string &moduleName)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    sptr<IBundleMgr> iBundleMgr = FormBmsHelper::GetInstance().GetBundleMgr();
    if (iBundleMgr == nullptr) {
        HILOG_ERROR("get IBundleMgr failed");
        return false;
    }

    BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(iBundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, FormUtil::GetCurrentAccountId()))) {
        HILOG_ERROR("get bundle info failed");
        return false;
    }

    for (const auto &moduleInfo : bundleInfo.moduleNames) {
        if (moduleInfo.compare(moduleName) == 0) {
            HILOG_DEBUG("module name is exist, name:%{public}s", moduleInfo.c_str());
            return true;
        }
    }

    HILOG_ERROR("module name is not exist");
    return false;
}

void FormShareMgr::RemoveFormShareInfo(const std::string &formShareInfoKey)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
    int64_t eventId = 0;
    for (auto iter = eventMap_.begin(); iter != eventMap_.end(); iter++) {
        if (iter->second == formShareInfoKey) {
            eventId = iter->first;
            break;
        }
    }

    if (eventId != 0) {
        eventHandler_->RemoveEvent(MSG::FORM_SHARE_INFO_DELAY_MSG, eventId);
        eventMap_.erase(eventId);
    }

    shareInfo_.erase(formShareInfoKey);
}

void FormShareMgr::FinishFreeInstallTask(const std::shared_ptr<FormFreeInstallOperator> &freeInstallOperator)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);

    int64_t eventId = 0;
    for (auto iter = freeInstallOperatorMap_.begin(); iter != freeInstallOperatorMap_.end(); iter++) {
        if (iter->second == freeInstallOperator) {
            eventId = iter->first;
            break;
        }
    }

    if (eventId != 0) {
        eventHandler_->RemoveEvent(MSG::FORM_PACKAGE_FREE_INSTALL_DELAY_MSG, eventId);
        freeInstallOperatorMap_.erase(eventId);
    }
}

void FormShareMgr::OnInstallFinished(const std::shared_ptr<FormFreeInstallOperator> &freeInstallOperator,
    const int resultCode, const std::string &formShareInfoKey)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    FinishFreeInstallTask(freeInstallOperator);

    if (resultCode != ERR_OK) {
        HILOG_ERROR("free install failed.");
        RemoveFormShareInfo(formShareInfoKey);
        return;
    }

    FormShareInfo info;
    {
        std::shared_lock<std::shared_timed_mutex> guard(mapMutex_);
        auto it = shareInfo_.find(formShareInfoKey);
        if (it != shareInfo_.end()) {
            info = it->second;
        } else {
            HILOG_ERROR("form share info is not find.");
            return;
        }
    }

    StartFormUser(info);
}

void FormShareMgr::HandleFormShareInfoTimeOut(const int64_t eventId)
{
    HILOG_DEBUG("%{public}s called, eventId:%{public}" PRId64 "", __func__, eventId);

    std::string formShareInfoKey;
    {
        std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
        auto it = eventMap_.find(eventId);
        if (it == eventMap_.end()) {
            HILOG_ERROR("event id is not find.");
            return;
        }
        formShareInfoKey = it->second;
        eventMap_.erase(eventId);
    }
    HILOG_DEBUG("form share info timeout, key: %{public}s", formShareInfoKey.c_str());

    RemoveFormShareInfo(formShareInfoKey);
}

void FormShareMgr::HandleFreeInstallTimeOut(const int64_t eventId)
{
    HILOG_DEBUG("%{public}s called, eventId:%{public}" PRId64 "", __func__, eventId);
    std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
    freeInstallOperatorMap_.erase(eventId);
}

bool FormShareMgr::AddProviderData(const FormItemInfo &info, WantParams &wantParams)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    auto formShareInfoKey = makeFormShareInfoKey(info);
    std::map<std::string, sptr<IInterface>> providerWantParams;
    {
        std::shared_lock<std::shared_timed_mutex> guard(mapMutex_);
        auto it = shareInfo_.find(formShareInfoKey);
        if (it == shareInfo_.end()) {
            HILOG_DEBUG("No shared provider data.");
            return true;
        }

        providerWantParams = it->second.providerData.GetParams();
    }

    for (auto iter = providerWantParams.begin(); iter != providerWantParams.end(); iter++) {
        if (wantParams.HasParam(iter->first)) {
            HILOG_ERROR("Provider defines form base data.");
            return false;
        }
    }

    for (auto iter = providerWantParams.begin(); iter != providerWantParams.end(); iter++) {
        wantParams.SetParam(iter->first, iter->second);
    }

    RemoveFormShareInfo(formShareInfoKey);
    return true;
}

/**
 * @brief Acquire share form data from form provider.
 * @param formId The Id of the from.
 * @param remoteDeviceId The device ID to share.
 * @param remoteObject Form provider proxy object.
 */
void FormShareMgr::ShareAcquireProviderFormInfo(const int64_t formId, const std::string &remoteDeviceId,
    const Want &want, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    int64_t requestCode = static_cast<int64_t>(want.GetLongParam(Constants::FORM_SHARE_REQUEST_CODE, 0));
    long connectId = want.GetLongParam(Constants::FORM_CONNECT_ID, 0);
    sptr<IFormProvider> formProviderProxy = iface_cast<IFormProvider>(remoteObject);

    if (formProviderProxy == nullptr) {
        FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
        SendResponse(requestCode, ERR_APPEXECFWK_FORM_COMMON_CODE);
        HILOG_ERROR("%{public}s fail, Failed to get formProviderProxy", __func__);
        return;
    }

    int error = formProviderProxy->ShareAcquireProviderFormInfo(formId, remoteDeviceId,
        FormSupplyCallback::GetInstance(), requestCode);
    if (error != ERR_OK) {
        SendResponse(requestCode, error);
        HILOG_ERROR("%{public}s fail, Failed to get acquire provider form info", __func__);
    }

    FormSupplyCallback::GetInstance()->RemoveConnection(connectId);
}

void FormShareMgr::AcquireFormProviderInfo(const int64_t formId, const std::string &remoteDeviceId,
    const AAFwk::WantParams &wantParams, const int64_t requestCode)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    FormRecord formRecord;
    bool isFormRecExist = FormDataMgr::GetInstance().GetFormRecord(formId, formRecord);
    if (!isFormRecExist) {
        HILOG_ERROR("form share info get formRecord failed.");
        SendResponse(requestCode, ERR_APPEXECFWK_FORM_GET_INFO_FAILED);
        return;
    }

    OHOS::DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = GetLocalDeviceInfo(formRecord.bundleName, deviceInfo);
    if (ret != ERR_OK) {
        HILOG_ERROR("AcquireFormProviderInfo Failed to get local device info.");
        SendResponse(requestCode, ERR_APPEXECFWK_FORM_GET_DEVICE_INFO_FAILED);
        return;
    }

    FormShareInfo formShareInfo;
    formShareInfo.formId = formRecord.formId;
    formShareInfo.formName = formRecord.formName;
    formShareInfo.bundleName = formRecord.bundleName;
    formShareInfo.moduleName = formRecord.moduleName;
    formShareInfo.abilityName = formRecord.abilityName;
    formShareInfo.formTempFlag = formRecord.formTempFlg;
    formShareInfo.dimensionId = formRecord.specification;
    formShareInfo.providerData = wantParams;
    formShareInfo.deviceId = std::string(deviceInfo.deviceId);
    formShareInfo.deviceName = std::string(deviceInfo.deviceName);

    if (formDmsClient_ == nullptr) {
        formDmsClient_ = std::make_shared<FormDistributedClient>();
    }
    int retval = formDmsClient_->ShareForm(remoteDeviceId, formShareInfo);
    if (retval != ERR_OK) {
        HILOG_ERROR("AcquireFormProviderInfo Failed to share form from DMS retval = %{public}d.", retval);
        SendResponse(requestCode, ERR_APPEXECFWK_FORM_DISTRIBUTED_SCHEDULE_FAILED);
        return;
    }
    SendResponse(requestCode, ERR_OK);
}

int FormShareMgr::GetLocalDeviceInfo(const std::string &bundleName, OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    auto &deviceManager = OHOS::DistributedHardware::DeviceManager::GetInstance();

    std::shared_ptr<MyDmInitCallback> deviceInitCallback_ = std::make_shared<MyDmInitCallback>();
    if (deviceInitCallback_ == nullptr) {
        HILOG_ERROR("FormShareMgr::GetLocalDeviceInfo MyDmInitCallback make_shared failed!");
        return ERR_INVALID_VALUE;
    }
    deviceManager.InitDeviceManager(bundleName, deviceInitCallback_);

    auto ret = deviceManager.GetLocalDeviceInfo(bundleName, deviceInfo);
    if (ret != ERR_OK) {
        HILOG_ERROR("FormShareMgr::GetLocalDeviceInfo Failed to get local device info.");
        return ret;
    }
    deviceManager.UnInitDeviceManager(bundleName);

    return ERR_OK;
}

void FormShareMgr::SendResponse(const int64_t requestCode, const int result)
{
    HILOG_DEBUG("FormMgrService SendResponse called, requestCode:%{public}" PRId64 " result:%{public}d",
        requestCode, result);
    std::unique_lock<std::shared_timed_mutex> guard(mapMutex_);
    auto iter = requestMap_.find(requestCode);
    if (iter == requestMap_.end()) {
        HILOG_DEBUG("No form shared request.");
        return ;
    }

    sptr<IFormHost> remoteFormHost = iface_cast<IFormHost>(iter->second);
    if (remoteFormHost == nullptr) {
        HILOG_ERROR("%{public}s fail, Failed to get form host proxy.", __func__);
        return;
    }
    remoteFormHost->OnShareFormResponse(requestCode, result);
    requestMap_.erase(requestCode);
}
}  // namespace AppExecFwk
}  // namespace OHOS
