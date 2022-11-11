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

#include "abilitymgrrest_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "ability_interceptor.h"
#include "app_no_response_disposer.h"
#include "implicit_start_processor.h"
#include "system_dialog_scheduler.h"
#undef private
#include "inner_mission_info.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << OFFSET_ZERO) | (ptr[1] << OFFSET_ONE) | (ptr[2] << OFFSET_TWO) | ptr[3];
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
    int timeout = static_cast<int>(GetU32Data(data));
    std::shared_ptr<AppNoResponseDisposer> appNoResponseDisposer = std::make_shared<AppNoResponseDisposer>(timeout);
    int pid = static_cast<int>(GetU32Data(data));
    AAFwk::AppNoResponseDisposer::SetMissionClosure task;
    AAFwk::AppNoResponseDisposer::ShowDialogClosure showDialogTask;
    appNoResponseDisposer->DisposeAppNoResponse(pid, task, showDialogTask);
    std::string bundleName(data, size);
    appNoResponseDisposer->PostTimeoutTask(pid, bundleName);
    Parcel wantParcel;
    Want *want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
    }
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<SystemDialogScheduler> systemDialogScheduler = std::make_shared<SystemDialogScheduler>();
    systemDialogScheduler->GetANRDialogWant(static_cast<int>(userId), pid, *want);
    std::vector<DialogAppInfo> dialogAppInfos;
    systemDialogScheduler->GetSelectorParams(dialogAppInfos);
    int32_t labelId = static_cast<int32_t>(GetU32Data(data));
    std::string appName(data, size);
    systemDialogScheduler->GetAppNameFromResource(labelId, bundleName, userId, appName);
    InnerMissionInfo innerMissionInfo;
    innerMissionInfo.ToJsonStr();
    std::string jsonStr(data, size);
    innerMissionInfo.FromJsonStr(jsonStr);
    std::vector<std::string> info;
    innerMissionInfo.Dump(info);
    nlohmann::json value;
    std::string node(data, size);
    JsonType jsonType = JsonType::STRING;
    return innerMissionInfo.CheckJsonNode(value, node, jsonType);
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

    char* ch = (char *)malloc(size + 1);
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

