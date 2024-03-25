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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H

#include "gmock/gmock.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "semaphore_ex.h"
#include "app_mgr_service_inner.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrServiceInner : public AppMgrServiceInner {
public:
    MockAppMgrServiceInner() : lock_(0)
    {}
    virtual ~MockAppMgrServiceInner()
    {}

    MOCK_METHOD6(LoadAbility,
        void(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
            std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<ApplicationInfo> appInfo,
            std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId));
    MOCK_METHOD2(AttachApplication, void(const pid_t pid, const sptr<IAppScheduler>& app));
    MOCK_METHOD1(ApplicationForegrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationBackgrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationTerminated, void(const int32_t recordId));
    MOCK_METHOD2(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD2(UpdateApplicationInfoInstalled, int(const std::string&, const int uid));
    MOCK_METHOD1(KillApplication, int32_t(const std::string& bundleName));
    MOCK_METHOD2(KillApplicationByUid, int(const std::string&, const int uid));
    MOCK_METHOD1(AbilityTerminated, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD4(ClearUpApplicationData, int32_t(const std::string&, const int32_t, const pid_t, int32_t userId));
    MOCK_METHOD3(ClearUpApplicationDataBySelf, int32_t(int32_t, pid_t, int32_t userId));
    MOCK_METHOD1(IsBackgroundRunningRestricted, int32_t(const std::string&));
    MOCK_METHOD1(GetAllRunningProcesses, int32_t(std::vector<RunningProcessInfo>&));
    MOCK_METHOD1(GetAllRenderProcesses, int32_t(std::vector<RenderProcessInfo>&));
    MOCK_METHOD1(RegisterAppStateCallback, void(const sptr<IAppStateCallback>& callback));
    MOCK_METHOD0(StopAllProcess, void());
    MOCK_CONST_METHOD0(QueryAppSpawnConnectionState, SpawnConnectionState());
    MOCK_CONST_METHOD2(AddAppDeathRecipient, void(const pid_t pid, const sptr<AppDeathRecipient>& appDeathRecipient));
    MOCK_METHOD1(KillProcessByAbilityToken, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(KillProcessesByUserId, void(int32_t userId));
    MOCK_METHOD5(AbilityBehaviorAnalysis,
        void(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken, const int32_t visibility,
            const int32_t perceptibility, const int32_t connectionState));
    MOCK_METHOD1(AddAbilityStageDone, void(const int32_t recordId));
    MOCK_METHOD0(GetConfiguration, std::shared_ptr<Configuration>());
    MOCK_METHOD2(IsSharedBundleRunning, bool(const std::string &bundleName, uint32_t versionCode));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));
    MOCK_METHOD3(StartChildProcess, int32_t(const pid_t hostPid, const std::string &srcEntry, pid_t &childPid));
    MOCK_METHOD1(GetChildProcessInfoForSelf, int32_t(ChildProcessInfo &info));
    MOCK_METHOD2(SetAppWaitingDebug, int32_t(const std::string &bundleName, bool isPersist));
    MOCK_METHOD0(CancelAppWaitingDebug, int32_t());
    MOCK_METHOD1(GetWaitingDebugApp, int32_t(std::vector<std::string> &debugInfoList));
    MOCK_METHOD1(IsWaitingDebugApp, bool(const std::string &bundleName));
    MOCK_METHOD0(ClearNonPersistWaitingDebugFlag, void());
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
    {}

    void Post()
    {
        if (currentCount_ > 1) {
            currentCount_--;
        } else {
            lock_.Post();
            currentCount_ = count_;
        }
    }
    // for mock function return int32_t
    int32_t Post4Int()
    {
        if (currentCount_ > 1) {
            currentCount_--;
        } else {
            lock_.Post();
            currentCount_ = count_;
        }
        return 0;
    }

    void Wait()
    {
        lock_.Wait();
    }

    void SetWaitCount(const int waitCount)
    {
        count_ = waitCount;
        currentCount_ = waitCount;
    }

    int32_t OpenAppSpawnConnection() override
    {
        return 0;
    }

private:
    Semaphore lock_;
    int32_t count_ = 1;
    int32_t currentCount_ = 1;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H
