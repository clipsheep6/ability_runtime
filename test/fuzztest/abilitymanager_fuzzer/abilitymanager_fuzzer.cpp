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

#include "abilitymanager_fuzzer.h"

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

void EmplaceCodeMap1()
{
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::TERMINATE_ABILITY));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ATTACH_ABILITY_THREAD));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ABILITY_TRANSITION_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CONNECT_ABILITY_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::DISCONNECT_ABILITY_DONE));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::ADD_WINDOW_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::TERMINATE_ABILITY_RESULT));
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
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::TERMINATE_ABILITY_BY_CALLER));
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
}

void EmplaceCodeMap2()
{
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
}

void EmplaceCodeMap3()
{
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_PROCESS_RUNNING_INFO));
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::CLEAR_UP_APPLICATION_DATA));
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
    codeMap_.emplace(codeMap_.size(), static_cast<int>(IAbilityManager::GET_SESSIONMANAGERSERVICE));
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
}

void EmplaceCodeMap4()
{
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
        EmplaceCodeMap1();
        EmplaceCodeMap2();
        EmplaceCodeMap3();
        EmplaceCodeMap4();
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

