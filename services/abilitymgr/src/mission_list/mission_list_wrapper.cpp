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

#include "mission_list_wrapper.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "configuration_convertor.h"
#include "distributed_client.h"
#include "distributed_parcel_helper.h"
#include "iservice_registry.h"
#include "mission_info_mgr.h"
#include "hilog_tag_wrapper.h"
#include "system_ability_definition.h"
#include "task_data_persistence_mgr.h"

#ifdef SUPPORT_SCREEN
#include "image_source.h"
#include "locale_config.h"
#include "window_manager.h"
#endif

namespace OHOS {
namespace AAFwk {
namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::ABILITYMGR,                                                         \
        "The caller is not system-app, can not use system-api");                               \
        return ERR_NOT_SYSTEM_APP;                                                             \
    }

constexpr int32_t MAX_NUMBER_OF_DISTRIBUTED_MISSIONS = 20;
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";

inline bool CheckIsRemote(const std::string &deviceId)
{
    return AbilityManagerService::GetPubInstance()->CheckIsRemote(deviceId);
}
inline bool GetLocalDeviceId(std::string &localDeviceId)
{
    return AbilityManagerService::GetPubInstance()->GetLocalDeviceId(localDeviceId);
}
inline bool JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    return AbilityManagerService::GetPubInstance()->JudgeSelfCalled(abilityRecord);
}
inline bool CheckCallerIsDmsProcess()
{
    return AbilityManagerService::GetPubInstance()->CheckCallerIsDmsProcess();
}
inline bool IsAbilityControllerForeground(const std::string &bundleName)
{
    return AbilityManagerService::GetPubInstance()->IsAbilityControllerForeground(bundleName);
}
inline bool IsAbilityControllerStartById(int32_t missionId)
{
    return AbilityManagerService::GetPubInstance()->IsAbilityControllerStartById(missionId);
}
inline std::shared_ptr<TaskHandlerWrap> GetTaskHandler()
{
    return AbilityManagerService::GetPubInstance()->GetTaskHandler();
}

std::u16string GetDmsInterfaceToken()
{
    return u"ohos.distributedschedule.accessToken";
}
sptr<IRemoteObject> GetDmsProxy()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "fail to get samgr.");
        return nullptr;
    }
    return samgrProxy->CheckSystemAbility(DISTRIBUTED_SCHED_SA_ID);
}

bool ReadMissionInfosFromParcel(Parcel& parcel,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    int32_t hasMissions = parcel.ReadInt32();
    if (hasMissions == 1) {
        int32_t len = parcel.ReadInt32();
        TAG_LOGD(AAFwkTag::DISTRIBUTED, "readLength is:%{public}d", len);
        if (len < 0) {
            return false;
        }
        size_t size = static_cast<size_t>(len);
        if ((size > parcel.GetReadableBytes()) || (missionInfos.max_size() < size)) {
            TAG_LOGE(AAFwkTag::DISTRIBUTED, "Failed to read MissionInfo vector, size = %{public}zu", size);
            return false;
        }
        missionInfos.clear();
        for (size_t i = 0; i < size; i++) {
            AAFwk::MissionInfo *ptr = parcel.ReadParcelable<AAFwk::MissionInfo>();
            if (ptr == nullptr) {
                TAG_LOGW(AAFwkTag::DISTRIBUTED, "read MissionInfo failed");
                return false;
            }
            missionInfos.emplace_back(*ptr);
            delete ptr;
        }
    }
    TAG_LOGI(AAFwkTag::DISTRIBUTED, "info size is:%{public}zu", missionInfos.size());
    return true;
}

int32_t DmsSetMissionContinueState(int32_t missionId, ContinueState state)
{
    TAG_LOGI(AAFwkTag::DISTRIBUTED,
        "SetMissionContinueState called. Mission id: %{public}d, state: %{public}d", missionId, state);
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "remote system ablity is null");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    MessageParcel reply;
    if (!data.WriteInterfaceToken(GetDmsInterfaceToken())) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    PARCEL_WRITE_HELPER(data, Int32, static_cast<int32_t>(state));
    PARCEL_TRANSACT_SYNC_RET_INT(remote, DistributedClient::SET_MISSION_CONTINUE_STATE, data, reply);
}

