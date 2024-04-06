/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_mgr_service_inner.h"

#include <cinttypes>
#include <csignal>
#include <mutex>
#include <queue>
#include <securec.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "ability_window_configuration.h"
#include "accesstoken_kit.h"
#include "app_config_data_manager.h"
#include "app_mem_info.h"
#include "app_mgr_service.h"
#include "app_process_data.h"
#include "app_state_observer_manager.h"
#include "app_utils.h"
#include "appfreeze_manager.h"
#include "application_state_observer_stub.h"
#include "appspawn_mount_permission.h"
#include "appspawn_util.h"
#include "bundle_constants.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "distributed_data_mgr.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif
#include "mem_mgr_client.h"
#include "mem_mgr_process_state_info.h"
#include "os_account_manager_wrapper.h"
#include "parameter.h"
#include "parameters.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "render_state_observer_manager.h"
#include "system_ability_definition.h"
#include "string_ex.h"
#include "time_util.h"
#include "ui_extension_utils.h"
#include "uri_permission_manager_client.h"
#ifdef APP_MGR_SERVICE_APPMS
#include "net_conn_client.h"
#endif
#include "application_info.h"
#include "meminfo.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Rosen;
using namespace OHOS::Security;

namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        TAG_LOGE(AAFwkTag::APPMGR, "The caller is not system-app, can not use system-api");    \
        return AAFwk::ERR_NOT_SYSTEM_APP;                                                      \
    }

#define CHECK_IS_SA_CALL(listener)                                                                              \
    auto instance = AAFwk::PermissionVerification::GetInstance();                                               \
    if ((listener) == nullptr || instance == nullptr || appRunningStatusModule_ == nullptr) {                     \
        TAG_LOGE(AAFwkTag::APPMGR, "Listener or getInstance is nullptr or appRunningStatusModule_ is nullptr"); \
        return ERR_INVALID_VALUE;                                                                               \
    }                                                                                                           \
    if (!instance->IsSACall()) {                                                                                \
        TAG_LOGE(AAFwkTag::APPMGR, "CallerToken not SA.");                                                      \
        return ERR_PERMISSION_DENIED;                                                                           \
    }

// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delay time setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
// delay register focus listener to wms
constexpr int REGISTER_FOCUS_DELAY = 5000;
constexpr int REGISTER_VISIBILITY_DELAY = 5000;
// Max render process number limitation for phone device.
constexpr int PHONE_MAX_RENDER_PROCESS_NUM = 40;
const std::string CLASS_NAME = "ohos.app.MainThread";
const std::string FUNC_NAME = "main";
const std::string RENDER_PARAM = "invalidparam";
const std::string COLD_START = "coldStart";
const std::string PERF_CMD = "perfCmd";
const std::string DEBUG_CMD = "debugCmd";
const std::string ENTER_SANDBOX = "sandboxApp";
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
const std::string PERMISSION_INTERNET = "ohos.permission.INTERNET";
const std::string PERMISSION_MANAGE_VPN = "ohos.permission.MANAGE_VPN";
const std::string PERMISSION_ACCESS_BUNDLE_DIR = "ohos.permission.ACCESS_BUNDLE_DIR";
const std::string PERMISSION_GET_BUNDLE_RESOURCES = "ohos.permission.GET_BUNDLE_RESOURCES";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
const std::string SUPPORT_ISOLATION_MODE = "persist.bms.supportIsolationMode";
const std::string SUPPORT_SERVICE_EXT_MULTI_PROCESS = "component.startup.extension.multiprocess.enable";
const std::string SERVICE_EXT_MULTI_PROCESS_WHITE_LIST = "component.startup.extension.multiprocess.whitelist";
const std::string SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
const std::string DEBUG_APP = "debugApp";
const std::string SERVICE_EXTENSION = ":ServiceExtension";
const std::string KEEP_ALIVE = ":KeepAlive";
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TSAN_FLAG_NAME = "tsanEnabled";
const std::string MEMMGR_PROC_NAME = "memmgrservice";
const int32_t SIGNAL_KILL = 9;
constexpr int32_t USER_SCALE = 200000;
#define ENUM_TO_STRING(s) #s
#define APP_ACCESS_BUNDLE_DIR 0x20
#define APP_OVERLAY_FLAG 0x100
#define GET_BUNDLE_RESOURCES_FLAG 0x200

constexpr int32_t BASE_USER_RANGE = 200000;

constexpr int32_t MAX_RESTART_COUNT = 3;
constexpr int32_t RESTART_INTERVAL_TIME = 120000;

constexpr ErrCode APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, 0x01);
 // Error code for already exist render.
constexpr ErrCode ERR_ALREADY_EXIST_RENDER = APPMGR_ERR_OFFSET + 100;
 // Error code for reaching render process number limitation.
constexpr ErrCode ERR_REACHING_MAXIMUM_RENDER_PROCESS_LIMITATION = APPMGR_ERR_OFFSET + 101;
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";

// Developer mode param
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
constexpr char PRODUCT_ASSERT_FAULT_DIALOG_ENABLED[] = "persisit.sys.abilityms.support_assert_fault_dialog";

// Msg length is less than 48 characters
const std::string EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT = "Terminate Ability TimeOut!";
const std::string EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT = "Terminate Application TimeOut!";
const std::string EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT = "Add Ability Stage TimeOut!";
const std::string EVENT_MESSAGE_START_SPECIFIED_PROCESS_TIMEOUT = "Start Specified Process Timeout!";
const std::string EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT = "Start Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT = "Start Process Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_DEFAULT = "AppMgrServiceInner HandleTimeOut!";

const std::string SYSTEM_BASIC = "system_basic";
const std::string SYSTEM_CORE = "system_core";
const std::string ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
const std::string PROCESS_EXIT_EVENT_TASK = "Send Process Exit Event Task";
const std::string KILL_PROCESS_REASON_PREFIX = "Kill Reason:";

constexpr int32_t ROOT_UID = 0;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t QUICKFIX_UID = 5524;
constexpr int32_t DEFAULT_USER_ID = 0;

constexpr int32_t BLUETOOTH_GROUPID = 1002;

#ifdef APP_MGR_SERVICE_APPMS
constexpr int32_t NETSYS_SOCKET_GROUPID = 1097;
#endif

constexpr int32_t DEFAULT_INVAL_VALUE = -1;

int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}

bool VerifyPermission(const BundleInfo &bundleInfo, const std::string &permissionName)
{
    if (permissionName.empty() || bundleInfo.reqPermissions.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "permissionName or reqPermissions is empty.");
        return false;
    }

    bool ret = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [permissionName] (const auto &reqPermission) {
            if (permissionName == reqPermission) {
                return true;
            }
            return false;
        });
    if (!ret) {
        TAG_LOGI(AAFwkTag::APPMGR, "Not request permission %{public}s", permissionName.c_str());
        return ret;
    }

    auto token = bundleInfo.applicationInfo.accessTokenId;
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, permissionName, false);
    if (result != Security::AccessToken::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::APPMGR, "StartProcess permission %{public}s not granted", permissionName.c_str());
        return false;
    }

    return true;
}
}  // namespace

using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;
using OHOS::AppExecFwk::Constants::PERMISSION_NOT_GRANTED;

AppMgrServiceInner::AppMgrServiceInner()
    : appProcessManager_(std::make_shared<AppProcessManager>()),
      remoteClientManager_(std::make_shared<RemoteClientManager>()),
      appRunningManager_(std::make_shared<AppRunningManager>()),
      configuration_(std::make_shared<Configuration>()),
      appDebugManager_(std::make_shared<AppDebugManager>()),
      appRunningStatusModule_(std::make_shared<AbilityRuntime::AppRunningStatusModule>()),
      securityModeManager_(std::make_shared<AdvancedSecurityModeManager>())
{}

void AppMgrServiceInner::Init()
{
    InitGlobalConfiguration();
    AddWatchParameter();
    supportIsolationMode_ = OHOS::system::GetParameter(SUPPORT_ISOLATION_MODE, "false");
    supportServiceExtMultiProcess_ = OHOS::system::GetParameter(SUPPORT_SERVICE_EXT_MULTI_PROCESS, "false");
    ParseServiceExtMultiProcessWhiteList();
    DelayedSingleton<AppStateObserverManager>::GetInstance()->Init();
    DelayedSingleton<RenderStateObserverManager>::GetInstance()->Init();
    dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue");
    if (securityModeManager_) {
        securityModeManager_->Init();
    }
}

AppMgrServiceInner::~AppMgrServiceInner()
{}

void AppMgrServiceInner::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);

    int32_t appIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, processName);
    TAG_LOGD(AAFwkTag::APPMGR, "processName = %{public}s", processName.c_str());
    auto mainAppRecord =
        appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (mainAppRecord != nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "main process exists.");
        mainAppRecord->SetScheduleNewProcessRequestState(true, want, hapModuleInfo.moduleName);
        auto moduleRecord = mainAppRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            TAG_LOGD(AAFwkTag::APPMGR, "module record is nullptr, add modules");
            std::vector<HapModuleInfo> hapModules = { hapModuleInfo };
            mainAppRecord->AddModules(appInfo, hapModules);
            mainAppRecord->AddAbilityStageBySpecifiedProcess(appInfo->bundleName);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "schedule new process request.");
        mainAppRecord->ScheduleNewProcessRequest(want, hapModuleInfo.moduleName);
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "main process do not exists.");
    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestResponse(want, "");
    }
}

void AppMgrServiceInner::LoadAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPMGR, "name:%{public}s.", abilityInfo->name.c_str());
    if (!CheckLoadAbilityConditions(token, abilityInfo, appInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "CheckLoadAbilityConditions failed");
        return;
    }
    if (abilityInfo->type == AbilityType::PAGE) {
        AbilityRuntime::FreezeUtil::LifecycleFlow flow = {token, AbilityRuntime::FreezeUtil::TimeoutState::LOAD};
        auto entry = std::to_string(AbilityRuntime::TimeUtil::SystemTimeMillisecond()) +
            "; AppMgrServiceInner::LoadAbility; the load lifecycle.";
        AbilityRuntime::FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = (want == nullptr) ? 0 : want->GetIntParam(DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleAndHapInfo failed");
        return;
    }

    std::string processName;
    MakeProcessName(abilityInfo, appInfo, hapModuleInfo, appIndex, processName);
    TAG_LOGD(AAFwkTag::APPMGR, "processName = %{public}s", processName.c_str());

    std::shared_ptr<AppRunningRecord> appRecord;
    // for isolation process
    std::string specifiedProcessFlag = "";
    bool isUIAbility = (abilityInfo->type == AppExecFwk::AbilityType::PAGE && abilityInfo->isStageBasedModel);
    bool isSpecifiedProcess = abilityInfo->isolationProcess &&
        AAFwk::AppUtils::GetInstance().IsStartSpecifiedProcess() && isUIAbility;
    if (isSpecifiedProcess) {
        specifiedProcessFlag = want->GetStringParam(PARAM_SPECIFIED_PROCESS_FLAG);
        TAG_LOGI(AAFwkTag::APPMGR, "specifiedProcessFlag = %{public}s", specifiedProcessFlag.c_str());
    }
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name,
        processName, appInfo->uid, bundleInfo, specifiedProcessFlag);
    if (appRecord && isUIAbility) {
        NotifyMemMgrPriorityChanged(appRecord);
    }

    if (!appRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "appRecord null");
        bool appExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByBundleName(bundleInfo.name);
        if (!appExistFlag) {
            NotifyAppRunningStatusEvent(
                bundleInfo.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
        }
        appRecord = CreateAppRunningRecord(token, preToken, appInfo, abilityInfo,
            processName, bundleInfo, hapModuleInfo, want, abilityRecordId);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "CreateAppRunningRecord failed, appRecord is nullptr");
            return;
        }
        if (isSpecifiedProcess && !specifiedProcessFlag.empty()) {
            appRecord->SetSpecifiedProcessFlag(specifiedProcessFlag);
        }
        if (hapModuleInfo.isStageBasedModel && !IsMainProcess(appInfo, hapModuleInfo)) {
            appRecord->SetKeepAliveAppState(false, false);
            TAG_LOGI(AAFwkTag::APPMGR, "The process %{public}s will not keepalive", hapModuleInfo.process.c_str());
        }
        SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::COLD);
        auto callRecord = GetAppRunningRecordByAbilityToken(preToken);
        if (callRecord != nullptr) {
            auto launchReson = (want == nullptr) ? 0 : want->GetIntParam("ohos.ability.launch.reason", 0);
            TAG_LOGD(AAFwkTag::APPMGR, "req: %{public}d, proc: %{public}s, call:%{public}d,%{public}s", launchReson,
                appInfo->name.c_str(), appRecord->GetCallerPid(), callRecord->GetBundleName().c_str());
        }
        uint32_t startFlags = (want == nullptr) ? 0 : AppspawnUtil::BuildStartFlags(*want, *abilityInfo);
        int32_t bundleIndex = (want == nullptr) ? 0 : want->GetIntParam(DLP_PARAMS_INDEX, 0);
        StartProcess(abilityInfo->applicationName, processName, startFlags, appRecord,
            appInfo->uid, bundleInfo, appInfo->bundleName, bundleIndex, appExistFlag);
        std::string perfCmd = (want == nullptr) ? "" : want->GetStringParam(PERF_CMD);
        bool isSandboxApp = (want == nullptr) ? false : want->GetBoolParam(ENTER_SANDBOX, false);
        (void)StartPerfProcess(appRecord, perfCmd, "", isSandboxApp);
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "have apprecord");
        SendAppStartupTypeEvent(appRecord, abilityInfo, AppStartType::MULTI_INSTANCE);
        int32_t requestProcCode = (want == nullptr) ? 0 : want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (requestProcCode != 0 && appRecord->GetRequestProcCode() == 0) {
            appRecord->SetRequestProcCode(requestProcCode);
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessReused(appRecord);
        }
        StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want, abilityRecordId);
    }
    PerfProfile::GetInstance().SetAbilityLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
    PerfProfile::GetInstance().Reset();
    appRecord->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);
}

bool AppMgrServiceInner::CheckLoadAbilityConditions(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    if (!token || !abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return false;
    }
    if (abilityInfo->name.empty() || appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "error abilityInfo or appInfo");
        return false;
    }
    if (abilityInfo->applicationName != appInfo->name) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo and appInfo have different appName, don't load for it");
        return false;
    }

    return true;
}

void AppMgrServiceInner::MakeServiceExtProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, std::string &processName) const
{
    if (abilityInfo == nullptr || appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Ability info or app info is nullptr.");
        return;
    }

    if (supportServiceExtMultiProcess_.compare("true") != 0) {
        return;
    }

    if (processName == appInfo->bundleName &&
        abilityInfo->extensionAbilityType == ExtensionAbilityType::SERVICE) {
        auto iter = std::find(
            serviceExtensionWhiteList_.begin(), serviceExtensionWhiteList_.end(), processName);
        if (iter != serviceExtensionWhiteList_.end()) {
            TAG_LOGD(AAFwkTag::APPMGR, "Application is in whiteList, skipping!");
            return;
        }

        processName += SERVICE_EXTENSION;
        if (appInfo->keepAlive) {
            processName += KEEP_ALIVE;
        }
    }
}

void AppMgrServiceInner::MakeProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, int32_t appIndex,
    std::string &processName) const
{
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }
    if (!abilityInfo->process.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "Process not null");
        processName = abilityInfo->process;
        return;
    }
    MakeProcessName(appInfo, hapModuleInfo, processName);
    MakeServiceExtProcessName(abilityInfo, appInfo, processName);
    if (appIndex != 0) {
        processName += std::to_string(appIndex);
    }
}

void AppMgrServiceInner::MakeProcessName(
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, std::string &processName) const
{
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "appInfo nill");
        return;
    }
    // check after abilityInfo, because abilityInfo contains extension process.
    if (hapModuleInfo.isStageBasedModel && !hapModuleInfo.process.empty()
        && hapModuleInfo.process != appInfo->bundleName) {
        processName = hapModuleInfo.process;
        TAG_LOGI(AAFwkTag::APPMGR, "Stage mode, Make processName:%{public}s", processName.c_str());
        return;
    }
    bool isRunInIsolationMode = CheckIsolationMode(hapModuleInfo);
    if (hapModuleInfo.isStageBasedModel && isRunInIsolationMode) {
        processName = appInfo->bundleName;
        processName.append(":");
        processName.append(hapModuleInfo.name);
        return;
    }
    if (!appInfo->process.empty()) {
        processName = appInfo->process;
        return;
    }
    processName = appInfo->bundleName;
}

bool AppMgrServiceInner::IsMainProcess(const std::shared_ptr<ApplicationInfo> &appInfo,
    const HapModuleInfo &hapModuleInfo) const
{
    if (!appInfo) {
        return true;
    }
    if (hapModuleInfo.process.empty()) {
        return true;
    }
    if (!appInfo->process.empty()) {
        if (hapModuleInfo.process == appInfo->process) {
            return true;
        }
    } else {
        if (hapModuleInfo.process == appInfo->bundleName) {
            return true;
        }
    }
    return false;
}

bool AppMgrServiceInner::CheckIsolationMode(const HapModuleInfo &hapModuleInfo) const
{
    IsolationMode isolationMode = hapModuleInfo.isolationMode;
    if (supportIsolationMode_.compare("true") == 0) {
        switch (isolationMode) {
            case IsolationMode::ISOLATION_FIRST:
                return true;
            case IsolationMode::ISOLATION_ONLY:
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool AppMgrServiceInner::GetBundleAndHapInfo(const AbilityInfo &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo,
    int32_t appIndex) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }

    auto userId = GetUserIdByUid(appInfo->uid);
    TAG_LOGD(AAFwkTag::APPMGR, "UserId:%{public}d.", userId);
    bool bundleMgrResult;
    if (appIndex == 0) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(appInfo->bundleName,
            BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
    } else {
        bundleMgrResult = (IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(appInfo->bundleName,
            appIndex, userId, bundleInfo)) == 0);
    }

    if (!bundleMgrResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleInfo is fail.");
        return false;
    }
    if (appIndex == 0) {
        bundleMgrResult = bundleMgrHelper->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
    } else {
        bundleMgrResult = (bundleMgrHelper->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo) == 0);
    }
    if (!bundleMgrResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetHapModuleInfo is fail.");
        return false;
    }

    return true;
}

