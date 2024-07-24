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

#include "abilityapppreloader_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "app_preloader.h"
#include "bundle_mgr_helper.h"
#undef protected
#undef private
#include "parcel.h"
#include <iostream>
#include "securec.h"
#include "configuration.h"

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

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    auto bundleMgrHelper = std::make_shared<AppExecFwk::BundleMgrHelper>();
    remoteClientManager->SetBundleManagerHelper(bundleMgrHelper);
    auto appPreloader = std::make_shared<AppPreloader>(remoteClientManager);
    std::string bundleName(data, size);
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data));
    PreloadRequest request;
    appPreloader->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    appPreloader->GetBundleManagerHelper();
    AbilityInfo abilityInfo;
    appPreloader->CheckPreloadConditions(abilityInfo);
    AAFwk::Want launchWant;
    appPreloader->GetLaunchWant(bundleName, userId, launchWant);
    appPreloader->GetLaunchAbilityInfo(launchWant, userId, abilityInfo);
    appPreloader->PreCheck(bundleName, PreloadMode::PRE_MAKE);
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

