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

#include "abilitycachemanagera_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_cache_manager.h"
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

void AbilityCacheManagerFuzztest1(bool boolParam, std::string &stringParam, int32_t int32Param)
{
    AbilityCacheManager& mgr = AbilityCacheManager::GetInstance();
    mgr.Init(int32Param, int32Param);
    std::shared_ptr<AbilityRecord> abilityRecord1;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord2->recordId_ = 2; // 2 means recordId
    std::shared_ptr<AbilityRecord> abilityRecord3 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord3->recordId_ = 3; // 3 means recordId
    mgr.AddToProcLru(abilityRecord2);
    mgr.AddToDevLru(abilityRecord2, abilityRecord2);
    mgr.AddToProcLru(abilityRecord3);
    mgr.AddToDevLru(abilityRecord3, abilityRecord3);
    mgr.Put(abilityRecord1);
    mgr.Remove(abilityRecord1);
    mgr.Put(abilityRecord2);
    mgr.Remove(abilityRecord2);
    AbilityRequest abilityRequest;
    SetElement(abilityRequest.want);
    bool ret = mgr.IsRecInfoSame(abilityRequest, abilityRecord2);
    abilityInfo.moduleName = "moduleName";
    abilityRequest.abilityInfo.moduleName = abilityInfo.moduleName;
    SetElement(want);
    std::shared_ptr<AbilityRecord> abilityRecord4 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    ret = mgr.IsRecInfoSame(abilityRequest, abilityRecord4);
    abilityInfo.moduleName = "moduleName1";
    mgr.IsRecInfoSame(abilityRequest, abilityRecord4);
    mgr.AddToProcLru(abilityRecord4);
    mgr.GetAbilityRecInProcList(abilityRequest);
    abilityRequest.appInfo.accessTokenId = applicationInfo.accessTokenId;
    mgr.GetAbilityRecInProcList(abilityRequest);
    mgr.Get(abilityRequest);
    mgr.FindRecordByToken(nullptr);
    sptr<Token> token = GetFuzzAbilityToken();
    mgr.FindRecordByToken(token);
    mgr.GetAbilityList();
    mgr.FindRecordBySessionId(stringParam);
    mgr.FindRecordByServiceKey(stringParam);
    mgr.RemoveLauncherDeathRecipient();
    mgr.SignRestartAppFlag(stringParam);
    mgr.DeleteInvalidServiceRecord(stringParam);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    AbilityCacheManagerFuzztest1(boolParam, stringParam, int32Param);
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