void AppMgrServiceInner::AttachApplication(const pid_t pid, const sptr<IAppScheduler> &appScheduler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", pid);
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
        return;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    AAFwk::EventInfo eventInfo;
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + eventInfo.bundleName;
    HITRACE_METER_NAME(HITRACE_TAG_APP, traceName);
    if (!appScheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "app client is null");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "attach, pid:%{public}d.", pid);
    sptr<AppDeathRecipient> appDeathRecipient = new (std::nothrow) AppDeathRecipient();
    if (appDeathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to create death recipient.");
        return;
    }
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    auto object = appScheduler->AsObject();
    if (!object || !object->AddDeathRecipient(appDeathRecipient)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to add DeathRecipient for %{public}s.", appRecord->GetProcessName().c_str());
        return;
    }

    appRecord->SetAppDeathRecipient(appDeathRecipient);
    appRecord->SetApplicationClient(appScheduler);
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        LaunchApplication(appRecord);
    }
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_ATTACH, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is null");
        return;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    std::string bundleName = "";
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        bundleName = applicationInfo->name;
    }
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + bundleName;
    HITRACE_METER_NAME(HITRACE_TAG_APP, traceName);

    if (!configuration_) {
        TAG_LOGE(AAFwkTag::APPMGR, "configuration_ is null");
        return;
    }

    if (appRecord->GetState() != ApplicationState::APP_STATE_CREATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "wrong app state:%{public}d", appRecord->GetState());
        return;
    }

    appRecord->LaunchApplication(*configuration_);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    int restartResidentProcCount = MAX_RESTART_COUNT;
    appRecord->SetRestartResidentProcCount(restartResidentProcCount);

    // There is no ability when the empty resident process starts
    // The status of all resident processes is ready
    // There is no process of switching the foreground, waiting for his first ability to start
    if (appRecord->IsEmptyKeepAliveApp()) {
        appRecord->AddAbilityStage();
        return;
    }

    if (appRecord->IsStartSpecifiedAbility()) {
        appRecord->AddAbilityStageBySpecifiedAbility(appRecord->GetBundleName());
        return;
    }
    appRecord->LaunchPendingAbilities();
    SendAppLaunchEvent(appRecord);
}

void AppMgrServiceInner::AddAbilityStageDone(const int32_t recordId)
{
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app record failed");
        return;
    }
    appRecord->AddAbilityStageDone();
}

void AppMgrServiceInner::ApplicationForegrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord || (!appRecord->IsUpdateStateFromService()
        && appRecord->GetApplicationPendingState() != ApplicationPendingState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app record failed");
        return;
    }
    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_READY || appState == ApplicationState::APP_STATE_BACKGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        bool needNotifyApp = appRunningManager_->IsApplicationFirstForeground(*appRecord);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp, false);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appState));
    }
    appRecord->SetUpdateStateFromService(false);
    appRecord->SetApplicationPendingState(ApplicationPendingState::READY);
    appRecord->PopForegroundingAbilityTokens();

    // push the foregrounded app front of RecentAppList.
    PushAppFront(recordId);
    TAG_LOGD(AAFwkTag::APPMGR, "application is foregrounded");
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
        eventInfo.bundleType = static_cast<int32_t>(applicationInfo->bundleType);
    }
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    eventInfo.processType = static_cast<int32_t>(appRecord->GetProcessType());
    int32_t callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerRecord = GetAppRunningRecordByPid(callerPid);
    if (callerRecord != nullptr) {
        eventInfo.callerBundleName = callerRecord->GetBundleName();
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "callerRecord is nullptr, can not get callerBundleName.");
    }
    AAFwk::EventReport::SendAppForegroundEvent(AAFwk::EventName::APP_FOREGROUND, eventInfo);
}

void AppMgrServiceInner::ApplicationBackgrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord || !appRecord->IsUpdateStateFromService()) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app record failed");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        bool needNotifyApp = !AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())
            && !AAFwk::UIExtensionUtils::IsWindowExtension(appRecord->GetExtensionType())
            && appRunningManager_->IsApplicationBackground(appRecord->GetBundleName());
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND, needNotifyApp, false);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        TAG_LOGW(AAFwkTag::APPMGR, "app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appRecord->GetState()));
    }
    appRecord->SetUpdateStateFromService(false);
    if (appRecord->GetApplicationPendingState() == ApplicationPendingState::BACKGROUNDING) {
        appRecord->SetApplicationPendingState(ApplicationPendingState::READY);
    }

    TAG_LOGD(AAFwkTag::APPMGR, "application is backgrounded");
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
        eventInfo.bundleType = static_cast<int32_t>(applicationInfo->bundleType);
    }
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    eventInfo.processType = static_cast<int32_t>(appRecord->GetProcessType());
    AAFwk::EventReport::SendAppBackgroundEvent(AAFwk::EventName::APP_BACKGROUND, eventInfo);
}

void AppMgrServiceInner::ApplicationTerminated(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app record failed");
        return;
    }
    appRecord->ApplicationTerminated();
    // Maybe can't get in here
    if (appRecord->IsKeepAliveApp()) {
        return;
    }
    if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
        TAG_LOGD(AAFwkTag::APPMGR, "current state is not background");
        return;
    }

    KillRenderProcess(appRecord);
    KillChildProcess(appRecord);
    KillAttachedChildProcess(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false, false);
    appRunningManager_->RemoveAppRunningRecordById(recordId);
    RemoveAppFromRecentListById(recordId);
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);

    ApplicationTerminatedSendProcessEvent(appRecord);

    auto uid = appRecord->GetUid();
    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

int32_t AppMgrServiceInner::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "The appRunningManager_ is nullptr.");
        return ERR_NO_INIT;
    }

    int32_t result = VerifyRequestPermission();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return result;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ fail.");
        return ERR_NO_INIT;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return ERR_NO_INIT;
    }
    auto userId = GetUserIdByUid(uid);
    ApplicationInfo appInfo;
    HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetApplicationInfo");
    bool bundleMgrResult = bundleMgrHelper->GetApplicationInfo(bundleName,
        ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo);
    if (!bundleMgrResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get applicationInfo.");
        return ERR_INVALID_OPERATION;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "uid value is %{public}d", uid);
    result = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo);
    if (result != ERR_OK) {
        TAG_LOGI(AAFwkTag::APPMGR, "The process corresponding to the package name did not start.");
    }

    return result;
}

int32_t AppMgrServiceInner::KillApplication(const std::string &bundleName)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    if (CheckCallerIsAppGallery()) {
        return KillApplicationByBundleName(bundleName);
    }

    auto result = VerifyProcessPermission(bundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return result;
    }

    return KillApplicationByBundleName(bundleName);
}

int32_t AppMgrServiceInner::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    int32_t result = ERR_OK;
    if (!CheckCallerIsAppGallery()) {
        result = VerifyProcessPermission(bundleName);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
            return result;
        }
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return ERR_NO_INIT;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        TAG_LOGI(AAFwkTag::APPMGR, "not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "KillApplicationByUid");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "KillApplication failed for bundleName:%{public}s pid:%{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

void AppMgrServiceInner::SendProcessExitEventTask(
    const std::shared_ptr<AppRunningRecord> &appRecord, time_t exitTime, int32_t count)
{
    if (appRecord == nullptr) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }
    if (appRecord->GetPriorityObject() == nullptr) {
        HILOG_ERROR("Get priority object is nullptr.");
        return;
    }
    auto pid = appRecord->GetPriorityObject()->GetPid();
    auto exitResult = !ProcessExist(pid);
    constexpr int32_t EXIT_SUCESS = 0;
    constexpr int32_t EXIT_FAILED = -1;
    AAFwk::EventInfo eventInfo;
    eventInfo.time = exitTime;
    eventInfo.pid = pid;
    eventInfo.processName = appRecord->GetProcessName();
    eventInfo.extensionType = static_cast<int32_t>(appRecord->GetExtensionType());

    if (exitResult) {
        eventInfo.exitResult = EXIT_SUCESS;
        AAFwk::EventReport::SendProcessExitEvent(AAFwk::EventName::PROCESS_EXIT, eventInfo);
        TAG_LOGI(AAFwkTag::APPMGR, "time : %{public}" PRId64 ", exitResult : %{public}d, pid : %{public}d",
            eventInfo.time, eventInfo.exitResult, eventInfo.pid);
        return;
    }

    if (--count <= 0) {
        eventInfo.exitResult = EXIT_FAILED;
        AAFwk::EventReport::SendProcessExitEvent(AAFwk::EventName::PROCESS_EXIT, eventInfo);
        TAG_LOGI(AAFwkTag::APPMGR, "time : %{public}" PRId64 ", exitResult : %{public}d, pid : %{public}d",
            eventInfo.time, eventInfo.exitResult, eventInfo.pid);
        return;
    }

    auto sendEventTask = [inner = shared_from_this(), appRecord, exitTime, count] () {
        inner->SendProcessExitEventTask(appRecord, exitTime, count);
    };
    taskHandler_->SubmitTask(sendEventTask, PROCESS_EXIT_EVENT_TASK, KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
}

void AppMgrServiceInner::SendProcessExitEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    time_t currentTime;
    time(&currentTime);
    constexpr int32_t RETRY_COUNT = 5;
    SendProcessExitEventTask(appRecord, currentTime, RETRY_COUNT);
    return;
}

int32_t AppMgrServiceInner::KillApplicationSelf()
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    auto bundleName = appRecord->GetBundleName();
    return KillApplicationByBundleName(bundleName);
}

int32_t AppMgrServiceInner::KillApplicationByBundleName(const std::string &bundleName)
{
    int result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;

    if (!appRunningManager_->ProcessExitByBundleName(bundleName, pids)) {
        TAG_LOGE(AAFwkTag::APPMGR, "The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGD(AAFwkTag::APPMGR, "The remote process exited successfully ");
        NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "KillApplicationByBundleName");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "KillApplicationSelf is failed for bundleName:%{public}s, pid: %{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUserId(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    if (VerifyAccountPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES, userId) == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return ERR_NO_INIT;
    }

    return KillApplicationByUserIdLocked(bundleName, userId);
}

int32_t AppMgrServiceInner::KillApplicationByUserIdLocked(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return ERR_NO_INIT;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "userId value is %{public}d", userId);
    int uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId));
    TAG_LOGI(AAFwkTag::APPMGR, "uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The process corresponding to the package name did not start.");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter, "KillApplicationByUserIdLocked");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "KillApplication is fail bundleName: %{public}s pid: %{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

int32_t AppMgrServiceInner::ClearUpApplicationData(const std::string &bundleName,
    int32_t callerUid, pid_t callerPid, const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = GetUserIdByUid(callerUid);
    }
    TAG_LOGI(AAFwkTag::APPMGR, "userId:%{public}d", userId);
    return ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, newUserId);
}

int32_t AppMgrServiceInner::ClearUpApplicationDataBySelf(int32_t callerUid, pid_t callerPid, int32_t userId)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    auto callerbundleName = appRecord->GetBundleName();
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    int32_t newUserId = userId;
    if (userId == DEFAULT_INVAL_VALUE) {
        newUserId = GetUserIdByUid(callerUid);
    }
    return ClearUpApplicationDataByUserId(callerbundleName, callerUid, callerPid, newUserId, true);
}

int32_t AppMgrServiceInner::ClearUpApplicationDataByUserId(
    const std::string &bundleName, int32_t callerUid, pid_t callerPid, const int userId, bool isBySelf)
{
    if (callerPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerPid:%{public}d", callerPid);
        return ERR_INVALID_OPERATION;
    }
    if (callerUid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid callerUid:%{public}d", callerUid);
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    // request to clear user information permission.
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, 0);
    int32_t result = AccessToken::AccessTokenKit::ClearUserGrantedPermissionState(tokenId);
    if (result) {
        TAG_LOGE(AAFwkTag::APPMGR, "ClearUserGrantedPermissionState failed, ret:%{public}d", result);
        return ERR_PERMISSION_DENIED;
    }
    // 2.delete bundle side user data
    if (!IN_PROCESS_CALL(bundleMgrHelper->CleanBundleDataFiles(bundleName, userId))) {
        TAG_LOGE(AAFwkTag::APPMGR, "Delete bundle side user data is fail");
        return ERR_INVALID_OPERATION;
    }
    // 3.kill application
    // 4.revoke user rights
    result = isBySelf ? KillApplicationSelf() : KillApplicationByUserId(bundleName, userId);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Kill Application by bundle name is fail");
        return ERR_INVALID_OPERATION;
    }
    // 5.revoke uri permission rights
    auto ret = IN_PROCESS_CALL(AAFwk::UriPermissionManagerClient::GetInstance().RevokeAllUriPermissions(tokenId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Revoke all uri permissions is failed");
        return ret;
    }
    auto dataMgr = OHOS::DistributedKv::DistributedDataMgr();
    auto dataRet = dataMgr.ClearAppStorage(bundleName, userId, 0, tokenId);
    if (dataRet != 0) {
        TAG_LOGW(
            AAFwkTag::APPMGR, "Distributeddata clear app storage failed, bundleName:%{public}s", bundleName.c_str());
    }
    NotifyAppStatusByCallerUid(bundleName, userId, callerUid,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord->GetSpawned()) {
            continue;
        }
        if (isPerm) {
            GetRunningProcesses(appRecord, info);
        } else {
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetRunningProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (!appRecord->GetSpawned()) {
            continue;
        }
        int32_t userIdTemp = static_cast<int32_t>(appRecord->GetUid() / USER_SCALE);
        if (userIdTemp == userId) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetProcessRunningInformation(RunningProcessInfo &info)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    GetRunningProcess(appRecord, info);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (isPerm) {
            GetRenderProcesses(appRecord, info);
        } else {
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetRenderProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");

    bool isMemmgrCall = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        MEMMGR_PROC_NAME);
    if (!isMemmgrCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not %{public}s. %{public}s", MEMMGR_PROC_NAME.c_str(), __func__);
        return ERR_INVALID_VALUE;
    }
    if (!(level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_CRITICAL ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_LOW)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Level value error!");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }

    return appRunningManager_->NotifyMemoryLevel(level);
}

int32_t AppMgrServiceInner::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start");

    bool isMemmgrCall = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        MEMMGR_PROC_NAME);
    if (!isMemmgrCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not %{public}s. %{public}s", MEMMGR_PROC_NAME.c_str(), __func__);
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }

    return appRunningManager_->NotifyProcMemoryLevel(procLevelMap);
}

int32_t AppMgrServiceInner::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA %{public}s", __func__);
        return ERR_INVALID_VALUE;
    }
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid is illegal!");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->DumpHeapMemory(pid, mallocInfo);
}

int32_t AppMgrServiceInner::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA %{public}s", __func__);
        return ERR_INVALID_VALUE;
    }
    if (info.pid == 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid is illegal!");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->DumpJsHeapMemory(info);
}

void AppMgrServiceInner::GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RunningProcessInfo> &info)
{
    RunningProcessInfo runningProcessInfo;
    GetRunningProcess(appRecord, runningProcessInfo);
    info.emplace_back(runningProcessInfo);
}

void AppMgrServiceInner::GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningProcessInfo &info)
{
    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPriorityObject()->GetPid();
    info.uid_ = appRecord->GetUid();
    info.state_ = static_cast<AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isFocused = appRecord->GetFocusFlag();
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    appRecord->GetBundleNames(info.bundleNames);
    info.processType_ = appRecord->GetProcessType();
    info.extensionType_ = appRecord->GetExtensionType();
    if (appRecord->GetUserTestInfo() != nullptr && system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        info.isTestMode = true;
    }
}

void AppMgrServiceInner::GetRenderProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RenderProcessInfo> &info)
{
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (renderRecordMap.empty()) {
        return;
    }
    for (auto iter : renderRecordMap) {
        auto renderRecord = iter.second;
        if (renderRecord != nullptr) {
            RenderProcessInfo renderProcessInfo;
            renderProcessInfo.bundleName_ = renderRecord->GetHostBundleName();
            renderProcessInfo.processName_ = renderRecord->GetProcessName();
            renderProcessInfo.pid_ = renderRecord->GetPid();
            renderProcessInfo.uid_ = renderRecord->GetUid();
            renderProcessInfo.hostUid_ = renderRecord->GetHostUid();
            info.emplace_back(renderProcessInfo);
        }
    }
}

int32_t AppMgrServiceInner::KillProcessByPid(const pid_t pid, const std::string& reason)
{
    if (!ProcessExist(pid)) {
        TAG_LOGI(AAFwkTag::APPMGR, "KillProcessByPid, process not exists, pid: %{public}d", pid);
        return AAFwk::ERR_KILL_PROCESS_NOT_EXIST;
    }
    std::string killReason = KILL_PROCESS_REASON_PREFIX + reason;
    int32_t ret = -1;
    if (pid > 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
    }
    AAFwk::EventInfo eventInfo;
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        return ret;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app informations");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (ret >= 0) {
        int64_t killTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count();
        killedPorcessMap_.emplace(killTime, appRecord->GetProcessName());
    }
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_PID, std::to_string(eventInfo.pid),
        EVENT_KEY_PROCESS_NAME, eventInfo.processName,
        EVENT_KEY_MESSAGE, killReason);
    return ret;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        if (CheckAllProcessExist(pids)) {
            return true;
        }
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

bool AppMgrServiceInner::GetAllPids(std::list<pid_t> &pids)
{
    for (const auto &appTaskInfo : appProcessManager_->GetRecentAppList()) {
        if (appTaskInfo) {
            auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
            if (appRecord) {
                pids.push_back(appTaskInfo->GetPid());
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }
    return (pids.empty() ? false : true);
}

bool AppMgrServiceInner::ProcessExist(pid_t pid)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/status", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) == 0) {
        return true;
    }
    return false;
}

