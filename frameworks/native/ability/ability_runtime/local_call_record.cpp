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
#include "local_call_record.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t FOREGROUND = 2;
constexpr int32_t BACKGROUND = 4;
}
int64_t LocalCallRecord::callRecordId = 0;
LocalCallRecord::LocalCallRecord(const AppExecFwk::ElementName& elementName)
{
    recordId_ = callRecordId++;
    elementName_ = elementName;
}

LocalCallRecord::~LocalCallRecord()
{
    if (remoteObject_ && callRecipient_) {
        remoteObject_->RemoveDeathRecipient(callRecipient_);
    }
}

void LocalCallRecord::SetRemoteObject(const sptr<IRemoteObject>& call)
{
    if (call == nullptr) {
        HILOG_ERROR("remote object is nullptr");
        return;
    }

    remoteObject_ = call;
    if (callRecipient_ == nullptr) {
        auto self(weak_from_this());
        auto diedTask = [self](const wptr<IRemoteObject>& remote) {
            auto record = self.lock();
            if (record == nullptr) {
                HILOG_ERROR("LocalCallRecord is null, OnCallStubDied failed.");
                return;
            }
            record->OnCallStubDied(remote);
        };
        callRecipient_ = new CallRecipient(diedTask);
    }
    remoteObject_->AddDeathRecipient(callRecipient_);
    HILOG_DEBUG("SetRemoteObject complete.");
}

void LocalCallRecord::SetRemoteObject(const sptr<IRemoteObject>& call,
    sptr<IRemoteObject::DeathRecipient> callRecipient)
{
    if (call == nullptr) {
        HILOG_ERROR("remote object is nullptr");
        return;
    }

    remoteObject_ = call;
    callRecipient_ = callRecipient;

    remoteObject_->AddDeathRecipient(callRecipient_);
    HILOG_DEBUG("SetRemoteObject2 complete.");
}

void LocalCallRecord::AddCaller(const std::shared_ptr<CallerCallBack>& callback)
{
    std::shared_ptr<CallerCallBack> caller = callback;
    caller->SetRecord(weak_from_this());
    callers_.emplace_back(callback);
}

bool LocalCallRecord::RemoveCaller(const std::shared_ptr<CallerCallBack>& callback)
{
    if (callers_.empty()) {
        HILOG_ERROR("this caller vector is empty.");
        return false;
    }

    auto iter = std::find(callers_.begin(), callers_.end(), callback);
    if (iter != callers_.end()) {
        callback->InvokeOnRelease(ON_RELEASE);
        callers_.erase(iter);
        return true;
    }

    HILOG_ERROR("this caller callback can't find.");
    return false;
}

void LocalCallRecord::OnCallStubDied(const wptr<IRemoteObject>& remote)
{
    HILOG_DEBUG("OnCallStubDied.");
    for (auto& callBack : callers_) {
        if (callBack != nullptr) {
            HILOG_ERROR("invoke caller's OnRelease.");
            callBack->InvokeOnRelease(ON_DIED);
        }
    }
}

void LocalCallRecord::InvokeCallBack() const
{
    if (remoteObject_ == nullptr) {
        HILOG_ERROR("remote object is nullptr, can't callback.");
        return;
    }

    for (auto& callBack : callers_) {
        if (callBack != nullptr && !callBack->IsCallBack()) {
            callBack->InvokeCallBack(remoteObject_);
        }
    }
    HILOG_DEBUG("finish callback with remote object.");
}

void LocalCallRecord::NotifyRemoteStateChanged(int32_t abilityState)
{
    if (remoteObject_ == nullptr) {
        HILOG_ERROR("remote object is nullptr, can't notify.");
        return;
    }
    std::string state = "";
    if (abilityState == FOREGROUND) {
        state = "foreground";
    } else if (abilityState == BACKGROUND) {
        state = "background";
    }
    HILOG_DEBUG("NotifyRemoteStateChanged, state = %{public}s.", state.c_str());

    for (auto& callBack : callers_) {
        if (callBack != nullptr && callBack->IsCallBack()) {
            HILOG_INFO("callback is not nullptr, and is callbcak ");
            callBack->InvokeOnNotify(state);
        }
    }
    HILOG_DEBUG("finish notify remote state changed.");
}

sptr<IRemoteObject> LocalCallRecord::GetRemoteObject() const
{
    return remoteObject_;
}

AppExecFwk::ElementName LocalCallRecord::GetElementName() const
{
    return elementName_;
}

bool LocalCallRecord::IsExistCallBack() const
{
    return !callers_.empty();
}

int LocalCallRecord::GetRecordId() const
{
    return recordId_;
}

std::vector<std::shared_ptr<CallerCallBack>> LocalCallRecord::GetCallers() const
{
    return callers_;
}

bool LocalCallRecord::IsSameObject(const sptr<IRemoteObject>& remote) const
{
    if (remote == nullptr) {
        HILOG_ERROR("input remote object is nullptr");
        return false;
    }

    bool retVal = (remoteObject_ == remote);
    HILOG_DEBUG("LocalCallRecord::%{public}s the input object same as local object is %{public}s.",
        __func__, retVal ? "true" : "false");
    return retVal;
}

void LocalCallRecord::SetIsSingleton(bool flag)
{
    isSingleton_ = flag;
}

bool LocalCallRecord::IsSingletonRemote()
{
    return isSingleton_;
}

void LocalCallRecord::SetConnection(const sptr<IRemoteObject> &connect)
{
    connection_ = connect;
}

sptr<IRemoteObject> LocalCallRecord::GetConnection()
{
    return connection_.promote();
}
} // namespace AbilityRuntime
} // namespace OHOS
