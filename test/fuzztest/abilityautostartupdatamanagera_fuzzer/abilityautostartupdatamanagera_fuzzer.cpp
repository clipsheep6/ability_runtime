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

#include "abilityautostartupdatamanagera_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_auto_startup_data_manager.h"
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
const std::string jsonStr1 = "{\n\"isAutoStartup\": true, \n\"isEdmForce\": true\n}";
const std::string jsonStr2 = "{\n\"isAutoStartup\": \"true\", \n\"isEdmForce\": \"true\"\n}";
const std::string jsonStr3 = "{\n\"isAutoStartup2\": true, \n\"isEdmForce2\": true\n}";
const std::string jsonStr4 = "{\n\"isAutoStartup2\": true, \n\"isEdmForce2\": true\n";
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

void AbilityAutoStartupDataManagerFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<AbilityAutoStartupDataManager> dataMgr = std::make_shared<AbilityAutoStartupDataManager>();
    dataMgr->CheckKvStore();
    AutoStartupInfo info1;
    info1.userId = int32Param;
    dataMgr->InsertAutoStartupData(info1, boolParam, boolParam); // branch info1.bundleName empty
    info1.bundleName = "com.example.fuzzTest";
    dataMgr->InsertAutoStartupData(info1, boolParam, boolParam); // branch info1.abilityName empty
    info1.abilityName = "MainAbility";
    dataMgr->InsertAutoStartupData(info1, boolParam, boolParam); // branch info1.accestoken empty
    info1.accessTokenId = "AccessTokenId";
    dataMgr->InsertAutoStartupData(info1, boolParam, boolParam); // branch info1.accestoken empty

    AutoStartupInfo info2;
    info2.userId = int32Param;
    dataMgr->UpdateAutoStartupData(info2, boolParam, boolParam); // branch info2.bundleName empty
    info2.bundleName = "com.example.fuzzTest";
    dataMgr->UpdateAutoStartupData(info2, boolParam, boolParam); // branch info2.abilityName empty
    info2.abilityName = "MainAbility";
    dataMgr->UpdateAutoStartupData(info2, boolParam, boolParam); // branch info2.accestoken empty
    info2.accessTokenId = "AccessTokenId";
    dataMgr->UpdateAutoStartupData(info2, boolParam, boolParam); // branch info2.accestoken empty

    AutoStartupInfo info3;
    info3.userId = int32Param;
    dataMgr->UpdateAutoStartupData(info3, boolParam, boolParam); // branch info3.bundleName empty
    info3.bundleName = "com.example.fuzzTest";
    dataMgr->UpdateAutoStartupData(info3, boolParam, boolParam); // branch info3.abilityName empty
    info3.abilityName = "MainAbility";
    dataMgr->UpdateAutoStartupData(info3, boolParam, boolParam); // branch info3.accestoken empty
    info3.accessTokenId = "AccessTokenId";
    dataMgr->UpdateAutoStartupData(info3, boolParam, boolParam); // branch userid

    dataMgr->DeleteAutoStartupData(stringParam, int32Param); // called

    AutoStartupInfo info4;
    info4.userId = int32Param;
    dataMgr->QueryAutoStartupData(info4); // branch info3.bundleName empty
    info4.bundleName = "com.example.fuzzTest";
    dataMgr->QueryAutoStartupData(info4); // branch info3.abilityName empty
    info4.abilityName = "MainAbility";
    dataMgr->QueryAutoStartupData(info4); // branch info3.accestoken empty
    info4.accessTokenId = "AccessTokenId";
    dataMgr->QueryAutoStartupData(info4); // branch userid

    std::vector<AutoStartupInfo> vecs;
    vecs.emplace_back(info1);
    dataMgr->QueryAllAutoStartupApplications(vecs, int32Param); // called
    dataMgr->GetCurrentAppAutoStartupData(stringParam, vecs, stringParam); //called
}

void AbilityAutoStartupDataManagerFuzztest2(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    std::shared_ptr<AbilityAutoStartupDataManager> dataMgr = std::make_shared<AbilityAutoStartupDataManager>();
    AutoStartupInfo info1;
    info1.userId = int32Param;
    dataMgr->ConvertAutoStartupStatusToValue(boolParam, boolParam, info1.abilityName);

    DistributedKv::Value value1(jsonStr1);
    dataMgr->ConvertAutoStartupStatusFromValue(value1, boolParam, boolParam); // branch json
    DistributedKv::Value value2(jsonStr2);
    dataMgr->ConvertAutoStartupStatusFromValue(value2, boolParam, boolParam); // branch json
    DistributedKv::Value value3(jsonStr3);
    dataMgr->ConvertAutoStartupStatusFromValue(value3, boolParam, boolParam); // branch json
    DistributedKv::Value value4(jsonStr4);
    dataMgr->ConvertAutoStartupStatusFromValue(value4, boolParam, boolParam); // branch discard jsonstr
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    AbilityAutoStartupDataManagerFuzztest1(boolParam, stringParam, int32Param);
    AbilityAutoStartupDataManagerFuzztest2(boolParam, stringParam, int32Param);
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

