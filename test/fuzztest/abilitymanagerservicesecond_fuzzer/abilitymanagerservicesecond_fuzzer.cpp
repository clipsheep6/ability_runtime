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

#include "abilitymanagerservicesecond_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_manager_service.h"
#undef protected
#undef private

#include "ability_record.h"

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
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::string stringParam(data, size);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();

    // fuzz for AbilityManagerService
    auto abilityms = std::make_shared<AbilityManagerService>();
    WantSenderInfo wantSenderInfo;
    abilityms->GetWantSender(wantSenderInfo, token);
    sptr<IWantSender> target;
    SenderInfo senderInfo;
    abilityms->SendWantSender(target, senderInfo);
    sptr<IWantSender> sender;
    abilityms->CancelWantSender(sender);
    abilityms->GetPendingWantUid(target);
    abilityms->GetPendingWantUserId(target);
    abilityms->GetPendingWantBundleName(target);
    abilityms->GetPendingWantCode(target);
    abilityms->GetPendingWantType(target);
    sptr<IWantReceiver> receiver;
    abilityms->RegisterCancelListener(sender, receiver);
    abilityms->UnregisterCancelListener(sender, receiver);
    std::shared_ptr<Want> wantPtr;
    abilityms->GetPendingRequestWant(target, wantPtr);
    abilityms->LockMissionForCleanup(int32Param);
    abilityms->UnlockMissionForCleanup(int32Param);
    sptr<IMissionListener> listener;
    abilityms->RegisterMissionListener(listener);
    abilityms->UnRegisterMissionListener(listener);
    std::vector<MissionInfo> missionInfos;
    abilityms->GetMissionInfos(stringParam, int32Param, missionInfos);
    abilityms->GetRemoteMissionInfos(stringParam, int32Param, missionInfos);
    MissionInfo missionInfo;
    abilityms->GetMissionInfo(stringParam, int32Param, missionInfo);
    abilityms->GetRemoteMissionInfo(stringParam, int32Param, missionInfo);
    abilityms->CleanMission(int32Param);
    abilityms->CleanAllMissions();
    abilityms->MoveMissionToFront(int32Param);
    StartOptions startOptions;
    abilityms->MoveMissionToFront(int32Param, startOptions);
    abilityms->GetMissionIdByToken(token);
    abilityms->IsAbilityControllerStartById(int32Param);
    abilityms->GetServiceRecordByElementName(stringParam);
    sptr<IAbilityConnection> callback;
    abilityms->GetConnectRecordListByCallback(callback);
    Uri uri("myFuzzTest");
    abilityms->AcquireDataAbility(uri, boolParam, token);
    sptr<IAbilityScheduler> dataAbilityScheduler;
    abilityms->ReleaseDataAbility(dataAbilityScheduler, token);
    abilityms->AttachAbilityThread(dataAbilityScheduler, token);
    abilityms->DumpFuncInit();
    abilityms->DumpSysFuncInit();
    std::vector<std::string> info;
    abilityms->DumpSysInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpSysMissionListInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpSysAbilityInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpSysStateInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpSysPendingInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpSysProcess(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DataDumpSysStateInner(stringParam, info, boolParam, boolParam, intParam);
    abilityms->DumpInner(stringParam, info);
    abilityms->DumpMissionListInner(stringParam, info);
    abilityms->DumpMissionInfosInner(stringParam, info);
    abilityms->DumpMissionInner(stringParam, info);
    abilityms->DumpStateInner(stringParam, info);
    abilityms->DumpSysState(stringParam, info, boolParam, boolParam, intParam);
    PacMap saveData;
    abilityms->AbilityTransitionDone(token, intParam, saveData);
    sptr<IRemoteObject> remoteObject = GetFuzzAbilityToken();
    abilityms->ScheduleConnectAbilityDone(token, remoteObject);
    abilityms->ScheduleDisconnectAbilityDone(token);
    abilityms->ScheduleCommandAbilityDone(token);
    abilityms->OnAbilityRequestDone(token, int32Param);
    AppInfo appInfo;
    abilityms->OnAppStateChanged(appInfo);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
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

