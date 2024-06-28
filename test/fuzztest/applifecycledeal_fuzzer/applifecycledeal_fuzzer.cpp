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

#include "applifecycledeal_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "app_lifecycle_deal.h"
#include "ability_record.h"
#include "message_parcel.h"
#include "securec.h"

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
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }

    return token;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AppLifeCycleDeal appLifeCycleDeal;
    sptr<IAppScheduler> thread = nullptr;
    appLifeCycleDeal.SetApplicationClient(thread);
    std::shared_ptr<AbilityRunningRecord> ability = nullptr;
    appLifeCycleDeal.LaunchAbility(ability);
    AppLaunchData launchData;
    Configuration config;
    appLifeCycleDeal.LaunchApplication(launchData, config);
    HapModuleInfo abilityStage;
    appLifeCycleDeal.AddAbilityStage(abilityStage);
    int32_t timeLevel = static_cast<int32_t>(GetU32Data(data));
    appLifeCycleDeal.ScheduleTrimMemory(timeLevel);
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    appLifeCycleDeal.ScheduleMemoryLevel(level);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    appLifeCycleDeal.ScheduleCleanAbility(token);
    Want want;
    std::string bundleName(data, size);
    appLifeCycleDeal.ScheduleAcceptWant(want, bundleName);
    sptr<IQuickFixCallback> callback = nullptr;
    int32_t recordId = 0;
    appLifeCycleDeal.NotifyLoadRepairPatch(bundleName, callback, recordId);
    appLifeCycleDeal.NotifyHotReloadPage(callback, recordId);
    appLifeCycleDeal.NotifyUnLoadRepairPatch(bundleName, callback, recordId);
    appLifeCycleDeal.GetApplicationClient();
    appLifeCycleDeal.LowMemoryWarning();
    appLifeCycleDeal.ScheduleForegroundRunning();
    appLifeCycleDeal.ScheduleBackgroundRunning();
    appLifeCycleDeal.ScheduleProcessSecurityExit();
    appLifeCycleDeal.ScheduleTerminate();
    return (appLifeCycleDeal.UpdateConfiguration(config) == 0);
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
