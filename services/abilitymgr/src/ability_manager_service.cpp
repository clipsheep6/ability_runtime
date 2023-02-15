/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_manager_service.h"
#include "accesstoken_kit.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <getopt.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include <csignal>
#include <cstdlib>

#include "ability_info.h"
#include "ability_interceptor.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "application_util.h"
#include "hitrace_meter.h"
#include "bundle_mgr_client.h"
#include "distributed_client.h"
#include "dlp_utils.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "mission_info_mgr.h"
#include "sa_mgr_client.h"
#include "system_ability_token_callback.h"
#include "softbus_bus_center.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "os_account_manager_wrapper.h"
#include "permission_constants.h"
#include "uri_permission_manager_client.h"
#include "xcollie/watchdog.h"
#include "hisysevent.h"
#include "connection_state_manager.h"

#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#include "input_manager.h"
#include "png.h"
#include "ui_service_mgr_client.h"
#endif

#ifdef EFFICIENCY_MANAGER_ENABLE
#include "suspend_manager_client.h"
#endif // EFFICIENCY_MANAGER_ENABLE

#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif // RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "container_manager_client.h"

#include "ability_bundle_event_callback.h"

using OHOS::AppExecFwk::ElementName;
using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AAFwk {
namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        HILOG_ERROR("The caller is not system-app, can not use system-api");                   \
        return ERR_NOT_SYSTEM_APP;                                                             \
    }

const std::string ARGS_USER_ID = "-u";
const std::string ARGS_CLIENT = "-c";
const std::string ILLEGAL_INFOMATION = "The arguments are illegal and you can enter '-h' for help.";

constexpr int32_t NEW_RULE_VALUE_SIZE = 6;
constexpr int64_t APP_ALIVE_TIME_MS = 1000;  // Allow background startup within 1 second after application startup
const std::string IS_DELEGATOR_CALL = "isDelegatorCall";
// Startup rule switch
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
const std::string NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI = "component.startup.newRules.except.LauncherSystemUI";
const std::string BACKGROUND_JUDGE_FLAG = "component.startup.backgroundJudge.flag";
const std::string WHITE_LIST_NORMAL_FLAG = "component.startup.whitelist.normal";
const std::string WHITE_LIST_ASS_WAKEUP_FLAG = "component.startup.whitelist.associatedWakeUp";
// White list app
const std::string BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
const std::string BUNDLE_NAME_SYSTEMUI = "com.ohos.systemui";
const std::string BUNDLE_NAME_SETTINGSDATA = "com.ohos.settingsdata";
const std::string BUNDLE_NAME_DEVICE_TEST = "com.ohos.devicetest";
const std::string BUNDLE_NAME_INPUTMETHOD_TEST = "com.acts.inputmethodtest";
const std::string BUNDLE_NAME_TELE_DATA = "com.ohos.telephonydataability";
const std::string BUNDLE_NAME_CONTACTS_DATA = "com.ohos.contactsdataability";
const std::string BUNDLE_NAME_NOTE = "com.ohos.note";
const std::string BUNDLE_NAME_PHOTO = "com.ohos.photos";
const std::string BUNDLE_NAME_SCREENSHOT = "com.huawei.ohos.screenshot";
const std::string BUNDLE_NAME_SERVICE_TEST = "com.amsst.stserviceabilityclient";
const std::string BUNDLE_NAME_SERVICE_SERVER_TEST = "com.amsst.stserviceabilityserver";
const std::string BUNDLE_NAME_SERVICE_SERVER2_TEST = "com.amsst.stserviceabilityserversecond";

// White list
const std::unordered_set<std::string> WHITE_LIST_NORMAL_SET = { BUNDLE_NAME_DEVICE_TEST,
                                                                BUNDLE_NAME_INPUTMETHOD_TEST,
                                                                BUNDLE_NAME_NOTE,
                                                                BUNDLE_NAME_PHOTO,
                                                                BUNDLE_NAME_SCREENSHOT,
                                                                BUNDLE_NAME_SERVICE_TEST,
                                                                BUNDLE_NAME_SERVICE_SERVER_TEST,
                                                                BUNDLE_NAME_SERVICE_SERVER2_TEST };
const std::unordered_set<std::string> WHITE_LIST_ASS_WAKEUP_SET = { BUNDLE_NAME_TELE_DATA,
                                                                    BUNDLE_NAME_CONTACTS_DATA,
                                                                    BUNDLE_NAME_DEVICE_TEST };
} // namespace

using namespace std::chrono;
using namespace std::chrono_literals;
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
using namespace BackgroundTaskMgr;
#endif
const bool CONCURRENCY_MODE_FALSE = false;
const int32_t MAIN_USER_ID = 100;
constexpr auto DATA_ABILITY_START_TIMEOUT = 5s;
constexpr int32_t NON_ANONYMIZE_LENGTH = 6;
constexpr uint32_t SCENE_FLAG_NORMAL = 0;
const int32_t MAX_NUMBER_OF_DISTRIBUTED_MISSIONS = 20;
const int32_t SWITCH_ACCOUNT_TRY = 3;
#ifdef ABILITY_COMMAND_FOR_TEST
const int32_t BLOCK_AMS_SERVICE_TIME = 65;
#endif
const std::string EMPTY_DEVICE_ID = "";
const int32_t APP_MEMORY_SIZE = 512;
const int32_t GET_PARAMETER_INCORRECT = -9;
const int32_t GET_PARAMETER_OTHER = -1;
const int32_t SIZE_10 = 10;
const int32_t ACCOUNT_MGR_SERVICE_UID = 3058;
const std::string BUNDLE_NAME_KEY = "bundleName";
const std::string DM_PKG_NAME = "ohos.distributedhardware.devicemanager";
const std::string ACTION_CHOOSE = "ohos.want.action.select";
const std::string HIGHEST_PRIORITY_ABILITY_ENTITY = "flag.home.intent.from.system";
const std::string DMS_API_VERSION = "dmsApiVersion";
const std::string DMS_IS_CALLER_BACKGROUND = "dmsIsCallerBackGround";
const std::string DMS_PROCESS_NAME = "distributedsched";
const std::string DMS_MISSION_ID = "dmsMissionId";
const std::string DLP_INDEX = "ohos.dlp.params.index";
const std::string BOOTEVENT_APPFWK_READY = "bootevent.appfwk.ready";
const std::string BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
const std::string BOOTEVENT_BOOT_ANIMATION_STARTED = "bootevent.bootanimation.started";
const std::string NEED_STARTINGWINDOW = "ohos.ability.NeedStartingWindow";
const int DEFAULT_DMS_MISSION_ID = -1;
const std::map<std::string, AbilityManagerService::DumpKey> AbilityManagerService::dumpMap = {
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--all", KEY_DUMP_ALL),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-a", KEY_DUMP_ALL),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--stack-list", KEY_DUMP_STACK_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-l", KEY_DUMP_STACK_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--stack", KEY_DUMP_STACK),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-s", KEY_DUMP_STACK),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission", KEY_DUMP_MISSION),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-m", KEY_DUMP_MISSION),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--top", KEY_DUMP_TOP_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-t", KEY_DUMP_TOP_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--waiting-queue", KEY_DUMP_WAIT_QUEUE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-w", KEY_DUMP_WAIT_QUEUE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--serv", KEY_DUMP_SERVICE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-e", KEY_DUMP_SERVICE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--data", KEY_DUMP_DATA),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-d", KEY_DUMP_DATA),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-focus", KEY_DUMP_FOCUS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-f", KEY_DUMP_FOCUS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--win-mode", KEY_DUMP_WINDOW_MODE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-z", KEY_DUMP_WINDOW_MODE),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission-list", KEY_DUMP_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-L", KEY_DUMP_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("--mission-infos", KEY_DUMP_MISSION_INFOS),
    std::map<std::string, AbilityManagerService::DumpKey>::value_type("-S", KEY_DUMP_MISSION_INFOS),
};

const std::map<std::string, AbilityManagerService::DumpsysKey> AbilityManagerService::dumpsysMap = {
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--all", KEY_DUMPSYS_ALL),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-a", KEY_DUMPSYS_ALL),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--mission-list", KEY_DUMPSYS_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-l", KEY_DUMPSYS_MISSION_LIST),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--ability", KEY_DUMPSYS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-i", KEY_DUMPSYS_ABILITY),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--extension", KEY_DUMPSYS_SERVICE),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-e", KEY_DUMPSYS_SERVICE),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--pending", KEY_DUMPSYS_PENDING),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-p", KEY_DUMPSYS_PENDING),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--process", KEY_DUMPSYS_PROCESS),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-r", KEY_DUMPSYS_PROCESS),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("--data", KEY_DUMPSYS_DATA),
    std::map<std::string, AbilityManagerService::DumpsysKey>::value_type("-d", KEY_DUMPSYS_DATA),
};

const std::map<int32_t, AppExecFwk::SupportWindowMode> AbilityManagerService::windowModeMap = {
    std::map<int32_t, AppExecFwk::SupportWindowMode>::value_type(MULTI_WINDOW_DISPLAY_FULLSCREEN,
        AppExecFwk::SupportWindowMode::FULLSCREEN),
    std::map<int32_t, AppExecFwk::SupportWindowMode>::value_type(MULTI_WINDOW_DISPLAY_PRIMARY,
        AppExecFwk::SupportWindowMode::SPLIT),
    std::map<int32_t, AppExecFwk::SupportWindowMode>::value_type(MULTI_WINDOW_DISPLAY_SECONDARY,
        AppExecFwk::SupportWindowMode::SPLIT),
    std::map<int32_t, AppExecFwk::SupportWindowMode>::value_type(MULTI_WINDOW_DISPLAY_FLOATING,
        AppExecFwk::SupportWindowMode::FLOATING),
};

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<AbilityManagerService>::GetInstance().get());
sptr<AbilityManagerService> AbilityManagerService::instance_;

AbilityManagerService::AbilityManagerService()
    : SystemAbility(ABILITY_MGR_SERVICE_ID, true),
      eventLoop_(nullptr),
      handler_(nullptr),
      state_(ServiceRunningState::STATE_NOT_START),
      iBundleManager_(nullptr)
{
    DumpFuncInit();
    DumpSysFuncInit();
}

AbilityManagerService::~AbilityManagerService()
{}

void AbilityManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_INFO("AMS has already started.");
        return;
    }
    HILOG_INFO("AMS starting.");
    if (!Init()) {
        HILOG_ERROR("Failed to init AMS.");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    eventLoop_->Run();
    /* Publish service maybe failed, so we need call this function at the last,
     * so it can't affect the TDD test program */
    instance_ = DelayedSingleton<AbilityManagerService>::GetInstance().get();
    if (instance_ == nullptr) {
        HILOG_ERROR("AMS enter OnStart, but instance_ is nullptr!");
        return;
    }
    bool ret = Publish(instance_);
    if (!ret) {
        HILOG_ERROR("Publish AMS failed!");
        return;
    }

    SetParameter(BOOTEVENT_APPFWK_READY.c_str(), "true");

    WatchParameter(BOOTEVENT_BOOT_COMPLETED.c_str(), AAFwk::ApplicationUtil::AppFwkBootEventCallback, nullptr);

    HILOG_INFO("AMS start success.");
}

bool AbilityManagerService::Init()
{
    eventLoop_ = AppExecFwk::EventRunner::Create(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    CHECK_POINTER_RETURN_BOOL(eventLoop_);

    handler_ = std::make_shared<AbilityEventHandler>(eventLoop_, weak_from_this());
    CHECK_POINTER_RETURN_BOOL(handler_);

    freeInstallManager_ = std::make_shared<FreeInstallManager>(weak_from_this());
    CHECK_POINTER_RETURN_BOOL(freeInstallManager_);

    // init user controller.
    userController_ = std::make_shared<UserController>();
    userController_->Init();

    InitConnectManager(MAIN_USER_ID, true);
    InitDataAbilityManager(MAIN_USER_ID, true);
    InitPendWantManager(MAIN_USER_ID, true);
    systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();

    amsConfigResolver_ = std::make_shared<AmsConfigurationParameter>();
    amsConfigResolver_->Parse();
    HILOG_INFO("ams config parse");
    InitMissionListManager(MAIN_USER_ID, true);
    SwitchManagers(U0_USER_ID, false);
    int amsTimeOut = amsConfigResolver_->GetAMSTimeOutTime();
    HILOG_INFO("amsTimeOut is %{public}d", amsTimeOut);
    std::string threadName = std::string(AbilityConfig::NAME_ABILITY_MGR_SERVICE) + "(" +
        std::to_string(eventLoop_->GetThreadId()) + ")";
#ifdef SUPPORT_ASAN
    constexpr int32_t timeout = 5 * 60 * 1000; // 5 min
    if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handler_, timeout) != 0) {
        HILOG_ERROR("HiviewDFX::Watchdog::GetInstance AddThread Fail");
    }
#else
    if (HiviewDFX::Watchdog::GetInstance().AddThread(threadName, handler_) != 0) {
        HILOG_ERROR("HiviewDFX::Watchdog::GetInstance AddThread Fail");
    }
#endif
#ifdef SUPPORT_GRAPHICS
    DelayedSingleton<SystemDialogScheduler>::GetInstance()->SetDeviceType(amsConfigResolver_->GetDeviceType());
    implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
#endif

    interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    interceptorExecuter_->AddInterceptor(std::make_shared<CrowdTestInterceptor>());
    interceptorExecuter_->AddInterceptor(std::make_shared<ControlInterceptor>());

    auto startResidentAppsTask = [aams = shared_from_this()]() { aams->StartResidentApps(); };
    handler_->PostTask(startResidentAppsTask, "StartResidentApps");

    SubscribeBackgroundTask();
    DelayedSingleton<ConnectionStateManager>::GetInstance()->Init();
    auto initStartupFlagTask = [aams = shared_from_this()]() { aams->InitStartupFlag(); };
    handler_->PostTask(initStartupFlagTask, "InitStartupFlag");

    // Register abilityBundleEventCallback to receive hap updates
    HILOG_INFO("Register abilityBundleEventCallback to receive hap updates.");
    sptr<AbilityBundleEventCallback> abilityBundleEventCallback_ =
        new (std::nothrow) AbilityBundleEventCallback(handler_);
    auto bms = GetBundleManager();
    if (bms && abilityBundleEventCallback_) {
        bool re = bms->RegisterBundleEventCallback(abilityBundleEventCallback_);
        if (!re) {
            HILOG_ERROR("RegisterBundleEventCallback failed!");
        }
    } else {
        HILOG_ERROR("Get BundleManager or abilieyBundleEventCallback failed!");
    }
    HILOG_INFO("Init success.");
    return true;
}

void AbilityManagerService::InitStartupFlag()
{
    startUpNewRule_ = CheckNewRuleSwitchState(COMPONENT_STARTUP_NEW_RULES);
    newRuleExceptLauncherSystemUI_ = CheckNewRuleSwitchState(NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI);
    backgroundJudgeFlag_ = CheckNewRuleSwitchState(BACKGROUND_JUDGE_FLAG);
    whiteListNormalFlag_ = CheckNewRuleSwitchState(WHITE_LIST_NORMAL_FLAG);
    whiteListassociatedWakeUpFlag_ = CheckNewRuleSwitchState(WHITE_LIST_ASS_WAKEUP_FLAG);
}

void AbilityManagerService::OnStop()
{
    HILOG_INFO("Stop AMS.");
    eventLoop_.reset();
    handler_.reset();
    state_ = ServiceRunningState::STATE_NOT_START;
}

ServiceRunningState AbilityManagerService::QueryServiceState() const
{
    return state_;
}

int AbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s coldStart:%{public}d", __func__, want.GetBoolParam("coldStart", false));
    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    int32_t ret = StartAbilityInner(want, nullptr, requestCode, -1, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    auto flags = want.GetFlags();
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        HILOG_ERROR("StartAbility with continuation flags is not allowed!");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    HILOG_INFO("Start ability come, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);

    int32_t ret = StartAbilityInner(want, callerToken, requestCode, -1, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int callerUid, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CHECK_DLP");
        if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
            VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
            !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
            HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
            return CHECK_PERMISSION_FAILED;
        }

        if (AbilityUtil::HandleDlpApp(const_cast<Want &>(want))) {
            return StartExtensionAbility(want, callerToken, userId, AppExecFwk::ExtensionAbilityType::SERVICE);
        }
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        auto isSpecificSA = AAFwk::PermissionVerification::GetInstance()->
            CheckSpecificSystemAbilityAccessPermission();
        if (!isSpecificSA) {
            HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
            return ERR_INVALID_CALLER;
        }
        HILOG_INFO("%{public}s: Caller is specific system ability.", __func__);
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, requestCode, GetUserId(), true);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    if (callerToken != nullptr && CheckIfOperateRemote(want)) {
        HILOG_INFO("%{public}s: try to StartRemoteAbility", __func__);
        return StartRemoteAbility(want, requestCode, validUserId, callerToken);
    }
    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (freeInstallManager_ == nullptr) {
            return ERR_INVALID_VALUE;
        }
        Want localWant = want;
        if (!localWant.GetDeviceId().empty()) {
            localWant.SetDeviceId("");
        }
        int32_t ret = freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken);
        if (ret != ERR_OK) {
            HILOG_DEBUG("StartFreeInstall ret : %{public}d", ret);
            return ret;
        }
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        if (!IsComponentInterceptionStart(want, callerToken, requestCode, result, abilityRequest)) {
            return ERR_OK;
        }
        abilityRequest.Voluation(want, requestCode, callerToken);
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        return implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (!IsComponentInterceptionStart(want, callerToken, requestCode, result, abilityRequest)) {
        return ERR_OK;
    }

    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        return ERR_WRONG_INTERFACE_CALL;
    } else if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        HILOG_DEBUG("Check call service or extension permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallServicePermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    } else {
        HILOG_DEBUG("Check call ability permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallAbilityPermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        HILOG_DEBUG("PreLoadAppDataAbilities:%{public}s.", abilityInfo.bundleName.c_str());
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d.",
                abilityInfo.bundleName.c_str(), result);
            return result;
        }
    }

    UpdateCallerInfo(abilityRequest.want);
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(validUserId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_DEBUG("Start service or extension, name is %{public}s.", abilityInfo.name.c_str());
        ReportEventToSuspendManager(abilityInfo);
        return connectManager->StartAbility(abilityRequest);
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        HILOG_ERROR("IsAbilityControllerStart failed: %{public}s.", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    ReportAbilitStartInfoToRSS(abilityInfo);
    ReportEventToSuspendManager(abilityInfo);
    HILOG_DEBUG("Start ability, name is %{public}s.", abilityInfo.name.c_str());
    return missionListManager->StartAbility(abilityRequest);
}

int AbilityManagerService::StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability setting.");
    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, requestCode, GetUserId(), true);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            HILOG_ERROR("can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        int32_t ret = freeInstallManager_->StartFreeInstall(want, validUserId, requestCode, callerToken);
        if (ret != ERR_OK) {
            HILOG_DEBUG("StartFreeInstall ret : %{public}d", ret);
            return ret;
        }
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(
            want, requestCode, callerToken, std::make_shared<AbilityStartSetting>(abilityStartSetting));
        abilityRequest.callType = AbilityCallType::START_SETTINGS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (!IsComponentInterceptionStart(want, callerToken, 0, result, abilityRequest)) {
            return ERR_OK;
        }
        if (result != ERR_OK) {
            HILOG_ERROR("implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (!IsComponentInterceptionStart(want, callerToken, 0, result, abilityRequest)) {
        return ERR_OK;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STATIC_CFG_PERMISSION;
    }
    result = CheckCallAbilityPermission(abilityRequest);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s CheckCallAbilityPermission error.", __func__);
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    abilityRequest.startSetting = std::make_shared<AbilityStartSetting>(abilityStartSetting);

    if (abilityInfo.type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability, use 'AcquireDataAbility()' instead.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support for page type ability.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WRONG_INTERFACE_CALL;
    }
#endif
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want);
    auto ret = missionListManager->StartAbility(abilityRequest);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbility(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability options.");
    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, requestCode, GetUserId(), true);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            HILOG_ERROR("can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        int32_t ret = freeInstallManager_->StartFreeInstall(want, validUserId, requestCode, callerToken);
        if (ret != ERR_OK) {
            HILOG_DEBUG("StartFreeInstall ret : %{public}d", ret);
            return ret;
        }
    }
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(want, requestCode, callerToken);
        abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
        abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions.GetWindowMode());
        abilityRequest.callType = AbilityCallType::START_OPTIONS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (!IsComponentInterceptionStart(want, callerToken, 0, result, abilityRequest)) {
            return ERR_OK;
        }
        if (result != ERR_OK) {
            HILOG_ERROR("implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    if (!IsComponentInterceptionStart(want, callerToken, 0, result, abilityRequest)) {
        return ERR_OK;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STATIC_CFG_PERMISSION;
    }
    result = CheckCallAbilityPermission(abilityRequest);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s CheckCallAbilityPermission error.", __func__);
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support for page type ability.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(),
                result);
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
    abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
    abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions.GetWindowMode());
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", oriValidUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

#ifdef SUPPORT_GRAPHICS
    if (abilityInfo.isStageBasedModel && !CheckWindowMode(startOptions.GetWindowMode(), abilityInfo.windowModes)) {
        return ERR_AAFWK_INVALID_WINDOW_MODE;
    }
#endif
    UpdateCallerInfo(abilityRequest.want);
    auto ret = missionListManager->StartAbility(abilityRequest);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

bool AbilityManagerService::IsBackgroundTaskUid(const int uid)
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    return bgtaskObserver_->IsBackgroundTaskUid(uid);
#else
    return false;
#endif
}

int AbilityManagerService::CheckOptExtensionAbility(const Want &want, AbilityRequest &abilityRequest,
    int32_t validUserId, AppExecFwk::ExtensionAbilityType extensionType)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Not extension ability, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    if (extensionType != AppExecFwk::ExtensionAbilityType::UNSPECIFIED &&
        extensionType != abilityInfo.extensionAbilityType) {
        HILOG_ERROR("Extension ability type not match, set type: %{public}d, real type: %{public}d",
            static_cast<int32_t>(extensionType), static_cast<int32_t>(abilityInfo.extensionAbilityType));
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    if (extensionType == AppExecFwk::ExtensionAbilityType::DATASHARE ||
        extensionType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        result = CheckCallServiceExtensionPermission(abilityRequest);
        if (result != ERR_OK) {
            return result;
        }
    } else {
        result = CheckCallOtherExtensionPermission(abilityRequest);
        if (result != ERR_OK) {
            return result;
        }
    }

    UpdateCallerInfo(abilityRequest.want);
    return ERR_OK;
}

void AbilityManagerService::SubscribeBackgroundTask()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    if (bgtaskObserver_) {
        return ;
    }
    bgtaskObserver_ = std::make_shared<BackgroundTaskObserver>();
    auto subscribeBackgroundTask = [aams = shared_from_this()]() {
        int attemptNums = 0;
        while ((BackgroundTaskMgrHelper::SubscribeBackgroundTask(
            *(aams->bgtaskObserver_))) != ERR_OK) {
            ++attemptNums;
            if (attemptNums > SUBSCRIBE_BACKGROUND_TASK_TRY) {
                HILOG_ERROR("subscribeBackgroundTask fail, attemptNums:%{public}d", attemptNums);
                return;
            }
            usleep(REPOLL_TIME_MICRO_SECONDS);
        }
    };
    handler_->PostTask(subscribeBackgroundTask, "SubscribeBackgroundTask");
#endif
}

void AbilityManagerService::ReportAbilitStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
        abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        std::vector<AppExecFwk::RunningProcessInfo> runningProcessInfos;
        if (IN_PROCESS_CALL(GetProcessRunningInfos(runningProcessInfos)) != ERR_OK) {
            return;
        }
        bool isColdStart = true;
        int32_t pid = 0;
        for (auto const &info : runningProcessInfos) {
            if (info.uid_ == abilityInfo.applicationInfo.uid) {
                isColdStart = false;
                pid = info.pid_;
                break;
            }
        }
        std::unordered_map<std::string, std::string> eventParams {
            { "name", "ability_start" },
            { "uid", std::to_string(abilityInfo.applicationInfo.uid) },
            { "bundleName", abilityInfo.applicationInfo.bundleName },
            { "abilityName", abilityInfo.name },
            { "pid", std::to_string(pid) }
        };
        ResourceSchedule::ResSchedClient::GetInstance().ReportData(
            ResourceSchedule::ResType::RES_TYPE_APP_ABILITY_START, isColdStart ? 1 : 0, eventParams);
    }
#endif
}

void AbilityManagerService::ReportEventToSuspendManager(const AppExecFwk::AbilityInfo &abilityInfo)
{
#ifdef EFFICIENCY_MANAGER_ENABLE
    std::string reason = (abilityInfo.type == AppExecFwk::AbilityType::PAGE) ?
        "THAW_BY_START_PAGE_ABILITY" : "THAW_BY_START_NOT_PAGE_ABILITY";
    SuspendManager::SuspendManagerClient::GetInstance().ThawOneApplication(
        abilityInfo.applicationInfo.uid,
        abilityInfo.applicationInfo.bundleName, reason);
#endif // EFFICIENCY_MANAGER_ENABLE
}

int AbilityManagerService::StartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    HILOG_INFO("Start extension ability come, bundlename: %{public}s, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_CALLER_IS_SYSTEM_APP;
    EventInfo eventInfo = BuildEventInfo(want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);
    EventReport::SendExtensionEvent(EventName::START_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);

    auto result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::START_EXTENSION_ERROR);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckDlpForExtension error.");
        return result;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        abilityRequest.Voluation(want, DEFAULT_INVAL_VALUE, callerToken);
        abilityRequest.callType = AbilityCallType::START_EXTENSION_TYPE;
        abilityRequest.extensionType = extensionType;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
#endif
    result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("Start extension begin, name is %{public}s.", abilityInfo.name.c_str());
    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::StopExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    HILOG_INFO("Stop extension ability come, bundlename: %{public}s, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_CALLER_IS_SYSTEM_APP;
    EventInfo eventInfo = BuildEventInfo(want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);
    EventReport::SendExtensionEvent(EventName::STOP_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);

    auto result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::STOP_EXTENSION_ERROR);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckDlpForExtension error.");
        return result;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }
    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckOptExtensionAbility(want, abilityRequest, validUserId, extensionType);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("Stop extension begin, name is %{public}s.", abilityInfo.name.c_str());
    eventInfo.errCode = connectManager->StopServiceAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is Null.");
        return ERR_INVALID_VALUE;
    }
    return TerminateAbilityWithFlag(token, resultCode, resultWant, true);
}

int AbilityManagerService::CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant)
{
    EventInfo eventInfo;
    EventReport::SendAbilityEvent(EventName::CLOSE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    return TerminateAbilityWithFlag(token, resultCode, resultWant, false);
}

int AbilityManagerService::TerminateAbilityWithFlag(const sptr<IRemoteObject> &token, int resultCode,
    const Want *resultWant, bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Terminate ability begin, flag:%{public}d.", flag);
    if (!VerificationAllToken(token)) {
        HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    int result = JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        HILOG_ERROR("System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->TerminateAbility(token);
    }

    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot terminate data ability, use 'ReleaseDataAbility()' instead.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto ownerUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(ownerUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
}

int AbilityManagerService::SendResultToAbility(int32_t requestCode, int32_t resultCode, Want &resultWant)
{
    HILOG_INFO("%{public}s", __func__);
    Security::AccessToken::NativeTokenInfo nativeTokenInfo;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(accessToken);
    int32_t result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(accessToken, nativeTokenInfo);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        result != ERR_OK || nativeTokenInfo.processName != DMS_PROCESS_NAME) {
        HILOG_ERROR("Check processName failed");
        return ERR_INVALID_VALUE;
    }
    int missionId = resultWant.GetIntParam(DMS_MISSION_ID, DEFAULT_DMS_MISSION_ID);
    resultWant.RemoveParam(DMS_MISSION_ID);
    if (missionId == DEFAULT_DMS_MISSION_ID) {
        HILOG_ERROR("MissionId is empty");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->SetResult(std::make_shared<AbilityResult>(requestCode, resultCode, resultWant));
    abilityRecord->SendResult();
    return ERR_OK;
}

int AbilityManagerService::StartRemoteAbility(const Want &want, int requestCode, int32_t validUserId,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s", __func__);
    Want remoteWant = want;
    if (AddStartControlParam(remoteWant, callerToken) != ERR_OK) {
        HILOG_ERROR("%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    if (AbilityUtil::IsStartFreeInstall(remoteWant)) {
        return freeInstallManager_ == nullptr ? ERR_INVALID_VALUE :
            freeInstallManager_->StartRemoteFreeInstall(remoteWant, requestCode, validUserId, callerToken);
    }
    if (remoteWant.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false)) {
        HILOG_INFO("%{public}s: try to StartAbilityForResult", __func__);
        int32_t missionId = GetMissionIdByAbilityToken(callerToken);
        if (missionId < 0) {
            return ERR_INVALID_VALUE;
        }
        remoteWant.SetParam(DMS_MISSION_ID, missionId);
    }

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    HILOG_DEBUG("get callerUid = %d, AccessTokenID = %u", callerUid, accessToken);
    int result = dmsClient.StartRemoteAbility(remoteWant, callerUid, requestCode, accessToken);
    if (result != ERR_NONE) {
        HILOG_ERROR("AbilityManagerService::StartRemoteAbility failed, result = %{public}d", result);
    }
    return result;
}

bool AbilityManagerService::CheckIsRemote(const std::string& deviceId)
{
    if (deviceId.empty()) {
        HILOG_INFO("CheckIsRemote: deviceId is empty.");
        return false;
    }
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId)) {
        HILOG_ERROR("CheckIsRemote: get local deviceId failed");
        return false;
    }
    if (localDeviceId == deviceId) {
        HILOG_INFO("CheckIsRemote: deviceId is local.");
        return false;
    }
    HILOG_DEBUG("CheckIsRemote, deviceId = %{public}s", AnonymizeDeviceId(deviceId).c_str());
    return true;
}

bool AbilityManagerService::CheckIfOperateRemote(const Want &want)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    if (deviceId.empty() || want.GetElement().GetBundleName().empty() ||
        want.GetElement().GetAbilityName().empty()) {
        HILOG_DEBUG("CheckIfOperateRemote: DeviceId or BundleName or GetAbilityName empty");
        return false;
    }
    return CheckIsRemote(deviceId);
}

bool AbilityManagerService::GetLocalDeviceId(std::string& localDeviceId)
{
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t errCode = GetLocalNodeDeviceInfo(DM_PKG_NAME.c_str(), localNode.get());
    if (errCode != ERR_OK) {
        HILOG_ERROR("AbilityManagerService::GetLocalNodeDeviceInfo errCode = %{public}d", errCode);
        return false;
    }
    if (localNode != nullptr) {
        localDeviceId = localNode->networkId;
        HILOG_DEBUG("get local deviceId, deviceId = %{public}s",
            AnonymizeDeviceId(localDeviceId).c_str());
        return true;
    }
    HILOG_ERROR("AbilityManagerService::GetLocalDeviceId localDeviceId null");
    return false;
}

std::string AbilityManagerService::AnonymizeDeviceId(const std::string& deviceId)
{
    if (deviceId.length() < NON_ANONYMIZE_LENGTH) {
        return EMPTY_DEVICE_ID;
    }
    std::string anonDeviceId = deviceId.substr(0, NON_ANONYMIZE_LENGTH);
    anonDeviceId.append("******");
    return anonDeviceId;
}

int AbilityManagerService::TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode)
{
    HILOG_INFO("Terminate ability by caller.");
    if (!VerificationAllToken(callerToken)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_GRAPHICS
    if (IsSystemUiApp(abilityRecord->GetAbilityInfo())) {
        HILOG_ERROR("System ui not allow terminate.");
        return ERR_INVALID_VALUE;
    }
#endif

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    auto missionListManager = GetListManagerByUserId(userId);
    auto connectManager = GetConnectManagerByUserId(userId);
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE:
        case AppExecFwk::AbilityType::EXTENSION: {
            if (!connectManager) {
                HILOG_ERROR("connectManager is nullptr.");
                return ERR_INVALID_VALUE;
            }
            auto result = connectManager->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
                    return ERR_WOULD_BLOCK;
                }

                if (!missionListManager) {
                    HILOG_ERROR("missionListManager is nullptr. userId=%{public}d", userId);
                    return ERR_INVALID_VALUE;
                }
                return missionListManager->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
#ifdef SUPPORT_GRAPHICS
        case AppExecFwk::AbilityType::PAGE: {
            if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
                return ERR_WOULD_BLOCK;
            }
            if (!missionListManager) {
                HILOG_ERROR("missionListManager is nullptr.");
                return ERR_INVALID_VALUE;
            }
            auto result = missionListManager->TerminateAbility(abilityRecord, requestCode);
            if (result == NO_FOUND_ABILITY_BY_CALLER) {
                if (!connectManager) {
                    HILOG_ERROR("connectManager is nullptr.");
                    return ERR_INVALID_VALUE;
                }
                return connectManager->TerminateAbility(abilityRecord, requestCode);
            }
            return result;
        }
#endif
        default:
            return ERR_INVALID_VALUE;
    }
}

int AbilityManagerService::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Minimize ability, fromUser:%{public}d.", fromUser);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    int result = JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Cannot minimize except page ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    auto missionListManager = GetListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
    if (!missionListManager) {
        HILOG_ERROR("missionListManager is Null.");
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MinimizeAbility(token, fromUser);
}

int AbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    return ConnectAbilityCommon(want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

int AbilityManagerService::ConnectAbilityCommon(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Connect ability called, element uri: %{public}s.", want.GetElement().GetURI().c_str());
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    if (extensionType == AppExecFwk::ExtensionAbilityType::SERVICE && IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);

    auto result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::CONNECT_SERVICE_ERROR);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckDlpForExtension error.");
        return result;
    }

    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);

    if (AbilityUtil::IsStartFreeInstall(want) && freeInstallManager_ != nullptr) {
        std::string localDeviceId;
        if (!GetLocalDeviceId(localDeviceId)) {
            HILOG_ERROR("%{public}s: Get Local DeviceId failed", __func__);
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_VALUE;
        }
        result = freeInstallManager_->ConnectFreeInstall(want, validUserId, callerToken, localDeviceId);
        if (result != ERR_OK) {
            eventInfo.errCode = result;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    Want abilityWant = want;
    std::string uri = abilityWant.GetUri().ToString();
    if (!uri.empty()) {
        // if the want include uri, it may only has uri information. it is probably a datashare extension.
        HILOG_INFO("%{public}s called. uri:%{public}s, userId %{public}d", __func__, uri.c_str(), validUserId);
        AppExecFwk::ExtensionAbilityInfo extensionInfo;
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

        AbilityRequest abilityRequest;
        abilityWant.SetParam("abilityConnectionObj", connect->AsObject());
        if (!IsComponentInterceptionStart(abilityWant, callerToken, 0, 0, abilityRequest)) {
            return ERR_OK;
        }
        abilityWant.RemoveParam("abilityConnectionObj");

        bool queryResult = IN_PROCESS_CALL(bms->QueryExtensionAbilityInfoByUri(uri, validUserId, extensionInfo));
        if (!queryResult || extensionInfo.name.empty() || extensionInfo.bundleName.empty()) {
            HILOG_ERROR("Invalid extension ability info.");
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_VALUE;
        }
        abilityWant.SetElementName(extensionInfo.bundleName, extensionInfo.name);
    }

    if (CheckIfOperateRemote(abilityWant)) {
        HILOG_INFO("AbilityManagerService::ConnectAbility. try to ConnectRemoteAbility");
        eventInfo.errCode = ConnectRemoteAbility(abilityWant, callerToken, connect->AsObject());
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }

    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        HILOG_INFO("%{public}s invalid Token.", __func__);
        eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, nullptr, extensionType);
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, callerToken, extensionType);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

