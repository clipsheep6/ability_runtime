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

#include "uiabilitylifecyclemanagera_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ui_ability_lifecycle_manager.h"
#undef private

#include "ability_record.h"

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
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
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

void DoSomethingInterestingWithMyAPI1(const char *data, size_t size)
{
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    auto uIAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>(userId);
    Want want1;
    bool boolParam = *data % ENABLE;
    std::string strParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    uint32_t requestId = static_cast<uint32_t>(GetU32Data(data));
    uIAbilityLifecycleManager->OnAcceptWantResponse(want1, strParam, requestId);
    uIAbilityLifecycleManager->OnStartSpecifiedProcessResponse(want1, strParam, requestId);
    uIAbilityLifecycleManager->OnStartSpecifiedAbilityTimeoutResponse(want1, requestId);
    uIAbilityLifecycleManager->OnStartSpecifiedProcessTimeoutResponse(want1, requestId);
    uIAbilityLifecycleManager->StartSpecifiedAbilityBySCB(want1);
    sptr<IRemoteObject> callStub;
    std::shared_ptr<AbilityRecord> abilityRecord4;
    uIAbilityLifecycleManager->CallRequestDone(abilityRecord4, callStub);
    sptr<IAbilityConnection> connect;
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    uIAbilityLifecycleManager->ReleaseCallLocked(connect,  element);
    std::shared_ptr<CallRecord> callRecord;
    uIAbilityLifecycleManager->OnCallConnectDied(callRecord);
    uIAbilityLifecycleManager->GetSessionIdByAbilityToken(token);
    std::vector<std::string> abilityList;
    uIAbilityLifecycleManager->GetActiveAbilityList(int32Param, abilityList, int32Param);
    std::shared_ptr<AbilityRecord> abilityRecord5;
    uIAbilityLifecycleManager->PrepareTerminateAbility(abilityRecord5);
    uIAbilityLifecycleManager->GetAbilityRecordsById(int32Param);
    uIAbilityLifecycleManager->CheckAbilityNumber(strParam, strParam, strParam);
    uIAbilityLifecycleManager->MoreAbilityNumbersSendEventInfo(int32Param, strParam, strParam, strParam);
    AppInfo info;
    info.state = AppState::TERMINATED;
    uIAbilityLifecycleManager->OnAppStateChanged(info);
    info.state = AppState::END;
    uIAbilityLifecycleManager->OnAppStateChanged(info);
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    auto uIAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>(userId);
    std::string strParam(data, size);
    uIAbilityLifecycleManager->SignRestartAppFlag(strParam);
    AbilityRequest abilityRequest;
    sptr<SessionInfo> sessionInfo;
    uint32_t sceneFlag = GetU32Data(data);
    bool boolParam = *data % ENABLE;
    uIAbilityLifecycleManager->StartUIAbility(abilityRequest, sessionInfo, sceneFlag, boolParam);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    int intParam = static_cast<int>(GetU32Data(data));
    AppExecFwk::PacMap saveData;
    uIAbilityLifecycleManager->AbilityTransactionDone(token, intParam, saveData);
    sptr<IAbilityScheduler> scheduler;
    uIAbilityLifecycleManager->AttachAbilityThread(scheduler, token);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    uIAbilityLifecycleManager->OnAbilityRequestDone(token, int32Param);
    uIAbilityLifecycleManager->IsContainsAbility(token);
    std::shared_ptr<AbilityRecord> abilityRecord;
    uIAbilityLifecycleManager->NotifySCBToMinimizeUIAbility(abilityRecord, token);
    std::shared_ptr<AbilityRecord> abilityRecord1;
    uIAbilityLifecycleManager->MinimizeUIAbility(abilityRecord1, boolParam, sceneFlag);
    sptr<SessionInfo> sessionInfo1;
    uIAbilityLifecycleManager->GetUIAbilityRecordBySessionInfo(sessionInfo1);
    Want *want;
    std::shared_ptr<AbilityRecord> abilityRecord2;
    uIAbilityLifecycleManager->CloseUIAbility(abilityRecord2, intParam, want, boolParam);
    sptr<IRemoteObject> rootSceneSession;
    uIAbilityLifecycleManager->SetRootSceneSession(rootSceneSession);
    AbilityRequest abilityRequest1;
    uIAbilityLifecycleManager->NotifySCBToStartUIAbility(abilityRequest1);
    sptr<SessionInfo> sessionInfo2;
    AbilityRequest abilityRequest2;
    uIAbilityLifecycleManager->NotifySCBToPreStartUIAbility(abilityRequest2, sessionInfo2);
    uint32_t msgId = static_cast<uint32_t>(GetU32Data(data));
    int64_t abilityRecordId = static_cast<int64_t>(GetU32Data(data));
    uIAbilityLifecycleManager->OnTimeOut(msgId, abilityRecordId, boolParam);
    std::shared_ptr<AbilityRecord> abilityRecord3;
    AbilityRequest abilityRequest3;
    uIAbilityLifecycleManager->OnAbilityDied(abilityRecord3);
    uIAbilityLifecycleManager->ResolveLocked(abilityRequest3);
    sptr<SessionInfo> sessionInfo3;
    uIAbilityLifecycleManager->CallUIAbilityBySCB(sessionInfo3, boolParam);
    DoSomethingInterestingWithMyAPI1(data, size);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
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

    char *ch = (char *)malloc(size + 1);
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