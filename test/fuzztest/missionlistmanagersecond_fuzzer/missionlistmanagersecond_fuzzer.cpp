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

#include "missionlistmanagersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "mission_list_manager.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
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
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    uint32_t uint32Param = static_cast<uint32_t>(GetU32Data(data));
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
    sptr<IAbilityConnection> connect = new MyAbilityConnection();
    std::vector<std::string> info;
    std::shared_ptr<Mission> mission = std::make_shared<Mission>(int32Param, abilityRecord);
    std::shared_ptr<MissionList> missionList = std::make_shared<MissionList>();

    // fuzz for MissionListManager
    auto missionListManager = std::make_shared<MissionListManager>(intParam);
    missionListManager->Init();
    AbilityRequest abilityRequest;
    missionListManager->OnTimeOut(uint32Param, int64Param);
    missionListManager->HandleLoadTimeout(abilityRecord);
    missionListManager->HandleForegroundTimeout(abilityRecord, boolParam);
    missionListManager->CompleteForegroundFailed(abilityRecord, boolParam);
    missionListManager->HandleTimeoutAndResumeAbility(abilityRecord, boolParam);
    missionListManager->DelayedResumeTimeout(abilityRecord);
    missionListManager->BackToCaller(abilityRecord);
    missionListManager->MoveToTerminateList(abilityRecord);
    missionListManager->GetAbilityRecordByCaller(abilityRecord, intParam);
    missionListManager->GetAbilityRecordByEventId(int64Param);
    missionListManager->OnAbilityDied(abilityRecord, int32Param);
    missionListManager->GetTargetMissionList(intParam, mission);
    missionListManager->GetMissionIdByAbilityToken(token);
    missionListManager->GetAbilityTokenByMissionId(int32Param);
    missionListManager->PostStartWaitingAbility();
    missionListManager->HandleAbilityDied(abilityRecord);
    missionListManager->HandleLauncherDied(abilityRecord);
    missionListManager->HandleAbilityDiedByDefault(abilityRecord);
    missionListManager->DelayedStartLauncher();
    missionListManager->BackToLauncher();
    missionListManager->SetMissionLabel(token, stringParam);
    std::shared_ptr<OHOS::Media::PixelMap> icon = std::make_shared<OHOS::Media::PixelMap>();
    missionListManager->SetMissionIcon(token, icon);
    missionListManager->CompleteFirstFrameDrawing(token);
    missionListManager->GetCancelStartingWindowTask(abilityRecord);
    missionListManager->PostCancelStartingWindowTask(abilityRecord);
    missionListManager->Dump(info);
    missionListManager->DumpMissionListByRecordId(info, boolParam, int32Param, info);
    missionListManager->DumpMissionList(info, boolParam, stringParam);
    missionListManager->DumpMissionInfos(info);
    missionListManager->DumpMission(intParam, info);
    missionListManager->ResolveLocked(abilityRequest);
    missionListManager->CallAbilityLocked(abilityRequest);
    AppExecFwk::ElementName element;
    missionListManager->ReleaseCallLocked(connect, element);
    missionListManager->ResolveAbility(abilityRecord, abilityRequest);
    missionListManager->GetAbilityRecordByName(element);
    std::shared_ptr<CallRecord> callRecord;
    missionListManager->OnCallConnectDied(callRecord);
    missionListManager->OnAcceptWantResponse(*want, stringParam);
    missionListManager->OnStartSpecifiedAbilityTimeoutResponse(*want);
    missionListManager->GetMissionBySpecifiedFlag(*want, stringParam);
    missionListManager->IsReachToLimitLocked(abilityRequest);
    sptr<ISnapshotHandler> snapshotHandler;
    missionListManager->RegisterSnapshotHandler(snapshotHandler);
    MissionSnapshot missionSnapshot;
    missionListManager->GetMissionSnapshot(int32Param, token, missionSnapshot, boolParam);
    std::vector<AbilityRunningInfo> abilityRunningInfo;
    missionListManager->GetAbilityRunningInfos(abilityRunningInfo, boolParam);
    missionListManager->UninstallApp(stringParam, int32Param);
    missionListManager->AddUninstallTags(stringParam, int32Param);
    missionListManager->EraseWaitingAbility(stringParam, int32Param);
    missionListManager->IsStarted();
    missionListManager->PauseManager();
    missionListManager->ResumeManager();
    std::list<std::shared_ptr<AbilityRecord>> foregroundList;
    missionListManager->GetAllForegroundAbilities(foregroundList);
    missionListManager->GetForegroundAbilities(missionList, foregroundList);
    missionListManager->IsExcludeFromMissions(mission);
#ifdef ABILITY_COMMAND_FOR_TEST
    missionListManager->BlockAbility(int32Param);
#endif
    std::vector<sptr<IRemoteObject>> tokens;
    missionListManager->SetMissionANRStateByTokens(tokens);
    missionListManager->listenerController_ = nullptr;
    missionListManager->RemoveMissionLocked(int32Param, boolParam);

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

