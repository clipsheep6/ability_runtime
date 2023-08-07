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

#define DISABLE_FUZZ
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
std::map<int, uint32_t> codeMap_;

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INDEX_ZERO] << INPUT_TWENTYFOUR) | (ptr[INDEX_ONE] << INPUT_SIXTEEN) |
        (ptr[INDEX_TWO] << INPUT_EIGHT) | ptr[INDEX_THREE];
}

void EmplaceCodeMap()
{
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SNAPSHOT_HANDLER));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::FINISH_USER_TEST));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CONNECTION_OBSERVER));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CONNECTION_OBSERVER));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY));
    codeMap_.emplace(codeMap_.size(),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE));
#ifndef DISABLE_FUZZ
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD));
#endif
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::SHARE_DATA_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_TOKEN));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_EXIT_APP));
    codeMap_.emplace(codeMap_.size(), static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON));
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

