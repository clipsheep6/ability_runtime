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

#include "disconnectability_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_connect_callback.h"
#include "ability_context_impl.h"
#include "parcel.h"
#include "want.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
constexpr size_t FOO_MAX_LEN = 1024;
class AbilityConnectCallbackFuzz : public AbilityRuntime::AbilityConnectCallback {
public:
    explicit AbilityConnectCallbackFuzz() {};
    virtual ~AbilityConnectCallbackFuzz() {};
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override {};
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override {};
};
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AbilityRuntime::AbilityContextImpl* context = new AbilityRuntime::AbilityContextImpl();
    if (!context) {
        return false;
    }

    // fuzz for want
    Parcel wantParcel;
    Want *want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
    }

    // fuzz for connection
    sptr<AbilityConnectCallbackFuzz> connection = new AbilityConnectCallbackFuzz();

    if (want && connection) {
        context->DisconnectAbility(*want, connection);
    }

    if (want) {
        delete want;
        want = nullptr;
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
    if (size == 0 || size > OHOS::FOO_MAX_LEN) {
        std::cout << "invalid size" << std::endl;
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

