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
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int LocalCallContainer::StartAbilityByCallInner(const Want& want, std::shared_ptr<CallerCallBack> callback,
    sptr<IRemoteObject> callerToken, int32_t accountId)
{
    AppExecFwk::ElementName element = want.GetElement();
    HILOG_DEBUG("start ability by call, element:%{public}s", element.GetURI().c_str());
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (element.GetBundleName().empty() || element.GetAbilityName().empty()) {
        HILOG_ERROR("the element of want is empty.");
        return ERR_INVALID_VALUE;
    }
    if (element.GetDeviceID().empty()) {
        HILOG_DEBUG("start ability by call, element:DeviceID is empty");
    }

    int32_t oriValidUserId = GetValidUserId(accountId);
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (!GetCallLocalRecord(element, localCallRecord, oriValidUserId)) {
        localCallRecord = std::make_shared<LocalCallRecord>(element);
        localCallRecord->SetUserId(oriValidUserId);
        HILOG_DEBUG("create local call record and set user id[%{public}d] to record", oriValidUserId);
    }
    localCallRecord->AddCaller(callback);
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
    sptr<CallerConnection> connect = new (std::nothrow) CallerConnection();
    if (connect == nullptr) {
        HILOG_ERROR("StartAbilityByCallInner Create local call connection failed");
        return ERR_INVALID_VALUE;
    }
    connections_.emplace(connect);
    connect->SetRecordAndContainer(localCallRecord, shared_from_this());
    HILOG_DEBUG("StartAbilityByCallInner connections_.size is %{public}zu", connections_.size());
    auto retval = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByCall(want, connect,
        callerToken, oriValidUserId);
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
        HILOG_ERROR("LocalCallContainer::ReleaseCall localCallRecord is nullptr");
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
        retval = RemoveSingletonCallLocalRecord(localCallRecord);
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

int32_t LocalCallContainer::RemoveSingletonCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (record == nullptr) {
        HILOG_ERROR("input params invalid value");
        return ERR_INVALID_VALUE;
    }

    auto iterRecord = callProxyRecords_.find(record->GetElementName().GetURI());
    if (iterRecord == callProxyRecords_.end()) {
        HILOG_ERROR("release record in singleton not found.");
        return ERR_INVALID_VALUE;
    }

    iterRecord->second.erase(record);
    if (iterRecord->second.empty()) {
        callProxyRecords_.erase(iterRecord);
    }

    return ERR_OK;
}