bool AppMgrServiceInner::CheckAllProcessExist(std::list<pid_t> &pids)
{
    for (auto iter = pids.begin(); iter != pids.end();) {
        if (!ProcessExist(*iter)) {
            iter = pids.erase(iter);
        } else {
            iter++;
        }
    }
    if (pids.empty()) {
        return true;
    }
    return false;
}

int64_t AppMgrServiceInner::SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByPid(const pid_t pid) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return nullptr;
    }
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::CreateAppRunningRecord(sptr<IRemoteObject> token,
    sptr<IRemoteObject> preToken, std::shared_ptr<ApplicationInfo> appInfo,
    std::shared_ptr<AbilityInfo> abilityInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return nullptr;
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "get app record failed");
        return nullptr;
    }

    appRecord->SetProcessAndExtensionType(abilityInfo);
    bool isKeepAlive = bundleInfo.isKeepAlive && bundleInfo.singleton;
    appRecord->SetKeepAliveAppState(isKeepAlive, false);
    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want, abilityRecordId);
    if (want) {
        appRecord->SetDebugApp(want->GetBoolParam(DEBUG_APP, false));
        appRecord->SetNativeDebug(want->GetBoolParam("nativeDebug", false));
        if (want->GetBoolParam(COLD_START, false)) {
            appRecord->SetDebugApp(true);
        }
        appRecord->SetPerfCmd(want->GetStringParam(PERF_CMD));
        appRecord->SetAppIndex(want->GetIntParam(DLP_PARAMS_INDEX, 0));
        appRecord->SetSecurityFlag(want->GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));
        appRecord->SetRequestProcCode(want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
        appRecord->SetCallerPid(want->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
        appRecord->SetCallerUid(want->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
        appRecord->SetCallerTokenId(want->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
        appRecord->SetAssignTokenId(want->GetIntParam("specifyTokenId", 0));
        appRecord->SetNativeStart(want->GetBoolParam("native", false));
    }

    if (preToken) {
        auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
        if (abilityRecord) {
            abilityRecord->SetPreToken(preToken);
        }
    }

    return appRecord;
}

void AppMgrServiceInner::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Terminate ability come.");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrServiceInner::TerminateAbility token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrServiceInner::TerminateAbility app is not exist!");
        return;
    }

    if (appRunningManager_) {
        std::shared_ptr<AppMgrServiceInner> appMgrServiceInner = shared_from_this();
        appRunningManager_->TerminateAbility(token, clearMissionFlag, appMgrServiceInner);
    }
}

void AppMgrServiceInner::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "state %{public}d.", static_cast<int32_t>(state));
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null!");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not find ability record!");
        return;
    }
    if (state == abilityRecord->GetState()) {
        TAG_LOGE(AAFwkTag::APPMGR, "current state is already, no need update!");
        return;
    }
    if (abilityRecord->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability info nullptr!");
        return;
    }
    auto type = abilityRecord->GetAbilityInfo()->type;
    if (type == AppExecFwk::AbilityType::SERVICE &&
        (state == AbilityState::ABILITY_STATE_CREATE ||
        state == AbilityState::ABILITY_STATE_TERMINATED ||
        state == AbilityState::ABILITY_STATE_CONNECTED ||
        state == AbilityState::ABILITY_STATE_DISCONNECTED)) {
        TAG_LOGI(
            AAFwkTag::APPMGR, "StateChangedNotifyObserver service type, state:%{public}d", static_cast<int32_t>(state));
        appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), true, false);
        return;
    }
    if (state > AbilityState::ABILITY_STATE_BACKGROUND || state < AbilityState::ABILITY_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::APPMGR, "state is not foreground or background!");
        return;
    }

    appRecord->SetUpdateStateFromService(true);
    appRecord->UpdateAbilityState(token, state);
}

void AppMgrServiceInner::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not find ability record!");
        return;
    }
    appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), false, false);
}

void AppMgrServiceInner::OnStop()
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager nullptr!");
        return;
    }

    appRunningManager_->ClearAppRunningRecordMap();
    CloseAppSpawnConnection();
}

ErrCode AppMgrServiceInner::OpenAppSpawnConnection()
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return ERR_INVALID_VALUE;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->OpenConnection();
    }
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppMgrServiceInner::CloseAppSpawnConnection() const
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        remoteClientManager_->GetSpawnClient()->CloseConnection();
    }
}

SpawnConnectionState AppMgrServiceInner::QueryAppSpawnConnectionState() const
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return SpawnConnectionState::STATE_NOT_CONNECT;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->QueryConnectionState();
    }
    return SpawnConnectionState::STATE_NOT_CONNECT;
}

void AppMgrServiceInner::SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return;
    }

    remoteClientManager_->SetSpawnClient(std::move(spawnClient));
}

void AppMgrServiceInner::SetBundleManagerHelper(const std::shared_ptr<BundleMgrHelper> &bundleMgrHelper)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ is nullptr.");
        return;
    }

    remoteClientManager_->SetBundleManagerHelper(bundleMgrHelper);
}

void AppMgrServiceInner::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Not abilityMgr call.", __func__);
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (callback != nullptr) {
        std::lock_guard lock(appStateCallbacksLock_);
        appStateCallbacks_.push_back(callback);
    }
}

void AppMgrServiceInner::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility,       // 0:false,1:true
    const int32_t perceptibility,   // 0:false,1:true
    const int32_t connectionState)  // 0:false,1:true
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app record is not exist for ability token");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability record is not exist for ability previous token");
        return;
    }
    if (preToken) {
        abilityRecord->SetPreToken(preToken);
    }
    abilityRecord->SetVisibility(visibility);
    abilityRecord->SetPerceptibility(perceptibility);
    abilityRecord->SetConnectionState(connectionState);
}

void AppMgrServiceInner::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app record is not exist for ability token");
        return;
    }

    // before exec ScheduleProcessSecurityExit return
    // The resident process won't let him die
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        std::list<pid_t> pids;
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillisecond())) {
            int32_t result = KillProcessByPid(pid, "KillProcessByAbilityToken");
            if (result < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "KillProcessByAbilityToken kill process is fail");
                return;
            }
        }
    }
}

void AppMgrServiceInner::KillProcessesByUserId(int32_t userId)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (!appRunningManager_->GetPidsByUserId(userId, pids)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The process corresponding to the userId did not start");
        return;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The remote process exited successfully ");
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto result = KillProcessByPid(*iter, "KillProcessesByUserId");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "KillProcessByPid is failed. pid: %{public}d", *iter);
            return;
        }
    }
}

void AppMgrServiceInner::KillProcessesByPids(std::vector<int32_t> &pids)
{
    for (const auto& pid: pids) {
        auto appRecord = GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
            return;
        }
        auto result = KillProcessByPid(pid, "KillProcessesByPids");
        if (result < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "KillProcessByPid is failed. pid: %{public}d", pid);
        }
    }
}

void AppMgrServiceInner::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord is nullptr");
        return;
    }
    auto pid = appRecord->GetPriorityObject()->GetPid();
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid");
        return;
    }
    auto callRecord = GetAppRunningRecordByAbilityToken(callerToken);
    if (callRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callRecord is nullptr");
        auto result = KillProcessByPid(pid, "AttachPidToParent");
        if (result < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "KillProcessByPid is failed. pid: %{public}d", pid);
        }
        return;
    }
    appRecord->SetParentAppRecord(callRecord);
    callRecord->AddChildAppRecord(pid, appRecord);
}

void AppMgrServiceInner::StartAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<AppRunningRecord> appRecord,
    const HapModuleInfo &hapModuleInfo, std::shared_ptr<AAFwk::Want> want, int32_t abilityRecordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "start ability");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is null");
        return;
    }

    if (want) {
        want->SetParam(DLP_PARAMS_SECURITY_FLAG, appRecord->GetSecurityFlag());

        auto isDebugApp = want->GetBoolParam(DEBUG_APP, false);
        if (isDebugApp && !appRecord->IsDebugApp()) {
            ProcessAppDebug(appRecord, isDebugApp);
        }
    }

    auto ability = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityInfo->launchMode == LaunchMode::SINGLETON && ability != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "same ability info in singleton launch mode, will not add ability");
        return;
    }

    if (ability && preToken) {
        TAG_LOGE(AAFwkTag::APPMGR, "Ability is already started");
        ability->SetPreToken(preToken);
        return;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want, abilityRecordId);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    if (!moduleRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "add moduleRecord failed");
        return;
    }

    ability = moduleRecord->GetAbilityRunningRecordByToken(token);
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "add ability failed");
        return;
    }

    if (preToken != nullptr) {
        ability->SetPreToken(preToken);
    }

    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_CREATE) {
        TAG_LOGE(AAFwkTag::APPMGR, "in create state, don't launch ability");
        return;
    }
    appRecord->LaunchAbility(ability);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return nullptr;
    }

    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &token) const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr.");
        return nullptr;
    }
    return appRunningManager_->GetTerminatingAppRunningRecord(token);
}

void AppMgrServiceInner::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Terminate ability come.");
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "Terminate ability error, token is null!");
        return;
    }

    auto appRecord = appRunningManager_->GetTerminatingAppRunningRecord(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Terminate ability error, appRecord is not exist!");
        return;
    }

    appRecord->AbilityTerminated(token);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppRecordId(const int32_t recordId) const
{
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager is nullptr");
        return nullptr;
    }
    const auto&& appRunningRecordMap = appRunningManager_->GetAppRunningRecordMap();
    const auto& iter = appRunningRecordMap.find(recordId);
    return iter != appRunningRecordMap.end() ? iter->second : nullptr;
}

void AppMgrServiceInner::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state,
    bool needNotifyApp,
    bool isFromWindowFocusChanged)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStateChanged come, app record is null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStateChanged begin, bundleName is %{public}s, state:%{public}d",
        appRecord->GetBundleName().c_str(), static_cast<int32_t>(state));
    {
        std::lock_guard lock(appStateCallbacksLock_);
        for (const auto &callback : appStateCallbacks_) {
            if (callback != nullptr) {
                callback->OnAppStateChanged(WrapAppProcessData(appRecord, state));
            }
        }
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStateChanged(
        appRecord, state, needNotifyApp, isFromWindowFocusChanged);
}

void AppMgrServiceInner::OnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStarted come, app record is null");
        return;
    }

    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStarted come, appRecord's priorityobject is null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStarted begin, bundleName is %{public}s, pid:%{public}d",
        appRecord->GetBundleName().c_str(), appRecord->GetPriorityObject()->GetPid());

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStarted(appRecord);
}


void AppMgrServiceInner::OnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStopped come, app record is null");
        return;
    }

    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnAppStarted come, appRecord's priorityObject is null");
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "OnAppStopped begin, bundleName is %{public}s, pid:%{public}d",
        appRecord->GetBundleName().c_str(), appRecord->GetPriorityObject()->GetPid());

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStopped(appRecord);
}

AppProcessData AppMgrServiceInner::WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppProcessData processData;
    auto appInfoList = appRecord->GetAppInfoList();
    for (const auto &list : appInfoList) {
        AppData data;
        data.appName = list->name;
        data.uid = list->uid;
        processData.appDatas.push_back(data);
    }
    processData.processName = appRecord->GetProcessName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.appState = state;
    processData.isFocused = appRecord->GetFocusFlag();
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty()) {
        for (auto iter : renderRecordMap) {
            auto renderRecord = iter.second;
            if (renderRecord != nullptr) {
                processData.renderPids.emplace_back(renderRecord->GetPid());
            }
        }
    }
    return processData;
}

void AppMgrServiceInner::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPMGR, "ability is null");
        return;
    }
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityRequestDone(ability->GetToken(), state);
        }
    }
}

void AppMgrServiceInner::StateChangedNotifyObserver(
    const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged)
{
    DelayedSingleton<AppStateObserverManager>::GetInstance()->StateChangedNotifyObserver(
        abilityStateData, isAbility, isFromWindowFocusChanged);
}

int32_t AppMgrServiceInner::StartPerfProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::string& perfCmd, const std::string& debugCmd, bool isSandboxApp) const
{
    if (!remoteClientManager_->GetSpawnClient() || !appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient or appRecord is null");
        return ERR_INVALID_OPERATION;
    }
    if (perfCmd.empty() && debugCmd.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "perfCmd is empty");
        return ERR_INVALID_OPERATION;
    }

    auto&& startMsg = appRecord->GetStartMsg();
    startMsg.code = static_cast<int32_t>(AppSpawn::ClientSocket::AppOperateCode::SPAWN_NATIVE_PROCESS);
    if (!isSandboxApp) {
        TAG_LOGD(AAFwkTag::APPMGR, "debuggablePipe sandbox: false.");
        startMsg.flags |= (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::NO_SANDBOX);
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe sandbox: true");
    }
    if (!perfCmd.empty()) {
        startMsg.renderParam = perfCmd;
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe perfCmd:%{public}s", perfCmd.c_str());
    } else {
        startMsg.renderParam = debugCmd;
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe debugCmd:%{public}s", debugCmd.c_str());
    }
    pid_t pid = 0;
    auto errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to spawn new native process, errCode %{public}08x", errCode);
        return errCode;
    }
    return ERR_OK;
}

void AppMgrServiceInner::SetOverlayInfo(const std::string &bundleName,
                                        const int32_t userId,
                                        AppSpawnStartMsg &startMsg)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ is nullptr.");
        return;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return;
    }
    auto overlayMgrProxy = bundleMgrHelper->GetOverlayManagerProxy();
    if (overlayMgrProxy !=  nullptr) {
        std::vector<OverlayModuleInfo> overlayModuleInfo;
        TAG_LOGD(AAFwkTag::APPMGR, "Check overlay app begin.");
        HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetOverlayModuleInfoForTarget");
        auto targetRet = IN_PROCESS_CALL(overlayMgrProxy->GetOverlayModuleInfoForTarget(
            bundleName, "", overlayModuleInfo, userId));
        if (targetRet == ERR_OK && overlayModuleInfo.size() != 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "Start an overlay app process.");
            startMsg.flags = startMsg.flags | APP_OVERLAY_FLAG;
            std::string overlayInfoPaths;
            for (auto it : overlayModuleInfo) {
                overlayInfoPaths += (it.hapPath + "|");
            }
            startMsg.overlayInfo = overlayInfoPaths;
        }
    }
}

void AppMgrServiceInner::SetAppEnvInfo(const BundleInfo &bundleInfo, AppSpawnStartMsg& startMsg)
{
    if (bundleInfo.applicationInfo.tsanEnabled) {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(1));
    } else {
        startMsg.appEnv.emplace(TSAN_FLAG_NAME, std::to_string(0));
    }

    if (!bundleInfo.applicationInfo.appEnvironments.empty()) {
        for (const auto& appEnvironment : bundleInfo.applicationInfo.appEnvironments) {
            startMsg.appEnv.emplace(appEnvironment.name, appEnvironment.value);
        }
    }
}

void AppMgrServiceInner::StartProcessVerifyPermission(const BundleInfo &bundleInfo, bool &hasAccessBundleDirReq,
                                                      uint8_t &setAllowInternet, uint8_t &allowInternet,
                                                      std::vector<int32_t> &gids, std::set<std::string> &permissions)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    hasAccessBundleDirReq = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_ACCESS_BUNDLE_DIR == reqPermission) {
                return true;
            }
            return false;
        });

    auto token = bundleInfo.applicationInfo.accessTokenId;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "AccessTokenKit::VerifyAccessToken");
        int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_INTERNET, false);
        if (result != Security::AccessToken::PERMISSION_GRANTED) {
            setAllowInternet = 1;
            allowInternet = 0;
    #ifdef APP_MGR_SERVICE_APPMS
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 0);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
        } else {
            auto ret = OHOS::NetManagerStandard::NetConnClient::GetInstance().SetInternetPermission(bundleInfo.uid, 1);
            TAG_LOGD(AAFwkTag::APPMGR, "SetInternetPermission, ret = %{public}d", ret);
            gids.push_back(NETSYS_SOCKET_GROUPID);
    #endif
        }

        result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_MANAGE_VPN, false);
        if (result == Security::AccessToken::PERMISSION_GRANTED) {
            gids.push_back(BLUETOOTH_GROUPID);
        }

        if (hasAccessBundleDirReq) {
            int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token,
                PERMISSION_ACCESS_BUNDLE_DIR, false);
            if (result != Security::AccessToken::PERMISSION_GRANTED) {
                TAG_LOGE(AAFwkTag::APPMGR, "StartProcess PERMISSION_ACCESS_BUNDLE_DIR NOT GRANTED");
                hasAccessBundleDirReq = false;
            }
        }
    }

    std::set<std::string> mountPermissionList = AppSpawn::AppspawnMountPermission::GetMountPermissionList();
    for (std::string permission : mountPermissionList) {
        if (Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, permission, false) ==
            Security::AccessToken::PERMISSION_GRANTED) {
            permissions.insert(permission);
        }
    }
}

