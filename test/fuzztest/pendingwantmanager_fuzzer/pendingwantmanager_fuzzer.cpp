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

#include "pendingwantmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "pending_want_key.h"
#include "pending_want_manager.h"
#include "resident_process_manager.h"
#include "sa_mgr_client.h"
#include "task_data_persistence_mgr.h"
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
    size_t sizeParam = static_cast<size_t>(GetU32Data(data));
    std::string stringParam(data, size);
    Parcel wantParcel;
    Want *want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::vector<std::string> info;
    AbilityRequest abilityRequest;

    // fuzz for PendingWantKey
    auto pendingWantKey = std::make_shared<PendingWantKey>();
    pendingWantKey->SetType(int32Param);
    pendingWantKey->SetBundleName(stringParam);
    pendingWantKey->SetRequestWho(stringParam);
    pendingWantKey->SetRequestCode(int32Param);
    pendingWantKey->SetRequestWant(*want);
    pendingWantKey->SetRequestResolvedType(stringParam);
    std::vector<WantsInfo> allWantsInfos;
    pendingWantKey->SetAllWantsInfos(allWantsInfos);
    pendingWantKey->SetFlags(int32Param);
    pendingWantKey->SetCode(int32Param);
    pendingWantKey->SetUserId(int32Param);
    pendingWantKey->GetType();
    pendingWantKey->GetBundleName();
    pendingWantKey->GetRequestWho();
    pendingWantKey->GetRequestCode();
    pendingWantKey->GetRequestWant();
    pendingWantKey->GetRequestResolvedType();
    pendingWantKey->GetAllWantsInfos();
    pendingWantKey->GetFlags();
    pendingWantKey->GetCode();
    pendingWantKey->GetUserId();

    // fuzz for PendingWantManager
    auto pendingWantManager = std::make_shared<PendingWantManager>();
    WantSenderInfo wantSenderInfo;
    pendingWantManager->GetWantSender(int32Param, int32Param, stringParam, wantSenderInfo, token);
    pendingWantManager->GetWantSenderLocked(int32Param, int32Param, int32Param, wantSenderInfo, token);
    PendingWantRecord pendingWantRecord;
    pendingWantManager->MakeWantSenderCanceledLocked(pendingWantRecord);
    pendingWantManager->GetPendingWantRecordByKey(pendingWantKey);
    pendingWantManager->CheckPendingWantRecordByKey(pendingWantKey, pendingWantKey);
    sptr<IWantSender> wantSenderPtr;
    SenderInfo senderInfo;
    pendingWantManager->SendWantSender(wantSenderPtr, senderInfo);
    pendingWantManager->CancelWantSender(stringParam, wantSenderPtr);
    pendingWantManager->CancelWantSenderLocked(pendingWantRecord, boolParam);
    pendingWantManager->DeviceIdDetermine(*want, token, int32Param, int32Param);
    pendingWantManager->PendingWantStartAbility(*want, token, int32Param, int32Param);
    pendingWantManager->PendingWantStartAbilitys(allWantsInfos, token, int32Param, int32Param);
    pendingWantManager->PendingWantPublishCommonEvent(*want, senderInfo, int32Param, int32Param);
    pendingWantManager->PendingRecordIdCreate();
    pendingWantManager->GetPendingWantRecordByCode(int32Param);
    pendingWantManager->GetPendingWantUid(wantSenderPtr);
    pendingWantManager->GetPendingWantUserId(wantSenderPtr);
    pendingWantManager->GetPendingWantBundleName(wantSenderPtr);
    pendingWantManager->GetPendingWantCode(wantSenderPtr);
    pendingWantManager->GetPendingWantType(wantSenderPtr);
    sptr<IWantReceiver> wantReceiverPtr;
    pendingWantManager->RegisterCancelListener(wantSenderPtr, wantReceiverPtr);
    pendingWantManager->UnregisterCancelListener(wantSenderPtr, wantReceiverPtr);
    std::shared_ptr<Want> wantPtr;
    pendingWantManager->GetPendingRequestWant(wantSenderPtr, wantPtr);
    std::shared_ptr<WantSenderInfo> wantSenderInfoPtr;
    pendingWantManager->GetWantSenderInfo(wantSenderPtr, wantSenderInfoPtr);
    pendingWantManager->ClearPendingWantRecord(stringParam, int32Param);
    pendingWantManager->ClearPendingWantRecordTask(stringParam, int32Param);
    pendingWantManager->Dump(info);
    pendingWantManager->DumpByRecordId(info, stringParam);

    // fuzz for ResidentProcessManager
    auto residentProcessManager = std::make_shared<ResidentProcessManager>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    residentProcessManager->StartResidentProcess(bundleInfos);
    residentProcessManager->StartResidentProcessWithMainElement(bundleInfos);
    AppExecFwk::HapModuleInfo hapModuleInfo;
    std::set<uint32_t> needEraseIndexSet;
    residentProcessManager->CheckMainElement(hapModuleInfo, stringParam, stringParam, needEraseIndexSet, sizeParam);

    // fuzz for SaMgrClient
    auto saMgrClient = std::make_shared<SaMgrClient>();
    saMgrClient->GetSystemAbility(int32Param);
    saMgrClient->RegisterSystemAbility(int32Param, token);

    // fuzz for TaskDataPersistenceMgr
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    taskDataPersistenceMgr->Init(intParam);
    std::list<InnerMissionInfo> missionInfoList;
    taskDataPersistenceMgr->LoadAllMissionInfo(missionInfoList);
    InnerMissionInfo innerMissionInfo;
    taskDataPersistenceMgr->SaveMissionInfo(innerMissionInfo);
    taskDataPersistenceMgr->DeleteMissionInfo(intParam);
    taskDataPersistenceMgr->RemoveUserDir(int32Param);
    MissionSnapshot missionSnapshot;
    taskDataPersistenceMgr->SaveMissionSnapshot(intParam, missionSnapshot);
    taskDataPersistenceMgr->GetSnapshot(intParam);
    taskDataPersistenceMgr->GetMissionSnapshot(intParam, missionSnapshot, boolParam);

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

    char* ch = (char *)malloc(size + 1);
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