int32_t LocalCallContainer::RemoveMultipleCallLocalRecord(const std::shared_ptr<LocalCallRecord> &record)
{
    if (record == nullptr) {
        HILOG_ERROR("input params invalid value");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::mutex> lock(multipleMutex_);
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

bool LocalCallContainer::IsCallBackCalled(const std::vector<std::shared_ptr<CallerCallBack>> &callers) const
{
    for (auto& callBack : callers) {
        if (callBack != nullptr && !callBack->IsCallBack()) {
            HILOG_INFO("%{public}s call back is not called.", __func__);
            return false;
        }
    }

    return true;
}

void LocalCallContainer::DumpCalls(std::vector<std::string>& info)
{
    HILOG_DEBUG("LocalCallContainer::DumpCalls called.");
    info.emplace_back("          caller connections:");
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : callProxyRecords_) {
        for (auto &itemCall : item.second) {
            std::string tempstr = "            LocalCallRecord";
            tempstr += " ID #" + std::to_string(itemCall->GetRecordId()) + "\n";
            tempstr += "              callee";
            tempstr += " uri[" + item.first + "]" + "\n";
            tempstr += "              callers #" + std::to_string(itemCall->GetCallers().size());
            if (IsCallBackCalled(itemCall->GetCallers())) {
                HILOG_INFO("%{public}s state is REQUESTEND.", __func__);
                tempstr += "  state #REQUESTEND";
            } else {
                HILOG_INFO("%{public}s state is REQUESTING.", __func__);
                tempstr += "  state #REQUESTING";
            }
            info.emplace_back(tempstr);
            }
    }
    return;
}

bool LocalCallContainer::GetCallLocalRecord(
    const AppExecFwk::ElementName& elementName, std::shared_ptr<LocalCallRecord>& localCallRecord, int32_t accountId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    HILOG_DEBUG("Get call local record by %{public}s and id %{public}d", elementName.GetURI().c_str(), accountId);
    for (auto pair : callProxyRecords_) {
        AppExecFwk::ElementName callElement;
        if (!callElement.ParseURI(pair.first)) {
            HILOG_ERROR("Parse uri to elementName failed, elementName uri: %{private}s", pair.first.c_str());
            continue;
        }
        // elementName in callProxyRecords_ has moduleName (sometimes not empty),
        // but the moduleName of input param elementName is usually empty.
        callElement.SetModuleName("");
        if ((pair.first != elementName.GetURI() && callElement.GetURI() != elementName.GetURI())) {
            continue;
        }

        for (auto &itemCall : pair.second) {
            if (itemCall != nullptr && itemCall->GetUserId() == accountId) {
                localCallRecord = itemCall;
                return true;
            }
        }
    }
    return false;
}

void LocalCallContainer::OnCallStubDied(const wptr<IRemoteObject>& remote)
{
    auto diedRemote = remote.promote();
    auto isExist = [&diedRemote](auto& record) {
        return record->IsSameObject(diedRemote);
    };

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &item : callProxyRecords_) {
            auto iter = std::find_if(item.second.begin(), item.second.end(), isExist);
            if (iter == item.second.end()) {
                continue;
            }
            HILOG_DEBUG("LocalCallContainer::OnCallStubDied singleton key[%{public}s]. notify died event",
                item.first.c_str());
            (*iter)->OnCallStubDied(remote);
            item.second.erase(iter);
            if (item.second.empty()) {
                HILOG_DEBUG("LocalCallContainer::OnCallStubDied singleton key[%{public}s] empty.", item.first.c_str());
                callProxyRecords_.erase(item.first);
                break;
            }
        }
    }

    std::lock_guard<std::mutex> lock(multipleMutex_);
    for (auto &item : multipleCallProxyRecords_) {
        HILOG_DEBUG("LocalCallContainer::OnCallStubDied multiple key[%{public}s].", item.first.c_str());
        auto iterMultiple = find_if(item.second.begin(), item.second.end(), isExist);
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
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = callProxyRecords_.find(strKey);
    if (iter == callProxyRecords_.end()) {
        std::set<std::shared_ptr<LocalCallRecord>> records = { localCallRecord };
        callProxyRecords_.emplace(strKey, records);
        return;
    }

    iter->second.emplace(localCallRecord);
}

void LocalCallContainer::SetMultipleCallLocalRecord(
    const AppExecFwk::ElementName& element, const std::shared_ptr<LocalCallRecord> &localCallRecord)
{
    HILOG_DEBUG("LocalCallContainer::SetMultipleCallLocalRecord called uri is %{private}s.", element.GetURI().c_str());
    const std::string strKey = element.GetURI();
    std::lock_guard<std::mutex> lock(multipleMutex_);
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
    if (localCallRecord_ == nullptr) {
        HILOG_DEBUG("local call record is nullptr.");
        return;
    }

    localCallRecord_->NotifyRemoteStateChanged(abilityState);

    HILOG_DEBUG("CallerConnection::OnRemoteStateChanged end. abilityState:%{public}d.", abilityState);
    return;
}

int32_t LocalCallContainer::GetCurrentUserId()
{
    if (currentUserId_ == DEFAULT_INVAL_VALUE) {
        auto osAccount = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
        if (osAccount == nullptr) {
            HILOG_ERROR("LocalCallContainer::GetCurrentUserId get osAccount is nullptr.");
            return DEFAULT_INVAL_VALUE;
        }

        osAccount->GetOsAccountLocalIdFromProcess(currentUserId_);
        HILOG_DEBUG("LocalCallContainer::GetCurrentUserId called. %{public}d", currentUserId_);
    }

    return currentUserId_;
}

int32_t LocalCallContainer::GetValidUserId(int32_t accountId)
{
    HILOG_DEBUG("LocalCallContainer::GetValidUserId is %{public}d", accountId);
    if (accountId > 0 && accountId != GetCurrentUserId()) {
        return accountId;
    }

    return DEFAULT_INVAL_VALUE;
}
} // namespace AbilityRuntime
} // namespace OHOS