void AppMgrServiceInner::StartProcess(const std::string &appName, const std::string &processName, uint32_t startFlags,
    std::shared_ptr<AppRunningRecord> appRecord, const int uid, const BundleInfo &bundleInfo,
    const std::string &bundleName, const int32_t bundleIndex, bool appExistFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is null");
        return;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient is null");
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get bundle manager helper fail.");
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }

    auto userId = GetUserIdByUid(uid);
    HspList hspList;
    ErrCode ret = bundleMgrHelper->GetBaseSharedBundleInfos(bundleName, hspList,
        AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBaseSharedBundleInfos failed: %{public}d", ret);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }

    DataGroupInfoList dataGroupInfoList;
    bool result = bundleMgrHelper->QueryDataGroupInfos(bundleName, userId, dataGroupInfoList);
    if (!result || dataGroupInfoList.empty()) {
        TAG_LOGD(AAFwkTag::APPMGR, "the bundle has no groupInfos");
    }

    bool hasAccessBundleDirReq;
    uint8_t setAllowInternet = 0;
    uint8_t allowInternet = 1;
    std::vector<int32_t> gids;
    std::set<std::string> permissions;
    StartProcessVerifyPermission(bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids,
                                 permissions);

    AppSpawnStartMsg startMsg;
    startMsg.uid = bundleInfo.uid;
    startMsg.gid = bundleInfo.gid;
    startMsg.gids = gids;
    startMsg.accessTokenId = bundleInfo.applicationInfo.accessTokenId;
    startMsg.apl = bundleInfo.applicationInfo.appPrivilegeLevel;
    startMsg.bundleName = bundleName;
    startMsg.renderParam = RENDER_PARAM;
    startMsg.flags = startFlags;
    startMsg.bundleIndex = bundleIndex;
    startMsg.setAllowInternet = setAllowInternet;
    startMsg.allowInternet = allowInternet;
    startMsg.hspList = hspList;
    startMsg.dataGroupInfoList = dataGroupInfoList;
    startMsg.hapFlags = bundleInfo.isPreInstallApp ? 1 : 0;

    startMsg.mountPermissionFlags = AppSpawn::AppspawnMountPermission::GenPermissionCode(permissions);
    startMsg.ownerId = bundleInfo.signatureInfo.appIdentifier;
    if (hasAccessBundleDirReq) {
        startMsg.flags = startMsg.flags | APP_ACCESS_BUNDLE_DIR;
    }

    if (VerifyPermission(bundleInfo, PERMISSION_GET_BUNDLE_RESOURCES)) {
        startMsg.flags = startMsg.flags | GET_BUNDLE_RESOURCES_FLAG;
    }

    SetOverlayInfo(bundleName, userId, startMsg);
    SetAppEnvInfo(bundleInfo, startMsg);

    TAG_LOGI(AAFwkTag::APPMGR, "apl is %{public}s, bundleName is %{public}s, startFlags is %{public}d.",
        startMsg.apl.c_str(), bundleName.c_str(), startFlags);

    bool bundleMgrResult = IN_PROCESS_CALL(bundleMgrHelper->GetBundleGidsByUid(bundleName, uid, startMsg.gids));
    if (!bundleMgrResult) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleGids is fail.");
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }

    startMsg.procName = processName;
    startMsg.accessTokenIdEx = bundleInfo.applicationInfo.accessTokenIdEx;

    SetProcessJITState(appRecord);
    PerfProfile::GetInstance().SetAppForkStartTime(GetTickCount());
    pid_t pid = 0;
    TAG_LOGD(AAFwkTag::APPMGR, "bundleName: %{public}s.", bundleName.c_str());
    ErrCode errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to spawn new app process, errCode %{public}08x", errCode);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "Start process success, pid: %{public}d, processName: %{public}s.",
        pid, processName.c_str());
    SetRunningSharedBundleList(bundleName, hspList);
    appRecord->GetPriorityObject()->SetPid(pid);
    appRecord->SetUid(startMsg.uid);
    appRecord->SetStartMsg(startMsg);
    appRecord->SetAppMgrServiceInner(weak_from_this());
    appRecord->SetSpawned();
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, false, false);
    AddAppToRecentList(appName, appRecord->GetProcessName(), pid, appRecord->GetRecordId());
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessCreated(appRecord);
    if (!appExistFlag) {
        OnAppStarted(appRecord);
    }
    PerfProfile::GetInstance().SetAppForkEndTime(GetTickCount());
    SendProcessStartEvent(appRecord);
    ProcessAppDebug(appRecord, appRecord->IsDebugApp());
}

void AppMgrServiceInner::SetProcessJITState(const std::shared_ptr<AppRunningRecord> appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "SetProcessJITState called.");
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr.");
        return;
    }
    if (!securityModeManager_) {
        HILOG_ERROR("securityModeManager_ is nullptr.");
        appRecord->SetJITEnabled(true);
        return;
    }
    appRecord->SetJITEnabled(securityModeManager_->IsJITEnabled());
}

AppDebugInfo AppMgrServiceInner::MakeAppDebugInfo(
    const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart)
{
    AppDebugInfo info;
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return info;
    }

    info.bundleName = appRecord->GetBundleName();
    auto priorityObject = appRecord->GetPriorityObject();
    if (priorityObject) {
        info.pid = priorityObject->GetPid();
    }
    info.uid = appRecord->GetUid();
    info.isDebugStart = isDebugStart;
    return info;
}

void AppMgrServiceInner::ProcessAppDebug(const std::shared_ptr<AppRunningRecord> &appRecord, const bool &isDebugStart)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (appRecord == nullptr || appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord or appDebugManager_ is nullptr.");
        return;
    }

    auto startDebug = [this, appRecord](const bool &isDebugStart) {
        std::vector<AppDebugInfo> debugInfos;
        debugInfos.emplace_back(MakeAppDebugInfo(appRecord, isDebugStart));
        appDebugManager_->StartDebug(debugInfos);
    };

    if (isDebugStart && !appRecord->IsDebugApp()) {
        appRecord->SetDebugApp(true);
        startDebug(true);
        return;
    }

    if (appRecord->IsDebugApp()) {
        startDebug(true);
        return;
    }

    auto bundleName = appRecord->GetBundleName();
    if (appDebugManager_->IsAttachDebug(bundleName)) {
        appRecord->SetAttachDebug(true);
        startDebug(false);
    }
}

void AppMgrServiceInner::UpDateStartupType(
    const std::shared_ptr<AbilityInfo> &info, int32_t &abilityType, int32_t &extensionType)
{
    if (info == nullptr) {
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s, abilityName:%{public}s", info->bundleName.c_str(),
        info->name.c_str());
    abilityType = static_cast<int32_t>(info->type);
    if (info->type != AbilityType::EXTENSION) {
        return;
    }

    extensionType = static_cast<int32_t>(info->extensionAbilityType);
}

bool AppMgrServiceInner::SendProcessStartEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return false;
    }

    AAFwk::EventInfo eventInfo;
    time_t currentTime;
    time(&currentTime);
    eventInfo.time = currentTime;
    eventInfo.callerUid = appRecord->GetCallerUid() == -1 ? IPCSkeleton::GetCallingUid() : appRecord->GetCallerUid();
    if (!appRecord->GetAbilities().empty()) {
        auto abilityinfo = appRecord->GetAbilities().begin()->second->GetAbilityInfo();
        UpDateStartupType(abilityinfo, eventInfo.abilityType, eventInfo.extensionType);
    } else {
        TAG_LOGI(AAFwkTag::APPMGR, "Abilities nullptr!");
    }

    eventInfo.callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerAppRecord = GetAppRunningRecordByPid(eventInfo.callerPid);
    if (callerAppRecord == nullptr) {
        Security::AccessToken::NativeTokenInfo nativeTokenInfo = {};
        auto token = appRecord->GetCallerTokenId() == -1 ?
            static_cast<int>(IPCSkeleton::GetCallingTokenID()) : appRecord->GetCallerTokenId();
        Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(token, nativeTokenInfo);
        eventInfo.callerBundleName = "";
        eventInfo.callerProcessName = nativeTokenInfo.processName;
    } else {
        if (callerAppRecord->GetBundleName().empty()) {
            eventInfo.callerBundleName = callerAppRecord->GetName();
        } else {
            eventInfo.callerBundleName = callerAppRecord->GetBundleName();
        }
        eventInfo.callerProcessName = callerAppRecord->GetProcessName();
    }
    if (!appRecord->GetBundleName().empty()) {
        eventInfo.bundleName = appRecord->GetBundleName();
    }
    eventInfo.processName = appRecord->GetProcessName();
    if (appRecord->GetPriorityObject() == nullptr) {
        HILOG_ERROR("appRecord's priorityObject is null");
    } else {
        eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    }
    AAFwk::EventReport::SendProcessStartEvent(AAFwk::EventName::PROCESS_START, eventInfo);
    TAG_LOGD(AAFwkTag::APPMGR, "%{public}s. time : %{public}" PRId64 ", abilityType : %{public}d, bundle : %{public}s,\
        uid : %{public}d, process : %{public}s",
        __func__, eventInfo.time, eventInfo.abilityType, eventInfo.callerBundleName.c_str(), eventInfo.callerUid,
        eventInfo.callerProcessName.c_str());
    SendReStartProcessEvent(eventInfo, appRecord);

    return true;
}

void AppMgrServiceInner::SendAppStartupTypeEvent(const std::shared_ptr<AppRunningRecord> &appRecord,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const AppStartType startType)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr, can not get app information");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo is nullptr, can not get ability information");
    } else {
        eventInfo.abilityName = abilityInfo->name;
    }
    if (appRecord->GetPriorityObject() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord's priorityObject is null");
    } else {
        eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    }
    eventInfo.startType = static_cast<int32_t>(startType);
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_STARTUP_TYPE, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::RemoveAppFromRecentList(const std::string &appName, const std::string &processName)
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    auto appTaskInfo = appProcessManager_->GetAppTaskInfoByProcessName(appName, processName);
    if (!appTaskInfo) {
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
    if (!appRecord) {
        appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
        return;
    }

    // Do not delete resident processes, before exec ScheduleProcessSecurityExit
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    startTime = SystemTimeMillisecond();
    pids.push_back(appTaskInfo->GetPid());
    appRecord->ScheduleProcessSecurityExit();
    if (!WaitForRemoteProcessExit(pids, startTime)) {
        int32_t result = KillProcessByPid(appTaskInfo->GetPid(), "RemoveAppFromRecentList");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "RemoveAppFromRecentList kill process is fail");
            return;
        }
    }
    appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
}

const std::list<const std::shared_ptr<AppTaskInfo>> &AppMgrServiceInner::GetRecentAppList() const
{
    return appProcessManager_->GetRecentAppList();
}

void AppMgrServiceInner::ClearRecentAppList()
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    if (GetAllPids(pids)) {
        return;
    }

    startTime = SystemTimeMillisecond();
    if (WaitForRemoteProcessExit(pids, startTime)) {
        appProcessManager_->ClearRecentAppList();
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        int32_t result = KillProcessByPid(*iter, "ClearRecentAppList");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "ClearRecentAppList kill process is fail");
            return;
        }
    }
    appProcessManager_->ClearRecentAppList();
}

void AppMgrServiceInner::OnRemoteDied(const wptr<IRemoteObject> &remote, bool isRenderProcess, bool isChildProcess)
{
    TAG_LOGD(AAFwkTag::APPMGR, "On remote died.");
    if (isRenderProcess) {
        OnRenderRemoteDied(remote);
        return;
    }
    if (isChildProcess) {
        OnChildProcessRemoteDied(remote);
        return;
    }

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard lock(exceptionLock_);
        appRecord = appRunningManager_->OnRemoteDied(remote);
    }
    if (appRecord == nullptr) {
        TAG_LOGI(AAFwkTag::APPMGR, "app record is not exist.");
        return;
    }

    ClearData(appRecord);
}

void AppMgrServiceInner::ClearAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord, bool containsApp)
{
    if (!appRecord) {
        return;
    }

    if (containsApp) {
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    }

    FinishUserTestLocked("App died", -1, appRecord);
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_REMOTE_DIED);

    for (const auto &item : appRecord->GetAbilities()) {
        const auto &abilityRecord = item.second;
        appRecord->StateChangedNotifyObserver(abilityRecord,
            static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true, false);
    }
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);

    // kill render if exist.
    KillRenderProcess(appRecord);
    KillChildProcess(appRecord);
    KillAttachedChildProcess(appRecord);

    SendProcessExitEvent(appRecord);

    if (!appRunningManager_->CheckAppRunningRecordIsExistByBundleName(appRecord->GetBundleName())) {
        OnAppStopped(appRecord);
    }

    if (appDebugManager_ != nullptr) {
        auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
        appDebugManager_->RemoveAppDebugInfo(info);
    }

    ClearAppRunningDataForKeepAlive(appRecord);

    auto uid = appRecord->GetUid();
    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

void AppMgrServiceInner::PushAppFront(const int32_t recordId)
{
    appProcessManager_->PushAppFront(recordId);
}

void AppMgrServiceInner::RemoveAppFromRecentListById(const int32_t recordId)
{
    appProcessManager_->RemoveAppFromRecentListById(recordId);
}

void AppMgrServiceInner::AddAppToRecentList(
    const std::string &appName, const std::string &processName, const pid_t pid, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    appProcessManager_->AddAppToRecentList(appName, processName, pid, recordId);
}

const std::shared_ptr<AppTaskInfo> AppMgrServiceInner::GetAppTaskInfoById(const int32_t recordId) const
{
    return appProcessManager_->GetAppTaskInfoById(recordId);
}

void AppMgrServiceInner::HandleTimeOut(const AAFwk::EventWrap &event)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager is nullptr");
        return;
    }

    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "HandleTimeOut, Hook_mode: no handle time out");
        return;
    }

    switch (event.GetEventId()) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            appRunningManager_->HandleTerminateTimeOut(event.GetParam());
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), event.GetParam());
            HandleTerminateApplicationTimeOut(event.GetParam());
            break;
        case AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), event.GetParam());
            HandleStartSpecifiedProcessTimeout(event.GetParam());
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), event.GetParam());
            HandleAddAbilityStageTimeOut(event.GetParam());
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            SendHiSysEvent(event.GetEventId(), event.GetParam());
            HandleStartSpecifiedAbilityTimeOut(event.GetParam());
            break;
        default:
            break;
    }
}
void AppMgrServiceInner::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->HandleAbilityAttachTimeOut(token);
}

void AppMgrServiceInner::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->PrepareTerminate(token);
}

void AppMgrServiceInner::HandleTerminateApplicationTimeOut(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    TerminateApplication(appRecord);
}

void AppMgrServiceInner::TerminateApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED_TIMEOUT);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false, false);
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        auto timeoutTask = [appRecord, pid, innerService = shared_from_this()]() {
            TAG_LOGI(AAFwkTag::APPMGR, "KillProcessByPid %{public}d", pid);
            int32_t result = innerService->KillProcessByPid(pid, "TerminateApplication");
            innerService->SendProcessExitEvent(appRecord);
            if (result < 0) {
                TAG_LOGE(AAFwkTag::APPMGR, "KillProcessByPid kill process is fail");
                return;
            }
        };
        if (!taskHandler_) {
            TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ is nullptr");
            return;
        }
        taskHandler_->SubmitTask(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
    }
    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    if (!appRunningManager_->CheckAppRunningRecordIsExistByBundleName(appRecord->GetBundleName())) {
        OnAppStopped(appRecord);
    }

    if (appDebugManager_) {
        auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
        appDebugManager_->RemoveAppDebugInfo(info);
    }

    auto uid = appRecord->GetUid();
    NotifyAppRunningStatusEvent(appRecord->GetBundleName(), uid, AbilityRuntime::RunningStatus::APP_RUNNING_STOP);
}

void AppMgrServiceInner::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    if (appRecord->IsNewProcessRequest() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestTimeoutResponse(appRecord->GetNewProcessRequestWant());
    }

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByToken(token, info);
}

void AppMgrServiceInner::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByPid(pid, info);
}

void AppMgrServiceInner::SetAbilityForegroundingFlagToAppRecord(const pid_t pid) const
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return;
    }

    appRunningManager_->SetAbilityForegroundingFlagToAppRecord(pid);
}

bool AppMgrServiceInner::CheckGetRunningInfoPermission() const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return false;
    }

    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return false;
    }

    return true;
}

void AppMgrServiceInner::LoadResidentProcess(const std::vector<AppExecFwk::BundleInfo> &infos)
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s called", __func__);

    TAG_LOGI(AAFwkTag::APPMGR, "bundle info size: [%{public}zu]", infos.size());
    StartResidentProcess(infos, -1, true);
}

void AppMgrServiceInner::StartResidentProcess(const std::vector<BundleInfo> &infos, int restartCount,
    bool isEmptyKeepAliveApp)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start resident process");
    if (infos.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "infos is empty!");
        return;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    for (auto &bundle : infos) {
        TAG_LOGI(AAFwkTag::APPMGR, "processName = [%{public}s]", bundle.applicationInfo.process.c_str());
        if (bundle.applicationInfo.process.empty()) {
            continue;
        }
        auto processName = bundle.applicationInfo.process;
        // Inspection records
        auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
            bundle.applicationInfo.name, processName, bundle.applicationInfo.uid, bundle);
        if (appRecord) {
            TAG_LOGI(AAFwkTag::APPMGR, "processName [%{public}s] Already exists ", processName.c_str());
            continue;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "Start empty resident process, processName = [%{public}s]", processName.c_str());
        StartEmptyResidentProcess(bundle, processName, restartCount, isEmptyKeepAliveApp);
    }
}

void AppMgrServiceInner::StartEmptyResidentProcess(
    const BundleInfo &info, const std::string &processName, int restartCount, bool isEmptyKeepAliveApp)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start bundle [%{public}s | processName [%{public}s]]", info.name.c_str(),
        processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        TAG_LOGI(AAFwkTag::APPMGR, "Failed to start resident process!");
        return;
    }

    bool appExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByBundleName(info.name);
    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);

    if (!appExistFlag) {
        NotifyAppRunningStatusEvent(info.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
    }

    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] failed!", processName.c_str());
        return;
    }

    appRecord->SetKeepAliveAppState(true, isEmptyKeepAliveApp);

    StartProcess(appInfo->name, processName, 0, appRecord, appInfo->uid, info, appInfo->bundleName, 0, appExistFlag);

    // If it is empty, the startup failed
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] failed!", processName.c_str());
        return;
    }

    if (restartCount > 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "StartEmptyResidentProcess restartCount : [%{public}d], ", restartCount);
        appRecord->SetRestartResidentProcCount(restartCount);
    }

    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    TAG_LOGI(AAFwkTag::APPMGR, "StartEmptyResidentProcess of pid : [%{public}d], ",
        appRecord->GetPriorityObject()->GetPid());
}

bool AppMgrServiceInner::CheckRemoteClient()
{
    if (!remoteClientManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return false;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appSpawnClient is null");
        return false;
    }

    if (!remoteClientManager_->GetBundleManagerHelper()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get bundle manager helper fail.");
        return false;
    }
    return true;
}

