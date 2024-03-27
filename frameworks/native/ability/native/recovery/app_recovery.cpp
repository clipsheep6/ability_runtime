/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "context/application_context.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_ability.h"
#include "mission_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "ohos_application.h"
#include "parcel.h"
#include "recovery_param.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
std::mutex g_mutex;
std::atomic<bool> g_blocked = false;
const int DELAY_TIME = 1000;

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
    std::lock_guard<std::mutex> lock(g_mutex);
    g_blocked = false;
}

static bool BlockMainThreadLocked()
{
    struct sigaction action;
    (void)memset_s(&action, sizeof(action), 0, sizeof(action));
    sigfillset(&action.sa_mask);
    action.sa_handler = SigQuitHandler;
    action.sa_flags = 0;
    if (sigaction(SIGQUIT, &action, nullptr) != 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery Failed to register signal");
        return false;
    }

    if (syscall(SYS_tgkill, getpid(), getpid(), SIGQUIT) != 0) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Failed to send SIGQUIT to main thread, errno(%d).", errno);
        return false;
    }
    int left = 1000000; // 1s
    constexpr int pollTime = 100; // 100us
    while (left > 0) {
        int ret = usleep(pollTime);
        if (ret == 0) {
            left -= pollTime;
        } else {
            left -= ret;
        }

        if (g_blocked) {
            return true;
        }
    }
    return false;
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

bool AppRecovery::AddAbility(std::shared_ptr<AbilityRuntime::UIAbility> ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery not enabled.");
        return false;
    }

    if (!abilityRecoverys_.empty() && !abilityInfo->recoverable) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery abilityRecoverys is not empty but ability recoverable is false.");
        return false;
    }
    ability_ = ability;
    std::shared_ptr<AbilityRecovery> abilityRecovery = std::make_shared<AbilityRecovery>();
    abilityRecovery->InitAbilityInfo(ability, abilityInfo, token);
    abilityRecovery->EnableAbilityRecovery(restartFlag_, saveOccasion_, saveMode_);
    ability->EnableAbilityRecovery(abilityRecovery);
    abilityRecoverys_.push_back(abilityRecovery);
    auto handler = mainHandler_.lock();
    if (handler != nullptr) {
        auto task = []() {
            AppRecovery::GetInstance().DeleteInValidMissionFiles();
        };
        if (!handler->PostTask(task, "AppRecovery:AddAbility", DELAY_TIME)) {
            TAG_LOGE(AAFwkTag::RECOVERY, "Failed to DeleteInValidMissionFiles.");
        }
    }
    return true;
}

bool AppRecovery::RemoveAbility(const sptr<IRemoteObject>& tokenId)
{
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery not enabled. not removeAbility");
        return false;
    }

    if (!tokenId) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery removeAbility tokenId is null.");
        return false;
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery removeAbility start");
    auto itr = std::find_if(abilityRecoverys_.begin(), abilityRecoverys_.end(),
        [&tokenId](std::shared_ptr<AbilityRecovery> &abilityRecovery) {
        return (abilityRecovery && abilityRecovery->GetToken() == tokenId);
    });
    if (itr != abilityRecoverys_.end()) {
        abilityRecoverys_.erase(itr);
        TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery removeAbility done");
    }
    return true;
}

bool AppRecovery::ScheduleSaveAppState(StateReason reason, uintptr_t ability)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery ScheduleSaveAppState begin");
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleSaveAppState. is not enabled");
        return false;
    }

    if (!ShouldSaveAppState(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ts not save ability state");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        auto abilityPtr = ability_.lock();
        if (!abilityPtr || !abilityPtr->GetAbilityContext()) {
            TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleSaveAppState ability or context is nullptr");
            return false;
        }
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!BlockMainThreadLocked()) {
            TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery Failed to block main thread, skip save state when appfreeze");
            return false;
        }
        OHOS::AbilityRuntime::JsUIAbility& jsAbility = static_cast<AbilityRuntime::JsUIAbility&>(*abilityPtr);
        AbilityRuntime::JsRuntime& runtime = const_cast<AbilityRuntime::JsRuntime&>(jsAbility.GetJsRuntime());
        auto& nativeEngine = runtime.GetNativeEngine();
        nativeEngine.AllowCrossThreadExecution();
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "ScheduleSaveAppState. main handler is not exist");
        return false;
    }

    auto task = [reason, ability]() {
        AppRecovery::GetInstance().DoSaveAppState(reason, ability);
    };
    if (!handler->PostTask(task, "AppRecovery:SaveAppState")) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Failed to schedule save app state.");
        return false;
    }

    return true;
}

void AppRecovery::SetRestartWant(std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery SetRestartWant begin");
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery SetRestartWant not enabled");
        return;
    }
    want_ = want;
}

