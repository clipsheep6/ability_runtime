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

#include "wantagenthelpertrigger_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "parcel.h"
#include "want_agent_info.h"
#include "want_agent_helper.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime::WantAgent;

#define DISABLE_FUZZ
namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
const std::string GET_BUNDLE_INFO_PERMISSION = "ohos.permission.GET_BUNDLE_INFO";
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    int requestCode = 0;
    WantAgentConstant::OperationType operationType = WantAgentConstant::OperationType::START_ABILITY;
    WantAgentConstant::Flags flag = WantAgentConstant::Flags::ONE_TIME_FLAG;
    std::shared_ptr<WantParams> extraInfo;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);
    int resultCode = 0;

    // get want agentInfo
    Parcel paramsParcel;
    WantParams* params = nullptr;
    if (paramsParcel.WriteBuffer(data, size)) {
        WantParams* params = WantParams::Unmarshalling(paramsParcel);
        if (params) {
            extraInfo = std::make_shared<WantParams>(*params);
        }
    }

    // get want agent
    std::shared_ptr<WantAgentInfo> paramInfo;
    WantAgentInfo wantAgentInfo(paramInfo);
    WantAgentInfo agentInfo(requestCode, operationType, flag, wants, extraInfo);
    std::shared_ptr<WantAgent> wantAgent = WantAgentHelper::GetWantAgent(agentInfo);
    if (wantAgent) {
        // trigger want agent
        TriggerInfo triggerInfo(GET_BUNDLE_INFO_PERMISSION, extraInfo, want, resultCode);
        WantAgentHelper::TriggerWantAgent(wantAgent, nullptr, triggerInfo);
    }

    if (params) {
        delete params;
        params = nullptr;
    }

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
#ifndef DISABLE_FUZZ
    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
#endif
    free(ch);
    ch = nullptr;
    return 0;
}