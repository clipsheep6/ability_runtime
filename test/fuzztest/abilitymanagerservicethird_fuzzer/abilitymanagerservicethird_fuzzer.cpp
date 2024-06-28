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

#include "abilitymanagerservicethird_fuzzer.h"

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

#define DISABLE_FUZZ
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
}

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

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    sptr<IRemoteObject> token = GetFuzzAbilityToken();

    // fuzz for AbilityManagerService
    auto abilityms = std::make_shared<AbilityManagerService>();
    abilityms->HandleLoadTimeOut(int64Param);
    abilityms->HandleActiveTimeOut(int64Param);
    abilityms->HandleInactiveTimeOut(int64Param);
    abilityms->HandleForegroundTimeOut(int64Param);
    abilityms->VerificationToken(token);
    abilityms->VerificationAllToken(token);
    sptr<IAbilityScheduler> scheduler;
    abilityms->GetDataAbilityManager(scheduler);
    abilityms->GetMissionListManagerByUserId(int32Param);
    abilityms->GetConnectManagerByUserId(int32Param);
    abilityms->GetDataAbilityManagerByUserId(int32Param);
    abilityms->GetConnectManagerByToken(token);

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

#ifndef DISABLE_FUZZ
    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
#endif
    free(ch);
    ch = nullptr;
    return 0;
}

