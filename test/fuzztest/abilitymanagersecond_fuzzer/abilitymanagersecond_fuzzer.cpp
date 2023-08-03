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

#include "abilitymanagersecond_fuzzer.h"

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
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_MISSION_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_LOCK_MODE_STATE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MINIMIZE_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::LOCK_MISSION_FOR_CLEANUP));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNLOCK_MISSION_FOR_CLEANUP));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_MISSION_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNREGISTER_MISSION_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_INFOS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_INFO_BY_ID));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLEAN_MISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLEAN_ALL_MISSIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_FRONT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_SNAPSHOT_BY_ID));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_USER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::STOP_USER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_ABILITY_CONTROLLER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::IS_USER_A_STABILITY_TEST));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_MISSION_LABEL));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DO_ABILITY_FOREGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DO_ABILITY_BACKGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSION_TO_FRONT_BY_OPTIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_ID_BY_ABILITY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_MISSION_ICON));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DUMP_ABILITY_INFO_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::STOP_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_COMPONENT_INTERCEPTION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SEND_ABILITY_RESULT_BY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_ROOT_SCENE_SESSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::PREPARE_TERMINATE_ABILITY));
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

