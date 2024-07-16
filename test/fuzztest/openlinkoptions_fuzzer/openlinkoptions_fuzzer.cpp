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

#include "openlinkoptions_fuzzer.h"
#include "ability_record.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "open_link_options.h"
#undef private
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<OpenLinkOptions> openLinkOptions = std::make_shared<OpenLinkOptions>();
    if (!openLinkOptions) {
        return false;
    };
    Parcel parcel;
    WantParams wantParams;
    bool boolParam = *data % ENABLE;
    openLinkOptions->SetParameters(wantParams);
    openLinkOptions->ReadParameters(parcel);
    openLinkOptions->ReadFromParcel(parcel);
    openLinkOptions->GetParameters();
    openLinkOptions->WriteParameters(wantParams, parcel);
    openLinkOptions->SetAppLinkingOnly(boolParam);
    openLinkOptions->GetAppLinkingOnly();
    openLinkOptions->Marshalling(parcel);
    openLinkOptions->Unmarshalling(parcel);
    std::shared_ptr<OpenLinkOptions> openLinkOptions2 = std::make_shared<OpenLinkOptions>();
    if (!openLinkOptions2) {
        return false;
    };
    openLinkOptions = openLinkOptions2;

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

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}