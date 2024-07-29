/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "appmgrapprunningrecord_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_running_record.h"
#undef private
#include "inner_mission_info.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
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
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
void DoSomethingInterestingWithMyAPI1(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    std::shared_ptr<ApplicationInfo> appInfo;
    std::shared_ptr<AbilityInfo> abilityInfo;
    sptr<IRemoteObject> token;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    int32_t abilityRecordId = static_cast<int32_t>(GetU32Data(data));
    infos->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want, abilityRecordId);
    std::vector<HapModuleInfo> moduleInfos;
    infos->AddModules(appInfo, moduleInfos);
    std::string bundleName(data, size);
    std::string moduleName(data, size);
    infos->GetModuleRecordByModuleName(bundleName, moduleName);
    infos->GetModuleRunningRecordByToken(token);
    infos->GetModuleRunningRecordByTerminateLists(token);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    infos->GetAbilityRunningRecord(eventId);
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    std::string reason(data, size);
    infos->ForceKillApp(reason);
    std::string description(data, size);
    infos->ScheduleAppCrash(description);
    Configuration config;
    infos->LaunchApplication(config);
    infos->AddAbilityStage();
    infos->AddAbilityStageBySpecifiedAbility(bundleName);
    infos->AddAbilityStageBySpecifiedProcess(bundleName);
    infos->AddAbilityStageDone();
    ApplicationInfo appInfos;
    infos->UpdateApplicationInfoInstalled(appInfos);
    std::shared_ptr<AbilityRunningRecord> ability;
    infos->LaunchAbility(ability);
    infos->LaunchPendingAbilities();
    infos->LowMemoryWarning();
    infos->ScheduleTerminate();
    infos->ScheduleProcessSecurityExit();
    infos->ScheduleClearPageStack();
    infos->ScheduleTrimMemory();
    infos->ScheduleMemoryLevel(level);
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    int32_t pid = static_cast<int32_t>(GetU32Data(data));
    infos->ScheduleHeapMemory(pid, mallocInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info1;
    infos->ScheduleJsHeapMemory(info1);
    infos->GetAbilityRunningRecordByToken(token);
    infos->GetAbilityByTerminateLists(token);
    AbilityState states = AbilityState::ABILITY_STATE_CREATE;
    infos->UpdateAbilityState(token, states);
    infos->PopForegroundingAbilityTokens();
}
void DoSomethingInterestingWithMyAPI2(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    bool isForce = *data % ENABLE;
    sptr<IRemoteObject> token;
    infos->TerminateAbility(token, isForce);
    infos->AbilityTerminated(token);
    std::weak_ptr<AppMgrServiceInner> inner;
    infos->SetAppMgrServiceInner(inner);
    sptr<AppDeathRecipient> appDeathRecipient;
    infos->SetAppDeathRecipient(appDeathRecipient);
    infos->GetPriorityObject();
    infos->RemoveAppDeathRecipient();
    Configuration config;
    infos->UpdateConfiguration(config);
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler;
    infos->SetTaskHandler(taskHandler);
    std::shared_ptr<AMSEventHandler> handler;
    infos->SetEventHandler(handler);
    infos->GetEventId();
    infos->IsLastAbilityRecord(token);
    infos->IsLastPageAbilityRecord(token);
    infos->ExtensionAbilityRecordExists();
    infos->SetTerminating();
    infos->IsTerminating();
    infos->IsKeepAliveApp();
    infos->IsEmptyKeepAliveApp();
    infos->IsMainProcess();
    bool isEmptyKeepAliveApp = *data % ENABLE;
    infos->SetEmptyKeepAliveAppState(isEmptyKeepAliveApp);
    bool isKeepAliveEnable = *data % ENABLE;
    infos->SetKeepAliveEnableState(isKeepAliveEnable);
    bool isSingleton = *data % ENABLE;
    infos->SetSingleton(isSingleton);
    bool isMainProcess = *data % ENABLE;
    infos->SetMainProcess(isMainProcess);
    bool isStageBasedModel = *data % ENABLE;
    infos->SetStageModelState(isStageBasedModel);
    infos->GetAllModuleRecord();
    infos->GetAppInfoList();
    infos->GetApplicationInfo();
    int count = static_cast<int>(GetU32Data(data));
    infos->SetRestartResidentProcCount(count);
    infos->DecRestartResidentProcCount();
    infos->GetRestartResidentProcCount();
    infos->CanRestartResidentProc();
}
void DoSomethingInterestingWithMyAPI3(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    int32_t state1 = static_cast<int32_t>(GetU32Data(data));
    bool isAbility = *data % ENABLE;
    bool isFromWindowFocusChanged = *data % ENABLE;
    std::shared_ptr<AbilityRunningRecord> ability;
    infos->StateChangedNotifyObserver(ability, state1,
        isAbility, isFromWindowFocusChanged);
    std::vector<std::string> bundleName;
    infos->GetBundleNames(bundleName);
    std::shared_ptr<UserTestRecord> record;
    infos->SetUserTestInfo(record);
    infos->GetUserTestInfo();
    std::shared_ptr<AbilityInfo> abilityInfo;
    infos->SetProcessAndExtensionType(abilityInfo);
    int requestId = static_cast<int>(GetU32Data(data));
    AAFwk::Want want;
    std::string moduleName(data, size);
    infos->SetSpecifiedAbilityFlagAndWant(requestId, want, moduleName);
    int32_t requestId1 = static_cast<int32_t>(GetU32Data(data));
    infos->SetScheduleNewProcessRequestState(requestId, want, moduleName);
    infos->IsNewProcessRequest();
    infos->IsStartSpecifiedAbility();
    infos->GetSpecifiedRequestId();
    infos->ResetSpecifiedRequestId();
    infos->ScheduleAcceptWant(moduleName);
    infos->ScheduleAcceptWantDone();
    infos->ScheduleNewProcessRequest(want, moduleName);
    infos->ScheduleNewProcessRequestDone();
    infos->ApplicationTerminated();
    infos->GetSpecifiedWant();
    infos->GetNewProcessRequestWant();
    infos->GetNewProcessRequestId();
    infos->ResetNewProcessRequestId();
    bool isDebugApp = *data % ENABLE;
    infos->SetDebugApp(isDebugApp);
    infos->IsDebugApp();
    infos->IsDebugging();
    bool isNativeDebug = *data % ENABLE;
    infos->SetNativeDebug(isNativeDebug);
    std::string perfCmd(data, size);
    infos->SetPerfCmd(perfCmd);
    bool multiThread = *data % ENABLE;
    infos->SetMultiThread(multiThread);
    std::shared_ptr<RenderRecord> record1;
    infos->AddRenderRecord(record1);
    infos->RemoveRenderRecord(record1);
}
void DoSomethingInterestingWithMyAPI4(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    infos->GetRenderRecordByPid(pid);
    infos->GetRenderRecordMap();
    AppSpawnStartMsg msg;
    infos->SetStartMsg(msg);
    infos->GetStartMsg();
    uint32_t msg1 = static_cast<uint32_t>(GetU32Data(data));
    int64_t timeOut = static_cast<uint32_t>(GetU32Data(data));
    infos->SendEventForSpecifiedAbility(msg1, timeOut);
    std::shared_ptr<AbilityRunningRecord> ability;
    AppStartType startType = AppStartType::COLD;
    infos->SendAppStartupTypeEvent(ability, startType);
    infos->SetKilling();
    infos->IsKilling();
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data));
    infos->SetAppIndex(appIndex);
    infos->GetAppIndex();
    bool securityFlag = *data % ENABLE;
    infos->SetSecurityFlag(securityFlag);
    infos->GetSecurityFlag();
    using Closure = std::function<void()>;
    Closure task;
    std::string msg2(data, size);
    infos->PostTask(msg2, timeOut, task);
    infos->CancelTask(msg2);
    sptr<IRemoteObject> token;
    infos->RemoveTerminateAbilityTimeoutTask(token);
    sptr<IQuickFixCallback> callback;
    std::string bundleName(data, size);
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    infos->NotifyLoadRepairPatch(bundleName, callback, recordId);
    infos->NotifyHotReloadPage(callback, recordId);
    infos->NotifyUnLoadRepairPatch(bundleName, callback, recordId);
    infos->IsContinuousTask();
    bool isContinuousTask = *data % ENABLE;
    infos->SetContinuousTaskAppState(isContinuousTask);
    bool isFocus = *data % ENABLE;
    infos->UpdateAbilityFocusState(token, isFocus);
    infos->GetFocusFlag();
    infos->GetAppStartTime();
    int64_t restartTimeMillis = static_cast<int64_t>(GetU32Data(data));
    infos->SetRestartTimeMillis(restartTimeMillis);
}
void DoSomethingInterestingWithMyAPI5(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    int32_t requestProcCode = static_cast<int32_t>(GetU32Data(data));
    infos->SetRequestProcCode(requestProcCode);
    infos->GetRequestProcCode();
    ProcessChangeReason reason1 = ProcessChangeReason::REASON_NONE;
    infos->SetProcessChangeReason(reason1);
    infos->GetProcessChangeReason();
    infos->IsUpdateStateFromService();
    bool isUpdateStateFromService = *data % ENABLE;
    infos->SetUpdateStateFromService(isUpdateStateFromService);
    infos->GetExtensionType();
    infos->GetProcessType();
    FaultData faultData;
    infos->NotifyAppFault(faultData);
    infos->IsAbilitytiesBackground();
    infos->SetAbilityForegroundingFlag();
    infos->GetAbilityForegroundingFlag();
    infos->SetSpawned();
    infos->GetSpawned();
    infos->GetChildAppRecordMap();
    std::shared_ptr<AppRunningRecord> appRecord;
    pid_t pid1 = static_cast<pid_t>(GetU32Data(data));
    infos->AddChildAppRecord(pid1, appRecord);
    infos->RemoveChildAppRecord(pid1);
    infos->ClearChildAppRecordMap();
    infos->SetParentAppRecord(appRecord);
    infos->GetParentAppRecord();
    int32_t state1 = static_cast<int32_t>(GetU32Data(data));
    infos->ChangeAppGcState(state1);
    bool isAttachDebug = *data % ENABLE;
    infos->SetAttachDebug(isAttachDebug);
    infos->isAttachDebug();
    ApplicationPendingState pendingState = ApplicationPendingState::READY;
    infos->SetApplicationPendingState(pendingState);
    infos->GetApplicationPendingState();
    bool isSplitScreenMode = *data % ENABLE;
    bool isFloatingWindowMode = *data % ENABLE;
    infos->GetSplitModeAndFloatingMode(isSplitScreenMode, isFloatingWindowMode);
    infos->GetChildProcessRecordByPid(pid1);
    infos->GetChildProcessRecordMap();
}
void DoSomethingInterestingWithMyAPI6(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    PreloadState state2 = PreloadState::NONE;
    infos->SetPreloadState(state2);
    infos->IsPreloading();
    infos->IsPreloaded();
    infos->GetAssignTokenId();
    int32_t tokenId = static_cast<int32_t>(GetU32Data(data));
    infos->SetAssignTokenId(tokenId);
    bool isNativeStart = *data % ENABLE;
    infos->SetNativeStart(isNativeStart);
    infos->isNativeStart();
    bool isRestartApp = *data % ENABLE;
    infos->SetRestartAppFlag(isRestartApp);
    infos->GetRestartAppFlag();
    bool flag1 = *data % ENABLE;
    infos->SetAssertionPauseFlag(flag1);
    infos->IsAssertionPause();
    bool jitEnabled = *data % ENABLE;
    infos->SetJITEnabled(jitEnabled);
    infos->IsJITEnabled();
    std::string result(data, size);
    infos->DumpIpcStart(result);
    infos->DumpIpcStop(result);
    infos->DumpIpcStat(result);
    infos->DumpFfrt(result);
    int32_t reason2 = static_cast<int32_t>(GetU32Data(data));
    infos->SetExitReason(reason2);
    infos->GetExitReason();
    std::string exitMsg(data, size);
    infos->SetExitMsg(exitMsg);
    infos->GetExitMsg();
    bool isSupport = *data % ENABLE;
    infos->SetSupportedProcessCache(isSupport);
    infos->GetSupportProcessCacheState();
    bool isAttached = *data % ENABLE;
    infos->SetAttachedToStatusBar(isAttached);
    infos->IsAttachedToStatusBar();
    sptr<IRemoteObject> browser;
    infos->SetBrowserHost(browser);
    infos->GetBrowserHost();
}
void DoSomethingInterestingWithMyAPI7(std::shared_ptr<AppRunningRecord> &infos, const char* data, size_t size)
{
    bool gpu = *data % ENABLE;
    infos->SetIsGPU(gpu);
    infos->GetIsGPU();
    pid_t gpuPid = static_cast<pid_t>(GetU32Data(data));
    infos->SetGPUPid(gpuPid);
    infos->GetGPUPid();
    infos->ScheduleCacheProcess();
    bool strictMode = *data % ENABLE;
    infos->SetStrictMode(strictMode);
    infos->IsStrictMode();
    bool isDepend = *data % ENABLE;
    infos->SetIsDependedOnArkWeb(isDepend);
    infos->IsDependedOnArkWeb();
    HapModuleInfo info2;
    std::string bundleName(data, size);
    infos->GetTheModuleInfoNeedToUpdated(bundleName, info2);
    infos->ScheduleForegroundRunning();
    infos->ScheduleBackgroundRunning();
    std::shared_ptr<AbilityRunningRecord> ability;
    infos->AbilityForeground(ability);
    infos->AbilityBackground(ability);
    infos->AbilityFocused(ability);
    infos->AbilityUnfocused(ability);
    uint32_t msg1 = static_cast<uint32_t>(GetU32Data(data));
    int64_t timeOut = static_cast<uint32_t>(GetU32Data(data));
    infos->SendEvent(msg1, timeOut);
    infos->SendClearTask(msg1, timeOut);
    std::shared_ptr<ModuleRunningRecord> record3;
    bool isExtensionDebug = *data % ENABLE;
    infos->RemoveModuleRecord(record3, isExtensionDebug);
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<ApplicationInfo> info;
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    std::string processName(data, size);
    std::shared_ptr<AppRunningRecord> infos = std::make_shared<AppRunningRecord>(info, recordId, processName);
    infos->GetBundleName();
    infos->GetCallerPid();
    int32_t pid = static_cast<int32_t>(GetU32Data(data));
    infos->SetCallerPid(pid);
    infos->GetCallerUid();
    int32_t uid = static_cast<int32_t>(GetU32Data(data));
    infos->SetCallerUid(uid);
    infos->GetCallerTokenId();
    int32_t tokenId = static_cast<int32_t>(GetU32Data(data));
    infos->SetCallerTokenId(tokenId);
    infos->IsLauncherApp();
    infos->GetRecordId();
    infos->GetName();
    infos->GetProcessName();
    infos->GetSpecifiedProcessFlag();
    std::string flag(data, size);
    infos->SetSpecifiedProcessFlag(flag);
    infos->GetSignCode();
    std::string signCode(data, size);
    infos->SetSignCode(signCode);
    infos->GetJointUserId();
    std::string jointUserId(data, size);
    infos->SetJointUserId(jointUserId);
    infos->GetUid();
    infos->SetUid(uid);
    infos->GetState();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    infos->SetState(state);
    infos->GetAbilities();
    sptr<IAppScheduler> thread;
    infos->SetApplicationClient(thread);
    infos->GetApplicationClient();
    DoSomethingInterestingWithMyAPI1(infos, data, size);
    DoSomethingInterestingWithMyAPI2(infos, data, size);
    DoSomethingInterestingWithMyAPI3(infos, data, size);
    DoSomethingInterestingWithMyAPI4(infos, data, size);
    DoSomethingInterestingWithMyAPI5(infos, data, size);
    DoSomethingInterestingWithMyAPI6(infos, data, size);
    DoSomethingInterestingWithMyAPI7(infos, data, size);
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
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
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

