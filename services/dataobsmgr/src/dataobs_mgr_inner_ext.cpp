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
#include "dataobs_mgr_inner_ext.h"

#include "data_ability_observer_stub.h"
#include "dataobs_mgr_errors.h"
#include "hilog_wrapper.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {

DataObsMgrInnerExt::DataObsMgrInnerExt() : root_(std::make_shared<Node>("root")) {}

DataObsMgrInnerExt::~DataObsMgrInnerExt() {}

Status DataObsMgrInnerExt::HandleRegisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver, bool isDescendants)
{
    if (dataObserver->AsObject() == nullptr) {
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<std::mutex> lock(nodeMutex_);
    auto deathRecipientRef = AddObsDeathRecipient(dataObserver->AsObject());
    if (deathRecipientRef == nullptr) {
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }

    std::vector<std::string> path = { uri.GetScheme(), uri.GetAuthority() };
    uri.GetPathSegments(path);
    if (root_ != nullptr && !root_->AddObserver(path, 0, Entry(dataObserver, deathRecipientRef, isDescendants))) {
        HILOG_ERROR("The number of subscribers for this uri : %{public}s has reached the upper limit.",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        RemoveObsDeathRecipient(dataObserver->AsObject());
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }
    return SUCCESS;
}

Status DataObsMgrInnerExt::HandleUnregisterObserver(Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver->AsObject() == nullptr) {
        HILOG_ERROR("dataObserver is null, uri : %{public}s has reached the upper limit.",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<std::mutex> lock(nodeMutex_);
    std::vector<std::string> path = { uri.GetScheme(), uri.GetAuthority() };
    uri.GetPathSegments(path);
    if (root_ != nullptr) {
        root_->RemoveObserver(path, 0, dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver->AsObject());
    return SUCCESS;
}

Status DataObsMgrInnerExt::HandleUnregisterObserver(sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver->AsObject() == nullptr) {
        HILOG_ERROR("dataObserver is null");
        return DATA_OBSERVER_IS_NULL;
    }
    std::lock_guard<std::mutex> lock(nodeMutex_);
    if (root_ != nullptr) {
        root_->RemoveObserver(dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver->AsObject(), true);
    return SUCCESS;
}

Status DataObsMgrInnerExt::HandleNotifyChange(const ChangeInfo &changeInfo)
{
    ObsMap changeRes;
    std::vector<std::string> path;
    {
        std::lock_guard<std::mutex> lock(nodeMutex_);
        for (auto &uri : changeInfo.uris_) {
            path.clear();
            path.emplace_back(uri.GetScheme());
            path.emplace_back(uri.GetAuthority());
            uri.GetPathSegments(path);
            root_->GetObs(path, 0, uri, changeRes);
        }
    }
    if (changeRes.empty()) {
        HILOG_ERROR("no obs for this uris, changeType:%{public}ud, num of uris:%{public}zu, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return NO_OBS_FOR_URI;
    }
    for (const auto &[obs, value] : changeRes) {
        if (obs != nullptr && !value.empty()) {
            obs->OnChangeExt({ changeInfo.changeType_, move(value), changeInfo.data_, changeInfo.size_ });
        }
    }

    return SUCCESS;
}

std::shared_ptr<DataObsMgrInnerExt::DeathRecipientRef> DataObsMgrInnerExt::AddObsDeathRecipient(
    const sptr<IRemoteObject> &dataObserver)
{
    auto it = obsRecipientRefs.find(dataObserver);
    if (it != obsRecipientRefs.end()) {
        if (std::numeric_limits<uint32_t>::max() - 1 < it->second->ref) {
            HILOG_ERROR("the num of observer reach max limit");
            return nullptr;
        }
    } else {
        std::weak_ptr<DataObsMgrInnerExt> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new DataObsCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto DataObsMgrInnerExt = thisWeakPtr.lock();
                if (DataObsMgrInnerExt) {
                    DataObsMgrInnerExt->OnCallBackDied(remote);
                }
            });
        dataObserver->AddDeathRecipient(deathRecipient);
        it = obsRecipientRefs.emplace(dataObserver, std::make_shared<DeathRecipientRef>(deathRecipient)).first;
    }
    HILOG_DEBUG("this observer will be added, sum:%{public}ud", it->second->ref.load(std::memory_order_relaxed));
    return it->second;
}

void DataObsMgrInnerExt::RemoveObsDeathRecipient(const sptr<IRemoteObject> &dataObserver, bool isForce)
{
    auto it = obsRecipientRefs.find(dataObserver);
    if (it == obsRecipientRefs.end()) {
        return;
    }

    if (isForce || it->second->ref <= 1) {
        HILOG_DEBUG("this observer deathRecipient will be remove, sum:%{public}ud",
            it->second->ref.load(std::memory_order_relaxed));
        dataObserver->RemoveDeathRecipient(it->second->deathRecipient);
        obsRecipientRefs.erase(it);
        return;
    }
}

void DataObsMgrInnerExt::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("this observer died");
    auto dataObserver = remote.promote();
    if (dataObserver == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(nodeMutex_);
    if (root_ != nullptr) {
        root_->RemoveObserver(dataObserver);
    }
    RemoveObsDeathRecipient(dataObserver, true);
}

DataObsMgrInnerExt::Node::Node(const std::string &name) : name_(name) {}

void DataObsMgrInnerExt::Node::GetObs(const std::vector<std::string> &path, uint32_t index, Uri &uri, ObsMap &obsRes)
{
    if (path.size() == index) {
        for (auto entry : entrys_) {
            obsRes.try_emplace(entry.observer, std::list<Uri>()).first->second.push_back(uri);
        }
        return;
    }

    for (const auto &entry : entrys_) {
        if (entry.isDescendants) {
            obsRes.try_emplace(entry.observer, std::list<Uri>()).first->second.push_back(uri);
        }
    }

    auto it = childrens_.find(path[index]);
    if (it == childrens_.end()) {
        return;
    }
    it->second->GetObs(path, ++index, uri, obsRes);

    return;
}

bool DataObsMgrInnerExt::Node::AddObserver(const std::vector<std::string> &path, uint32_t index, const Entry &entry)
{
    if (path.size() == index) {
        if (entrys_.size() >= OBS_NUM_MAX) {
            return false;
        }
        entry.deathRecipientRef->ref++;
        entrys_.emplace_back(entry);
        return true;
    }
    auto it = childrens_.try_emplace(path[index], std::make_shared<Node>(path[index])).first;
    return it->second->AddObserver(path, ++index, entry);
}

bool DataObsMgrInnerExt::Node::RemoveObserver(const std::vector<std::string> &path, uint32_t index,
    sptr<IDataAbilityObserver> dataObserver)
{
    if (index == path.size()) {
        entrys_.remove_if([dataObserver](const Entry &entry) {
            if (entry.observer->AsObject() != dataObserver->AsObject()) {
                return false;
            }
            entry.deathRecipientRef->ref--;
            return true;
        });
        return entrys_.empty() && childrens_.empty();
    }
    auto child = childrens_.find(path[index]);
    if (child != childrens_.end() && child->second->RemoveObserver(path, ++index, dataObserver)) {
        childrens_.erase(child);
    }
    return entrys_.empty() && childrens_.empty();
}

bool DataObsMgrInnerExt::Node::RemoveObserver(sptr<IRemoteObject> dataObserver)
{
    for (auto child = childrens_.begin(); child != childrens_.end();) {
        if (child->second->RemoveObserver(dataObserver)) {
            child = childrens_.erase(child);
        } else {
            child++;
        }
    }
    entrys_.remove_if([dataObserver](const Entry &entry) {
        if (entry.observer->AsObject() != dataObserver) {
            return false;
        }
        entry.deathRecipientRef->ref--;
        return true;
    });
    return entrys_.empty() && childrens_.empty();
}

inline bool DataObsMgrInnerExt::Node::RemoveObserver(sptr<IDataAbilityObserver> dataObserver)
{
    auto obs = dataObserver->AsObject();
    return obs != nullptr && RemoveObserver(obs);
}

} // namespace AAFwk
} // namespace OHOS
