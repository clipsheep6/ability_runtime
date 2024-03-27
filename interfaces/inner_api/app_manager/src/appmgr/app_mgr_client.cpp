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

#include "app_mgr_client.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"

#include "app_mgr_interface.h"
#include "app_service_manager.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "app_mem_info.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrRemoteHolder : public std::enable_shared_from_this<AppMgrRemoteHolder> {
public:
    AppMgrRemoteHolder() = default;

    virtual ~AppMgrRemoteHolder() = default;

    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        serviceManager_ = std::move(serviceMgr);
    }

    AppMgrResultCode ConnectAppMgrService()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return ConnectAppMgrServiceInner();
    }

    sptr<IRemoteObject> GetRemoteObject()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!remote_) {
            (void) ConnectAppMgrServiceInner();
        }
        return remote_;
    }

private:
    void HandleRemoteDied(const wptr<IRemoteObject>& remote)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!remote_) {
            return;
        }

        if (remote_ == remote.promote()) {
            remote_->RemoveDeathRecipient(deathRecipient_);
            remote_ = nullptr;
        }
    }

    AppMgrResultCode ConnectAppMgrServiceInner()
    {
        if (!serviceManager_) {
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
        remote_ = serviceManager_->GetAppMgrService();
        if (!remote_) {
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }

        auto me = shared_from_this();
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new AppMgrDeathRecipient(me));
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "%{public}s :Failed to create AppMgrDeathRecipient!", __func__);
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
        if ((remote_->IsProxyObject()) && (!remote_->AddDeathRecipient(deathRecipient_))) {
            TAG_LOGE(AAFwkTag::APPMGR, "%{public}s :Add death recipient to AppMgrService failed.", __func__);
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }

        return AppMgrResultCode::RESULT_OK;
    }

    class AppMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AppMgrDeathRecipient(const std::shared_ptr<AppMgrRemoteHolder>& holder) : owner_(holder) {}

        virtual ~AppMgrDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject>& remote) override
        {
            std::shared_ptr<AppMgrRemoteHolder> holder = owner_.lock();
            if (holder) {
                holder->HandleRemoteDied(remote);
            }
        }

    private:
        std::weak_ptr<AppMgrRemoteHolder> owner_;
    };

private:
    std::unique_ptr<AppServiceManager> serviceManager_;
    sptr<IRemoteObject> remote_;
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};

AppMgrClient::AppMgrClient()
{
    SetServiceManager(std::make_unique<AppServiceManager>());
}

AppMgrClient::~AppMgrClient()
{}

AppMgrResultCode AppMgrClient::LoadAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    const AbilityInfo &abilityInfo, const ApplicationInfo &appInfo, const AAFwk::Want &want,
    int32_t abilityRecordId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            // From here, separate AbilityInfo and ApplicationInfo from AA.
            std::shared_ptr<AbilityInfo> abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
            std::shared_ptr<ApplicationInfo> appInfoPtr = std::make_shared<ApplicationInfo>(appInfo);
            std::shared_ptr<AAFwk::Want> wantPtr = std::make_shared<AAFwk::Want>(want);
            amsService->LoadAbility(token, preToken, abilityInfoPtr, appInfoPtr, wantPtr, abilityRecordId);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->TerminateAbility(token, clearMissionFlag);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateAbilityState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->UpdateExtensionState(token, state);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->RegisterAppStateCallback(callback);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const int32_t visibility, const int32_t perceptibility,
    const int32_t connectionState)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessByAbilityToken(token);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillProcessesByUserId(int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->KillProcessesByUserId(userId);
            return AppMgrResultCode::RESULT_OK;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->UpdateApplicationInfoInstalled(bundleName, uid);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplication(const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplication(bundleName);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplicationByUid(bundleName, uid);
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::KillApplicationSelf()
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            int32_t result = amsService->KillApplicationSelf();
            if (result == ERR_OK) {
                return AppMgrResultCode::RESULT_OK;
            }
            return AppMgrResultCode::ERROR_KILL_APPLICATION;
        }
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationData(const std::string &bundleName, const int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->ClearUpApplicationData(bundleName, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationDataBySelf(int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->ClearUpApplicationDataBySelf(userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRunningProcesses(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetProcessRunningInfosByUserId(info, userId);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetProcessRunningInformation(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetAllRenderProcesses(info);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::NotifyMemoryLevel(MemoryLevel level)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());

    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->NotifyMemoryLevel(level));
}

AppMgrResultCode AppMgrClient::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "DumpHeapMemory: service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->DumpHeapMemory(pid, mallocInfo));
}

AppMgrResultCode AppMgrClient::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "DumpJsHeapMemory: service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return AppMgrResultCode(service->DumpJsHeapMemory(info));
}

AppMgrResultCode AppMgrClient::GetConfiguration(Configuration& config)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->GetConfiguration(config);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::ConnectAppMgrService()
{
    if (mgrHolder_) {
        return mgrHolder_->ConnectAppMgrService();
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
}

void AppMgrClient::SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
{
    if (!mgrHolder_) {
        mgrHolder_ = std::make_shared<AppMgrRemoteHolder>();
    }
    mgrHolder_->SetServiceManager(std::move(serviceMgr));
}

void AppMgrClient::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->AbilityAttachTimeOut(token);
}

void AppMgrClient::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->PrepareTerminate(token);
}

void AppMgrClient::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->GetRunningProcessInfoByToken(token, info);
        }
    }
}

