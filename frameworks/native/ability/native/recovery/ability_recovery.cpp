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

#include "ability_recovery.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "file_ex.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "parcel.h"
#include "recovery_param.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want_params.h"


namespace OHOS {
namespace AppExecFwk {
namespace {
static std::string GetSaveAppCachePath(int32_t savedStateId)
{
    auto context = AbilityRuntime::Context::GetApplicationContext();
    if (context == nullptr) {
        return "";
    }

    std::string fileDir = context->GetFilesDir();
    HILOG_DEBUG("AppRecovery GetSaveAppCachePath fileDir %{public}s.", fileDir.c_str());
    if (fileDir.empty() || !OHOS::FileExists(fileDir)) {
        HILOG_ERROR("AppRecovery GetSaveAppCachePath fileDir is empty or fileDir is not exists.");
        return "";
    }

    std::string fileName = std::to_string(savedStateId) + ".state";
    return fileDir + "/" + fileName;
}
}

AbilityRecovery::AbilityRecovery() : isEnable_(false), restartFlag_(RestartFlag::ALWAYS_RESTART),
    saveOccasion_(SaveOccasionFlag::SAVE_WHEN_ERROR), saveMode_(SaveModeFlag::SAVE_WITH_FILE)
{
}

AbilityRecovery::~AbilityRecovery()
{
}

bool AbilityRecovery::InitAbilityInfo(const std::shared_ptr<Ability>& ability,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const sptr<IRemoteObject>& token)
{
    isEnable_ = true;
    ability_ = ability;
    abilityInfo_ = abilityInfo;
    token_ = token;
    std::shared_ptr<AAFwk::AbilityManagerClient> ams = AAFwk::AbilityManagerClient::GetInstance();
    if (ams == nullptr) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp. ams client is not exist.");
        return false;
    }

    ams->EnableRecoverAbility(token);
    return true;
}

void AbilityRecovery::EnableAbilityRecovery(uint16_t restartFlag, uint16_t saveFlag, uint16_t saveMode)
{
    isEnable_ = true;
    restartFlag_ = restartFlag;
    saveOccasion_ = saveFlag;
    saveMode_ = saveMode;
}

bool AbilityRecovery::SaveAbilityState()
{
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        HILOG_ERROR("AppRecovery ability is nullptr");
        return false;
    }

    AAFwk::WantParams wantParams;
    int32_t status = ability->OnSaveState(AppExecFwk::StateType::APP_RECOVERY, wantParams);
    if (!(status == AppExecFwk::OnSaveResult::ALL_AGREE || status == AppExecFwk::OnSaveResult::RECOVERY_AGREE)) {
        HILOG_ERROR("AppRecovery Failed to save user params.");
        return false;
    }

#ifdef SUPPORT_GRAPHICS
    std::string pageStack = ability->GetContentInfo();
    if (!pageStack.empty()) {
        wantParams.SetParam("pageStack", AAFwk::String::Box(pageStack));
    } else {
        HILOG_ERROR("AppRecovery Failed to get page stack.");
    }
#endif
    if (saveMode_ == SaveModeFlag::SAVE_WITH_FILE) {
        SerializeDataToFile(abilityInfo->descriptionId, wantParams);
    } else if (saveMode_ == SaveModeFlag::SAVE_WITH_SHARED_MEMORY) {
        params_ = wantParams;
    }
    return true;
}

bool AbilityRecovery::SerializeDataToFile(int32_t savedStateId, WantParams& params)
{
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        HILOG_ERROR("AppRecovery %{public}s failed to persisted file path.", __func__);
        return false;
    }
    Parcel parcel;
    if (!params.Marshalling(parcel)) {
        HILOG_ERROR("AppRecovery %{public}s failed to Marshalling want param. ret", __func__);
        return false;
    }
    int fd = open(file.c_str(), O_RDWR | O_CREAT, (mode_t)0600);
    if (fd <= 0) {
        HILOG_ERROR("AppRecovery %{public}s failed to open %{public}s.", __func__, file.c_str());
        return false;
    }
    size_t sz = parcel.GetDataSize();
    uintptr_t buf = parcel.GetData();
    if (sz == 0 || buf == 0) {
        HILOG_ERROR("AppRecovery %{public}s failed to get parcel data.", __func__);
        close(fd);
        return false;
    }
    ssize_t nwrite = write(fd, reinterpret_cast<uint8_t*>(buf), sz);
    if (nwrite <= 0) {
        HILOG_ERROR("AppRecovery%{public}s failed to persist parcel data %{public}d.", __func__, errno);
    }
    close(fd);
    return true;
}

