/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "call_container.h"

#include "ability_connect_callback_stub.h"
#include "ability_util.h"
#include "ability_manager_service.h"

namespace OHOS {
namespace AAFwk {
CallContainer::CallContainer()
{}

CallContainer::~CallContainer()
{
    std::for_each(deathRecipientMap_.begin(),
        deathRecipientMap_.end(),
        [&](RecipientMapType::reference recipient) {
            recipient.first->RemoveDeathRecipient(recipient.second);
        });

    deathRecipientMap_.clear();
    callRecordMap_.clear();
}

void CallContainer::AddCallRecord(const sptr<IAbilityConnection> & connect,
    const std::shared_ptr<CallRecord>& callRecord)
{
    CHECK_POINTER(callRecord);
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());

    auto iter = callRecordMap_.find(connect->AsObject());
    if (iter != callRecordMap_.end()) {
        RemoveConnectDeathRecipient(connect);
        callRecordMap_.erase(callRecordMap_.find(connect->AsObject()));
    }

    AddConnectDeathRecipient(connect);
    callRecord->SetConCallBack(connect);
    callRecordMap_.emplace(connect->AsObject(), callRecord);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Add call record to callcontainer, target: %{public}s",
        callRecord->GetTargetServiceName().GetURI().c_str());
}

std::shared_ptr<CallRecord> CallContainer::GetCallRecord(const sptr<IAbilityConnection> & connect) const
{
    CHECK_POINTER_AND_RETURN(connect, nullptr);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), nullptr);

    auto mapIter = callRecordMap_.find(connect->AsObject());
    if (mapIter != callRecordMap_.end()) {
        return mapIter->second;
    }

    return nullptr;
}

bool CallContainer::RemoveCallRecord(const sptr<IAbilityConnection> & connect)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call container release call record by callback.");
    CHECK_POINTER_AND_RETURN(connect, false);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), false);

    auto iter = callRecordMap_.find(connect->AsObject());
    if (iter != callRecordMap_.end()) {
        auto callrecord = iter->second;
        if (callrecord) {
            callrecord->SchedulerDisconnectDone();
        }
        RemoveConnectDeathRecipient(connect);
        callRecordMap_.erase(callRecordMap_.find(connect->AsObject()));
        TAG_LOGD(AAFwkTag::ABILITYMGR, "remove call record is success.");
        return true;
    }

    if (callRecordMap_.empty()) {
        // notify soft resouce service.
        TAG_LOGD(AAFwkTag::ABILITYMGR, "this ability has no callrecord.");
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "remove call record is not exist.");
    return false;
}

void CallContainer::OnConnectionDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGW(AAFwkTag::ABILITYMGR, "Call back is died.");
    auto object = remote.promote();
    CHECK_POINTER(object);

    std::shared_ptr<CallRecord> callRecord = nullptr;
    auto mapIter = callRecordMap_.find(object);
    if (mapIter != callRecordMap_.end()) {
        callRecord = mapIter->second;
    }

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    auto task = [abilityManagerService, callRecord]() {
        abilityManagerService->OnCallConnectDied(callRecord);
    };
    handler->SubmitTask(task);
}

bool CallContainer::CallRequestDone(const sptr<IRemoteObject> &callStub)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call Request Done start.");

    CHECK_POINTER_AND_RETURN(callStub, false);

    std::for_each(callRecordMap_.begin(),
        callRecordMap_.end(),
        [&callStub](CallMapType::reference service) {
            std::shared_ptr<CallRecord> callRecord = service.second;
            if (callRecord && callRecord->IsCallState(CallState::REQUESTING)) {
                callRecord->SetCallStub(callStub);
                callRecord->SchedulerConnectDone();
            }
        });

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call Request Done end.");
    return true;
}

void CallContainer::Dump(std::vector<std::string> &info) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Dump call records.");
    for (const auto &iter : callRecordMap_) {
        auto callRecord = iter.second;
        if (callRecord) {
            callRecord->Dump(info);
        }
    }
}

bool CallContainer::IsNeedToCallRequest() const
{
    for (const auto &iter : callRecordMap_) {
        auto callRecord = iter.second;
        if (callRecord && !callRecord->IsCallState(CallState::REQUESTED)) {
            return true;
        }
    }
    return false;
}

void CallContainer::AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "This death recipient has been added.");
        return;
    } else {
        std::weak_ptr<CallContainer> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto callContainer = thisWeakPtr.lock();
                if (callContainer) {
                    callContainer->OnConnectionDied(remote);
                }
            });
        if (!connect->AsObject()->AddDeathRecipient(deathRecipient)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient failed.");
        }
        deathRecipientMap_.emplace(connect->AsObject(), deathRecipient);
    }
}

void CallContainer::RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect)
{
    CHECK_POINTER(connect);
    CHECK_POINTER(connect->AsObject());
    auto it = deathRecipientMap_.find(connect->AsObject());
    if (it != deathRecipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        deathRecipientMap_.erase(it);
        return;
    }
}

bool CallContainer::IsExistConnection(const sptr<IAbilityConnection> &connect)
{
    return callRecordMap_.find(connect->AsObject()) != callRecordMap_.end();
}
}  // namespace AAFwk
}  // namesspace OHOS