void AppMgrClient::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->GetRunningProcessInfoByPid(pid, info);
        }
    }
}

void AppMgrClient::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        sptr<IAmsMgr> amsService = service->GetAmsMgr();
        if (amsService != nullptr) {
            amsService->SetAbilityForegroundingFlagToAppRecord(pid);
        }
    }
}

void AppMgrClient::AddAbilityStageDone(const int32_t recordId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->AddAbilityStageDone(recordId);
}

void AppMgrClient::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->StartupResidentProcess(bundleInfos);
}

int AppMgrClient::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->StartUserTestProcess(want, observer, bundleInfo, userId);
}

int AppMgrClient::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->FinishUserTest(msg, resultCode, bundleName);
}

void AppMgrClient::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->StartSpecifiedAbility(want, abilityInfo);
}

void AppMgrClient::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->StartSpecifiedProcess(want, abilityInfo);
}

void AppMgrClient::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->RegisterStartSpecifiedAbilityResponse(response);
}

void AppMgrClient::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->ScheduleAcceptWantDone(recordId, want, flag);
}

void AppMgrClient::ScheduleNewProcessRequest(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return;
    }

    service->ScheduleNewProcessRequestDone(recordId, want, flag);
}

AppMgrResultCode AppMgrClient::UpdateConfiguration(const Configuration &config)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    service->UpdateConfiguration(config);
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->RegisterConfigurationObserver(observer);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

AppMgrResultCode AppMgrClient::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        int32_t result = service->UnregisterConfigurationObserver(observer);
        if (result == ERR_OK) {
            return AppMgrResultCode::RESULT_OK;
        }
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int AppMgrClient::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }

    return service->GetAbilityRecordsByProcessID(pid, tokens);
}

int AppMgrClient::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService->GetApplicationInfoByProcessID(pid, application, debug);
}

int32_t AppMgrClient::StartNativeProcessForDebugger(const AAFwk::Want &want) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->StartNativeProcessForDebugger(want);
}

int AppMgrClient::PreStartNWebSpawnProcess()
{
    TAG_LOGI(AAFwkTag::APPMGR, "PreStartNWebSpawnProcess");

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->PreStartNWebSpawnProcess();
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int AppMgrClient::StartRenderProcess(const std::string &renderParam,
                                     int32_t ipcFd, int32_t sharedFd,
                                     int32_t crashFd, pid_t &renderPid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->StartRenderProcess(renderParam, ipcFd, sharedFd, crashFd,
                                           renderPid);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

void AppMgrClient::AttachRenderProcess(const sptr<IRenderScheduler> &renderScheduler)
{
    if (!renderScheduler) {
        TAG_LOGI(AAFwkTag::APPMGR, "renderScheduler is nullptr");
        return;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "AttachRenderProcess");
        service->AttachRenderProcess(renderScheduler->AsObject());
    }
}

int AppMgrClient::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service != nullptr) {
        return service->GetRenderProcessTerminationStatus(renderPid, status);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

sptr<IRemoteObject> AppMgrClient::GetRemoteObject()
{
    return mgrHolder_->GetRemoteObject();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AppMgrClient::BlockAppService()
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s", __func__);
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "service is nullptr");
        return AppMgrResultCode::ERROR_SERVICE_NOT_READY;
    }
    return service->BlockAppService();
}
#endif

void AppMgrClient::SetCurrentUserId(const int32_t userId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        return;
    }
    amsService->SetCurrentUserId(userId);
}