EventInfo AbilityManagerService::BuildEventInfo(const Want &want, int32_t userId)
{
    EventInfo eventInfo;
    eventInfo.userId = userId;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    return eventInfo;
}

int AbilityManagerService::DisconnectAbility(const sptr<IAbilityConnection> &connect)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Disconnect ability begin.");
    EventInfo eventInfo;
    EventReport::SendExtensionEvent(EventName::DISCONNECT_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    if (ERR_OK != DisconnectLocalAbility(connect) &&
        ERR_OK != DisconnectRemoteAbility(connect->AsObject())) {
        eventInfo.errCode = INNER_ERR;
        EventReport::SendExtensionEvent(EventName::DISCONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ERR_OK;
}

int AbilityManagerService::ConnectLocalAbility(const Want &want, const int32_t userId,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Connect local ability begin.");
    if (!JudgeMultiUserConcurrency(userId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    ErrCode result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, callerToken, userId);

    Want requestWant = want;
    requestWant.SetParam("abilityConnectionObj", connect->AsObject());
    if (!IsComponentInterceptionStart(requestWant, callerToken, 0, result, abilityRequest)) {
        return ERR_OK;
    }

    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }

    if (abilityRequest.abilityInfo.isStageBasedModel) {
        bool isService = (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE);
        if (isService && extensionType != AppExecFwk::ExtensionAbilityType::SERVICE) {
            HILOG_ERROR("Service extension type, please use ConnectAbility.");
            return ERR_WRONG_INTERFACE_CALL;
        }
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    int32_t validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : userId;
    HILOG_DEBUG("validUserId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not Service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    result = CheckCallServicePermission(abilityRequest);
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s CheckCallServicePermission error.", __func__);
        return result;
    }
    result = PreLoadAppDataAbilities(abilityInfo.bundleName, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("ConnectAbility: App data ability preloading failed, '%{public}s', %{public}d",
            abilityInfo.bundleName.c_str(),
            result);
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }

    ReportEventToSuspendManager(abilityInfo);
    return connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken);
}

int AbilityManagerService::ConnectRemoteAbility(Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("%{public}s begin ConnectAbilityRemote", __func__);
    if (AddStartControlParam(want, callerToken) != ERR_OK) {
        HILOG_ERROR("%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    DistributedClient dmsClient;
    return dmsClient.ConnectRemoteAbility(want, connect);
}

int AbilityManagerService::DisconnectLocalAbility(const sptr<IAbilityConnection> &connect)
{
    HILOG_INFO("Disconnect local ability begin.");
    CHECK_POINTER_AND_RETURN(connectManager_, ERR_NO_INIT);
    if (connectManager_->DisconnectAbilityLocked(connect) == ERR_OK) {
        return ERR_OK;
    }
    // If current connectManager_ does not exist connect, then try connectManagerU0
    auto connectManagerU0 = GetConnectManagerByUserId(U0_USER_ID);
    CHECK_POINTER_AND_RETURN(connectManagerU0, ERR_NO_INIT);
    return connectManagerU0->DisconnectAbilityLocked(connect);
}

int AbilityManagerService::DisconnectRemoteAbility(const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("%{public}s begin DisconnectAbilityRemote", __func__);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    DistributedClient dmsClient;
    return dmsClient.DisconnectRemoteAbility(connect, callerUid, accessToken);
}

int AbilityManagerService::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("ContinueMission missionId: %{public}d", missionId);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    DistributedClient dmsClient;
    return dmsClient.ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
}

int AbilityManagerService::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    HILOG_INFO("ContinueAbility missionId = %{public}d, version = %{public}u.", missionId, versionCode);

    sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->ContinueAbility(deviceId, versionCode);
    return ERR_OK;
}

int AbilityManagerService::StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status)
{
    HILOG_INFO("Start Continuation.");
    if (!CheckIfOperateRemote(want)) {
        HILOG_ERROR("deviceId or bundle name or abilityName empty");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    int32_t appUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    HILOG_INFO("AbilityManagerService::Try to StartContinuation, AccessTokenID = %{public}u", accessToken);
    int32_t missionId = GetMissionIdByAbilityToken(abilityToken);
    if (missionId == -1) {
        HILOG_ERROR("AbilityManagerService::StartContinuation failed to get missionId.");
        return ERR_INVALID_VALUE;
    }
    DistributedClient dmsClient;
    auto result =  dmsClient.StartContinuation(want, missionId, appUid, status, accessToken);
    if (result != ERR_OK) {
        HILOG_ERROR("StartContinuation failed, result = %{public}d, notify caller", result);
        NotifyContinuationResult(missionId, result);
    }
    return result;
}

void AbilityManagerService::NotifyCompleteContinuation(const std::string &deviceId,
    int32_t sessionId, bool isSuccess)
{
    HILOG_INFO("NotifyCompleteContinuation.");
    DistributedClient dmsClient;
    dmsClient.NotifyCompleteContinuation(Str8ToStr16(deviceId), sessionId, isSuccess);
}

int AbilityManagerService::NotifyContinuationResult(int32_t missionId, int32_t result)
{
    HILOG_INFO("Notify Continuation Result : %{public}d.", result);

    auto abilityToken = GetAbilityTokenByMissionId(missionId);
    CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->NotifyContinuationResult(result);
    return ERR_OK;
}

int AbilityManagerService::StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag)
{
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StartSyncRemoteMissions(devId, fixConflict, tag);
}

int AbilityManagerService::StopSyncRemoteMissions(const std::string& devId)
{
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.StopSyncRemoteMissions(devId);
}

int AbilityManagerService::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    return DelayedSingleton<ConnectionStateManager>::GetInstance()->RegisterObserver(observer);
}

int AbilityManagerService::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    return DelayedSingleton<ConnectionStateManager>::GetInstance()->UnregisterObserver(observer);
}

int AbilityManagerService::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        HILOG_ERROR("can not get dlp connection infos if caller is not sa.");
        return CHECK_PERMISSION_FAILED;
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->GetDlpConnectionInfos(infos);

    return ERR_OK;
}

int AbilityManagerService::RegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        HILOG_ERROR("RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
}

int AbilityManagerService::UnRegisterMissionListener(const std::string &deviceId,
    const sptr<IRemoteMissionListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    std::string localDeviceId;
    if (!GetLocalDeviceId(localDeviceId) || localDeviceId == deviceId) {
        HILOG_ERROR("RegisterMissionListener: Check DeviceId failed");
        return REGISTER_REMOTE_MISSION_LISTENER_FAIL;
    }
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.UnRegisterMissionListener(Str8ToStr16(deviceId), listener->AsObject());
}

void AbilityManagerService::RemoveAllServiceRecord()
{
    CHECK_POINTER_LOG(connectManager_, "Connect manager not init.");
    connectManager_->RemoveAll();
}

sptr<IWantSender> AbilityManagerService::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("Get want Sender.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, nullptr);

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, nullptr);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t userId = wantSenderInfo.userId;
    bool bundleMgrResult = false;
    if (userId < 0) {
        if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
            GetOsAccountLocalIdFromUid(callerUid, userId) != 0) {
            HILOG_ERROR("GetOsAccountLocalIdFromUid failed. uid=%{public}d", callerUid);
            return nullptr;
        }
    }

    int32_t appUid = 0;
    if (!wantSenderInfo.allWants.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        std::string bundleName = wantSenderInfo.allWants.back().want.GetElement().GetBundleName();
        bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (bundleMgrResult) {
            appUid = bundleInfo.uid;
        }
        HILOG_INFO("App bundleName: %{public}s, uid: %{public}d", bundleName.c_str(), appUid);
    }

    std::string apl;
    if (!wantSenderInfo.bundleName.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(wantSenderInfo.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (bundleMgrResult) {
            apl = bundleInfo.applicationInfo.appPrivilegeLevel;
        }
    }

    HILOG_INFO("AbilityManagerService::GetWantSender: bundleName = %{public}s", wantSenderInfo.bundleName.c_str());
    return pendingWantManager_->GetWantSender(callerUid, appUid, apl, wantSenderInfo, callerToken);
}

int AbilityManagerService::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    HILOG_INFO("Send want sender.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    return pendingWantManager_->SendWantSender(target, senderInfo);
}

void AbilityManagerService::CancelWantSender(const sptr<IWantSender> &sender)
{
    HILOG_INFO("Cancel want sender.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(sender->AsObject());

    int userId = -1;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(callerUid, userId) != 0) {
        HILOG_ERROR("GetOsAccountLocalIdFromUid failed. uid=%{public}d", callerUid);
        return;
    }
    AppExecFwk::BundleInfo bundleInfo;
    bool bundleMgrResult = IN_PROCESS_CALL(
        bms->GetBundleInfo(record->GetKey()->GetBundleName(),
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleInfo is fail.");
        return;
    }

    auto apl = bundleInfo.applicationInfo.appPrivilegeLevel;
    pendingWantManager_->CancelWantSender(apl, sender);
}

int AbilityManagerService::GetPendingWantUid(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantUid(target);
}

int AbilityManagerService::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantUserId(target);
}

std::string AbilityManagerService::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    HILOG_INFO("Get pending want bundle name.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, "");
    CHECK_POINTER_AND_RETURN(target, "");
    return pendingWantManager_->GetPendingWantBundleName(target);
}

int AbilityManagerService::GetPendingWantCode(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantCode(target);
}

int AbilityManagerService::GetPendingWantType(const sptr<IWantSender> &target)
{
    HILOG_INFO("%{public}s:begin.", __func__);

    if (pendingWantManager_ == nullptr) {
        HILOG_ERROR("%s, pendingWantManager_ is nullptr", __func__);
        return -1;
    }
    if (target == nullptr) {
        HILOG_ERROR("%s, target is nullptr", __func__);
        return -1;
    }
    return pendingWantManager_->GetPendingWantType(target);
}

void AbilityManagerService::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("Register cancel listener.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager_->RegisterCancelListener(sender, receiver);
}

void AbilityManagerService::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver)
{
    HILOG_INFO("Unregister cancel listener.");
    CHECK_POINTER(pendingWantManager_);
    CHECK_POINTER(sender);
    CHECK_POINTER(receiver);
    pendingWantManager_->UnregisterCancelListener(sender, receiver);
}

int AbilityManagerService::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HILOG_INFO("Get pending request want.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(want, ERR_INVALID_VALUE);
    CHECK_CALLER_IS_SYSTEM_APP;
    return pendingWantManager_->GetPendingRequestWant(target, want);
}

int AbilityManagerService::LockMissionForCleanup(int32_t missionId)
{
    HILOG_INFO("request unlock mission for clean up all, id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->SetMissionLockedState(missionId, true);
}

int AbilityManagerService::UnlockMissionForCleanup(int32_t missionId)
{
    HILOG_INFO("request unlock mission for clean up all, id :%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->SetMissionLockedState(missionId, false);
}

int AbilityManagerService::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    HILOG_INFO("request RegisterMissionListener ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->RegisterMissionListener(listener);
}

int AbilityManagerService::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    HILOG_INFO("request RegisterMissionListener ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    return currentMissionListManager_->UnRegisterMissionListener(listener);
}

int AbilityManagerService::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("request GetMissionInfos.");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfos(deviceId, numMax, missionInfos);
    }

    return currentMissionListManager_->GetMissionInfos(numMax, missionInfos);
}

int AbilityManagerService::GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    HILOG_INFO("GetRemoteMissionInfos begin");
    DistributedClient dmsClient;
    int result = dmsClient.GetMissionInfos(deviceId, numMax, missionInfos);
    if (result != ERR_OK) {
        HILOG_ERROR("GetRemoteMissionInfos failed, result = %{public}d", result);
        return result;
    }
    return ERR_OK;
}

int AbilityManagerService::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HILOG_INFO("request GetMissionInfo, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        return GetRemoteMissionInfo(deviceId, missionId, missionInfo);
    }

    return currentMissionListManager_->GetMissionInfo(missionId, missionInfo);
}

int AbilityManagerService::GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    HILOG_INFO("GetMissionInfoFromDms begin");
    std::vector<MissionInfo> missionVector;
    int result = GetRemoteMissionInfos(deviceId, MAX_NUMBER_OF_DISTRIBUTED_MISSIONS, missionVector);
    if (result != ERR_OK) {
        return result;
    }
    for (auto iter = missionVector.begin(); iter != missionVector.end(); iter++) {
        if (iter->id == missionId) {
            missionInfo = *iter;
            return ERR_OK;
        }
    }
    HILOG_WARN("missionId not found");
    return ERR_INVALID_VALUE;
}

int AbilityManagerService::CleanMission(int32_t missionId)
{
    HILOG_INFO("request CleanMission, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    return currentMissionListManager_->ClearMission(missionId);
}

int AbilityManagerService::CleanAllMissions()
{
    HILOG_INFO("request CleanAllMissions ");
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    Want want;
    want.SetElementName(AbilityConfig::LAUNCHER_BUNDLE_NAME, AbilityConfig::LAUNCHER_ABILITY_NAME);
    if (!IsAbilityControllerStart(want, AbilityConfig::LAUNCHER_BUNDLE_NAME)) {
        HILOG_ERROR("IsAbilityControllerStart failed: %{public}s", want.GetBundle().c_str());
        return ERR_WOULD_BLOCK;
    }

    return currentMissionListManager_->ClearAllMissions();
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId)
{
    HILOG_INFO("request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerStartById(missionId)) {
        HILOG_ERROR("IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    return currentMissionListManager_->MoveMissionToFront(missionId);
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HILOG_INFO("request MoveMissionToFront, missionId:%{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    CHECK_CALLER_IS_SYSTEM_APP;

    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerStartById(missionId)) {
        HILOG_ERROR("IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    auto options = std::make_shared<StartOptions>(startOptions);
    return currentMissionListManager_->MoveMissionToFront(missionId, options);
}

int32_t AbilityManagerService::GetMissionIdByToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("request GetMissionIdByToken.");
    if (!token) {
        HILOG_ERROR("token is invalid.");
        return -1;
    }

    if (IPCSkeleton::GetCallingPid() != getpid()) {
        HILOG_ERROR("%{public}s: Only support same process call.", __func__);
        return -1;
    }

    return GetMissionIdByAbilityToken(token);
}

bool AbilityManagerService::IsAbilityControllerStartById(int32_t missionId)
{
    InnerMissionInfo innerMissionInfo;
    int getMission = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        missionId, innerMissionInfo);
    if (getMission != ERR_OK) {
        HILOG_ERROR("cannot find mission info from MissionInfoList by missionId: %{public}d", missionId);
        return true;
    }
    if (!IsAbilityControllerStart(innerMissionInfo.missionInfo.want, innerMissionInfo.missionInfo.want.GetBundle())) {
        HILOG_ERROR("IsAbilityControllerStart failed: %{public}s",
            innerMissionInfo.missionInfo.want.GetBundle().c_str());
        return false;
    }
    return true;
}

std::shared_ptr<AbilityRecord> AbilityManagerService::GetServiceRecordByElementName(const std::string &element)
{
    if (!connectManager_) {
        HILOG_ERROR("Connect manager is nullptr.");
        return nullptr;
    }
    return connectManager_->GetServiceRecordByElementName(element);
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityManagerService::GetConnectRecordListByCallback(
    sptr<IAbilityConnection> callback)
{
    if (!connectManager_) {
        HILOG_ERROR("Connect manager is nullptr.");
        std::list<std::shared_ptr<ConnectionRecord>> connectList;
        return connectList;
    }
    return connectManager_->GetConnectRecordListByCallback(callback);
}

sptr<IAbilityScheduler> AbilityManagerService::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, nullptr);

    auto localUri(uri);
    if (localUri.GetScheme() != AbilityConfig::SCHEME_DATA_ABILITY) {
        HILOG_ERROR("Acquire data ability with invalid uri scheme.");
        return nullptr;
    }
    std::vector<std::string> pathSegments;
    localUri.GetPathSegments(pathSegments);
    if (pathSegments.empty()) {
        HILOG_ERROR("Acquire data ability with invalid uri path.");
        return nullptr;
    }

    auto userId = GetValidUserId(INVALID_USER_ID);
    AbilityRequest abilityRequest;
    std::string dataAbilityUri = localUri.ToString();
    HILOG_INFO("%{public}s, called. userId %{public}d", __func__, userId);
    bool queryResult = IN_PROCESS_CALL(bms->QueryAbilityInfoByUri(dataAbilityUri, userId, abilityRequest.abilityInfo));
    if (!queryResult || abilityRequest.abilityInfo.name.empty() || abilityRequest.abilityInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid ability info for data ability acquiring.");
        return nullptr;
    }

    abilityRequest.callerToken = callerToken;
    if (CheckCallDataAbilityPermission(abilityRequest) != ERR_OK) {
        HILOG_ERROR("Invalid ability request info for data ability acquiring.");
        return nullptr;
    }

    HILOG_DEBUG("Query data ability info: %{public}s|%{public}s|%{public}s",
        abilityRequest.appInfo.name.c_str(), abilityRequest.appInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());

    if (CheckStaticCfgPermission(abilityRequest.abilityInfo) != AppExecFwk::Constants::PERMISSION_GRANTED) {
        if (!VerificationAllToken(callerToken)) {
            HILOG_INFO("VerificationAllToken fail");
            return nullptr;
        }
    }

    if (abilityRequest.abilityInfo.applicationInfo.singleton) {
        userId = U0_USER_ID;
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    CHECK_POINTER_AND_RETURN(dataAbilityManager, nullptr);
    ReportEventToSuspendManager(abilityRequest.abilityInfo);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isNotHap = isSaCall || isShellCall;
    return dataAbilityManager->Acquire(abilityRequest, tryBind, callerToken, isNotHap);
}

int AbilityManagerService::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (!dataAbilityScheduler || !callerToken) {
        HILOG_ERROR("dataAbilitySchedule or callerToken is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<DataAbilityManager> dataAbilityManager = GetDataAbilityManager(dataAbilityScheduler);
    if (!dataAbilityManager) {
        HILOG_ERROR("dataAbilityScheduler is not exists");
        return ERR_INVALID_VALUE;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    bool isNotHap = isSaCall || isShellCall;
    return dataAbilityManager->Release(dataAbilityScheduler, callerToken, isNotHap);
}

int AbilityManagerService::AttachAbilityThread(
    const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Attach ability thread.");
    CHECK_POINTER_AND_RETURN(scheduler, ERR_INVALID_VALUE);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    auto type = abilityInfo.type;
    // force timeout ability for test
    if (IsNeedTimeoutForTest(abilityInfo.name, AbilityRecord::ConvertAbilityState(AbilityState::INITIAL))) {
        HILOG_WARN("force timeout ability for test, state:INITIAL, ability: %{public}s",
            abilityInfo.name.c_str());
        return ERR_OK;
    }
    int returnCode = -1;
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        returnCode = connectManager->AttachAbilityThreadLocked(scheduler, token);
    } else if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        returnCode = dataAbilityManager->AttachAbilityThread(scheduler, token);
    } else {
        int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
        auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
        if (!missionListManager) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
            return ERR_INVALID_VALUE;
        }
        returnCode = missionListManager->AttachAbilityThread(scheduler, token);
    }
    return returnCode;
}

void AbilityManagerService::DumpFuncInit()
{
    dumpFuncMap_[KEY_DUMP_ALL] = &AbilityManagerService::DumpInner;
    dumpFuncMap_[KEY_DUMP_MISSION] = &AbilityManagerService::DumpMissionInner;
    dumpFuncMap_[KEY_DUMP_SERVICE] = &AbilityManagerService::DumpStateInner;
    dumpFuncMap_[KEY_DUMP_DATA] = &AbilityManagerService::DataDumpStateInner;
    dumpFuncMap_[KEY_DUMP_MISSION_LIST] = &AbilityManagerService::DumpMissionListInner;
    dumpFuncMap_[KEY_DUMP_MISSION_INFOS] = &AbilityManagerService::DumpMissionInfosInner;
}

void AbilityManagerService::DumpSysFuncInit()
{
    dumpsysFuncMap_[KEY_DUMPSYS_ALL] = &AbilityManagerService::DumpSysInner;
    dumpsysFuncMap_[KEY_DUMPSYS_MISSION_LIST] = &AbilityManagerService::DumpSysMissionListInner;
    dumpsysFuncMap_[KEY_DUMPSYS_ABILITY] = &AbilityManagerService::DumpSysAbilityInner;
    dumpsysFuncMap_[KEY_DUMPSYS_SERVICE] = &AbilityManagerService::DumpSysStateInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PENDING] = &AbilityManagerService::DumpSysPendingInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PROCESS] = &AbilityManagerService::DumpSysProcess;
    dumpsysFuncMap_[KEY_DUMPSYS_DATA] = &AbilityManagerService::DataDumpSysStateInner;
}

