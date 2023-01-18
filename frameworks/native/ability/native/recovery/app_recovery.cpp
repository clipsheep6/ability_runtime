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

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

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

namespace OHOS {
namespace AppExecFwk {
AppRecovery::AppRecovery() : isEnable_(false), restartFlag_(RestartFlag::ALWAYS_RESTART),
    saveOccasion_(SaveOccasionFlag::SAVE_WHEN_ERROR), saveMode_(SaveModeFlag::SAVE_WITH_FILE)
{
}

AppRecovery::~AppRecovery()
{
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

bool AppRecovery::AddAbility(const std::shared_ptr<Ability>& ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    if (!isEnable_) {
        return false;
    }

    if (!abilityRecoverys_.empty()) {
        HILOG_ERROR("AppRecovery Only support single ability application at now.");
        return false;
    }

    std::shared_ptr<AbilityRecovery> abilityRecovery = std::make_shared<AbilityRecovery>();
    abilityRecovery->InitAbilityInfo(ability, abilityInfo, token);
    abilityRecovery->EnableAbilityRecovery(restartFlag_, saveOccasion_, saveMode_);
    ability->EnableAbilityRecovery(abilityRecovery);
    abilityRecoverys_.push_back(abilityRecovery);
    return true;
}

bool AppRecovery::ScheduleSaveAppState(StateReason reason)
{
    HILOG_INFO("%{public}s, reason:%{public}d", __func__, reason);
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery ScheduleSaveAppState. is not enabled");
        return false;
    }

    if (!ShouldSaveAppState(reason)) {
        HILOG_ERROR("AppRecovery ts not save ability state");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        HILOG_ERROR("ScheduleSaveAppState not support APP_FREEZE");
        return false;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        HILOG_ERROR("ScheduleSaveAppState. main handler is not exist");
        return false;
    }

    auto task = [reason]() {
        AppRecovery::GetInstance().DoSaveAppState(reason);
    };
    if (!handler->PostTask(task)) {
        HILOG_ERROR("Failed to schedule save app state.");
        return false;
    }

    return true;
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
        HILOG_ERROR("Failed to schedule recover app");
    }
    HILOG_INFO("AppRecovery ScheduleRecoverApp done");
    return true;
}

bool AppRecovery::TryRecoverApp(StateReason reason)
{
    HILOG_INFO("%{public}s, reason:%{public}d, isEnable_:%{public}d", __func__, reason, isEnable_);
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
    for (auto& i : abilityRecoverys_) {
        if (i->ScheduleRecoverAbility(reason)) {
            break;
        }
    }
}

void AppRecovery::DoSaveAppState(StateReason reason)
{
    for (auto& i : abilityRecoverys_) {
        i->ScheduleSaveAbilityState(reason);
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

    bool ret = true;
    switch (reason) {
        case StateReason::DEVELOPER_REQUEST:
            ret = true;
            break;

        case StateReason::LIFECYCLE:
            ret = false;
            break;

        case StateReason::CPP_CRASH:
            if ((restartFlag_ & RestartFlag::CPP_CRASH_NO_RESTART) != 0) {
                ret = false;
            }
            break;

        case StateReason::JS_ERROR:
            if ((restartFlag_ & RestartFlag::JS_CRASH_NO_RESTART) != 0) {
                ret = false;
            }
            break;

        case StateReason::APP_FREEZE:
            if ((restartFlag_ & RestartFlag::APP_FREEZE_NO_RESTART) != 0) {
                ret = false;
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