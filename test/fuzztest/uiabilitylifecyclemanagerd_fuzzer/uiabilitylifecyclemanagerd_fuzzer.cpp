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

#include "uiabilitylifecyclemanagerd_fuzzer.h"

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
    std::shared_ptr<AbilityRecord> ability;
    uIAbilityLifecycleManager->HandleLoadTimeout(ability);
    std::shared_ptr<AbilityRecord> ability1;
    AAFwk::AbilityState state = AAFwk::AbilityState::INITIAL;
    uIAbilityLifecycleManager->HandleForegroundFailed(ability1, state);
    std::shared_ptr<AbilityRecord> ability2;
    uIAbilityLifecycleManager->HandleForegroundTimeout(ability2);
    std::shared_ptr<AbilityRecord> ability3;
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::string strParam(data, size);
    uIAbilityLifecycleManager->NotifySCBToHandleException(ability3, int32Param, strParam);
    std::shared_ptr<AbilityRecord> ability4;
    uIAbilityLifecycleManager->MoveToBackground(ability4);
    std::shared_ptr<AbilityRecord> ability6;
    uint32_t msgId = static_cast<uint32_t>(GetU32Data(data));
    bool boolParam = *data % ENABLE;
    uIAbilityLifecycleManager->PrintTimeOutLog(ability6, msgId, boolParam);
    std::shared_ptr<AbilityRecord> ability7;
    uIAbilityLifecycleManager->DelayCompleteTerminate(ability7);
    std::shared_ptr<AbilityRecord> ability8;
    uIAbilityLifecycleManager->CompleteTerminate(ability8);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    uIAbilityLifecycleManager->IsContainsAbilityInner(token);
    const std::shared_ptr<AbilityRecord> abilityRecord;
    AbilityRequest abilityRequest;
    uIAbilityLifecycleManager->NotifyAbilityToken(token, abilityRequest);
    int intParam = static_cast<int>(GetU32Data(data));
    Want *want;
    uIAbilityLifecycleManager->CloseUIAbilityInner(abilityRecord, intParam, want, boolParam);
    AbilityRequest abilityRequest1;
    uIAbilityLifecycleManager->CallAbilityLocked(abilityRequest1);
    Want want1;
    AbilityRequest request;
    uIAbilityLifecycleManager->NotifyStartSpecifiedAbility(request, want1);
    uIAbilityLifecycleManager->NotifyRestartSpecifiedAbility(request, token);
    AbilityRequest abilityRequestFront;
    std::shared_ptr<AbilityRecord> abilityRecordFront;
    std::shared_ptr<AbilityRecord> callerAbility;
    std::shared_ptr<StartOptions> startOptions;
    uIAbilityLifecycleManager->MoveAbilityToFront(
        abilityRequestFront, abilityRecordFront, callerAbility, startOptions);

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