void AppMgrServiceInner::RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Restart resident process failed, the appRecord is nullptr.");
        return;
    }
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    appRecord->SetRestartTimeMillis(static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS));
    appRecord->DecRestartResidentProcCount();

    auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
        return (appRecord != nullptr && appRecord->GetBundleName() == appRunningRecord->GetBundleName());
    };
    auto findIter = find_if(restartResedentTaskList_.begin(), restartResedentTaskList_.end(), findRestartResidentTask);
    if (findIter != restartResedentTaskList_.end()) {
        restartResedentTaskList_.erase(findIter);
    }

    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "restart resident process failed!");
        return;
    }

    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    BundleInfo bundleInfo;
    auto callerUid = IPCSkeleton::GetCallingUid();
    auto userId = GetUserIdByUid(callerUid);
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(
        appRecord->GetBundleName(), BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleInfo fail.");
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    TAG_LOGI(AAFwkTag::APPMGR, "the resident process [%{public}s] remaining restarts num is [%{public}d]",
        appRecord->GetProcessName().c_str(), (int)appRecord->GetRestartResidentProcCount());
    StartResidentProcess(infos, appRecord->GetRestartResidentProcCount(), appRecord->IsEmptyKeepAliveApp());
}

void AppMgrServiceInner::NotifyAppStatus(const std::string &bundleName, const std::string &eventData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "bundle name is %{public}s, event is %{public}s",
        bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void AppMgrServiceInner::NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t userId,
    const int32_t callerUid, const std::string &eventData)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "%{public}s called, bundle name is %{public}s, , userId is %{public}d, event is %{public}s", __func__,
        bundleName.c_str(), userId, eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, userId);
    want.SetParam(Constants::UID, callerUid);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

int32_t AppMgrServiceInner::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterApplicationStateObserver(
        observer, bundleNameList);
}

int32_t AppMgrServiceInner::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrServiceInner::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterAppForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterAppForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::RegisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrServiceInner::GetForegroundApplications(std::vector<AppStateData> &list)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    appRunningManager_->GetForegroundApplications(list);
    return ERR_OK;
}

int AppMgrServiceInner::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "Enter");
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-b");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    if (KillApplicationByUserIdLocked(bundleName, userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to kill the application");
        return ERR_INVALID_VALUE;
    }

    HapModuleInfo hapModuleInfo;
    if (GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get HapModuleInfo for TestRunner");
        return ERR_INVALID_VALUE;
    }

    std::string processName;
    MakeProcessName(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo), hapModuleInfo, processName);
    TAG_LOGI(AAFwkTag::APPMGR, "processName = [%{public}s]", processName.c_str());

    // Inspection records
    auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
        bundleInfo.applicationInfo.name, processName, bundleInfo.applicationInfo.uid, bundleInfo);
    if (appRecord) {
        TAG_LOGI(AAFwkTag::APPMGR, "processName [%{public}s] Already exists ", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    return StartEmptyProcess(want, observer, bundleInfo, processName, userId);
}

int AppMgrServiceInner::GetHapModuleInfoForTestRunner(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    TAG_LOGI(AAFwkTag::APPMGR, "Enter");
    if (!observer) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer nullptr.");
        return ERR_INVALID_VALUE;
    }

    bool moduleJson = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduleJson = bundleInfo.hapModuleInfos.back().isModuleJson;
    }
    if (moduleJson) {
        std::string moduleName = want.GetStringParam("-m");
        if (moduleName.empty()) {
            UserTestAbnormalFinish(observer, "No module name is specified.");
            return ERR_INVALID_VALUE;
        }

        bool found = false;
        for (auto item : bundleInfo.hapModuleInfos) {
            if (item.moduleName == moduleName) {
                hapModuleInfo = item;
                found = true;
                break;
            }
        }
        if (!found) {
            UserTestAbnormalFinish(observer, "The specified module name is not found.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int AppMgrServiceInner::UserTestAbnormalFinish(const sptr<IRemoteObject> &observer, const std::string &msg)
{
    sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(observer);
    if (!observerProxy) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to get ITestObserver proxy");
        return ERR_INVALID_VALUE;
    }
    observerProxy->TestFinished(msg, -1);
    return ERR_OK;
}

int AppMgrServiceInner::StartEmptyProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &info, const std::string &processName, const int userId)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter bundle [%{public}s | processName [%{public}s]]", info.name.c_str(),
        processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to start the process being tested!");
        return ERR_INVALID_VALUE;
    }

    bool appExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByBundleName(info.name);
    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    if (!appExistFlag) {
        NotifyAppRunningStatusEvent(info.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to start process [%{public}s]!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    auto isDebug = want.GetBoolParam(DEBUG_APP, false);
    TAG_LOGI(AAFwkTag::APPMGR, "Set Debug : %{public}s", (isDebug ? "true" : "false"));
    appRecord->SetDebugApp(isDebug);
    if (want.GetBoolParam(COLD_START, false)) {
        appRecord->SetDebugApp(true);
    }

    std::shared_ptr<UserTestRecord> testRecord = std::make_shared<UserTestRecord>();
    if (!testRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to make UserTestRecord!");
        return ERR_INVALID_VALUE;
    }
    testRecord->want = want;
    testRecord->observer = observer;
    testRecord->isFinished = false;
    testRecord->userId = userId;
    appRecord->SetUserTestInfo(testRecord);

    int32_t bundleIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
    uint32_t startFlags = AppspawnUtil::BuildStartFlags(want, info.applicationInfo);
    StartProcess(appInfo->name, processName, startFlags, appRecord, appInfo->uid, info, appInfo->bundleName,
        bundleIndex, appExistFlag);

    // If it is empty, the startup failed
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to start process [%{public}s]!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    appRecord->SetTaskHandler(taskHandler_);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    TAG_LOGI(AAFwkTag::APPMGR, "StartEmptyProcess OK pid : [%{public}d]", appRecord->GetPriorityObject()->GetPid());

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName, const pid_t &pid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "Enter");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
        return ERR_INVALID_VALUE;
    }

    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "unstart user test");
        return ERR_INVALID_VALUE;
    }

    FinishUserTestLocked(msg, resultCode, appRecord);

    int ret = KillApplicationByUserIdLocked(bundleName, userTestRecord->userId);
    if (ret) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to kill process.");
        return ret;
    }

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTestLocked(
    const std::string &msg, const int64_t &resultCode, const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Enter");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid appRecord");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<ffrt::mutex> lock(userTestLock_);
    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        TAG_LOGD(AAFwkTag::APPMGR, "not start user test");
        return ERR_INVALID_VALUE;
    }
    if (!userTestRecord->isFinished) {
        sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(userTestRecord->observer);
        if (!observerProxy) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to get ITestObserver proxy");
            return ERR_INVALID_VALUE;
        }
        observerProxy->TestFinished(msg, resultCode);

        userTestRecord->isFinished = true;
    }

    return ERR_OK;
}

void AppMgrServiceInner::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Start specified ability.");
    if (!CheckRemoteClient()) {
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);

    int32_t appIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, processName);

    std::vector<HapModuleInfo> hapModules;
    hapModules.emplace_back(hapModuleInfo);

    std::shared_ptr<AppRunningRecord> appRecord;
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        bool appExistFlag = appRunningManager_->CheckAppRunningRecordIsExistByBundleName(bundleInfo.name);
        if (!appExistFlag) {
            NotifyAppRunningStatusEvent(
                bundleInfo.name, appInfo->uid, AbilityRuntime::RunningStatus::APP_RUNNING_START);
        }
        // new app record
        appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "start process [%{public}s] failed!", processName.c_str());
            return;
        }
        if (hapModuleInfo.isStageBasedModel && !IsMainProcess(appInfo, hapModuleInfo)) {
            appRecord->SetKeepAliveAppState(false, false);
            TAG_LOGD(AAFwkTag::APPMGR, "The process %{public}s will not keepalive", hapModuleInfo.process.c_str());
        }
        auto wantPtr = std::make_shared<AAFwk::Want>(want);
        if (wantPtr != nullptr) {
            appRecord->SetCallerPid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
            appRecord->SetCallerUid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
            appRecord->SetCallerTokenId(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
            appRecord->SetDebugApp(wantPtr->GetBoolParam(DEBUG_APP, false));
            if (appRecord->IsDebugApp()) {
                ProcessAppDebug(appRecord, true);
            }
            appRecord->SetNativeDebug(wantPtr->GetBoolParam("nativeDebug", false));
            if (wantPtr->GetBoolParam(COLD_START, false)) {
                appRecord->SetDebugApp(true);
            }
            appRecord->SetPerfCmd(wantPtr->GetStringParam(PERF_CMD));
        }
        appRecord->SetProcessAndExtensionType(abilityInfoPtr);
        appRecord->SetTaskHandler(taskHandler_);
        appRecord->SetEventHandler(eventHandler_);
        appRecord->SendEventForSpecifiedAbility(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG,
            AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT);
        uint32_t startFlags = AppspawnUtil::BuildStartFlags(want, abilityInfo);
        int32_t bundleIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
        StartProcess(appInfo->name, processName, startFlags, appRecord, appInfo->uid, bundleInfo, appInfo->bundleName,
            bundleIndex, appExistFlag);

        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        appRecord->AddModules(appInfo, hapModules);
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "process is exist");
        auto isDebugApp = want.GetBoolParam(DEBUG_APP, false);
        if (isDebugApp && !appRecord->IsDebugApp()) {
            ProcessAppDebug(appRecord, isDebugApp);
        }

        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            TAG_LOGD(AAFwkTag::APPMGR, "module record is nullptr, add modules");
            appRecord->AddModules(appInfo, hapModules);
            appRecord->AddAbilityStageBySpecifiedAbility(appInfo->bundleName);
        } else {
            TAG_LOGD(AAFwkTag::APPMGR, "schedule accept want");
            appRecord->ScheduleAcceptWant(hapModuleInfo.moduleName);
        }
    }
}

void AppMgrServiceInner::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!response) {
        TAG_LOGE(AAFwkTag::APPMGR, "response is nullptr, register failed.");
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Not abilityMgr call.", __func__);
        return;
    }

    startSpecifiedAbilityResponse_ = response;
}

void AppMgrServiceInner::ScheduleAcceptWantDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Schedule accept want done, flag: %{public}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app record failed.");
        return;
    }
    appRecord->ScheduleAcceptWantDone();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnAcceptWantResponse(want, flag);
    }
}

void AppMgrServiceInner::HandleStartSpecifiedAbilityTimeOut(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called start specified ability time out!");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::ScheduleNewProcessRequestDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "ScheduleNewProcessRequestDone, flag: %{public}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app record failed.");
        return;
    }
    appRecord->ScheduleNewProcessRequestDone();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestResponse(want, flag);
    }
}

void AppMgrServiceInner::HandleStartSpecifiedProcessTimeout(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called start specified process time out!");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnNewProcessRequestTimeoutResponse(appRecord->GetNewProcessRequestWant());
    }
}

int32_t AppMgrServiceInner::UpdateConfiguration(const Configuration &config)
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return ERR_INVALID_VALUE;
    }
    CHECK_CALLER_IS_SYSTEM_APP;

    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyUpdateConfigurationPerm();
    if (ret != ERR_OK) {
        return ret;
    }

    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    TAG_LOGI(AAFwkTag::APPMGR, "changeKeyV size :%{public}zu", changeKeyV.size());
    if (config.GetItem(AAFwk::GlobalConfigurationKey::THEME).empty() && changeKeyV.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "changeKeyV is empty");
        return ERR_INVALID_VALUE;
    }
    configuration_->Merge(changeKeyV, config);
    // all app
    int32_t result = appRunningManager_->UpdateConfiguration(config);
    HandleConfigurationChange(config);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "update error, not notify");
        return result;
    }
    // notify
    std::lock_guard<ffrt::mutex> notifyLock(configurationObserverLock_);
    for (auto &observer : configurationObservers_) {
        if (observer != nullptr) {
            observer->OnConfigurationUpdated(config);
        }
    }
    return result;
}

void AppMgrServiceInner::HandleConfigurationChange(const Configuration &config)
{
    std::lock_guard lock(appStateCallbacksLock_);
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->NotifyConfigurationChange(config, currentUserId_);
        }
    }
}

int32_t AppMgrServiceInner::RegisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");

    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrServiceInner::Register error: observer is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> registerLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const sptr<IConfigurationObserver> &item) {
            return (item && item->AsObject() == observer->AsObject());
        }
    );
    if (it != configurationObservers_.end()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrServiceInner::Register error: observer exist");
        return ERR_INVALID_VALUE;
    }
    configurationObservers_.push_back(observer);
    return NO_ERROR;
}

int32_t AppMgrServiceInner::UnregisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    TAG_LOGI(AAFwkTag::APPMGR, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrServiceInner::Register error: observer is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> unregisterLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const sptr<IConfigurationObserver> &item) {
            return (item && item->AsObject() == observer->AsObject());
        }
    );
    if (it != configurationObservers_.end()) {
        configurationObservers_.erase(it);
        return NO_ERROR;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "end");
    return ERR_INVALID_VALUE;
}

void AppMgrServiceInner::InitGlobalConfiguration()
{
    if (!configuration_) {
        TAG_LOGE(AAFwkTag::APPMGR, "configuration_ is null");
        return;
    }

#ifdef SUPPORT_GRAPHICS
    // Currently only this interface is known
    auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
    TAG_LOGI(AAFwkTag::APPMGR, "current global language is : %{public}s", language.c_str());
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
#endif

    // Assign to default colorMode "light"
    TAG_LOGI(AAFwkTag::APPMGR, "current global colorMode is : %{public}s", ConfigurationInner::COLOR_MODE_LIGHT);
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);

    // Get input pointer device
    std::string hasPointerDevice = system::GetParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
    TAG_LOGI(AAFwkTag::APPMGR, "current hasPointerDevice is %{public}s", hasPointerDevice.c_str());
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, hasPointerDevice);

    // Get DeviceType
    auto deviceType = GetDeviceType();
    TAG_LOGI(AAFwkTag::APPMGR, "current deviceType is %{public}s", deviceType);
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, deviceType);
}

std::shared_ptr<AppExecFwk::Configuration> AppMgrServiceInner::GetConfiguration()
{
    return configuration_;
}

void AppMgrServiceInner::KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Kill application by appRecord.");
    if (!appRecord || !taskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "appRecord or taskHandler_ is nullptr.");
        return;
    }

    auto pid = appRecord->GetPriorityObject()->GetPid();
    appRecord->SetTerminating();
    appRecord->ScheduleProcessSecurityExit();

    auto startTime = SystemTimeMillisecond();
    std::list<pid_t> pids = {pid};
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The remote process exited successfully");
        return;
    }

    auto timeoutTask = [pid, innerService = shared_from_this()]() {
        TAG_LOGI(AAFwkTag::APPMGR, "KillProcessByPid %{public}d", pid);
        int32_t result = innerService->KillProcessByPid(pid, "KillApplicationByRecord");
        if (result < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "Kill application by app record failed, pid: %{public}d", pid);
            return;
        }
    };
    taskHandler_->SubmitTask(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
}

void AppMgrServiceInner::SendHiSysEvent(const int32_t innerEventId, const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called AppMgrServiceInner SendHiSysEvent!");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "SendHiSysEvent, Hook_mode: no handle time out");
        return;
    }

    std::string eventName = AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT;
    int32_t pid = appRecord->GetPriorityObject()->GetPid();
    int32_t uid = appRecord->GetUid();
    std::string packageName = appRecord->GetBundleName();
    std::string processName = appRecord->GetProcessName();
    std::string msg = AppExecFwk::AppFreezeType::APP_LIFECYCLE_TIMEOUT;
    msg += ",";
    int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    switch (innerEventId) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT;
            break;
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT;
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT;
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT;
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG:
            msg += EVENT_MESSAGE_START_SPECIFIED_PROCESS_TIMEOUT;
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        default:
            msg += EVENT_MESSAGE_DEFAULT;
            break;
    }

    TAG_LOGW(AAFwkTag::APPMGR, "LIFECYCLE_TIMEOUT, eventName = %{public}s, uid = %{public}d, pid = %{public}d, \
        packageName = %{public}s, processName = %{public}s, msg = %{public}s",
        eventName.c_str(), uid, pid, packageName.c_str(), processName.c_str(), msg.c_str());
    AppfreezeManager::ParamInfo info = {
        .typeId = typeId,
        .pid = pid,
        .eventName = eventName,
        .bundleName = packageName,
        .msg = msg
    };
    AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
}

int AppMgrServiceInner::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
        return ERR_NAME_NOT_FOUND;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (!isSaCall && callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verify failed.");
        return ERR_PERMISSION_DENIED;
    }
    for (auto &item : appRecord->GetAbilities()) {
        tokens.emplace_back(item.first);
    }
    return ERR_OK;
}

int AppMgrServiceInner::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
    bool &debug)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "no permissions.");
        return ERR_PERMISSION_DENIED;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord for PID:%{public}d", pid);
        return ERR_NAME_NOT_FOUND;
    }

    auto info = appRecord->GetApplicationInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ApplicationInfo is nullptr !");
        return ERR_NO_INIT;
    }
    application = *info;
    debug = appRecord->IsDebugApp();
    return ERR_OK;
}

