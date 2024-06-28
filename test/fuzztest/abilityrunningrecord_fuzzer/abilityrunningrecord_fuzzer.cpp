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

#include "abilityrunningrecord_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_running_record.h"
#include "ability_record.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
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
    std::shared_ptr<AbilityInfo> info;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    int32_t abilityRecordId = static_cast<int32_t>(GetU32Data(data));
    AbilityRunningRecord abilityRecord(info, token, abilityRecordId);
    Parcel wantParcel;
    std::shared_ptr<Want> want = nullptr;
    abilityRecord.SetWant(want);
    AppExecFwk::AbilityState state = AppExecFwk::AbilityState::ABILITY_STATE_READY;
    abilityRecord.SetState(state);
    sptr<IRemoteObject> pretoken = GetFuzzAbilityToken();
    abilityRecord.SetPreToken(pretoken);
    int32_t visibility = static_cast<int32_t>(GetU32Data(data));
    abilityRecord.SetVisibility(visibility);
    int32_t perceptibility = static_cast<int32_t>(GetU32Data(data));
    abilityRecord.SetPerceptibility(perceptibility);
    int32_t connectionState = static_cast<int32_t>(GetU32Data(data));
    abilityRecord.SetConnectionState(connectionState);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    abilityRecord.SetEventId(eventId);
    int32_t ownerUserId = static_cast<int64_t>(GetU32Data(data));
    abilityRecord.SetOwnerUserId(ownerUserId);
    bool flag = *data % ENABLE;
    abilityRecord.SetIsSingleUser(flag);
    bool isFocus = *data % ENABLE;
    abilityRecord.UpdateFocusState(isFocus);
    abilityRecord.GetName();
    abilityRecord.GetAbilityInfo();
    abilityRecord.GetWant();
    abilityRecord.GetToken();
    abilityRecord.GetState();
    abilityRecord.GetLastLaunchTime();
    abilityRecord.GetPreToken();
    abilityRecord.GetVisibility();
    abilityRecord.GetPerceptibility();
    abilityRecord.GetConnectionState();
    abilityRecord.GetEventId();
    abilityRecord.SetTerminating();
    abilityRecord.IsTerminating();
    abilityRecord.GetOwnerUserId();
    abilityRecord.IsSingleUser();
    abilityRecord.GetFocusFlag();
    return abilityRecord.IsSameState(state);
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