void AbilityManagerService::DumpSysInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    DumpSysMissionListInner(args, info, isClient, isUserID, userId);
    DumpSysStateInner(args, info, isClient, isUserID, userId);
    DumpSysPendingInner(args, info, isClient, isUserID, userId);
    DumpSysProcess(args, info, isClient, isUserID, userId);
}

void AbilityManagerService::DumpSysMissionListInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = missionListManagers_.find(userId);
        if (it == missionListManagers_.end()) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = currentMissionListManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpMissionList(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}
void AbilityManagerService::DumpSysAbilityInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = missionListManagers_.find(userId);
        if (it == missionListManagers_.end()) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = currentMissionListManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() >= MIN_DUMP_ARGUMENT_NUM) {
        HILOG_INFO("argList = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        try {
            auto abilityId = static_cast<int32_t>(std::stoi(argList[1]));
            targetManager->DumpMissionListByRecordId(info, isClient, abilityId, params);
        } catch (...) {
            HILOG_WARN("stoi(%{public}s) failed", argList[1].c_str());
            info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
        }
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    HILOG_INFO("DumpSysStateInner begin:%{public}s", args.c_str());
    std::shared_ptr<AbilityConnectManager> targetManager;

    if (isUserID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = connectManagers_.find(userId);
        if (it == connectManagers_.end()) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = connectManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpState(info, isClient);
    } else {
        HILOG_INFO("uri = %{public}s", argList[1].c_str());
        std::vector<std::string> params(argList.begin() + MIN_DUMP_ARGUMENT_NUM, argList.end());
        targetManager->DumpStateByUri(info, isClient, argList[1], params);
    }
}

void AbilityManagerService::DumpSysPendingInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<PendingWantManager> targetManager;
    if (isUserID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = pendingWantManagers_.find(userId);
        if (it == pendingWantManagers_.end()) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = pendingWantManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpByRecordId(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->Dump(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysProcess(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> ProcessInfos;
    int ret = 0;
    if (isUserID) {
        ret = GetProcessRunningInfosByUserId(ProcessInfos, userId);
    } else {
        ret = GetProcessRunningInfos(ProcessInfos);
    }

    if (ret != ERR_OK || ProcessInfos.size() == 0) {
        return;
    }

    std::string dumpInfo = "  AppRunningRecords:";
    info.push_back(dumpInfo);
    auto processInfoID = 0;
    auto hasProcessName = (argList.size() == MIN_DUMP_ARGUMENT_NUM ? true : false);
    for (const auto& ProcessInfo : ProcessInfos) {
        if (hasProcessName && argList[1] != ProcessInfo.processName_) {
            continue;
        }

        dumpInfo = "    AppRunningRecord ID #" + std::to_string(processInfoID);
        processInfoID++;
        info.push_back(dumpInfo);
        dumpInfo = "      process name [" + ProcessInfo.processName_ + "]";
        info.push_back(dumpInfo);
        dumpInfo = "      pid #" + std::to_string(ProcessInfo.pid_) +
            "  uid #" + std::to_string(ProcessInfo.uid_);
        info.push_back(dumpInfo);
        auto appState = static_cast<AppState>(ProcessInfo.state_);
        dumpInfo = "      state #" + DelayedSingleton<AppScheduler>::GetInstance()->ConvertAppState(appState);
        info.push_back(dumpInfo);
    }
}

void AbilityManagerService::DataDumpSysStateInner(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    std::shared_ptr<DataAbilityManager> targetManager;
    if (isUserID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = dataAbilityManagers_.find(userId);
        if (it == dataAbilityManagers_.end()) {
            info.push_back("error: No user found.");
            return;
        }
        targetManager = it->second;
    } else {
        targetManager = dataAbilityManager_;
    }

    CHECK_POINTER(targetManager);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        targetManager->DumpSysState(info, isClient);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpInner(const std::string &args, std::vector<std::string> &info)
{
    if (currentMissionListManager_) {
        currentMissionListManager_->Dump(info);
    }
}

void AbilityManagerService::DumpMissionListInner(const std::string &args, std::vector<std::string> &info)
{
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionList(info, false, "");
    }
}

void AbilityManagerService::DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info)
{
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionInfos(info);
    }
}

void AbilityManagerService::DumpMissionInner(const std::string &args, std::vector<std::string> &info)
{
    CHECK_POINTER_LOG(currentMissionListManager_, "Current mission manager not init.");
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        info.push_back("error: invalid argument, please see 'ability dump -h'.");
        return;
    }
    int missionId = DEFAULT_INVAL_VALUE;
    (void)StrToInt(argList[1], missionId);
    currentMissionListManager_->DumpMission(missionId, info);
}

void AbilityManagerService::DumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    CHECK_POINTER_LOG(connectManager_, "Current mission manager not init.");
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        connectManager_->DumpState(info, false, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        connectManager_->DumpState(info, false);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DataDumpStateInner(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager_->DumpState(info, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        dataAbilityManager_->DumpState(info);
    } else {
        info.emplace_back("error: invalid argument, please see 'ability dump -h'.");
    }
}

void AbilityManagerService::DumpState(const std::string &args, std::vector<std::string> &info)
{
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto it = dumpMap.find(argList[0]);
    if (it == dumpMap.end()) {
        return;
    }
    DumpKey key = it->second;
    auto itFunc = dumpFuncMap_.find(key);
    if (itFunc != dumpFuncMap_.end()) {
        auto dumpFunc = itFunc->second;
        if (dumpFunc != nullptr) {
            (this->*dumpFunc)(args, info);
            return;
        }
    }
    info.push_back("error: invalid argument, please see 'ability dump -h'.");
}

void AbilityManagerService::DumpSysState(
    const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int userId)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }
    auto it = dumpsysMap.find(argList[0]);
    if (it == dumpsysMap.end()) {
        return;
    }
    DumpsysKey key = it->second;
    auto itFunc = dumpsysFuncMap_.find(key);
    if (itFunc != dumpsysFuncMap_.end()) {
        auto dumpsysFunc = itFunc->second;
        if (dumpsysFunc != nullptr) {
            (this->*dumpsysFunc)(args, info, isClient, isUserID, userId);
            return;
        }
    }
    info.push_back("error: invalid argument, please see 'ability dump -h'.");
}

int AbilityManagerService::AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Ability transition done come, state:%{public}d.", state);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    HILOG_DEBUG("Ability transition done come, state:%{public}d, name:%{public}s", state, abilityInfo.name.c_str());
    auto type = abilityInfo.type;
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    // force timeout ability for test
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    bool isTerminate = abilityRecord->IsAbilityState(AbilityState::TERMINATING) && targetState == AbilityState::INITIAL;
    std::string tempState = isTerminate ? AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING) :
        AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    if (IsNeedTimeoutForTest(abilityInfo.name, tempState)) {
        HILOG_WARN("force timeout ability for test, state:%{public}s, ability: %{public}s",
            tempState.c_str(),
            abilityInfo.name.c_str());
        return ERR_OK;
    }
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->AbilityTransitionDone(token, state);
    }
    if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return dataAbilityManager->AbilityTransitionDone(token, state);
    }
    int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
    if (!missionListManager) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->AbilityTransactionDone(token, state, saveData);
}

int AbilityManagerService::ScheduleConnectAbilityDone(
    const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule connect ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleConnectAbilityDoneLocked(token, remoteObject);
}

int AbilityManagerService::ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule disconnect ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleDisconnectAbilityDoneLocked(token);
}

int AbilityManagerService::ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule command ability done.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    // force timeout ability for test
    if (IsNeedTimeoutForTest(abilityRecord->GetAbilityInfo().name, std::string("COMMAND"))) {
        HILOG_WARN("force timeout ability for test, state:COMMAND, ability: %{public}s",
            abilityRecord->GetAbilityInfo().name.c_str());
        return ERR_OK;
    }
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleCommandAbilityDoneLocked(token);
}

void AbilityManagerService::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    HILOG_INFO("On ability request done, name is %{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;

    auto type = abilityRecord->GetAbilityInfo().type;
    switch (type) {
        case AppExecFwk::AbilityType::SERVICE:
        case AppExecFwk::AbilityType::EXTENSION: {
            auto connectManager = GetConnectManagerByUserId(userId);
            if (!connectManager) {
                HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
                return;
            }
            connectManager->OnAbilityRequestDone(token, state);
            break;
        }
        case AppExecFwk::AbilityType::DATA: {
            auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
            if (!dataAbilityManager) {
                HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
                return;
            }
            dataAbilityManager->OnAbilityRequestDone(token, state);
            break;
        }
        default: {
            int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
            auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
            if (!missionListManager) {
                HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
                return;
            }
            missionListManager->OnAbilityRequestDone(token, state);
            break;
        }
    }
}

void AbilityManagerService::OnAppStateChanged(const AppInfo &info)
{
    HILOG_INFO("On app state changed.");
    CHECK_POINTER_LOG(connectManager_, "Connect manager not init.");
    CHECK_POINTER_LOG(currentMissionListManager_, "Current mission list manager not init.");
    connectManager_->OnAppStateChanged(info);
    currentMissionListManager_->OnAppStateChanged(info);
    dataAbilityManager_->OnAppStateChanged(info);
}

std::shared_ptr<AbilityEventHandler> AbilityManagerService::GetEventHandler()
{
    return handler_;
}

void AbilityManagerService::InitMissionListManager(int userId, bool switchUser)
{
    bool find = false;
    {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto iterator = missionListManagers_.find(userId);
        find = (iterator != missionListManagers_.end());
        if (find) {
            if (switchUser) {
                DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId);
                currentMissionListManager_ = iterator->second;
            }
        }
    }
    if (!find) {
        auto manager = std::make_shared<MissionListManager>(userId);
        manager->Init();
        std::unique_lock<std::shared_mutex> lock(managersMutex_);
        missionListManagers_.emplace(userId, manager);
        if (switchUser) {
            currentMissionListManager_ = manager;
        }
    }
}

// multi user scene
int AbilityManagerService::GetUserId()
{
    if (userController_) {
        auto userId = userController_->GetCurrentUserId();
        HILOG_INFO("%{public}s, userId is %{public}d", __func__, userId);
        return userId;
    }
    return U0_USER_ID;
}

void AbilityManagerService::StartHighestPriorityAbility(int32_t userId, bool isBoot)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    auto func = []() {
        auto client = ContainerManagerClient::GetInstance();
        if (client == nullptr) {
            HILOG_ERROR("%{public}s get ContainerManagerClient null", __func__);
        } else {
            client->NotifyBootComplete(0);
            HILOG_INFO("StartSystemApplication NotifyBootComplete");
        }
    };
    std::thread(func).detach();

    /* Query the highest priority ability or extension ability, and start it. usually, it is OOBE or launcher */
    Want want;
    want.AddEntity(HIGHEST_PRIORITY_ABILITY_ENTITY);
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo;
    int attemptNums = 0;
    while (!IN_PROCESS_CALL(bms->ImplicitQueryInfoByPriority(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT, userId,
        abilityInfo, extensionAbilityInfo))) {
        HILOG_INFO("Waiting query highest priority ability info completed.");
        ++attemptNums;
        if (!isBoot && attemptNums > SWITCH_ACCOUNT_TRY) {
            HILOG_ERROR("Query highest priority ability failed.");
            return;
        }
        AbilityRequest abilityRequest;
        if (!IsComponentInterceptionStart(want, nullptr, 0, 0, abilityRequest)) {
            return;
        }
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    if (abilityInfo.name.empty() && extensionAbilityInfo.name.empty()) {
        HILOG_ERROR("Query highest priority ability failed");
        return;
    }

    Want abilityWant; // donot use 'want' here, because the entity of 'want' is not empty
    if (!abilityInfo.name.empty()) {
        /* highest priority ability */
        HILOG_INFO("Start the highest priority ability. bundleName: %{public}s, ability:%{public}s",
            abilityInfo.bundleName.c_str(), abilityInfo.name.c_str());
        abilityWant.SetElementName(abilityInfo.bundleName, abilityInfo.name);
    } else {
        /* highest priority extension ability */
        HILOG_INFO("Start the highest priority extension ability. bundleName: %{public}s, ability:%{public}s",
            extensionAbilityInfo.bundleName.c_str(), extensionAbilityInfo.name.c_str());
        abilityWant.SetElementName(extensionAbilityInfo.bundleName, extensionAbilityInfo.name);
    }

#ifdef SUPPORT_GRAPHICS
    abilityWant.SetParam(NEED_STARTINGWINDOW, false);
    // wait BOOT_ANIMATION_STARTED to start LAUNCHER
    WaitParameter(BOOTEVENT_BOOT_ANIMATION_STARTED.c_str(), "true", amsConfigResolver_->GetBootAnimationTimeoutTime());
#endif

    /* note: OOBE APP need disable itself, otherwise, it will be started when restart system everytime */
    (void)StartAbility(abilityWant, userId, DEFAULT_INVAL_VALUE);
}

int AbilityManagerService::GenerateAbilityRequest(
    const Want &want, int requestCode, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() != 0 &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        (const_cast<Want &>(want)).SetParam(DLP_INDEX, abilityRecord->GetAppIndex());
    }
    request.want = want;
    request.requestCode = requestCode;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    sptr<IRemoteObject> abilityInfoCallback = want.GetRemoteObject(Want::PARAM_RESV_ABILITY_INFO_CALLBACK);
    if (abilityInfoCallback != nullptr) {
        auto isPerm = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
        if (isPerm) {
            request.abilityInfoCallback = abilityInfoCallback;
        }
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);
#ifdef SUPPORT_GRAPHICS
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, userId, callerToken);
    }
