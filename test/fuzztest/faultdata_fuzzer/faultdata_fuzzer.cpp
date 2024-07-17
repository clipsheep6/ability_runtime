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

#include "faultdata_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "fault_data.h"
#undef protected
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

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
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

Want& SetElement(Want &want)
{
    return want.SetElementName("deviceId", "bundleName", "ability", "moduleName");
}

void FaultDataFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    FaultData faultData;
    Parcel parcel1;
    parcel1.WriteInt32(int32Param);
    faultData.ReadFromParcel(parcel1); // branch name failed
    Parcel parcel2;
    parcel2.WriteString(stringParam);
    parcel2.WriteInt32(int32Param);
    faultData.ReadFromParcel(parcel2); // branch message failed
    Parcel parcel3;
    parcel3.WriteString(stringParam);
    parcel3.WriteString(stringParam);
    faultData.ReadFromParcel(parcel3); // branch stack failed
    Parcel parcel4;
    parcel4.WriteString(stringParam);
    parcel4.WriteString(stringParam);
    parcel4.WriteString(stringParam);
    faultData.ReadFromParcel(parcel4); // branch FaultType failed

    Parcel parcel5;
    parcel5.WriteString(stringParam);
    parcel5.WriteString(stringParam);
    parcel5.WriteString(stringParam);
    parcel5.WriteInt32(int32Param);
    faultData.ReadFromParcel(parcel5); // branch FaultType failed

    Parcel parcel6;
    parcel6.WriteString(stringParam);
    parcel6.WriteString(stringParam);
    parcel6.WriteString(stringParam);
    parcel6.WriteInt32(int32Param);
    parcel6.WriteString(stringParam);
    faultData.ReadFromParcel(parcel6); // branch FaultType failed
    parcel6.WriteBool(boolParam);
    faultData.ReadFromParcel(parcel6);
    Parcel parcel7;
    faultData.Marshalling(parcel7);
}

void FaultDataFuzztest2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    AppFaultDataBySA faultData;
    Parcel appParcel1;
    appParcel1.WriteInt32(int32Param);
    faultData.ReadFromParcel(appParcel1); // branch name failed
    Parcel appParcel2;
    appParcel2.WriteString(stringParam);
    appParcel2.WriteInt32(int32Param);
    faultData.ReadFromParcel(appParcel2); // branch message failed
    Parcel appParcel3;
    appParcel3.WriteString(stringParam);
    appParcel3.WriteString(stringParam);
    faultData.ReadFromParcel(appParcel3); // branch stack failed
    Parcel appParcel4;
    appParcel4.WriteString(stringParam);
    appParcel4.WriteString(stringParam);
    appParcel4.WriteString(stringParam);
    faultData.ReadFromParcel(appParcel4); // branch FaultType failed
    Parcel appParcel5;
    appParcel5.WriteString(stringParam);
    appParcel5.WriteString(stringParam);
    appParcel5.WriteString(stringParam);
    appParcel5.WriteInt32(int32Param);
    faultData.ReadFromParcel(appParcel5); // branch FaultType failed
    Parcel appParcel6;
    faultData.Marshalling(appParcel6);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    FaultDataFuzztest1(boolParam, stringParam, int32Param);
    FaultDataFuzztest2(boolParam, stringParam, int32Param);
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