int AppMgrServiceInner::VerifyProcessPermission() const
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    return isCallingPerm ? ERR_OK : ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyProcessPermission(const std::string &bundleName) const
{
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (isSaCall || isShellCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        auto appRecord = GetAppRunningRecordByPid(callerPid);
        if (!appRecord || appRecord->GetBundleName() != bundleName) {
            TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

int AppMgrServiceInner::VerifyProcessPermission(const sptr<IRemoteObject> &token) const
{
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        auto callerUid = IPCSkeleton::GetCallingUid();
        auto appRecord = GetAppRunningRecordByAbilityToken(token);
        if (!appRecord || appRecord->GetUid() != callerUid) {
            TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    return ERR_OK;
}

bool AppMgrServiceInner::CheckCallerIsAppGallery()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return false;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app running record by calling pid failed. callingPId: %{public}d", callerPid);
        return false;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (!bundleMgrHelper) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }
    auto callerBundleName = appRecord->GetBundleName();
    if (callerBundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerBundleName is empty.");
        return false;
    }
    std::string appGalleryBundleName;
    if (!bundleMgrHelper->QueryAppGalleryBundleName(appGalleryBundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "QueryAppGalleryBundleName failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "callerBundleName:%{public}s, appGalleryBundleName:%{public}s", callerBundleName.c_str(),
        appGalleryBundleName.c_str());

    return callerBundleName == appGalleryBundleName;
}

bool AppMgrServiceInner::VerifyAPL() const
{
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return false;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app running record by calling pid failed. callingPId: %{public}d", callerPid);
        return false;
    }

    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get application info failed.");
        return false;
    }

    auto apl = applicationInfo->appPrivilegeLevel;
    if (apl != SYSTEM_BASIC && apl != SYSTEM_CORE) {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not system_basic or system_core.");
        return false;
    }
    return true;
}

int AppMgrServiceInner::VerifyAccountPermission(const std::string &permissionName, const int userId) const
{
    if (userId != currentUserId_) {
        auto isCallingPermAccount = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS);
        if (!isCallingPermAccount) {
            TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission accounts verification failed", __func__);
            return ERR_PERMISSION_DENIED;
        }
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(permissionName);
    return isCallingPerm ? ERR_OK : ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyRequestPermission() const
{
    auto callerUid = IPCSkeleton::GetCallingUid();

    if (callerUid == ROOT_UID || callerUid == FOUNDATION_UID) {
        return ERR_OK;
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed, callerUid: %{public}d", callerUid);
        return ERR_PERMISSION_DENIED;
    }
}

int AppMgrServiceInner::PreStartNWebSpawnProcess(const pid_t hostPid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "called");
    if (hostPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such app Record, pid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    ErrCode errCode = nwebSpawnClient->PreStartNWebSpawnProcess();
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to spawn new render process, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }

    return 0;
}

int AppMgrServiceInner::StartRenderProcess(const pid_t hostPid, const std::string &renderParam,
    int32_t ipcFd, int32_t sharedFd, int32_t crashFd, pid_t &renderPid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "start render process, hostPid:%{public}d", hostPid);
    if (hostPid <= 0 || renderParam.empty() || ipcFd <= 0 || sharedFd <= 0 ||
        crashFd <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid param: hostPid:%{public}d renderParam:%{private}s "
                    "ipcFd:%{public}d  crashFd:%{public}d sharedFd:%{public}d",
            hostPid, renderParam.c_str(), ipcFd, crashFd, sharedFd);
        return ERR_INVALID_VALUE;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr, not start render process");
        return ERR_INVALID_VALUE;
    }

    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty() && !AAFwk::AppUtils::GetInstance().IsUseMultiRenderProcess()) {
        for (auto iter : renderRecordMap) {
            if (iter.second != nullptr) {
                renderPid = iter.second->GetPid();
                if (ProcessExist(renderPid)) {
                    TAG_LOGW(AAFwkTag::APPMGR,
                        "already exist render process,do not request again, renderPid:%{public}d", renderPid);
                    return ERR_ALREADY_EXIST_RENDER;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler) {
                    TAG_LOGW(AAFwkTag::APPMGR, "render process not realy exist, renderPid:%{public}d", renderPid);
                    OnRenderRemoteDied(scheduler->AsObject());
                }
            }
        }
    }

    // The phone device allows a maximum of 40 render processes to be created.
    if (AAFwk::AppUtils::GetInstance().IsLimitMaximumOfRenderProcess() &&
        renderRecordMap.size() >= PHONE_MAX_RENDER_PROCESS_NUM) {
        TAG_LOGE(AAFwkTag::APPMGR, "Reaching the maximum render process limitation, hostPid:%{public}d", hostPid);
        return ERR_REACHING_MAXIMUM_RENDER_PROCESS_LIMITATION;
    }

    auto renderRecord = RenderRecord::CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, crashFd, appRecord);
    if (!renderRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "create render record failed, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    return StartRenderProcessImpl(renderRecord, appRecord, renderPid);
}

void AppMgrServiceInner::AttachRenderProcess(const pid_t pid, const sptr<IRenderScheduler> &scheduler)
{
    TAG_LOGD(AAFwkTag::APPMGR, "attach render process start");
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid render process pid:%{public}d", pid);
        return;
    }
    if (!scheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "render scheduler is null");
        return;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return;
    }

    TAG_LOGI(AAFwkTag::APPMGR, "attach render process pid:%{public}d", pid);
    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such app Record, pid:%{public}d", pid);
        return;
    }

    auto renderRecord = appRecord->GetRenderRecordByPid(pid);
    if (!renderRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such render Record, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsRenderProcess(true);
    renderRecord->SetScheduler(scheduler);
    renderRecord->SetDeathRecipient(appDeathRecipient);
    renderRecord->RegisterDeathRecipient();

    // notify fd to render process
    scheduler->NotifyBrowserFd(renderRecord->GetIpcFd(),
                               renderRecord->GetSharedFd(),
                               renderRecord->GetCrashFd());
}

bool AppMgrServiceInner::GenerateRenderUid(int32_t &renderUid)
{
    std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
    int32_t uid = lastRenderUid_ + 1;
    bool needSecondScan = true;
    if (uid > Constants::END_UID_FOR_RENDER_PROCESS) {
        uid = Constants::START_UID_FOR_RENDER_PROCESS;
        needSecondScan = false;
    }

    if (renderUidSet_.empty()) {
        renderUid = uid;
        renderUidSet_.insert(renderUid);
        lastRenderUid_ = renderUid;
        return true;
    }

    for (int32_t i = uid; i <= Constants::END_UID_FOR_RENDER_PROCESS; i++) {
        if (renderUidSet_.find(i) == renderUidSet_.end()) {
            renderUid = i;
            renderUidSet_.insert(renderUid);
            lastRenderUid_ = renderUid;
            return true;
        }
    }

    if (needSecondScan) {
        for (int32_t i = Constants::START_UID_FOR_RENDER_PROCESS; i <= lastRenderUid_; i++) {
            if (renderUidSet_.find(i) == renderUidSet_.end()) {
                renderUid = i;
                renderUidSet_.insert(renderUid);
                lastRenderUid_ = renderUid;
                return true;
            }
        }
    }

    return false;
}

int AppMgrServiceInner::StartRenderProcessImpl(const std::shared_ptr<RenderRecord> &renderRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &renderPid)
{
    if (!renderRecord || !appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderRecord or appRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    int32_t renderUid = Constants::INVALID_UID;
    if (!GenerateRenderUid(renderUid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Generate renderUid failed");
        return ERR_INVALID_OPERATION;
    }

    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    startMsg.renderParam = renderRecord->GetRenderParam();
    startMsg.uid = renderUid;
    startMsg.gid = renderUid;
    startMsg.code = 0; // 0: DEFAULT
    pid_t pid = 0;
    ErrCode errCode = nwebSpawnClient->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to spawn new render process, errCode %{public}08x", errCode);
        std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
        renderUidSet_.erase(renderUid);
        return ERR_INVALID_VALUE;
    }
    renderPid = pid;
    renderRecord->SetPid(pid);
    renderRecord->SetUid(renderUid);
    appRecord->AddRenderRecord(renderRecord);
    TAG_LOGI(AAFwkTag::APPMGR,
        "start render process success, hostPid:%{public}d, hostUid:%{public}d, pid:%{public}d, uid:%{public}d",
        renderRecord->GetHostPid(), renderRecord->GetHostUid(), pid, renderUid);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessCreated(renderRecord);
    return 0;
}

int AppMgrServiceInner::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remoteClientManager_ is null");
        return ERR_INVALID_VALUE;
    }
    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg;
    startMsg.pid = renderPid;
    startMsg.code = 1; // 1: GET_RENDER_TERMINATION_STATUS
    ErrCode errCode = nwebSpawnClient->GetRenderProcessTerminationStatus(startMsg, status);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to get render process termination status, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "Get render process termination status success, renderPid:%{public}d, status:%{public}d",
        renderPid, status);
    return 0;
}

void AppMgrServiceInner::OnRenderRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::APPMGR, "On render remote died.");
    if (appRunningManager_) {
        auto renderRecord = appRunningManager_->OnRemoteRenderDied(remote);
        if (renderRecord) {
            {
                std::lock_guard<ffrt::mutex> lock(renderUidSetLock_);
                renderUidSet_.erase(renderRecord->GetUid());
            }
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
        }
    }
}

void AppMgrServiceInner::AddWatchParameter()
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s called.", __func__);
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(shared_from_this());
    int ret = WatchParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, PointerDeviceEventCallback,
        context);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "watch parameter %{public}s failed with %{public}d.",
            AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, ret);
    }
}

void AppMgrServiceInner::InitFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "begin initFocus listener.");
    if (focusListener_) {
        return;
    }

    focusListener_ = new WindowFocusChangedListener(shared_from_this(), taskHandler_);
    auto registerTask = [innerService = shared_from_this()]() {
        if (innerService) {
            TAG_LOGI(AAFwkTag::APPMGR, "RegisterFocusListener task");
            innerService->RegisterFocusListener();
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTask(registerTask, "RegisterFocusListenerTask", REGISTER_FOCUS_DELAY);
        TAG_LOGI(AAFwkTag::APPMGR, "Submit RegisterFocusListenerTask");
    }
}

void AppMgrServiceInner::RegisterFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "RegisterFocusListener begin");
    if (!focusListener_) {
        TAG_LOGE(AAFwkTag::APPMGR, "no focusListener_");
        return;
    }
    WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
    TAG_LOGI(AAFwkTag::APPMGR, "RegisterFocusListener end");
}

void AppMgrServiceInner::FreeFocusListener()
{
    TAG_LOGI(AAFwkTag::APPMGR, "FreeFocusListener begin");
    if (!focusListener_) {
        TAG_LOGE(AAFwkTag::APPMGR, "no focusListener_");
        return;
    }
    WindowManager::GetInstance().UnregisterFocusChangedListener(focusListener_);
    focusListener_ = nullptr;
    TAG_LOGI(AAFwkTag::APPMGR, "FreeFocusListener end");
}

void AppMgrServiceInner::HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "focused, invalid focusChangeInfo");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "focused, uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "focused, no such appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, true)) {
        TAG_LOGD(
            AAFwkTag::APPMGR, "only change ability focus state, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationFirstFocused(*appRecord);
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp, true);
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        TAG_LOGW(AAFwkTag::APPMGR, "unfocused, invalid focusChangeInfo");
        return;
    }
    TAG_LOGD(
        AAFwkTag::APPMGR, "unfocused, uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "unfocused, no such appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, false)) {
        TAG_LOGD(AAFwkTag::APPMGR,
            "only change ability from focus to unfocus, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationUnfocused(appRecord->GetBundleName());
    OnAppStateChanged(appRecord, appRecord->GetState(), needNotifyApp, true);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::InitWindowVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Begin.");
    if (windowVisibilityChangedListener_ != nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "Visibility listener has been initiated.");
        return;
    }
    windowVisibilityChangedListener_ =
        new (std::nothrow) WindowVisibilityChangedListener(weak_from_this(), taskHandler_);
    auto registerTask = [innerService = weak_from_this()] () {
        auto inner = innerService.lock();
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Service inner is nullptr.");
            return;
        }
        if (inner->windowVisibilityChangedListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Window visibility changed listener is nullptr.");
            return;
        }
        WindowManager::GetInstance().RegisterVisibilityChangedListener(inner->windowVisibilityChangedListener_);
    };

    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Task handler is nullptr.");
        return;
    }
    taskHandler_->SubmitTask(registerTask, "RegisterVisibilityListener.", REGISTER_VISIBILITY_DELAY);
    TAG_LOGD(AAFwkTag::APPMGR, "End.");
}

void AppMgrServiceInner::FreeWindowVisibilityChangedListener()
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (windowVisibilityChangedListener_ == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "Visibility listener has been freed.");
        return;
    }
    WindowManager::GetInstance().UnregisterVisibilityChangedListener(windowVisibilityChangedListener_);
}

void AppMgrServiceInner::HandleWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (windowVisibilityInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "Window visibility info is empty.");
        return;
    }
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App running manager is nullptr.");
        return;
    }
    appRunningManager_->OnWindowVisibilityChanged(windowVisibilityInfos);
}

void AppMgrServiceInner::PointerDeviceEventCallback(const char *key, const char *value, void *context)
{
    TAG_LOGI(AAFwkTag::APPMGR, "%{public}s called.", __func__);
    auto weak = static_cast<std::weak_ptr<AppMgrServiceInner>*>(context);
    if (weak == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "context is nullptr.");
        return;
    }

    auto appMgrServiceInner = weak->lock();
    if (appMgrServiceInner == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "app manager service inner is nullptr.");
        return;
    }

    if ((strcmp(key, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE) != 0) ||
        ((strcmp(value, "true") != 0) && (strcmp(value, "false") != 0))) {
        TAG_LOGE(AAFwkTag::APPMGR, "key %{public}s or value %{public}s mismatch.", key, value);
        return;
    }

    Configuration changeConfig;
    if (!changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, value)) {
        TAG_LOGE(AAFwkTag::APPMGR, "add %{public}s item to configuration failed.", key);
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "update config %{public}s to %{public}s", key, value);
    auto result = IN_PROCESS_CALL(appMgrServiceInner->UpdateConfiguration(changeConfig));
    if (result != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "update config failed with %{public}d, key: %{public}s, value: %{public}s.", result,
            key, value);
        return;
    }
}

bool AppMgrServiceInner::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "app running manager is nullptr.");
        return false;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission deny, not SA.");
        return false;
    }

    return appRunningManager_->GetAppRunningStateByBundleName(bundleName);
}

int32_t AppMgrServiceInner::NotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission deny, not quick_fix.");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrServiceInner::NotifyLoadPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int &patchVerion)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_DEBUG("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        HILOG_DEBUG("Permission deny, not quick_fix.");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyLoadPatch(bundleName, callback, patchVerion);
}

int32_t AppMgrServiceInner::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission deny, not quick_fix.");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyHotReloadPage(bundleName, callback);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrServiceInner::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerToken not SA %{public}s", __func__);
        return ERR_INVALID_VALUE;
    }

    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app running record by pid failed. pid: %{public}d", pid);
        return false;
    }
    appRecord->SetContinuousTaskAppState(isContinuousTask);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);

    return ERR_OK;
}
#endif

int32_t AppMgrServiceInner::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission deny, not quick_fix.");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyUnLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrServiceInner::NotifyUnLoadPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_DEBUG("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    if (IPCSkeleton::GetCallingUid() != QUICKFIX_UID) {
        HILOG_DEBUG("Permission deny, not quick_fix.");
        return ERR_PERMISSION_DENIED;
    }

    return appRunningManager_->NotifyUnLoadPatch(bundleName, callback);
}

void AppMgrServiceInner::AppRecoveryNotifyApp(int32_t pid, const std::string& bundleName,
    FaultDataType faultType, const std::string& markers)
{
    if (faultType != FaultDataType::APP_FREEZE) {
        TAG_LOGI(AAFwkTag::APPMGR,
            "AppRecovery NotifyApp to kill is: bundleName: %{public}s, faultType: "
            "%{public}d, pid: %{public}d", bundleName.c_str(), faultType, pid);
        KillProcessByPid(pid, "AppRecoveryNotifyApp");
        return;
    }

    std::string timeOutName = "waitSaveTask" + std::to_string(pid) + bundleName;
    if (markers == "appRecovery") {
        TAG_LOGI(AAFwkTag::APPMGR, "waitSaveTask finish, but not kill process "
            "immediately, wait for dump stack util 2s timeout");
        return;
    }

    if (markers != "recoveryTimeout") {
        return;
    }
    auto waitSaveTask = [pid, bundleName, innerService = shared_from_this()]() {
        auto appRecord = innerService->GetAppRunningRecordByPid(pid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
            return;
        }
        std::string name = appRecord->GetBundleName();
        if (bundleName == name) {
            TAG_LOGI(AAFwkTag::APPMGR,
                "waitSaveTask timeout %{public}s,pid == %{public}d is going to exit due to AppRecovery.",
                bundleName.c_str(), pid);
            innerService->KillProcessByPid(pid, "AppRecoveryNotifyApp");
        }
    };
    constexpr int32_t timeOut = 2000;
    taskHandler_->SubmitTask(waitSaveTask, timeOutName, timeOut);
}

int32_t AppMgrServiceInner::NotifyAppFault(const FaultData &faultData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called.");
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName = appRecord->GetBundleName();

    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        if (CheckAppFault(appRecord, faultData)) {
            return ERR_OK;
        }

        if (faultData.waitSaveState) {
            AppRecoveryNotifyApp(pid, bundleName, FaultDataType::APP_FREEZE, "recoveryTimeout");
        }
    }

    auto notifyAppTask = [appRecord, pid, callerUid, bundleName, faultData, innerService = shared_from_this()]() {
        if (faultData.faultType == FaultDataType::APP_FREEZE) {
            AppfreezeManager::AppInfo info = {
                .pid = pid,
                .uid = callerUid,
                .bundleName = bundleName,
                .processName = bundleName,
            };
            AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
        }

        TAG_LOGW(AAFwkTag::APPMGR,
            "FaultData is: name: %{public}s, faultType: %{public}d, uid: %{public}d, pid: %{public}d,"
            "bundleName: %{public}s, faultData.forceExit==%{public}d, faultData.waitSaveState==%{public}d",
            faultData.errorObject.name.c_str(), faultData.faultType,
            callerUid, pid, bundleName.c_str(), faultData.forceExit, faultData.waitSaveState);
    };

    if (AppExecFwk::AppfreezeManager::GetInstance()->IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPMGR,
            "heap dump, don't reportEvent and kill:%{public}s, pid:%{public}d, bundleName:%{public}s.",
            faultData.errorObject.name.c_str(), pid, bundleName.c_str());
        return ERR_OK;
    }

    if (!dfxTaskHandler_) {
        TAG_LOGW(AAFwkTag::APPMGR, "get dfx ffrt handler failed!");
        return ERR_INVALID_VALUE;
    }

    dfxTaskHandler_->SubmitTask(notifyAppTask, "NotifyAppFaultTask");

    if (appRecord->GetApplicationInfo()->asanEnabled) {
        TAG_LOGI(AAFwkTag::APPMGR,
            "FaultData %{public}s, pid == %{public}d is asan app, don't kill.", bundleName.c_str(), pid);
        return ERR_OK;
    }

    KillFaultApp(pid, bundleName, faultData);

    return ERR_OK;
}

