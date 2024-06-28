/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dataobs_mgr_service.h"

#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include "string_ex.h"

#include "dataobs_mgr_errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"
#include "common_utils.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DataObsMgrService>::GetInstance().get());

DataObsMgrService::DataObsMgrService()
    : SystemAbility(DATAOBS_MGR_SERVICE_SA_ID, true),
      state_(DataObsServiceRunningState::STATE_NOT_START)
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();
    dataObsMgrInnerPref_ = std::make_shared<DataObsMgrInnerPref>();
}

DataObsMgrService::~DataObsMgrService()
{}

void DataObsMgrService::OnStart()
{
    if (state_ == DataObsServiceRunningState::STATE_RUNNING) {
        TAG_LOGI(AAFwkTag::DBOBSMGR, "Dataobs Manager Service has already started.");
        return;
    }
    if (!Init()) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "failed to init service.");
        return;
    }
    state_ = DataObsServiceRunningState::STATE_RUNNING;
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    if (!Publish(DelayedSingleton<DataObsMgrService>::GetInstance().get())) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Init Publish failed!");
        return;
    }

    TAG_LOGI(AAFwkTag::DBOBSMGR, "Dataobs Manager Service start success.");
}

bool DataObsMgrService::Init()
{
    handler_ = TaskHandlerWrap::GetFfrtHandler();
    return true;
}

void DataObsMgrService::OnStop()
{
    TAG_LOGI(AAFwkTag::DBOBSMGR, "stop service");
    handler_.reset();
    state_ = DataObsServiceRunningState::STATE_NOT_START;
}

DataObsServiceRunningState DataObsMgrService::QueryServiceState() const
{
    return state_;
}

int DataObsMgrService::RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObserver is nullptr, uri:%{public}s.",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_ is nullptr, uri:%{public}s.",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    int status;
    if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
        status = dataObsMgrInnerPref_->HandleRegisterObserver(uri, dataObserver);
    } else {
        status = dataObsMgrInner_->HandleRegisterObserver(uri, dataObserver);
    }

    if (status != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Observer register failed: %{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return status;
    }
    return NO_ERROR;
}

int DataObsMgrService::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObserver is nullptr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_ is nullptr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    int status;
    if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
        status = dataObsMgrInnerPref_->HandleUnregisterObserver(uri, dataObserver);
    } else {
        status = dataObsMgrInner_->HandleUnregisterObserver(uri, dataObserver);
    }

    if (status != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "Observer unregister failed: %{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return status;
    }
    return NO_ERROR;
}

int DataObsMgrService::NotifyChange(const Uri &uri)
{
    if (handler_ == nullptr) {
        TAG_LOGE(
            AAFwkTag::DBOBSMGR, "handler is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr || dataObsMgrInnerPref_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgr is nullptr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    {
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            TAG_LOGE(AAFwkTag::DBOBSMGR, "The number of task has reached the upper limit, uri:%{public}s",
                CommonUtils::Anonymous(uri.ToString()).c_str());
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    ChangeInfo changeInfo = { ChangeInfo::ChangeType::OTHER, { uri } };
    handler_->SubmitTask([this, uri, changeInfo]() {
        if (const_cast<Uri &>(uri).GetScheme() == SHARE_PREFERENCES) {
            dataObsMgrInnerPref_->HandleNotifyChange(uri);
        } else {
            dataObsMgrInner_->HandleNotifyChange(uri);
            dataObsMgrInnerExt_->HandleNotifyChange(changeInfo);
        }
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });

    return NO_ERROR;
}

Status DataObsMgrService::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObserver is nullptr, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_ is nullptr, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto innerUri = uri;
    return dataObsMgrInnerExt_->HandleRegisterObserver(innerUri, dataObserver, isDescendants);
}

Status DataObsMgrService::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObserver is nullptr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_ is nullptr, uri:%{public}s",
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto innerUri = uri;
    return dataObsMgrInnerExt_->HandleUnregisterObserver(innerUri, dataObserver);
}

Status DataObsMgrService::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObserver is nullptr");
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_ is nullptr");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    return dataObsMgrInnerExt_->HandleUnregisterObserver(dataObserver);
}

Status DataObsMgrService::DeepCopyChangeInfo(const ChangeInfo &src, ChangeInfo &dst) const
{
    dst = src;
    if (dst.size_ == 0) {
        return SUCCESS;
    }
    dst.data_ = new (std::nothrow) uint8_t[dst.size_];
    if (dst.data_ == nullptr) {
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    errno_t ret = memcpy_s(dst.data_, dst.size_, src.data_, src.size_);
    if (ret != EOK) {
        delete [] static_cast<uint8_t *>(dst.data_);
        dst.data_ = nullptr;
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    return SUCCESS;
}

Status DataObsMgrService::NotifyChangeExt(const ChangeInfo &changeInfo)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "handler is nullptr");
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "dataObsMgrInner_:%{public}d or dataObsMgrInnerExt_ is nullptr",
            dataObsMgrInner_ == nullptr);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }
    ChangeInfo changes;
    Status result = DeepCopyChangeInfo(changeInfo, changes);
    if (result != SUCCESS) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "copy data failed, changeType:%{public}ud, num of uris:%{public}zu, data is "
            "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return result;
    }

    {
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            TAG_LOGE(AAFwkTag::DBOBSMGR,
                "The number of task has reached the upper limit, changeType:%{public}ud, num of "
                "uris:%{public}zu, data is nullptr:%{public}d, size:%{public}ud",
                changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    handler_->SubmitTask([this, changes]() {
        dataObsMgrInnerExt_->HandleNotifyChange(changes);
        for (auto &uri : changes.uris_) {
            dataObsMgrInner_->HandleNotifyChange(uri);
        }
        delete [] static_cast<uint8_t *>(changes.data_);
        std::lock_guard<ffrt::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
    return SUCCESS;
}

int DataObsMgrService::Dump(int fd, const std::vector<std::u16string>& args)
{
    std::string result;
    Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "%{public}s, dprintf error.", __func__);
        return DATAOBS_HIDUMP_ERROR;
    }
    return SUCCESS;
}

void DataObsMgrService::Dump(const std::vector<std::u16string>& args, std::string& result) const
{
    auto size = args.size();
    if (size == 0) {
        ShowHelp(result);
        return;
    }

    std::string optionKey = Str16ToStr8(args[0]);
    if (optionKey != "-h") {
        result.append("error: unkown option.\n");
    }
    ShowHelp(result);
}

void DataObsMgrService::ShowHelp(std::string& result) const
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n");
}
}  // namespace AAFwk
}  // namespace OHOS
