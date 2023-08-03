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

#include "abilitymanagerthird_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_service.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t INPUT_EIGHT = 8;
constexpr size_t INPUT_SIXTEEN = 16;
constexpr size_t INPUT_TWENTYFOUR = 24;
}
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
std::map<int, int> codeMap_;

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INDEX_ZERO] << INPUT_TWENTYFOUR) | (ptr[INDEX_ONE] << INPUT_SIXTEEN) |
        (ptr[INDEX_TWO] << INPUT_EIGHT) | ptr[INDEX_THREE];
}

void EmplaceCodeMap()
{
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::COMMAND_ABILITY_WINDOW_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CALL_ABILITY_BY_SCB));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_ABILITY_TO_BACKGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONNECT_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DISCONNECT_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::STOP_SERVICE_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY_ADD_CALLER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_SENDER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SEND_PENDING_WANT_SENDER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CANCEL_PENDING_WANT_SENDER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_UID));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_BUNDLENAME));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_USERID));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_TYPE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_CODE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_CANCEL_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNREGISTER_CANCEL_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_REQUEST_WANT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PENDING_WANT_SENDER_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_SHOW_ON_LOCK_SCREEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SEND_APP_NOT_RESPONSE_PROCESS_ID));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY_FOR_SETTINGS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_ABILITY_MISSION_SNAPSHOT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_APP_MEMORY_SIZE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::IS_RAM_CONSTRAINED_DEVICE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_ABILITY_RUNNING_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_EXTENSION_RUNNING_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PROCESS_RUNNING_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLEAR_UP_APPLICATION_DATA));
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    if (codeMap_.size() == 0) {
        EmplaceCodeMap();
    }
    uint32_t code = GetU32Data(data) % codeMap_.size();
    code = codeMap_[code];

    MessageParcel parcel;
    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    parcel.WriteBuffer(data, size);
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    DelayedSingleton<AbilityManagerService>::GetInstance()->OnRemoteRequest(code, parcel, reply, option);

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

    char* ch = static_cast<char*>(malloc(size + 1));
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