bool AppMgrServiceInner::CheckAppFault(const std::shared_ptr<AppRunningRecord> &appRecord, const FaultData &faultData)
{
    if (faultData.timeoutMarkers != "" && !taskHandler_->CancelTask(faultData.timeoutMarkers)) {
        return true;
    }

    if (appRecord->IsDebugging()) {
        return true;
    }
    return false;
}

int32_t AppMgrServiceInner::KillFaultApp(int32_t pid, const std::string &bundleName, const FaultData &faultData)
{
    auto killAppTask = [pid, bundleName, faultData, innerService = shared_from_this()]() {
        if (faultData.forceExit && !faultData.waitSaveState) {
            TAG_LOGI(AAFwkTag::APPMGR, "FaultData %{public}s,pid == %{public}d is going to exit due to %{public}s.",
                bundleName.c_str(), pid, innerService->FaultTypeToString(faultData.faultType).c_str());
            innerService->KillProcessByPid(pid, "KillFaultApp");
            return;
        }
    };
    constexpr int32_t waitTime = 2000;
    // wait 2s before kill application
    taskHandler_->SubmitTask(killAppTask, "killAppTask", waitTime);
    return ERR_OK;
}

void AppMgrServiceInner::TimeoutNotifyApp(int32_t pid, int32_t uid,
    const std::string& bundleName, const FaultData &faultData)
{
    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        AppfreezeManager::AppInfo info = {
            .pid = pid,
            .uid = uid,
            .bundleName = bundleName,
            .processName = bundleName,
        };
        AppExecFwk::AppfreezeManager::GetInstance()->AppfreezeHandleWithStack(faultData, info);
    }
}

int32_t AppMgrServiceInner::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    std::string callerBundleName;
    if (auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper(); bundleMgrHelper != nullptr) {
        int32_t callingUid = IPCSkeleton::GetCallingUid();
        IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
    }
#ifdef ABILITY_FAULT_AND_EXIT_TEST
    if ((AAFwk::PermissionVerification::GetInstance()->IsSACall()) ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
#else
    if ((AAFwk::PermissionVerification::GetInstance()->IsSACall()) || callerBundleName == SCENE_BOARD_BUNDLE_NAME) {
#endif
        int32_t pid = faultData.pid;
        auto record = GetAppRunningRecordByPid(pid);
        if (record == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "no such AppRunningRecord");
            return ERR_INVALID_VALUE;
        }

        FaultData transformedFaultData = ConvertDataTypes(faultData);
        int32_t uid = record->GetUid();
        std::string bundleName = record->GetBundleName();

        if (faultData.errorObject.name == "appRecovery") {
            AppRecoveryNotifyApp(pid, bundleName, faultData.faultType, "appRecovery");
            return ERR_OK;
        }

        if (transformedFaultData.timeoutMarkers.empty()) {
            transformedFaultData.timeoutMarkers = "notifyFault:" + transformedFaultData.errorObject.name +
                std::to_string(pid) + "-" + std::to_string(SystemTimeMillisecond());
        }
        const int64_t timeout = 11000;
        if (faultData.faultType == FaultDataType::APP_FREEZE) {
            if (!AppExecFwk::AppfreezeManager::GetInstance()->IsHandleAppfreeze(bundleName) || record->IsDebugging()) {
                return ERR_OK;
            }
            auto timeoutNotifyApp = std::bind(&AppMgrServiceInner::TimeoutNotifyApp, this,
                pid, uid, bundleName, transformedFaultData);
            taskHandler_->SubmitTask(timeoutNotifyApp, transformedFaultData.timeoutMarkers, timeout);
        }
        record->NotifyAppFault(transformedFaultData);
        TAG_LOGW(AAFwkTag::APPMGR, "FaultDataBySA is: name: %{public}s, faultType: %{public}s, uid: %{public}d,"
            "pid: %{public}d, bundleName: %{public}s", faultData.errorObject.name.c_str(),
            FaultTypeToString(faultData.faultType).c_str(), uid, pid, bundleName.c_str());
    } else {
        TAG_LOGD(AAFwkTag::APPMGR, "this is not called by SA.");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

FaultData AppMgrServiceInner::ConvertDataTypes(const AppFaultDataBySA &faultData)
{
    FaultData newfaultData;
    newfaultData.faultType = faultData.faultType;
    newfaultData.errorObject.message =
        "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    newfaultData.errorObject.message += faultData.errorObject.message;
    newfaultData.errorObject.name = faultData.errorObject.name;
    newfaultData.errorObject.stack = faultData.errorObject.stack;
    newfaultData.timeoutMarkers = faultData.timeoutMarkers;
    newfaultData.waitSaveState = faultData.waitSaveState;
    newfaultData.notifyApp = faultData.notifyApp;
    newfaultData.forceExit = faultData.forceExit;
    newfaultData.token = faultData.token;
    newfaultData.state = faultData.state;
    return newfaultData;
}

std::string AppMgrServiceInner::FaultTypeToString(AppExecFwk::FaultDataType type)
{
    std::string typeStr = "UNKNOWN";
    switch (type) {
        case AppExecFwk::FaultDataType::CPP_CRASH:
            typeStr = "CPP_CRASH";
            break;
        case AppExecFwk::FaultDataType::JS_ERROR:
            typeStr = "JS_ERROR";
            break;
        case AppExecFwk::FaultDataType::APP_FREEZE:
            typeStr = "APP_FREEZE";
            break;
        case AppExecFwk::FaultDataType::PERFORMANCE_CONTROL:
            typeStr = "PERFORMANCE_CONTROL";
            break;
        case AppExecFwk::FaultDataType::RESOURCE_CONTROL:
            typeStr = "RESOURCE_CONTROL";
            break;
        default:
            break;
    }
    return typeStr;
}

bool AppMgrServiceInner::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    if (!CheckGetRunningInfoPermission()) {
        return false;
    }
    for (const auto &it : runningSharedBundleList_) {
        for (const auto &item : it.second) {
            if (item.bundleName == bundleName && item.versionCode == versionCode) {
                return true;
            }
        }
    }
    return false;
}

int32_t AppMgrServiceInner::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!CheckGetRunningInfoPermission()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    isRunning = appRunningManager_->CheckAppRunningRecordIsExistByBundleName(bundleName);
    return ERR_OK;
}

int32_t AppMgrServiceInner::StartNativeProcessForDebugger(const AAFwk::Want &want) const
{
    auto&& bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get bundle manager helper error.");
        return ERR_INVALID_OPERATION;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe bundleName:%{public}s", want.GetElement().GetBundleName().c_str());
    TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe moduleName:%{public}s", want.GetElement().GetModuleName().c_str());
    TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe abilityName:%{public}s", want.GetElement().GetAbilityName().c_str());

    AbilityInfo abilityInfo;
    auto userId = GetCurrentAccountId();
    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(want, GetFlag(), userId, abilityInfo));
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, 0)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleAndHapInfo failed");
        return ERR_INVALID_OPERATION;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, 0, processName);

    auto&& appRecord =
        appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The appRecord not found.");
        return ERR_INVALID_OPERATION;
    }

    bool isSandboxApp = want.GetBoolParam(ENTER_SANDBOX, false);
    if (isSandboxApp) {
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe sandbox: true");
    }
    auto&& cmd = want.GetStringParam(PERF_CMD);
    if (cmd.size() == 0) {
        cmd = want.GetStringParam(DEBUG_CMD);
        TAG_LOGI(AAFwkTag::APPMGR, "debuggablePipe debugCmd:%{public}s", cmd.c_str());
        if (!appInfo->debug) {
            TAG_LOGE(AAFwkTag::APPMGR, "The app is not debug mode.");
            return ERR_INVALID_OPERATION;
        }
        return StartPerfProcess(appRecord, "", cmd, isSandboxApp);
    } else {
        return StartPerfProcess(appRecord, cmd, "", isSandboxApp);
    }
}

int32_t AppMgrServiceInner::GetFlag() const
{
    return AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA;
}

int32_t AppMgrServiceInner::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "QueryActiveOsAccountIds failed.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s, QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

void AppMgrServiceInner::SetRunningSharedBundleList(const std::string &bundleName,
    const std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList)
{
    runningSharedBundleList_.try_emplace(bundleName, baseSharedBundleInfoList);
}

void AppMgrServiceInner::RemoveRunningSharedBundleList(const std::string &bundleName)
{
    auto iterator = runningSharedBundleList_.find(bundleName);
    if (iterator == runningSharedBundleList_.end()) {
        return;
    }
    runningSharedBundleList_.erase(iterator);
}

void AppMgrServiceInner::SetCurrentUserId(const int32_t userId)
{
    if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "set current userId: %{public}d", userId);
    currentUserId_ = userId;
}

int32_t AppMgrServiceInner::GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    auto callerRecord = GetAppRunningRecordByPid(pid);
    if (callerRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "callerRecord is nullptr, can not get callerBundleName.");
        return ERR_INVALID_OPERATION;
    }
    bundleName = callerRecord->GetBundleName();
    uid = callerRecord->GetUid();
    return ERR_OK;
}

void AppMgrServiceInner::KillRenderProcess(const std::shared_ptr<AppRunningRecord> &appRecord) {
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty()) {
        for (auto iter : renderRecordMap) {
            auto renderRecord = iter.second;
            if (renderRecord && renderRecord->GetPid() > 0) {
                TAG_LOGD(AAFwkTag::APPMGR, "Kill render process when host died.");
                KillProcessByPid(renderRecord->GetPid(), "KillRenderProcess");
                {
                    std::lock_guard lock(renderUidSetLock_);
                    renderUidSet_.erase(renderRecord->GetUid());
                }
                DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
            }
        }
    }
}

int32_t AppMgrServiceInner::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    uint64_t memSize = OHOS::MemInfo::GetPssByPid(pid);
    memorySize = static_cast<int32_t>(memSize);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetRunningProcessInformation(
    const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "The appRunningManager is nullptr!");
        return ERR_NO_INIT;
    }

    if (remoteClientManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The remoteClientManager_ is nullptr!");
        return ERR_NO_INIT;
    }
    auto bundleMgrHelper = remoteClientManager_->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "The bundleMgrHelper is nullptr!");
        return ERR_NO_INIT;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "userid value is %{public}d", userId);
    int uid = IN_PROCESS_CALL(bundleMgrHelper->GetUidByBundleName(bundleName, userId));
    TAG_LOGI(AAFwkTag::APPMGR, "uid value is %{public}d", uid);
    const auto &appRunningRecordMap = appRunningManager_->GetAppRunningRecordMap();
    for (const auto &item : appRunningRecordMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        for (const auto &appInfo : appInfoList) {
            if (appInfo == nullptr) {
                continue;
            }
            if (appInfo->bundleName == bundleName && appInfo->uid == uid) {
                GetRunningProcesses(appRecord, info);
                break;
            }
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::ChangeAppGcState(pid_t pid, int32_t state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called, pid:%{public}d, state:%{public}d.", pid, state);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such appRecord");
        return ERR_INVALID_VALUE;
    }
    return appRecord->ChangeAppGcState(state);
}

int32_t AppMgrServiceInner::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appDebugManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    return appDebugManager_->RegisterAppDebugListener(listener);
}

int32_t AppMgrServiceInner::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appDebugManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    return appDebugManager_->UnregisterAppDebugListener(listener);
}

int32_t AppMgrServiceInner::AttachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Developer Mode is false.");
        return ERR_INVALID_OPERATION;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr.");
        return ERR_NO_INIT;
    }
    appRunningManager_->SetAttachAppDebug(bundleName, true);

    auto debugInfos = appRunningManager_->GetAppDebugInfosByBundleName(bundleName, false);
    if (!debugInfos.empty() && appDebugManager_ != nullptr) {
        appDebugManager_->StartDebug(debugInfos);
    }

    NotifyAbilitysDebugChange(bundleName, true);
    return ERR_OK;
}

int32_t AppMgrServiceInner::DetachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr.");
        return ERR_NO_INIT;
    }

    auto debugInfos = appRunningManager_->GetAppDebugInfosByBundleName(bundleName, true);
    if (!debugInfos.empty()) {
        appRunningManager_->SetAttachAppDebug(bundleName, false);
        if (appDebugManager_ != nullptr) {
            appDebugManager_->StopDebug(debugInfos);
        }
    }

    NotifyAbilitysDebugChange(bundleName, false);
    return ERR_OK;
}

int32_t AppMgrServiceInner::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    HILOG_DEBUG("Called, bundle name is %{public}s, persist flag is %{public}d.", bundleName.c_str(), isPersist);
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer mode is false.");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    if (bundleName.empty()) {
        HILOG_ERROR("The bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    InitAppWaitingDebugList();

    bool isClear = false;
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        if (!waitingDebugBundleList_.empty()) {
            waitingDebugBundleList_.clear();
            isClear = true;
        }
    }
    if (isClear) {
        DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
    }

    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        waitingDebugBundleList_.try_emplace(bundleName, isPersist);
    }
    if (isPersist) {
        return DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->SetAppWaitingDebugInfo(
            bundleName);
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::CancelAppWaitingDebug()
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer mode is false.");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        waitingDebugBundleList_.clear();
    }
    return DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
}

int32_t AppMgrServiceInner::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer mode is false.");
        return AAFwk::ERR_NOT_DEVELOPER_MODE;
    }

    InitAppWaitingDebugList();

    std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
    if (waitingDebugBundleList_.empty()) {
        HILOG_DEBUG("The waiting debug bundle list is empty.");
        return ERR_OK;
    }

    for (const auto &item : waitingDebugBundleList_) {
        std::string debugBundleInfo;
        debugBundleInfo.append("bundle name : ").append(item.first).append(", persist : ")
            .append(item.second ? "true" : "false");
        debugInfoList.emplace_back(debugBundleInfo);
    }
    return ERR_OK;
}

void AppMgrServiceInner::InitAppWaitingDebugList()
{
    HILOG_DEBUG("Called.");
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        if (isInitAppWaitingDebugListExecuted_) {
            HILOG_DEBUG("No need to initialize again.");
            return;
        }
        isInitAppWaitingDebugListExecuted_ = true;
    }

    std::vector<std::string> bundleNameList;
    DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->GetAppWaitingDebugList(bundleNameList);
    if (!bundleNameList.empty()) {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        for (const auto &item : bundleNameList) {
            waitingDebugBundleList_.try_emplace(item, true);
        }
    }
}

bool AppMgrServiceInner::IsWaitingDebugApp(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    InitAppWaitingDebugList();

    std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
    if (waitingDebugBundleList_.empty()) {
        HILOG_DEBUG("The waiting debug bundle list is empty.");
        return false;
    }

    for (const auto &item : waitingDebugBundleList_) {
        if (item.first == bundleName) {
            return true;
        }
    }
    return false;
}

void AppMgrServiceInner::ClearNonPersistWaitingDebugFlag()
{
    HILOG_DEBUG("Called.");
    bool isClear = false;
    {
        std::lock_guard<ffrt::mutex> lock(waitingDebugLock_);
        for (const auto &item : waitingDebugBundleList_) {
            if (!item.second) {
                isClear = true;
                break;
            }
        }
        if (isClear) {
            waitingDebugBundleList_.clear();
        }
    }

    if (isClear) {
        DelayedSingleton<AbilityRuntime::AppConfigDataManager>::GetInstance()->ClearAppWaitingDebugInfo();
    }
}

int32_t AppMgrServiceInner::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (response == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Response is nullptr.");
        return ERR_INVALID_VALUE;
    }

    abilityDebugResponse_ = response;
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAbilitysDebugChange(const std::string &bundleName, const bool &isAppDebug)
{
    if (appRunningManager_ == nullptr || abilityDebugResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ or abilityDebugResponse is nullptr.");
        return ERR_NO_INIT;
    }

    std::vector<sptr<IRemoteObject>> tokens;
    appRunningManager_->GetAbilityTokensByBundleName(bundleName, tokens);
    if (!tokens.empty()) {
        isAppDebug ? abilityDebugResponse_->OnAbilitysDebugStarted(tokens) :
            abilityDebugResponse_->OnAbilitysDebugStoped(tokens);
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyAbilitysAssertDebugChange(
    const std::shared_ptr<AppRunningRecord> &appRecord, bool isAssertDebug)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appRecord == nullptr || abilityDebugResponse_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Record or abilityDebugResponse is nullptr.");
        return ERR_NO_INIT;
    }

    std::vector<sptr<IRemoteObject>> abilityTokens;
    auto abilities = appRecord->GetAbilities();
    for (const auto &token : abilities) {
        abilityTokens.emplace_back(token.first);
    }

    if (!abilityTokens.empty()) {
        abilityDebugResponse_->OnAbilitysAssertDebugChange(abilityTokens, isAssertDebug);
    }
    return ERR_OK;
}

bool AppMgrServiceInner::IsAttachDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (appRunningManager_ == nullptr || bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ or bundleName is nullptr.");
        return false;
    }
    return appDebugManager_->IsAttachDebug(bundleName);
}