#endif
    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    HILOG_DEBUG("QueryAbilityInfo from bms, userId is %{public}d.", userId);
    int32_t appIndex = want.GetIntParam(DLP_INDEX, 0);
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryAbilityInfo(want, abilityInfoFlag, userId, request.abilityInfo));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxAbilityInfo(want, appIndex,
            abilityInfoFlag, userId, request.abilityInfo));
    }
    if (request.abilityInfo.name.empty() || request.abilityInfo.bundleName.empty()) {
        // try to find extension
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        if (appIndex == 0) {
            IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag, userId, extensionInfos));
        } else {
            IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
                abilityInfoFlag, userId, extensionInfos));
        }
        if (extensionInfos.size() <= 0) {
            HILOG_ERROR("GenerateAbilityRequest error. Get extension info failed.");
            return RESOLVE_ABILITY_ERR;
        }

        AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
        if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
            HILOG_ERROR("extensionInfo empty.");
            return RESOLVE_ABILITY_ERR;
        }
        HILOG_DEBUG("Extension ability info found, name=%{public}s.",
            extensionInfo.name.c_str());
        // For compatibility translates to AbilityInfo
        InitAbilityInfoFromExtension(extensionInfo, request.abilityInfo);
    }
    HILOG_DEBUG("QueryAbilityInfo success, ability name: %{public}s, is stage mode: %{public}d.",
        request.abilityInfo.name.c_str(), request.abilityInfo.isStageBasedModel);
    if (request.abilityInfo.type == AppExecFwk::AbilityType::SERVICE && request.abilityInfo.isStageBasedModel) {
        HILOG_INFO("Stage mode, abilityInfo SERVICE type reset EXTENSION.");
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    }

    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        HILOG_ERROR("Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.uid = request.appInfo.uid;
    HILOG_DEBUG("GenerateAbilityRequest end, app name: %{public}s, bundle name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.appInfo.bundleName.c_str(), request.uid);

    HILOG_INFO("GenerateAbilityRequest, moduleName: %{public}s.", request.abilityInfo.moduleName.c_str());
    request.want.SetModuleName(request.abilityInfo.moduleName);

    if (want.GetBoolParam(Want::PARAM_RESV_START_RECENT, false) &&
        AAFwk::PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        request.startRecent = true;
    }

    return ERR_OK;
}

int AbilityManagerService::GenerateExtensionAbilityRequest(
    const Want &want, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() != 0 &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        (const_cast<Want &>(want)).SetParam(DLP_INDEX, abilityRecord->GetAppIndex());
    }
    request.want = want;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    HILOG_DEBUG("QueryExtensionAbilityInfo from bms, userId is %{public}d.", userId);
    // try to find extension
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    int32_t appIndex = want.GetIntParam(DLP_INDEX, 0);
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag, userId, extensionInfos));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
            abilityInfoFlag, userId, extensionInfos));
    }
    if (extensionInfos.size() <= 0) {
        HILOG_ERROR("GenerateAbilityRequest error. Get extension info failed.");
        return RESOLVE_ABILITY_ERR;
    }

    AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        HILOG_ERROR("extensionInfo empty.");
        return RESOLVE_ABILITY_ERR;
    }
    HILOG_DEBUG("Extension ability info found, name=%{public}s.",
        extensionInfo.name.c_str());
    // For compatibility translates to AbilityInfo
    InitAbilityInfoFromExtension(extensionInfo, request.abilityInfo);

    HILOG_DEBUG("QueryAbilityInfo success, ability name: %{public}s, is stage mode: %{public}d.",
        request.abilityInfo.name.c_str(), request.abilityInfo.isStageBasedModel);

    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        HILOG_ERROR("Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.uid = request.appInfo.uid;
    HILOG_DEBUG("GenerateAbilityRequest end, app name: %{public}s, bundle name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.appInfo.bundleName.c_str(), request.uid);

    HILOG_INFO("GenerateExtensionAbilityRequest, moduleName: %{public}s.", request.abilityInfo.moduleName.c_str());
    request.want.SetModuleName(request.abilityInfo.moduleName);

    return ERR_OK;
}

int AbilityManagerService::TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId)
{
    HILOG_INFO("Terminate ability result, startId: %{public}d", startId);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    int result = JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("target ability is not service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->TerminateAbilityResult(token, startId);
}

int AbilityManagerService::StopServiceAbility(const Want &want, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Stop service ability.");
    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, validUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("validUserId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Target ability is not service type.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (connectManager == nullptr) {
        return ERR_INVALID_VALUE;
    }

    return connectManager->StopServiceAbility(abilityRequest);
}

void AbilityManagerService::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    CHECK_POINTER(abilityRecord);

    auto manager = GetListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
    if (manager && abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
        manager->OnAbilityDied(abilityRecord, GetUserId());
        return;
    }

    auto connectManager = GetConnectManagerByToken(abilityRecord->GetToken());
    if (connectManager) {
        connectManager->OnAbilityDied(abilityRecord, GetUserId());
        return;
    }

    auto dataAbilityManager = GetDataAbilityManagerByToken(abilityRecord->GetToken());
    if (dataAbilityManager) {
        dataAbilityManager->OnAbilityDied(abilityRecord);
    }
}

void AbilityManagerService::OnCallConnectDied(std::shared_ptr<CallRecord> callRecord)
{
    CHECK_POINTER(callRecord);
    if (currentMissionListManager_) {
        currentMissionListManager_->OnCallConnectDied(callRecord);
    }
}

void AbilityManagerService::GetMaxRestartNum(int &max, bool isRootLauncher)
{
    if (amsConfigResolver_) {
        max = amsConfigResolver_->GetMaxRestartNum(isRootLauncher);
    }
}

void AbilityManagerService::GetRestartIntervalTime(int &restartIntervalTime)
{
    if (amsConfigResolver_) {
        restartIntervalTime = amsConfigResolver_->GetRestartIntervalTime();
    }
}

int AbilityManagerService::KillProcess(const std::string &bundleName)
{
    HILOG_DEBUG("Kill process, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, KILL_PROCESS_FAILED);
    int32_t userId = GetUserId();
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        HILOG_ERROR("Failed to get bundle info when kill process.");
        return GET_BUNDLE_INFO_FAILED;
    }

    if (bundleInfo.isKeepAlive) {
        HILOG_ERROR("Can not kill keep alive process.");
        return KILL_PROCESS_KEEP_ALIVE;
    }

    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);
    if (ret != ERR_OK) {
        return KILL_PROCESS_FAILED;
    }
    return ERR_OK;
}

int AbilityManagerService::ClearUpApplicationData(const std::string &bundleName)
{
    HILOG_DEBUG("ClearUpApplicationData, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->ClearUpApplicationData(bundleName);
    if (ret != ERR_OK) {
        return CLEAR_APPLICATION_DATA_FAIL;
    }
    return ERR_OK;
}

int AbilityManagerService::UninstallApp(const std::string &bundleName, int32_t uid)
{
    HILOG_DEBUG("Uninstall app, bundleName: %{public}s, uid=%{public}d", bundleName.c_str(), uid);
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        HILOG_ERROR("%{public}s: Not bundleMgr call.", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    int32_t targetUserId = uid / BASE_USER_RANGE;
    if (targetUserId == U0_USER_ID) {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        for (auto item: missionListManagers_) {
            if (item.second) {
                item.second->UninstallApp(bundleName, uid);
            }
        }
    } else {
        auto listManager = GetListManagerByUserId(targetUserId);
        if (listManager) {
            listManager->UninstallApp(bundleName, uid);
        }
    }

    if (pendingWantManager_) {
        pendingWantManager_->ClearPendingWantRecord(bundleName, uid);
    }
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->KillApplicationByUid(bundleName, uid);
    if (ret != ERR_OK) {
        return UNINSTALL_APP_FAILED;
    }
    return ERR_OK;
}

sptr<AppExecFwk::IBundleMgr> AbilityManagerService::GetBundleManager()
{
    if (iBundleManager_ == nullptr) {
        auto bundleObj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundleObj == nullptr) {
            HILOG_ERROR("Failed to get bundle manager service.");
            return nullptr;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    }
    return iBundleManager_;
}

int AbilityManagerService::PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId)
{
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name when app data abilities preloading.");
        return ERR_INVALID_VALUE;
    }

    auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
    if (dataAbilityManager == nullptr) {
        HILOG_ERROR("Invalid data ability manager when app data abilities preloading.");
        return ERR_INVALID_STATE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, GET_ABILITY_SERVICE_FAILED);

    AppExecFwk::BundleInfo bundleInfo;
    bool ret = IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId));
    if (!ret) {
        HILOG_ERROR("Failed to get bundle info when app data abilities preloading, userId is %{public}d", userId);
        return RESOLVE_APP_ERR;
    }

    HILOG_INFO("App data abilities preloading for bundle '%{public}s'...", bundleName.data());

    auto begin = system_clock::now();
    AbilityRequest dataAbilityRequest;
    dataAbilityRequest.appInfo = bundleInfo.applicationInfo;
    for (auto it = bundleInfo.abilityInfos.begin(); it != bundleInfo.abilityInfos.end(); ++it) {
        if (it->type != AppExecFwk::AbilityType::DATA) {
            continue;
        }
        if ((system_clock::now() - begin) >= DATA_ABILITY_START_TIMEOUT) {
            HILOG_ERROR("App data ability preloading for '%{public}s' timeout.", bundleName.c_str());
            return ERR_TIMED_OUT;
        }
        dataAbilityRequest.abilityInfo = *it;
        dataAbilityRequest.uid = bundleInfo.uid;
        HILOG_INFO("App data ability preloading: '%{public}s.%{public}s'...", it->bundleName.c_str(), it->name.c_str());

        auto dataAbility = dataAbilityManager->Acquire(dataAbilityRequest, false, nullptr, false);
        if (dataAbility == nullptr) {
            HILOG_ERROR(
                "Failed to preload data ability '%{public}s.%{public}s'.", it->bundleName.c_str(), it->name.c_str());
            return ERR_NULL_OBJECT;
        }
    }

    HILOG_INFO("App data abilities preloading done.");

    return ERR_OK;
}

bool AbilityManagerService::IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const
{
    if (info.bundleName != AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return false;
    }
    return (info.name == AbilityConfig::SYSTEM_UI_NAVIGATION_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_STATUS_BAR ||
        info.name == AbilityConfig::SYSTEM_UI_ABILITY_NAME);
}

bool AbilityManagerService::IsSystemUI(const std::string &bundleName) const
{
    return bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME;
}

void AbilityManagerService::HandleLoadTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle load timeout.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, eventId);
        }
    }
}

void AbilityManagerService::HandleActiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle active timeout.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, eventId);
        }
    }
}

void AbilityManagerService::HandleInactiveTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle inactive timeout.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, eventId);
        }
    }

    for (auto& item : connectManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, eventId);
        }
    }
}

void AbilityManagerService::HandleForegroundTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle foreground timeout.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, eventId);
        }
    }
}

void AbilityManagerService::HandleBackgroundTimeOut(int64_t eventId)
{
    HILOG_DEBUG("Handle background timeout.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::BACKGROUND_TIMEOUT_MSG, eventId);
        }
    }
}

bool AbilityManagerService::VerificationToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Verification token.");
    CHECK_POINTER_RETURN_BOOL(dataAbilityManager_);
    CHECK_POINTER_RETURN_BOOL(connectManager_);
    CHECK_POINTER_RETURN_BOOL(currentMissionListManager_);

    if (currentMissionListManager_->GetAbilityRecordByToken(token)) {
        return true;
    }
    if (currentMissionListManager_->GetAbilityFromTerminateList(token)) {
        return true;
    }

    if (dataAbilityManager_->GetAbilityRecordByToken(token)) {
        HILOG_INFO("Verification token4.");
        return true;
    }

    if (connectManager_->GetServiceRecordByToken(token)) {
        HILOG_INFO("Verification token5.");
        return true;
    }

    HILOG_ERROR("Failed to verify token.");
    return false;
}

bool AbilityManagerService::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("VerificationAllToken.");
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchMissionListManagers");
        for (auto item: missionListManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }

            if (item.second && item.second->GetAbilityFromTerminateList(token)) {
                return true;
            }
        }
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchDataAbilityManagers_");
        for (auto item: dataAbilityManagers_) {
            if (item.second && item.second->GetAbilityRecordByToken(token)) {
                return true;
            }
        }
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "VerificationAllToken::SearchConnectManagers_");
        for (auto item: connectManagers_) {
            if (item.second && item.second->GetServiceRecordByToken(token)) {
                return true;
            }
        }
    }
    HILOG_ERROR("Failed to verify all token.");
    return false;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManager(
    const sptr<IAbilityScheduler> &scheduler)
{
    if (scheduler == nullptr) {
        HILOG_ERROR("the param ability scheduler is nullptr");
        return nullptr;
    }

    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->ContainsDataAbility(scheduler)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<MissionListManager> AbilityManagerService::GetListManagerByUserId(int32_t userId)
{
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByUserId(int32_t userId)
{
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    auto it = connectManagers_.find(userId);
    if (it != connectManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByUserId(int32_t userId)
{
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    auto it = dataAbilityManagers_.find(userId);
    if (it != dataAbilityManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByToken(
    const sptr<IRemoteObject> &token)
{
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto item: connectManagers_) {
        if (item.second && item.second->GetServiceRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto item: dataAbilityManagers_) {
        if (item.second && item.second->GetAbilityRecordByToken(token)) {
            return item.second;
        }
    }

    return nullptr;
}

void AbilityManagerService::StartResidentApps()
{
    HILOG_DEBUG("%{public}s", __func__);
    ConnectBmsService();
    auto bms = GetBundleManager();
    CHECK_POINTER_IS_NULLPTR(bms);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfos(OHOS::AppExecFwk::GET_BUNDLE_DEFAULT, bundleInfos, U0_USER_ID))) {
        HILOG_ERROR("Get resident bundleinfos failed");
        return;
    }

    HILOG_INFO("StartResidentApps GetBundleInfos size: %{public}zu", bundleInfos.size());

    DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcessWithMainElement(bundleInfos);
    if (!bundleInfos.empty()) {
#ifdef SUPPORT_GRAPHICS
        WaitParameter(BOOTEVENT_BOOT_ANIMATION_STARTED.c_str(), "true",
            amsConfigResolver_->GetBootAnimationTimeoutTime());
#endif
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

void AbilityManagerService::ConnectBmsService()
{
    HILOG_DEBUG("%{public}s", __func__);
    HILOG_INFO("Waiting AppMgr Service run completed.");
    while (!DelayedSingleton<AppScheduler>::GetInstance()->Init(shared_from_this())) {
        HILOG_ERROR("failed to init AppScheduler");
        usleep(REPOLL_TIME_MICRO_SECONDS);
    }

    HILOG_INFO("Waiting BundleMgr Service run completed.");
    /* wait until connected to bundle manager service */
    while (iBundleManager_ == nullptr) {
        sptr<IRemoteObject> bundle_obj =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (bundle_obj == nullptr) {
            HILOG_ERROR("failed to get bundle manager service");
            usleep(REPOLL_TIME_MICRO_SECONDS);
            continue;
        }
        iBundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(bundle_obj);
    }

    HILOG_INFO("Connect bms success!");
}

int AbilityManagerService::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    HILOG_INFO("Get pending request info.");
    CHECK_POINTER_AND_RETURN(pendingWantManager_, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(target, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(info, ERR_INVALID_VALUE);
    return pendingWantManager_->GetWantSenderInfo(target, info);
}

int AbilityManagerService::GetAppMemorySize()
{
    HILOG_INFO("service GetAppMemorySize start");
    const char *key = "const.product.arkheaplimit";
    const char *def = "512m";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    int resultInt = 0;
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        if (valueGet == nullptr) {
            HILOG_WARN("%{public}s, valueGet is nullptr", __func__);
            return APP_MEMORY_SIZE;
        }
        int len = strlen(valueGet);
        for (int i = 0; i < len; i++) {
            if (valueGet[i] >= '0' && valueGet[i] <= '9') {
                resultInt *= SIZE_10;
                resultInt += valueGet[i] - '0';
            }
        }
        if (resultInt == 0) {
            return APP_MEMORY_SIZE;
        }
        return resultInt;
    }
    return APP_MEMORY_SIZE;
}

bool AbilityManagerService::IsRamConstrainedDevice()
{
    HILOG_INFO("service IsRamConstrainedDevice start");
    const char *key = "const.product.islowram";
    const char *def = "0";
    char *valueGet = nullptr;
    unsigned int len = 128;
    int ret = GetParameter(key, def, valueGet, len);
    if ((ret != GET_PARAMETER_OTHER) && (ret != GET_PARAMETER_INCORRECT)) {
        int value = atoi(valueGet);
        if (value) {
            return true;
        }
        return false;
    }
    return false;
}

int AbilityManagerService::GetMissionSaveTime() const
{
    if (!amsConfigResolver_) {
        return 0;
    }

    return amsConfigResolver_->GetMissionSaveTime();
}

int32_t AbilityManagerService::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is Null.");
        return -1;
    }
    if (!JudgeSelfCalled(abilityRecord)) {
        return -1;
    }
    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(userId);
    if (!missionListManager) {
        HILOG_ERROR("missionListManager is Null. owner mission userId=%{public}d", userId);
        return -1;
    }
    return missionListManager->GetMissionIdByAbilityToken(token);
}

sptr<IRemoteObject> AbilityManagerService::GetAbilityTokenByMissionId(int32_t missionId)
{
    if (!currentMissionListManager_) {
        return nullptr;
    }
    return currentMissionListManager_->GetAbilityTokenByMissionId(missionId);
}

int AbilityManagerService::StartRemoteAbilityByCall(const Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<IRemoteObject> &connect)
{
    HILOG_INFO("%{public}s begin StartRemoteAbilityByCall", __func__);
    Want remoteWant = want;
    if (AddStartControlParam(remoteWant, callerToken) != ERR_OK) {
        HILOG_ERROR("%{public}s AddStartControlParam failed.", __func__);
        return ERR_INVALID_VALUE;
    }
    DistributedClient dmsClient;
    return dmsClient.StartRemoteAbilityByCall(remoteWant, connect);
}

int AbilityManagerService::ReleaseRemoteAbility(const sptr<IRemoteObject> &connect,
    const AppExecFwk::ElementName &element)
{
    DistributedClient dmsClient;
    return dmsClient.ReleaseRemoteAbility(connect, element);
}

int AbilityManagerService::StartAbilityByCall(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("call ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && !JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (CheckIfOperateRemote(want)) {
        HILOG_INFO("start remote ability by call");
        return StartRemoteAbilityByCall(want, callerToken, connect->AsObject());
    }

    int32_t callerUserId = GetValidUserId(DEFAULT_INVAL_VALUE);
    if (!JudgeMultiUserConcurrency(callerUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = want;
    abilityRequest.connect = connect;
    result = GenerateAbilityRequest(want, -1, abilityRequest, callerToken, GetUserId());
    if (!IsComponentInterceptionStart(want, callerToken, 0, result, abilityRequest)) {
        return ERR_OK;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }

    if (!abilityRequest.abilityInfo.isStageBasedModel) {
        HILOG_ERROR("target ability is not stage base model.");
        return RESOLVE_CALL_ABILITY_VERSION_ERR;
    }

    result = CheckStartByCallPermission(abilityRequest);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckStartByCallPermission fail, result: %{public}d", result);
        return result;
    }

    HILOG_DEBUG("abilityInfo.applicationInfo.singleton is %{public}s",
        abilityRequest.abilityInfo.applicationInfo.singleton ? "true" : "false");

    if (!currentMissionListManager_) {
        HILOG_ERROR("currentMissionListManager_ is Null. curentUserId=%{public}d", GetUserId());
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want);
    ReportEventToSuspendManager(abilityRequest.abilityInfo);
    return currentMissionListManager_->ResolveLocked(abilityRequest);
}

int AbilityManagerService::ReleaseCall(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_DEBUG("Release called ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    std::string elementName = element.GetURI();
    HILOG_DEBUG("try to release called ability, name: %{public}s.", elementName.c_str());

    if (CheckIsRemote(element.GetDeviceID())) {
        HILOG_INFO("release remote ability");
        return ReleaseRemoteAbility(connect->AsObject(), element);
    }

    return currentMissionListManager_->ReleaseCallLocked(connect, element);
}

int AbilityManagerService::JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo, int callerUid)
{
    HILOG_DEBUG("Judge ability visible begin.");
    if (!abilityInfo.visible) {
        HILOG_INFO("Ability visible is false.");
        if (callerUid == -1) {
            callerUid = IPCSkeleton::GetCallingUid();
        }
        if (!CheckCallerEligibility(abilityInfo, callerUid)) {
            HILOG_ERROR("called ability has no permission.");
            return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
        }
    }
    HILOG_DEBUG("Judge ability visible success.");
    return ERR_OK;
}

bool AbilityManagerService::CheckCallerEligibility(const AppExecFwk::AbilityInfo &abilityInfo, int callerUid)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        auto bms = GetBundleManager();
        if (!bms) {
            HILOG_ERROR("fail to get bundle manager.");
            return false;
        }

        if (AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
            return true;
        }

        std::string bundleName;
        bool result = IN_PROCESS_CALL(bms->GetBundleNameForUid(callerUid, bundleName));
        if (!result) {
            HILOG_ERROR("GetBundleNameForUid from bms fail.");
            return false;
        }
        AppExecFwk::ApplicationInfo callerAppInfo;
        result = IN_PROCESS_CALL(bms->GetApplicationInfo(bundleName,
            AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
            GetUserId(), callerAppInfo));
        if (!result) {
            HILOG_ERROR("GetApplicationInfo from bms fail.");
            return false;
        }

        auto apl = callerAppInfo.appPrivilegeLevel;
        if (apl != AbilityUtil::SYSTEM_BASIC && apl != AbilityUtil::SYSTEM_CORE) {
            HILOG_DEBUG("caller is normal app.");
            auto callerTokenId = IPCSkeleton::GetCallingTokenID();
            auto targetTokenId = abilityInfo.applicationInfo.accessTokenId;
            if (callerTokenId != targetTokenId) {
                HILOG_ERROR("the bundle name of caller is different from target one, caller: %{public}s "
                            "target: %{public}s",
                    bundleName.c_str(),
                    abilityInfo.bundleName.c_str());
                return false;
            }
        }
    }
    HILOG_DEBUG("Success to check caller permission.");
    return true;
}

int AbilityManagerService::StartUser(int userId)
{
    HILOG_DEBUG("%{public}s, userId:%{public}d", __func__, userId);
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        HILOG_ERROR("%{public}s: Permission verification failed, not account process", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (userController_) {
        return userController_->StartUser(userId, true);
    }
    return 0;
}

int AbilityManagerService::StopUser(int userId, const sptr<IStopUserCallback> &callback)
{
    HILOG_DEBUG("%{public}s", __func__);
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        HILOG_ERROR("%{public}s: Permission verification failed, not account process", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    auto ret = -1;
    if (userController_) {
        ret = userController_->StopUser(userId);
        HILOG_DEBUG("ret = %{public}d", ret);
    }
    if (callback) {
        callback->OnStopUserDone(userId, ret);
    }
    return 0;
}

void AbilityManagerService::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("On accept want response");
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnAcceptWantResponse(want, flag);
}

void AbilityManagerService::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnStartSpecifiedAbilityTimeoutResponse(want);
}

int AbilityManagerService::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    HILOG_DEBUG("Get running ability infos.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!currentMissionListManager_ || !connectManager_ || !dataAbilityManager_) {
        return ERR_INVALID_VALUE;
    }

    currentMissionListManager_->GetAbilityRunningInfos(info, isPerm);
    UpdateFocusState(info);

    return ERR_OK;
}

void AbilityManagerService::UpdateFocusState(std::vector<AbilityRunningInfo> &info)
{
    if (info.empty()) {
        return;
    }

#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        return;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        HILOG_WARN("%{public}s abilityRecord is null.", __func__);
        return;
    }

    for (auto &item : info) {
        if (item.uid == abilityRecord->GetUid() && item.pid == abilityRecord->GetPid() &&
            item.ability == abilityRecord->GetWant().GetElement()) {
            item.abilityState = static_cast<int>(AbilityState::ACTIVE);
            break;
        }
    }
#endif
}

int AbilityManagerService::GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info)
{
    HILOG_DEBUG("Get extension infos, upperLimit : %{public}d", upperLimit);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!connectManager_) {
        return ERR_INVALID_VALUE;
    }

    connectManager_->GetExtensionRunningInfos(upperLimit, info, GetUserId(), isPerm);
    return ERR_OK;
}

