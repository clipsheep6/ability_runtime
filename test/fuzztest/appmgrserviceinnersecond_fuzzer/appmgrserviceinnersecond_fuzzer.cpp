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

#include "appmgrserviceinnersecond_fuzzer.h"

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
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AppMgrServiceInner* appMgrServiceInner = new AppMgrServiceInner();
    sptr<IApplicationStateObserver> observer;
    std::vector<std::string> bundleNameList;
    appMgrServiceInner->RegisterApplicationStateObserver(observer, bundleNameList);
    appMgrServiceInner->UnregisterApplicationStateObserver(observer);
    std::vector<AppStateData> list;
    appMgrServiceInner->GetForegroundApplications(list);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
    }
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    BundleInfo bundleInfo;
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    appMgrServiceInner->StartUserTestProcess(*want, token, bundleInfo, userId);
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->GetHapModuleInfoForTestRunner(*want, token, bundleInfo, hapModuleInfo);
    std::string msg(data, size);
    std::string processName(data, size);
    appMgrServiceInner->StartEmptyProcess(*want, token, bundleInfo, processName, userId);
    int64_t resultCode = static_cast<int64_t>(GetU32Data(data));
    std::string bundleName(data, size);
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    appMgrServiceInner->FinishUserTest(msg, resultCode, bundleName, pid);
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->FinishUserTestLocked(msg, resultCode, appRecord);
    AbilityInfo abilityInfo;
    appMgrServiceInner->StartSpecifiedAbility(*want, abilityInfo);
    sptr<IStartSpecifiedAbilityResponse> response;
    appMgrServiceInner->RegisterStartSpecifiedAbilityResponse(response);
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    std::string flag(data, size);
    appMgrServiceInner->ScheduleAcceptWantDone(recordId, *want, flag);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    appMgrServiceInner->HandleStartSpecifiedAbilityTimeOut(eventId);
    Configuration config;
    appMgrServiceInner->UpdateConfiguration(config);
    sptr<IConfigurationObserver> configurationObserver;
    appMgrServiceInner->RegisterConfigurationObserver(configurationObserver);
    appMgrServiceInner->UnregisterConfigurationObserver(configurationObserver);
    appMgrServiceInner->InitGlobalConfiguration();
    appMgrServiceInner->GetConfiguration();
    appMgrServiceInner->KillApplicationByRecord(appRecord);
    int32_t innerEventId = static_cast<int32_t>(GetU32Data(data));
    appMgrServiceInner->SendHiSysEvent(innerEventId, eventId);
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrServiceInner->GetAbilityRecordsByProcessID(static_cast<int>(pid), tokens);
    ApplicationInfo application;
    bool debug;
    appMgrServiceInner->GetApplicationInfoByProcessID(static_cast<int>(pid), application, debug);
    appMgrServiceInner->VerifyProcessPermission("");
    appMgrServiceInner->VerifyAPL();
    std::string permissionName(data, size);
    appMgrServiceInner->VerifyAccountPermission(permissionName, static_cast<int>(userId));
    pid_t hostPid = static_cast<pid_t>(GetU32Data(data));
    appMgrServiceInner->PreStartNWebSpawnProcess(hostPid);
    std::string renderParam(data, size);
    int32_t ipcFd = static_cast<int32_t>(GetU32Data(data));
    int32_t sharedFd = static_cast<int32_t>(GetU32Data(data));
    pid_t renderPid = static_cast<pid_t>(GetU32Data(data));
    appMgrServiceInner->StartRenderProcess(hostPid, renderParam, ipcFd, sharedFd, renderPid);
    sptr<IRenderScheduler> scheduler;
    appMgrServiceInner->AttachRenderProcess(pid, scheduler);
    std::shared_ptr<RenderRecord> renderRecord;
    appMgrServiceInner->StartRenderProcessImpl(renderRecord, appRecord, renderPid);
    int status = static_cast<int>(GetU32Data(data));
    appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    wptr<IRemoteObject> remote;
    appMgrServiceInner->OnRenderRemoteDied(remote);
    appMgrServiceInner->BuildStartFlags(*want, abilityInfo);
    sptr<OHOS::Rosen::FocusChangeInfo> focusChangeInfo;
    appMgrServiceInner->HandleFocused(focusChangeInfo);
    appMgrServiceInner->HandleUnfocused(focusChangeInfo);
    sptr<IQuickFixCallback> callback;
    appMgrServiceInner->NotifyLoadRepairPatch(bundleName, callback);
    appMgrServiceInner->NotifyHotReloadPage(bundleName, callback);
    bool isContinuousTask = *data % ENABLE;
    appMgrServiceInner->SetContinuousTaskProcess(static_cast<int32_t>(pid), isContinuousTask);
    appMgrServiceInner->NotifyUnLoadRepairPatch(bundleName, callback);
    return appMgrServiceInner->GetAppRunningStateByBundleName(bundleName);
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
