/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "app_no_response_disposer.h"

#include <csignal>

#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_scheduler.h"
#include "errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
const std::string TASK_NAME_ANR = "ANR_TIME_OUT_TASK";

AppNoResponseDisposer::AppNoResponseDisposer(const int timeout): timeout_(timeout) {}

#ifdef SUPPORT_GRAPHICS
int AppNoResponseDisposer::DisposeAppNoRespose(int pid,
    const SetMissionClosure &task, const ShowDialogClosure &showDialogTask) const
 {
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(appScheduler, ERR_INVALID_VALUE);

    AppExecFwk::ApplicationInfo appInfo;
    if (appScheduler->GetApplicationInfoByProcessID(pid, appInfo) != ERR_OK) {
        HILOG_ERROR("Get application info failed.");
        return ERR_INVALID_VALUE;
    }

    auto ret = ExcuteANRSaveStackInfoTask(pid, task);
    if (ret != ERR_OK) {
        HILOG_ERROR("excute anr save stack info task failed.");
        return ret;
    }

    auto callback = [disposer = shared_from_this(), pid, bundleName = appInfo.bundleName]() {
        CHECK_POINTER(disposer);
        disposer->PostTimeoutTask(pid, bundleName);
        HILOG_WARN("user choose to kill no response app.");
    };

    showDialogTask(appInfo.labelId, appInfo.bundleName, callback);

    HILOG_INFO("DisposeAppNoRespose success.");
    return ERR_OK;
}
#else
int AppNoResponseDisposer::DisposeAppNoRespose(int pid, const SetMissionClosure &task) const
 {
    HILOG_INFO("DisposeAppNoRespose start.");
    auto ret = PostTimeoutTask(pid);
    if (ret != ERR_OK) {
        HILOG_ERROR("post anr timeout task failed.");
        return ret;
    }

    ret = ExcuteANRSaveStackInfoTask(pid, task);
    if (ret != ERR_OK) {
        HILOG_ERROR("excute anr save stack info task failed.");
        return ret;
    }

    HILOG_INFO("DisposeAppNoRespose success.");
    return ERR_OK;
}
#endif

int AppNoResponseDisposer::PostTimeoutTask(int pid, std::string bundleName) const
{
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(appScheduler, ERR_INVALID_VALUE);

    AppExecFwk::ApplicationInfo appInfo;
    if (appScheduler->GetApplicationInfoByProcessID(pid, appInfo) != ERR_OK) {
        HILOG_ERROR("Get application info failed.");
        return ERR_INVALID_VALUE;
    }

    // if callback process, check the process must be the same bundle name.
    if (!bundleName.empty() && appInfo.bundleName != bundleName) {
        HILOG_ERROR("this application is not exist.");
        return ERR_INVALID_VALUE;
    }

    auto timeoutTask = [pid]() {
        if (kill(pid, SIGKILL) != ERR_OK) {
            HILOG_ERROR("Kill app not response process failed.");
        }
        HILOG_WARN("send kill app not response process signal.");
    };
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER_AND_RETURN(abilityMgr, ERR_INVALID_VALUE);
    abilityMgr->GetEventHandler()->PostTask(timeoutTask, TASK_NAME_ANR, timeout_);

    return ERR_OK;
}

int AppNoResponseDisposer::ExcuteANRSaveStackInfoTask(int pid, const SetMissionClosure &task) const
{
    std::vector<sptr<IRemoteObject>> tokens;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    CHECK_POINTER_AND_RETURN(appScheduler, ERR_INVALID_VALUE);

    if (appScheduler->GetAbilityRecordsByProcessID(pid, tokens) != ERR_OK) {
        HILOG_ERROR("Get ability record failed.");
        return ERR_INVALID_VALUE;
    }

    task(tokens);

    if (kill(pid, SIGUSR1) != ERR_OK) {
        HILOG_ERROR("Send singal SIGUSR1 error.");
        return SEND_USR1_SIG_FAIL;
    }

    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
