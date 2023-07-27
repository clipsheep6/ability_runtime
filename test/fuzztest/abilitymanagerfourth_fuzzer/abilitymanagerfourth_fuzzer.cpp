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

#include "abilitymanagerfourth_fuzzer.h"

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
}
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
std::map<int, int> codeMap_;

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

void EmplaceCodeMap()
{
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY_FOR_OPTIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::BLOCK_AMS_SERVICE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::BLOCK_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::BLOCK_APP_SERVICE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_CALL_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::RELEASE_CALL_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONNECT_ABILITY_WITH_TYPE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_UI_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CALL_REQUEST_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY_AS_CALLER_BY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_ABILITY_AS_CALLER_FOR_OPTIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MINIMIZE_UI_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::TERMINATE_UI_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONNECT_UI_EXTENSION_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_UI_ABILITY_BY_SCB));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MINIMIZE_UI_ABILITY_BY_SCB));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLOSE_UI_ABILITY_BY_SCB));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REQUEST_DIALOG_SERVICE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_SPECIFIED_ABILITY_BY_SCB));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SET_SESSIONMANAGERSERVICE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_CONTINUATION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::NOTIFY_CONTINUATION_RESULT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::NOTIFY_COMPLETE_CONTINUATION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONTINUE_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONTINUE_MISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SEND_RESULT_TO_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_REMOTE_ON_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_REMOTE_OFF_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONTINUE_MISSION_OF_BUNDLENAME));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_REMOTE_MISSION_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNREGISTER_REMOTE_MISSION_LISTENER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_SYNC_MISSIONS));
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

