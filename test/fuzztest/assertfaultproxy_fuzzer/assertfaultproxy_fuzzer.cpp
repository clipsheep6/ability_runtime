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

#include "assertfaultproxy_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "assert_fault_proxy.h"
#undef private
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

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

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AAFwk::UserStatus status = AAFwk::ASSERT_TERMINATE;
    wptr<IRemoteObject> remote;
    sptr<IRemoteObject> impl;
    auto assertFaultProxy = std::make_shared<AbilityRuntime::AssertFaultProxy>(impl);
    assertFaultProxy->NotifyDebugAssertResult(status);

    AbilityRuntime::AssertFaultRemoteDeathRecipient::RemoteDiedHandler handler;
    auto assertFaultRemoteDeathRecipient =
        std::make_shared<AbilityRuntime::AssertFaultRemoteDeathRecipient>(handler);
    assertFaultRemoteDeathRecipient->OnRemoteDied(remote);

    auto modalSystemAssertUIExtension = std::make_shared<AbilityRuntime::ModalSystemAssertUIExtension>();
    Want want;
    modalSystemAssertUIExtension->CreateModalUIExtension(want);

    auto assertDialogConnection =
        std::make_shared<AbilityRuntime::ModalSystemAssertUIExtension::AssertDialogConnection>();
    assertDialogConnection->SetReqeustAssertDialogWant(want);
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject;
    int intParam = static_cast<int>(GetU32Data(data));
    assertDialogConnection->OnAbilityConnectDone(element, remoteObject, intParam);
    assertDialogConnection->OnAbilityDisconnectDone(element, intParam);
    modalSystemAssertUIExtension->DisconnectSystemUI();
    modalSystemAssertUIExtension->TryNotifyOneWaitingThread();
    modalSystemAssertUIExtension->TryNotifyOneWaitingThreadInner();
    modalSystemAssertUIExtension->GetConnection();
    modalSystemAssertUIExtension->dialogConnectionCallback_ = nullptr;
    modalSystemAssertUIExtension->GetConnection();

    return true;
}
}  // namespace OHOS

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