int AbilityManagerService::GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfos(info);
}

int AbilityManagerService::GetProcessRunningInfosByUserId(
    std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId)
{
    return DelayedSingleton<AppScheduler>::GetInstance()->GetProcessRunningInfosByUserId(info, userId);
}

void AbilityManagerService::ClearUserData(int32_t userId)
{
    HILOG_DEBUG("%{public}s", __func__);
    std::unique_lock<std::shared_mutex> lock(managersMutex_);
    missionListManagers_.erase(userId);
    connectManagers_.erase(userId);
    dataAbilityManagers_.erase(userId);
    pendingWantManagers_.erase(userId);
}

int AbilityManagerService::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return 0;
    }

    if (!currentMissionListManager_) {
        HILOG_ERROR("snapshot: currentMissionListManager_ is nullptr.");
        return INNER_ERR;
    }
    currentMissionListManager_->RegisterSnapshotHandler(handler);
    HILOG_INFO("snapshot: AbilityManagerService register snapshot handler success.");
    return ERR_OK;
}

int32_t AbilityManagerService::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot, bool isLowResolution)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (CheckIsRemote(deviceId)) {
        HILOG_INFO("get remote mission snapshot.");
        return GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshot);
    }
    HILOG_INFO("get local mission snapshot.");
    if (!currentMissionListManager_) {
        HILOG_ERROR("snapshot: currentMissionListManager_ is nullptr.");
        return INNER_ERR;
    }
    auto token = GetAbilityTokenByMissionId(missionId);
    bool result = currentMissionListManager_->GetMissionSnapshot(missionId, token, missionSnapshot, isLowResolution);
    if (!result) {
        return INNER_ERR;
    }
    return ERR_OK;
}

void AbilityManagerService::UpdateMissionSnapShot(const sptr<IRemoteObject>& token)
{
    CHECK_POINTER_LOG(currentMissionListManager_, "Current mission manager not init.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord && !JudgeSelfCalled(abilityRecord)) {
        return;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        auto bms = GetBundleManager();
        CHECK_POINTER_IS_NULLPTR(bms);
        AppExecFwk::ApplicationInfo appInfo;
        if (!IN_PROCESS_CALL(bms->GetApplicationInfo(BUNDLE_NAME_LAUNCHER,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetUserId(), appInfo))) {
            HILOG_ERROR("Not found GetApplicationInfo according to the bundle name.");
            return;
        }
        auto tokenId = IPCSkeleton::GetCallingTokenID();
        if (tokenId != appInfo.accessTokenId) {
            HILOG_ERROR("%{public}s: Permission verification failed", __func__);
            return;
        }
    }
    currentMissionListManager_->UpdateSnapShot(token);
}

void AbilityManagerService::EnableRecoverAbility(const sptr<IRemoteObject>& token)
{
    if (token == nullptr) {
        return;
    }
    auto record = Token::GetAbilityRecordByToken(token);
    if (record == nullptr) {
        HILOG_ERROR("%{public}s AppRecovery::failed find abilityRecord by given token.", __func__);
        return;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = record->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        HILOG_ERROR("AppRecovery ScheduleRecoverAbility not self, not enabled");
        return;
    }

    auto it = appRecoveryHistory_.find(record->GetUid());
    if (it == appRecoveryHistory_.end()) {
        appRecoveryHistory_.emplace(record->GetUid(), 0);
    }
}

void AbilityManagerService::RecoverAbilityRestart(const Want& want)
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t userId = GetValidUserId(DEFAULT_INVAL_VALUE);
    int32_t ret = StartAbility(want, userId, 0);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s AppRecovery::failed to restart ability.  %{public}d", __func__, ret);
    }
    IPCSkeleton::SetCallingIdentity(identity);
}

void AbilityManagerService::ScheduleRecoverAbility(const sptr<IRemoteObject>& token, int32_t reason)
{
    if (token == nullptr) {
        return;
    }
    auto record = Token::GetAbilityRecordByToken(token);
    if (record == nullptr) {
        HILOG_ERROR("%{public}s AppRecovery::failed find abilityRecord by given token.", __func__);
        return;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = record->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        HILOG_ERROR("AppRecovery ScheduleRecoverAbility not self, not enabled");
        return;
    }

    AAFwk::Want want;
    {
        std::lock_guard<std::recursive_mutex> guard(globalLock_);
        auto type = record->GetAbilityInfo().type;
        if (type != AppExecFwk::AbilityType::PAGE) {
            HILOG_ERROR("%{public}s AppRecovery::only do recover for page ability.", __func__);
            return;
        }

        constexpr int64_t MIN_RECOVERY_TIME = 60;
        int64_t now = time(nullptr);
        auto it = appRecoveryHistory_.find(record->GetUid());
        if ((it != appRecoveryHistory_.end()) &&
            (it->second + MIN_RECOVERY_TIME > now)) {
            HILOG_ERROR("%{public}s AppRecovery recover app more than once in one minute, just kill app(%{public}d).",
                __func__, record->GetPid());
            kill(record->GetPid(), SIGKILL);
            return;
        }

        auto appInfo = record->GetApplicationInfo();
        auto abilityInfo = record->GetAbilityInfo();
        appRecoveryHistory_[record->GetUid()] = now;
        want = record->GetWant();
        want.SetParam(AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART, true);

        HiSysEventWrite(HiSysEvent::Domain::AAFWK, "APP_RECOVERY", HiSysEvent::EventType::BEHAVIOR,
            "APP_UID", record->GetUid(),
            "VERSION_CODE", std::to_string(appInfo.versionCode),
            "VERSION_NAME", appInfo.versionName,
            "BUNDLE_NAME", appInfo.bundleName,
            "ABILITY_NAME", abilityInfo.name);
        kill(record->GetPid(), SIGKILL);
    }

    constexpr int delaytime = 2000;
    std::string taskName = "AppRecovery_kill:" + std::to_string(record->GetPid());
    auto task = std::bind(&AbilityManagerService::RecoverAbilityRestart, this, want);
    handler_->PostTask(task, taskName, delaytime);
}

int32_t AbilityManagerService::GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& missionSnapshot)
{
    HILOG_INFO("GetRemoteMissionSnapshotInfo begin");
    std::unique_ptr<MissionSnapshot> missionSnapshotPtr = std::make_unique<MissionSnapshot>();
    DistributedClient dmsClient;
    int result = dmsClient.GetRemoteMissionSnapshotInfo(deviceId, missionId, missionSnapshotPtr);
    if (result != ERR_OK) {
        HILOG_ERROR("GetRemoteMissionSnapshotInfo failed, result = %{public}d", result);
        return result;
    }
    missionSnapshot = *missionSnapshotPtr;
    return ERR_OK;
}

void AbilityManagerService::StartFreezingScreen()
{
    HILOG_INFO("%{public}s", __func__);
#ifdef SUPPORT_GRAPHICS
    std::vector<Rosen::DisplayId> displayIds = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    IN_PROCESS_CALL_WITHOUT_RET(Rosen::DisplayManager::GetInstance().Freeze(displayIds));
#endif
}

void AbilityManagerService::StopFreezingScreen()
{
    HILOG_INFO("%{public}s", __func__);
#ifdef SUPPORT_GRAPHICS
    std::vector<Rosen::DisplayId> displayIds = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    IN_PROCESS_CALL_WITHOUT_RET(Rosen::DisplayManager::GetInstance().Unfreeze(displayIds));
#endif
}

void AbilityManagerService::UserStarted(int32_t userId)
{
    HILOG_INFO("%{public}s", __func__);
    InitConnectManager(userId, false);
    InitMissionListManager(userId, false);
    InitDataAbilityManager(userId, false);
    InitPendWantManager(userId, false);
}

void AbilityManagerService::SwitchToUser(int32_t oldUserId, int32_t userId)
{
    HILOG_INFO("%{public}s, oldUserId:%{public}d, newUserId:%{public}d", __func__, oldUserId, userId);
    SwitchManagers(userId);
    PauseOldUser(oldUserId);
    bool isBoot = false;
    if (oldUserId == U0_USER_ID) {
        isBoot = true;
    }
    ConnectBmsService();
    StartUserApps(userId, isBoot);
    PauseOldConnectManager(oldUserId);
}

