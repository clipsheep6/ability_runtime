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

#include "finishusertest_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_client.h"
#include "app_mgr_client.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
namespace OHOS {
    bool DoSomethingInterestingWithMyAPI(const char* data)
    {
        auto abilityMgr = AbilityManagerClient::GetInstance();
        if (!abilityMgr) {
            std::cout << "Get ability manager client failed." << std::endl;
            return false;
        }

        int resultCode = 0;
        ErrCode ret = abilityMgr->FinishUserTest(data, resultCode, "com.ohos.launcher");
        if (ret != 0) {
            return false;
        }

        // fuzz for AppMgrClient
        AppMgrClient* appMgrClient = new AppMgrClient();
        if (!appMgrClient) {
            return false;
        }
        
        if (appMgrClient->FinishUserTest(data, resultCode, "com.ohos.launcher") != 0) {
            return false;
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

    OHOS::DoSomethingInterestingWithMyAPI(ch);
    free(ch);
    ch = nullptr;
    return 0;
}

