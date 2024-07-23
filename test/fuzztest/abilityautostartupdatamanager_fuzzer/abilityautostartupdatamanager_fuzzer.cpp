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

#include "abilityautostartupdatamanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#define private public
#include "ability_auto_startup_data_manager.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr uint8_t ENABLE = 2;
constexpr size_t U32_AT_SIZE = 4;
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

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    auto abilityAutoStartupDataManager = std::make_shared<AbilityRuntime::AbilityAutoStartupDataManager>();
    AbilityRuntime::AutoStartupInfo info;
    bool isAutoStartup = *data % ENABLE;
    bool isEdmForce = *data % ENABLE;
    abilityAutoStartupDataManager->InsertAutoStartupData(info, isAutoStartup, isEdmForce);
    abilityAutoStartupDataManager->UpdateAutoStartupData(info, isAutoStartup, isEdmForce);
    std::string strParam(data, size);
    int32_t in32Param = static_cast<int32_t>(GetU32Data(data));
    abilityAutoStartupDataManager->QueryAutoStartupData(info);
    std::vector<AbilityRuntime::AutoStartupInfo> infoList;
    abilityAutoStartupDataManager->QueryAllAutoStartupApplications(infoList, in32Param);
    abilityAutoStartupDataManager->GetCurrentAppAutoStartupData(strParam, infoList, strParam);
    abilityAutoStartupDataManager->GetKvStore();
    abilityAutoStartupDataManager->CheckKvStore();
    DistributedKv::Value value;
    DistributedKv::Key key;
    abilityAutoStartupDataManager->ConvertAutoStartupStatusFromValue(value, isAutoStartup, isEdmForce);
    abilityAutoStartupDataManager->ConvertAutoStartupDataToKey(info);
    abilityAutoStartupDataManager->ConvertAutoStartupInfoFromKeyAndValue(key, value);
    nlohmann::json jsonObject;
    std::string keys(data, size);
    std::string values(data, size);
    bool checkEmpty = *data % ENABLE;
    abilityAutoStartupDataManager->IsEqual(jsonObject, keys, values, checkEmpty);
    abilityAutoStartupDataManager->IsEqual(jsonObject, keys, values);
    abilityAutoStartupDataManager->IsEqual(key, info);
    abilityAutoStartupDataManager->IsEqual(key, strParam);
    abilityAutoStartupDataManager->IsEqual(key, in32Param);
    abilityAutoStartupDataManager->DeleteAutoStartupData(info);
    abilityAutoStartupDataManager->DeleteAutoStartupData(strParam, in32Param);

    return true;
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

