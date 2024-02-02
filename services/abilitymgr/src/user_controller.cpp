/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "user_controller.h"

#include "ability_manager_service.h"
#include "app_scheduler.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_session_manager_service.h"
#include "os_account_manager_wrapper.h"
#include "scene_board_judgement.h"
#include "user_callback.h"
#include "task_data_persistence_mgr.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AppExecFwk;
namespace {
const int64_t USER_SWITCH_TIMEOUT = 3 * 1000; // 3s
}

UserItem::UserItem(int32_t id) : userId_(id)
{}

UserItem::~UserItem() {}

int32_t UserItem::GetUserId()
{
    return userId_;
}

void UserItem::SetState(const UserState &state)
{
    if (curState_ == state) {
        return;
    }
    lastState_ = curState_;
    curState_ = state;
}

UserState UserItem::GetState()
{
    return curState_;
}

UserController::UserController()
{
}

UserController::~UserController()
{
}

void UserController::Init()
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (!handler) {
        return;
    }

    if (eventHandler_) {
        return;
    }
    eventHandler_ = std::make_shared<UserEventHandler>(handler, shared_from_this());
}

void UserController::ClearAbilityUserItems(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    if (userItems_.count(userId)) {
        userItems_.erase(userId);
    }
}

void UserController::StartUser(int32_t userId, sptr<IUserCallback> callback, bool isForeground)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD) {
        HILOG_ERROR("StartUser userId is invalid:%{public}d", userId);
        callback->OnStartUserDone(userId, INVALID_USERID_VALUE);
        return;
    }

    if (IsCurrentUser(userId)) {
        HILOG_WARN("StartUser user is already current:%{public}d", userId);
        callback->OnStartUserDone(userId, ERR_OK);
        return;
    }

    if (!IsExistOsAccount(userId)) {
        HILOG_ERROR("StartUser not exist such account:%{public}d", userId);
        callback->OnStartUserDone(userId, INVALID_USERID_VALUE);
        return;
    }

    if (isForeground && GetCurrentUserId() != USER_ID_NO_HEAD && !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        // start freezing screen
        SetFreezingNewUserId(userId);
        DelayedSingleton<AbilityManagerService>::GetInstance()->StartFreezingScreen();
    }

    auto oldUserId = GetCurrentUserId();
    auto userItem = GetOrCreateUserItem(userId);
    auto state = userItem->GetState();
    if (state == STATE_STOPPING || state == STATE_SHUTDOWN) {
        HILOG_ERROR("StartUser user is stop now, userId:%{public}d", userId);
        callback->OnStartUserDone(userId, ERR_DEAD_OBJECT);
        return;
    }

    if (isForeground) {
        SetCurrentUserId(userId);
        // notify wms switching now
    }

    bool needStart = false;
    if (state == STATE_BOOTING) {
        needStart = true;
        // send user start msg.
        SendSystemUserStart(userId);
    }

    if (isForeground) {
        SendSystemUserCurrent(oldUserId, userId);
        SendReportUserSwitch(oldUserId, userId, userItem);
        SendUserSwitchTimeout(oldUserId, userId, userItem);
    }

    if (needStart) {
        BroadcastUserStarted(userId);
    }

    UserBootDone(userItem);
    if (isForeground) {
        MoveUserToForeground(oldUserId, userId, callback);
    }
}

int32_t UserController::StopUser(int32_t userId)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD || userId == USER_ID_DEFAULT) {
        HILOG_ERROR("userId is invalid:%{public}d", userId);
        return -1;
    }

    if (IsCurrentUser(userId)) {
        HILOG_WARN("user is already current:%{public}d", userId);
        return 0;
    }

    if (!IsExistOsAccount(userId)) {
        HILOG_ERROR("not exist such account:%{public}d", userId);
        return -1;
    }

    BroadcastUserStopping(userId);

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("appScheduler is null");
        return -1;
    }
    appScheduler->KillProcessesByUserId(userId);

    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto taskDataPersistenceMgr = DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
        if (!taskDataPersistenceMgr) {
            HILOG_ERROR("taskDataPersistenceMgr is null");
            return -1;
        }
        taskDataPersistenceMgr->RemoveUserDir(userId);
    }

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        HILOG_ERROR("abilityManagerService is null");
        return -1;
    }
    abilityManagerService->ClearUserData(userId);

    BroadcastUserStopped(userId);
    return 0;
}

int32_t UserController::LogoutUser(int32_t userId)
{
    if (userId < 0 || userId == USER_ID_NO_HEAD) {
        HILOG_ERROR("userId is invalid:%{public}d", userId);
        return INVALID_USERID_VALUE;
    }
    if (!IsExistOsAccount(userId)) {
        HILOG_ERROR("not exist such account:%{public}d", userId);
        return INVALID_USERID_VALUE;
    }
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityManagerService) {
        HILOG_ERROR("abilityManagerService is null");
        return -1;
    }
    abilityManagerService->RemoveLauncherDeathRecipient(userId);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_INFO("SceneBoard exit normally.");
        Rosen::MockSessionManagerService::GetInstance().NotifyNotKillService();
    }
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (!appScheduler) {
        HILOG_ERROR("appScheduler is null");
        return INVALID_USERID_VALUE;
    }
    appScheduler->KillProcessesByUserId(userId);
    abilityManagerService->ClearUserData(userId);
    if (IsCurrentUser(userId)) {
        SetCurrentUserId(0);
    }
    ClearAbilityUserItems(userId);
    return 0;
}

int32_t UserController::GetCurrentUserId()
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    return currentUserId_;
}

std::shared_ptr<UserItem> UserController::GetUserItem(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    return nullptr;
}

bool UserController::IsCurrentUser(int32_t userId)
{
    int32_t oldUserId = GetCurrentUserId();
    if (oldUserId == userId) {
        auto userItem = GetUserItem(userId);
        if (userItem) {
            HILOG_WARN("IsCurrentUser userId is already current:%{public}d", userId);
            return true;
        }
    }
    return false;
}

bool UserController::IsExistOsAccount(int32_t userId)
{
    bool isExist = false;
    auto errCode = DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->IsOsAccountExists(userId, isExist);
    return (errCode == 0) && isExist;
}

std::shared_ptr<UserItem> UserController::GetOrCreateUserItem(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return it->second;
    }

    auto userItem = std::make_shared<UserItem>(userId);
    userItems_.emplace(userId, userItem);
    return userItem;
}

void UserController::SetCurrentUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> guard(userLock_);
    currentUserId_ = userId;
    HILOG_DEBUG("set current userId: %{public}d", userId);
    DelayedSingleton<AppScheduler>::GetInstance()->SetCurrentUserId(userId);
}

void UserController::MoveUserToForeground(int32_t oldUserId, int32_t newUserId, sptr<IUserCallback> callback)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return;
    }
    manager->SwitchToUser(oldUserId, newUserId, callback);
    BroadcastUserBackground(oldUserId);
    BroadcastUserForeground(newUserId);
}

void UserController::UserBootDone(std::shared_ptr<UserItem> &item)
{
    if (!item) {
        return;
    }
    int32_t userId = item->GetUserId();

    std::lock_guard<ffrt::mutex> guard(userLock_);
    auto it = userItems_.find(userId);
    if (it != userItems_.end()) {
        return;
    }

    if (item != it->second) {
        return;
    }
    item->SetState(UserState::STATE_STARTED);
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!manager) {
        return;
    }
    manager->UserStarted(userId);
}

void UserController::BroadcastUserStarted(int32_t userId)
{
    // broadcast event user start.
}

void UserController::BroadcastUserBackground(int32_t userId)
{
    // broadcast event user switch to bg.
}

void UserController::BroadcastUserForeground(int32_t userId)
{
    // broadcast event user switch to fg.
}

void UserController::BroadcastUserStopping(int32_t userId)
{
}

void UserController::BroadcastUserStopped(int32_t userId)
{
}

void UserController::SendSystemUserStart(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_SYSTEM_USER_START, eventData));
}

void UserController::ProcessEvent(const EventWrap &event)
{
    auto eventId = event.GetEventId();
    auto eventData = static_cast<UserEvent*>(event.GetEventData().get());
    if (!eventData) {
        HILOG_DEBUG("no event data, event id: %{public}u.", eventId);
        return;
    }

    HILOG_DEBUG("Event id obtained: %{public}u.", eventId);
    switch (eventId) {
        case UserEventHandler::EVENT_SYSTEM_USER_START: {
            HandleSystemUserStart(eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_SYSTEM_USER_CURRENT: {
            HandleSystemUserCurrent(eventData->oldUserId, eventData->newUserId);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH: {
            HandleReportUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_CONTINUE_USER_SWITCH: {
            HandleContinueUserSwitch(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_USER_SWITCH_TIMEOUT: {
            HandleUserSwitchTimeout(eventData->oldUserId, eventData->newUserId, eventData->userItem);
            break;
        }
        case UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE: {
            HandleUserSwitchDone(eventData->newUserId);
            break;
        }
        default: {
            HILOG_WARN("Unsupported  event.");
            break;
        }
    }
}

void UserController::SendSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_SYSTEM_USER_CURRENT, eventData));
}

void UserController::SendReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_REPORT_USER_SWITCH, eventData));
}

void UserController::SendUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_USER_SWITCH_TIMEOUT,
        eventData), USER_SWITCH_TIMEOUT);
}

void UserController::SendContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->oldUserId = oldUserId;
    eventData->newUserId = newUserId;
    eventData->userItem = usrItem;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_CONTINUE_USER_SWITCH, eventData));
}

void UserController::SendUserSwitchDone(int32_t userId)
{
    auto handler = eventHandler_;
    if (!handler) {
        return;
    }

    auto eventData = std::make_shared<UserEvent>();
    eventData->newUserId = userId;
    handler->SendEvent(EventWrap(UserEventHandler::EVENT_REPORT_USER_SWITCH_DONE,
        eventData));
}

void UserController::HandleSystemUserStart(int32_t userId)
{
    // notify system mgr user start.
}

void UserController::HandleSystemUserCurrent(int32_t oldUserId, int32_t newUserId)
{
    // notify system mgr user switch to new.
}

void UserController::HandleReportUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    // notify user switch observers, not support yet.
}

void UserController::HandleUserSwitchTimeout(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    // other observers
    SendContinueUserSwitch(oldUserId, newUserId, usrItem);
}

void UserController::HandleContinueUserSwitch(int32_t oldUserId, int32_t newUserId,
    std::shared_ptr<UserItem> &usrItem)
{
    auto manager = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (manager && !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        manager->StopFreezingScreen();
    }
    SendUserSwitchDone(newUserId);
}

void UserController::HandleUserSwitchDone(int32_t userId)
{
    // notify wms switching done.
    // notify user switch observers.
}

int32_t UserController::GetFreezingNewUserId() const
{
    return freezingNewUserId_;
}

void UserController::SetFreezingNewUserId(int32_t userId)
{
    freezingNewUserId_ = userId;
}
}
}
