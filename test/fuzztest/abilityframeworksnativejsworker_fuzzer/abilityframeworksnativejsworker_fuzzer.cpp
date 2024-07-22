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

#include "abilityframeworksnativejsworker_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "js_worker.h"
#undef private

#include "ability_record.h"
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
constexpr uint8_t ENABLE = 2;
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
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect("/data/test/codePath");
    workerInfo->packagePathStr = "/data/test/packagePath";
    workerInfo->hapPath = panda::panda_file::StringPacProtect("/data/test/hapPath");
    workerInfo->moduleName = "moduleName";
    AbilityRuntime::AssetHelper helper = AbilityRuntime::AssetHelper(workerInfo);
    std::string jsonStr(data, size);
    uint8_t *buff = nullptr;
    size_t buffSize;
    helper.GetSafeData(jsonStr, &buff, &buffSize);
    helper.NormalizedFileName(jsonStr);
    bool useSecureMem = *data % ENABLE;
    bool isRestricted = *data % ENABLE;
    std::vector<uint8_t> content;
    helper.ReadAmiData(jsonStr, &buff, &buffSize, content, useSecureMem, isRestricted);
    helper.ReadFilePathData(jsonStr, &buff, &buffSize, content, useSecureMem, isRestricted);
    helper.GetAmi(jsonStr, jsonStr);
    AbilityRuntime::GetContainerId();
    bool isDebugApp = *data % ENABLE;
    bool isNativeStart = *data % ENABLE;
    AbilityRuntime::StartDebuggerInWorkerModule(isDebugApp, isNativeStart);
    NativeEngine *nativeEngine = nullptr;
    AbilityRuntime::InitWorkerFunc(nativeEngine);
    AbilityRuntime::OffWorkerFunc(nativeEngine);
    int32_t id = static_cast<int32_t>(GetU32Data(data));
    AbilityRuntime::UpdateContainerScope(id);
    AbilityRuntime::RestoreContainerScope(id);
    AbilityRuntime::SetJsFramework();
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

