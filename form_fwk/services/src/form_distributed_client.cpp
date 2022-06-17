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

#include "form_distributed_client.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "hilog_wrapper.h"
#include "form_mgr_errors.h"

namespace OHOS {
namespace AppExecFwk {
static const std::u16string DMS_PROXY_INTERFACE_TOKEN = u"ohos.distributedschedule.accessToken";
void FormDistributedClient::GetDmsServiceProxy()
{
    HILOG_DEBUG("SHAREFORM:: func call");
    if(dmsProxy_ != nullptr){
        HILOG_DEBUG("samgr to get.");
        return;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOG_ERROR("fail to get samgr.");
        return;
    }
    dmsProxy_ = samgrProxy->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
}

int FormDistributedClient::ShareForm(
    const std::string &remoteDeviceId, const FormShareInfo &formShareInfo)
{
    HILOG_DEBUG("SHAREFORM:: func call");
    if (remoteDeviceId.empty()) {
        HILOG_ERROR("fail to input deviceId.");
        return ERR_APPEXECFWK_FORM_INVALID_PARAM;
    }

    GetDmsServiceProxy();
    if (dmsProxy_ == nullptr) {
        HILOG_ERROR("fail to get dmsProxy.");
        return ERR_APPEXECFWK_FORM_GET_DMS_PROXY_FAILED;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DMS_PROXY_INTERFACE_TOKEN)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(remoteDeviceId)) {
        HILOG_ERROR("WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&formShareInfo)) {
        HILOG_ERROR("WriteParcelable failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t error = dmsProxy_->SendRequest(START_REMOTE_SHARE_FORM, data, reply, option);
    if (error != ERR_NONE) {
        HILOG_ERROR("request failed, error: %{public}d", error);
        return error;
    }
    int32_t result = reply.ReadInt32();
    HILOG_DEBUG("get result from server data = %{public}d", result);
    return result;
}
/**
 * @brief Add the DmsProxy instance for debug.
 * @param dmsProxy the dmsProxy object.
 */
void FormDistributedClient::SetDmsProxy(const sptr<IRemoteObject> &dmsProxy)
{
    dmsProxy_ = dmsProxy;
}
}  // namespace AppExecFwk
}  // namespace OHOS
