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
#include "dataobs_mgr_inner_pref.h"

#include "data_ability_observer_stub.h"
#include "dataobs_mgr_errors.h"
#include "hilog_wrapper.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {

DataObsMgrInnerPref::DataObsMgrInnerPref() {}

DataObsMgrInnerPref::~DataObsMgrInnerPref() {}

int DataObsMgrInnerPref::HandleRegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    std::lock_guard<std::mutex> lock(preferenceMutex_);

    auto [obsPair, flag] = observers_.try_emplace(uri.ToString(), std::list<sptr<IDataAbilityObserver>>());
    if (!flag && obsPair->second.size() > OBS_NUM_MAX) {
        HILOG_ERROR("The number of subscribers for this uri : %{public}s has reached the upper limit.",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_OBS_LIMMIT;
    }

    for (auto obs = obsPair->second.begin(); obs != obsPair->second.end(); obs++) {
        if ((*obs)->AsObject() == dataObserver->AsObject()) {
            HILOG_ERROR("the obs has registered on this uri : %{public}s",
                CommonUtils::Anonymous(uri.ToString()).c_str());
            return OBS_EXIST;
        }
    }

    obsPair->second.push_back(dataObserver);

    AddObsDeathRecipient(dataObserver);
    return NO_ERROR;
}

int DataObsMgrInnerPref::HandleUnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    std::lock_guard<std::mutex> lock(preferenceMutex_);

    auto obsPair = observers_.find(uri.ToString());
    if (obsPair == observers_.end()) {
        HILOG_WARN("no obs on this uri : %{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return NO_OBS_FOR_URI;
    }

    HILOG_DEBUG("obs num is %{public}zu on this uri : %{public}s", obsPair->second.size(),
        CommonUtils::Anonymous(uri.ToString()).c_str());
    auto obs = obsPair->second.begin();
    for (; obs != obsPair->second.end(); obs++) {
        if ((*obs)->AsObject() == dataObserver->AsObject()) {
            break;
        }
    }
    if (obs == obsPair->second.end()) {
        HILOG_WARN("no obs on this uri : %{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return NO_OBS_FOR_URI;
    }
    obsPair->second.remove(*obs);
    if (obsPair->second.empty()) {
        observers_.erase(obsPair);
    }

    if (!HaveRegistered(dataObserver)) {
        RemoveObsDeathRecipient(dataObserver->AsObject());
    }
    return NO_ERROR;
}

int DataObsMgrInnerPref::HandleNotifyChange(const Uri &uri)
{
    std::list<sptr<IDataAbilityObserver>> obsList;
    std::lock_guard<std::mutex> lock(preferenceMutex_);
    {
        std::string uriStr = uri.ToString();
        size_t pos = uriStr.find('?');
        if (pos == std::string::npos) {
            HILOG_WARN("the current uri is missing the query section : %{public}s",
                CommonUtils::Anonymous(uriStr).c_str());
            return INVALID_PARAM;
        }
        std::string observerKey = uriStr.substr(0, pos);
        auto obsPair = observers_.find(observerKey);
        if (obsPair == observers_.end()) {
            HILOG_DEBUG("there is no obs on the uri : %{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
            return NO_OBS_FOR_URI;
        }
        obsList = obsPair->second;
    }

    for (auto &obs : obsList) {
        if (obs != nullptr) {
            obs->OnChangePreferences(const_cast<Uri &>(uri).GetQuery());
        }
    }

    HILOG_DEBUG("called end on the uri : %{public}s,obs num: %{public}zu",
        CommonUtils::Anonymous(uri.ToString()).c_str(), obsList.size());
    return NO_ERROR;
}

void DataObsMgrInnerPref::AddObsDeathRecipient(sptr<IDataAbilityObserver> dataObserver)
{
    if ((dataObserver == nullptr) || dataObserver->AsObject() == nullptr) {
        return;
    }

    auto it = obsRecipient_.find(dataObserver->AsObject());
    if (it != obsRecipient_.end()) {
        HILOG_WARN("this death recipient has been added.");
        return;
    } else {
        std::weak_ptr<DataObsMgrInnerPref> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new DataObsCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto dataObsMgrInner = thisWeakPtr.lock();
                if (dataObsMgrInner) {
                    dataObsMgrInner->OnCallBackDied(remote);
                }
            });
        if (!dataObserver->AsObject()->AddDeathRecipient(deathRecipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
        obsRecipient_.emplace(dataObserver->AsObject(), deathRecipient);
    }
}

void DataObsMgrInnerPref::RemoveObsDeathRecipient(sptr<IRemoteObject> dataObserver)
{
    if (dataObserver == nullptr) {
        return;
    }

    auto it = obsRecipient_.find(dataObserver);
    if (it != obsRecipient_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        obsRecipient_.erase(it);
        return;
    }
}

void DataObsMgrInnerPref::OnCallBackDied(const wptr<IRemoteObject> &remote)
{
    auto dataObserver = remote.promote();
    if (dataObserver == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(preferenceMutex_);

    if (dataObserver == nullptr) {
        HILOG_ERROR("dataObserver is nullptr.");
        return;
    }

    RemoveObs(dataObserver);
}

void DataObsMgrInnerPref::RemoveObs(sptr<IRemoteObject> dataObserver)
{
    for (auto iter = observers_.begin(); iter != observers_.end();) {
        auto &obsList = iter->second;
        for (auto it = obsList.begin(); it != obsList.end(); it++) {
            if ((*it)->AsObject() == dataObserver) {
                HILOG_DEBUG("Erase an observer form list.");
                obsList.erase(it);
                break;
            }
        }
        if (obsList.size() == 0) {
            iter = observers_.erase(iter);
        } else {
            iter++;
        }
    }
    RemoveObsDeathRecipient(dataObserver);
}

bool DataObsMgrInnerPref::HaveRegistered(sptr<IDataAbilityObserver> dataObserver)
{
    for (auto &[key, value] : observers_) {
        auto obs = std::find(value.begin(), value.end(), dataObserver);
        if (obs != value.end()) {
            return true;
        }
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
