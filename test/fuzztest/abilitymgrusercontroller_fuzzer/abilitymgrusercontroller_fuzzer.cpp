/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "abilitymgrusercontroller_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "user_controller.h"
#undef private

#include "securec.h"
#include <iostream>
#include "user_callback.h"

using namespace OHOS::AAFwk;

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
constexpr uint8_t ENABLE = 2;
}
uint32_t GetU32Data(const char *ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
            ptr[INPUT_THREE];
}
bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    int32_t id = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<UserItem> userItem = std::make_shared<UserItem>(id);
    UserState state = STATE_BOOTING;
    userItem->GetUserId();
    userItem->SetState(state);
    userItem->GetState();
    std::shared_ptr<UserController> userController = std::make_shared<UserController>();
    userController->Init();
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    userController->ClearAbilityUserItems(userId);
    userController->StopUser(userId);
    userController->LogoutUser(userId);
    userController->GetCurrentUserId();
    userController->GetUserItem(userId);
    userController->IsCurrentUser(userId);
    userController->IsExistOsAccount(userId);
    userController->GetOrCreateUserItem(userId);
    userController->SetCurrentUserId(userId);
    int32_t newUserId = static_cast<int32_t>(GetU32Data(data));
    int32_t oldUserId = static_cast<int32_t>(GetU32Data(data));
    int32_t id1 = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<UserItem> item = std::make_shared<UserItem>(id1);
    userController->UserBootDone(item);
    userController->BroadcastUserStarted(userId);
    userController->BroadcastUserBackground(userId);
    userController->BroadcastUserForeground(userId);
    userController->BroadcastUserStopping(userId);
    userController->SendSystemUserStart(userId);
    int32_t eventId = static_cast<int32_t>(GetU32Data(data));
    EventWrap event(eventId);
    userController->ProcessEvent(event);
    userController->SendSystemUserCurrent(oldUserId, newUserId);
    int32_t param1 = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<UserItem> userItemTwo = std::make_shared<UserItem>(param1);
    userController->SendReportUserSwitch(oldUserId, newUserId, userItem);
    userController->SendUserSwitchTimeout(oldUserId, newUserId, userItem);
    userController->SendContinueUserSwitch(oldUserId, newUserId, userItem);
    userController->SendUserSwitchDone(userId);
    userController->HandleSystemUserStart(userId);
    userController->HandleSystemUserCurrent(oldUserId, newUserId);
    userController->HandleReportUserSwitch(oldUserId, newUserId, userItem);
    userController->HandleUserSwitchTimeout(oldUserId, newUserId, userItem);
    userController->HandleContinueUserSwitch(oldUserId, newUserId, userItem);
    userController->HandleUserSwitchDone(userId);
    userController->GetFreezingNewUserId();
    userController->SetFreezingNewUserId(userId);
    return true;
}
} // namespace OHOS

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