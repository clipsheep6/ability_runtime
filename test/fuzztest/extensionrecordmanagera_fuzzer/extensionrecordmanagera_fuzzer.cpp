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

#include "extensionrecordmanagera_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "extension_record_manager.h"
#undef protected
#undef private

#include "ability_record.h"
#include "extension_record.h"
#include "extension_record_factory.h"

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

void FuzztestExtensionRecordManagerFunc1(std::shared_ptr<ExtensionRecordManager> mgr, bool boolParam,
    const std::string &stringParam, int32_t int32Param)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<ExtensionRecord> record = std::make_shared<ExtensionRecord>(abilityRecord);
    mgr->GenerateExtensionRecordId(int32Param);
    mgr->AddExtensionRecord(int32Param, record);
    mgr->RemoveExtensionRecord(int32Param);
    mgr->AddExtensionRecordToTerminatedList(int32Param);
    mgr->AddExtensionRecordToTerminatedList(1); // 1 means valid id, construct exist recordId
    mgr->AddExtensionRecord(1, record); // 1 means valid id
    std::shared_ptr<ExtensionRecord> extensionRecord = std::make_shared<ExtensionRecord>();
    mgr->GetExtensionRecord(1, stringParam, extensionRecord, boolParam);  // 1 means valid id
    mgr->GetExtensionRecord(int32Param, stringParam, extensionRecord, boolParam);
    AppExecFwk::AbilityInfo abilityInfo;
    mgr->IsBelongToManager(abilityInfo);
    std::vector<std::string> extensionList;
    mgr->GetActiveUIExtensionList(int32Param, extensionList);
    mgr->GetActiveUIExtensionList(stringParam, extensionList);
    mgr->GetAbilityRecordBySessionInfo(nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->uiExtensionComponentId = int32Param;
    mgr->AddExtensionRecord(int32Param, nullptr);
    record->abilityRecord_ = abilityRecord;
    mgr->AddExtensionRecord(int32Param, record);
    record->abilityRecord_->sessionInfo_ = sessionInfo;
    mgr->GetAbilityRecordBySessionInfo(sessionInfo);
    mgr->extensionRecords_.clear();
    mgr->GetAbilityRecordBySessionInfo(sessionInfo);
}

void FuzztestExtensionRecordManagerFunc2(std::shared_ptr<ExtensionRecordManager> mgr, bool boolParam,
    const std::string &stringParam, int32_t int32Param)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<ExtensionRecord> record = std::make_shared<ExtensionRecord>(abilityRecord);
    AAFwk::AbilityRequest abilityRequest;
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    sessionInfo->uiExtensionComponentId = int32Param;
    mgr->AddExtensionRecord(int32Param, nullptr);
    record->abilityRecord_ = abilityRecord;
    mgr->AddExtensionRecord(int32Param, record);
    record->abilityRecord_->sessionInfo_ = sessionInfo;
    mgr->IsHostSpecifiedProcessValid(abilityRequest, record, stringParam);

    record->processMode_ = PROCESS_MODE_INSTANCE;
    mgr->UpdateProcessName(abilityRequest, record);
    record->processMode_ = PROCESS_MODE_TYPE;
    mgr->UpdateProcessName(abilityRequest, record);
    record->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    mgr->UpdateProcessName(abilityRequest, record);

    mgr->GetHostBundleNameForExtensionId(1, "bundleName"); // 1 means id
    mgr->AddExtensionRecord(1, record); // 1 means id
    mgr->GetHostBundleNameForExtensionId(1, "bundleName"); // 1 means id, exist.
    mgr->GetHostBundleNameForExtensionId(int32Param, "bundleName"); // 1 means id, exist.
    abilityRecord->SetUIExtensionAbilityId(-1);
    mgr->AddExtensionRecord(1, record); // 1 means id
    abilityRecord->SetUIExtensionAbilityId(1);
    mgr->AddPreloadUIExtensionRecord(abilityRecord);
    PreLoadUIExtensionMapKey key;
    mgr->RemoveAllPreloadUIExtensionRecord(key); // called
    mgr->IsPreloadExtensionRecord(abilityRequest, stringParam, record, boolParam);  // called
    mgr->RemovePreloadUIExtensionRecordById(key, int32Param);  // called
    mgr->RemovePreloadUIExtensionRecord(key);  // called
    mgr->GetOrCreateExtensionRecordInner(abilityRequest, stringParam, record, boolParam);  // called
    mgr->StartAbility(abilityRequest); // called
    mgr->IsFocused(int32Param, nullptr);  // called
    mgr->AddExtensionRecord(0, record); // 1 means id
    mgr->GetRootCallerTokenLocked(int32Param);
    mgr->CreateExtensionRecord(abilityRequest, stringParam, record, int32Param);
    mgr->GetUIExtensionRootHostInfo(nullptr);
    mgr->GetUIExtensionRootHostInfo(token);
    mgr->extensionRecords_.clear();
}

void FuzztestExtensionRecordManagerFunc3(std::shared_ptr<ExtensionRecordManager> mgr, bool boolParam,
    const std::string &stringParam, int32_t int32Param)
{
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    std::shared_ptr<ExtensionRecord> record = std::make_shared<ExtensionRecord>(abilityRecord);
    AAFwk::AbilityRequest abilityRequest;
    UIExtensionSessionInfo uiExtensionSessionInfo;
    mgr->GetUIExtensionSessionInfo(nullptr, uiExtensionSessionInfo);
    mgr->AddExtensionRecord(0, record);
    mgr->AddExtensionRecord(1, record); // 1 means id
    mgr->GetExtensionRecordById(int32Param); // called
    MGR->LoadTimeout(int32Param); // called
    mgr->ForegroundTimeout(int32Param); // called
    mgr->BackgroundTimeout(int32Param); // called
    mgr->TerminateTimeout(int32Param); // called
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    std::string stringParam(data, size);
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<ExtensionRecordManager> mgr = std::make_shared<ExtensionRecordManager>(100); // 100 mainUserId
    FuzztestExtensionRecordManagerFunc1(mgr, boolParam, stringParam, int32Param);
    FuzztestExtensionRecordManagerFunc2(mgr, boolParam, stringParam, int32Param);
    FuzztestExtensionRecordManagerFunc3(mgr, boolParam, stringParam, int32Param);
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

