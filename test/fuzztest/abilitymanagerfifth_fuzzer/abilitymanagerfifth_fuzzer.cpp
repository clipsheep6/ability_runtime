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

#include "abilitymanagerfifth_fuzzer.h"

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
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::STOP_SYNC_MISSIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_SNAPSHOT_HANDLER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_MISSION_SNAPSHOT_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UPDATE_MISSION_SNAPSHOT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSIONS_TO_FOREGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::MOVE_MISSIONS_TO_BACKGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UPDATE_MISSION_SNAPSHOT_FROM_WMS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::START_USER_TEST));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::FINISH_USER_TEST));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DELEGATOR_DO_ABILITY_FOREGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DELEGATOR_DO_ABILITY_BACKGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_TOP_ABILITY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DUMP_STATE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DUMPSYS_STATE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::FORCE_TIMEOUT));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_WMS_HANDLER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::COMPLETEFIRSTFRAMEDRAWING));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::REGISTER_CONNECTION_OBSERVER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::UNREGISTER_CONNECTION_OBSERVER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_DLP_CONNECTION_INFOS));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_TOP_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::FREE_INSTALL_ABILITY_FROM_REMOTE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ADD_FREE_INSTALL_OBSERVER));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ABILITY_RECOVERY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ABILITY_RECOVERY_ENABLE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::QUERY_MISSION_VAILD));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::VERIFY_PERMISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ACQUIRE_SHARE_DATA));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::SHARE_DATA_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_ABILITY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::FORCE_EXIT_APP));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::RECORD_APP_EXIT_REASON));
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

