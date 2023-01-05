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

#include "appmgrserviceinnerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_mgr_service_inner.h"
#undef private

#include "ability_record.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << OFFSET_ZERO) | (ptr[1] << OFFSET_ONE) | (ptr[2] << OFFSET_TWO) | ptr[3];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AppMgrServiceInner* appMgrServiceInner = new AppMgrServiceInner();
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    appMgrServiceInner->LoadAbility(token, preToken, abilityInfo, appInfo, want);
    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo, appInfo);
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data));
    std::string processName(data, size);
    appMgrServiceInner->MakeProcessName(abilityInfo, appInfo, hapModuleInfo, appIndex, processName);
    appMgrServiceInner->MakeProcessName(appInfo, hapModuleInfo, processName);
    AbilityInfo abilityInfoObj;
    BundleInfo bundleInfo;
    appMgrServiceInner->GetBundleAndHapInfo(abilityInfoObj, appInfo, bundleInfo, hapModuleInfo, appIndex);
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    sptr<IAppScheduler> appScheduler = nullptr;
    appMgrServiceInner->AttachApplication(pid, appScheduler);
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->LaunchApplication(appRecord);
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    appMgrServiceInner->AddAbilityStageDone(recordId);
    appMgrServiceInner->ApplicationForegrounded(recordId);
    appMgrServiceInner->ApplicationBackgrounded(recordId);
    appMgrServiceInner->ApplicationTerminated(recordId);
    std::string bundleName(data, size);
    appMgrServiceInner->KillApplication(bundleName);
    int uid = static_cast<int>(GetU32Data(data));
    appMgrServiceInner->KillApplicationByUid(bundleName, uid);
    int userId = static_cast<int>(GetU32Data(data));
    appMgrServiceInner->KillApplicationByUserId(bundleName, userId);
    appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, userId);
    int32_t callerUid = static_cast<int32_t>(GetU32Data(data));
    pid_t callerPid = static_cast<pid_t>(GetU32Data(data));
    appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid);
    std::vector<RunningProcessInfo> info;
    appMgrServiceInner->GetAllRunningProcesses(info);
    appMgrServiceInner->GetProcessRunningInfosByUserId(info, userId);
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    appMgrServiceInner->NotifyMemoryLevel(level);
    appMgrServiceInner->StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want);
    appMgrServiceInner->GetAppRunningRecordByAbilityToken(token);
    appMgrServiceInner->AbilityTerminated(token);
    appMgrServiceInner->GetAppRunningRecordByAppRecordId(recordId);
    ApplicationState state = ApplicationState::APP_STATE_BEGIN;
    bool needNotifyApp = *data % ENABLE;
    appMgrServiceInner->OnAppStateChanged(appRecord, state, needNotifyApp);
    std::shared_ptr<AbilityRunningRecord> ability;
    AppExecFwk::AbilityState abilityState = AppExecFwk::AbilityState::ABILITY_STATE_BEGIN;
    appMgrServiceInner->OnAbilityStateChanged(ability, abilityState);
    std::string appName(data, size);
    uint32_t startFlags = GetU32Data(data);
    int32_t bundleIndex = static_cast<int32_t>(GetU32Data(data));
    appMgrServiceInner->StartProcess(appName, processName, startFlags, appRecord, uid, bundleName, bundleIndex);
    appMgrServiceInner->RemoveAppFromRecentList(appName, processName);
    wptr<IRemoteObject> remote = nullptr;
    bool isRenderProcess = *data % ENABLE;
    appMgrServiceInner->OnRemoteDied(remote, isRenderProcess);
    bool containsApp = *data % ENABLE;
    appMgrServiceInner->ClearAppRunningData(appRecord, containsApp);
    appMgrServiceInner->PushAppFront(recordId);
    appMgrServiceInner->RemoveAppFromRecentListById(recordId);
    appMgrServiceInner->AddAppToRecentList(appName, processName, pid, recordId);
    appMgrServiceInner->GetAppTaskInfoById(recordId);
    sptr<AppDeathRecipient> appDeathRecipient = nullptr;
    appMgrServiceInner->AddAppDeathRecipient(pid, appDeathRecipient);
    std::shared_ptr<AMSEventHandler> handler;
    appMgrServiceInner->SetEventHandler(handler);
    appMgrServiceInner->HandleAbilityAttachTimeOut(token);
    appMgrServiceInner->PrepareTerminate(token);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    appMgrServiceInner->HandleTerminateApplicationTimeOut(eventId);
    appMgrServiceInner->HandleAddAbilityStageTimeOut(eventId);
    RunningProcessInfo runningProcessInfo;
    appMgrServiceInner->GetRunningProcessInfoByToken(token, runningProcessInfo);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrServiceInner->LoadResidentProcess(bundleInfos);
    int restartCount = static_cast<int>(GetU32Data(data));
    bool isEmptyKeepAliveApp = *data % ENABLE;
    appMgrServiceInner->StartResidentProcess(bundleInfos, restartCount, isEmptyKeepAliveApp);
    appMgrServiceInner->StartEmptyResidentProcess(bundleInfo, processName, restartCount, isEmptyKeepAliveApp);
    appMgrServiceInner->RestartResidentProcess(appRecord);
    const std::string eventData(data, size);
    appMgrServiceInner->NotifyAppStatus(bundleName, eventData);
    appMgrServiceInner->NotifyAppStatusByCallerUid(bundleName, static_cast<int32_t>(userId), callerUid, eventData);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size > OHOS::FOO_MAX_LEN || size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
