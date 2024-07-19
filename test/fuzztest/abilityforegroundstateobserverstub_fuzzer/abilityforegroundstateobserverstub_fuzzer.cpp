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

#include "abilityforegroundstateobserverstub_fuzzer.h"

#define private public
#include "ability_foreground_state_observer_stub.h"
#include "ability_foreground_state_observer_interface.h"
#undef private

#include "securec.h"
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
}

class AbilityForegroundStateObserverStubFUZZ : public AbilityForegroundStateObserverStub {
public:
    AbilityForegroundStateObserverStubFUZZ() = default;
    virtual ~AbilityForegroundStateObserverStubFUZZ() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override{};
};

const std::u16string  DECLARE_INTERFACE_DESCRIPTOR_TOKEN = u"ohos.appexecfwk.IAbilityForegroundStateObserver";

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IAbilityForegroundStateObserver::Message::ON_ABILITY_STATE_CHANGED);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(DECLARE_INTERFACE_DESCRIPTOR_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AbilityForegroundStateObserverStub> JsStub =
        std::make_shared<AbilityForegroundStateObserverStubFUZZ>();
    JsStub->OnRemoteRequest(code, parcel, reply, option);
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    RemoteDiedHandler handler;
    std::shared_ptr<AbilityForegroundStateObserverRecipient> infos =
        std::make_shared<AbilityForegroundStateObserverRecipient>(handler);
    wptr<IRemoteObject> remote;
    infos->OnRemoteDied(remote);
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

