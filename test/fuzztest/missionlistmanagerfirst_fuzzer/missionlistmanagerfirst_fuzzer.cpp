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

#include "missionlistmanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "mission_list_manager.h"
#undef private

#include "ability_record.h"
#include "mission_info_mgr.h"

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

std::shared_ptr<AbilityRecord> GetFuzzAbilityRecord()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord;
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
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
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::vector<std::string> info;

    // fuzz for MissionListManager
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    auto launcherList = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    missionListManager->launcherList_ = launcherList;
    missionListManager->defaultStandardList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    missionListManager->defaultSingleList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    missionListManager->currentMissionLists_.push_front(launcherList);
    if (!missionListManager->listenerController_) {
        missionListManager->listenerController_ = std::make_shared<MissionListenerController>();
    }
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(intParam);
    AbilityRequest abilityRequest;
    missionListManager->StartAbility(abilityRequest);
    missionListManager->StartAbility(abilityRecord, abilityRecord, abilityRequest);
    missionListManager->MinimizeAbility(token, boolParam);
    sptr<IMissionListener> listener;
    missionListManager->RegisterMissionListener(listener);
    missionListManager->UnRegisterMissionListener(listener);
    std::vector<MissionInfo> missionInfos;
    missionListManager->GetMissionInfos(int32Param, missionInfos);
    MissionInfo missionInfo;
    missionListManager->GetMissionInfo(int32Param, missionInfo);
    std::shared_ptr<StartOptions> startOptions = std::make_shared<StartOptions>();
    missionListManager->MoveMissionToFront(int32Param, startOptions);
    missionListManager->MoveMissionToFront(int32Param, boolParam, startOptions);
    missionListManager->EnqueueWaitingAbility(abilityRequest);
    missionListManager->EnqueueWaitingAbilityToFront(abilityRequest);
    missionListManager->StartWaitingAbility();
    missionListManager->StartAbilityLocked(abilityRecord, abilityRecord, abilityRequest);
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    missionListManager->HandleReusedMissionAndAbility(abilityRequest, mission, abilityRecord);
    missionListManager->GetMissionName(abilityRequest);
    InnerMissionInfo innerMissionInfo;
    missionListManager->CreateOrReusedMissionInfo(abilityRequest, innerMissionInfo);
    missionListManager->GetTargetMissionAndAbility(abilityRequest, mission, abilityRecord);
    missionListManager->BuildInnerMissionInfo(innerMissionInfo, stringParam, abilityRequest);
    missionListManager->GetTargetMissionList(abilityRecord, abilityRequest);
    missionListManager->GetTargetMissionListByLauncher(abilityRequest);
    missionListManager->GetTargetMissionListByDefault(abilityRecord, abilityRequest);
    missionListManager->GetReusedMission(abilityRequest);
    missionListManager->GetReusedSpecifiedMission(abilityRequest);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();
    missionListManager->MoveMissionToTargetList(boolParam, missionList, mission);
    missionListManager->MoveNoneTopMissionToDefaultList(mission);
    missionListManager->MoveMissionListToTop(missionList);
    missionListManager->MinimizeAbilityLocked(abilityRecord, boolParam);
    missionListManager->GetCurrentTopAbilityLocked();
    sptr<IAbilityScheduler> scheduler;
    missionListManager->AttachAbilityThread(scheduler, token);
    missionListManager->OnAbilityRequestDone(token, int32Param);
    AppInfo appInfo;
    missionListManager->OnAppStateChanged(appInfo);
    missionListManager->GetAbilityRecordByToken(token);
    missionListManager->GetMissionById(intParam);

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
