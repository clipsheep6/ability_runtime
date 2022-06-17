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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_SHARE_MGR_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_SHARE_MGR_H

#include <map>
#include <shared_mutex>
#include <singleton.h>
#include "device_manager.h"
#include "event_handler.h"
#include "form_distributed_client.h"
#include "form_share_info.h"
#include "form_item_info.h"
#include "form_free_install_operator.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
/**
 * @class FormShareMgr
 * Form share manager.
 */
class FormShareMgr final : public DelayedRefSingleton<FormShareMgr>{
    DECLARE_DELAYED_REF_SINGLETON(FormShareMgr);
public:
    DISALLOW_COPY_AND_MOVE(FormShareMgr);

    inline void SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
    {
        eventHandler_ = handler;
    }
    /**
     * @brief Form share.
     * @param formId Indicates the unique id of form.
     * @param deviceId Indicates the device ID to share.
     * @param callerToken Host client.
     * @param requestCode the request code of this share form.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ShareForm(const int64_t formId, const std::string &deviceId, const sptr<IRemoteObject> &callerToken,
        const int64_t requestCode);
    int RecvFormShareInfoFromRemote(const FormShareInfo &info);
    /**
     * @brief Acquire share form data from form provider.
     * @param formId The Id of the from.
     * @param remoteDeviceId The device ID to share.
     * @param remoteObject Form provider proxy object.
     */
    void ShareAcquireProviderFormInfo(const int64_t formId, const std::string &remoteDeviceId, const Want &want,
        const sptr<IRemoteObject> &remoteObject);
    void AcquireFormProviderInfo(const int64_t formId, const std::string &remoteDeviceId,
        const AAFwk::WantParams &wantParams, const int64_t requestCode);
    bool AddProviderData(const FormItemInfo &info, WantParams &wantParams);
    void HandleFormShareInfoTimeOut(const int64_t eventId);
    void HandleFreeInstallTimeOut(const int64_t eventId);

    /**
     * @brief Free install was finished.
     * @param FormFreeInstallOperator is free install operator object.
     * @param resultCode is free install result.
     * @param formShareInfoKey is form share info key.
     */
    void OnInstallFinished(const std::shared_ptr<FormFreeInstallOperator> &freeInstallOperator,
        const int resultCode, const std::string &formShareInfoKey);
    void SendResponse(const int64_t requestCode, const int result);

    void PostAcquireFormProviderInfo(const int64_t formId, const std::string &remoteDeviceId,
        const AAFwk::WantParams &wantParams, const int64_t requestCode);
private:
    int GetLocalDeviceInfo(const std::string &bundleName, OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);
    std::string makeFormShareInfoKey(const FormItemInfo &info);
    std::string makeFormShareInfoKey(const FormShareInfo &info);
    void RemoveFormShareInfo(const std::string &formShareInfoKey);
    void FinishFreeInstallTask(const std::shared_ptr<FormFreeInstallOperator> &freeInstallOperator);
    bool IsExistFormPackage(const std::string &bundleName, const std::string &moduleName);
    bool CheckFormShareInfo(const FormShareInfo &info);
    void StartFormUser(const FormShareInfo &info);
    int HandleRecvFormShareInfoFromRemoteTask(const FormShareInfo &info);
private:
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::shared_ptr<FormDistributedClient> formDmsClient_ = nullptr;
    //map for <formShareInfoKey, FormShareInfo>
    std::map<std::string, FormShareInfo> shareInfo_;
    //map for <eventId, formShareInfoKey>
    std::map<int64_t, std::string> eventMap_;
    //map for <eventId, std::shared_ptr<FormFreeInstallOperator>>
    std::map<int64_t, std::shared_ptr<FormFreeInstallOperator>> freeInstallOperatorMap_;
    //map for <requestCode, formHostClient>
    std::map<int64_t, sptr<IRemoteObject>> requestMap_;
    mutable std::shared_timed_mutex mapMutex_ {};
};

class MyDmInitCallback final : public OHOS::DistributedHardware::DmInitCallback {
    void OnRemoteDied() override
    {}
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_SHARE_MGR_H
