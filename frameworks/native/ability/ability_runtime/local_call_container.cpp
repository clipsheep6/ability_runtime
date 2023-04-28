/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    const Want& want, const std::shared_ptr<CallerCallBack>& callback, const sptr<IRemoteObject>& callerToken)
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
        if (localCallRecord == nullptr) {
            HILOG_ERROR("LocalCallContainer::StartAbilityByCallInner Create local call record failed");
            return ERR_INVALID_VALUE;
        }
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
        HILOG_ERROR("LocalCallContainer::StartAbilityByCallInner abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    if (connect == nullptr) {
        HILOG_ERROR("LocalCallContainer::StartAbilityByCallInner Create local call connection failed");
        return ERR_INVALID_VALUE;
    }
    connections_.emplace(connect);
    connect->SetRecordAndContainer(localCallRecord, shared_from_this());
    HILOG_DEBUG("LocalCallContainer::StartAbilityByCallInner connections_.size is %{public}zu", connections_.size());
    auto retval = abilityClient->StartAbilityByCall(want, connect, callerToken);
    if (retval != ERR_OK) {
        ClearFailedCallConnection(callback);
    }
    return retval;
}

int LocalCallContainer::ReleaseCall(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("LocalCallContainer::ReleaseCall begin.");
    if (callback == nullptr) {
        HILOG_ERROR("LocalCallContainer::ReleaseCall input params is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("LocalCallContainer::ReleaseCall abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto localCallRecord = callback->GetRecord();
    if (localCallRecord == nullptr) {
        HILOG_ERROR("LocalCallContainer::ReleaseCall abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    localCallRecord->RemoveCaller(callback);
    if (localCallRecord->IsExistCallBack()) {
        // just release callback.
        HILOG_DEBUG("LocalCallContainer::ReleaseCall, The callee has onther callers, just release this callback.");
        return ERR_OK;
    }
    auto connect = iface_cast<CallerConnection>(localCallRecord->GetConnection());
    if (connect == nullptr) {
        HILOG_ERROR("LocalCallContainer::ReleaseCall connection conversion failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t retval = ERR_OK;
    if (localCallRecord->IsSingletonRemote()) {
        retval = RemoveSingletonCallLocalRecord(localCallRecord->GetElementName().GetURI());
    } else {
        retval = RemoveMultipleCallLocalRecord(localCallRecord);
    }

    if (retval != ERR_OK) {
        HILOG_ERROR("Remove call local record failed");
        return retval;
    }

    connections_.erase(connect);
    if (abilityClient->ReleaseCall(connect, localCallRecord->GetElementName()) != ERR_OK) {
        HILOG_ERROR("ReleaseCall failed.");
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("LocalCallContainer::ReleaseCall end.");
    return ERR_OK;
}

void LocalCallContainer::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback)
{
    HILOG_DEBUG("LocalCallContainer::ClearFailedCallConnection called");
    if (callback == nullptr) {
        HILOG_ERROR("LocalCallContainer::ClearFailedCallConnection callback is nullptr");
        return;
    }

    auto localCallRecord = callback->GetRecord();
    if (localCallRecord == nullptr) {
        HILOG_ERROR("LocalCallContainer::ClearFailedCallConnection localCallRecord is nullptr");
        return;
    }

    auto connect = iface_cast<CallerConnection>(localCallRecord->GetConnection());
    if (connect == nullptr) {
        HILOG_ERROR("LocalCallContainer::ClearFailedCallConnection connection conversion failed.");
        return;
    }

    connections_.erase(connect);
}

int32_t LocalCallContainer::RemoveSingletonCallLocalRecord(const std::string &uri)
{
    callProxyRecords_.erase(uri);
    return ERR_OK;
}

int32_t LocalCallContainer::RemoveMultipleCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record)
{
    if (record == nullptr) {
        HILOG_ERROR("input params invalid value");
        return ERR_INVALID_VALUE;
    }

    auto iterRecord = multipleCallProxyRecords_.find(record->GetElementName().GetURI());
    if (iterRecord == multipleCallProxyRecords_.end()) {
        HILOG_ERROR("release record in multiple not found.");
        return ERR_INVALID_VALUE;
    }

    iterRecord->second.erase(record);
    if (iterRecord->second.empty()) {
        multipleCallProxyRecords_.erase(iterRecord);
    }

    return ERR_OK;
}

void LocalCallContainer::DumpCalls(std::vector<std::string>& info) const
{
    HILOG_DEBUG("LocalCallContainer::DumpCalls called.");
    info.emplace_back("          caller connections:");
    for (auto iter = callProxyRecords_.begin(); iter != callProxyRecords_.end(); iter++) {
        std::string tempstr = "            LocalCallRecord";
        tempstr += " ID #" + std::to_string(iter->second->GetRecordId()) + "\n";
        tempstr += "              callee";
        tempstr += " uri[" + iter->first + "]" + "\n";
        tempstr += "              callers #" + std::to_string(iter->second->GetCallers().size());
        bool flag = true;
        for (auto& callBack : iter->second->GetCallers()) {
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

bool LocalCallContainer::GetCallLocalRecord(
    const AppExecFwk::ElementName& elementName, std::shared_ptr<LocalCallRecord>& localCallRecord)
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

void LocalCallContainer::OnCallStubDied(const wptr<IRemoteObject>& remote)
{
    auto diedRemote = remote.promote();
    auto isExist = [&diedRemote](auto& record) {
        return record.second->IsSameObject(diedRemote);
    };

    auto iter = std::find_if(callProxyRecords_.begin(), callProxyRecords_.end(), isExist);
    if (iter != callProxyRecords_.end() && iter->second != nullptr) {
        iter->second->OnCallStubDied(remote);
        callProxyRecords_.erase(iter);
        return;
    }

    auto isMultipleExit = [&diedRemote] (auto& record) {
        return record->IsSameObject(diedRemote);
    };
    for (auto &item : multipleCallProxyRecords_) {
        HILOG_DEBUG("LocalCallContainer::OnCallStubDied multiple key[%{public}s].", item.first.c_str());
        auto iterMultiple = find_if(item.second.begin(), item.second.end(), isMultipleExit);
        if (iterMultiple == item.second.end()) {
            continue;
        }
        HILOG_DEBUG("LocalCallContainer::OnCallStubDied multiple key[%{public}s]. notify died event",
            item.first.c_str());
        (*iterMultiple)->OnCallStubDied(remote);
        item.second.erase(iterMultiple);
        if (item.second.empty()) {
            HILOG_DEBUG("LocalCallContainer::OnCallStubDied multiple key[%{public}s] empty.", item.first.c_str());
            multipleCallProxyRecords_.erase(item.first);
            break;
        }
    }
    HILOG_DEBUG("LocalCallContainer::OnCallStubDied end.");
}

void LocalCallContainer::SetCallLocalRecord(
    const AppExecFwk::ElementName& element, const std::shared_ptr<LocalCallRecord> &localCallRecord)
{
    HILOG_DEBUG("LocalCallContainer::SetCallLocalRecord called uri is %{private}s.", element.GetURI().c_str());
    const std::string strKey = element.GetURI();
    callProxyRecords_.emplace(strKey, localCallRecord);
}

void LocalCallContainer::SetMultipleCallLocalRecord(
    const AppExecFwk::ElementName& element, const std::shared_ptr<LocalCallRecord> &localCallRecord)
{
    HILOG_DEBUG("LocalCallContainer::SetMultipleCallLocalRecord called uri is %{private}s.", element.GetURI().c_str());
    const std::string strKey = element.GetURI();
    auto iter = multipleCallProxyRecords_.find(strKey);
    if (iter == multipleCallProxyRecords_.end()) {
        std::set<std::shared_ptr<LocalCallRecord>> records = { localCallRecord };
        multipleCallProxyRecords_.emplace(strKey, records);
        return;
    }

    iter->second.emplace(localCallRecord);
}

void CallerConnection::SetRecordAndContainer(const std::shared_ptr<LocalCallRecord> &localCallRecord,
    const std::weak_ptr<LocalCallContainer> &container)
{
    if (localCallRecord == nullptr) {
        HILOG_DEBUG("CallerConnection::SetRecordAndContainer input param is nullptr.");
        return;
    }
    localCallRecord_ = localCallRecord;
    container_ = container;
    localCallRecord_->SetConnection(this->AsObject());
}

void CallerConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int code)
{
    HILOG_DEBUG("CallerConnection::OnAbilityConnectDone start %{public}s .", element.GetURI().c_str());
    auto container = container_.lock();
    if (container == nullptr || localCallRecord_ == nullptr) {
        HILOG_ERROR("CallerConnection::OnAbilityConnectDone container or record is nullptr.");
        return;
    }

    const bool isSingleton = (code == static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON));
    localCallRecord_->SetIsSingleton(isSingleton);

    auto callRecipient = new (std::nothrow) CallRecipient(
        std::bind(&LocalCallContainer::OnCallStubDied, container, std::placeholders::_1));
    localCallRecord_->SetRemoteObject(remoteObject, callRecipient);

    if (isSingleton) {
        container->SetCallLocalRecord(element, localCallRecord_);
    } else {
        container->SetMultipleCallLocalRecord(element, localCallRecord_);
    }

    localCallRecord_->InvokeCallBack();
    HILOG_DEBUG("CallerConnection::OnAbilityConnectDone end. code:%{public}d.", code);
    return;
}

void CallerConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int code)
{
    HILOG_DEBUG("CallerConnection::OnAbilityDisconnectDone start %{public}s %{public}d.",
        element.GetURI().c_str(), code);
}

void CallerConnection::OnRemoteStateChanged(const AppExecFwk::ElementName &element, int32_t abilityState)
{
    HILOG_DEBUG("CallerConnection::OnRemoteStateChanged start %{public}s .", element.GetURI().c_str());
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (localCallRecord_ == nullptr) {
        HILOG_DEBUG("local call record is nullptr.");
        return;
    }

    localCallRecord_->NotifyRemoteStateChanged(abilityState);

    HILOG_DEBUG("CallerConnection::OnRemoteStateChanged end. abilityState:%{public}d.", abilityState);
    return;
}
} // namespace AbilityRuntime
} // namespace OHOS
