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

#include "dataabilitymanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "data_ability_manager.h"
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
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    std::string stringParam(data, size);
    std::shared_ptr<AbilityRecord> abilityRecord = GetFuzzAbilityRecord();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::vector<std::string> info;
    AbilityRequest abilityRequest;

    // fuzz for DataAbilityManager
    auto dataAbilityManager = std::make_shared<DataAbilityManager>();
    sptr<IRemoteObject> client;
    dataAbilityManager->Acquire(abilityRequest, boolParam, client, boolParam);
    sptr<IAbilityScheduler> scheduler;
    dataAbilityManager->Release(scheduler, client, boolParam);
    dataAbilityManager->ContainsDataAbility(scheduler);
    dataAbilityManager->AttachAbilityThread(scheduler, token);
    dataAbilityManager->AbilityTransitionDone(token, intParam);
    dataAbilityManager->OnAbilityRequestDone(token, int32Param);
    dataAbilityManager->OnAbilityDied(abilityRecord);
    AppInfo appInfo;
    dataAbilityManager->OnAppStateChanged(appInfo);
    dataAbilityManager->GetAbilityRecordById(int64Param);
    dataAbilityManager->GetAbilityRecordByToken(token);
    dataAbilityManager->GetAbilityRecordByScheduler(scheduler);
    dataAbilityManager->Dump(data, intParam);
    dataAbilityManager->LoadLocked(stringParam, abilityRequest);
    dataAbilityManager->DumpLocked(data, intParam);
    dataAbilityManager->DumpState(info, stringParam);
    dataAbilityManager->DumpSysState(info, boolParam, stringParam);
    std::vector<AbilityRunningInfo> AbilityRunningInfoVector;
    dataAbilityManager->GetAbilityRunningInfos(AbilityRunningInfoVector, boolParam);
    dataAbilityManager->RestartDataAbility(abilityRecord);
    std::shared_ptr<DataAbilityRecord> record;
    dataAbilityManager->ReportDataAbilityAcquired(client, boolParam, record);
    dataAbilityManager->ReportDataAbilityReleased(client, boolParam, record);

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

