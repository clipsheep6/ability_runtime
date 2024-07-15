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

#include "abilityautostartupdatamanagerb_fuzzer.h"

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

const std::string jsonKey1 = "{\n\"bundleName\": \"com.example.fuzzTest\", \n\"moduleName\": \"module\",\
    \n\"abilityName\": \"ability\", \n\"appCloneIndex\": -1, \n\"accessTokenId\": \"token\",\
    \n\"userId\": 100 }";
const std::string jsonValue1 = "{\n\"abilityTypeName\": \"typeName\"}";
const std::string jsonValue1Illegal = "{\n\"abilityTypeName\": \"typeName\"";

const std::string jsonKey2 = "{\n\"bundleName2\": \"com.example.fuzzTest2\"}";
const std::string jsonValue2 = "{\n\"abilityTypeName2\": \"typeName\"}";
const std::string jsonValue3 = "{\n\"abilityTypeName\": 1 }";
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
    AutoStartupInfo info;
    info.bundleName = "com.example.fuzzTest";
    info.moduleName = "stringParam";
    info.abilityName = "MainAbility";
    info.appCloneIndex = int32Param;
    info.accessTokenId = "accessTokenId";
    info.userId = int32Param;
    dataMgr->ConvertAutoStartupDataToKey(info);

    DistributedKv::Key key1(jsonKey1);
    DistributedKv::Key key1Illegal(jsonValue1Illegal);
    DistributedKv::Value value1(jsonValue1);
    dataMgr->ConvertAutoStartupInfoFromKeyAndValue(key1, value1);
    dataMgr->ConvertAutoStartupInfoFromKeyAndValue(key1Illegal, value1);

    DistributedKv::Key key2(jsonKey2);
    DistributedKv::Value value2(jsonValue2);
    DistributedKv::Value value2Illegal(jsonValue1Illegal);
    DistributedKv::Value value3(jsonValue3);
    dataMgr->ConvertAutoStartupInfoFromKeyAndValue(key2, value2);
    dataMgr->ConvertAutoStartupInfoFromKeyAndValue(key2, value2Illegal);
    dataMgr->ConvertAutoStartupInfoFromKeyAndValue(key2, value3);

    nlohmann::json jsonObject = nlohmann::json::parse(key1.ToString(), nullptr, false);
    dataMgr->IsEqual(jsonObject, "abilityName", "ability", boolParam); // branch,return true
    dataMgr->IsEqual(jsonObject, "abilityName", "NaN", boolParam); // branch,return true
    dataMgr->IsEqual(jsonObject, "userId", "ability", boolParam); // branch
    dataMgr->IsEqual(jsonObject, "NaN", "ability", boolParam); // branch

    dataMgr->IsEqual(jsonObject, "userId", 100); // 100 means userid,branch
    dataMgr->IsEqual(jsonObject, "userId", int32Param); // branch
    dataMgr->IsEqual(jsonObject, "NaN", int32Param);
    dataMgr->IsEqual(key1, info);
    dataMgr->IsEqual(key2, info);
    dataMgr->IsEqual(key1Illegal, info);

    dataMgr->IsEqual(key1, "token");
    dataMgr->IsEqual(key1, "NaN");
    dataMgr->IsEqual(key1Illegal, "token");

    dataMgr->IsEqual(key1, 100); // 100 means userid
    dataMgr->IsEqual(key1, int32Param);
    dataMgr->IsEqual(key1Illegal, 0);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    AbilityAutoStartupDataManagerFuzztest1(boolParam, stringParam, int32Param);
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

