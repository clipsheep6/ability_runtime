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

#include "extensionrecordmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "extension_record_manager.h"
#undef private

#include "ability_record.h"

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
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    auto extensionRecordManager = std::make_shared<AbilityRuntime::ExtensionRecordManager>(int32Param);
    extensionRecordManager->GenerateExtensionRecordId(int32Param);
    std::shared_ptr<AbilityRuntime::ExtensionRecord> record;
    extensionRecordManager->AddExtensionRecord(int32Param, record);
    extensionRecordManager->RemoveExtensionRecord(int32Param);
    extensionRecordManager->AddExtensionRecordToTerminatedList(int32Param);
    AppExecFwk::AbilityInfo abilityInfo;
    extensionRecordManager->IsBelongToManager(abilityInfo);
    auto focusToken = GetFuzzAbilityToken();
    extensionRecordManager->IsFocused(int32Param, focusToken);
    std::vector<std::string> extensionList;
    extensionRecordManager->GetActiveUIExtensionList(int32Param, extensionList);
    std::string strParam(data, size);
    extensionRecordManager->GetActiveUIExtensionList(strParam, extensionList);
    AAFwk::AbilityRequest abilityRequest;
    extensionRecordManager->StartAbility(abilityRequest);
    std::shared_ptr<AbilityRuntime::ExtensionRecord> extensionRecord;
    extensionRecordManager->CreateExtensionRecord(abilityRequest, strParam, extensionRecord, int32Param);
    bool boolParam = *data % ENABLE;
    extensionRecordManager->IsPreloadExtensionRecord(abilityRequest, strParam, extensionRecord, boolParam);
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord;
    extensionRecordManager->AddPreloadUIExtensionRecord(abilityRecord);
    AbilityRuntime::ExtensionRecordManager::PreLoadUIExtensionMapKey preLoadUIExtensionInfo;
    extensionRecordManager->RemoveAllPreloadUIExtensionRecord(preLoadUIExtensionInfo);
    std::tuple<std::string, std::string, std::string, std::string> extensionRecordMapKey;
    extensionRecordManager->RemovePreloadUIExtensionRecord(extensionRecordMapKey);
    extensionRecordManager->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, int32Param);
    extensionRecordManager->GetOrCreateExtensionRecord(abilityRequest, strParam, abilityRecord, boolParam);
    sptr<AAFwk::SessionInfo> sessionInfo;
    extensionRecordManager->GetAbilityRecordBySessionInfo(sessionInfo);
    auto token = GetFuzzAbilityToken();
    extensionRecordManager->GetUIExtensionRootHostInfo(token);
    UIExtensionSessionInfo uiExtensionSessionInfo;
    extensionRecordManager->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    extensionRecordManager->LoadTimeout(int32Param);
    extensionRecordManager->ForegroundTimeout(int32Param);
    extensionRecordManager->BackgroundTimeout(int32Param);
    extensionRecordManager->TerminateTimeout(int32Param);
    extensionRecordManager->GetHostBundleNameForExtensionId(int32Param, strParam);
    extensionRecordManager->GetRootCallerTokenLocked(int32Param);
    extensionRecordManager->GetOrCreateExtensionRecordInner(abilityRequest, strParam, extensionRecord, boolParam);
    extensionRecordManager->IsHostSpecifiedProcessValid(abilityRequest, record, strParam);
    std::list<sptr<IRemoteObject>> callerList;
    mgr->GetCallerTokenList(int32Param, callerList);

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