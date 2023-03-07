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

#include "apprunningrecordsecond_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_running_record.h"
#undef private

#include "ability_record.h"
#include "app_running_manager.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
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
    std::shared_ptr<ApplicationInfo> appInfo;
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    std::string processName(data, size);
    AppRunningRecord* appRecord = new AppRunningRecord(appInfo, recordId, processName);
    appRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    std::shared_ptr<AbilityRunningRecord> ability;
    appRecord->LaunchAbility(ability);
    appRecord->AddAbilityStage();
    std::string bundleName(data, size);
    appRecord->AddAbilityStageBySpecifiedAbility(bundleName);
    appRecord->LaunchPendingAbilities();
    Configuration config;
    appRecord->LaunchApplication(config);
    MessageParcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
    }
    std::string moduleName(data, size);
    bool isContinuousTask = *data % ENABLE;
    appRecord->SetContinuousTaskAppState(isContinuousTask);
    appRecord->RegisterAppDeathRecipient();
    sptr<AppDeathRecipient> appDeathRecipient;
    appRecord->SetAppDeathRecipient(appDeathRecipient);
    appRecord->RemoveAppDeathRecipient();
    std::vector<HapModuleInfo> moduleInfos;
    appRecord->AddModules(appInfo, moduleInfos);
    std::shared_ptr<AbilityInfo> abilityInfo;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> wantptr;
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, wantptr);
    appRecord->ScheduleForegroundRunning();
    appRecord->ScheduleBackgroundRunning();
    appRecord->AbilityForeground(ability);
    appRecord->ScheduleProcessSecurityExit();
    appRecord->ScheduleTrimMemory();
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    appRecord->ScheduleMemoryLevel(level);
    appRecord->LowMemoryWarning();
    AppExecFwk::AbilityState abilityState = AppExecFwk::AbilityState::ABILITY_STATE_CREATE;
    bool isAbility = *data % ENABLE;
    appRecord->StateChangedNotifyObserver(ability, static_cast<int32_t>(abilityState), isAbility);
    bool isFocus = *data % ENABLE;
    appRecord->UpdateAbilityFocusState(token, isFocus);
    appRecord->UpdateAbilityState(token, abilityState);
    appRecord->AbilityFocused(ability);
    appRecord->AbilityUnfocused(ability);
    appRecord->AbilityBackground(ability);
    appRecord->PopForegroundingAbilityTokens();
    uint32_t msg = GetU32Data(data);
    int64_t timeOut = static_cast<int64_t>(GetU32Data(data));
    appRecord->SendEventForSpecifiedAbility(msg, timeOut);
    appRecord->SendEvent(msg, timeOut);
    std::string stringMsg(data, size);
    Closure task;
    appRecord->PostTask(stringMsg, timeOut, task);
    appRecord->IsLastAbilityRecord(token);
    appRecord->IsLastPageAbilityRecord(token);
    appRecord->ScheduleAcceptWant(moduleName);
    appRecord->UpdateConfiguration(config);
    std::string description(data, size);
    appRecord->ScheduleAppCrash(description);
    appRecord->GetBundleName();
    appRecord->IsLauncherApp();
    appRecord->GetRecordId();
    appRecord->GetName();
    appRecord->GetProcessName();
    appRecord->GetAppInfoList();
    appRecord->GetAbilities();
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    appRecord->GetAbilityRunningRecord(eventId);
    appRecord->GetModuleRecordByModuleName(bundleName, moduleName);
    appRecord->GetModuleRunningRecordByToken(token);
    appRecord->GetModuleRunningRecordByTerminateLists(token);
    appRecord->GetAbilityRunningRecordByToken(token);
    appRecord->GetAbilityByTerminateLists(token);
    appRecord->GetAllModuleRecord();
    appRecord->GetPriorityObject();
    appRecord->GetEventId();
    appRecord->GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo);
    appRecord->CanRestartResidentProc();
    std::vector<std::string> bundleNames;
    appRecord->GetBundleNames(bundleNames);
    appRecord->IsStartSpecifiedAbility();
    appRecord->GetSpecifiedWant();
    appRecord->IsContinuousTask();
    appRecord->GetFocusFlag();
    appRecord->GetAppStartTime();
    std::shared_ptr<ModuleRunningRecord> moduleRecord;
    appRecord->RemoveModuleRecord(moduleRecord);
    std::string reason(data, size);
    appRecord->ForceKillApp(reason);
    bool isForce = *data % ENABLE;
    appRecord->TerminateAbility(token, isForce);
    appRecord->AbilityTerminated(token);
    appRecord->ScheduleTerminate();
    appRecord->ApplicationTerminated();
    appRecord->RemoveTerminateAbilityTimeoutTask(token);
    sptr<IQuickFixCallback> callback;
    appRecord->NotifyLoadRepairPatch(bundleName, callback, recordId);
    appRecord->NotifyHotReloadPage(callback, recordId);
    appRecord->NotifyUnLoadRepairPatch(bundleName, callback, recordId);
    return appRecord->IsLauncherApp();
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
