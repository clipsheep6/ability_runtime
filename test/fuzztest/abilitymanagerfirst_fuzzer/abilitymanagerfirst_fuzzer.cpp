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

#include "abilitymanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_service.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
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
    return (ptr[0] << INPUT_TWENTYFOUR) | (ptr[1] << INPUT_SIXTEEN) | (ptr[2] << INPUT_EIGHT) | ptr[3];
}

void EmplaceCodeMap()
{
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::TERMINATE_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ATTACH_ABILITY_THREAD));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ABILITY_TRANSITION_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONNECT_ABILITY_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DISCONNECT_ABILITY_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ADD_WINDOW_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::LIST_STACK_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_RECENT_MISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REMOVE_MISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REMOVE_STACK));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::COMMAND_ABILITY_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_SNAPSHOT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ACQUIRE_DATA_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::RELEASE_DATA_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_TOP));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::KILL_PROCESS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNINSTALL_APP));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_FLOATING_STACK));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_SPLITSCREEN_STACK));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CHANGE_FOCUS_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MINIMIZE_MULTI_WINDOW));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MAXIMIZE_MULTI_WINDOW));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_FLOATING_MISSIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLOSE_MULTI_WINDOW));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_END));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::COMPEL_VERIFY_PERMISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::POWER_OFF));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::POWER_ON));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::LUCK_MISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNLUCK_MISSION));
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

