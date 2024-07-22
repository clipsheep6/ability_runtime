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

#include "uiabilitylifecyclemanagerc_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ui_ability_lifecycle_manager.h"
#include "ability_record.h"
#undef private
#undef protected

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

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    auto uIAbilityLifecycleManager = std::make_shared<UIAbilityLifecycleManager>(userId);
    std::string strParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    bool boolParam = *data % ENABLE;
    AbilityRequest abilityRequest;
    uIAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, boolParam);
    abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON;
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        uIAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, boolParam);
    }

    abilityRequest.startRecent = true;
    if (abilityRequest.startRecent) {
        uIAbilityLifecycleManager->GetReusedStandardPersistentId(abilityRequest, boolParam);
    }

    uIAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, boolParam);
    abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON;
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        uIAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, boolParam);
    }

    abilityRequest.startRecent = true;
    if (abilityRequest.startRecent) {
        uIAbilityLifecycleManager->GetReusedCollaboratorPersistentId(abilityRequest, boolParam);
    }
    AppExecFwk::AbilityInfo abilityInfo;
    uIAbilityLifecycleManager->GenerateProcessNameForNewProcessMode(abilityInfo);
    AbilityRequest abilityRequest1;
    uIAbilityLifecycleManager->PreCreateProcessName(abilityRequest1);
    std::shared_ptr<AbilityRecord> abilityRecordPtr;
    uIAbilityLifecycleManager->UpdateProcessName(abilityRequest1, abilityRecordPtr);
    AbilityRequest abilityRequest2;
    std::shared_ptr<AbilityRecord> abilityRecord;
    uIAbilityLifecycleManager->UpdateAbilityRecordLaunchReason(abilityRequest2, abilityRecord);
    std::shared_ptr<AbilityRecord> abilityRecord1;
    uIAbilityLifecycleManager->EraseAbilityRecord(abilityRecord1);
    std::shared_ptr<AbilityRecord> abilityRecord4;
    int state1 = static_cast<int>(GetU32Data(data));
    uIAbilityLifecycleManager->DispatchState(abilityRecord4, state1);
    AAFwk::AbilityState state = AAFwk::AbilityState::INITIAL;
    std::shared_ptr<AbilityRecord> abilityRecord8;
    uIAbilityLifecycleManager->DispatchForeground(abilityRecord8, boolParam, state);

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