int32_t DmsGetMissionInfos(const std::string& deviceId, int32_t numMissions,
    std::vector<AAFwk::MissionInfo>& missionInfos)
{
    TAG_LOGI(AAFwkTag::DISTRIBUTED, "called");
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "remote system abiity is null");
        return INVALID_PARAMETERS_ERR;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDmsInterfaceToken())) {
        TAG_LOGD(AAFwkTag::DISTRIBUTED, "write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String16, Str8ToStr16(deviceId));
    PARCEL_WRITE_HELPER(data, Int32, numMissions);
    int32_t ret = remote->SendRequest(DistributedClient::GET_MISSION_INFOS, data, reply, option);
    if (ret != ERR_NONE) {
        TAG_LOGW(AAFwkTag::DISTRIBUTED, "sendRequest fail, error: %{public}d", ret);
        return ret;
    }
    return ReadMissionInfosFromParcel(reply, missionInfos) ? ERR_NONE : ERR_FLATTEN_OBJECT;
}

int32_t DmsGetMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    std::unique_ptr<MissionSnapshot>& missionSnapshot)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "deviceId is null");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = GetDmsProxy();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "remote is null");
        return INVALID_PARAMETERS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDmsInterfaceToken())) {
        return ERR_FLATTEN_OBJECT;
    }
    PARCEL_WRITE_HELPER(data, String, deviceId);
    PARCEL_WRITE_HELPER(data, Int32, missionId);
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(DistributedClient::GET_REMOTE_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != ERR_NONE) {
        TAG_LOGE(AAFwkTag::DISTRIBUTED, "transact failed, error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr(reply.ReadParcelable<MissionSnapshot>());
    missionSnapshot = std::move(missionSnapshotPtr);
    return ERR_NONE;
}

int32_t GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetRemoteMissionSnapshotInfo begin");
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr = std::make_unique<MissionSnapshot>();
    int result = DmsGetMissionSnapshotInfo(deviceId, missionId, missionSnapshotPtr);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetRemoteMissionSnapshotInfo failed, result = %{public}d", result);
        return result;
    }
    missionSnapshot = *missionSnapshotPtr;
    return ERR_OK;
}

MissionListWrapper g_missionListWrapperInstance;
} // namespace

#ifdef SUPPORT_SCREEN
class WindowFocusChangedListener : public Rosen::IFocusChangedListener {
public:
    virtual ~WindowFocusChangedListener() = default;

    void OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override
    {
        if (!focusChangeInfo) {
            return;
        }
        FocusStateInfo focusInfo;
        focusInfo.isFocus = true;
        focusInfo.token = focusChangeInfo->abilityToken_;
        MissionListWrapper::GetInstance().HandleFocusChange(focusInfo);
    }
    void OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override
    {
        if (!focusChangeInfo) {
            return;
        }
        FocusStateInfo focusInfo;
        focusInfo.isFocus = false;
        focusInfo.token = focusChangeInfo->abilityToken_;
        MissionListWrapper::GetInstance().HandleFocusChange(focusInfo);
    }
};
#endif

MissionListWrapper &MissionListWrapper::GetInstance()
{
    return g_missionListWrapperInstance;
}

MissionListWrapper::~MissionListWrapper() {}

void MissionListWrapper::InitMissionListManager(int32_t userId, bool switchUser)
{
    std::lock_guard lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        if (switchUser) {
            DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
            currentMissionListManager_ = it->second;
        }
        return;
    }
    auto manager = std::make_shared<MissionListManager>(userId);
    manager->Init();
    missionListManagers_.emplace(userId, manager);
    if (switchUser) {
        currentMissionListManager_ = manager;
    }
}

void MissionListWrapper::UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid)
{
    if (userId == U0_USER_ID) {
        auto missionListManagers = GetMissionListManagers();
        for (auto& item : missionListManagers) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto listManager = GetMissionListManagerByUserId(userId);
        if (listManager) {
            listManager->UninstallApp(bundleName, uid);
        }
    }
}

void MissionListWrapper::RemoveUser(int32_t userId)
{
    auto taskDataPersistenceMgr = DelayedSingleton<TaskDataPersistenceMgr>::GetInstance();
    if (!taskDataPersistenceMgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "taskDataPersistenceMgr is null");
        return;
    }
    taskDataPersistenceMgr->RemoveUserDir(userId);
    std::lock_guard lock(managersMutex_);
    missionListManagers_.erase(userId);
}

std::shared_ptr<MissionListManager> MissionListWrapper::GetMissionListManagerByUserId(int32_t userId)
{
    std::lock_guard lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get Manager. UserId = %{public}d", userId);
    return nullptr;
}

std::shared_ptr<AbilityRecord> MissionListWrapper::GetFocusAbility()
{
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> token = GetFocusWindow();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return nullptr;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
    }
    return abilityRecord;