bool AbilityRecovery::ReadSerializeDataFromFile(int32_t savedStateId, WantParams& params)
{
    std::string file = GetSaveAppCachePath(savedStateId);
    if (file.empty()) {
        HILOG_ERROR("AppRecovery %{public}s failed to persisted file path.", __func__);
        return false;
    }

    char path[PATH_MAX] = {0};
    if (realpath(file.c_str(), path) == nullptr) {
        HILOG_ERROR("AppRecovery realpath error, errno is %{public}d.", errno);
        return false;
    }

    int32_t fd = open(path, O_RDONLY);
    if (fd <= 0) {
        HILOG_ERROR("AppRecovery fopen error");
        remove(path);
        return false;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) < 0) {
        close(fd);
        remove(path);
        return false;
    }

    auto mapFile = static_cast<uint8_t*>(mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mapFile == MAP_FAILED) {
        close(fd);
        remove(path);
        return false;
    }

    if (!parcel_.ParseFrom(reinterpret_cast<uintptr_t>(mapFile), statbuf.st_size)) {
        munmap(mapFile, statbuf.st_size);
        close(fd);
        remove(path);
        return false;
    }

    auto parsedParam = WantParams::Unmarshalling(parcel_);
    if (parsedParam != nullptr) {
        params = *parsedParam;
        delete parsedParam;
    } else {
        munmap(mapFile, statbuf.st_size);
        close(fd);
        remove(path);
        return false;
    }

    munmap(mapFile, statbuf.st_size);
    close(fd);
    remove(path);
    return true;
}

bool AbilityRecovery::ScheduleSaveAbilityState(StateReason reason)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery not enable");
        return false;
    }

    if (!IsSaveAbilityState(reason)) {
        HILOG_ERROR("AppRecovery ts not save ability state");
        return false;
    }
    return SaveAbilityState();
}

bool AbilityRecovery::ScheduleRecoverAbility(StateReason reason)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery not enable");
        return false;
    }

    std::shared_ptr<AAFwk::AbilityManagerClient> ams = AAFwk::AbilityManagerClient::GetInstance();
    if (ams == nullptr) {
        HILOG_ERROR("AppRecovery ScheduleRecoverApp. ams client is not exist.");
        return false;
    }

    auto token = token_.promote();
    if (token == nullptr) {
        return false;
    }
    HILOG_INFO("AbilityRecovery call ams to ScheduleRecoverAbility");
    ams->ScheduleRecoverAbility(token, reason);
    return true;
}

bool AbilityRecovery::PersistState()
{
    auto abilityInfo = abilityInfo_.lock();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AppRecovery ability is nullptr");
        return false;
    }

    if (!params_.IsEmpty()) {
        SerializeDataToFile(abilityInfo->descriptionId, params_);
    }
    HILOG_INFO("AbilityRecovery PersistState done");
    return true;
}

bool AbilityRecovery::LoadSavedState(StateReason reason)
{
    auto abilityInfo = abilityInfo_.lock();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AppRecovery abilityInfo is nullptr");
        return false;
    }

    if (hasTryLoad_) {
        return hasLoaded_;
    }

    hasTryLoad_ = true;

    if (!ReadSerializeDataFromFile(abilityInfo->descriptionId, params_)) {
        HILOG_ERROR("AppRecovery ScheduleRestoreAbilityState. failed to find record for id:%{public}d",
            abilityInfo->descriptionId);
        hasLoaded_ = false;
        return hasLoaded_;
    }

    auto stringObj = AAFwk::IString::Query(params_.GetParam("pageStack"));
    if (stringObj != nullptr) {
        pageStack_ = AAFwk::String::Unbox(stringObj);
    }
    hasLoaded_ = true;
    return hasLoaded_;
}

bool AbilityRecovery::ScheduleRestoreAbilityState(StateReason reason, const Want &want)
{
    if (!isEnable_) {
        HILOG_ERROR("AppRecovery not enable");
        return false;
    }

    if (!IsSaveAbilityState(reason)) {
        HILOG_ERROR("AppRecovery ts not save ability state");
        return false;
    }

    if (!LoadSavedState(reason)) {
        HILOG_ERROR("AppRecovery ScheduleRestoreAbilityState no saved state ");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    WantParams &wantCurrent = const_cast<WantParams&>(wantParams);
    for (auto& i : params_.GetParams()) {
        wantCurrent.SetParam(i.first, i.second.GetRefPtr());
    }
    HILOG_INFO("AbilityRecovery ScheduleRestoreAbilityState done");
    return true;
}

std::string AbilityRecovery::GetSavedPageStack(StateReason reason)
{
    if (!LoadSavedState(reason)) {
        HILOG_ERROR("AppRecovery GetSavedPageStack no saved state ");
        return "";
    }

    if (pageStack_.empty()) {
        HILOG_ERROR("AppRecovery GetSavedPageStack empty.");
    }
    return pageStack_;
}

bool AbilityRecovery::IsSaveAbilityState(StateReason reason)
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
        case StateReason::APP_FREEZE:
            if ((saveOccasion_ & SaveOccasionFlag::SAVE_WHEN_ERROR) != 0) {
                ret = true;
            }
            break;

        default:
            ret = false;
            break;
    }
    return ret;
}

uint16_t AbilityRecovery::GetRestartFlag() const
{
    return restartFlag_;
}

uint16_t AbilityRecovery::GetSaveOccasionFlag() const
{
    return saveOccasion_;
}

uint16_t AbilityRecovery::GetSaveModeFlag() const
{
    return saveMode_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS