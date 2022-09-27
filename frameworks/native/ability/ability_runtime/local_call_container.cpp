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

#include "local_call_container.h"

#include "hilog_wrapper.h"
#include "ability_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
int LocalCallContainer::StartAbilityByCallInner(
    const Want &want, const std::shared_ptr<CallerCallBack> &callback, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("start ability by call.");

    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (want.GetElement().GetBundleName().empty() ||
        want.GetElement().GetAbilityName().empty()) {
        HILOG_ERROR("the element of want is empty.");
        return ERR_INVALID_VALUE;
    }

    if (want.GetElement().GetDeviceID().empty()) {
        HILOG_DEBUG("start ability by call, element:DeviceID is empty");
    }

    HILOG_DEBUG("start ability by call, element:%{public}s", want.GetElement().GetURI().c_str());

    AppExecFwk::ElementName element = want.GetElement();
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (!GetCallLocalRecord(element, localCallRecord)) {
        localCallRecord = std::make_shared<LocalCallRecord>(element);
        if (!localCallRecord) {
            HILOG_ERROR("localCallRecord create fail.");
            return ERR_INVALID_VALUE;
        }
        std::string uri = element.GetURI();
        callProxyRecords_.emplace(uri, localCallRecord);
    }

    HILOG_DEBUG("start ability by call, localCallRecord->AddCaller(callback) begin");
    localCallRecord->AddCaller(callback);
    HILOG_DEBUG("start ability by call, localCallRecord->AddCaller(callback) end");

    auto remote = localCallRecord->GetRemoteObject();
    // already finish call request.
    if (remote) {
        HILOG_DEBUG("start ability by call, callback->InvokeCallBack(remote) begin");
        callback->InvokeCallBack(remote);
        HILOG_DEBUG("start ability by call, callback->InvokeCallBack(remote) end");
        if (!want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            return ERR_OK;
        }
    }
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("LocalCallContainer::Resolve abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(this->AsObject());
    HILOG_DEBUG("start ability by call, abilityClient->StartAbilityByCall call");
    return abilityClient->StartAbilityByCall(want, connect, callerToken);
}

int LocalCallContainer::ReleaseCall(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("LocalCallContainer::ReleaseCall begin.");
    auto isExist = [&callback](auto &record) {
        return record.second->RemoveCaller(callback);
    };

    auto iter = std::find_if(callProxyRecords_.begin(), callProxyRecords_.end(), isExist);
    if (iter == callProxyRecords_.end()) {
        HILOG_ERROR("release localCallRecord failed.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<LocalCallRecord> record = iter->second;
    if (record == nullptr) {
        HILOG_ERROR("record is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (record->IsExistCallBack()) {
        // just release callback.
        HILOG_DEBUG("LocalCallContainer::ReleaseCall, The callee has onther callers, just release this callback.");
        return ERR_OK;
    }

    // notify ams this connect need to release.
    AppExecFwk::ElementName elementName = record->GetElementName();
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("LocalCallContainer::Resolve abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(this->AsObject());
    if (abilityClient->ReleaseCall(connect, elementName) != ERR_OK) {
        HILOG_ERROR("ReleaseCall failed.");
        return ERR_INVALID_VALUE;
    }

    callProxyRecords_.erase(iter);
    HILOG_DEBUG("LocalCallContainer::ReleaseCall end.");
    return ERR_OK;
}

void LocalCallContainer::DumpCalls(std::vector<std::string> &info) const
{
    HILOG_DEBUG("LocalCallContainer::DumpCalls called.");
    info.emplace_back("          caller connections:");
    for (auto iter = callProxyRecords_.begin(); iter != callProxyRecords_.end(); iter++) {
        std::string tempstr = "            LocalCallRecord";
        tempstr += " ID #" + std::to_string (iter->second->GetRecordId()) + "\n";
        tempstr += "              callee";
        tempstr += " uri[" + iter->first + "]" + "\n";
        tempstr += "              callers #" + std::to_string (iter->second->GetCallers().size());
        bool flag = true;
        for (auto &callBack : iter->second->GetCallers()) {
            if (callBack != nullptr && !callBack->IsCallBack()) {
                HILOG_INFO("%{public}s call back is not called.", __func__);
                flag = false;
                break;
            }
        }
        if (flag) {
            HILOG_INFO("%{public}s state is REQUESTEND.", __func__);
            tempstr += "  state #REQUESTEND";
        } else {
            HILOG_INFO("%{public}s state is REQUESTING.", __func__);
            tempstr += "  state #REQUESTING";
        }
        info.emplace_back(tempstr);
    }
    return;
}

void LocalCallContainer::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("LocalCallContainer::OnAbilityConnectDone start %{public}s .", element.GetURI().c_str());
    if (resultCode != ERR_OK) {
        HILOG_ERROR("OnAbilityConnectDone failed.");
    }
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (GetCallLocalRecord(element, localCallRecord)) {
        auto callRecipient = new (std::nothrow) CallRecipient(
            std::bind(&LocalCallContainer::OnCallStubDied, this, std::placeholders::_1));
        if (callRecipient == nullptr) {
            HILOG_ERROR("callRecipient == nullptr");
            return;
        }
        localCallRecord->SetRemoteObject(remoteObject, callRecipient);
        localCallRecord->InvokeCallBack();
    }

    HILOG_DEBUG("LocalCallContainer::OnAbilityConnectDone end. resultCode:%{public}d.", resultCode);
    return;
}

void LocalCallContainer::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
}

bool LocalCallContainer::GetCallLocalRecord(
    const AppExecFwk::ElementName &elementName, std::shared_ptr<LocalCallRecord> &localCallRecord)
{
    for (auto pair : callProxyRecords_) {
        AppExecFwk::ElementName callElement;
        if (!callElement.ParseURI(pair.first)) {
            HILOG_ERROR("Parse uri to elementName failed, elementName uri: %{private}s", pair.first.c_str());
            continue;
        }
        // elementName in callProxyRecords_ has moduleName (sometimes not empty),
        // but the moduleName of input param elementName is usually empty.
        callElement.SetModuleName("");
        if ((pair.first == elementName.GetURI() || callElement.GetURI() == elementName.GetURI()) && pair.second) {
            localCallRecord = pair.second;
            return true;
        }
    }
    return false;
}

void LocalCallContainer::OnCallStubDied(const wptr<IRemoteObject> &remote)
{
    auto diedRemote = remote.promote();
    auto isExist = [&diedRemote](auto &record) {
        return record.second->IsSameObject(diedRemote);
    };

    auto iter = std::find_if(callProxyRecords_.begin(), callProxyRecords_.end(), isExist);
    if (iter == callProxyRecords_.end()) {
        HILOG_ERROR("StubDied object not found from localCallRecord.");
        return;
    }

    iter->second->OnCallStubDied(remote);
    callProxyRecords_.erase(iter);
    HILOG_DEBUG("LocalCallContainer::OnCallStubDied end.");
}
} // namespace AbilityRuntime
} // namespace OHOS
