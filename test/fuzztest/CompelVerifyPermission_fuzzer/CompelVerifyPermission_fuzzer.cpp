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

#include "CompelVerifyPermission_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_client.h"

using namespace OHOS::AAFwk;

namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        auto abilityMgr = AbilityManagerClient::GetInstance();
        if (!abilityMgr) {
            std::cout << "Get ability manager client failed." << std::endl;
            return false;
        }

        std::string message;
        abilityMgr->CompelVerifyPermission(reinterpret_cast<const char*>(data), 0, 0, message);
        abilityMgr->CompelVerifyPermission(reinterpret_cast<const char*>(data), 0, 100, message);
        abilityMgr->CompelVerifyPermission(reinterpret_cast<const char*>(data), 100, 100, message);

        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

