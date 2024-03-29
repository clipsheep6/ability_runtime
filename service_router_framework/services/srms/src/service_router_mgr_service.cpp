/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "service_router_mgr_service.h"

#include <memory>
#include <string>

#include "ability_manager_client.h"
#include "app_log_wrapper.h"
#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "service_router_data_mgr.h"
#include "string_ex.h"
#include "sr_samgr_helper.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string NAME_SERVICE_ROUTER_MGR_SERVICE = "ServiceRouterMgrService";
const std::string TASK_NAME = "ServiceRouterUnloadTask";
const int64_t UNLOAD_DELAY_TIME = 90000;
}

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<ServiceRouterMgrService>::GetInstance().get());

ServiceRouterMgrService::ServiceRouterMgrService() : SystemAbility(SERVICE_ROUTER_MGR_SERVICE_ID, true)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRMS instance create");
}

ServiceRouterMgrService::~ServiceRouterMgrService()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRMS instance destroy");
}

void ServiceRouterMgrService::OnStart()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "SRMS starting...");
    Init();
    bool ret = Publish(this);
    if (!ret) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Publish SRMS failed!");
        return;
    }
    DelayUnloadTask();
    TAG_LOGI(AAFwkTag::SER_ROUTER, "SRMS start success.");
}

void ServiceRouterMgrService::OnStop()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Stop SRMS.");
}

void ServiceRouterMgrService::Init()
{
    LoadAllBundleInfos();
    InitEventRunnerAndHandler();
    SubscribeCommonEvent();
}

void ServiceRouterMgrService::DelayUnloadTask()
{
    if (handler_ == nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "DelayUnloadTask, handler_ is nullptr");
        return;
    }

    std::lock_guard<std::mutex> lock(delayTaskMutex_);
    handler_->RemoveTask(TASK_NAME);
    auto task = [this]() {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "UnloadSA start.");
        sptr<ISystemAbilityManager> saManager =
            OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saManager == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "UnloadSA, GetSystemAbilityManager is null.");
            return;
        }
        int32_t result = saManager->UnloadSystemAbility(OHOS::SERVICE_ROUTER_MGR_SERVICE_ID);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "UnloadSA, UnloadSystemAbility result: %{public}d", result);
            return;
        }
        TAG_LOGI(AAFwkTag::SER_ROUTER, "UnloadSA success.");
    };
    handler_->PostTask(task, TASK_NAME, UNLOAD_DELAY_TIME);
}

bool ServiceRouterMgrService::LoadAllBundleInfos()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "LoadAllBundleInfos start");
    bool ret = ServiceRouterDataMgr::GetInstance().LoadAllBundleInfos();
    TAG_LOGD(AAFwkTag::SER_ROUTER, "LoadAllBundleInfos end");
    return ret;
}

bool ServiceRouterMgrService::InitEventRunnerAndHandler()
{
    std::lock_guard<std::mutex> lock(mutex_);
    runner_ = EventRunner::Create(NAME_SERVICE_ROUTER_MGR_SERVICE);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "%{public}s fail, Failed to init due to create runner error", __func__);
        return false;
    }
    handler_ = std::make_shared<EventHandler>(runner_);
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "%{public}s fail, Failed to init due to create handler error", __func__);
        return false;
    }
    return true;
}

bool ServiceRouterMgrService::ServiceRouterMgrService::SubscribeCommonEvent()
{
    if (eventSubscriber_ != nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "subscribeCommonEvent already subscribed.");
        return true;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);

    eventSubscriber_ = std::make_shared<SrCommonEventSubscriber>(subscribeInfo);
    eventSubscriber_->SetEventHandler(handler_);
    if (!EventFwk::CommonEventManager::SubscribeCommonEvent(eventSubscriber_)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "subscribeCommonEvent subscribed failure.");
        return false;
    };
    TAG_LOGI(AAFwkTag::SER_ROUTER, "subscribeCommonEvent subscribed success.");
    return true;
}

int32_t ServiceRouterMgrService::QueryBusinessAbilityInfos(const BusinessAbilityFilter &filter,
    std::vector< BusinessAbilityInfo> &businessAbilityInfos)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "coldStart:");
    DelayUnloadTask();
    return ServiceRouterDataMgr::GetInstance().QueryBusinessAbilityInfos(filter, businessAbilityInfos);
}

int32_t ServiceRouterMgrService::QueryPurposeInfos(const Want &want, const std::string purposeName,
    std::vector<PurposeInfo> &purposeInfos)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "coldStart:");
    DelayUnloadTask();
    return ServiceRouterDataMgr::GetInstance().QueryPurposeInfos(want, purposeName, purposeInfos);
}

int32_t ServiceRouterMgrService::StartUIExtensionAbility(const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "StartUIExtensionAbility start:");
    DelayUnloadTask();
    return IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartUIExtensionAbility(sessionInfo, userId));
}

int32_t ServiceRouterMgrService::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "ConnectUIExtensionAbility start:");
    DelayUnloadTask();
    return IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->
        ConnectUIExtensionAbility(want, connect, sessionInfo, userId));
}
}  // namespace AbilityRuntime
}  // namespace OHOS