#endif
    return nullptr;
}

std::shared_ptr<MissionListManager> MissionListWrapper::GetCurrentMissionListManager()
{
    std::lock_guard lock(managersMutex_);
    return currentMissionListManager_;
}

MissionListMap MissionListWrapper::GetMissionListManagers()
{
    std::lock_guard lock(managersMutex_);
    return missionListManagers_;
}

std::shared_ptr<MissionListManager> MissionListWrapper::GetMissionListManagerByCalling()
{
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (userId == U0_USER_ID) {
        return GetCurrentMissionListManager();
    }
    return GetMissionListManagerByUserId(userId);
}

int32_t MissionListWrapper::StartAbility(AbilityRequest &abilityRequest, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->StartAbility(abilityRequest);
}

int32_t MissionListWrapper::MoveAbilityToBackground(std::shared_ptr<AbilityRecord> abilityRecord, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MoveAbilityToBackground(abilityRecord);
}

int32_t MissionListWrapper::MinimizeAbility(sptr<IRemoteObject> token, bool fromUser,
    int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MinimizeAbility(token, fromUser);
}

int32_t MissionListWrapper::TerminateAbility(std::shared_ptr<AbilityRecord> abilityRecord, int32_t resultCode,
    const Want *resultWant, bool flag, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
}

#ifdef SUPPORT_SCREEN
int32_t MissionListWrapper::RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
    std::lock_guard lock(wmsHandlerMutex_);
    wmsHandler_ = handler;
    return ERR_OK;
}

sptr<IWindowManagerServiceHandler> MissionListWrapper::GetWmsHandler()
{
    std::lock_guard lock(wmsHandlerMutex_);
    return wmsHandler_;
}

sptr<IRemoteObject> MissionListWrapper::GetFocusWindow()
{
    auto wmsHandler = GetWmsHandler();
    if (wmsHandler == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "wmsHandler null");
        return nullptr;
    }

    sptr<IRemoteObject> token;
    wmsHandler->GetFocusWindow(token);
    return token;
}

void MissionListWrapper::CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    CHECK_POINTER(missionListManager);
    missionListManager->CompleteFirstFrameDrawing(abilityToken);
}

void MissionListWrapper::UpdateMissionSnapShot(sptr<IRemoteObject> token, std::shared_ptr<Media::PixelMap> pixelMap)
{
    if (!PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)) {
        return;
    }
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->UpdateSnapShot(token, pixelMap);
    }
}

int32_t MissionListWrapper::SetMissionLabel(sptr<IRemoteObject> token, const std::string &label)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel not self, not enabled");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager when set mission label.");
        return -1;
    }

    return missionListManager->SetMissionLabel(token, label);
}

int32_t MissionListWrapper::SetMissionIcon(sptr<IRemoteObject> token, std::shared_ptr<Media::PixelMap> icon)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not self, not enable to set mission icon");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager.");
        return -1;
    }

    return missionListManager->SetMissionIcon(token, icon);
}

namespace {
std::shared_ptr<Global::Resource::ResourceManager> CreateResourceManager(const std::string &loadPath)
{
    if (loadPath.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Invalid app resource.");
        return nullptr;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo(locale);
    AppExecFwk::Configuration cfg;
    if (AbilityManagerService::GetPubInstance()->GetConfiguration(cfg) == 0) {
        std::string colorMode = cfg.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getColorMode is %{public}s.", colorMode.c_str());
        resConfig->SetColorMode(AppExecFwk::ConvertColorMode(colorMode));
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "getColorMode failed.");
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    resourceMgr->UpdateResConfig(*resConfig);

    if (!resourceMgr->AddResource(loadPath.c_str())) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s AddResource failed.", __func__);
        return nullptr;
    }
    return resourceMgr;
}

std::shared_ptr<Media::PixelMap> GetPixelMap(const uint32_t windowIconId,
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr)
{
    if (resourceMgr == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s resource manager does not exist.", __func__);
        return nullptr;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource;
    std::unique_ptr<uint8_t[]> iconOut;
    size_t len = 0;
    if (resourceMgr->GetMediaDataById(windowIconId, len, iconOut) != Global::Resource::RState::SUCCESS) {
        return nullptr;
    }
    imageSource = Media::ImageSource::CreateImageSource(iconOut.get(), len, opts, errorCode);
    if (errorCode != 0 || imageSource == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create icon id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        TAG_LOGE(
            AAFwkTag::ABILITYMGR, "Failed to create PixelMap id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OUT.");
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}
} // namespace

std::string MissionListWrapper::GetLabel(std::string loadPath, int32_t labelId, int32_t abilityId)
{
    std::string strLabel;
    auto resourceMgr = CreateResourceManager(loadPath);
    if (!resourceMgr) {
        return strLabel;
    }

    auto result = resourceMgr->GetStringById(labelId, strLabel);
    if (result != OHOS::Global::Resource::RState::SUCCESS) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s. Failed to GetStringById.", __func__);
    }

    {
        std::lock_guard lock(abilityBgInfoMutex_);
        abilityBgInfos_.emplace(abilityId, std::make_shared<AbilityBgInfo>(AbilityBgInfo{0, nullptr, resourceMgr}));
    }

    return strLabel;
}

void MissionListWrapper::ReleaseAbilityBgInfo(int32_t abilityId)
{
    std::lock_guard lock(abilityBgInfoMutex_);
    abilityBgInfos_.erase(abilityId);
}

void MissionListWrapper::StartingWindowHot(sptr<AbilityTransitionInfo> info, int32_t missionId)
{
    auto windowHandler = GetWmsHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get WMS handler failed.");
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId);
    if (!pixelMap) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s, Get snapshot failed.", __func__);
    }

    windowHandler->StartingWindow(info, pixelMap);
}

void MissionListWrapper::StartingWindowCold(sptr<AbilityTransitionInfo> info, uint32_t bgImageId, uint32_t bgColorId,
    int32_t abilityId, const std::string &loadPath)
{
    auto windowHandler = GetWmsHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get WMS handler failed.");
        return;
    }
    std::shared_ptr<AbilityBgInfo> abilityBgInfo;
    {
        std::lock_guard lock(abilityBgInfoMutex_);
        auto it = abilityBgInfos_.find(abilityId);
        if (it != abilityBgInfos_.end() && it->second) {
            abilityBgInfo = it->second;
        }
    }
    if (abilityBgInfo == nullptr) {
        abilityBgInfo = std::make_shared<AbilityBgInfo>();
    }
    if (abilityBgInfo->resourceMgr == nullptr) {
        abilityBgInfo->resourceMgr = CreateResourceManager(loadPath);
        if (abilityBgInfo->resourceMgr == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Get resourceMgr failed.");
            return;
        }
    }
    if (abilityBgInfo->startingWindowBg == nullptr) {
        abilityBgInfo->startingWindowBg = GetPixelMap(bgImageId, abilityBgInfo->resourceMgr);
        auto colorErrval = abilityBgInfo->resourceMgr->GetColorById(bgColorId, abilityBgInfo->bgColor);
        if (colorErrval != OHOS::Global::Resource::RState::SUCCESS) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to GetColorById.");
            abilityBgInfo->bgColor = 0xdfffffff;
        }
    }

    windowHandler->StartingWindow(info, abilityBgInfo->startingWindowBg, abilityBgInfo->bgColor);
}

void MissionListWrapper::NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo,
    sptr<AbilityTransitionInfo> toInfo, bool& animaEnabled)
{
    auto windowHandler = GetWmsHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get WMS handler failed.");
        return;
    }
    windowHandler->NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
}

void MissionListWrapper::CancelStartingWindow(sptr<IRemoteObject> token)
{
    auto windowHandler = GetWmsHandler();
    if (!windowHandler) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Get WMS handler failed.");
        return;
    }
    windowHandler->CancelStartingWindow(token);
}
void MissionListWrapper::SetMissionAbilityState(int32_t missionId, AbilityState state)
{
    DelayedSingleton<MissionInfoMgr>::GetInstance()->SetMissionAbilityState(missionId, state);
}
void MissionListWrapper::NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info)
{
    auto windowHandler = GetWmsHandler();
    if (windowHandler) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Notification window manager UIAbility abnormal death.");
        windowHandler->NotifyAnimationAbilityDied(info);
    }
}

void MissionListWrapper::RegisterFocusListener()
{
    std::lock_guard lock(focusListenerMutex_);
    if (focusListener_ != nullptr) {
        return;
    }
    focusListener_ = sptr<WindowFocusChangedListener>(new WindowFocusChangedListener());
    Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
}

void MissionListWrapper::HandleFocusChange(const FocusStateInfo &focusChangeInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle focused event");
    auto taskHandler = GetTaskHandler();
    CHECK_POINTER(taskHandler);
    taskHandler->SubmitTask([this, focusChangeInfo]() {
        auto missionListManager = GetCurrentMissionListManager();
        CHECK_POINTER(missionListManager);

        int32_t missionId = missionListManager->GetMissionIdByAbilityToken(focusChangeInfo.token);
        if (focusChangeInfo.isFocus) {
            missionListManager->NotifyMissionFocused(missionId);
        } else {
            missionListManager->NotifyMissionUnfocused(missionId);
        }
        });
}
#endif

int32_t MissionListWrapper::MoveMissionToFront(int32_t missionId)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->MoveMissionToFront(missionId);
}

int32_t MissionListWrapper::MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->MoveMissionToFront(missionId, startOptions);
}

int32_t MissionListWrapper::GetMissionWantById(int32_t missionId, Want &missionWant)
{
    InnerMissionInfo innerMissionInfo;
    int32_t result = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        missionId, innerMissionInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "cannot find mission info from MissionInfoList by missionId: %{public}d", missionId);
        return result;
    }

    missionWant = innerMissionInfo.missionInfo.want;
    return ERR_OK;
}

int32_t MissionListWrapper::AttachAbilityThread(sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token,
    int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
    return missionListManager->AttachAbilityThread(scheduler, token);
}

void MissionListWrapper::DumpMissionList(const std::string &args, std::vector<std::string> &info,
    bool isClient, bool isUserID, int32_t userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        auto missionListManager = GetMissionListManagerByUserId(userId);
        if (missionListManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = missionListManager;
    } else {
        targetManager = GetCurrentMissionListManager();
    }

    CHECK_POINTER(targetManager);
    targetManager->DumpMissionList(info, isClient, args);
}

void MissionListWrapper::DumpSysAbilityInner(std::vector<std::string> &info, bool isClient, int32_t abilityId,
    const std::vector<std::string> &params, bool isUserID, int32_t userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        auto missionListManager = GetMissionListManagerByUserId(userId);
        if (missionListManager == nullptr) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = missionListManager;
    } else {
        targetManager = GetCurrentMissionListManager();
    }
    CHECK_POINTER(targetManager);
    targetManager->DumpMissionListByRecordId(info, isClient, abilityId, params);
}

void MissionListWrapper::DumpInner(std::vector<std::string> &info)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->Dump(info);
    }
}

void MissionListWrapper::DumpMissionListInner(std::vector<std::string> &info)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->DumpMissionList(info, false, "");
    }
}

void MissionListWrapper::DumpMissionInfosInner(std::vector<std::string> &info)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->DumpMissionInfos(info);
    }
}

void MissionListWrapper::DumpMissionInner(int32_t missionId, std::vector<std::string> &info)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_LOG(missionListManager, "Current mission manager not init.");
    missionListManager->DumpMission(missionId, info);
}

int32_t MissionListWrapper::AbilityTransitionDone(sptr<IRemoteObject> token, int32_t state,
    const PacMap &saveData, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->AbilityTransactionDone(token, state, saveData);
}

void MissionListWrapper::OnAbilityRequestDone(sptr<IRemoteObject> token, int32_t state, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. userId=%{public}d", userId);
        return;
    }
    missionListManager->OnAbilityRequestDone(token, state);
}

void MissionListWrapper::OnAppStateChanged(const AppInfo &info)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_LOG(missionListManager, "Current mission list manager not init.");
    missionListManager->OnAppStateChanged(info);
}

bool MissionListWrapper::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    auto manager = GetMissionListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
    if (manager && abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
        manager->OnAbilityDied(abilityRecord, currentUserId);
        return true;
    }
    return false;
}

void MissionListWrapper::OnCallConnectDied(std::shared_ptr<CallRecord> callRecord)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager) {
        missionListManager->OnCallConnectDied(callRecord);
    }
}

void MissionListWrapper::HandleLoadTimeOut(int32_t abilityRecordId, bool isHalf)
{
    auto missionListManagers = GetMissionListManagers();
    for (auto &item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordId, isHalf);
        }
    }
}

void MissionListWrapper::HandleInactiveTimeOut(int32_t abilityRecordId)
{
    auto missionListManagers = GetMissionListManagers();
    for (auto &item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void MissionListWrapper::HandleActiveTimeOut(int32_t abilityRecordId)
{
    auto missionListManagers = GetMissionListManagers();
    for (auto &item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void MissionListWrapper::HandleForegroundTimeOut(int32_t abilityRecordId, bool isHalf)
{
    auto missionListManagers = GetMissionListManagers();
    for (auto &item : missionListManagers) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecordId, isHalf);
        }
    }
}

bool MissionListWrapper::VerificationToken(sptr<IRemoteObject> token)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_RETURN_BOOL(missionListManager);

    if (missionListManager->GetAbilityRecordByToken(token)) {
        return true;
    }
    if (missionListManager->GetAbilityFromTerminateList(token)) {
        return true;
    }
    return false;
}

bool MissionListWrapper::VerificationAllToken(sptr<IRemoteObject> token)
{
    std::lock_guard lock(managersMutex_);
    for (auto &item: missionListManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return true;
        }
        if (item.second && item.second->GetAbilityFromTerminateList(token)) {
            return true;
        }
    }
    return false;
}

int32_t MissionListWrapper::GetMissionIdByAbilityToken(sptr<IRemoteObject> token, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. owner mission userId=%{public}d", userId);
        return -1;
    }
    return missionListManager->GetMissionIdByAbilityToken(token);
}

sptr<IRemoteObject> MissionListWrapper::GetAbilityTokenByMissionId(int32_t missionId)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return nullptr;
    }
    return missionListManager->GetAbilityTokenByMissionId(missionId);
}

int32_t MissionListWrapper::ResolveLocked(const AbilityRequest &abilityRequest, int32_t userId)
{
    auto missionListMgr = GetMissionListManagerByUserId(userId);
    if (missionListMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is Null. Designated User Id=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return missionListMgr->ResolveLocked(abilityRequest);
}

int32_t MissionListWrapper::ReleaseCall(sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->ReleaseCallLocked(connect, element);
}

void MissionListWrapper::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return;
    }
    missionListManager->OnAcceptWantResponse(want, flag);
}
void MissionListWrapper::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    auto missionListManager = GetCurrentMissionListManager();
    if (!missionListManager) {
        return;
    }
    missionListManager->OnStartSpecifiedAbilityTimeoutResponse(want);
}

int32_t MissionListWrapper::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_INVALID_VALUE);
    missionListManager->GetAbilityRunningInfos(info, isPerm);
    return ERR_OK;
}

void MissionListWrapper::EnableRecoverAbility(int32_t missionId, int32_t userId)
{
    auto missionListMgr = GetMissionListManagerByUserId(userId);
    if (missionListMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is nullptr");
        return;
    }
    missionListMgr->EnableRecoverAbility(missionId);
}

void MissionListWrapper::PauseOldMissionListManager(int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldMissionListManager:%{public}d-----begin", __func__, userId);
    auto manager = GetMissionListManagerByUserId(userId);
    CHECK_POINTER(manager);
    manager->PauseManager();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, PauseOldMissionListManager:%{public}d-----end", __func__, userId);
}

void MissionListWrapper::StartUserApps()
{
    auto missionListManager = GetCurrentMissionListManager();
    if (missionListManager && missionListManager->IsStarted()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "missionListManager ResumeManager");
        missionListManager->ResumeManager();
    }
}

int32_t MissionListWrapper::DoAbilityForeground(std::shared_ptr<AbilityRecord> abilityRecord, uint32_t flag)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->DoAbilityForeground(abilityRecord, flag);
}
#ifdef ABILITY_COMMAND_FOR_TEST
int32_t MissionListWrapper::BlockAbility(int32_t abilityRecordId)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    return missionListManager->BlockAbility(abilityRecordId);
}
#endif
std::shared_ptr<AbilityRecord> MissionListWrapper::GetAbilityRecordByMissionId(int32_t missionId)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, nullptr);
    std::shared_ptr<Mission> mission = missionListManager->GetMissionById(missionId);
    if (!mission) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "mission is null.");
        return nullptr;
    }
    return mission->GetAbilityRecord();
}

bool MissionListWrapper::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> targetRecord, int32_t userId)
{
    auto missionListMgr = GetMissionListManagerByUserId(userId);
    if (missionListMgr == nullptr) {
        return false;
    }
    return missionListMgr->IsAbilityStarted(abilityRequest, targetRecord);
}

void MissionListWrapper::CallRequestDone(std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callStub)
{
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER(missionListManager);
    missionListManager->CallRequestDone(abilityRecord, callStub);
}

int32_t MissionListWrapper::SignRestartAppFlag(const std::string &bundleName, int32_t userId)
{
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (missionListManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is nullptr. userId:%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    missionListManager->SignRestartAppFlag(bundleName);
    return ERR_OK;
}

int32_t MissionListWrapper::SetMissionLockedState(int32_t missionId, bool lockedState)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request unlock mission for clean up all, id :%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->SetMissionLockedState(missionId, lockedState);
}

int32_t MissionListWrapper::RegisterMissionListener(sptr<IMissionListener> listener)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request RegisterMissionListener ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->RegisterMissionListener(listener);
}

int32_t MissionListWrapper::UnRegisterMissionListener(sptr<IMissionListener> listener)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request UnRegisterMissionListener ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return missionListManager->UnRegisterMissionListener(listener);
}

int32_t MissionListWrapper::GetMissionInfos(const std::string &deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request GetMissionInfos.");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfos(deviceId, numMax, missionInfos);
    }

    return missionListManager->GetMissionInfos(numMax, missionInfos);
}

int32_t MissionListWrapper::GetRemoteMissionInfos(const std::string &deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetRemoteMissionInfos begin");
    int32_t result = DmsGetMissionInfos(deviceId, numMax, missionInfos);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetRemoteMissionInfos failed, result = %{public}d", result);
        return result;
    }
    return ERR_OK;
}

int32_t MissionListWrapper::GetMissionInfo(const std::string &deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request GetMissionInfo, missionId:%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfo(deviceId, missionId, missionInfo);
    }

    return missionListManager->GetMissionInfo(missionId, missionInfo);
}

int32_t MissionListWrapper::GetRemoteMissionInfo(const std::string &deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetMissionInfoFromDms begin");
    std::vector<MissionInfo> missionVector;
    int32_t result = GetRemoteMissionInfos(deviceId, MAX_NUMBER_OF_DISTRIBUTED_MISSIONS, missionVector);
    if (result != ERR_OK) {
        return result;
    }
    for (auto iter = missionVector.begin(); iter != missionVector.end(); iter++) {
        if (iter->id == missionId) {
            missionInfo = *iter;
            return ERR_OK;
        }
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "missionId not found");
    return ERR_INVALID_VALUE;
}

int32_t MissionListWrapper::CleanMission(int32_t missionId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request CleanMission, missionId:%{public}d", missionId);
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return missionListManager->ClearMission(missionId);
}

int32_t MissionListWrapper::CleanAllMissions()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "request CleanAllMissions ");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME);
    if (!AbilityManagerService::GetPubInstance()->IsAbilityControllerStart(
        want, AbilityConfig::LAUNCHER_BUNDLE_NAME)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart failed: %{public}s", want.GetBundle().c_str());
        return ERR_WOULD_BLOCK;
    }

    return missionListManager->ClearAllMissions();
}

int32_t MissionListWrapper::MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    auto wmsHandler = GetWmsHandler();
    if (wmsHandler) {
        auto ret = wmsHandler->MoveMissionsToForeground(missionIds, topMissionId);
        if (ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionsToForeground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
#endif // SUPPORT_SCREEN
    return ERR_NO_INIT;
}

int32_t MissionListWrapper::MoveMissionsToBackground(const std::vector<int32_t> &missionIds,
    std::vector<int32_t> &result)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    auto wmsHandler = GetWmsHandler();
    if (wmsHandler) {
        auto ret = wmsHandler->MoveMissionsToBackground(missionIds, result);
        if (ret) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionsToBackground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
#endif // SUPPORT_SCREEN
    return ERR_NO_INIT;
}

int32_t MissionListWrapper::GetTopAbility(sptr<IRemoteObject> &token)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_SCREEN
    auto wmsHandler = GetWmsHandler();
    if (!wmsHandler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wmsHandler_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    wmsHandler->GetFocusWindow(token);

    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return ERR_INVALID_VALUE;
    }
#endif
    return ERR_OK;
}

AppExecFwk::ElementName MissionListWrapper::GetTopAbility(bool isNeedLocalDeviceId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s start.", __func__);
    AppExecFwk::ElementName elementName = {};
#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    int32_t ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret) {
        return elementName;
    }
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr");
        return elementName;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s abilityRecord is null.", __func__);
        return elementName;
    }
    elementName = abilityRecord->GetElementName();
    bool isDeviceEmpty = elementName.GetDeviceID().empty();
    std::string localDeviceId;
    if (isDeviceEmpty && isNeedLocalDeviceId && GetLocalDeviceId(localDeviceId)) {
        elementName.SetDeviceID(localDeviceId);
    }
#endif
    return elementName;
}

int32_t MissionListWrapper::GetMissionIdByToken(sptr<IRemoteObject> token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "request GetMissionIdByToken.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is null.");
        return ERR_INVALID_VALUE;
    }
    if (!JudgeSelfCalled(abilityRecord) && !CheckCallerIsDmsProcess()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission deny.");
        return ERR_INVALID_VALUE;
    }
    return GetMissionIdByAbilityToken(token, abilityRecord->GetOwnerMissionUserId());
}

int32_t MissionListWrapper::RegisterSnapshotHandler(sptr<ISnapshotHandler> handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return 0;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, INNER_ERR);
    missionListManager->RegisterSnapshotHandler(handler);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerService register snapshot handler success.");
    return ERR_OK;
}

int32_t MissionListWrapper::MoveAbilityToBackground(sptr<IRemoteObject> token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Move ability to background begin");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(ownerUserId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MoveAbilityToBackground(abilityRecord);
}

int32_t MissionListWrapper::GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
    MissionSnapshot &missionSnapshot, bool isLowResolution)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "get remote mission snapshot.");
        return GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshot);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "get local mission snapshot.");
    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, INNER_ERR);
    auto token = GetAbilityTokenByMissionId(missionId);
    bool result = missionListManager->GetMissionSnapshot(missionId, token, missionSnapshot, isLowResolution);
    if (!result) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int32_t MissionListWrapper::DelegatorDoAbilityForeground(sptr<IRemoteObject> token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t appPid = abilityRecord->GetPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerPid: %{public}d, appPid: %{public}d", callerPid, appPid);
    if (callerPid != appPid) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller is not the application itself");
        return ERR_INVALID_VALUE;
    }

    auto missionListManager = GetCurrentMissionListManager();
    CHECK_POINTER_AND_RETURN(missionListManager, ERR_NO_INIT);
    auto missionId = missionListManager->GetMissionIdByAbilityToken(token);
    if (missionId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid mission id.");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "enter missionId : %{public}d", missionId);
    if (!IsAbilityControllerStartById(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    return missionListManager->MoveMissionToFront(missionId);
}

int32_t MissionListWrapper::DelegatorDoAbilityBackground(sptr<IRemoteObject> token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t appPid = abilityRecord->GetPid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerPid: %{public}d, appPid: %{public}d", callerPid, appPid);
    if (callerPid != appPid) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Caller is not the application itself");
        return ERR_INVALID_VALUE;
    }
    return MinimizeAbility(token, true, abilityRecord->GetOwnerMissionUserId());
}

int32_t MissionListWrapper::SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord null. state: %{public}d", state);
        return -1;
    }

    int32_t missionId = abilityRecord->GetMissionId();
    if (missionId == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState failed to get missionId. State: %{public}d", state);
        return ERR_INVALID_VALUE;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SetMissionContinueState not self, not enabled. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetMissionListManagerByUserId(userId);
    if (!missionListManager) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to find mission list manager. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto setResult = missionListManager->SetMissionContinueState(token, missionId, state);
    if (setResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "missionListManager set failed, result: %{public}d, mission id: %{public}d, state: %{public}d",
            setResult, missionId, state);
        return setResult;
    }

    auto result = DmsSetMissionContinueState(missionId, state);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "Notify DMS client failed, result: %{public}d. Mission id: %{public}d, state: %{public}d",
            result, missionId, state);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "SetMissionContinueState end. Mission id: %{public}d, state: %{public}d", missionId, state);
    return ERR_OK;
}

int32_t MissionListWrapper::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    auto missionListMgr = GetMissionListManagerByUserId(userId);
    if (missionListMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionListMgr is nullptr.");
        return ERR_INVALID_VALUE;
    }

    return missionListMgr->IsValidMissionIds(missionIds, results);
}

void MissionListWrapper::GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityLists,
    int32_t pid, ActiveAbilityMode mode, int32_t userId)
{
    std::shared_ptr<MissionListManager> missionListMgr;
    if (mode == ActiveAbilityMode::CALLING_UID) {
        missionListMgr = GetMissionListManagerByCalling();
    } else if (mode == ActiveAbilityMode::USER_ID) {
        missionListMgr == GetMissionListManagerByUserId(userId);
    } else {
        auto missionListManagers = GetMissionListManagers();
        for (auto& item: missionListManagers) {
            if (!item.second) {
                continue;
            }
            std::vector<std::string> abilityList;
            item.second->GetActiveAbilityList(bundleName, abilityList, pid);
            if (!abilityList.empty()) {
                abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
            }
        }
        return;
    }

    if (missionListMgr) {
        missionListMgr->GetActiveAbilityList(bundleName, abilityLists, pid);
    }
}
} // namespace AAFwk
} // namespace OHOS