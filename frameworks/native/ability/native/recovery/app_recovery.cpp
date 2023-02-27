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

#include "app_recovery.h"

#include <csignal>
#include <mutex>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <syscall.h>
#include <unistd.h>

#include "ability_runtime/js_ability.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"

#include "hilog_wrapper.h"
#include "parcel.h"
#include "want_params.h"
#include "recovery_param.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
std::mutex g_mutex;
std::atomic<bool> g_blocked = false;

AppRecovery::AppRecovery() : isEnable_(false), restartFlag_(RestartFlag::ALWAYS_RESTART),
    saveOccasion_(SaveOccasionFlag::SAVE_WHEN_ERROR), saveMode_(SaveModeFlag::SAVE_WITH_FILE)
{
}

AppRecovery::~AppRecovery()
{
}

static void SigQuitHandler(int signal)
{
    g_blocked = true;
    HILOG_ERROR("AppRecovery SigQuitHandler0");
    g_mutex.lock();
    HILOG_ERROR("AppRecovery SigQuitHandler1");
    g_blocked = false;
}

static void BlockMainThread()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_handler = SigQuitHandler;
    action.sa_flags = 0;
    if (sigaction(3, &action, nullptr) != 0) {
        HILOG_ERROR("AppRecovery Failed to register signal(%d)", 3);
    }

    g_mutex.lock();
    if (syscall(SYS_tgkill, getpid(), getpid(), 3) != 0) {
        HILOG_ERROR("Failed to send SIGDUMP to main thread, errno(%d).", errno);
    }

    int left = 2;
    while(!g_blocked && left > 0) {
        left = sleep(left);
    }
    HILOG_INFO("Main thread status:%{public}d", g_blocked.load());
}

AppRecovery& AppRecovery::GetInstance()
{
    static AppRecovery instance;
    return instance;
}

bool AppRecovery::InitApplicationInfo(const std::shared_ptr<EventHandler>& mainHandler,
    const std::shared_ptr<ApplicationInfo>& applicationInfo)
{
    mainHandler_ = mainHandler;
    applicationInfo_ = applicationInfo;
    return true;
}

bool AppRecovery::AddAbility(std::shared_ptr<Ability> ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery not enabled.");
        return false;
    }

    if (!abilityRecoverys_.empty() && !abilityInfo->recoverable) {
        HILOG_ERROR("AppRecovery abilityRecoverys is not empty but ability recoverable is false.");
        return false;
    }
    ability_ = ability;
    std::shared_ptr<AbilityRecovery> abilityRecovery = std::make_shared<AbilityRecovery>();
    abilityRecovery->InitAbilityInfo(ability, abilityInfo, token);
    abilityRecovery->EnableAbilityRecovery(restartFlag_, saveOccasion_, saveMode_);
    ability->EnableAbilityRecovery(abilityRecovery);
    abilityRecoverys_.push_back(abilityRecovery);
    return true;
}

bool AppRecovery::removeAbility(const sptr<IRemoteObject>& tokenId)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery not enabled. not removeAbility");
        return false;
    }

    if (!tokenId) {
        HILOG_ERROR("AppRecovery removeAbility tokenId is null.");
        return false;
    }
    HILOG_INFO("AppRecovery removeAbility start");
    auto itr = std::find_if(abilityRecoverys_.begin(), abilityRecoverys_.end(),
        [&tokenId](std::shared_ptr<AbilityRecovery> &abilityRecovery) {
        return (abilityRecovery && abilityRecovery->GetToken() == tokenId);
    });
    if (itr != abilityRecoverys_.end()) {
        abilityRecoverys_.erase(itr);
        HILOG_DEBUG("AppRecovery removeAbility done");
    }
    return true;
}

bool AppRecovery::ScheduleSaveAppState(StateReason reason, uintptr_t ability)
{
    HILOG_INFO("AppRecovery ScheduleSaveAppState begin");
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery ScheduleSaveAppState. is not enabled");
        return false;
    }

    if (!ShouldSaveAppState(reason)) {
        HILOG_ERROR("AppRecovery ts not save ability state");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        auto abilityPtr = ability_.lock();
        if (!abilityPtr || !abilityPtr->GetAbilityContext()) {
            HILOG_ERROR("AppRecovery ScheduleSaveAppState ability or context is nullptr");
            return false;
        }
        HILOG_INFO("AppRecovery BlockMainThread start");
        BlockMainThread();
        HILOG_INFO("AppRecovery BlockMainThread end");
        OHOS::AbilityRuntime::JsAbility& jsAbility = static_cast<AbilityRuntime::JsAbility&>(*abilityPtr);
        jsAbility.getJsRuntime()->EnableCrossThreadExecution();
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        HILOG_ERROR("ScheduleSaveAppState. main handler is not exist");
        return false;
    }

    auto task = [reason, ability]() {
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
    };
    if (!handler->PostTask(task)) {
        HILOG_ERROR("Failed to schedule save app state.");
        return false;
    }

    return true;
}

void AppRecovery::setRestartWant(std::shared_ptr<AAFwk::Want> want)
{
    HILOG_INFO("AppRecovery setRestartWant begin");
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery setRestartWant not enabled");
        return;
    }
    want_ = want;
}

bool AppRecovery::ScheduleRecoverApp(StateReason reason)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp. is not enabled");
        return false;
    }

    if (!ShouldRecoverApp(reason)) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp. is not recover app");
        return false;
    }

    if (abilityRecoverys_.empty()) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp ability is nullptr");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        DoRecoverApp(reason);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp main handler is not exist");
        return false;
    }

    // may we save state in other thread or just restart.
    // 1. check whether main handler is still avaliable
    // 2. do state saving in main thread or just restart app with no state?
    // 3. create an recovery thread for saving state, just block jsvm mult-thread checking mechaism

    auto task = [reason]() {
        AppRecovery::GetInstance().DoRecoverApp(reason);
    };
    if (!handler->PostTask(task)) {
        HILOG_ERROR("Failed to schedule save app state.");
    }

    return true;
}

bool AppRecovery::TryRecoverApp(StateReason reason)
{
    if (!isEnable_) {
        return false;
    }

    ScheduleSaveAppState(reason);
    PersistAppState();
    return ScheduleRecoverApp(reason);
}

void AppRecovery::DoRecoverApp(StateReason reason)
{
    HILOG_INFO("AppRecovery DoRecoverApp begin");
    if (abilityRecoverys_.empty()) {
        HILOG_ERROR("AppRecovery no ability exist! ");
        return;
    }
    AAFwk::Want *want = nullptr;
    if (want_ != nullptr) {
        want = want_.get();
    }
    abilityRecoverys_.front()->ScheduleRecoverAbility(reason, want);
}

void AppRecovery::DoSaveAppState(StateReason reason, uintptr_t ability)
{
    HILOG_DEBUG("AppRecovery DoSaveAppState begin");
    auto appInfo = applicationInfo_.lock();
    if (appInfo == nullptr || abilityRecoverys_.empty()) {
        HILOG_ERROR("AppRecovery Application or ability info is not exist.");
        return;
    }

    bool onlySaveTargetAbility = (ability != 0);
    for (auto& abilityRecoveryRecord : abilityRecoverys_) {
        if (!onlySaveTargetAbility) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            HILOG_DEBUG("AppRecovery not onlySaveTargetAbility ScheduleSaveAbilityState");
            continue;
        }
        if (abilityRecoveryRecord->IsSameAbility(ability)) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            HILOG_DEBUG("AppRecovery IsSameAbility ScheduleSaveAbilityState");
            break;
        }
    }
}

void AppRecovery::EnableAppRecovery(uint16_t restartFlag, uint16_t saveFlag, uint16_t saveMode)
{
    isEnable_ = true;
    restartFlag_ = restartFlag;
    saveOccasion_ = saveFlag;
    saveMode_ = saveMode;
}

bool AppRecovery::ShouldSaveAppState(StateReason reason)
{
    bool ret = false;
    switch (reason) {
        case StateReason::DEVELOPER_REQUEST:
            ret = true;
            break;

        case StateReason::LIFECYCLE:
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_BACKGROUND) != 0) {
                ret = true;
            }
            break;

        case StateReason::CPP_CRASH:
        case StateReason::JS_ERROR:
        case StateReason::APP_FREEZE: // appfreeze could not callback to js function safely.
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_ERROR) != 0) {
                ret = true;
            }
            break;
    }
    return ret;
}

bool AppRecovery::ShouldRecoverApp(StateReason reason)
{
    if (restartFlag_ == RestartFlag::NO_RESTART) {
        return false;
    }

    bool ret = false;
    bool isAlwaysStart = false;
    if (restartFlag_ == RestartFlag::ALWAYS_RESTART) {
        isAlwaysStart = true;
    }
    switch (reason) {
        case StateReason::DEVELOPER_REQUEST:
            ret = true;
            break;

        case StateReason::LIFECYCLE:
            ret = false;
            break;

        case StateReason::CPP_CRASH:
            ret = false;
            break;

        case StateReason::JS_ERROR:
            if (isAlwaysStart || (restartFlag_ & RestartFlag::RESTART_WHEN_JS_CRASH) != 0) {
                ret = true;
            }
            break;

        case StateReason::APP_FREEZE:
            if (isAlwaysStart || (restartFlag_ & RestartFlag::RESTART_WHEN_APP_FREEZE) != 0) {
                ret = true;
            }
            break;
    }
    return ret;
}

bool AppRecovery::PersistAppState()
{
    if (saveMode_ == SaveModeFlag::SAVE_WITH_FILE) {
        return true;
    }

    bool ret = true;
    for (auto& abilityRecovery : abilityRecoverys_) {
        ret = ret && abilityRecovery->PersistState();
    }
    return ret;
}

bool AppRecovery::IsEnabled() const
{
    return isEnable_;
}

uint16_t AppRecovery::GetRestartFlag() const
{
    return restartFlag_;
}

uint16_t AppRecovery::GetSaveOccasionFlag() const
{
    return saveOccasion_;
}

uint16_t AppRecovery::GetSaveModeFlag() const
{
    return saveMode_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS