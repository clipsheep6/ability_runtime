/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "system_ability_definition.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {
const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<DataObsMgrService>::GetInstance().get());

DataObsMgrService::DataObsMgrService()
    : SystemAbility(DATAOBS_MGR_SERVICE_SA_ID, true),
      eventLoop_(nullptr),
      handler_(nullptr),
      state_(DataObsServiceRunningState::STATE_NOT_START)
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();
}

DataObsMgrService::~DataObsMgrService()
{}

void DataObsMgrService::OnStart()
{
    if (state_ == DataObsServiceRunningState::STATE_RUNNING) {
        HILOG_INFO("Dataobs Manager Service has already started.");
        return;
    }
    if (!Init()) {
        HILOG_ERROR("failed to init service.");
        return;
    }
    state_ = DataObsServiceRunningState::STATE_RUNNING;
    eventLoop_->Run();
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    if (!Publish(DelayedSingleton<DataObsMgrService>::GetInstance().get())) {
        HILOG_ERROR("Init Publish failed!");
        return;
    }

    HILOG_INFO("Dataobs Manager Service start success.");
}

bool DataObsMgrService::Init()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("DataObsMgrService");
    if (eventLoop_ == nullptr) {
        return false;
    }

    handler_ = std::make_shared<AppExecFwk::EventHandler>(eventLoop_);
    handler_->PostTask([]() { MemoryGuard cacheGuard; }, AppExecFwk::EventQueue::Priority::IMMEDIATE);

    return true;
}

void DataObsMgrService::OnStop()
{
    HILOG_INFO("stop service");
    eventLoop_.reset();
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
        HILOG_ERROR("dataObserver is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto status = dataObsMgrInner_->HandleRegisterObserver(uri, dataObserver);
    if (status != NO_ERROR) {
        HILOG_ERROR("Observer register failed: %{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    return NO_ERROR;
}

int DataObsMgrService::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        HILOG_ERROR("dataObserver is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto status = dataObsMgrInner_->HandleUnregisterObserver(uri, dataObserver);
    if (!status) {
        HILOG_ERROR("Observer unregister failed: %{public}d, uri:%{public}s", status,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    return NO_ERROR;
}

int DataObsMgrService::NotifyChange(const Uri &uri)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    {
        std::lock_guard<std::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            HILOG_ERROR("The number of task has reached the upper limit, uri:%{public}s",
                CommonUtils::Anonymous(uri.ToString()).c_str());
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    ChangeInfo changeInfo = { ChangeInfo::ChangeType::OTHER, { uri } };
    bool ret = handler_->PostTask([this, uri, changeInfo]() {
        dataObsMgrInner_->HandleNotifyChange(uri);
        dataObsMgrInnerExt_->HandleNotifyChange(changeInfo);
        std::lock_guard<std::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
    if (!ret) {
        HILOG_ERROR("Post NotifyChange fail, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }

    return NO_ERROR;
}

Status DataObsMgrService::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants)
{
    if (dataObserver == nullptr) {
        HILOG_ERROR("dataObserver is nullptr, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, uri:%{public}s, isDescendants:%{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto innerUri = uri;
    return dataObsMgrInnerExt_->HandleRegisterObserver(innerUri, dataObserver, isDescendants);
}

Status DataObsMgrService::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        HILOG_ERROR("dataObserver is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, uri:%{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    auto innerUri = uri;
    return dataObsMgrInnerExt_->HandleUnregisterObserver(innerUri, dataObserver);
}

Status DataObsMgrService::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver)
{
    if (dataObserver == nullptr) {
        HILOG_ERROR("dataObserver is nullptr");
        return DATA_OBSERVER_IS_NULL;
    }

    if (dataObsMgrInnerExt_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr");
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    return dataObsMgrInnerExt_->HandleUnregisterObserver(dataObserver);
}

Status DataObsMgrService::NotifyChangeExt(const ChangeInfo &changeInfo)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, changeType:%{public}ud, num of uris:%{public}zu, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return DATAOBS_SERVICE_HANDLER_IS_NULL;
    }

    if (dataObsMgrInner_ == nullptr || dataObsMgrInnerExt_ == nullptr) {
        HILOG_ERROR("dataObsMgrInner_ is nullptr, changeType:%{public}ud, num of uris:%{public}zu, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return DATAOBS_SERVICE_INNER_IS_NULL;
    }

    {
        std::lock_guard<std::mutex> lck(taskCountMutex_);
        if (taskCount_ >= TASK_COUNT_MAX) {
            HILOG_ERROR("The number of task has reached the upper limit, changeType:%{public}ud, num of "
                        "uris:%{public}zu, data is nullptr:%{public}d, size:%{public}ud",
                changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
            return DATAOBS_SERVICE_TASK_LIMMIT;
        }
        ++taskCount_;
    }

    bool ret = handler_->PostTask([this, changeInfo]() {
        dataObsMgrInnerExt_->HandleNotifyChange(changeInfo);
        for (auto &uri : changeInfo.uris_) {
            dataObsMgrInner_->HandleNotifyChange(uri);
        }
        std::lock_guard<std::mutex> lck(taskCountMutex_);
        --taskCount_;
    });
    if (!ret) {
        HILOG_ERROR("Post NotifyChangeExt fail, changeType:%{public}ud, num of uris:%{public}zu, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return DATAOBS_SERVICE_POST_TASK_FAILED;
    }
    return SUCCESS;
}

int DataObsMgrService::Dump(int fd, const std::vector<std::u16string>& args)
{
    std::string result;
    Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        HILOG_ERROR("%{public}s, dprintf error.", __func__);
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

std::shared_ptr<EventHandler> DataObsMgrService::GetEventHandler()
{
    return handler_;
}
}  // namespace AAFwk
}  // namespace OHOS