void AppMgrServiceInner::ApplicationTerminatedSendProcessEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App record is nullptr.");
        return;
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }

    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App running manager is nullptr.");
        return;
    }
    if (!appRunningManager_->CheckAppRunningRecordIsExistByBundleName(appRecord->GetBundleName())) {
        OnAppStopped(appRecord);
    }

    if (appDebugManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App debug manager is nullptr.");
        return;
    }
    auto info = MakeAppDebugInfo(appRecord, appRecord->IsDebugApp());
    appDebugManager_->RemoveAppDebugInfo(info);

    TAG_LOGD(AAFwkTag::APPMGR, "Application is terminated.");
    SendProcessExitEvent(appRecord);
}

void AppMgrServiceInner::ClearAppRunningDataForKeepAlive(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App record is nullptr.");
        return;
    }

    if (appRecord->IsKeepAliveApp()) {
        auto restartProcess = [appRecord, innerService = shared_from_this()]() {
            innerService->RestartResidentProcess(appRecord);
        };
        if (taskHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "taskHandler_ is nullptr.");
            return;
        }
        if (appRecord->CanRestartResidentProc()) {
            taskHandler_->SubmitTask(restartProcess, "RestartResidentProcess");
        } else {
            auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
                return (appRecord != nullptr && appRunningRecord != nullptr &&
                        appRecord->GetBundleName() == appRunningRecord->GetBundleName());
            };
            auto findIter = find_if(restartResedentTaskList_.begin(), restartResedentTaskList_.end(),
                findRestartResidentTask);
            if (findIter != restartResedentTaskList_.end()) {
                TAG_LOGW(AAFwkTag::APPMGR, "The restart app task has been registered.");
                return;
            }
            restartResedentTaskList_.emplace_back(appRecord);
            TAG_LOGD(AAFwkTag::APPMGR, "Post restart resident process delay task.");
            taskHandler_->SubmitTask(restartProcess, "RestartResidentProcessDelayTask", RESTART_INTERVAL_TIME);
        }
    }
}

int32_t AppMgrServiceInner::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!JudgeSelfCalledByToken(token, pageStateData)) {
        return ERR_PERMISSION_DENIED;
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnPageShow(pageStateData);
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!JudgeSelfCalledByToken(token, pageStateData)) {
        return ERR_PERMISSION_DENIED;
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnPageHide(pageStateData);
    return ERR_OK;
}

bool AppMgrServiceInner::JudgeSelfCalledByToken(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null.");
        return false;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "app is not exist!");
        return false;
    }
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    if (appRecord->GetApplicationInfo() == nullptr ||
        ((appRecord->GetApplicationInfo())->accessTokenId) != callingTokenId) {
        TAG_LOGE(AAFwkTag::APPMGR, "Is not self, not enabled");
        return false;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not find ability record");
        return false;
    }
    if (abilityRecord->GetBundleName() != pageStateData.bundleName ||
        abilityRecord->GetModuleName() != pageStateData.moduleName ||
        abilityRecord->GetName() != pageStateData.abilityName) {
        TAG_LOGE(AAFwkTag::APPMGR, "can not map the ability");
        return false;
    }
    return true;
}

void AppMgrServiceInner::SendReStartProcessEvent(const AAFwk::EventInfo &eventInfo,
    const std::shared_ptr<AppRunningRecord> &appRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }
    std::lock_guard<ffrt::mutex> lock(killpedProcessMapLock_);
    int64_t restartTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    for (auto iter = killedPorcessMap_.begin(); iter != killedPorcessMap_.end();) {
        int64_t killTime = iter->first;
        if (restartTime - killTime > 2000) {
            killedPorcessMap_.erase(iter++);
            continue;
        }
        AAFwk::EventInfo currentEventInfo;
        currentEventInfo = eventInfo;
        currentEventInfo.time = restartTime;
        std::string processName = appRecord->GetProcessName();
        currentEventInfo.appUid = appRecord->GetUid();
        if (currentEventInfo.bundleName == currentEventInfo.callerBundleName &&
            processName != currentEventInfo.callerProcessName) {
            currentEventInfo.processName = processName;
            AAFwk::EventReport::SendKeyEvent(AAFwk::EventName::RESTART_PROCESS_BY_SAME_APP,
                HiSysEventType::BEHAVIOR, eventInfo);
            killedPorcessMap_.erase(iter++);
            continue;
        }
        iter++;
    }
}

int32_t AppMgrServiceInner::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Call.");
    CHECK_IS_SA_CALL(listener);
    auto appRunningStatusListener = iface_cast<AbilityRuntime::AppRunningStatusListenerInterface>(listener);
    return appRunningStatusModule_->RegisterListener(appRunningStatusListener);
}

int32_t AppMgrServiceInner::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Call.");
    CHECK_IS_SA_CALL(listener);
    auto appRunningStatusListener = iface_cast<AbilityRuntime::AppRunningStatusListenerInterface>(listener);
    return appRunningStatusModule_->UnregisterListener(appRunningStatusListener);
}

int32_t AppMgrServiceInner::StartChildProcess(const pid_t hostPid, const std::string &srcEntry, pid_t &childPid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "StarChildProcess, hostPid:%{public}d", hostPid);
    auto errCode = StartChildProcessPreCheck(hostPid);
    if (errCode != ERR_OK) {
        return errCode;
    }
    if (hostPid <= 0 || srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid param: hostPid:%{public}d srcEntry:%{private}s", hostPid, srcEntry.c_str());
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return ERR_NO_INIT;
    }
    auto appRecord = GetAppRunningRecordByPid(hostPid);
    auto childProcessRecord = ChildProcessRecord::CreateChildProcessRecord(hostPid, srcEntry, appRecord);
    return StartChildProcessImpl(childProcessRecord, appRecord, childPid);
}

int32_t AppMgrServiceInner::StartChildProcessPreCheck(const pid_t callingPid)
{
    if (!AAFwk::AppUtils::GetInstance().IsMultiProcessModel()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Multi process model is not enabled");
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(callingPid);
    if (appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Already in child process.");
        return ERR_ALREADY_EXISTS;
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::StartChildProcessImpl(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &childPid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such appRecord, childPid:%{public}d.", childPid);
        return ERR_NAME_NOT_FOUND;
    }
    if (!childProcessRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such child process record, childPid:%{public}d.", childPid);
        return ERR_NAME_NOT_FOUND;
    }
    auto spawnClient = remoteClientManager_->GetSpawnClient();
    if (!spawnClient) {
        TAG_LOGE(AAFwkTag::APPMGR, "spawnClient is null");
        return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
    }

    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    startMsg.procName = childProcessRecord->GetProcessName();
    pid_t pid = 0;
    ErrCode errCode = spawnClient->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to spawn new child process, errCode %{public}08x", errCode);
        return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
    }

    childPid = pid;
    childProcessRecord->SetPid(pid);
    childProcessRecord->SetUid(startMsg.uid);
    appRecord->AddChildProcessRecord(pid, childProcessRecord);
    TAG_LOGI(AAFwkTag::APPMGR, "Start child process success, pid:%{public}d, hostPid:%{public}d, uid:%{public}d",
        pid, childProcessRecord->GetHostPid(), startMsg.uid);
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetChildProcessInfoForSelf(ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return ERR_NO_INIT;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (appRunningManager_->GetAppRunningRecordByPid(callingPid)) {
        TAG_LOGD(AAFwkTag::APPMGR, "record of callingPid is not child record.");
        return ERR_NAME_NOT_FOUND;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(callingPid);
    if (!appRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "No such appRecord, childPid:%{public}d", callingPid);
        return ERR_NAME_NOT_FOUND;
    }
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    auto iter = childRecordMap.find(callingPid);
    if (iter != childRecordMap.end()) {
        auto childProcessRecord = iter->second;
        return GetChildProcessInfo(childProcessRecord, appRecord, info);
    }
    return ERR_NAME_NOT_FOUND;
}

int32_t AppMgrServiceInner::GetChildProcessInfo(const std::shared_ptr<ChildProcessRecord> childProcessRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, ChildProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!childProcessRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such child process record.");
        return ERR_NAME_NOT_FOUND;
    }
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such appRecord.");
        return ERR_NAME_NOT_FOUND;
    }
    info.pid = childProcessRecord->GetPid();
    info.hostPid = childProcessRecord->GetHostPid();
    info.uid = childProcessRecord->GetUid();
    info.bundleName = appRecord->GetBundleName();
    info.processName = childProcessRecord->GetProcessName();
    info.srcEntry = childProcessRecord->GetSrcEntry();
    info.jitEnabled = appRecord->IsJITEnabled();
    return ERR_OK;
}

void AppMgrServiceInner::AttachChildProcess(const pid_t pid, const sptr<IChildScheduler> &childScheduler)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AttachChildProcess pid:%{public}d", pid);
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid child process pid:%{public}d", pid);
        return;
    }
    if (!childScheduler) {
        TAG_LOGE(AAFwkTag::APPMGR, "childScheduler is null");
        return;
    }
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is null");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such app Record, pid:%{public}d", pid);
        return;
    }
    auto childRecord = appRecord->GetChildProcessRecordByPid(pid);
    if (!childRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such child process Record, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsChildProcess(true);
    childRecord->SetScheduler(childScheduler);
    childRecord->SetDeathRecipient(appDeathRecipient);
    childRecord->RegisterDeathRecipient();

    childScheduler->ScheduleLoadJs();
}

void AppMgrServiceInner::OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (appRunningManager_) {
        appRunningManager_->OnChildProcessRemoteDied(remote);
    }
}

void AppMgrServiceInner::KillChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord) {
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }
    auto childRecordMap = appRecord->GetChildProcessRecordMap();
    if (childRecordMap.empty()) {
        return;
    }
    for (auto iter : childRecordMap) {
        auto childRecord = iter.second;
        if (!childRecord) {
            continue;
        }
        auto childPid = childRecord->GetPid();
        if (childPid > 0) {
            TAG_LOGI(AAFwkTag::APPMGR, "Kill child process when host died, childPid:%{public}d.", childPid);
            KillProcessByPid(childPid, "KillChildProcess");
        }
    }
}

void AppMgrServiceInner::ExitChildProcessSafelyByChildPid(const pid_t pid)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid <= 0.");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecordByChildProcessPid(pid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such app Record, pid:%{public}d", pid);
        return;
    }
    auto childRecord = appRecord->GetChildProcessRecordByPid(pid);
    if (!childRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "no such child process Record, pid:%{public}d", pid);
        return;
    }
    childRecord->ScheduleExitProcessSafely();
    childRecord->RemoveDeathRecipient();
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    pids.push_back(pid);
    if (WaitForRemoteProcessExit(pids, startTime)) {
        TAG_LOGI(AAFwkTag::APPMGR, "The remote child process exited successfully, pid:%{public}d.", pid);
        appRecord->RemoveChildProcessRecord(childRecord);
        return;
    }
    childRecord->RegisterDeathRecipient();
    int32_t result = KillProcessByPid(pid, "ExitChildProcessSafelyByChildPid");
    if (result < 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "KillChildProcessByPid kill process is fail.");
        return;
    }
}

void AppMgrServiceInner::KillAttachedChildProcess(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }
    auto parentAppRecord = appRecord->GetParentAppRecord();
    if (parentAppRecord) {
        parentAppRecord->RemoveChildAppRecord(appRecord->GetPriorityObject()->GetPid());
    }
    std::vector<pid_t> pids;
    std::queue<std::shared_ptr<AppRunningRecord>> queue;
    queue.push(appRecord);
    while (!queue.empty()) {
        auto front = queue.front();
        queue.pop();
        if (front == nullptr) {
            continue;
        }
        auto childAppRecordMap = front->GetChildAppRecordMap();
        for (const auto& [pid, weakChildAppRecord] : childAppRecordMap) {
            auto childRecord = weakChildAppRecord.lock();
            if (childRecord) {
                queue.push(childRecord);
                pids.push_back(pid);
            }
        }
        front->ClearChildAppRecordMap();
    }
    for (const auto& pid : pids) {
        KillProcessByPid(pid, "KillAttachedChildProcess");
    }
}

void AppMgrServiceInner::NotifyAppRunningStatusEvent(
    const std::string &bundle, int32_t uid, AbilityRuntime::RunningStatus runningStatus)
{
    if (appRunningStatusModule_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Get app running status module object is nullptr.");
        return;
    }
    appRunningStatusModule_->NotifyAppRunningStatusEvent(bundle, uid, runningStatus);
}

void AppMgrServiceInner::SendAppLaunchEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is null");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "applicationInfo is nullptr");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }
    if (appRecord->GetPriorityObject() != nullptr) {
        eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    }
    eventInfo.processName = appRecord->GetProcessName();
    int32_t callerPid = appRecord->GetCallerPid() == -1 ? IPCSkeleton::GetCallingPid() : appRecord->GetCallerPid();
    auto callerRecord = GetAppRunningRecordByPid(callerPid);
    if (callerRecord != nullptr) {
        eventInfo.callerBundleName = callerRecord->GetBundleName();
        eventInfo.callerUid = callerRecord->GetUid();
        eventInfo.callerState = static_cast<int32_t>(callerRecord->GetState());
        auto callerApplicationInfo = callerRecord->GetApplicationInfo();
        if (callerApplicationInfo != nullptr) {
            eventInfo.callerVersionName = callerApplicationInfo->versionName;
            eventInfo.callerVersionCode = callerApplicationInfo->versionCode;
        }
    }
    AAFwk::EventReport::SendAppLaunchEvent(AAFwk::EventName::APP_LAUNCH, eventInfo);
}

bool AppMgrServiceInner::IsFinalAppProcessByBundleName(const std::string &bundleName)
{
    if (appRunningManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App running manager is nullptr.");
        return false;
    }

    auto name = bundleName;
    if (bundleName.empty()) {
        auto callingPid = IPCSkeleton::GetCallingPid();
        auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callingPid);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Get app running record is nullptr.");
            return false;
        }
        name = appRecord->GetBundleName();
    }

    auto count = appRunningManager_->GetAllAppRunningRecordCountByBundleName(name);
    TAG_LOGD(AAFwkTag::APPMGR, "Get application %{public}s process list size[%{public}d].", name.c_str(), count);
    return count == 1;
}

void AppMgrServiceInner::ParseServiceExtMultiProcessWhiteList()
{
    auto serviceExtMultiProcessWhiteList =
        OHOS::system::GetParameter(SERVICE_EXT_MULTI_PROCESS_WHITE_LIST, "");
    if (serviceExtMultiProcessWhiteList.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "Service extension multi process white list is empty.");
        return;
    }
    SplitStr(serviceExtMultiProcessWhiteList, ";", serviceExtensionWhiteList_);
}

void AppMgrServiceInner::ClearProcessByToken(sptr<IRemoteObject> token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is null");
        return;
    }

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard lock(exceptionLock_);
        appRecord = GetAppRunningRecordByAbilityToken(token);
        if (appRecord == nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "app record is not exist for ability token");
            return;
        }
        appRecord->SetApplicationClient(nullptr);
        auto recordId = appRecord->GetRecordId();
        if (appRunningManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
            return;
        }
        appRunningManager_->RemoveAppRunningRecordById(recordId);
    }
    ClearData(appRecord);
}

void AppMgrServiceInner::ClearData(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (appRecord == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "app record is nullptr.");
        return;
    }
    ClearAppRunningData(appRecord, false);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
}

int32_t AppMgrServiceInner::RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->RegisterRenderStateObserver(observer);
}

int32_t AppMgrServiceInner::UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer)
{
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->UnregisterRenderStateObserver(observer);
}

void AppMgrServiceInner::SetAppAssertionPauseState(int32_t pid, bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    if (!system::GetBoolParameter(PRODUCT_ASSERT_FAULT_DIALOG_ENABLED, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Product of assert fault dialog is not enabled.");
        return;
    }
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Developer Mode is false.");
        return;
    }

    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::APPMGR, "Caller is not foundation.");
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such appRecord pid is %{public}d.", pid);
        return;
    }
    appRecord->SetAssertionPauseFlag(flag);
    auto isDebugStart = appRecord->IsDebugApp() || appRecord->isAttachDebug();
    if (!isDebugStart) {
        std::vector<AppDebugInfo> debugInfos;
        debugInfos.emplace_back(MakeAppDebugInfo(appRecord, flag));
        flag ? appDebugManager_->StartDebug(debugInfos) : appDebugManager_->StopDebug(debugInfos);
    }

    NotifyAbilitysAssertDebugChange(appRecord, flag);
}

int32_t AppMgrServiceInner::UpdateRenderState(pid_t renderPid, int32_t state)
{
    int32_t hostPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "No such appRecord, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto renderRecord = appRecord->GetRenderRecordByPid(renderPid);
    if (renderRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "renderPid:%{pubclic}d not exist.", renderPid);
        return ERR_INVALID_VALUE;
    }
    return DelayedSingleton<RenderStateObserverManager>::GetInstance()->OnRenderStateChanged(
        renderRecord, state);
}

int32_t AppMgrServiceInner::SignRestartAppFlag(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    return appRunningManager_->SignRestartAppFlag(bundleName);
}

int32_t AppMgrServiceInner::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    if (!appRunningManager_) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    return appRunningManager_->GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
}

bool AppMgrServiceInner::NotifyMemMgrPriorityChanged(const std::shared_ptr<AppRunningRecord> appRecord)
{
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return false;
    }
    auto priorityObject = appRecord->GetPriorityObject();
    if (!priorityObject) {
        TAG_LOGE(AAFwkTag::APPMGR, "priorityObject is nullptr.");
        return false;
    }
    int32_t pid = priorityObject->GetPid();
    int32_t uid = appRecord->GetUid();
    TAG_LOGI(AAFwkTag::APPMGR, "NotifyMemMgrPriorityChanged, pid:%{public}d, uid:%{public}d", pid, uid);

    Memory::MemMgrProcessStateInfo info;
    info.pid_ = pid;
    info.uid_ = uid;
    info.reason_ = Memory::ProcPriorityUpdateReason::START_ABILITY;
    int32_t result = ERR_OK;
    {
        HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
        result = Memory::MemMgrClient::GetInstance().NotifyProcessStateChangedAsync(info);
    }
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "NotifyPriorityChangedSync error, result:%{public}d.", result);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