void AbilityManagerService::SwitchManagers(int32_t userId, bool switchUser)
{
    HILOG_INFO("%{public}s, SwitchManagers:%{public}d-----begin", __func__, userId);
    InitConnectManager(userId, switchUser);
    if (userId != U0_USER_ID) {
        InitMissionListManager(userId, switchUser);
    }
    InitDataAbilityManager(userId, switchUser);
    InitPendWantManager(userId, switchUser);
    HILOG_INFO("%{public}s, SwitchManagers:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldUser(int32_t userId)
{
    HILOG_INFO("%{public}s, PauseOldUser:%{public}d-----begin", __func__, userId);
    PauseOldMissionListManager(userId);
    HILOG_INFO("%{public}s, PauseOldUser:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldMissionListManager(int32_t userId)
{
    HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----begin", __func__, userId);
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it == missionListManagers_.end()) {
        HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end1", __func__, userId);
        return;
    }
    auto manager = it->second;
    if (!manager) {
        HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end2", __func__, userId);
        return;
    }
    manager->PauseManager();
    HILOG_INFO("%{public}s, PauseOldMissionListManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::PauseOldConnectManager(int32_t userId)
{
    HILOG_INFO("%{public}s, PauseOldConnectManager:%{public}d-----begin", __func__, userId);
    if (userId == U0_USER_ID) {
        HILOG_INFO("%{public}s, u0 not stop, id:%{public}d-----nullptr", __func__, userId);
        return;
    }

    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    auto it = connectManagers_.find(userId);
    if (it == connectManagers_.end()) {
        HILOG_INFO("%{public}s, PauseOldConnectManager:%{public}d-----no user", __func__, userId);
        return;
    }
    auto manager = it->second;
    if (!manager) {
        HILOG_INFO("%{public}s, PauseOldConnectManager:%{public}d-----nullptr", __func__, userId);
        return;
    }
    manager->StopAllExtensions();
    HILOG_INFO("%{public}s, PauseOldConnectManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::StartUserApps(int32_t userId, bool isBoot)
{
    HILOG_INFO("StartUserApps, userId:%{public}d, currentUserId:%{public}d", userId, GetUserId());
    if (currentMissionListManager_ && currentMissionListManager_->IsStarted()) {
        HILOG_INFO("missionListManager ResumeManager");
        currentMissionListManager_->ResumeManager();
    }
    StartHighestPriorityAbility(userId, isBoot);
}

void AbilityManagerService::InitConnectManager(int32_t userId, bool switchUser)
{
    bool find = false;
    {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = connectManagers_.find(userId);
        find = (it != connectManagers_.end());
        if (find) {
            if (switchUser) {
                connectManager_ = it->second;
            }
        }
    }
    if (!find) {
        auto manager = std::make_shared<AbilityConnectManager>(userId);
        manager->SetEventHandler(handler_);
        std::unique_lock<std::shared_mutex> lock(managersMutex_);
        connectManagers_.emplace(userId, manager);
        if (switchUser) {
            connectManager_ = manager;
        }
    }
}

void AbilityManagerService::InitDataAbilityManager(int32_t userId, bool switchUser)
{
    bool find = false;
    {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = dataAbilityManagers_.find(userId);
        find = (it != dataAbilityManagers_.end());
        if (find) {
            if (switchUser) {
                dataAbilityManager_ = it->second;
            }
        }
    }
    if (!find) {
        auto manager = std::make_shared<DataAbilityManager>();
        std::unique_lock<std::shared_mutex> lock(managersMutex_);
        dataAbilityManagers_.emplace(userId, manager);
        if (switchUser) {
            dataAbilityManager_ = manager;
        }
    }
}

void AbilityManagerService::InitPendWantManager(int32_t userId, bool switchUser)
{
    bool find = false;
    {
        std::shared_lock<std::shared_mutex> lock(managersMutex_);
        auto it = pendingWantManagers_.find(userId);
        find = (it != pendingWantManagers_.end());
        if (find) {
            if (switchUser) {
                pendingWantManager_ = it->second;
            }
        }
    }
    if (!find) {
        auto manager = std::make_shared<PendingWantManager>();
        std::unique_lock<std::shared_mutex> lock(managersMutex_);
        pendingWantManagers_.emplace(userId, manager);
        if (switchUser) {
            pendingWantManager_ = manager;
        }
    }
}

int32_t AbilityManagerService::GetValidUserId(const int32_t userId)
{
    HILOG_DEBUG("%{public}s, userId = %{public}d.", __func__, userId);
    int32_t validUserId = userId;

    if (DEFAULT_INVAL_VALUE == userId) {
        validUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        HILOG_INFO("%{public}s, validUserId = %{public}d, CallingUid = %{public}d.", __func__, validUserId,
            IPCSkeleton::GetCallingUid());
        if (validUserId == U0_USER_ID) {
            validUserId = GetUserId();
        }
    }
    return validUserId;
}

int AbilityManagerService::SetAbilityController(const sptr<IAbilityController> &abilityController,
    bool imAStabilityTest)
{
    HILOG_DEBUG("%{public}s, imAStabilityTest: %{public}d", __func__, imAStabilityTest);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyControllerPerm();
    if (!isPerm) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    abilityController_ = abilityController;
    controllerIsAStabilityTest_ = imAStabilityTest;
    HILOG_DEBUG("%{public}s, end", __func__);
    return ERR_OK;
}

int AbilityManagerService::SendANRProcessID(int pid)
{
    HILOG_INFO("SendANRProcessID come, pid is %{public}d", pid);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    AppExecFwk::ApplicationInfo appInfo;
    bool debug;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler->GetApplicationInfoByProcessID(pid, appInfo, debug) == ERR_OK) {
        auto it = appRecoveryHistory_.find(appInfo.uid);
        if (it != appRecoveryHistory_.end()) {
            return ERR_OK;
        }
    }

    if (debug) {
        HILOG_ERROR("SendANRProcessID error, debug mode.");
        return ERR_INVALID_VALUE;
    }

    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (!sysDialog) {
        HILOG_ERROR("SystemDialogScheduler is nullptr.");
        return ERR_INVALID_VALUE;
    }

    Want want;
    if (!sysDialog->GetANRDialogWant(GetUserId(), pid, want)) {
        HILOG_ERROR("GetANRDialogWant failed.");
        return ERR_INVALID_VALUE;
    }
    return StartAbility(want);
}

bool AbilityManagerService::IsRunningInStabilityTest()
{
    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    bool ret = abilityController_ != nullptr && controllerIsAStabilityTest_;
    HILOG_DEBUG("%{public}s, IsRunningInStabilityTest: %{public}d", __func__, ret);
    return ret;
}

bool AbilityManagerService::IsAbilityControllerStart(const Want &want, const std::string &bundleName)
{
    HILOG_DEBUG("method call, controllerIsAStabilityTest_: %{public}d", controllerIsAStabilityTest_);
    if (abilityController_ == nullptr) {
        HILOG_DEBUG("abilityController_ is nullptr");
        return true;
    }

    if (controllerIsAStabilityTest_) {
        bool isStart = abilityController_->AllowAbilityStart(want, bundleName);
        if (!isStart) {
            HILOG_INFO("Not finishing start ability because controller starting: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

bool AbilityManagerService::IsAbilityControllerForeground(const std::string &bundleName)
{
    HILOG_DEBUG("method call, controllerIsAStabilityTest_: %{public}d", controllerIsAStabilityTest_);
    if (abilityController_ == nullptr) {
        HILOG_DEBUG("abilityController_ is nullptr");
        return true;
    }

    if (controllerIsAStabilityTest_) {
        bool isResume = abilityController_->AllowAbilityBackground(bundleName);
        if (!isResume) {
            HILOG_INFO("Not finishing terminate ability because controller resuming: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

int32_t AbilityManagerService::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionInfo.applicationInfo;
    abilityInfo.bundleName = extensionInfo.bundleName;
    abilityInfo.package = extensionInfo.moduleName;
    abilityInfo.moduleName = extensionInfo.moduleName;
    abilityInfo.name = extensionInfo.name;
    abilityInfo.srcEntrance = extensionInfo.srcEntrance;
    abilityInfo.srcPath = extensionInfo.srcEntrance;
    abilityInfo.iconPath = extensionInfo.icon;
    abilityInfo.iconId = extensionInfo.iconId;
    abilityInfo.label = extensionInfo.label;
    abilityInfo.labelId = extensionInfo.labelId;
    abilityInfo.description = extensionInfo.description;
    abilityInfo.descriptionId = extensionInfo.descriptionId;
    abilityInfo.priority = extensionInfo.priority;
    abilityInfo.permissions = extensionInfo.permissions;
    abilityInfo.readPermission = extensionInfo.readPermission;
    abilityInfo.writePermission = extensionInfo.writePermission;
    abilityInfo.uri = extensionInfo.uri;
    abilityInfo.extensionAbilityType = extensionInfo.type;
    abilityInfo.visible = extensionInfo.visible;
    abilityInfo.resourcePath = extensionInfo.resourcePath;
    abilityInfo.enabled = extensionInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionInfo.process;
    abilityInfo.metadata = extensionInfo.metadata;
    abilityInfo.compileMode = extensionInfo.compileMode;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    if (!extensionInfo.hapPath.empty()) {
        abilityInfo.hapPath = extensionInfo.hapPath;
    }
    return 0;
}

int AbilityManagerService::StartUserTest(const Want &want, const sptr<IRemoteObject> &observer)
{
    HILOG_DEBUG("enter");
    if (observer == nullptr) {
        HILOG_ERROR("observer is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-b");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, START_USER_TEST_FAIL);
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, U0_USER_ID))) {
        HILOG_ERROR("Failed to get bundle info by U0_USER_ID %{public}d.", U0_USER_ID);
        int32_t userId = GetUserId();
        if (!IN_PROCESS_CALL(
            bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
            HILOG_ERROR("Failed to get bundle info by userId %{public}d.", userId);
            return GET_BUNDLE_INFO_FAILED;
        }
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->StartUserTest(want, observer, bundleInfo, GetUserId());
}

int AbilityManagerService::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    HILOG_DEBUG("enter");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->FinishUserTest(msg, resultCode, bundleName);
}

int AbilityManagerService::GetTopAbility(sptr<IRemoteObject> &token)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("Permission verification failed");
        return CHECK_PERMISSION_FAILED;
    }
#ifdef SUPPORT_GRAPHICS
    if (!wmsHandler_) {
        HILOG_ERROR("wmsHandler_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    wmsHandler_->GetFocusWindow(token);

    if (!token) {
        HILOG_ERROR("token is nullptr");
        return ERR_INVALID_VALUE;
    }
#endif
    return ERR_OK;
}

int AbilityManagerService::DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId < 0) {
        HILOG_ERROR("Invalid mission id.");
        return ERR_INVALID_VALUE;
    }

    NotifyHandleMoveAbility(token, 0);
    return DelegatorMoveMissionToFront(missionId);
}

int AbilityManagerService::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");
    NotifyHandleMoveAbility(token, 1);
    return MinimizeAbility(token, true);
}

int AbilityManagerService::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    HILOG_DEBUG("DoAbilityForeground, sceneFlag:%{public}u", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);
    if (!VerificationToken(token) && !VerificationAllToken(token)) {
        HILOG_ERROR("%{public}s token error.", __func__);
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    int result = JudgeAbilityVisibleControl(abilityRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("%{public}s JudgeAbilityVisibleControl error.", __func__);
        return result;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Cannot minimize except page ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        HILOG_ERROR("IsAbilityControllerForeground false.");
        return ERR_WOULD_BLOCK;
    }

    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("pending state is FOREGROUND.");
        abilityRecord->SetPendingState(AbilityState::FOREGROUND);
        return ERR_OK;
    } else {
        HILOG_DEBUG("pending state is not FOREGROUND.");
        abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }
    abilityRecord->ProcessForegroundAbility(flag);
    return ERR_OK;
}

int AbilityManagerService::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    HILOG_DEBUG("DoAbilityBackground, sceneFlag:%{public}u", flag);
    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->lifeCycleStateInfo_.sceneFlag = flag;
    int ret = MinimizeAbility(token);
    abilityRecord->lifeCycleStateInfo_.sceneFlag = SCENE_FLAG_NORMAL;
    return ret;
}

int AbilityManagerService::DelegatorMoveMissionToFront(int32_t missionId)
{
    HILOG_INFO("enter missionId : %{public}d", missionId);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    if (!IsAbilityControllerStartById(missionId)) {
        HILOG_ERROR("IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    return currentMissionListManager_->MoveMissionToFront(missionId);
}

void AbilityManagerService::UpdateCallerInfo(Want& want)
{
    int32_t tokenId = (int32_t)IPCSkeleton::GetCallingTokenID();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);
}

bool AbilityManagerService::JudgeMultiUserConcurrency(const int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (userId == U0_USER_ID) {
        HILOG_DEBUG("%{public}s, userId is 0.", __func__);
        return true;
    }

    HILOG_DEBUG("userId : %{public}d, current userId : %{public}d", userId, GetUserId());

    // Only non-concurrent mode is supported
    bool concurrencyMode = CONCURRENCY_MODE_FALSE;
    if (!concurrencyMode) {
        return (userId == GetUserId());
    }

    return true;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerService::ForceTimeoutForTest(const std::string &abilityName, const std::string &state)
{
    if (abilityName.empty()) {
        HILOG_ERROR("abilityName is empty.");
        return INVALID_DATA;
    }
    if (abilityName == "clean") {
        timeoutMap_.clear();
        return ERR_OK;
    }
    if (state != AbilityRecord::ConvertAbilityState(AbilityState::INITIAL) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::INACTIVE) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::FOREGROUND) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::BACKGROUND) &&
        state != AbilityRecord::ConvertAbilityState(AbilityState::TERMINATING) &&
        state != std::string("COMMAND")) {
        HILOG_ERROR("lifecycle state is invalid.");
        return INVALID_DATA;
    }
    timeoutMap_.insert(std::make_pair(state, abilityName));
    return ERR_OK;
}
#endif

int AbilityManagerService::CheckStaticCfgPermission(AppExecFwk::AbilityInfo &abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        // do not need check static config permission when start ability by SA
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (abilityInfo.applicationInfo.accessTokenId == tokenId) {
        return ERR_OK;
    }

    if ((abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) ||
        (abilityInfo.type == AppExecFwk::AbilityType::DATA)) {
        // just need check the read permission and write permission of extension ability or data ability
        if (!abilityInfo.readPermission.empty()) {
            int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.readPermission);
            if (checkReadPermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            HILOG_WARN("verify access token fail, read permission: %{public}s", abilityInfo.readPermission.c_str());
        }
        if (!abilityInfo.writePermission.empty()) {
            int checkWritePermission = AccessTokenKit::VerifyAccessToken(tokenId, abilityInfo.writePermission);
            if (checkWritePermission == ERR_OK) {
                return AppExecFwk::Constants::PERMISSION_GRANTED;
            }
            HILOG_WARN("verify access token fail, write permission: %{public}s", abilityInfo.writePermission.c_str());
        }

        if (!abilityInfo.readPermission.empty() || !abilityInfo.writePermission.empty()) {
            // 'readPermission' and 'writePermission' take precedence over 'permission'
            // when 'readPermission' or 'writePermission' is not empty, no need check 'permission'
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    // verify permission if 'permission' is not empty
    if (abilityInfo.permissions.empty() ||
        AccessTokenKit::VerifyAccessToken(tokenId, PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY) == ERR_OK) {
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    for (auto permission : abilityInfo.permissions) {
        if (AccessTokenKit::VerifyAccessToken(tokenId, permission)
            != AppExecFwk::Constants::PERMISSION_GRANTED) {
            HILOG_ERROR("verify access token fail, permission: %{public}s", permission.c_str());
            return AppExecFwk::Constants::PERMISSION_NOT_GRANTED;
        }
    }

    return AppExecFwk::Constants::PERMISSION_GRANTED;
}

bool AbilityManagerService::IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const
{
    for (auto iter = timeoutMap_.begin(); iter != timeoutMap_.end(); iter++) {
        if (iter->first == state && iter->second == abilityName) {
            return true;
        }
    }
    return false;
}

bool AbilityManagerService::GetValidDataAbilityUri(const std::string &abilityInfoUri, std::string &adjustUri)
{
    // note: do not use abilityInfo.uri directly, need check uri first.
    size_t firstSeparator = abilityInfoUri.find_first_of('/');
    size_t lastSeparator = abilityInfoUri.find_last_of('/');
    if (lastSeparator - firstSeparator != 1) {
        HILOG_ERROR("ability info uri error, uri: %{public}s", abilityInfoUri.c_str());
        return false;
    }

    adjustUri = abilityInfoUri;
    adjustUri.insert(lastSeparator, "/");
    return true;
}

bool AbilityManagerService::GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
    const std::string &mainAbility, std::string &uri)
{
    if (abilityInfos.empty() || mainAbility.empty()) {
        HILOG_ERROR("abilityInfos or mainAbility is empty. mainAbility: %{public}s", mainAbility.c_str());
        return false;
    }

    std::string dataAbilityUri;
    for (auto abilityInfo : abilityInfos) {
        if (abilityInfo.type == AppExecFwk::AbilityType::DATA &&
            abilityInfo.name == mainAbility) {
            dataAbilityUri = abilityInfo.uri;
            HILOG_INFO("get data ability uri: %{public}s", dataAbilityUri.c_str());
            break;
        }
    }

    return GetValidDataAbilityUri(dataAbilityUri, uri);
}

void AbilityManagerService::GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info,
    std::shared_ptr<AbilityRecord> &abilityRecord)
{
    AbilityRunningInfo runningInfo;
    AppExecFwk::RunningProcessInfo processInfo;

    runningInfo.ability = abilityRecord->GetWant().GetElement();
    runningInfo.startTime = abilityRecord->GetStartTime();
    runningInfo.abilityState = static_cast<int>(abilityRecord->GetAbilityState());

    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    runningInfo.pid = processInfo.pid_;
    runningInfo.uid = processInfo.uid_;
    runningInfo.processName = processInfo.processName_;
    info.emplace_back(runningInfo);
}

int AbilityManagerService::VerifyAccountPermission(int32_t userId)
{
    if ((userId < 0) || (userController_ && (userController_->GetCurrentUserId() == userId))) {
        return ERR_OK;
    }
    return AAFwk::PermissionVerification::GetInstance()->VerifyAccountPermission();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerService::BlockAmsService()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (handler_) {
        HILOG_DEBUG("%{public}s begin post block ams service task", __func__);
        auto BlockAmsServiceTask = [aams = shared_from_this()]() {
            while (1) {
                HILOG_DEBUG("%{public}s begin waiting", __func__);
                std::this_thread::sleep_for(BLOCK_AMS_SERVICE_TIME*1s);
            }
        };
        handler_->PostTask(BlockAmsServiceTask, "blockamsservice");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}

int AbilityManagerService::BlockAbility(int32_t abilityRecordId)
{
    HILOG_DEBUG("%{public}s", __func__);
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    return currentMissionListManager_->BlockAbility(abilityRecordId);
}

int AbilityManagerService::BlockAppService()
{
    HILOG_DEBUG("%{public}s", __func__);
    return DelayedSingleton<AppScheduler>::GetInstance()->BlockAppService();
}
#endif

int AbilityManagerService::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    int32_t validUserId = GetValidUserId(userId);
    if (freeInstallManager_ == nullptr) {
        HILOG_ERROR("freeInstallManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return freeInstallManager_->FreeInstallAbilityFromRemote(want, callback, validUserId, requestCode);
}

AppExecFwk::ElementName AbilityManagerService::GetTopAbility()
{
    HILOG_DEBUG("%{public}s start.", __func__);
    AppExecFwk::ElementName elementName = {};
#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret) {
        return elementName;
    }
    if (!token) {
        HILOG_ERROR("token is nullptr");
        return elementName;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("%{public}s abilityRecord is null.", __func__);
        return elementName;
    }
    elementName = abilityRecord->GetWant().GetElement();
    bool isDeviceEmpty = elementName.GetDeviceID().empty();
    std::string localDeviceId;
    bool hasLocalDeviceId = GetLocalDeviceId(localDeviceId);
    if (isDeviceEmpty && hasLocalDeviceId) {
        elementName.SetDeviceID(localDeviceId);
    }
#endif
    return elementName;
}

int AbilityManagerService::Dump(int fd, const std::vector<std::u16string>& args)
{
    HILOG_DEBUG("Dump begin fd: %{public}d", fd);
    std::string result;
    auto errCode = Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        HILOG_ERROR("dprintf error");
        return ERR_AAFWK_HIDUMP_ERROR;
    }
    HILOG_DEBUG("Dump end");
    return errCode;
}

int AbilityManagerService::Dump(const std::vector<std::u16string>& args, std::string& result)
{
    ErrCode errCode = ERR_OK;
    auto size = args.size();
    if (size == 0) {
        ShowHelp(result);
        return errCode;
    }

    std::vector<std::string> argsStr;
    for (auto arg : args) {
        argsStr.emplace_back(Str16ToStr8(arg));
    }

    if (argsStr[0] == "-h") {
        ShowHelp(result);
    } else {
        errCode = ProcessMultiParam(argsStr, result);
        if (errCode == ERR_AAFWK_HIDUMP_INVALID_ARGS) {
            ShowIllegalInfomation(result);
        }
    }
    return errCode;
}

ErrCode AbilityManagerService::ProcessMultiParam(std::vector<std::string>& argsStr, std::string& result)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    bool isClient = false;
    bool isUser = false;
    int userID = DEFAULT_INVAL_VALUE;
    std::vector<std::string>::iterator it;
    for (it = argsStr.begin(); it != argsStr.end();) {
        if (*it == ARGS_CLIENT) {
            isClient = true;
            it = argsStr.erase(it);
            continue;
        }
        if (*it == ARGS_USER_ID) {
            it = argsStr.erase(it);
            if (it == argsStr.end()) {
                HILOG_ERROR("ARGS_USER_ID id invalid");
                return ERR_AAFWK_HIDUMP_INVALID_ARGS;
            }
            (void)StrToInt(*it, userID);
            if (userID < 0) {
                HILOG_ERROR("ARGS_USER_ID id invalid");
                return ERR_AAFWK_HIDUMP_INVALID_ARGS;
            }
            isUser = true;
            it = argsStr.erase(it);
            continue;
        }
        it++;
    }
    std::string cmd;
    for (unsigned int i = 0; i < argsStr.size(); i++) {
        cmd.append(argsStr[i]);
        if (i != argsStr.size() - 1) {
            cmd.append(" ");
        }
    }
    HILOG_INFO("%{public}s, isClient:%{public}d, userID is : %{public}d, cmd is : %{public}s",
        __func__, isClient, userID, cmd.c_str());

    std::vector<std::string> dumpResults;
    DumpSysState(cmd, dumpResults, isClient, isUser, userID);
    for (auto it : dumpResults) {
        result += it + "\n";
    }
    return ERR_OK;
}

void AbilityManagerService::ShowHelp(std::string& result)
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n")
        .append("-a [-c | -u {UserId}]       ")
        .append("dump all ability infomation in the system or all ability infomation of client/UserId\n")
        .append("-l                          ")
        .append("dump all mission list information in the system\n")
        .append("-i {AbilityRecordId}        ")
        .append("dump an ability infomation by ability record id\n")
        .append("-e                          ")
        .append("dump all extension infomation in the system(FA: ServiceAbilityRecords, Stage: ExtensionRecords)\n")
        .append("-p [PendingWantRecordId]    ")
        .append("dump all pendingwant record infomation in the system\n")
        .append("-r                          ")
        .append("dump all process in the system\n")
        .append("-d                          ")
        .append("dump all data ability infomation in the system");
}

void AbilityManagerService::ShowIllegalInfomation(std::string& result)
{
    result.append(ILLEGAL_INFOMATION);
}

int AbilityManagerService::DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("DumpAbilityInfoDone begin");
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord nullptr");
        return ERR_INVALID_VALUE;
    }
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    abilityRecord->DumpAbilityInfoDone(infos);
    return ERR_OK;
}

#ifdef SUPPORT_GRAPHICS
int AbilityManagerService::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        HILOG_ERROR("SetMissionLabel not self, not enabled");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(userId);
    if (!missionListManager) {
        HILOG_ERROR("failed to find mission list manager when set mission label.");
        return -1;
    }

    return missionListManager->SetMissionLabel(token, label);
}

int AbilityManagerService::SetMissionIcon(const sptr<IRemoteObject> &token,
    const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    HILOG_DEBUG("%{public}s", __func__);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("no such ability record");
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        HILOG_ERROR("not self, not enable to set mission icon");
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(userId);
    if (!missionListManager) {
        HILOG_ERROR("failed to find mission list manager.");
        return -1;
    }

    return missionListManager->SetMissionIcon(token, icon);
}

int AbilityManagerService::RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler> &handler)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isGatewayCall = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
    if (!isSaCall && !isGatewayCall) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    wmsHandler_ = handler;
    HILOG_DEBUG("%{public}s: WMS handler registered successfully.", __func__);
    return ERR_OK;
}