bool AppRecovery::ScheduleRecoverApp(StateReason reason)
{
    if (!isEnable_) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleRecoverApp. is not enabled");
        return false;
    }

    if (!ShouldRecoverApp(reason)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleRecoverApp. is not recover app");
        return false;
    }

    if (abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleRecoverApp ability is nullptr");
        return false;
    }

    if (reason == StateReason::APP_FREEZE) {
        DoRecoverApp(reason);
        return true;
    }

    auto handler = mainHandler_.lock();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery ScheduleRecoverApp main handler is not exist");
        return false;
    }

    // may we save state in other thread or just restart.
    // 1. check whether main handler is still avaliable
    // 2. do state saving in main thread or just restart app with no state?
    // 3. create an recovery thread for saving state, just block jsvm mult-thread checking mechaism

    auto task = [reason]() {
        AppRecovery::GetInstance().DoRecoverApp(reason);
    };
    if (!handler->PostTask(task, "AppRecovery:RecoverApp")) {
        TAG_LOGE(AAFwkTag::RECOVERY, "Failed to schedule save app state.");
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
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery DoRecoverApp begin");
    if (abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery no ability exist! ");
        return;
    }
    AAFwk::Want *want = nullptr;
    if (want_ != nullptr) {
        want = want_.get();
    }

    if (abilityRecoverys_.size() == 1) {
        if (abilityRecoverys_.front()->IsOnForeground()) {
            abilityRecoverys_.front()->ScheduleRecoverAbility(reason, want);
            return;
        }
    }

    for (auto itr = abilityRecoverys_.rbegin(); itr != abilityRecoverys_.rend(); itr++) {
        if ((*itr)->IsOnForeground()) {
            (*itr)->ScheduleRecoverAbility(reason, want);
            break;
        }
    }
    TAG_LOGW(AAFwkTag::RECOVERY, "AppRecovery no foreground ability, not DoRecoverApp!");
}

void AppRecovery::DoSaveAppState(StateReason reason, uintptr_t ability)
{
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery DoSaveAppState begin");
    auto appInfo = applicationInfo_.lock();
    if (appInfo == nullptr || abilityRecoverys_.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery Application or ability info is not exist.");
        return;
    }

    bool onlySaveTargetAbility = (ability != 0);
    for (auto& abilityRecoveryRecord : abilityRecoverys_) {
        if (!onlySaveTargetAbility) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery not onlySaveTargetAbility ScheduleSaveAbilityState");
            continue;
        }
        if (abilityRecoveryRecord->IsSameAbility(ability)) {
            abilityRecoveryRecord->ScheduleSaveAbilityState(reason);
            TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery IsSameAbility ScheduleSaveAbilityState");
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

void AppRecovery::DeleteInValidMissionFiles()
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return;
    }

    std::string fileDir = context->GetFilesDir();
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery DeleteInValidMissionFiles fileDir: %{public}s", fileDir.c_str());
    if (fileDir.empty() || !OHOS::FileExists(fileDir)) {
        TAG_LOGD(AAFwkTag::RECOVERY,
            "AppRecovery DeleteInValidMissionFiles fileDir is empty or fileDir is not exists.");
        return;
    }
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;

    if (!GetMissionIds(fileDir, missionIds)) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery get mssion file id error.");
        return;
    }
    if (missionIds.empty()) {
        TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery no mission file, no need delete it.");
        return;
    }
    std::shared_ptr<AAFwk::AbilityManagerClient> abilityMgr = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery DeleteInValidMissionFiles. abilityMgr client is not exist.");
        return;
    }
    abilityMgr->IsValidMissionIds(missionIds, results);
    if (results.empty()) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery DeleteInValidMissionFiles. results is empty.");
        return;
    }
    for (auto& item : results) {
        TAG_LOGI(AAFwkTag::RECOVERY, "AppRecovery missionResult: missionId: %{public}d, isValid: %{public}d",
            item.missionId, item.isValid);
        if (!item.isValid) {
            DeleteInValidMissionFileById(fileDir, item.missionId);
        }
    }
}

void AppRecovery::DeleteInValidMissionFileById(std::string fileDir, int32_t missionId)
{
    std::string fileName = std::to_string(missionId) + ".state";
    std::string file = fileDir + "/" + fileName;
    bool ret = OHOS::RemoveFile(file);
    if (!ret) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery delete the file: %{public}s failed", file.c_str());
    }
    TAG_LOGD(AAFwkTag::RECOVERY, "AppRecovery delete the file: %{public}s done", file.c_str());
}

bool AppRecovery::GetMissionIds(std::string path, std::vector<int32_t> &missionIds)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery GetMissionIds open dir error.");
        return false;
    }
    struct dirent *ptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (ptr == nullptr) {
            TAG_LOGE(AAFwkTag::RECOVERY, "AppRecovery GetMissionIds read dir error.");
            return false;
        }
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_REG) {
            std::string fileName = ptr->d_name;
            auto pos = fileName.find_first_of(".");
            if (pos != std::string::npos) {
                std::string missionIdStr = fileName.substr(0, pos);
                missionIds.push_back(atoi(missionIdStr.c_str()));
            }
        } else {
            continue;
        }
    }
    closedir(dir);
    return true;
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