int32_t AppMgrClient::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }

    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService != nullptr) {
        return amsService->GetBundleNameByPid(pid, bundleName, uid);
    }
    return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
}

int32_t AppMgrClient::NotifyAppFault(const FaultData &faultData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyAppFault(faultData);
}

int32_t AppMgrClient::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyAppFaultBySA(faultData);
}

int32_t AppMgrClient::ChangeAppGcState(pid_t pid, int32_t state)
{
    if (mgrHolder_ == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->ChangeAppGcState(pid, state);
}

int32_t AppMgrClient::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->RegisterAppDebugListener(listener);
}

int32_t AppMgrClient::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->UnregisterAppDebugListener(listener);
}

int32_t AppMgrClient::AttachAppDebug(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->AttachAppDebug(bundleName);
}

int32_t AppMgrClient::DetachAppDebug(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->DetachAppDebug(bundleName);
}

int32_t AppMgrClient::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    if (!IsAmsServiceReady()) {
        HILOG_ERROR("App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->SetAppWaitingDebug(bundleName, isPersist);
}

int32_t AppMgrClient::CancelAppWaitingDebug()
{
    if (!IsAmsServiceReady()) {
        HILOG_ERROR("App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->CancelAppWaitingDebug();
}

int32_t AppMgrClient::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    if (!IsAmsServiceReady()) {
        HILOG_ERROR("App manager service is not ready.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->GetWaitingDebugApp(debugInfoList);
}

bool AppMgrClient::IsWaitingDebugApp(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        HILOG_ERROR("App manager service is not ready.");
        return false;
    }
    return amsService_->IsWaitingDebugApp(bundleName);
}

void AppMgrClient::ClearNonPersistWaitingDebugFlag()
{
    if (!IsAmsServiceReady()) {
        HILOG_ERROR("App manager service is not ready.");
        return;
    }
    amsService_->ClearNonPersistWaitingDebugFlag();
}

int32_t AppMgrClient::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (!IsAmsServiceReady()) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return amsService_->RegisterAbilityDebugResponse(response);
}

bool AppMgrClient::IsAttachDebug(const std::string &bundleName)
{
    if (!IsAmsServiceReady()) {
        return false;
    }
    return amsService_->IsAttachDebug(bundleName);
}

void AppMgrClient::SetAppAssertionPauseState(int32_t pid, bool flag)
{
    if (!IsAmsServiceReady()) {
        return;
    }
    amsService_->SetAppAssertionPauseState(pid, flag);
}

bool AppMgrClient::IsAmsServiceReady()
{
    if (mgrHolder_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "mgrHolder_ is nullptr.");
        return false;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App manager service is nullptr.");
        return false;
    }

    amsService_ = service->GetAmsMgr();
    if (amsService_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService_ is nullptr.");
        return false;
    }
    return true;
}

int32_t AppMgrClient::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterApplicationStateObserver(observer, bundleNameList);
}

int32_t AppMgrClient::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrClient::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyPageShow(token, pageStateData);
}

int32_t AppMgrClient::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->NotifyPageHide(token, pageStateData);
}

int32_t AppMgrClient::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterAppRunningStatusListener(listener);
}

int32_t AppMgrClient::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Listener is nullptr.");
        return ERR_INVALID_DATA;
    }

    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterAppRunningStatusListener(listener);
}

bool AppMgrClient::IsFinalAppProcess()
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return false;
    }
    return service->IsFinalAppProcess();
}

void AppMgrClient::ClearProcessByToken(sptr<IRemoteObject> token) const
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return;
    }
    sptr<IAmsMgr> amsService = service->GetAmsMgr();
    if (amsService == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsService is nullptr.");
        return;
    }
    amsService->ClearProcessByToken(token);
}

int32_t AppMgrClient::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->RegisterRenderStateObserver(observer);
}

int32_t AppMgrClient::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UnregisterRenderStateObserver(observer);
}

int32_t AppMgrClient::UpdateRenderState(pid_t renderPid, int32_t state)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->UpdateRenderState(renderPid, state);
}

int32_t AppMgrClient::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    sptr<IAppMgr> service = iface_cast<IAppMgr>(mgrHolder_->GetRemoteObject());
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Service is nullptr.");
        return AppMgrResultCode::ERROR_SERVICE_NOT_CONNECTED;
    }
    return service->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}
}  // namespace AppExecFwk
}  // namespace OHOS
