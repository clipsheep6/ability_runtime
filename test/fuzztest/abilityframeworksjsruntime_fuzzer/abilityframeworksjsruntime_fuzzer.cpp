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

#include "abilityframeworksjsruntime_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "js_runtime.h"
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
using AppLibPathMap = std::map<std::string, std::vector<std::string>>;
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
bool DoSomethingInterestingWithMyAPIadd(const char* data, size_t size)
{
    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    bool needBreakPoint = *data % ENABLE;
    uint32_t instanceId = static_cast<uint32_t>(GetU32Data(data));
    jsRuntime->StartDebugger(needBreakPoint, instanceId);
    jsRuntime->StopDebugger();
    jsRuntime->FinishPreload();
    jsRuntime->GetEcmaVm();
    jsRuntime->GetNativeEnginePointer();
    AbilityRuntime::Runtime::DebugOption debugOptionStart;
    jsRuntime->StartProfiler(debugOptionStart);
    jsRuntime->ReloadFormComponent();
    jsRuntime->DoCleanWorkAfterStageCleaned();
    jsRuntime->Deinitialize();
    jsRuntime->InitLoop();
    jsRuntime->SetRequestAotCallback();
    std::map<std::string, std::string> contextInfoMap;
    std::map<std::string, std::vector<std::vector<std::string>>> pkgContextInfoMap;
    std::map<std::string, std::string> pkgAliasMap;
    jsRuntime->GetPkgContextInfoListMap(contextInfoMap, pkgContextInfoMap, pkgAliasMap);
    jsRuntime->InitConsoleModule();
    jsRuntime->InitTimerModule();
    AbilityRuntime::Runtime::Options optionsImpl;
    jsRuntime->InitWorkerModule(optionsImpl);
    jsRuntime->LoadAotFile(optionsImpl);
    std::map<std::string, std::string> moduleAndPath;
    jsRuntime->RegisterQuickFixQueryFunc(moduleAndPath);
    return true;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    auto jsRuntime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsRuntime->GetChildOptions();
    jsRuntime->GetLanguage();
    jsRuntime->GetNativeEngine();
    jsRuntime->GetNapiEnv();
    auto task = []() {};
    std::string jsonStr(data, size);
    int64_t delayTime = static_cast<int64_t>(GetU32Data(data));
    jsRuntime->PostTask(task, jsonStr, delayTime);
    jsRuntime->PostSyncTask(task, jsonStr);
    jsRuntime->RemoveTask(jsonStr);
    bool isPrivate = *data % ENABLE;
    jsRuntime->DumpHeapSnapshot(isPrivate);
    jsRuntime->DumpCpuProfile();
    jsRuntime->DestroyHeapProfiler();
    jsRuntime->ForceFullGC();
    uint32_t tid = static_cast<uint32_t>(GetU32Data(data));
    jsRuntime->ForceFullGC(tid);
    bool isFullGC = *data % ENABLE;
    jsRuntime->DumpHeapSnapshot(tid, isFullGC);
    jsRuntime->AllowCrossThreadExecution();
    jsRuntime->GetHeapPrepare();
    bool isBackground = *data % ENABLE;
    jsRuntime->NotifyApplicationState(isBackground);
    jsRuntime->SuspendVM(tid);
    jsRuntime->ResumeVM(tid);
    jsRuntime->UpdateModuleNameAndAssetPath(jsonStr);
    jsRuntime->PreloadSystemModule(jsonStr);
    AbilityRuntime::Runtime::DebugOption debugOptionMode;
    debugOptionMode.isStartWithDebug = true;
    debugOptionMode.processName = "test";
    debugOptionMode.isDebugApp = true;
    debugOptionMode.isStartWithNative = false;
    jsRuntime->StartDebugMode(debugOptionMode);
    jsRuntime->StopDebugMode();
    jsRuntime->NotifyHotReloadPage();
    DoSomethingInterestingWithMyAPIadd(data, size);
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