sptr<IWindowManagerServiceHandler> AbilityManagerService::GetWMSHandler() const
{
    return wmsHandler_;
}

void AbilityManagerService::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::shared_lock<std::shared_mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->CompleteFirstFrameDrawing(abilityToken);
        }
    }
}

int32_t AbilityManagerService::ShowPickerDialog(
    const Want& want, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    AAFwk::Want newWant = want;
    constexpr char PICKER_DIALOG_ABILITY_BUNDLE_NAME[] = "com.ohos.sharepickerdialog";
    constexpr char PICKER_DIALOG_ABILITY_NAME[] = "PickerDialog";
    constexpr char TOKEN_KEY[] = "ohos.ability.params.token";
    newWant.SetElementName(PICKER_DIALOG_ABILITY_BUNDLE_NAME, PICKER_DIALOG_ABILITY_NAME);
    newWant.SetParam(TOKEN_KEY, callerToken);
    // note: clear actions
    newWant.SetAction("");
    return IN_PROCESS_CALL(StartAbility(newWant, DEFAULT_INVAL_VALUE, userId));
}
#endif

bool AbilityManagerService::CheckWindowMode(int32_t windowMode,
    const std::vector<AppExecFwk::SupportWindowMode>& windowModes) const
{
    HILOG_INFO("Window mode is %{public}d.", windowMode);
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED) {
        return true;
    }
    auto it = windowModeMap.find(windowMode);
    if (it != windowModeMap.end()) {
        auto bmsWindowMode = it->second;
        for (auto mode : windowModes) {
            if (mode == bmsWindowMode) {
                return true;
            }
        }
    }
    return false;
}

int AbilityManagerService::CheckCallServicePermission(const AbilityRequest &abilityRequest)
{
    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    if (abilityRequest.abilityInfo.isStageBasedModel) {
        auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
        HILOG_INFO("extensionType is %{public}d.", static_cast<int>(extensionType));
        if (extensionType == AppExecFwk::ExtensionAbilityType::SERVICE ||
            extensionType == AppExecFwk::ExtensionAbilityType::DATASHARE) {
            return CheckCallServiceExtensionPermission(abilityRequest);
        } else {
            return CheckCallOtherExtensionPermission(abilityRequest);
        }
    } else {
        return CheckCallServiceAbilityPermission(abilityRequest);
    }
}

int AbilityManagerService::CheckCallDataAbilityPermission(AbilityRequest &abilityRequest)
{
    HILOG_INFO("%{public}s begin", __func__);
    abilityRequest.appInfo = abilityRequest.abilityInfo.applicationInfo;
    abilityRequest.uid = abilityRequest.appInfo.uid;
    if (abilityRequest.appInfo.name.empty() || abilityRequest.appInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid app info for data ability acquiring.");
        return ERR_INVALID_VALUE;
    }
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("BMS query result is not a data ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest);
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start DataAbility");
        return result;
    }

    return ERR_OK;
}

AAFwk::PermissionVerification::VerificationInfo AbilityManagerService::CreateVerificationInfo(
    const AbilityRequest &abilityRequest)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    HILOG_DEBUG("Call ServiceAbility or DataAbility, target bundleName: %{public}s.",
        abilityRequest.appInfo.bundleName.c_str());
    if (whiteListassociatedWakeUpFlag_ &&
        WHITE_LIST_ASS_WAKEUP_SET.find(abilityRequest.appInfo.bundleName) != WHITE_LIST_ASS_WAKEUP_SET.end()) {
        HILOG_DEBUG("Call ServiceAbility or DataAbility, target bundle in white-list, allow associatedWakeUp.");
        verificationInfo.associatedWakeUp = true;
    } else {
        verificationInfo.associatedWakeUp = abilityRequest.appInfo.bundleName == BUNDLE_NAME_SETTINGSDATA ?
                                            true : abilityRequest.appInfo.associatedWakeUp;
    }
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_INFO("Caller is not an application.");
        return verificationInfo;
    }
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        verificationInfo.apiTargetVersion = callerAbility->GetApplicationInfo().apiTargetVersion;
    }

    return verificationInfo;
}

int AbilityManagerService::CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("CheckCallServiceExtensionPermission begin");
    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start ServiceExtension or DataShareExtension");
    }
    return result;
}

int AbilityManagerService::CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("CheckCallOtherExtensionPermission begin");
    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    HILOG_DEBUG("OtherExtension type: %{public}d.", static_cast<int32_t>(extensionType));
    if (extensionType == AppExecFwk::ExtensionAbilityType::WINDOW) {
        return ERR_OK;
    }
    const std::string fileAccessPermission = "ohos.permission.FILE_ACCESS_MANAGER";
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION &&
        AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(fileAccessPermission)) {
        HILOG_DEBUG("Temporary, FILEACCESS_EXTENSION use serviceExtension start-up rule.");
        return CheckCallServiceExtensionPermission(abilityRequest);
    }

    HILOG_ERROR("CheckCallOtherExtensionPermission, Not SA, can not start other Extension");
    return CHECK_PERMISSION_FAILED;
}


int AbilityManagerService::CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("%{public}s begin", __func__);
    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest);
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start ServiceAbility");
    }
    return result;
}

int AbilityManagerService::CheckCallAbilityPermission(const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin", __func__);
    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest);
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start PageAbility(FA) or Ability(Stage)");
    }
    return result;
}

int AbilityManagerService::CheckStartByCallPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("%{public}s begin", __func__);
    // check whether the target ability is singleton mode and page type.
    if (abilityRequest.abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
        abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
        HILOG_DEBUG("Called ability is common ability and singleton.");
    } else {
        HILOG_ERROR("Called ability is not common ability or singleton.");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    if (!IsUseNewStartUpRule(abilityRequest)) {
        return CheckCallerPermissionOldRule(abilityRequest, true);
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    if (AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo) != ERR_OK) {
        HILOG_ERROR("Do not have permission to StartAbilityByCall.");
        return RESOLVE_CALL_NO_PERMISSIONS;
    }
    HILOG_DEBUG("The caller has permission to resolve the call proxy of common ability.");

    return ERR_OK;
}

int AbilityManagerService::IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }

    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        isBackgroundCall = true;
        return ERR_OK;
    }

    if (!abilityRequest.callerToken && abilityRequest.want.GetBoolParam(IS_DELEGATOR_CALL, false)) {
        // The call is from AbilityDelegator, no need to check permission
        isBackgroundCall = false;
        return ERR_OK;
    }

    if (AbilityUtil::IsStartFreeInstall(abilityRequest.want)) {
        isBackgroundCall = false;
        return ERR_OK;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(callerAbility->GetToken(), processInfo);
    } else {
        auto callerPid = IPCSkeleton::GetCallingPid();
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (processInfo.processName_.empty() && !AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
            HILOG_ERROR("Can not find caller application by callerPid, callerPid: %{private}d.", callerPid);
            return ERR_INVALID_VALUE;
        }
    }

    if (backgroundJudgeFlag_) {
        isBackgroundCall = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND &&
            !processInfo.isFocused;
    } else {
        isBackgroundCall = !processInfo.isFocused;
        if (!processInfo.isFocused && processInfo.state_ == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            // Allow background startup within 1 second after application startup if state is FOREGROUND
            int64_t aliveTime = AbilityUtil::SystemTimeMillis() - processInfo.startTimeMillis_;
            isBackgroundCall = aliveTime > APP_ALIVE_TIME_MS;
            HILOG_DEBUG(
                "Process %{public}s is alive %{public}s ms.",
                processInfo.processName_.c_str(), std::to_string(aliveTime).c_str());
        }
    }
    HILOG_DEBUG("backgroundJudgeFlag: %{public}d, isBackgroundCall: %{public}d, callerAppState: %{public}d.",
        static_cast<int32_t>(backgroundJudgeFlag_),
        static_cast<int32_t>(isBackgroundCall),
        static_cast<int32_t>(processInfo.state_));

    return ERR_OK;
}

int AbilityManagerService::CheckCallerPermissionOldRule(const AbilityRequest &abilityRequest, const bool isStartByCall)
{
    if (IPCSkeleton::GetCallingTokenID() == abilityRequest.abilityInfo.applicationInfo.accessTokenId) {
        return ERR_OK;
    }
    if (isStartByCall) {
        auto abilityInfo = abilityRequest.abilityInfo;
        auto callerUid = abilityRequest.callerUid;

        if (!CheckCallerEligibility(abilityInfo, callerUid)) {
            HILOG_ERROR("called ability has no permission.");
            return RESOLVE_CALL_NO_PERMISSIONS;
        }

        HILOG_DEBUG("the caller has permission to resolve the call proxy of common ability.");
        // check whether the target ability is singleton mode and page type.
        if (abilityInfo.type == AppExecFwk::AbilityType::PAGE &&
            abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
            HILOG_DEBUG("called ability is common ability and singleton.");
        } else {
            HILOG_ERROR("called ability is not common ability or singleton.");
            return RESOLVE_CALL_ABILITY_TYPE_ERR;
        }
        return ERR_OK;
    } else {
        return JudgeAbilityVisibleControl(abilityRequest.abilityInfo);
    }
}

bool AbilityManagerService::IsUseNewStartUpRule(const AbilityRequest &abilityRequest)
{
    if (!startUpNewRule_) {
        return false;
    }

    if (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_INFO("Caller is not an application.");
        return true;
    }

    // TEMP, white list
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        const std::string bundleName = callerAbility->GetApplicationInfo().bundleName;
        HILOG_DEBUG("IsUseNewStartUpRule, caller bundleName is %{public}s.", bundleName.c_str());
        if (whiteListNormalFlag_ && WHITE_LIST_NORMAL_SET.find(bundleName) != WHITE_LIST_NORMAL_SET.end()) {
            return false;
        }
        if (newRuleExceptLauncherSystemUI_ &&
            (bundleName == BUNDLE_NAME_LAUNCHER || bundleName == BUNDLE_NAME_SYSTEMUI)) {
            return false;
        }
    }
    HILOG_INFO("Use new startup rule");
    return true;
}

bool AbilityManagerService::CheckNewRuleSwitchState(const std::string &param)
{
    char value[NEW_RULE_VALUE_SIZE] = "false";
    int retSysParam = GetParameter(param.c_str(), "false", value, NEW_RULE_VALUE_SIZE);
    HILOG_INFO("CheckNewRuleSwitchState, %{public}s value is %{public}s.", param.c_str(), value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    return false;
}

bool AbilityManagerService::GetStartUpNewRuleFlag() const
{
    return startUpNewRule_;
}

void AbilityManagerService::CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    if (!JudgeSelfCalled(abilityRecord)) {
        return;
    }
    abilityRecord->CallRequestDone(callStub);
}

int AbilityManagerService::AddStartControlParam(Want &want, const sptr<IRemoteObject> &callerToken)
{
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        return ERR_OK;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    int32_t apiVersion = abilityRecord->GetApplicationInfo().apiTargetVersion;
    want.SetParam(DMS_API_VERSION, apiVersion);
    bool isCallerBackground = true;
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->
        GetRunningProcessInfoByToken(abilityRecord->GetToken(), processInfo);
    if (backgroundJudgeFlag_) {
        isCallerBackground = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    } else {
        isCallerBackground = !processInfo.isFocused;
    }
    want.SetParam(DMS_IS_CALLER_BACKGROUND, isCallerBackground);
    return ERR_OK;
}

int AbilityManagerService::CheckDlpForExtension(
    const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, EventInfo &eventInfo, const EventName &eventName)
{
    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendExtensionEvent(eventName, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

bool AbilityManagerService::JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return true;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID && !AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
        HILOG_ERROR("Is not self, not enabled");
        return false;
    }

    return true;
}

int AbilityManagerService::SetComponentInterception(
    const sptr<AppExecFwk::IComponentInterception> &componentInterception)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
    if (!isPerm) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    std::lock_guard<std::recursive_mutex> guard(globalLock_);
    componentInterception_ = componentInterception;
    HILOG_DEBUG("%{public}s, end", __func__);
    return ERR_OK;
}

bool AbilityManagerService::IsComponentInterceptionStart(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int componentStatus, AbilityRequest &request)
{
    if (componentInterception_ != nullptr) {
        Want newWant = want;
        int32_t type = static_cast<int32_t>(request.abilityInfo.type);
        newWant.SetParam("abilityType", type);
        int32_t launchMode = static_cast<int32_t>(request.abilityInfo.launchMode);
        newWant.SetParam("launchMode", launchMode);

        HILOG_DEBUG("%{public}s", __func__);
        sptr<Want> extraParam = new (std::nothrow) Want();
        bool isStart = componentInterception_->AllowComponentStart(newWant, callerToken,
            requestCode, componentStatus, extraParam);
        UpdateAbilityRequestInfo(extraParam, request);
        if (!isStart) {
            HILOG_INFO("not finishing start component because interception");
            return false;
        }
    }
    return true;
}

void AbilityManagerService::NotifyHandleMoveAbility(const sptr<IRemoteObject> &abilityToken, int code)
{
    if (componentInterception_ != nullptr) {
        componentInterception_->NotifyHandleMoveAbility(abilityToken, code);
    }
}

void AbilityManagerService::UpdateAbilityRequestInfo(const sptr<Want> &want, AbilityRequest &request)
{
    if (want == nullptr) {
        return;
    }
    sptr<IRemoteObject> tempCallBack = want->GetRemoteObject(Want::PARAM_RESV_ABILITY_INFO_CALLBACK);
    if (tempCallBack == nullptr) {
        return;
    }
    request.want.SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
    request.want.SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE,
        want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0));
    request.abilityInfoCallback = tempCallBack;
}

int32_t AbilityManagerService::SendResultToAbilityByToken(const Want &want, const sptr<IRemoteObject> &abilityToken,
    int32_t requestCode, int32_t resultCode, int32_t userId)
{
    HILOG_DEBUG("%{public}s, requestCode: %{public}d, resultCode: %{public}d", __func__, requestCode, resultCode);
    auto isGatewayCall = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
    if (!isGatewayCall) {
        HILOG_ERROR("%{public}s, Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    std::shared_ptr<AbilityRecord> abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("%{public}s, abilityRecord is null", __func__);
        return ERR_INVALID_VALUE;
    }
    abilityRecord->SetResult(std::make_shared<AbilityResult>(requestCode, resultCode, want));
    abilityRecord->SendResult();
    return ERR_OK;
}

std::shared_ptr<AbilityRecord> AbilityManagerService::GetFocusAbility()
{
#ifdef SUPPORT_GRAPHICS
    sptr<IRemoteObject> token;
    if (!wmsHandler_) {
        HILOG_ERROR("wmsHandler_ is nullptr.");
        return nullptr;
    }

    wmsHandler_->GetFocusWindow(token);
    if (!token) {
        HILOG_ERROR("token is nullptr");
        return nullptr;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is nullptr.");
    }
    return abilityRecord;
#endif

    return nullptr;
}
}  // namespace AAFwk
}  // namespace OHOS
