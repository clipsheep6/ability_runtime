/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <getopt.h>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#include "ability_background_connection.h"
#include "ability_debug_deal.h"
#include "ability_info.h"
#include "ability_interceptor.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "app_exit_reason_data_manager.h"
#include "application_util.h"
#include "bundle_mgr_client.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "connection_state_manager.h"
#include "distributed_client.h"
#include "dlp_utils.h"
#include "errors.h"
#include "extension_config.h"
#include "freeze_util.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "insight_intent_execute_callback_proxy.h"
#include "insight_intent_execute_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#include "mission_info.h"
#include "mission_info_mgr.h"
#include "mock_session_manager_service.h"
#include "modal_system_ui_extension.h"
#include "os_account_manager_wrapper.h"
#include "modal_system_ui_extension.h"
#include "parameters.h"
#include "permission_constants.h"
#include "recovery_param.h"
#include "sa_mgr_client.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "softbus_bus_center.h"
#include "start_ability_handler/start_ability_sandbox_savefile.h"
#include "start_options.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "system_ability_token_callback.h"
#include "extension_record_manager.h"
#include "ui_extension_utils.h"
#include "uri_permission_manager_client.h"
#include "view_data.h"
#include "xcollie/watchdog.h"
#ifdef SUPPORT_GRAPHICS
#include "dialog_session_record.h"
#include "application_anr_listener.h"
#include "display_manager.h"
#include "input_manager.h"
#endif

#ifdef EFFICIENCY_MANAGER_ENABLE
#include "suspend_manager_client.h"
#include "process_frozen_state_observer.h"
#endif // EFFICIENCY_MANAGER_ENABLE

#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif // RESOURCE_SCHEDULE_SERVICE_ENABLE

using OHOS::AppExecFwk::ElementName;
using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
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
constexpr int32_t REGISTER_FOCUS_DELAY = 5000;
constexpr size_t OFFSET = 32;
const std::string IS_DELEGATOR_CALL = "isDelegatorCall";
// Startup rule switch
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
const std::string NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI = "component.startup.newRules.except.LauncherSystemUI";
const std::string BACKGROUND_JUDGE_FLAG = "component.startup.backgroundJudge.flag";
const std::string WHITE_LIST_ASS_WAKEUP_FLAG = "component.startup.whitelist.associatedWakeUp";
// White list app
const std::string BUNDLE_NAME_SETTINGSDATA = "com.ohos.settingsdata";
const std::string BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
// Support prepare terminate
constexpr int32_t PREPARE_TERMINATE_ENABLE_SIZE = 6;
const char* PREPARE_TERMINATE_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
// UIExtension type
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string SYSTEM_SHARE = "share";
const std::string SYSTEM_SHARE_TYPE = "sysPicker/share";
// Share picker params
constexpr char SHARE_PICKER_DIALOG_BUNDLE_NAME_KEY[] = "const.system.sharePicker.bundleName";
constexpr char SHARE_PICKER_DIALOG_ABILITY_NAME_KEY[] = "const.system.sharePicker.abilityName";
constexpr char SHARE_PICKER_UIEXTENSION_NAME_KEY[] = "const.system.sharePicker.UIExtensionAbilityName";
constexpr char SHARE_PICKER_DIALOG_DEFAULY_BUNDLE_NAME[] = "com.ohos.sharepickerdialog";
constexpr char SHARE_PICKER_DIALOG_DEFAULY_ABILITY_NAME[] = "PickerDialog";
constexpr char TOKEN_KEY[] = "ohos.ability.params.token";
// Developer mode param
constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
// Broker params key
const std::string KEY_VISIBLE_ID = "ohos.anco.param.visible";
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";
const std::string SHELL_ASSISTANT_BUNDLENAME = "com.huawei.shell_assistant";
const std::string SHELL_ASSISTANT_ABILITYNAME = "MainAbility";
const std::string AMS_DIALOG_BUNDLENAME = "com.ohos.amsdialog";

const std::string DEBUG_APP = "debugApp";
const std::string AUTO_FILL_PASSWORD_TPYE = "autoFill/password";
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";

constexpr int32_t FOUNDATION_UID = 5523;
const std::string FRS_BUNDLE_NAME = "com.ohos.formrenderservice";

const std::unordered_set<std::string> WHITE_LIST_ASS_WAKEUP_SET = { BUNDLE_NAME_SETTINGSDATA };
std::atomic<bool> g_isDmsAlive = false;

bool CheckCallerIsDlpManager(const std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleManager)
{
    if (!bundleManager) {
        return false;
    }

    std::string bundleName;
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bundleManager->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        HILOG_WARN("Get Bundle Name failed.");
        return false;
    }
    if (bundleName != DLP_BUNDLE_NAME) {
        HILOG_WARN("Wrong Caller.");
        return false;
    }
    return true;
}
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
const int32_t BROKER_UID = 5557;
const int32_t BROKER_RESERVE_UID = 5005;
const int32_t DMS_UID = 5522;
const int32_t PREPARE_TERMINATE_TIMEOUT_MULTIPLE = 10;
const int32_t BOOTEVENT_COMPLETED_DELAY_TIME = 1000;
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
const std::string PERMISSIONMGR_BUNDLE_NAME = "com.ohos.permissionmanager";
const std::string PERMISSIONMGR_ABILITY_NAME = "com.ohos.permissionmanager.GrantAbility";
const std::string IS_CALL_BY_SCB = "isCallBySCB";
const std::string PROCESS_SUFFIX = "embeddable";
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
      state_(ServiceRunningState::STATE_NOT_START),
      bundleMgrHelper_(nullptr)
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
    AddSystemAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(DISTRIBUTED_SCHED_SA_ID);
    AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    HILOG_INFO("AMS start success.");
}

bool AbilityManagerService::Init()
{
    HiviewDFX::Watchdog::GetInstance().InitFfrtWatchdog(); // For ffrt watchdog available in foundation
    taskHandler_ = TaskHandlerWrap::CreateQueueHandler(AbilityConfig::NAME_ABILITY_MGR_SERVICE);
    eventHandler_ = std::make_shared<AbilityEventHandler>(taskHandler_, weak_from_this());
    freeInstallManager_ = std::make_shared<FreeInstallManager>(weak_from_this());
    CHECK_POINTER_RETURN_BOOL(freeInstallManager_);

    // init user controller.
    userController_ = std::make_shared<UserController>();
    userController_->Init();

    InitConnectManager(MAIN_USER_ID, true);
    InitDataAbilityManager(MAIN_USER_ID, true);
    InitPendWantManager(MAIN_USER_ID, true);
    systemDataAbilityManager_ = std::make_shared<DataAbilityManager>();

    AmsConfigurationParameter::GetInstance().Parse();
    HILOG_INFO("ams config parse");
    std::string deviceType = OHOS::system::GetDeviceType();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_ = std::make_shared<UIAbilityLifecycleManager>();
        uiAbilityLifecycleManager_->SetDevice(deviceType);
    } else {
        InitMissionListManager(MAIN_USER_ID, true);
    }
    SwitchManagers(U0_USER_ID, false);
#ifdef SUPPORT_GRAPHICS
    auto anrListenerTask = []() {
        auto anrListener = std::make_shared<ApplicationAnrListener>();
        MMI::InputManager::GetInstance()->SetAnrObserver(anrListener);
    };
    taskHandler_->SubmitTask(anrListenerTask, "AnrListenerTask");
    DelayedSingleton<SystemDialogScheduler>::GetInstance()->SetDeviceType(deviceType);
    implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        InitFocusListener();
    }
#endif

    DelayedSingleton<ConnectionStateManager>::GetInstance()->Init(taskHandler_);

    interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    interceptorExecuter_->AddInterceptor(std::make_shared<CrowdTestInterceptor>());
    interceptorExecuter_->AddInterceptor(std::make_shared<ControlInterceptor>());
    afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    afterCheckExecuter_->AddInterceptor(std::make_shared<DisposedRuleInterceptor>());
#ifdef SUPPORT_ERMS
    afterCheckExecuter_->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
#else
    interceptorExecuter_->AddInterceptor(std::make_shared<EcologicalRuleInterceptor>());
#endif
    afterCheckExecuter_->SetTaskHandler(taskHandler_);
    bool isAppJumpEnabled = OHOS::system::GetBoolParameter(
        OHOS::AppExecFwk::PARAMETER_APP_JUMP_INTERCEPTOR_ENABLE, false);
    if (isAppJumpEnabled) {
        HILOG_INFO("App jump intercetor enabled, add AbilityJumpInterceptor to Executer");
        interceptorExecuter_->AddInterceptor(std::make_shared<AbilityJumpInterceptor>());
    }
    InitStartAbilityChain();

    abilityAutoStartupService_ = std::make_shared<AbilityRuntime::AbilityAutoStartupService>();

    dialogSessionRecord_ = std::make_shared<DialogSessionRecord>();

    auto startResidentAppsTask = [aams = shared_from_this()]() { aams->StartResidentApps(); };
    taskHandler_->SubmitTask(startResidentAppsTask, "StartResidentApps");

    auto initStartupFlagTask = [aams = shared_from_this()]() { aams->InitStartupFlag(); };
    taskHandler_->SubmitTask(initStartupFlagTask, "InitStartupFlag");

    auto initPrepareTerminateConfigTask = [aams = shared_from_this()]() { aams->InitPrepareTerminateConfig(); };
    taskHandler_->SubmitTask(initPrepareTerminateConfigTask, "InitPrepareTerminateConfig");

    RegisterSuspendObserver();

    auto initExtensionConfigTask = []() {
        DelayedSingleton<ExtensionConfig>::GetInstance()->LoadExtensionConfiguration();
    };
    taskHandler_->SubmitTask(initExtensionConfigTask, "InitExtensionConfigTask");

    auto bootCompletedTask = [handler = taskHandler_]() {
        if (ApplicationUtil::IsBootCompleted()) {
            auto task = []() {
                ApplicationUtil::AppFwkBootEventCallback(BOOTEVENT_BOOT_COMPLETED.c_str(), "true", nullptr);
            };
            handler->SubmitTask(task, "BootCompletedDelayTask", BOOTEVENT_COMPLETED_DELAY_TIME);
        } else {
            WatchParameter(BOOTEVENT_BOOT_COMPLETED.c_str(), ApplicationUtil::AppFwkBootEventCallback, nullptr);
        }
    };
    taskHandler_->SubmitTask(bootCompletedTask, "BootCompletedTask");
    SubscribeScreenUnlockedEvent();
    HILOG_INFO("Init success.");
    return true;
}

void AbilityManagerService::InitStartupFlag()
{
    startUpNewRule_ = CheckNewRuleSwitchState(COMPONENT_STARTUP_NEW_RULES);
    newRuleExceptLauncherSystemUI_ = CheckNewRuleSwitchState(NEW_RULES_EXCEPT_LAUNCHER_SYSTEMUI);
    backgroundJudgeFlag_ = CheckNewRuleSwitchState(BACKGROUND_JUDGE_FLAG);
    whiteListassociatedWakeUpFlag_ = CheckNewRuleSwitchState(WHITE_LIST_ASS_WAKEUP_FLAG);
}

void AbilityManagerService::InitStartAbilityChain()
{
    auto startSandboxSaveFile = std::make_shared<StartAbilitySandboxSavefile>();
    startAbilityChain_.emplace(startSandboxSaveFile->GetPriority(), startSandboxSaveFile);
}

void AbilityManagerService::OnStop()
{
    HILOG_INFO("Stop AMS.");
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (bgtaskObserver_) {
        int ret = BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*bgtaskObserver_);
        if (ret != ERR_OK) {
            HILOG_ERROR("unsubscribe bgtask failed, err:%{public}d.", ret);
        }
    }
#endif
    if (abilityBundleEventCallback_) {
        auto bms = GetBundleManager();
        if (bms) {
            bool ret = IN_PROCESS_CALL(bms->UnregisterBundleEventCallback(abilityBundleEventCallback_));
            if (ret != ERR_OK) {
                HILOG_ERROR("unsubscribe bundle event callback failed, err:%{public}d.", ret);
            }
        }
    }
    eventHandler_.reset();
    taskHandler_.reset();
    state_ = ServiceRunningState::STATE_NOT_START;
}

ServiceRunningState AbilityManagerService::QueryServiceState() const
{
    return state_;
}

int AbilityManagerService::StartAbility(const Want &want, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (want.GetBoolParam(DEBUG_APP, false) && !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }
    HILOG_INFO("%{public}s coldStart:%{public}d", __func__, want.GetBoolParam("coldStart", false));
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    int32_t ret = StartAbilityWrap(want, nullptr, requestCode, userId);
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
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
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

    int32_t ret = StartAbilityWrap(want, callerToken, requestCode, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int32_t AbilityManagerService::StartAbilityByInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
    uint64_t intentId, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string bundleNameFromWant = want.GetElement().GetBundleName();
    std::string bundleNameFromIntentMgr = "";
    if (DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->
        GetBundleName(intentId, bundleNameFromIntentMgr) != ERR_OK) {
        HILOG_ERROR("no such bundle matched intentId");
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("no such bundle matched token");
        return ERR_INVALID_VALUE;
    }
    std::string bundleNameFromAbilityRecord = abilityRecord->GetAbilityInfo().bundleName;

    if (!bundleNameFromWant.empty() && bundleNameFromWant == bundleNameFromIntentMgr &&
        bundleNameFromWant == bundleNameFromAbilityRecord) {
        HILOG_INFO("bundleName match");
        return StartAbility(want, callerToken, userId, -1);
    }
    HILOG_ERROR("bundleName not match");
    return ERR_INSIGHT_INTENT_START_INVALID_COMPONENT;
}

int AbilityManagerService::StartAbilityByUIContentSession(const Want &want, const sptr<IRemoteObject> &callerToken,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode)
{
    if (!callerToken || !sessionInfo) {
        HILOG_ERROR("callerToken or sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> token;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            HILOG_ERROR("wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }

    if (!token) {
        HILOG_ERROR("token is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (token != sessionInfo->callerToken) {
        HILOG_ERROR("callerToken is not equal to top ablity token");
        return NOT_TOP_ABILITY;
    }
    return StartAbility(want, callerToken, userId, requestCode);
}

int AbilityManagerService::StartAbilityByUIContentSession(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode)
{
    if (!callerToken || !sessionInfo) {
        HILOG_ERROR("callerToken or sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> token;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            HILOG_ERROR("wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }

    if (!token) {
        HILOG_ERROR("token is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (token != sessionInfo->callerToken) {
        HILOG_ERROR("callerToken is not equal to top ablity token");
        return NOT_TOP_ABILITY;
    }
    return StartAbility(want, startOptions, callerToken, userId, requestCode);
}

int AbilityManagerService::StartAbilityAsCaller(const Want &want, const sptr<IRemoteObject> &callerToken,
    sptr<IRemoteObject> asCallerSoureToken, int32_t userId, int requestCode, bool isSendDialogResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto flags = want.GetFlags();
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        HILOG_ERROR("StartAbility with continuation flags is not allowed!");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    AAFwk::Want newWant = want;
    if (asCallerSoureToken != nullptr) {
        HILOG_DEBUG("Start as caller, UpdateCallerInfo");
        UpdateAsCallerSourceInfo(newWant, asCallerSoureToken);
    }

    HILOG_INFO("Start ability come, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetAbilityName().c_str(), userId);
    std::string callerPkg;
    std::string targetPkg;
    if (AbilityUtil::CheckJumpInterceptorWant(newWant, callerPkg, targetPkg)) {
        HILOG_INFO("the call is from interceptor dialog, callerPkg:%{public}s, targetPkg:%{public}s",
            callerPkg.c_str(), targetPkg.c_str());
        AbilityUtil::AddAbilityJumpRuleToBms(callerPkg, targetPkg, GetUserId());
    }
    int32_t ret = StartAbilityWrap(newWant, callerToken, requestCode, userId, true, isSendDialogResult);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int AbilityManagerService::StartAbilityPublicPrechainCheck(StartAbilityParams &params)
{
    // 1. CheckCallerToken
    if (params.callerToken != nullptr && !VerificationAllToken(params.callerToken)) {
        auto isSpecificSA = AAFwk::PermissionVerification::GetInstance()->
            CheckSpecificSystemAbilityAccessPermission();
        if (!isSpecificSA) {
            HILOG_ERROR("%{public}s VerificationAllToken failed.", __func__);
            return ERR_INVALID_CALLER;
        }
        HILOG_INFO("%{public}s: Caller is specific system ability.", __func__);
    }

    // 2. validUserId, multi-user
    if (!JudgeMultiUserConcurrency(params.GetValidUserId())) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    return ERR_OK;
}

int AbilityManagerService::StartAbilityPrechainInterceptor(StartAbilityParams &params)
{
    auto interceptorResult = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(params.want, params.requestCode, GetUserId(), true);
    if (interceptorResult != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return interceptorResult;
    }

    return ERR_OK;
}

bool AbilityManagerService::StartAbilityInChain(StartAbilityParams &params, int &result)
{
    std::shared_ptr<StartAbilityHandler> reqHandler;
    for (const auto &item : startAbilityChain_) {
        if (item.second->MatchStartRequest(params)) {
            reqHandler = item.second;
            break;
        }
    }

    if (!reqHandler) {
        return false;
    }

    result = StartAbilityPublicPrechainCheck(params);
    if (result != ERR_OK) {
        return true;
    }
    result = StartAbilityPrechainInterceptor(params);
    if (result != ERR_OK) {
        return true;
    }
    result = reqHandler->HandleStartRequest(params);
    return true;
}

int AbilityManagerService::StartAbilityWrap(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int32_t userId, bool isStartAsCaller, bool isSendDialogResult)
{
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.isStartAsCaller = isStartAsCaller;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

    return StartAbilityInner(want, callerToken, requestCode, userId, isStartAsCaller, isSendDialogResult);
}

int AbilityManagerService::StartAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int32_t userId, bool isStartAsCaller, bool isSendDialogResult)
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
            InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
            return StartExtensionAbilityInner(want, callerToken, userId,
                AppExecFwk::ExtensionAbilityType::SERVICE, false);
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
        HILOG_INFO("try to StartRemoteAbility");
        return StartRemoteAbility(want, requestCode, validUserId, callerToken);
    }

    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
#ifdef SUPPORT_GRAPHICS
    if (ImplicitStartProcessor::IsImplicitStartAction(want)) {
        ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode, result);
        if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
            return componentRequest.requestResult;
        }
        abilityRequest.Voluation(want, requestCode, callerToken);
        if (!isStartAsCaller) {
            HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(abilityRequest.want, callerToken);
        }
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        return implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode, result);
    if (CheckProxyComponent(want, result) && !IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    std::string callerBundleName = abilityRecord ? abilityRecord->GetAbilityInfo().bundleName : "";
    std::string callerAbilityName = abilityRecord ? abilityRecord->GetAbilityInfo().name : "";
    bool selfFreeInstallEnable = (result == RESOLVE_ABILITY_ERR && want.GetElement().GetModuleName() != "" &&
        want.GetElement().GetBundleName() == callerBundleName);
    bool isStartFreeInstallByWant = AbilityUtil::IsStartFreeInstall(want);
    if (isStartFreeInstallByWant || selfFreeInstallEnable) {
        if (freeInstallManager_ == nullptr) {
            return ERR_INVALID_VALUE;
        }
        Want localWant = want;
        if (!localWant.GetDeviceId().empty()) {
            localWant.SetDeviceId("");
        }
        if (!isStartAsCaller) {
            HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(localWant, callerToken);
        }

        if (isStartFreeInstallByWant) {
            return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, true);
        }
        int32_t ret = freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, false);
        if (ret == ERR_OK) {
            result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
        }
    }

    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }

    std::string dialogSessionId = want.GetStringParam("dialogSessionId");
    isSendDialogResult = false;
    if (!dialogSessionId.empty() && dialogSessionRecord_->GetDialogCallerInfo(dialogSessionId) != nullptr) {
        isSendDialogResult = true;
        dialogSessionRecord_->ClearDialogContext(dialogSessionId);
    }
    if (!isStartAsCaller) {
        HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(abilityRequest.want, callerToken);
    } else if (callerBundleName == AMS_DIALOG_BUNDLENAME ||
        (isSendDialogResult && want.GetBoolParam("isSelector", false))) {
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        implicitStartProcessor_->ResetCallingIdentityAsCaller(abilityRequest.want.GetIntParam(
            Want::PARAM_RESV_CALLER_TOKEN, 0));
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo, isStartAsCaller,
        abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Cannot start data ability by start ability.");
        return ERR_WRONG_INTERFACE_CALL;
    } else if (type == AppExecFwk::AbilityType::EXTENSION) {
        auto isSACall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        auto isGatewayCall = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
        auto isSystemAppCall = AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall();
        auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
        auto isToPermissionMgr = IsTargetPermission(want);
        if (!isSACall && !isGatewayCall && !isSystemAppCall && !isShellCall && !isToPermissionMgr) {
            HILOG_ERROR("Cannot start extension by start ability, use startServiceExtensionAbility.");
            return ERR_WRONG_INTERFACE_CALL;
        }

        if (IPCSkeleton::GetCallingUid() == BROKER_UID || isSystemAppCall || isToPermissionMgr) {
            result = CheckCallServicePermission(abilityRequest);
            if (result != ERR_OK) {
                HILOG_ERROR("Check permission failed");
                return result;
            }
        }
    } else if (type == AppExecFwk::AbilityType::SERVICE) {
        HILOG_DEBUG("Check call service or extension permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallServicePermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    } else if ((callerBundleName == SHELL_ASSISTANT_BUNDLENAME && callerAbilityName == SHELL_ASSISTANT_ABILITYNAME) ||
        IPCSkeleton::GetCallingUid() == BROKER_UID) {
        if (abilityRequest.collaboratorType != CollaboratorType::RESERVE_TYPE && !abilityInfo.visible) {
            HILOG_DEBUG("Check permission failed");
            return CHECK_PERMISSION_FAILED;
        }
        HILOG_DEBUG("Check call service or extension permission, name is %{public}s.", abilityInfo.name.c_str());
        auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
        if (collaborator == nullptr) {
            HILOG_ERROR("Collaborator is nullptr.");
            return CHECK_PERMISSION_FAILED;
        }
        result = collaborator->CheckCallAbilityPermission(abilityRequest.want);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed from broker.");
            return CHECK_PERMISSION_FAILED;
        }
    } else if (!isSendDialogResult || want.GetBoolParam("isSelector", false)) {
        HILOG_DEBUG("Check call ability permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallAbilityPermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    }

    Want newWant = abilityRequest.want;
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(newWant, requestCode, GetUserId(), true, callerToken);
    bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
    newWant.RemoveParam("queryWantFromErms");
    if (result != ERR_OK && isReplaceWantExist == false) {
        HILOG_ERROR("DoProcess failed or replaceWant not exist");
        return result;
    }
    if (result != ERR_OK && isReplaceWantExist && !isSendDialogResult &&
        callerBundleName != AMS_DIALOG_BUNDLENAME) {
        std::string dialogSessionId;
        std::vector<DialogAppInfo> dialogAppInfos(1);
        if (GenerateDialogSessionRecord(abilityRequest, GetUserId(), dialogSessionId, dialogAppInfos, false)) {
            HILOG_DEBUG("create dialog by ui extension");
            return CreateModalDialog(newWant, callerToken, dialogSessionId);
        }
        HILOG_ERROR("create dialog by ui extension failed");
        return INNER_ERR;
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

    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(validUserId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_INFO("Start service or extension, name is %{public}s.", abilityInfo.name.c_str());
        ReportEventToSuspendManager(abilityInfo);
        InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
        return connectManager->StartAbility(abilityRequest);
    }

    if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        HILOG_ERROR("IsAbilityControllerStart failed: %{public}s.", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }
    // sceneboard
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToSuspendManager(abilityInfo);
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        return uiAbilityLifecycleManager_->NotifySCBToStartUIAbility(abilityRequest, oriValidUserId);
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
    if (want.GetBoolParam(DEBUG_APP, false) && !system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

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

    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, requestCode, GetUserId(), true);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            HILOG_ERROR("can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        Want localWant = want;
        UpdateCallerInfo(localWant, callerToken);
        return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, true);
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
        ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode);
        if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
            return componentRequest.requestResult;
        }
        abilityRequest.Voluation(
            want, requestCode, callerToken, std::make_shared<AbilityStartSetting>(abilityStartSetting));
        abilityRequest.callType = AbilityCallType::START_SETTINGS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode, result);
    if (result != ERR_OK && !IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
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

    result = CheckStaticCfgPermission(abilityInfo, false, -1);
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

    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, requestCode, GetUserId(), true, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
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
    if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        UpdateCallerInfo(abilityRequest.want, callerToken);
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        return uiAbilityLifecycleManager_->NotifySCBToStartUIAbility(abilityRequest, oriValidUserId);
    }
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);
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
    HILOG_DEBUG("Start ability with startOptions.");
    return StartAbilityForOptionWrap(want, startOptions, callerToken, userId, requestCode, false);
}

int AbilityManagerService::StartAbilityAsCaller(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, sptr<IRemoteObject> asCallerSoureToken,
    int32_t userId, int requestCode)
{
    HILOG_DEBUG("Start ability as caller with startOptions.");
    CHECK_CALLER_IS_SYSTEM_APP;

    AAFwk::Want newWant = want;
    if (asCallerSoureToken != nullptr) {
        HILOG_DEBUG("start as caller, UpdateCallerInfo");
        UpdateAsCallerSourceInfo(newWant, asCallerSoureToken);
    }

    return StartAbilityForOptionWrap(newWant, startOptions, callerToken, userId, requestCode, true);
}

int AbilityManagerService::StartAbilityForOptionWrap(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode, bool isStartAsCaller)
{
    StartAbilityParams startParams(const_cast<Want &>(want));
    startParams.callerToken = callerToken;
    startParams.userId = userId;
    startParams.requestCode = requestCode;
    startParams.isStartAsCaller = isStartAsCaller;
    startParams.startOptions = &startOptions;
    startParams.SetValidUserId(GetValidUserId(userId));

    int result = ERR_OK;
    if (StartAbilityInChain(startParams, result)) {
        return result;
    }

    return StartAbilityForOptionInner(want, startOptions, callerToken, userId, requestCode, isStartAsCaller);
}

int AbilityManagerService::StartAbilityForOptionInner(const Want &want, const StartOptions &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode, bool isStartAsCaller)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
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
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;

    if (AbilityUtil::IsStartFreeInstall(want)) {
        if (CheckIfOperateRemote(want) || freeInstallManager_ == nullptr) {
            HILOG_ERROR("can not start remote free install");
            return ERR_INVALID_VALUE;
        }
        Want localWant = want;
        if (!isStartAsCaller) {
            HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(localWant, callerToken);
        }
        return freeInstallManager_->StartFreeInstall(localWant, validUserId, requestCode, callerToken, true);
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
        ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode);
        if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
            return componentRequest.requestResult;
        }
        abilityRequest.Voluation(want, requestCode, callerToken);
        abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
        abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions.GetWindowMode());
        if (AppUtils::GetInstance().JudgePCDevice()) {
            if (startOptions.windowLeftUsed_) {
                abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, startOptions.GetWindowLeft());
            }
            if (startOptions.windowTopUsed_) {
                abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_TOP, startOptions.GetWindowTop());
            }
            if (startOptions.windowWidthUsed_) {
                abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, startOptions.GetWindowWidth());
            }
            if (startOptions.windowHeightUsed_) {
                abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, startOptions.GetWindowHeight());
            }
            bool withAnimation = startOptions.GetWithAnimation();
            auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
            if (!withAnimation && abilityRecord != nullptr &&
                abilityRecord->GetAbilityInfo().bundleName == abilityRequest.want.GetBundle()) {
                abilityRequest.want.SetParam(Want::PARAM_RESV_WITH_ANIMATION, withAnimation);
            }
        }
        abilityRequest.callType = AbilityCallType::START_OPTIONS_TYPE;
        CHECK_POINTER_AND_RETURN(implicitStartProcessor_, ERR_IMPLICIT_START_ABILITY_FAIL);
        if (!isStartAsCaller) {
            HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
            UpdateCallerInfo(abilityRequest.want, callerToken);
        }
        result = implicitStartProcessor_->ImplicitStartAbility(abilityRequest, validUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("implicit start ability error.");
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return result;
    }
    if (want.GetAction().compare(ACTION_CHOOSE) == 0) {
        return ShowPickerDialog(want, validUserId, callerToken);
    }
#endif
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode, result);
    if (result != ERR_OK && !IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    if (!isStartAsCaller) {
        HILOG_DEBUG("do not start as caller, UpdateCallerInfo");
        UpdateCallerInfo(abilityRequest.want, callerToken);
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo, isStartAsCaller,
        abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0));
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

    if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        eventInfo.errCode = ERR_WOULD_BLOCK;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_WOULD_BLOCK;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo.isStageBasedModel && !CheckWindowMode(startOptions.GetWindowMode(), abilityInfo.windowModes)) {
        return ERR_AAFWK_INVALID_WINDOW_MODE;
    }
#endif

    abilityRequest.want.SetParam(Want::PARAM_RESV_DISPLAY_ID, startOptions.GetDisplayID());
    abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, startOptions.GetWindowMode());
    if (AppUtils::GetInstance().JudgePCDevice()) {
        if (startOptions.windowLeftUsed_) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, startOptions.GetWindowLeft());
        }
        if (startOptions.windowTopUsed_) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_TOP, startOptions.GetWindowTop());
        }
        if (startOptions.windowHeightUsed_) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, startOptions.GetWindowHeight());
        }
        if (startOptions.windowWidthUsed_) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, startOptions.GetWindowWidth());
        }
        bool withAnimation = startOptions.GetWithAnimation();
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (!withAnimation && abilityRecord != nullptr &&
            abilityRecord->GetAbilityInfo().bundleName == abilityRequest.want.GetBundle()) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_WITH_ANIMATION, withAnimation);
        }
    }

    Want newWant = abilityRequest.want;
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(newWant, requestCode, GetUserId(), true, callerToken);
    bool isReplaceWantExist = newWant.GetBoolParam("queryWantFromErms", false);
    newWant.RemoveParam("queryWantFromErms");
    if (result != ERR_OK && isReplaceWantExist == false) {
        HILOG_ERROR("DoProcess failed or replaceWant not exist");
        return result;
    }
    if (result != ERR_OK && isReplaceWantExist) {
        std::string dialogSessionId;
        std::vector<DialogAppInfo> dialogAppInfos(1);
        if (GenerateDialogSessionRecord(abilityRequest, GetUserId(), dialogSessionId, dialogAppInfos, false)) {
            HILOG_DEBUG("create dialog by ui extension");
            return CreateModalDialog(newWant, callerToken, dialogSessionId);
        }
        HILOG_ERROR("create dialog by ui extension failed");
        return INNER_ERR;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        return uiAbilityLifecycleManager_->NotifySCBToStartUIAbility(abilityRequest, oriValidUserId);
    }
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is Null. userId=%{public}d", oriValidUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    auto ret = missionListManager->StartAbility(abilityRequest);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int32_t AbilityManagerService::RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_ABILITY_CONTINUATION) == Want::FLAG_ABILITY_CONTINUATION) {
        HILOG_ERROR("RequestDialogService with continuation flags is not allowed!");
        return ERR_INVALID_CONTINUATION_FLAG;
    }

    HILOG_INFO("request dialog service, target is %{public}s", want.GetElement().GetURI().c_str());
    return RequestDialogServiceInner(want, callerToken, -1, -1);
}

int32_t AbilityManagerService::ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("called.");
    if (callerToken == nullptr) {
        HILOG_ERROR("callerToken is nullptr");
        return INNER_ERR;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return INNER_ERR;
    }
    auto abilityInfo = abilityRecord->GetAbilityInfo();

    EventInfo eventInfo;
    eventInfo.userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    eventInfo.pid = IPCSkeleton::GetCallingPid();
    eventInfo.bundleName = abilityInfo.bundleName;
    eventInfo.moduleName = abilityInfo.moduleName;
    eventInfo.abilityName = abilityInfo.name;
    EventReport::SendAppEvent(EventName::DRAWN_COMPLETED, HiSysEventType::BEHAVIOR, eventInfo);
    return ERR_OK;
}

int32_t AbilityManagerService::RequestDialogServiceInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int requestCode, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (callerToken == nullptr || !VerificationAllToken(callerToken)) {
        HILOG_WARN("caller is invalid.");
        return ERR_INVALID_CALLER;
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CHECK_DLP");
        if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, want) ||
            !DlpUtils::DlpAccessOtherAppsCheck(callerToken, want)) {
            HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
            return CHECK_PERMISSION_FAILED;
        }

        if (AbilityUtil::HandleDlpApp(const_cast<Want &>(want))) {
            HILOG_ERROR("Cannot handle dlp by RequestDialogService.");
            return ERR_WRONG_INTERFACE_CALL;
        }
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, requestCode, GetUserId(), true);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    int32_t oriValidUserId = GetValidUserId(userId);
    int32_t validUserId = oriValidUserId;
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    result = GenerateAbilityRequest(want, requestCode, abilityRequest, callerToken, validUserId);
    ComponentRequest componentRequest = initComponentRequest(callerToken, requestCode, result);
    if (CheckProxyComponent(want, result) && !IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error when RequestDialogService.");
        return result;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo, false, -1);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    auto type = abilityInfo.type;
    if (type == AppExecFwk::AbilityType::PAGE) {
        result = CheckCallAbilityPermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    } else if (type == AppExecFwk::AbilityType::EXTENSION &&
        abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        HILOG_DEBUG("Check call ability permission, name is %{public}s.", abilityInfo.name.c_str());
        result = CheckCallServicePermission(abilityRequest);
        if (result != ERR_OK) {
            HILOG_ERROR("Check permission failed");
            return result;
        }
    } else {
        HILOG_ERROR("RequestDialogService do not support other component.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, requestCode, GetUserId(), true, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    if (type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(validUserId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
            return ERR_INVALID_VALUE;
        }
        HILOG_DEBUG("request dialog service, start service extension,name is %{public}s.", abilityInfo.name.c_str());
        ReportEventToSuspendManager(abilityInfo);
        return connectManager->StartAbility(abilityRequest);
    }

    if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }
    if (!IsAbilityControllerStart(want, abilityInfo.bundleName)) {
        HILOG_ERROR("IsAbilityControllerStart failed : %{public}s.", abilityInfo.bundleName.c_str());
        return ERR_WOULD_BLOCK;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToSuspendManager(abilityInfo);
        abilityRequest.userId = oriValidUserId;
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        return uiAbilityLifecycleManager_->NotifySCBToStartUIAbility(abilityRequest, oriValidUserId);
    }
    auto missionListManager = GetListManagerByUserId(oriValidUserId);
    if (missionListManager == nullptr) {
        HILOG_ERROR("missionListManager is nullptr. userId:%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    ReportAbilitStartInfoToRSS(abilityInfo);
    ReportEventToSuspendManager(abilityInfo);
    HILOG_DEBUG("RequestDialogService, start ability, name is %{public}s.", abilityInfo.name.c_str());
    return missionListManager->StartAbility(abilityRequest);
}

int AbilityManagerService::StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (sessionInfo->userId == DEFAULT_INVAL_VALUE) {
        sessionInfo->userId = GetUserId();
    } else if (sessionInfo->userId != GetUserId()) {
        HILOG_ERROR("this userId is not valid");
        return ERR_CROSS_USER;
    }

    auto currentUserId = GetUserId();
    EventInfo eventInfo = BuildEventInfo(sessionInfo->want, currentUserId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);

    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    auto requestCode = sessionInfo->requestCode;
    if (sessionInfo->want.GetBoolParam(IS_CALL_BY_SCB, true)) {
        HILOG_DEBUG("interceptorExecuter_ called.");
        auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(sessionInfo->want, requestCode, currentUserId, true);
        if (result != ERR_OK) {
            HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
            eventInfo.errCode = result;
            EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
            return result;
        }
    }

    AbilityRequest abilityRequest;
    auto result = GenerateAbilityRequest(sessionInfo->want, requestCode, abilityRequest,
        sessionInfo->callerToken, currentUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        return result;
    }
    abilityRequest.collaboratorType = sessionInfo->collaboratorType;

    auto abilityInfo = abilityRequest.abilityInfo;
    if (!AAFwk::PermissionVerification::GetInstance()->IsSystemAppCall() &&
        abilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support for page type ability.");
        return ERR_INVALID_VALUE;
    }

    if (sessionInfo->want.GetBoolParam(IS_CALL_BY_SCB, true)) {
        HILOG_DEBUG("afterCheckExecuter_ called.");
        result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE : afterCheckExecuter_->DoProcess(
            abilityRequest.want, requestCode, GetUserId(), true, sessionInfo->callerToken);
        if (result != ERR_OK) {
            HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
            return result;
        }
    }

    if (!AbilityUtil::IsSystemDialogAbility(abilityInfo.bundleName, abilityInfo.name)) {
        result = PreLoadAppDataAbilities(abilityInfo.bundleName, currentUserId);
        if (result != ERR_OK) {
            HILOG_ERROR("StartAbility: App data ability preloading failed, '%{public}s', %{public}d",
                abilityInfo.bundleName.c_str(), result);
            return result;
        }
    }

    if (uiAbilityLifecycleManager_ == nullptr) {
        HILOG_ERROR("uiAbilityLifecycleManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    ReportAbilitStartInfoToRSS(abilityInfo);
    ReportAbilitAssociatedStartInfoToRSS(abilityInfo, static_cast<int64_t>(
        ResourceSchedule::ResType::AssociatedStartType::SCB_START_ABILITY), sessionInfo->callerToken);
    return uiAbilityLifecycleManager_->StartUIAbility(abilityRequest, sessionInfo);
}

bool AbilityManagerService::CheckCallingTokenId(const std::string &bundleName)
{
    auto bms = GetBundleManager();
    if (bms == nullptr) {
        HILOG_ERROR("bms is invalid.");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bms->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetUserId(), appInfo));
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    if (accessTokenId != appInfo.accessTokenId) {
        HILOG_ERROR("Permission verification failed");
        return false;
    }
    return true;
}

bool AbilityManagerService::IsBackgroundTaskUid(const int uid)
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::lock_guard<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (bgtaskObserver_) {
        return bgtaskObserver_->IsBackgroundTaskUid(uid);
    }
    return false;
#else
    return false;
#endif
}

bool AbilityManagerService::IsDmsAlive() const
{
    return g_isDmsAlive.load();
}

void AbilityManagerService::AppUpgradeCompleted(const std::string &bundleName, int32_t uid)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        HILOG_ERROR("Not sa call");
        return;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);
    auto userId = uid / BASE_USER_RANGE;

    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(
        bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId))) {
        HILOG_ERROR("Failed to get bundle info.");
        return;
    }

    RecordAppExitReasonAtUpgrade(bundleInfo);

    if (userId != U0_USER_ID) {
        HILOG_ERROR("Application upgrade for non U0 users.");
        return;
    }

    if (!bundleInfo.isKeepAlive) {
        HILOG_WARN("Not a resident application.");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos = { bundleInfo };
    DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcessWithMainElement(bundleInfos);

    if (!bundleInfos.empty()) {
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

int32_t AbilityManagerService::RecordAppExitReason(Reason exitReason)
{
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !currentMissionListManager_) {
        HILOG_ERROR("currentMissionListManager_ is null.");
        return ERR_NULL_OBJECT;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_NULL_OBJECT);

    std::string bundleName;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (IN_PROCESS_CALL(bms->GetNameForUid(callerUid, bundleName)) != ERR_OK) {
        HILOG_ERROR("Get Bundle Name failed.");
        return ERR_INVALID_VALUE;
    }

    std::vector<std::string> abilityList;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->GetActiveAbilityList(bundleName, abilityList);
    } else {
        currentMissionListManager_->GetActiveAbilityList(bundleName, abilityList);
    }

    return DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        bundleName, abilityList, exitReason);
}

int32_t AbilityManagerService::ForceExitApp(const int32_t pid, Reason exitReason)
{
    if (exitReason < REASON_UNKNOWN || exitReason > REASON_UPGRADE) {
        HILOG_ERROR("Force exit reason invalid.");
        return ERR_INVALID_VALUE;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Not sa or shell call");
        return ERR_PERMISSION_DENIED;
    }

    std::string bundleName;
    int32_t uid;
    DelayedSingleton<AppScheduler>::GetInstance()->GetBundleNameByPid(pid, bundleName, uid);
    if (bundleName.empty()) {
        HILOG_ERROR("Get bundle name by pid failed.");
        return ERR_INVALID_VALUE;
    }

    int32_t targetUserId = uid / BASE_USER_RANGE;
    std::vector<std::string> abilityLists;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->GetActiveAbilityList(bundleName, abilityLists, targetUserId);
    } else if (targetUserId == U0_USER_ID) {
        std::lock_guard lock(managersMutex_);
        for (auto item: missionListManagers_) {
            if (item.second) {
                std::vector<std::string> abilityList;
                item.second->GetActiveAbilityList(bundleName, abilityList);
                if (!abilityList.empty()) {
                    abilityLists.insert(abilityLists.end(), abilityList.begin(), abilityList.end());
                }
            }
        }
    } else {
        auto listManager = GetListManagerByUserId(targetUserId);
        if (listManager) {
            listManager->GetActiveAbilityList(bundleName, abilityLists);
        }
    }

    int32_t result = DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        bundleName, abilityLists, exitReason);

    DelayedSingleton<AppScheduler>::GetInstance()->KillApplication(bundleName);

    return result;
}

int32_t AbilityManagerService::GetConfiguration(AppExecFwk::Configuration& config)
{
    auto appMgr = GetAppMgr();
    if (appMgr == nullptr) {
        HILOG_WARN("GetAppMgr failed");
        return -1;
    }

    return appMgr->GetConfiguration(config);
}

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> AbilityManagerService::GetAppMgr()
{
    if (appMgr_) {
        return appMgr_;
    }

    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    appMgr_ = OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(object);
    return appMgr_;
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

    auto result = CheckStaticCfgPermission(abilityInfo, false, -1);
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

    UpdateCallerInfo(abilityRequest.want, abilityRequest.callerToken);
    return ERR_OK;
}

void AbilityManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId: %{public}d add", systemAbilityId);
    switch (systemAbilityId) {
        case BACKGROUND_TASK_MANAGER_SERVICE_ID: {
            SubscribeBackgroundTask();
            break;
        }
        case DISTRIBUTED_SCHED_SA_ID: {
            g_isDmsAlive.store(true);
            break;
        }
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            SubscribeBundleEventCallback();
            break;
        }
        default:
            break;
    }
}

void AbilityManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId: %{public}d remove", systemAbilityId);
    switch (systemAbilityId) {
        case BACKGROUND_TASK_MANAGER_SERVICE_ID: {
            UnSubscribeBackgroundTask();
            break;
        }
        case DISTRIBUTED_SCHED_SA_ID: {
            g_isDmsAlive.store(false);
            break;
        }
        case BUNDLE_MGR_SERVICE_SYS_ABILITY_ID: {
            UnsubscribeBundleEventCallback();
            break;
        }
        default:
            break;
    }
}

void AbilityManagerService::SubscribeBackgroundTask()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (!bgtaskObserver_) {
        bgtaskObserver_ = std::make_shared<BackgroundTaskObserver>();
    }
    int ret = BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgtaskObserver_);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s failed, err:%{public}d.", __func__, ret);
        return;
    }
    bgtaskObserver_->GetContinuousTaskApps();
    HILOG_INFO("%{public}s success.", __func__);
#endif
}

void AbilityManagerService::UnSubscribeBackgroundTask()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::unique_lock<ffrt::mutex> lock(bgtaskObserverMutex_);
    if (!bgtaskObserver_) {
        return;
    }
    HILOG_INFO("%{public}s success.", __func__);
#endif
}

void AbilityManagerService::SubscribeBundleEventCallback()
{
    HILOG_DEBUG("SubscribeBundleEventCallback to receive hap updates.");
    if (abilityBundleEventCallback_) {
        return;
    }

    // Register abilityBundleEventCallback to receive hap updates
    abilityBundleEventCallback_ =
        new (std::nothrow) AbilityBundleEventCallback(taskHandler_, abilityAutoStartupService_);
    auto bms = GetBundleManager();
    if (bms) {
        bool ret = IN_PROCESS_CALL(bms->RegisterBundleEventCallback(abilityBundleEventCallback_));
        if (!ret) {
            HILOG_ERROR("RegisterBundleEventCallback failed!");
        }
    } else {
        HILOG_ERROR("Get BundleManager failed!");
    }
    HILOG_DEBUG("SubscribeBundleEventCallback success.");
}

void AbilityManagerService::UnsubscribeBundleEventCallback()
{
    if (!abilityBundleEventCallback_) {
        return;
    }
    abilityBundleEventCallback_ = nullptr;
    HILOG_DEBUG("UnsubscribeBundleEventCallback success.");
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

void AbilityManagerService::ReportAbilitAssociatedStartInfoToRSS(
    const AppExecFwk::AbilityInfo &abilityInfo, int64_t type, const sptr<IRemoteObject> &callerToken)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (callerToken == nullptr) {
        HILOG_WARN("associated start caller token is nullptr");
        return;
    }

    auto callerAbility = Token::GetAbilityRecordByToken(callerToken);
    if (callerAbility == nullptr) {
        HILOG_WARN("associated start caller  ability is nullptr");
        return;
    }
    int32_t callerUid = callerAbility->GetUid();
    int32_t callerPid = callerAbility->GetPid();
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "associated_start" },
        { "caller_uid", std::to_string(callerUid) },
        { "caller_pid", std::to_string(callerPid) },
        { "callee_uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "callee_bundle_name", abilityInfo.applicationInfo.bundleName }
    };
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        ResourceSchedule::ResType::RES_TYPE_APP_ASSOCIATED_START, type, eventParams);
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

void AbilityManagerService::RegisterSuspendObserver()
{
#ifdef EFFICIENCY_MANAGER_ENABLE
    if (!taskHandler_) {
        HILOG_ERROR("taskhandler null");
        return;
    }
    taskHandler_->SubmitTask([taskHandler = taskHandler_]() {
            ProcessFrozenStateObserver::RegisterSuspendObserver(taskHandler);
        });
#endif // EFFICIENCY_MANAGER_ENABLE
}

int AbilityManagerService::StartExtensionAbility(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
{
    InsightIntentExecuteParam::RemoveInsightIntent(const_cast<Want &>(want));
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        return StartExtensionAbilityInner(want, callerToken, userId, extensionType, false);
    }
    return StartExtensionAbilityInner(want, callerToken, userId, extensionType, true);
}

int AbilityManagerService::RequestModalUIExtension(const Want &want)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return RequestModalUIExtensionInner(want);
}

int AbilityManagerService::RequestModalUIExtensionInner(const Want &want)
{
    sptr<IRemoteObject> token = nullptr;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        HILOG_ERROR("token is nullptr.");
        return ERR_INVALID_VALUE;
    }

    // Gets the record corresponding to the current focus appliaction
    auto record = Token::GetAbilityRecordByToken(token);
    if (!record) {
        HILOG_ERROR("Record is nullptr.");
        return ERR_INVALID_VALUE;
    }

    // Gets the abilityName, bundleName, modulename corresponding to the current focus appliaction
    EventInfo focusInfo;
    focusInfo.bundleName = record->GetAbilityInfo().bundleName;
    focusInfo.abilityName = record->GetAbilityInfo().name;

    // Gets the abilityName, bundleName, modulename corresponding to the caller appliaction
    EventInfo callerInfo;
    callerInfo.bundleName = want.GetParams().GetStringParam("bundleName");
    HILOG_INFO("focusbundlname: %{public}s, callerbundlname: %{public}s.",
        focusInfo.bundleName.c_str(), callerInfo.bundleName.c_str());

    // Compare
    if (focusInfo.bundleName == callerInfo.bundleName) {
        HILOG_DEBUG("CreateModalUIExtension is called!");
        return record->CreateModalUIExtension(want);
    }

    HILOG_DEBUG("Window Modal System Create UIExtension is called!");
    auto connection = std::make_shared<Rosen::ModalSystemUiExtension>();
    return connection->CreateModalUIExtension(want);
}

int AbilityManagerService::StartExtensionAbilityInner(const Want &want, const sptr<IRemoteObject> &callerToken,
    int32_t userId, AppExecFwk::ExtensionAbilityType extensionType, bool checkSystemCaller)
{
    HILOG_INFO("Start extension ability come, bundlename: %{public}s, ability is %{public}s, userId is %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    if (checkSystemCaller) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }
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
        eventInfo.errCode = result;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
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

    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, 0, GetUserId(), false, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    auto connectManager = GetConnectManagerByUserId(validUserId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", validUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }
    UpdateCallerInfo(abilityRequest.want, callerToken);
    HILOG_INFO("Start extension begin, name is %{public}s.", abilityInfo.name.c_str());
    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    ReportAbilitAssociatedStartInfoToRSS(abilityRequest.abilityInfo, static_cast<int64_t>(
        ResourceSchedule::ResType::AssociatedStartType::EXTENSION_START_ABILITY), callerToken);
    return eventInfo.errCode;
}

void AbilityManagerService::SetPickerElementName(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    CHECK_POINTER_IS_NULLPTR(extensionSessionInfo);
    std::string targetType = extensionSessionInfo->want.GetStringParam(UIEXTENSION_TARGET_TYPE_KEY);
    if (extensionSessionInfo->want.GetElement().GetBundleName().empty() &&
        extensionSessionInfo->want.GetElement().GetAbilityName().empty() && !targetType.empty()) {
        std::string abilityName;
        std::string bundleName;
        std::string pickerType;
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        auto pickerMap = AmsConfigurationParameter::GetInstance().GetPickerMap();
        auto it = pickerMap.find(targetType);
        if (it == pickerMap.end()) {
            HILOG_ERROR("can not find the targetType: %{public}s", targetType.c_str());
            return;
        }
        pickerType = it->second;
        auto bms = GetBundleManager();
        CHECK_POINTER(bms);
        int32_t validUserId = GetValidUserId(userId);
        HILOG_INFO("targetType: %{public}s, pickerType: %{public}s, userId: %{public}d",
            targetType.c_str(), pickerType.c_str(), validUserId);
        auto ret = IN_PROCESS_CALL(bms->QueryExtensionAbilityInfosOnlyWithTypeName(pickerType,
            static_cast<int32_t>(AppExecFwk::GetExtensionAbilityInfoFlag::GET_EXTENSION_ABILITY_INFO_WITH_PERMISSION),
            validUserId,
            extensionInfos));
        if (ret != ERR_OK) {
            HILOG_ERROR("QueryExtensionAbilityInfosOnlyWithTypeName failed");
            return;
        }
        abilityName = extensionInfos[0].name;
        bundleName = extensionInfos[0].bundleName;
        HILOG_INFO("abilityName: %{public}s, bundleName: %{public}s", abilityName.c_str(), bundleName.c_str());
        extensionSessionInfo->want.SetElementName(bundleName, abilityName);
        WantParams &parameters = const_cast<WantParams &>(extensionSessionInfo->want.GetParams());
        parameters.SetParam(UIEXTENSION_TYPE_KEY, AAFwk::String::Box(pickerType));
        extensionSessionInfo->want.SetParams(parameters);
    }
}

void AbilityManagerService::SetAutoFillElementName(const sptr<SessionInfo> &extensionSessionInfo)
{
    HILOG_DEBUG("Called.");
    CHECK_POINTER_IS_NULLPTR(extensionSessionInfo);
    if (extensionSessionInfo->want.GetStringParam(UIEXTENSION_TYPE_KEY) != AUTO_FILL_PASSWORD_TPYE) {
        return;
    }
    std::vector<std::string> argList;
    SplitStr(KEY_AUTO_FILL_ABILITY, "/", argList);
    if (argList.size() != ARGC_THREE) {
        HILOG_ERROR("Parse auto fill extension element name failed.");
        return;
    }
    extensionSessionInfo->want.SetElementName(argList[INDEX_ZERO], argList[INDEX_TWO]);
    extensionSessionInfo->want.SetModuleName(argList[INDEX_ONE]);
}

int AbilityManagerService::StartUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    HILOG_DEBUG("Start ui extension ability come");
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    SetPickerElementName(extensionSessionInfo, userId);
    SetAutoFillElementName(extensionSessionInfo);
    std::string extensionTypeStr = extensionSessionInfo->want.GetStringParam(UIEXTENSION_TYPE_KEY);
    AppExecFwk::ExtensionAbilityType extensionType = extensionTypeStr.empty() ?
        AppExecFwk::ExtensionAbilityType::UI : AppExecFwk::ConvertToExtensionAbilityType(extensionTypeStr);
    EventInfo eventInfo = BuildEventInfo(extensionSessionInfo->want, userId);
    eventInfo.extensionType = static_cast<int32_t>(extensionType);
    EventReport::SendExtensionEvent(EventName::START_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);

    if (InsightIntentExecuteParam::IsInsightIntentExecute(extensionSessionInfo->want)) {
        int32_t result = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateWant(
            extensionSessionInfo->want, AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY);
        if (result != ERR_OK) {
            return result;
        }
    }

    sptr<IRemoteObject> callerToken = extensionSessionInfo->callerToken;

    if (!DlpUtils::OtherAppsAccessDlpCheck(callerToken, extensionSessionInfo->want) ||
        VerifyAccountPermission(userId) == CHECK_PERMISSION_FAILED ||
        !DlpUtils::DlpAccessOtherAppsCheck(callerToken, extensionSessionInfo->want)) {
        HILOG_ERROR("StartUIExtensionAbility: Permission verification failed.");
        eventInfo.errCode = CHECK_PERMISSION_FAILED;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return CHECK_PERMISSION_FAILED;
    }

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        HILOG_ERROR("StartUIExtensionAbility VerificationAllToken failed.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (callerRecord == nullptr || !JudgeSelfCalled(callerRecord)) {
        HILOG_ERROR("invalid callerToken.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(extensionSessionInfo->want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    if (ImplicitStartProcessor::IsImplicitStartAction(extensionSessionInfo->want)) {
        HILOG_ERROR("UI extension ability donot support implicit start.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_VALUE;
    }

    AbilityRequest abilityRequest;
    abilityRequest.Voluation(extensionSessionInfo->want, DEFAULT_INVAL_VALUE, callerToken);
    abilityRequest.callType = AbilityCallType::START_EXTENSION_TYPE;
    abilityRequest.sessionInfo = extensionSessionInfo;
    result = GenerateEmbeddableUIAbilityRequest(extensionSessionInfo->want, abilityRequest, callerToken, validUserId);
    CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);
    abilityRequest.sessionInfo->uiExtensionComponentId = (
        static_cast<uint64_t>(callerRecord->GetRecordId()) << OFFSET) |
        static_cast<uint64_t>(abilityRequest.sessionInfo->persistentId);
    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        abilityRequest.sessionInfo->uiExtensionComponentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }
    abilityRequest.extensionType = abilityRequest.abilityInfo.extensionAbilityType;

    auto abilityInfo = abilityRequest.abilityInfo;
    validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : validUserId;
    HILOG_DEBUG("userId is : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckOptExtensionAbility(extensionSessionInfo->want, abilityRequest, validUserId, extensionType);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckOptExtensionAbility error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    sptr<IRemoteObject> parentToken = extensionSessionInfo->parentToken;
    if (parentToken && parentToken != callerToken) {
        UpdateCallerInfoFromToken(abilityRequest.want, parentToken);
    }

    result = JudgeAbilityVisibleControl(abilityInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("JudgeAbilityVisibleControl error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, 0, GetUserId(), false, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
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
        if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission()) {
            HILOG_ERROR("VerificationAllToken failed.");
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
            return ERR_INVALID_CALLER;
        }
        HILOG_DEBUG("Caller is specific system ability.");
    }

    int32_t validUserId = GetValidUserId(userId);
    if (!JudgeMultiUserConcurrency(validUserId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_CROSS_USER;
    }

    if (callerToken != nullptr && CheckIfOperateRemote(want)) {
        auto callerUid = IPCSkeleton::GetCallingUid();
        uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
        DistributedClient dmsClient;
        return dmsClient.StopRemoteExtensionAbility(want, callerUid, accessToken, eventInfo.extensionType);
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

void AbilityManagerService::StopSwitchUserDialog()
{
    HILOG_DEBUG("Stop switch user dialog extension ability come");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_ERROR("Scene board enabled.");
        return;
    }

    if (userController_ == nullptr || userController_->GetFreezingNewUserId() == DEFAULT_INVAL_VALUE) {
        HILOG_ERROR("Get last userId error.");
        return;
    }

    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (sysDialog == nullptr) {
        HILOG_ERROR("System dialog scheduler instance is nullptr.");
        return;
    }

    if (!PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        HILOG_ERROR("The caller is not system-app, can not use system-api");
        return;
    }

    Want stopWant = sysDialog->GetSwitchUserDialogWant();
    StopSwitchUserDialogInner(stopWant, userController_->GetFreezingNewUserId());
    userController_->SetFreezingNewUserId(DEFAULT_INVAL_VALUE);
    return;
}

void AbilityManagerService::StopSwitchUserDialogInner(const Want &want, const int32_t lastUserId)
{
    HILOG_DEBUG("Stop switch user dialog inner come");
    EventInfo eventInfo = BuildEventInfo(want, lastUserId);
    eventInfo.extensionType = static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SERVICE);
    EventReport::SendExtensionEvent(EventName::STOP_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);
    AbilityRequest abilityRequest;
    auto result =
        GenerateExtensionAbilityRequest(want, abilityRequest, nullptr, lastUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    auto stopUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : lastUserId;
    result = CheckOptExtensionAbility(want, abilityRequest, stopUserId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != ERR_OK) {
        HILOG_ERROR("Check extensionAbility type error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto connectManager = GetConnectManagerByUserId(stopUserId);
    if (connectManager == nullptr) {
        HILOG_ERROR("ConnectManager is nullptr. userId:%{public}d", stopUserId);
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    eventInfo.errCode = connectManager->StopServiceAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        HILOG_ERROR("EventInfo errCode is %{public}d", eventInfo.errCode);
        EventReport::SendExtensionEvent(EventName::STOP_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

int AbilityManagerService::MoveAbilityToBackground(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Move ability to background begin");
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
    auto missionListManager = GetListManagerByUserId(ownerUserId);
    if (!missionListManager) {
        HILOG_ERROR("missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
        return ERR_INVALID_VALUE;
    }
    return missionListManager->MoveAbilityToBackground(abilityRecord);
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
    if (missionListManager) {
        NotifyHandleAbilityStateChange(token, TERMINATE_ABILITY_CODE);
        return missionListManager->TerminateAbility(abilityRecord, resultCode, resultWant, flag);
    }
    HILOG_WARN("missionListManager is Null. ownerUserId=%{public}d", ownerUserId);
    if (uiAbilityLifecycleManager_ && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return uiAbilityLifecycleManager_->CloseUIAbility(abilityRecord, resultCode, resultWant, false);
    }
    return ERR_INVALID_VALUE;
}

int AbilityManagerService::TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
    const Want *resultWant)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Terminate ui extension ability begin.");
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(extensionSessionInfo->callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::shared_ptr<AbilityConnectManager> connectManager;
    std::shared_ptr<AbilityRecord> targetRecord;
    GetConnectManagerAndUIExtensionBySessionInfo(extensionSessionInfo, connectManager, targetRecord);
    CHECK_POINTER_AND_RETURN(targetRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_INVALID_VALUE);

    if (!JudgeSelfCalled(targetRecord) && !JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto result = JudgeAbilityVisibleControl(targetRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("JudgeAbilityVisibleControl error.");
        return result;
    }

    if (!UIExtensionUtils::IsUIExtension(targetRecord->GetAbilityInfo().extensionAbilityType)) {
        HILOG_ERROR("Cannot terminate except ui extension ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    extensionSessionInfo->uiExtensionComponentId = (
        static_cast<uint64_t>(abilityRecord->GetRecordId()) << OFFSET) |
        static_cast<uint64_t>(extensionSessionInfo->persistentId);
    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        extensionSessionInfo->uiExtensionComponentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    connectManager->TerminateAbilityWindowLocked(targetRecord, extensionSessionInfo);
    return ERR_OK;
}

int AbilityManagerService::CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!uiAbilityLifecycleManager_) {
        HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("close session: %{public}d, resultCode: %{public}d", sessionInfo->persistentId, sessionInfo->resultCode);
    auto abilityRecord = uiAbilityLifecycleManager_->GetUIAbilityRecordBySessionInfo(sessionInfo);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    if (sessionInfo->isClearSession) {
        const auto &abilityInfo = abilityRecord->GetAbilityInfo();
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            DeleteAbilityRecoverInfo(abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name);
    }

    EventInfo eventInfo;
    eventInfo.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    eventInfo.abilityName = abilityRecord->GetAbilityInfo().name;
    EventReport::SendAbilityEvent(EventName::CLOSE_UI_ABILITY_BY_SCB, HiSysEventType::BEHAVIOR, eventInfo);
    eventInfo.errCode = uiAbilityLifecycleManager_->CloseUIAbility(abilityRecord, sessionInfo->resultCode,
        &(sessionInfo->want), sessionInfo->isClearSession);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendAbilityEvent(EventName::CLOSE_UI_ABILITY_BY_SCB_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
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
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRecord = uiAbilityLifecycleManager_->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    abilityRecord->SetResult(std::make_shared<AbilityResult>(requestCode, resultCode, resultWant));
    abilityRecord->SendResult(0, 0);
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
        int32_t missionId = -1;
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            missionId = uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(callerToken);
            if (!missionId) {
                HILOG_ERROR("Invalid missionId id.");
                return ERR_INVALID_VALUE;
            }
        } else {
            missionId = GetMissionIdByAbilityToken(callerToken);
        }
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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

int AbilityManagerService::MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
    bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Minimize ui extension ability, fromUser:%{public}d.", fromUser);
    CHECK_POINTER_AND_RETURN(extensionSessionInfo, ERR_INVALID_VALUE);
    auto abilityRecord = Token::GetAbilityRecordByToken(extensionSessionInfo->callerToken);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    std::shared_ptr<AbilityConnectManager> connectManager;
    std::shared_ptr<AbilityRecord> targetRecord;
    GetConnectManagerAndUIExtensionBySessionInfo(extensionSessionInfo, connectManager, targetRecord);
    CHECK_POINTER_AND_RETURN(targetRecord, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_INVALID_VALUE);

    auto result = JudgeAbilityVisibleControl(targetRecord->GetAbilityInfo());
    if (result != ERR_OK) {
        HILOG_ERROR("JudgeAbilityVisibleControl error.");
        return result;
    }

    if (!UIExtensionUtils::IsUIExtension(targetRecord->GetAbilityInfo().extensionAbilityType)) {
        HILOG_ERROR("Cannot minimize except ui extension ability.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    extensionSessionInfo->uiExtensionComponentId = (
        static_cast<uint64_t>(abilityRecord->GetRecordId()) << OFFSET) |
        static_cast<uint64_t>(extensionSessionInfo->persistentId);
    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        extensionSessionInfo->uiExtensionComponentId, extensionSessionInfo->want.GetElement().GetURI().c_str());
    connectManager->BackgroundAbilityWindowLocked(targetRecord, extensionSessionInfo);
    return ERR_OK;
}

int AbilityManagerService::MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!uiAbilityLifecycleManager_) {
        HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = uiAbilityLifecycleManager_->GetUIAbilityRecordBySessionInfo(sessionInfo);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!IsAbilityControllerForeground(abilityRecord->GetAbilityInfo().bundleName)) {
        return ERR_WOULD_BLOCK;
    }

    return uiAbilityLifecycleManager_->MinimizeUIAbility(abilityRecord, fromUser);
}

int AbilityManagerService::ConnectAbility(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    return ConnectAbilityCommon(want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
}

int AbilityManagerService::ConnectAbilityCommon(
    const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    AppExecFwk::ExtensionAbilityType extensionType, int32_t userId, bool isQueryExtensionOnly)
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
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
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
    AbilityRequest abilityRequest;
    std::string uri = abilityWant.GetUri().ToString();
    if (!uri.empty()) {
        // if the want include uri, it may only has uri information. it is probably a datashare extension.
        HILOG_INFO("%{public}s called. uri:%{public}s, userId %{public}d", __func__, uri.c_str(), validUserId);
        AppExecFwk::ExtensionAbilityInfo extensionInfo;
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

        abilityWant.SetParam("abilityConnectionObj", connect->AsObject());
        ComponentRequest componentRequest = initComponentRequest(callerToken);
        if (!IsComponentInterceptionStart(abilityWant, componentRequest, abilityRequest)) {
            return componentRequest.requestResult;
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
    UpdateCallerInfo(abilityWant, callerToken);

    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        HILOG_INFO("%{public}s invalid Token.", __func__);
        eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, nullptr, extensionType);
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, callerToken, extensionType, nullptr,
        isQueryExtensionOnly);
    if (eventInfo.errCode != ERR_OK) {
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
}

int AbilityManagerService::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Connect ui extension called, bundlename: %{public}s, ability is %{public}s, userId is %{pravite}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);

    if (IsCrossUserCall(userId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }

    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);
    sptr<IRemoteObject> callerToken = sessionInfo->callerToken;

    if (callerToken != nullptr && !VerificationAllToken(callerToken)) {
        HILOG_ERROR("ConnectUIExtensionAbility VerificationAllToken failed.");
        eventInfo.errCode = ERR_INVALID_VALUE;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_INVALID_CALLER;
    }

    auto result = CheckDlpForExtension(want, callerToken, userId, eventInfo, EventName::CONNECT_SERVICE_ERROR);
    if (result != ERR_OK) {
        HILOG_ERROR("CheckDlpForExtension error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    result = interceptorExecuter_ == nullptr ? ERR_INVALID_VALUE :
        interceptorExecuter_->DoProcess(want, 0, GetUserId(), false);
    if (result != ERR_OK) {
        HILOG_ERROR("interceptorExecuter_ is nullptr or DoProcess return error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        return result;
    }

    int32_t validUserId = GetValidUserId(userId);

    Want abilityWant = want;
    AbilityRequest abilityRequest;
    std::string uri = abilityWant.GetUri().ToString();
    if (!uri.empty()) {
        // if the want include uri, it may only has uri information.
        HILOG_INFO("%{public}s called. uri:%{public}s, userId %{public}d", __func__, uri.c_str(), validUserId);
        AppExecFwk::ExtensionAbilityInfo extensionInfo;
        auto bms = GetBundleManager();
        CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

        abilityWant.SetParam("abilityConnectionObj", connect->AsObject());
        ComponentRequest componentRequest = initComponentRequest(callerToken);
        if (!IsComponentInterceptionStart(abilityWant, componentRequest, abilityRequest)) {
            return componentRequest.requestResult;
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

    UpdateCallerInfo(abilityWant, callerToken);

    if (callerToken != nullptr && callerToken->GetObjectDescriptor() != u"ohos.aafwk.AbilityToken") {
        HILOG_INFO("%{public}s invalid Token.", __func__);
        eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, nullptr,
            AppExecFwk::ExtensionAbilityType::UI, sessionInfo, false, connectInfo);
        if (eventInfo.errCode != ERR_OK) {
            EventReport::SendExtensionEvent(EventName::CONNECT_SERVICE_ERROR, HiSysEventType::FAULT, eventInfo);
        }
        return eventInfo.errCode;
    }
    eventInfo.errCode = ConnectLocalAbility(abilityWant, validUserId, connect, callerToken,
        AppExecFwk::ExtensionAbilityType::UI, sessionInfo, false, connectInfo);
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

int AbilityManagerService::DisconnectAbility(sptr<IAbilityConnection> connect)
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
    AppExecFwk::ExtensionAbilityType extensionType, const sptr<SessionInfo> &sessionInfo,
    bool isQueryExtensionOnly, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Connect local ability begin.");
    bool isEnterpriseAdmin = AAFwk::UIExtensionUtils::IsEnterpriseAdmin(extensionType);
    if (!isEnterpriseAdmin && !JudgeMultiUserConcurrency(userId)) {
        HILOG_ERROR("Multi-user non-concurrent mode is not satisfied.");
        return ERR_CROSS_USER;
    }

    AbilityRequest abilityRequest;
    ErrCode result = ERR_OK;
    if (isQueryExtensionOnly ||
        AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        result = GenerateExtensionAbilityRequest(want, abilityRequest, callerToken, userId);
    } else {
        result = GenerateAbilityRequest(want, DEFAULT_INVAL_VALUE, abilityRequest, callerToken, userId);
    }
    abilityRequest.sessionInfo = sessionInfo;

    Want requestWant = want;
    CHECK_POINTER_AND_RETURN_LOG(connect, ERR_INVALID_VALUE, "connect is nullptr");
    CHECK_POINTER_AND_RETURN_LOG(connect->AsObject(), ERR_INVALID_VALUE, "abilityConnectionObj is nullptr");
    requestWant.SetParam("abilityConnectionObj", connect->AsObject());
    HILOG_DEBUG("requestWant SetParam success");
    ComponentRequest componentRequest = initComponentRequest(callerToken);
    if (!IsComponentInterceptionStart(requestWant, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }

    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }

    if (abilityRequest.abilityInfo.isStageBasedModel) {
        bool isService =
            (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE);
        if (isService && extensionType != AppExecFwk::ExtensionAbilityType::SERVICE) {
            HILOG_ERROR("Service extension type, please use ConnectAbility.");
            return ERR_WRONG_INTERFACE_CALL;
        }
    }
    auto abilityInfo = abilityRequest.abilityInfo;
    int32_t validUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : userId;
    HILOG_DEBUG("validUserId : %{public}d, singleton is : %{public}d",
        validUserId, static_cast<int>(abilityInfo.applicationInfo.singleton));

    result = CheckStaticCfgPermission(abilityInfo, false, -1);
    if (result != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("CheckStaticCfgPermission error, result is %{public}d.", result);
        return ERR_STATIC_CFG_PERMISSION;
    }

    AppExecFwk::ExtensionAbilityType targetExtensionType = abilityInfo.extensionAbilityType;
    HILOG_DEBUG("extension type %{public}d.", targetExtensionType);
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        if (!AAFwk::UIExtensionUtils::IsUIExtension(targetExtensionType)
            && targetExtensionType != AppExecFwk::ExtensionAbilityType::WINDOW) {
            HILOG_ERROR("Try to connect UI extension, but target ability is not UI extension.");
            return ERR_WRONG_INTERFACE_CALL;
        }

        // Cause window has used this api, don't check it when type is window.
        if (targetExtensionType != AppExecFwk::ExtensionAbilityType::WINDOW &&
            !PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_CONNECT_UI_EXTENSION_ABILITY)) {
            HILOG_ERROR("Permission %{public}s verification failed.",
                PermissionConstants::PERMISSION_CONNECT_UI_EXTENSION_ABILITY);
            return ERR_PERMISSION_DENIED;
        }
    }

    auto type = abilityInfo.type;
    if (type != AppExecFwk::AbilityType::SERVICE && type != AppExecFwk::AbilityType::EXTENSION) {
        HILOG_ERROR("Connect ability failed, target ability is not Service.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, 0, GetUserId(), false, callerToken);
    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
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
    return connectManager->ConnectAbilityLocked(abilityRequest, connect, callerToken, sessionInfo, connectInfo);
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
    auto connectManager = GetConnectManagerByUserId(U0_USER_ID);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    if (connectManager->DisconnectAbilityLocked(connect) == ERR_OK) {
        return ERR_OK;
    }

    // EnterpriseAdminExtensionAbility Scene
    connectManager = GetConnectManagerByUserId(USER_ID_DEFAULT);
    CHECK_POINTER_AND_RETURN(connectManager, ERR_NO_INIT);
    return connectManager->DisconnectAbilityLocked(connect);
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
    HILOG_INFO("amsServ %{public}s called.", __func__);
    HILOG_INFO("ContinueMission missionId: %{public}d", missionId);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    DistributedClient dmsClient;
    return dmsClient.ContinueMission(srcDeviceId, dstDeviceId, missionId, callBack, wantParams);
}

int AbilityManagerService::ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
    const std::string &bundleName, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams)
{
    HILOG_INFO("amsServ %{public}s called.", __func__);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    DistributedClient dmsClient;
    return dmsClient.ContinueMission(srcDeviceId, dstDeviceId, bundleName, callBack, wantParams);
}

int AbilityManagerService::ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode)
{
    HILOG_INFO("ContinueAbility missionId = %{public}d, version = %{public}u.", missionId, versionCode);

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRecord = uiAbilityLifecycleManager_->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
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
    int32_t missionId = -1;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        missionId = uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(abilityToken);
        if (!missionId) {
            HILOG_ERROR("Invalid missionId id.");
            return ERR_INVALID_VALUE;
        }
    } else {
        missionId = GetMissionIdByAbilityToken(abilityToken);
    }
    if (missionId < 0) {
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

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRecord = uiAbilityLifecycleManager_->GetAbilityRecordsById(missionId);
    } else {
        sptr<IRemoteObject> abilityToken = GetAbilityTokenByMissionId(missionId);
        CHECK_POINTER_AND_RETURN(abilityToken, ERR_INVALID_VALUE);
        abilityRecord = Token::GetAbilityRecordByToken(abilityToken);
    }
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

int AbilityManagerService::GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData)
{
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        HILOG_ERROR("can not get connection data if caller is not sa.");
        return CHECK_PERMISSION_FAILED;
    }
    DelayedSingleton<ConnectionStateManager>::GetInstance()->GetConnectionData(connectionData);

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

int AbilityManagerService::RegisterOnListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterOnListener(type, listener->AsObject());
}

int AbilityManagerService::RegisterOffListener(const std::string &type,
    const sptr<IRemoteOnListener> &listener)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    CHECK_POINTER_AND_RETURN(listener, ERR_INVALID_VALUE);
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    DistributedClient dmsClient;
    return dmsClient.RegisterOffListener(type, listener->AsObject());
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

    bool isSystemApp = false;
    if (!wantSenderInfo.bundleName.empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(wantSenderInfo.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (bundleMgrResult) {
            isSystemApp = bundleInfo.applicationInfo.isSystemApp;
        }
    }

    HILOG_INFO("AbilityManagerService::GetWantSender: bundleName = %{public}s", wantSenderInfo.bundleName.c_str());
    return pendingWantManager_->GetWantSender(callerUid, appUid, isSystemApp, wantSenderInfo, callerToken);
}

int AbilityManagerService::SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo)
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

    sptr<IRemoteObject> obj = sender->AsObject();
    if (!obj || obj->IsProxyObject()) {
        HILOG_ERROR("obj is nullptr or obj is a proxy obj.");
        return;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    sptr<PendingWantRecord> record = iface_cast<PendingWantRecord>(obj);

    int userId = -1;
    if (DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        GetOsAccountLocalIdFromUid(callerUid, userId) != 0) {
        HILOG_ERROR("GetOsAccountLocalIdFromUid failed. uid=%{public}d", callerUid);
        return;
    }
    bool isSystemApp = false;
    if (record->GetKey() != nullptr && !record->GetKey()->GetBundleName().empty()) {
        AppExecFwk::BundleInfo bundleInfo;
        bool bundleMgrResult = IN_PROCESS_CALL(bms->GetBundleInfo(record->GetKey()->GetBundleName(),
            AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
        if (!bundleMgrResult) {
            HILOG_ERROR("GetBundleInfo is fail.");
            return;
        }
        isSystemApp = bundleInfo.applicationInfo.isSystemApp;
    }

    pendingWantManager_->CancelWantSender(isSystemApp, sender);
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

void AbilityManagerService::RegisterCancelListener(const sptr<IWantSender> &sender,
    const sptr<IWantReceiver> &receiver)
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

void AbilityManagerService::SetLockedState(int32_t sessionId, bool lockedState)
{
    HILOG_INFO("request lock abilityRecord, sessionId :%{public}d", sessionId);
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return;
    }
    auto abilityRecord = uiAbilityLifecycleManager_->GetAbilityRecordsById(sessionId);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is null.");
        return;
    }
    abilityRecord->SetLockedState(lockedState);
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
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }

    if (!IsAbilityControllerStartById(missionId)) {
        HILOG_ERROR("IsAbilityControllerStart false");
        return ERR_WOULD_BLOCK;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (!uiAbilityLifecycleManager_) {
            HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
            return ERR_INVALID_VALUE;
        }
        return uiAbilityLifecycleManager_->MoveMissionToFront(missionId);
    }

    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    return currentMissionListManager_->MoveMissionToFront(missionId);
}

int AbilityManagerService::MoveMissionToFront(int32_t missionId, const StartOptions &startOptions)
{
    HILOG_INFO("request MoveMissionToFront, missionId:%{public}d", missionId);
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (!uiAbilityLifecycleManager_) {
            HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
            return ERR_INVALID_VALUE;
        }
        return uiAbilityLifecycleManager_->MoveMissionToFront(missionId, options);
    }
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);

    return currentMissionListManager_->MoveMissionToFront(missionId, options);
}

int AbilityManagerService::MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    if (wmsHandler_) {
        auto ret = wmsHandler_->MoveMissionsToForeground(missionIds, topMissionId);
        if (ret) {
            HILOG_ERROR("MoveMissionsToForeground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
    return ERR_NO_INIT;
}

int AbilityManagerService::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!PermissionVerification::GetInstance()->VerifyMissionPermission()) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return CHECK_PERMISSION_FAILED;
    }
    if (wmsHandler_) {
        auto ret = wmsHandler_->MoveMissionsToBackground(missionIds, result);
        if (ret) {
            HILOG_ERROR("MoveMissionsToBackground failed, missiondIds may be invalid");
            return ERR_INVALID_VALUE;
        } else {
            return NO_ERROR;
        }
    }
    return ERR_NO_INIT;
}

int32_t AbilityManagerService::GetMissionIdByToken(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("request GetMissionIdByToken.");
    if (!token) {
        HILOG_ERROR("token is invalid.");
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
    HILOG_INFO("called. userId %{public}d", userId);
    bool queryResult = IN_PROCESS_CALL(bms->QueryAbilityInfoByUri(dataAbilityUri, userId, abilityRequest.abilityInfo));
    if (!queryResult || abilityRequest.abilityInfo.name.empty() || abilityRequest.abilityInfo.bundleName.empty()) {
        HILOG_ERROR("Invalid ability info for data ability acquiring.");
        return nullptr;
    }

    abilityRequest.callerToken = callerToken;
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall && CheckCallDataAbilityPermission(abilityRequest, isShellCall, isSaCall) != ERR_OK) {
        HILOG_ERROR("Invalid ability request info for data ability acquiring.");
        return nullptr;
    }

    HILOG_DEBUG("Query data ability info: %{public}s|%{public}s|%{public}s",
        abilityRequest.appInfo.name.c_str(), abilityRequest.appInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());

    if (CheckStaticCfgPermission(abilityRequest.abilityInfo, false, -1, true, isSaCall) !=
        AppExecFwk::Constants::PERMISSION_GRANTED) {
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
    bool isNotHap = isSaCall || isShellCall;
    UpdateCallerInfo(abilityRequest.want, callerToken);
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
    HILOG_INFO("LoadLifecycle: Attach ability thread.");
    CHECK_POINTER_AND_RETURN(scheduler, ERR_INVALID_VALUE);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !VerificationAllToken(token)) {
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
        HILOG_WARN("force timeout ability for test, state:INITIAL, ability: %{public}s", abilityInfo.name.c_str());
        return ERR_OK;
    }
    if (type == AppExecFwk::AbilityType::SERVICE || type == AppExecFwk::AbilityType::EXTENSION) {
        auto connectManager = GetConnectManagerByUserId(userId);
        if (!connectManager) {
            HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return connectManager->AttachAbilityThreadLocked(scheduler, token);
    } else if (type == AppExecFwk::AbilityType::DATA) {
        auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
        if (!dataAbilityManager) {
            HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
            return ERR_INVALID_VALUE;
        }
        return dataAbilityManager->AttachAbilityThread(scheduler, token);
    } else {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::LOAD };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) + "; AbilityManagerService::AttachAbilityThread;" +
            " the end of load lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
            return uiAbilityLifecycleManager_->AttachAbilityThread(scheduler, token);
        }
        int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
        auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
        if (!missionListManager) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->AttachAbilityThread(scheduler, token);
    }
}

void AbilityManagerService::DumpFuncInit()
{
    dumpFuncMap_[KEY_DUMP_SERVICE] = &AbilityManagerService::DumpStateInner;
    dumpFuncMap_[KEY_DUMP_DATA] = &AbilityManagerService::DataDumpStateInner;

    dumpFuncMap_[KEY_DUMP_ALL] = &AbilityManagerService::DumpInner;
    dumpFuncMap_[KEY_DUMP_MISSION] = &AbilityManagerService::DumpMissionInner;
    dumpFuncMap_[KEY_DUMP_MISSION_LIST] = &AbilityManagerService::DumpMissionListInner;
    dumpFuncMap_[KEY_DUMP_MISSION_INFOS] = &AbilityManagerService::DumpMissionInfosInner;
}

void AbilityManagerService::DumpSysFuncInit()
{
    dumpsysFuncMap_[KEY_DUMPSYS_ALL] = &AbilityManagerService::DumpSysInner;
    dumpsysFuncMap_[KEY_DUMPSYS_SERVICE] = &AbilityManagerService::DumpSysStateInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PENDING] = &AbilityManagerService::DumpSysPendingInner;
    dumpsysFuncMap_[KEY_DUMPSYS_PROCESS] = &AbilityManagerService::DumpSysProcess;
    dumpsysFuncMap_[KEY_DUMPSYS_DATA] = &AbilityManagerService::DataDumpSysStateInner;

    dumpsysFuncMap_[KEY_DUMPSYS_MISSION_LIST] = &AbilityManagerService::DumpSysMissionListInner;
    dumpsysFuncMap_[KEY_DUMPSYS_ABILITY] = &AbilityManagerService::DumpSysAbilityInner;
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        DumpSysMissionListInnerBySCB(args, info, isClient, isUserID, userId);
        return;
    }
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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

void AbilityManagerService::DumpSysMissionListInnerBySCB(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (isUserID) {
        if (!CheckUserIdActive(userId)) {
            info.push_back("error: No user found.");
            return;
        }
    } else {
        userId = GetUserId();
    }

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return;
    }

    if (!uiAbilityLifecycleManager_) {
        HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
        return;
    }

    if (argList.size() == MIN_DUMP_ARGUMENT_NUM) {
        uiAbilityLifecycleManager_->DumpMissionList(info, isClient, userId, argList[1]);
    } else if (argList.size() < MIN_DUMP_ARGUMENT_NUM) {
        uiAbilityLifecycleManager_->DumpMissionList(info, isClient, userId);
    } else {
        info.emplace_back("error: invalid argument, please see 'hidumper -s AbilityManagerService -a '-h''.");
    }
}

void AbilityManagerService::DumpSysAbilityInner(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        DumpSysAbilityInnerBySCB(args, info, isClient, isUserID, userId);
        return;
    }
    std::shared_ptr<MissionListManager> targetManager;
    if (isUserID) {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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

void AbilityManagerService::DumpSysAbilityInnerBySCB(
    const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId)
{
    if (isUserID) {
        if (!CheckUserIdActive(userId)) {
            info.push_back("error: No user found.");
            return;
        }
    } else {
        userId = GetUserId();
    }

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

            if (!uiAbilityLifecycleManager_) {
                HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
                return;
            }
            uiAbilityLifecycleManager_->DumpMissionListByRecordId(info, isClient, abilityId, params, userId);
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
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (!uiAbilityLifecycleManager_) {
            HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
            return;
        }
        uiAbilityLifecycleManager_->Dump(info);
        return;
    }

    if (currentMissionListManager_) {
        currentMissionListManager_->Dump(info);
    }
}

void AbilityManagerService::DumpMissionListInner(const std::string &args, std::vector<std::string> &info)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (!uiAbilityLifecycleManager_) {
            HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
            return;
        }
        uiAbilityLifecycleManager_->DumpMissionList(info, false, GetUserId(), " ");
        return;
    }
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionList(info, false, "");
    }
}

void AbilityManagerService::DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_INFO("call");
        Rosen::WindowManager::GetInstance().DumpSessionAll(info);
        return;
    }
    if (currentMissionListManager_) {
        currentMissionListManager_->DumpMissionInfos(info);
    }
}

void AbilityManagerService::DumpMissionInner(const std::string &args, std::vector<std::string> &info)
{
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

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_INFO("call");
        Rosen::WindowManager::GetInstance().DumpSessionWithId(missionId, info);
        return;
    }

    CHECK_POINTER_LOG(currentMissionListManager_, "Current mission manager not init.");
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
    HILOG_INFO("Lifecycle: state:%{public}d.", state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && !VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    HILOG_INFO("Lifecycle: bundle: %{public}s, ability: %{public}s.", abilityInfo.bundleName.c_str(),
        abilityInfo.name.c_str());
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

    if (targetState == AbilityState::BACKGROUND) {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::BACKGROUND };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) +
            "; AbilityManagerService::AbilityTransitionDone; the end of background lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    } else if (targetState != AbilityState::INITIAL) {
        FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::FOREGROUND };
        auto entry = std::to_string(AbilityUtil::SystemTimeMillis()) +
            "; AbilityManagerService::AbilityTransitionDone; the end of foreground lifecycle.";
        entry += " the end of foreground lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return uiAbilityLifecycleManager_->AbilityTransactionDone(token, state, saveData);
    } else {
        int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
        auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
        if (!missionListManager) {
            HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
            return ERR_INVALID_VALUE;
        }
        return missionListManager->AbilityTransactionDone(token, state, saveData);
    }
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

int AbilityManagerService::ScheduleCommandAbilityWindowDone(
    const sptr<IRemoteObject> &token,
    const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("enter.");
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (!UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)
        && !UIExtensionUtils::IsWindowExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        HILOG_ERROR("target ability is not ui or window extension.");
        return ERR_INVALID_VALUE;
    }
    auto userId = abilityRecord->GetApplicationInfo().uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (!connectManager) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return ERR_INVALID_VALUE;
    }
    return connectManager->ScheduleCommandAbilityWindowDone(token, sessionInfo, winCmd, abilityCmd);
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
        case AppExecFwk::AbilityType::DATA: {
            auto dataAbilityManager = GetDataAbilityManagerByUserId(userId);
            if (!dataAbilityManager) {
                HILOG_ERROR("dataAbilityManager is Null. userId=%{public}d", userId);
                return;
            }
            dataAbilityManager->OnAbilityRequestDone(token, state);
            break;
        }
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
        default: {
            if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
                uiAbilityLifecycleManager_->OnAbilityRequestDone(token, state);
            } else {
                int32_t ownerMissionUserId = abilityRecord->GetOwnerMissionUserId();
                auto missionListManager = GetListManagerByUserId(ownerMissionUserId);
                if (!missionListManager) {
                    HILOG_ERROR("missionListManager is Null. userId=%{public}d", ownerMissionUserId);
                    return;
                }
                missionListManager->OnAbilityRequestDone(token, state);
            }
            break;
        }
    }
}

void AbilityManagerService::OnAppStateChanged(const AppInfo &info)
{
    HILOG_INFO("On app state changed.");
    CHECK_POINTER_LOG(connectManager_, "Connect manager not init.");
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        CHECK_POINTER_LOG(currentMissionListManager_, "Current mission list manager not init.");
    }
    connectManager_->OnAppStateChanged(info);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnAppStateChanged(info, GetUserId());
    } else {
        currentMissionListManager_->OnAppStateChanged(info);
    }
    dataAbilityManager_->OnAppStateChanged(info);
}

std::shared_ptr<AbilityEventHandler> AbilityManagerService::GetEventHandler()
{
    return eventHandler_;
}

void AbilityManagerService::InitMissionListManager(int userId, bool switchUser)
{
    bool find = false;
    {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        std::unique_lock<ffrt::mutex> lock(managersMutex_);
        missionListManagers_.emplace(userId, manager);
        if (switchUser) {
            currentMissionListManager_ = manager;
        }
    }
}

// multi user scene
int32_t AbilityManagerService::GetUserId() const
{
    if (userController_) {
        auto userId = userController_->GetCurrentUserId();
        HILOG_DEBUG("userId is %{public}d", userId);
        return userId;
    }
    return U0_USER_ID;
}

void AbilityManagerService::StartHighestPriorityAbility(int32_t userId, bool isBoot)
{
    HILOG_DEBUG("%{public}s", __func__);
    auto bms = GetBundleManager();
    CHECK_POINTER(bms);

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
        ComponentRequest componentRequest = initComponentRequest();
        if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
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
    WaitParameter(BOOTEVENT_BOOT_ANIMATION_STARTED.c_str(), "true",
        AmsConfigurationParameter::GetInstance().GetBootAnimationTimeoutTime());
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

    if (abilityRecord != nullptr) {
        (const_cast<Want &>(want)).SetParam(DEBUG_APP, abilityRecord->IsDebugApp());
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
            IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag,
                userId, extensionInfos));
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
        request.extensionProcessMode = extensionInfo.extensionProcessMode;
        HILOG_DEBUG("Extension ability info found, name=%{public}s.",
            extensionInfo.name.c_str());
        // For compatibility translates to AbilityInfo
        InitAbilityInfoFromExtension(extensionInfo, request.abilityInfo);
    }
    HILOG_DEBUG("QueryAbilityInfo success, ability name: %{public}s, is stage mode: %{public}d.",
        request.abilityInfo.name.c_str(), request.abilityInfo.isStageBasedModel);

    if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE)) {
        request.collaboratorType = CollaboratorType::RESERVE_TYPE;
    } else if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE)) {
        request.collaboratorType = CollaboratorType::OTHERS_TYPE;
    }
    if (request.collaboratorType != CollaboratorType::DEFAULT_TYPE &&
        !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto collaborator = GetCollaborator(request.collaboratorType);
        if (collaborator == nullptr) {
            HILOG_ERROR("collaborator GetCollaborator is nullptr.");
            return ERR_COLLABORATOR_NOT_REGISTER;
        }

        uint64_t accessTokenIDEx = IPCSkeleton::GetCallingFullTokenID();

        int32_t ret = collaborator->NotifyStartAbility(request.abilityInfo, userId, request.want, accessTokenIDEx);
        if (ret != ERR_OK) {
            HILOG_ERROR("collaborator: notify broker start ability failed");
            return ERR_COLLABORATOR_NOTIFY_FAILED;
        }

        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryAbilityInfo(request.want, abilityInfoFlag, userId, request.abilityInfo));

        if (request.want.GetBoolParam(KEY_VISIBLE_ID, false) && !request.abilityInfo.visible) {
            request.abilityInfo.visible = true;
            HILOG_DEBUG("request.abilityInfo.visible set true");
        }

        HILOG_INFO("collaborator notify broker start ability success");
    }

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
    HILOG_DEBUG("GenerateAbilityRequest end, app name: %{public}s, moduleName name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.abilityInfo.moduleName.c_str(), request.uid);

    request.want.SetModuleName(request.abilityInfo.moduleName);

    if (want.GetBoolParam(Want::PARAM_RESV_START_RECENT, false) &&
        AAFwk::PermissionVerification::GetInstance()->VerifyStartRecentAbilityPermission()) {
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
    request.extensionProcessMode = extensionInfo.extensionProcessMode;

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

int AbilityManagerService::StopServiceAbility(const Want &want, int32_t userId, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("call.");

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        auto abilityRecord = Token::GetAbilityRecordByToken(token);
        if (abilityRecord == nullptr) {
            HILOG_ERROR("callerRecord is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

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

    auto res = JudgeAbilityVisibleControl(abilityInfo);
    if (res != ERR_OK) {
        HILOG_ERROR("Target ability is invisible");
        return res;
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
            uiAbilityLifecycleManager_->OnAbilityDied(abilityRecord);
            return;
        }
    } else {
        auto manager = GetListManagerByUserId(abilityRecord->GetOwnerMissionUserId());
        if (manager && abilityRecord->GetAbilityInfo().type == AbilityType::PAGE) {
            ReleaseAbilityTokenMap(abilityRecord->GetToken());
            manager->OnAbilityDied(abilityRecord, GetUserId());
            return;
        }
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnCallConnectDied(callRecord);
        return;
    }
    if (currentMissionListManager_) {
        currentMissionListManager_->OnCallConnectDied(callRecord);
    }
}

void AbilityManagerService::ReleaseAbilityTokenMap(const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
    for (auto iter = callStubTokenMap_.begin(); iter != callStubTokenMap_.end(); iter++) {
        if (iter->second == token) {
            callStubTokenMap_.erase(iter);
            break;
        }
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

int AbilityManagerService::ClearUpApplicationData(const std::string &bundleName, const int32_t userId)
{
    HILOG_DEBUG("ClearUpApplicationData, bundleName: %{public}s", bundleName.c_str());
    CHECK_CALLER_IS_SYSTEM_APP;
    int ret = DelayedSingleton<AppScheduler>::GetInstance()->ClearUpApplicationData(bundleName, userId);
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->UninstallApp(bundleName, uid, targetUserId);
    } else if (targetUserId == U0_USER_ID) {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(bundleName);
    return ERR_OK;
}

std::shared_ptr<AppExecFwk::BundleMgrHelper> AbilityManagerService::GetBundleManager()
{
    if (bundleMgrHelper_ == nullptr) {
        bundleMgrHelper_ = AbilityUtil::GetBundleManagerHelper();
    }
    return bundleMgrHelper_;
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

    HILOG_DEBUG("App data abilities preloading for bundle '%{public}s'...", bundleName.data());

    auto begin = system_clock::now();
    AbilityRequest dataAbilityRequest;
    UpdateCallerInfo(dataAbilityRequest.want, nullptr);
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
        HILOG_INFO("App data ability preloading: '%{public}s.%{public}s'...",
            it->bundleName.c_str(), it->name.c_str());

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

void AbilityManagerService::HandleLoadTimeOut(int64_t abilityRecordId, bool isHalf)
{
    HILOG_DEBUG("Handle load timeout.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordId, isHalf);
        return;
    }
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecordId, isHalf);
        }
    }
}

void AbilityManagerService::HandleActiveTimeOut(int64_t abilityRecordId)
{
    HILOG_DEBUG("Handle active timeout.");
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::ACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void AbilityManagerService::HandleInactiveTimeOut(int64_t abilityRecordId)
{
    HILOG_DEBUG("Handle inactive timeout.");
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }

    for (auto& item : connectManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecordId);
        }
    }
}

void AbilityManagerService::HandleForegroundTimeOut(int64_t abilityRecordId, bool isHalf)
{
    HILOG_DEBUG("Handle foreground timeout.");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecordId, isHalf);
        return;
    }
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item : missionListManagers_) {
        if (item.second) {
            item.second->OnTimeOut(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecordId, isHalf);
        }
    }
}

void AbilityManagerService::HandleShareDataTimeOut(int64_t uniqueId)
{
    WantParams wantParam;
    int32_t ret = GetShareDataPairAndReturnData(nullptr, ERR_TIMED_OUT, uniqueId, wantParam);
    if (ret) {
        HILOG_ERROR("acqurieShareData failed.");
    }
}

int32_t AbilityManagerService::GetShareDataPairAndReturnData(std::shared_ptr<AbilityRecord> abilityRecord,
    const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
{
    HILOG_INFO("resultCode:%{public}d, uniqueId:%{public}d, wantParam size:%{public}d.",
        resultCode, uniqueId, wantParam.Size());
    auto it = iAcquireShareDataMap_.find(uniqueId);
    if (it != iAcquireShareDataMap_.end()) {
        auto shareDataPair = it->second;
        if (abilityRecord && shareDataPair.first != abilityRecord->GetAbilityRecordId()) {
            HILOG_ERROR("abilityRecord is not the abilityRecord from request.");
            return ERR_INVALID_VALUE;
        }
        auto callback = shareDataPair.second;
        if (!callback) {
            HILOG_ERROR("callback object is nullptr.");
            return ERR_INVALID_VALUE;
        }
        auto ret = callback->AcquireShareDataDone(resultCode, wantParam);
        iAcquireShareDataMap_.erase(it);
        return ret;
    }
    HILOG_ERROR("iAcquireShareData is null.");
    return ERR_INVALID_VALUE;
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

    if (connectManager_->GetExtensionByTokenFromServiceMap(token)) {
        HILOG_INFO("Verification token5.");
        return true;
    }

    if (connectManager_->GetExtensionByTokenFromTerminatingMap(token)) {
        HILOG_INFO("Verification token5.");
        return true;
    }

    HILOG_ERROR("Failed to verify token.");
    return false;
}

bool AbilityManagerService::VerificationAllToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("VerificationAllToken.");
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        if (uiAbilityLifecycleManager_ != nullptr && uiAbilityLifecycleManager_->IsContainsAbility(token)) {
            return true;
        }
    } else {
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
            if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
                return true;
            }
            if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
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

    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto& item: dataAbilityManagers_) {
        if (item.second && item.second->ContainsDataAbility(scheduler)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<MissionListManager> AbilityManagerService::GetListManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = missionListManagers_.find(userId);
    if (it != missionListManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<AbilityConnectManager> AbilityManagerService::GetConnectManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    auto it = connectManagers_.find(userId);
    if (it != connectManagers_.end()) {
        return it->second;
    }
    HILOG_ERROR("%{public}s, Failed to get Manager. UserId = %{public}d", __func__, userId);
    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByUserId(int32_t userId)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
    for (auto item: connectManagers_) {
        if (item.second && item.second->GetExtensionByTokenFromServiceMap(token)) {
            return item.second;
        }
        if (item.second && item.second->GetExtensionByTokenFromTerminatingMap(token)) {
            return item.second;
        }
    }

    return nullptr;
}

std::shared_ptr<DataAbilityManager> AbilityManagerService::GetDataAbilityManagerByToken(
    const sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
            AmsConfigurationParameter::GetInstance().GetBootAnimationTimeoutTime());
#endif
        DelayedSingleton<ResidentProcessManager>::GetInstance()->StartResidentProcess(bundleInfos);
    }
}

void AbilityManagerService::StartAutoStartupApps()
{
    HILOG_DEBUG("Called.");
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return;
    }
    std::vector<AutoStartupInfo> infoList;
    int32_t result = abilityAutoStartupService_->QueryAllAutoStartupApplicationsWithoutPermission(infoList);
    if (result != ERR_OK) {
        HILOG_ERROR("Failed to query data.");
        return;
    }

    constexpr int retryCount = 5;
    RetryStartAutoStartupApps(infoList, retryCount);
}

void AbilityManagerService::RetryStartAutoStartupApps(
    const std::vector<AutoStartupInfo> &infoList, int32_t retryCount)
{
    HILOG_DEBUG("Called, retryCount: %{public}d, infoList.size:%{public}zu", retryCount, infoList.size());
    std::vector<AutoStartupInfo> failedList;
    for (auto info : infoList) {
        AppExecFwk::ElementName element;
        element.SetBundleName(info.bundleName);
        element.SetAbilityName(info.abilityName);
        element.SetModuleName(info.moduleName);
        Want want;
        want.SetElement(element);
        want.SetParam(Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON, true);
        if (StartAbility(want) != ERR_OK) {
            failedList.push_back(info);
        }
    }

    HILOG_DEBUG("RetryCount: %{public}d, failedList.size:%{public}zu", retryCount, failedList.size());
    if (!failedList.empty() && retryCount > 0) {
        auto retryStartAutoStartupAppsTask = [aams = weak_from_this(), list = failedList, retryCount]() {
            auto obj = aams.lock();
            if (obj == nullptr) {
                HILOG_ERROR("Retry start auto startup app error, obj is nullptr");
                return;
            }
            obj->RetryStartAutoStartupApps(list, retryCount - 1);
        };
        constexpr int delaytime = 2000;
        taskHandler_->SubmitTask(retryStartAutoStartupAppsTask, "RetryStartAutoStartupApps", delaytime);
    }
}

void AbilityManagerService::SubscribeScreenUnlockedEvent()
{
    HILOG_DEBUG("Called.");
    // add listen screen unlocked.
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    auto callback = [abilityManager = weak_from_this()]() {
        HILOG_DEBUG("On screen unlocked.");
        auto abilityMgr = abilityManager.lock();
        if (abilityMgr == nullptr) {
            HILOG_ERROR("Invalid abilityMgr pointer.");
            return;
        }
        auto taskHandler = abilityMgr->GetTaskHandler();
        if (taskHandler == nullptr) {
            HILOG_ERROR("Invalid taskHandler pointer.");
            return;
        }
        auto startAutoStartupAppsTask = [abilityManager]() {
            auto abilityMgr = abilityManager.lock();
            if (abilityMgr == nullptr) {
                HILOG_ERROR("Invalid abilityMgr pointer.");
                return;
            }
            abilityMgr->StartAutoStartupApps();
            abilityMgr->UnSubscribeScreenUnlockedEvent();
        };
        taskHandler->SubmitTask(startAutoStartupAppsTask, "StartAutoStartupApps");
    };
    screenSubscriber_ = std::make_shared<AbilityRuntime::AbilityManagerEventSubscriber>(subscribeInfo, callback);
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(screenSubscriber_);
    if (!subResult) {
        constexpr int retryCount = 20;
        RetrySubscribeScreenUnlockedEvent(retryCount);
    }
}

void AbilityManagerService::UnSubscribeScreenUnlockedEvent()
{
    HILOG_DEBUG("Called.");
    bool subResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(screenSubscriber_);
    HILOG_DEBUG("Screen unlocked event subscriber unsubscribe result is %{public}d.", subResult);
}

void AbilityManagerService::RetrySubscribeScreenUnlockedEvent(int32_t retryCount)
{
    HILOG_DEBUG("RetryCount: %{public}d.", retryCount);
    auto retrySubscribeScreenUnlockedEventTask = [aams = weak_from_this(), screenSubscriber = screenSubscriber_,
                                                     retryCount]() {
        bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(screenSubscriber);
        auto obj = aams.lock();
        if (obj == nullptr) {
            HILOG_ERROR("Retry subscribe screen unlocked event, obj is nullptr.");
            return;
        }
        if (!subResult && retryCount > 0) {
            obj->RetrySubscribeScreenUnlockedEvent(retryCount - 1);
        }
    };
    constexpr int delaytime = 200;
    taskHandler_->SubmitTask(retrySubscribeScreenUnlockedEventTask, "RetrySubscribeScreenUnlockedEvent", delaytime);
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
    std::lock_guard<ffrt::mutex> guard(globalLock_);
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

int32_t AbilityManagerService::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is Null.");
        return -1;
    }

    if (!JudgeSelfCalled(abilityRecord) && (IPCSkeleton::GetCallingPid() != getpid())) {
        return -1;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(token);
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
    int32_t missionId = -1;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        missionId = uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(callerToken);
        if (!missionId) {
            HILOG_ERROR("Invalid missionId id.");
            return ERR_INVALID_VALUE;
        }
    } else {
        missionId = GetMissionIdByAbilityToken(callerToken);
    }
    if (missionId < 0) {
        return ERR_INVALID_VALUE;
    }
    remoteWant.SetParam(DMS_MISSION_ID, missionId);
    DistributedClient dmsClient;
    return dmsClient.StartRemoteAbilityByCall(remoteWant, connect);
}

int AbilityManagerService::ReleaseRemoteAbility(const sptr<IRemoteObject> &connect,
    const AppExecFwk::ElementName &element)
{
    DistributedClient dmsClient;
    return dmsClient.ReleaseRemoteAbility(connect, element);
}

int AbilityManagerService::StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, int32_t accountId)
{
    HILOG_INFO("call ability.");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    if (IsCrossUserCall(accountId)) {
        CHECK_CALLER_IS_SYSTEM_APP;
    }

    if (VerifyAccountPermission(accountId) == CHECK_PERMISSION_FAILED) {
        HILOG_ERROR("%{public}s: Permission verification failed.", __func__);
        return CHECK_PERMISSION_FAILED;
    }

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

    int32_t oriValidUserId = GetValidUserId(accountId);
    if (!JudgeMultiUserConcurrency(oriValidUserId)) {
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
    ComponentRequest componentRequest = initComponentRequest(callerToken, -1, result);
    if (CheckProxyComponent(want, result) && !IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
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
    UpdateCallerInfo(abilityRequest.want, callerToken);
    result = afterCheckExecuter_ == nullptr ? ERR_INVALID_VALUE :
        afterCheckExecuter_->DoProcess(abilityRequest.want, 0, GetUserId(), false, callerToken);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToSuspendManager(abilityRequest.abilityInfo);
        abilityRequest.want.SetParam(IS_CALL_BY_SCB, false);
        return uiAbilityLifecycleManager_->ResolveLocked(abilityRequest, oriValidUserId);
    }

    auto missionListMgr = GetListManagerByUserId(oriValidUserId);
    if (missionListMgr == nullptr) {
        HILOG_ERROR("missionListMgr is Null. Designated User Id=%{public}d", oriValidUserId);
        return ERR_INVALID_VALUE;
    }
    ReportEventToSuspendManager(abilityRequest.abilityInfo);
    if (!IsComponentInterceptionStart(want, componentRequest, abilityRequest)) {
        return componentRequest.requestResult;
    }

    if (result != ERR_OK) {
        HILOG_ERROR("afterCheckExecuter_ is nullptr or DoProcess return error.");
        return result;
    }

    return missionListMgr->ResolveLocked(abilityRequest);
}

int AbilityManagerService::StartAbilityJust(AbilityRequest &abilityRequest, int32_t validUserId)
{
    HILOG_DEBUG("call");
    UpdateCallerInfo(abilityRequest.want, abilityRequest.callerToken);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        ReportEventToSuspendManager(abilityRequest.abilityInfo);
        return uiAbilityLifecycleManager_->ResolveLocked(abilityRequest, validUserId);
    }

    auto missionListMgr = GetListManagerByUserId(validUserId);
    if (missionListMgr == nullptr) {
        HILOG_ERROR("missionListMgr is Null. Designated User Id=%{public}d", validUserId);
        return ERR_INVALID_VALUE;
    }
    ReportEventToSuspendManager(abilityRequest.abilityInfo);

    return missionListMgr->ResolveLocked(abilityRequest);
}

int AbilityManagerService::ReleaseCall(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_DEBUG("Release called ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    }

    std::string elementName = element.GetURI();
    HILOG_DEBUG("try to release called ability, name: %{public}s.", elementName.c_str());

    if (CheckIsRemote(element.GetDeviceID())) {
        HILOG_INFO("release remote ability");
        return ReleaseRemoteAbility(connect->AsObject(), element);
    }

    int result = ERR_OK;
    if (IsReleaseCallInterception(connect, element, result)) {
        return result;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return uiAbilityLifecycleManager_->ReleaseCallLocked(connect, element);
    }

    return currentMissionListManager_->ReleaseCallLocked(connect, element);
}

int AbilityManagerService::JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Call.");
    if (abilityInfo.visible) {
        return ERR_OK;
    }
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    if (callerTokenId == abilityInfo.applicationInfo.accessTokenId ||
        callerTokenId == static_cast<uint32_t>(IPCSkeleton::GetSelfTokenID())) {  // foundation call is allowed
        return ERR_OK;
    }
    if (AccessTokenKit::VerifyAccessToken(callerTokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY) == AppExecFwk::Constants::PERMISSION_GRANTED) {
        return ERR_OK;
    }
    if (AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
        return ERR_OK;
    }
    HILOG_ERROR("callerToken: %{private}u, targetToken: %{private}u, caller doesn's have permission",
        callerTokenId, abilityInfo.applicationInfo.accessTokenId);
    return ABILITY_VISIBLE_FALSE_DENY_REQUEST;
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

int AbilityManagerService::LogoutUser(int32_t userId)
{
    if (IPCSkeleton::GetCallingUid() != ACCOUNT_MGR_SERVICE_UID) {
        HILOG_ERROR("Permission verification failed, not account process");
        return CHECK_PERMISSION_FAILED;
    }

    if (userController_) {
        auto ret = userController_->LogoutUser(userId);
        HILOG_DEBUG("logout user return = %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

void AbilityManagerService::OnAcceptWantResponse(
    const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("On accept want response");
    if (uiAbilityLifecycleManager_ && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnAcceptWantResponse(want, flag);
        return;
    }
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnAcceptWantResponse(want, flag);
}

void AbilityManagerService::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (uiAbilityLifecycleManager_ && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnStartSpecifiedAbilityTimeoutResponse(want);
        return;
    }
    if (!currentMissionListManager_) {
        return;
    }
    currentMissionListManager_->OnStartSpecifiedAbilityTimeoutResponse(want);
}

void AbilityManagerService::OnStartSpecifiedProcessResponse(const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("flag = %{public}s", flag.c_str());
    if (uiAbilityLifecycleManager_ && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnStartSpecifiedProcessResponse(want, flag);
        return;
    }
}

void AbilityManagerService::OnStartSpecifiedProcessTimeoutResponse(const AAFwk::Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (uiAbilityLifecycleManager_ && Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->OnStartSpecifiedAbilityTimeoutResponse(want);
        return;
    }
}

int AbilityManagerService::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info)
{
    HILOG_DEBUG("Get running ability infos.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->GetAbilityRunningInfos(info, isPerm, GetUserId());
    } else {
        if (!currentMissionListManager_) {
            return ERR_INVALID_VALUE;
        }
        currentMissionListManager_->GetAbilityRunningInfos(info, isPerm);
    }

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
            item.ability == abilityRecord->GetElementName()) {
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
    std::unique_lock<ffrt::mutex> lock(managersMutex_);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        missionListManagers_.erase(userId);
    }
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

void AbilityManagerService::UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
    const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        currentMissionListManager_->UpdateSnapShot(token, pixelMap);
    }
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
    {
        std::lock_guard<ffrt::mutex> guard(globalLock_);
        auto it = appRecoveryHistory_.find(record->GetUid());
        if (it == appRecoveryHistory_.end()) {
            appRecoveryHistory_.emplace(record->GetUid(), 0);
        }
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        const auto& abilityInfo = record->GetAbilityInfo();
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
            abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name,
            uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(token));
    } else {
        auto userId = record->GetOwnerMissionUserId();
        auto missionListMgr = GetListManagerByUserId(userId);
        if (missionListMgr == nullptr) {
            HILOG_ERROR("missionListMgr is nullptr");
            return;
        }
        missionListMgr->EnableRecoverAbility(record->GetMissionId());
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

void AbilityManagerService::ReportAppRecoverResult(const int32_t appId, const AppExecFwk::ApplicationInfo &appInfo,
    const std::string& abilityName, const std::string& result)
{
    HiSysEventWrite(HiSysEvent::Domain::AAFWK, "APP_RECOVERY", HiSysEvent::EventType::BEHAVIOR,
        "APP_UID", appId,
        "VERSION_CODE", std::to_string(appInfo.versionCode),
        "VERSION_NAME", appInfo.versionName,
        "BUNDLE_NAME", appInfo.bundleName,
        "ABILITY_NAME", abilityName,
        "RECOVERY_RESULT", result);
}

void AbilityManagerService::AppRecoverKill(pid_t pid, int32_t reason)
{
    AppExecFwk::AppFaultDataBySA faultDataSA;
    faultDataSA.errorObject.name = "appRecovery";
    switch (reason) {
        case AppExecFwk::StateReason::CPP_CRASH:
            faultDataSA.faultType = AppExecFwk::FaultDataType::CPP_CRASH;
            break;
        case AppExecFwk::StateReason::JS_ERROR:
            faultDataSA.faultType = AppExecFwk::FaultDataType::JS_ERROR;
            break;
        case AppExecFwk::StateReason::LIFECYCLE:
        case AppExecFwk::StateReason::APP_FREEZE:
            faultDataSA.faultType = AppExecFwk::FaultDataType::APP_FREEZE;
            break;
        default:
            faultDataSA.faultType = AppExecFwk::FaultDataType::UNKNOWN;
    }
    faultDataSA.pid = pid;
    IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultDataSA));
}

void AbilityManagerService::ScheduleRecoverAbility(const sptr<IRemoteObject>& token, int32_t reason, const Want *want)
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

    if (reason == AppExecFwk::StateReason::APP_FREEZE) {
        RecordAppExitReason(REASON_APP_FREEZE);
    }

    AAFwk::Want curWant;
    {
        std::lock_guard<ffrt::mutex> guard(globalLock_);
        auto type = record->GetAbilityInfo().type;
        if (type != AppExecFwk::AbilityType::PAGE) {
            HILOG_ERROR("%{public}s AppRecovery::only do recover for page ability.", __func__);
            return;
        }

        constexpr int64_t MIN_RECOVERY_TIME = 60;
        int64_t now = time(nullptr);
        auto it = appRecoveryHistory_.find(record->GetUid());
        auto appInfo = record->GetApplicationInfo();
        auto abilityInfo = record->GetAbilityInfo();

        if ((it != appRecoveryHistory_.end()) &&
            (it->second + MIN_RECOVERY_TIME > now)) {
            HILOG_ERROR("%{public}s AppRecovery recover app more than once in one minute, just kill app(%{public}d).",
                __func__, record->GetPid());
            ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_WITHIN_ONE_MINUTE");
            AppRecoverKill(record->GetPid(), reason);
            return;
        }

        if (want != nullptr) {
            HILOG_DEBUG("BundleName:%{public}s targetBundleName:%{public}s.",
                appInfo.bundleName.c_str(), want->GetElement().GetBundleName().c_str());
            if (want->GetElement().GetBundleName().empty() ||
                (appInfo.bundleName.compare(want->GetElement().GetBundleName()) != 0)) {
                HILOG_ERROR("AppRecovery BundleName not match, Not recovery ability!");
                ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_BUNDLE_NAME_NOT_MATCH");
                return;
            } else if (want->GetElement().GetAbilityName().empty()) {
                HILOG_DEBUG("AppRecovery recovery target ability is empty");
                ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "FAIL_TARGET_ABILITY_EMPTY");
                return;
            } else {
                auto bms = GetBundleManager();
                if (bms == nullptr) {
                    HILOG_ERROR("bms is nullptr");
                    return;
                }
                AppExecFwk::BundleInfo bundleInfo;
                auto bundleName = want->GetElement().GetBundleName();
                int32_t userId = GetUserId();
                bool ret = IN_PROCESS_CALL(
                    bms->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo,
                    userId));
                if (!ret) {
                    HILOG_ERROR("AppRecovery Failed to get bundle info, not do recovery!");
                    return;
                }
                bool isRestartPage = false;
                auto abilityName = want->GetElement().GetAbilityName();
                for (auto it = bundleInfo.abilityInfos.begin(); it != bundleInfo.abilityInfos.end(); ++it) {
                    if ((abilityName.compare(it->name) == 0) && it->type == AppExecFwk::AbilityType::PAGE) {
                        isRestartPage = true;
                        break;
                    }
                }
                if (!isRestartPage) {
                    HILOG_INFO("AppRecovery the target ability type is not PAGE!");
                    ReportAppRecoverResult(record->GetUid(), appInfo, abilityName, "FAIL_TARGET_ABILITY_NOT_PAGE");
                    return;
                }
            }
        }

        appRecoveryHistory_[record->GetUid()] = now;
        curWant = (want == nullptr) ? record->GetWant() : *want;
        curWant.SetParam(AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART, true);

        ReportAppRecoverResult(record->GetUid(), appInfo, abilityInfo.name, "SUCCESS");
        AppRecoverKill(record->GetPid(), reason);
    }

    constexpr int delaytime = 1000;
    std::string taskName = "AppRecovery_kill:" + std::to_string(record->GetPid());
    auto task = std::bind(&AbilityManagerService::RecoverAbilityRestart, this, curWant);
    HILOG_INFO("AppRecovery RecoverAbilityRestart task begin");
    taskHandler_->SubmitTask(task, taskName, delaytime);
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

void AbilityManagerService::StartSwitchUserDialog()
{
    HILOG_DEBUG("Start switch user dialog extension ability come");
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_ERROR("Scene board enabled, dialog not show.");
        return;
    }

    if (userController_ == nullptr) {
        HILOG_ERROR("User Controller instance is nullptr.");
        return;
    }

    auto sysDialog = DelayedSingleton<SystemDialogScheduler>::GetInstance();
    if (sysDialog == nullptr) {
        HILOG_ERROR("System dialog scheduler instance is nullptr.");
        return;
    }

    Want dialogWant = sysDialog->GetSwitchUserDialogWant();
    StartSwitchUserDialogInner(dialogWant, userController_->GetFreezingNewUserId());
}


void AbilityManagerService::StartSwitchUserDialogInner(const Want &want, int32_t lastUserId)
{
    HILOG_DEBUG("Start switch user dialog inner come");
    EventInfo eventInfo = BuildEventInfo(want, lastUserId);
    eventInfo.extensionType = static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SERVICE);
    EventReport::SendExtensionEvent(EventName::START_SERVICE, HiSysEventType::BEHAVIOR, eventInfo);
    AbilityRequest abilityRequest;
    auto result = GenerateExtensionAbilityRequest(want, abilityRequest, nullptr, lastUserId);
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request local error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto abilityInfo = abilityRequest.abilityInfo;
    auto startUserId = abilityInfo.applicationInfo.singleton ? U0_USER_ID : lastUserId;
    result = CheckOptExtensionAbility(want, abilityRequest, startUserId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (result != ERR_OK) {
        HILOG_ERROR("Check extensionAbility type error.");
        eventInfo.errCode = result;
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
        return;
    }

    auto connectManager = GetConnectManagerByUserId(startUserId);
    if (connectManager == nullptr) {
        InitConnectManager(startUserId, false);
        connectManager = GetConnectManagerByUserId(startUserId);
        if (connectManager == nullptr) {
            HILOG_ERROR("ConnectManager is nullptr. userId=%{public}d", startUserId);
            eventInfo.errCode = ERR_INVALID_VALUE;
            EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
            return;
        }
    }

    eventInfo.errCode = connectManager->StartAbility(abilityRequest);
    if (eventInfo.errCode != ERR_OK) {
        HILOG_ERROR("EventInfo errCode is %{public}d", eventInfo.errCode);
        EventReport::SendExtensionEvent(EventName::START_EXTENSION_ERROR, HiSysEventType::FAULT, eventInfo);
    }
}

void AbilityManagerService::StartFreezingScreen()
{
    HILOG_INFO("%{public}s", __func__);
#ifdef SUPPORT_GRAPHICS
    StartSwitchUserDialog();
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
    StopSwitchUserDialog();
#endif
}

void AbilityManagerService::UserStarted(int32_t userId)
{
    HILOG_INFO("%{public}s", __func__);
    InitConnectManager(userId, false);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        InitMissionListManager(userId, false);
    }
    InitDataAbilityManager(userId, false);
    InitPendWantManager(userId, false);
}

void AbilityManagerService::SwitchToUser(int32_t oldUserId, int32_t userId)
{
    HILOG_INFO("%{public}s, oldUserId:%{public}d, newUserId:%{public}d", __func__, oldUserId, userId);
    SwitchManagers(userId);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        PauseOldUser(oldUserId);
        ConnectBmsService();
        StartUserApps();
    }
    bool isBoot = oldUserId == U0_USER_ID ? true : false;
    StartHighestPriorityAbility(userId, isBoot);
    PauseOldConnectManager(oldUserId);
}

void AbilityManagerService::SwitchManagers(int32_t userId, bool switchUser)
{
    HILOG_INFO("%{public}s, SwitchManagers:%{public}d-----begin", __func__, userId);
    InitConnectManager(userId, switchUser);
    if (userId != U0_USER_ID && !Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
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
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
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

    std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
    manager->PauseExtensions();
    HILOG_INFO("%{public}s, PauseOldConnectManager:%{public}d-----end", __func__, userId);
}

void AbilityManagerService::StartUserApps()
{
    if (currentMissionListManager_ && currentMissionListManager_->IsStarted()) {
        HILOG_INFO("missionListManager ResumeManager");
        currentMissionListManager_->ResumeManager();
    }
}

void AbilityManagerService::InitConnectManager(int32_t userId, bool switchUser)
{
    bool find = false;
    {
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        manager->SetTaskHandler(taskHandler_);
        manager->SetEventHandler(eventHandler_);
        std::unique_lock<ffrt::mutex> lock(managersMutex_);
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
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        std::unique_lock<ffrt::mutex> lock(managersMutex_);
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
        std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
        std::unique_lock<ffrt::mutex> lock(managersMutex_);
        pendingWantManagers_.emplace(userId, manager);
        if (switchUser) {
            pendingWantManager_ = manager;
        }
    }
}

int32_t AbilityManagerService::GetValidUserId(const int32_t userId)
{
    HILOG_DEBUG("userId = %{public}d.", userId);
    int32_t validUserId = userId;

    if (DEFAULT_INVAL_VALUE == userId) {
        validUserId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        HILOG_INFO("validUserId = %{public}d, CallingUid = %{public}d.", validUserId,
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

    std::lock_guard<ffrt::mutex> guard(globalLock_);
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
        std::lock_guard<ffrt::mutex> guard(globalLock_);
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
    std::lock_guard<ffrt::mutex> guard(globalLock_);
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
    abilityInfo.extensionTypeName = extensionInfo.extensionTypeName;
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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        Rosen::FocusChangeInfo focusChangeInfo;
        Rosen::WindowManager::GetInstance().GetFocusWindowInfo(focusChangeInfo);
        token = focusChangeInfo.abilityToken_;
    } else {
        if (!wmsHandler_) {
            HILOG_ERROR("wmsHandler_ is nullptr.");
            return ERR_INVALID_VALUE;
        }
        wmsHandler_->GetFocusWindow(token);
    }

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
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto sessionId = uiAbilityLifecycleManager_->GetSessionIdByAbilityToken(token);
        if (!sessionId) {
            HILOG_ERROR("Invalid session id.");
            return ERR_INVALID_VALUE;
        }
        NotifyHandleAbilityStateChange(token, ABILITY_MOVE_TO_FOREGROUND_CODE);
        auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        auto want = abilityRecord->GetWant();
        if (!IsAbilityControllerStart(want, want.GetBundle())) {
            HILOG_ERROR("SceneBoard IsAbilityControllerStart failed: %{public}s", want.GetBundle().c_str());
            return ERR_WOULD_BLOCK;
        }
        return ERR_OK;
    }
    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId < 0) {
        HILOG_ERROR("Invalid mission id.");
        return ERR_INVALID_VALUE;
    }
    NotifyHandleAbilityStateChange(token, ABILITY_MOVE_TO_FOREGROUND_CODE);
    return DelegatorMoveMissionToFront(missionId);
}

int AbilityManagerService::DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("enter");
    NotifyHandleAbilityStateChange(token, ABILITY_MOVE_TO_BACKGROUND_CODE);
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return ERR_OK;
    }
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

    std::lock_guard<ffrt::mutex> guard(globalLock_);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
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

    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    return currentMissionListManager_->DoAbilityForeground(abilityRecord, flag);
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

void AbilityManagerService::UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken)
{
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!abilityRecord) {
        HILOG_WARN("%{public}s caller abilityRecord is null.", __func__);
        want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, std::string(""));
        want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, std::string(""));
    } else {
        std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
        want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
        std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
        want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
        want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
    }
}

void AbilityManagerService::UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken)
{
    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(asCallerSourceToken, processInfo);

    auto abilityRecord = Token::GetAbilityRecordByToken(asCallerSourceToken);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;

    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, processInfo.uid_);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, processInfo.pid_);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
}

void AbilityManagerService::UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_WARN("caller abilityRecord is null.");
        return;
    }

    int32_t tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    int32_t callerUid = abilityRecord->GetUid();
    int32_t callerPid = abilityRecord->GetPid();
    want.RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, tokenId);
    want.RemoveParam(Want::PARAM_RESV_CALLER_UID);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, callerUid);
    want.RemoveParam(Want::PARAM_RESV_CALLER_PID);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, callerPid);

    std::string callerBundleName = abilityRecord->GetAbilityInfo().bundleName;
    want.RemoveParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    std::string callerAbilityName = abilityRecord->GetAbilityInfo().name;
    want.RemoveParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, callerAbilityName);
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

int AbilityManagerService::CheckStaticCfgPermission(AppExecFwk::AbilityInfo &abilityInfo, bool isStartAsCaller,
    uint32_t callerTokenId, bool isData, bool isSaCall)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData) {
        isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    }
    if (isSaCall) {
        // do not need check static config permission when start ability by SA
        return AppExecFwk::Constants::PERMISSION_GRANTED;
    }

    uint32_t tokenId;
    if (isStartAsCaller) {
        tokenId = callerTokenId;
    } else {
        tokenId = IPCSkeleton::GetCallingTokenID();
    }

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
    if (abilityInfo.permissions.empty() || AccessTokenKit::VerifyAccessToken(tokenId,
        PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY) == ERR_OK) {
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

    runningInfo.ability = abilityRecord->GetElementName();
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
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    if (taskHandler_) {
        HILOG_DEBUG("%{public}s begin post block ams service task", __func__);
        auto BlockAmsServiceTask = [aams = shared_from_this()]() {
            while (1) {
                HILOG_DEBUG("%{public}s begin waiting", __func__);
                std::this_thread::sleep_for(BLOCK_AMS_SERVICE_TIME*1s);
            }
        };
        taskHandler_->SubmitTask(BlockAmsServiceTask, "blockamsservice");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}

int AbilityManagerService::BlockAbility(int32_t abilityRecordId)
{
    HILOG_DEBUG("%{public}s", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        return uiAbilityLifecycleManager_->BlockAbility(abilityRecordId, GetUserId());
    }
    CHECK_POINTER_AND_RETURN(currentMissionListManager_, ERR_NO_INIT);
    return currentMissionListManager_->BlockAbility(abilityRecordId);
}

int AbilityManagerService::BlockAppService()
{
    HILOG_DEBUG("%{public}s", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Not shell call");
        return ERR_PERMISSION_DENIED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->BlockAppService();
}
#endif

int AbilityManagerService::FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
    int32_t userId, int requestCode)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != DMS_UID) {
        HILOG_ERROR("The interface only support for DMS");
        return CHECK_PERMISSION_FAILED;
    }
    int32_t validUserId = GetValidUserId(userId);
    if (freeInstallManager_ == nullptr) {
        HILOG_ERROR("freeInstallManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return freeInstallManager_->FreeInstallAbilityFromRemote(want, callback, validUserId, requestCode);
}

AppExecFwk::ElementName AbilityManagerService::GetTopAbility(bool isNeedLocalDeviceId)
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
    elementName = abilityRecord->GetElementName();
    bool isDeviceEmpty = elementName.GetDeviceID().empty();
    std::string localDeviceId;
    if (isDeviceEmpty && isNeedLocalDeviceId && GetLocalDeviceId(localDeviceId)) {
        elementName.SetDeviceID(localDeviceId);
    }
#endif
    return elementName;
}

AppExecFwk::ElementName AbilityManagerService::GetElementNameByToken(sptr<IRemoteObject> token,
    bool isNeedLocalDeviceId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s start.", __func__);
    AppExecFwk::ElementName elementName = {};
#ifdef SUPPORT_GRAPHICS
    if (!token) {
        HILOG_ERROR("token is nullptr");
        return elementName;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("%{public}s abilityRecord is null.", __func__);
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

int AbilityManagerService::SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
{
    HILOG_DEBUG("SetMissionContinueState begin. State: %{public}d", state);

    CHECK_POINTER_AND_RETURN(token, ERR_INVALID_VALUE);

    int32_t missionId = GetMissionIdByAbilityToken(token);
    if (missionId == -1) {
        HILOG_ERROR("SetMissionContinueState failed to get missionId. State: %{public}d", state);
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("SetMissionContinueState: No such ability record. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        HILOG_ERROR("SetMissionContinueState not self, not enabled. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto userId = abilityRecord->GetOwnerMissionUserId();
    auto missionListManager = GetListManagerByUserId(userId);
    if (!missionListManager) {
        HILOG_ERROR("failed to find mission list manager. Mission id: %{public}d, state: %{public}d",
            missionId, state);
        return -1;
    }

    auto setResult = missionListManager->SetMissionContinueState(token, missionId, state);
    if (setResult != ERR_OK) {
        HILOG_ERROR("missionListManager set failed, result: %{public}d, mission id: %{public}d, state: %{public}d",
            setResult, missionId, state);
        return setResult;
    }

    DistributedClient dmsClient;
    auto result =  dmsClient.SetMissionContinueState(missionId, state);
    if (result != ERR_OK) {
        HILOG_ERROR("Notify DMS client failed, result: %{public}d. Mission id: %{public}d, state: %{public}d",
            result, missionId, state);
    }

    HILOG_DEBUG("SetMissionContinueState end. Mission id: %{public}d, state: %{public}d", missionId, state);
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
    std::lock_guard<ffrt::mutex> lock(managersMutex_);
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
    std::string sharePickerBundleName =
        OHOS::system::GetParameter(SHARE_PICKER_DIALOG_BUNDLE_NAME_KEY, SHARE_PICKER_DIALOG_DEFAULY_BUNDLE_NAME);
    std::string sharePickerAbilityName =
        OHOS::system::GetParameter(SHARE_PICKER_DIALOG_ABILITY_NAME_KEY, SHARE_PICKER_DIALOG_DEFAULY_ABILITY_NAME);
    newWant.SetElementName(sharePickerBundleName, sharePickerAbilityName);
    newWant.SetParam(TOKEN_KEY, callerToken);
    // note: clear actions
    newWant.SetAction("");
    return IN_PROCESS_CALL(StartAbility(newWant, DEFAULT_INVAL_VALUE, userId));
}

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

int AbilityManagerService::PrepareTerminateAbility(const sptr<IRemoteObject> &token,
    sptr<IPrepareTerminateCallback> &callback)
{
    HILOG_DEBUG("call");
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!CheckPrepareTerminateEnable()) {
        callback->DoPrepareTerminate();
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("record is nullptr.");
        callback->DoPrepareTerminate();
        return ERR_INVALID_VALUE;
    }

    if (!JudgeSelfCalled(abilityRecord)) {
        HILOG_ERROR("Not self call.");
        callback->DoPrepareTerminate();
        return CHECK_PERMISSION_FAILED;
    }

    auto type = abilityRecord->GetAbilityInfo().type;
    if (type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Only support PAGE.");
        callback->DoPrepareTerminate();
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    auto timeoutTask = [&callback]() {
        callback->DoPrepareTerminate();
    };
    int prepareTerminateTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * PREPARE_TERMINATE_TIMEOUT_MULTIPLE;
    if (taskHandler_) {
        taskHandler_->SubmitTask(timeoutTask, "PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()),
            prepareTerminateTimeout);
    }

    bool res = abilityRecord->PrepareTerminateAbility();
    if (!res) {
        callback->DoPrepareTerminate();
    }
    taskHandler_->CancelTask("PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    return ERR_OK;
}

void AbilityManagerService::HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    HILOG_INFO("handle focused event");
    if (!currentMissionListManager_) {
        HILOG_ERROR("current mission manager is null");
        return;
    }

    int32_t missionId = GetMissionIdByAbilityToken(focusChangeInfo->abilityToken_);
    currentMissionListManager_->NotifyMissionFocused(missionId);
}

void AbilityManagerService::HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    HILOG_INFO("handle unfocused event");
    if (!currentMissionListManager_) {
        HILOG_ERROR("current mission manager is null");
        return;
    }

    int32_t missionId = GetMissionIdByAbilityToken(focusChangeInfo->abilityToken_);
    currentMissionListManager_->NotifyMissionUnfocused(missionId);
}

void AbilityManagerService::InitFocusListener()
{
    HILOG_INFO("Init ability focus listener");
    if (focusListener_) {
        return;
    }

    focusListener_ = new WindowFocusChangedListener(shared_from_this(), taskHandler_);
    auto registerTask = [innerService = shared_from_this()]() {
        if (innerService) {
            HILOG_INFO("RegisterFocusListener task");
            innerService->RegisterFocusListener();
        }
    };
    if (taskHandler_) {
        taskHandler_->SubmitTask(registerTask, "RegisterFocusListenerTask", REGISTER_FOCUS_DELAY);
    }
}

void AbilityManagerService::RegisterFocusListener()
{
    HILOG_INFO("Register focus listener");
    if (!focusListener_) {
        HILOG_ERROR("no listener obj");
        return;
    }
    Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
    HILOG_INFO("Register focus listener success");
}

void AbilityManagerService::InitPrepareTerminateConfig()
{
    char value[PREPARE_TERMINATE_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(PREPARE_TERMINATE_ENABLE_PARAMETER, "false", value, PREPARE_TERMINATE_ENABLE_SIZE);
    HILOG_INFO("CheckPrepareTerminateEnable, %{public}s value is %{public}s.", PREPARE_TERMINATE_ENABLE_PARAMETER,
        value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        isPrepareTerminateEnable_ = true;
    }
}
#endif

int AbilityManagerService::CheckCallServicePermission(const AbilityRequest &abilityRequest)
{
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

int AbilityManagerService::CheckCallDataAbilityPermission(AbilityRequest &abilityRequest, bool isShell, bool isSACall)
{
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

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest,
        true, isShell, isSACall);
    if (isShell) {
        verificationInfo.isBackgroundCall = true;
    }
    if (!isShell && IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall, true) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo,
        isShell);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start DataAbility");
        return result;
    }

    return ERR_OK;
}

AAFwk::PermissionVerification::VerificationInfo AbilityManagerService::CreateVerificationInfo(
    const AbilityRequest &abilityRequest, bool isData, bool isShell, bool isSA)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    HILOG_DEBUG("Call ServiceAbility or DataAbility, target bundleName: %{public}s.",
        abilityRequest.appInfo.bundleName.c_str());
    if (whiteListassociatedWakeUpFlag_ &&
        WHITE_LIST_ASS_WAKEUP_SET.find(abilityRequest.appInfo.bundleName) != WHITE_LIST_ASS_WAKEUP_SET.end()) {
        HILOG_DEBUG("Call ServiceAbility or DataAbility, target bundle in white-list, allow associatedWakeUp.");
        verificationInfo.associatedWakeUp = true;
    } else {
        verificationInfo.associatedWakeUp = abilityRequest.appInfo.associatedWakeUp;
    }
    if (!isData) {
        isSA = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        isShell = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    }
    if (isSA || isShell) {
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

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        HILOG_ERROR("Do not have permission to start ServiceExtension or DataShareExtension");
    }
    return result;
}

int AbilityManagerService::CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("Call");
    if (IPCSkeleton::GetCallingUid() != BROKER_UID && AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    HILOG_DEBUG("OtherExtension type: %{public}d.", static_cast<int32_t>(extensionType));
    if (extensionType == AppExecFwk::ExtensionAbilityType::WINDOW) {
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::ADS_SERVICE) {
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD) {
        if (!abilityRequest.appInfo.isSystemApp) {
            HILOG_ERROR("The application requesting the call is a non system application.");
            return CHECK_PERMISSION_FAILED;
        }
        std::string jsonDataStr = abilityRequest.want.GetStringParam(WANT_PARAMS_VIEW_DATA_KEY);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(jsonDataStr.c_str());
        if (!CheckCallingTokenId(viewData.bundleName)) {
            HILOG_ERROR("Not %{public}s called, not allowed.", viewData.bundleName.c_str());
            return ERR_WRONG_INTERFACE_CALL;
        }
        return ERR_OK;
    }
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        return ERR_OK;
    }
    const std::string fileAccessPermission = "ohos.permission.FILE_ACCESS_MANAGER";
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION &&
        AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(fileAccessPermission)) {
        HILOG_DEBUG("Temporary, FILEACCESS_EXTENSION use serviceExtension start-up rule.");
        return CheckCallServiceExtensionPermission(abilityRequest);
    }

    HILOG_ERROR("Not SA, can not start other Extension");
    return CHECK_PERMISSION_FAILED;
}


int AbilityManagerService::CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("Call");
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
    HILOG_DEBUG("Call");

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
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
    HILOG_INFO("Call");
    // check whether the target ability is page type and not specified mode.
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::PAGE ||
        abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        HILOG_ERROR("Called ability is not common ability.");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
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

int AbilityManagerService::IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall,
    bool isData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData && AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        isBackgroundCall = true;
        return ERR_OK;
    }

    if (!isData && (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AbilityUtil::IsStartFreeInstall(abilityRequest.want))) {
        isBackgroundCall = false;
        return ERR_OK;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        if (callerAbility->IsForeground() || callerAbility->GetAbilityForegroundingFlag()) {
            isBackgroundCall = false;
            return ERR_OK;
        }
        // CallerAbility is not foreground, so check process state
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(callerAbility->GetToken(), processInfo);
    } else {
        auto callerPid = IPCSkeleton::GetCallingPid();
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (processInfo.processName_.empty() && !AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
            HILOG_DEBUG("Can not find caller application by callerPid: %{private}d.", callerPid);
            if (AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
                HILOG_DEBUG("Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
                isBackgroundCall = false;
                return ERR_OK;
            }
            HILOG_ERROR("Caller does not have PERMISSION_START_ABILITIES_FROM_BACKGROUND, REJECT.");
            return ERR_INVALID_VALUE;
        }
    }

    if (IsDelegatorCall(processInfo, abilityRequest)) {
        HILOG_DEBUG("The call is from AbilityDelegator, allow background-call.");
        isBackgroundCall = false;
        return ERR_OK;
    }

    if (backgroundJudgeFlag_) {
        isBackgroundCall = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND &&
            !processInfo.isFocused && !processInfo.isAbilityForegrounding;
    } else {
        isBackgroundCall = !processInfo.isFocused;
        if (!processInfo.isFocused && processInfo.state_ == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            // Allow background startup within 1 second after application startup if state is FOREGROUND
            int64_t aliveTime = AbilityUtil::SystemTimeMillis() - processInfo.startTimeMillis_;
            isBackgroundCall = aliveTime > APP_ALIVE_TIME_MS;
            HILOG_DEBUG("Process %{public}s is alive %{public}s ms.",
                processInfo.processName_.c_str(), std::to_string(aliveTime).c_str());
        }
    }
    HILOG_DEBUG("backgroundJudgeFlag: %{public}d, isBackgroundCall: %{public}d, callerAppState: %{public}d.",
        static_cast<int32_t>(backgroundJudgeFlag_),
        static_cast<int32_t>(isBackgroundCall),
        static_cast<int32_t>(processInfo.state_));

    return ERR_OK;
}

bool AbilityManagerService::IsTargetPermission(const Want &want) const
{
    if (want.GetElement().GetBundleName() == PERMISSIONMGR_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == PERMISSIONMGR_ABILITY_NAME) {
        return true;
    }

    return false;
}

inline bool AbilityManagerService::IsDelegatorCall(
    const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest)
{
    /*  To make sure the AbilityDelegator is not counterfeited
     *   1. The caller-process must be test-process
     *   2. The callerToken must be nullptr
     */
    if (processInfo.isTestProcess &&
        !abilityRequest.callerToken && abilityRequest.want.GetBoolParam(IS_DELEGATOR_CALL, false)) {
        return true;
    }
    return false;
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
    {
        std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
        callStubTokenMap_[callStub] = token;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER(abilityRecord);
    if (!JudgeSelfCalled(abilityRecord)) {
        return;
    }

    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->CallRequestDone(abilityRecord, callStub);
        return;
    }

    if (!currentMissionListManager_) {
        HILOG_ERROR("currentMissionListManager_ is null.");
        return;
    }
    currentMissionListManager_->CallRequestDone(abilityRecord, callStub);
}

void AbilityManagerService::GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token)
{
    std::lock_guard<ffrt::mutex> autoLock(abilityTokenLock_);
    auto it = callStubTokenMap_.find(callStub);
    if (it == callStubTokenMap_.end()) {
        token = nullptr;
        return;
    }
    token = callStubTokenMap_[callStub];
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
    // check if form frs
    auto callingUid = IPCSkeleton::GetCallingUid();
    std::string bundleName = want.GetBundle();
    if (callingUid == FOUNDATION_UID && FRS_BUNDLE_NAME == bundleName) {
        return ERR_OK;
    }

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

    std::lock_guard<ffrt::mutex> guard(globalLock_);
    componentInterception_ = componentInterception;
    HILOG_DEBUG("%{public}s, end", __func__);
    return ERR_OK;
}

bool AbilityManagerService::IsComponentInterceptionStart(const Want &want, ComponentRequest &componentRequest,
    AbilityRequest &request)
{
    if (componentInterception_ != nullptr) {
        Want newWant = want;
        int32_t type = static_cast<int32_t>(request.abilityInfo.type);
        newWant.SetParam("abilityType", type);
        int32_t launchMode = static_cast<int32_t>(request.abilityInfo.launchMode);
        newWant.SetParam("launchMode", launchMode);
        int32_t callType = static_cast<int32_t>(request.callType);
        newWant.SetParam("callType", callType);
        if (callType == AbilityCallType::CALL_REQUEST_TYPE) {
            newWant.SetParam("abilityConnectionObj", request.connect->AsObject());
        }
        int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
        newWant.SetParam("accessTokenId", tokenId);

        HILOG_DEBUG("%{public}s", __func__);
        sptr<Want> extraParam = new (std::nothrow) Want();
        bool isStart = componentInterception_->AllowComponentStart(newWant, componentRequest.callerToken,
            componentRequest.requestCode, componentRequest.componentStatus, extraParam);
        componentRequest.requestResult = extraParam->GetIntParam("requestResult", 0);
        UpdateAbilityRequestInfo(extraParam, request);
        if (!isStart) {
            HILOG_INFO("not finishing start component because interception, requestResult: %{public}d",
                componentRequest.requestResult);
            return false;
        }
    }
    return true;
}

bool AbilityManagerService::CheckProxyComponent(const Want &want, const int result)
{
    // do proxy component while the deviceId is not empty
    return ((!want.GetElement().GetDeviceID().empty()) || (result != ERR_OK));
}

bool AbilityManagerService::IsReleaseCallInterception(const sptr<IAbilityConnection> &connect,
    const AppExecFwk::ElementName &element, int &result)
{
    if  (componentInterception_ == nullptr) {
        return false;
    }
    HILOG_DEBUG("%{public}s", __func__);
    sptr<Want> extraParam = new (std::nothrow) Want();
    bool isInterception = componentInterception_->ReleaseCallInterception(connect->AsObject(), element, extraParam);
    result = extraParam->GetIntParam("requestResult", 0);
    return isInterception;
}

void AbilityManagerService::NotifyHandleAbilityStateChange(const sptr<IRemoteObject> &abilityToken, int opCode)
{
    if (componentInterception_ != nullptr) {
        componentInterception_->NotifyHandleAbilityStateChange(abilityToken, opCode);
    }
}

ComponentRequest AbilityManagerService::initComponentRequest(const sptr<IRemoteObject> &callerToken,
    const int requestCode, const int componentStatus)
{
    ComponentRequest componentRequest;
    componentRequest.callerToken = callerToken;
    componentRequest.requestCode = requestCode;
    componentRequest.componentStatus = componentStatus;
    return componentRequest;
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
    int32_t procCode = want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
    if (procCode != 0) {
        request.want.SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE, procCode);
    }
    int32_t tokenCode = want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0);
    if (tokenCode != 0) {
        request.want.SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, tokenCode);
    }
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
    abilityRecord->SendResult(0, 0);
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

int AbilityManagerService::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
{
    sptr<IRemoteObject> token;
    auto ret = GetTopAbility(token);
    if (ret != ERR_OK) {
        HILOG_ERROR("GetTopAbility failed");
        return ret;
    }

    bool focused = false;
    int32_t userId = GetValidUserId(DEFAULT_INVAL_VALUE);
    auto connectManager = GetConnectManagerByUserId(userId);
    if (connectManager) {
        focused = connectManager->IsUIExtensionFocused(uiExtensionTokenId, token)
            || connectManager->IsWindowExtensionFocused(uiExtensionTokenId, token);
    } else {
        HILOG_WARN("connectManager is nullptr, userId: %{public}d", userId);
    }
    if (!focused && userId != U0_USER_ID) {
        HILOG_DEBUG("Check connectManager in user0");
        connectManager = GetConnectManagerByUserId(U0_USER_ID);
        if (connectManager) {
            focused = connectManager->IsUIExtensionFocused(uiExtensionTokenId, token)
                || connectManager->IsWindowExtensionFocused(uiExtensionTokenId, token);
        } else {
            HILOG_WARN("connectManager is nullptr, userId: 0");
        }
    }
    isFocused = focused;
    HILOG_DEBUG("isFocused: %{public}d", isFocused);
    return ERR_OK;
}

int AbilityManagerService::AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (freeInstallManager_ == nullptr) {
        HILOG_ERROR("freeInstallManager_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return freeInstallManager_->AddFreeInstallObserver(observer);
}

int32_t AbilityManagerService::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    auto missionlistMgr = GetListManagerByUserId(userId);
    if (missionlistMgr == nullptr) {
        HILOG_ERROR("missionlistMgr is nullptr.");
        return ERR_INVALID_VALUE;
    }

    return missionlistMgr->IsValidMissionIds(missionIds, results);
}

int AbilityManagerService::VerifyPermission(const std::string &permission, int pid, int uid)
{
    HILOG_INFO("permission=%{public}s, pid=%{public}d, uid=%{public}d",
        permission.c_str(),
        pid,
        uid);
    if (permission.empty()) {
        HILOG_ERROR("VerifyPermission permission invalid");
        return CHECK_PERMISSION_FAILED;
    }

    auto bms = GetBundleManager();
    CHECK_POINTER_AND_RETURN(bms, ERR_INVALID_VALUE);

    std::string bundleName;
    if (IN_PROCESS_CALL(bms->GetNameForUid(uid, bundleName)) != ERR_OK) {
        HILOG_ERROR("VerifyPermission failed to get bundle name by uid");
        return CHECK_PERMISSION_FAILED;
    }

    int account = -1;
    DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->GetOsAccountLocalIdFromUid(uid, account);
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        account, appInfo))) {
        HILOG_ERROR("VerifyPermission failed to get application info");
        return CHECK_PERMISSION_FAILED;
    }

    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        HILOG_ERROR("VerifyPermission %{public}d: PERMISSION_DENIED", appInfo.accessTokenId);
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}

int32_t AbilityManagerService::AcquireShareData(
    const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData)
{
    HILOG_DEBUG("missionId is %{public}d.", missionId);
    CHECK_CALLER_IS_SYSTEM_APP;
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        abilityRecord = uiAbilityLifecycleManager_->GetAbilityRecordsById(missionId);
    } else {
        if (!currentMissionListManager_) {
            HILOG_ERROR("currentMissionListManager_ is null.");
            return ERR_INVALID_VALUE;
        }
        std::shared_ptr<Mission> mission = currentMissionListManager_->GetMissionById(missionId);
        if (!mission) {
            HILOG_ERROR("mission is null.");
            return ERR_INVALID_VALUE;
        }
        abilityRecord = mission->GetAbilityRecord();
    }
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is null.");
        return ERR_INVALID_VALUE;
    }
    uniqueId_ = (uniqueId_ == INT_MAX) ? 0 : (uniqueId_ + 1);
    std::pair<int64_t, const sptr<IAcquireShareDataCallback>> shareDataPair =
        std::make_pair(abilityRecord->GetAbilityRecordId(), shareData);
    iAcquireShareDataMap_.emplace(uniqueId_, shareDataPair);
    abilityRecord->ShareData(uniqueId_);
    return ERR_OK;
}

int32_t AbilityManagerService::ShareDataDone(
    const sptr<IRemoteObject> &token, const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
{
    HILOG_INFO("resultCode:%{public}d, uniqueId:%{public}d.", resultCode, uniqueId);
    if (!VerificationAllToken(token)) {
        return ERR_INVALID_VALUE;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }
    CHECK_POINTER_AND_RETURN_LOG(eventHandler_, ERR_INVALID_VALUE, "fail to get abilityEventHandler.");
    eventHandler_->RemoveEvent(SHAREDATA_TIMEOUT_MSG, uniqueId);
    return GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
}

int32_t AbilityManagerService::NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
{
    HILOG_DEBUG("requestCode is %{public}d.", requestCode);

    //caller check
    if (!CheckCallerIsDlpManager(GetBundleManager())) {
        HILOG_WARN("caller check failed");
        return ERR_INVALID_CALLER;
    }

    for (const auto &item : startAbilityChain_) {
        if (item.second->GetHandlerName() == StartAbilitySandboxSavefile::handlerName_) {
            auto savefileHandler = (StartAbilitySandboxSavefile*)(item.second.get());
            savefileHandler->HandleResult(want, resultCode, requestCode);
            break;
        }
    }
    return ERR_OK;
}

void AbilityManagerService::RecordAppExitReasonAtUpgrade(const AppExecFwk::BundleInfo &bundleInfo)
{
    if (bundleInfo.abilityInfos.empty()) {
        HILOG_ERROR("abilityInfos is empty.");
        return;
    }

    std::vector<std::string> abilityList;
    for (auto abilityInfo : bundleInfo.abilityInfos) {
        if (!abilityInfo.name.empty()) {
            abilityList.push_back(abilityInfo.name);
        }
    }

    if (!abilityList.empty()) {
        DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
            bundleInfo.name, abilityList, REASON_UPGRADE);
    }
}

void AbilityManagerService::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return;
    }
    uiAbilityLifecycleManager_->SetRootSceneSession(rootSceneSession);
}

void AbilityManagerService::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return;
    }
    uiAbilityLifecycleManager_->CallUIAbilityBySCB(sessionInfo);
}

int32_t AbilityManagerService::SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService)
{
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    HILOG_INFO("Call SetSessionManagerService of WMS.");
    auto ret = Rosen::MockSessionManagerService::GetInstance().SetSessionManagerService(sessionManagerService);
    if (ret) {
        HILOG_DEBUG("Call SetSessionManagerService of WMS.");
        return ERR_OK;
    }
    HILOG_ERROR("SMS SetSessionManagerService return false.");
    return ERR_OK;
}

bool AbilityManagerService::CheckPrepareTerminateEnable()
{
    if (!isPrepareTerminateEnable_) {
        HILOG_DEBUG("Only support PC.");
        return false;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission()) {
        HILOG_DEBUG("failed, please apply permission ohos.permission.PREPARE_APP_TERMINATE");
        return false;
    }
    return true;
}

void AbilityManagerService::StartSpecifiedAbilityBySCB(const Want &want)
{
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return;
    }
    int32_t userId = GetUserId();
    uiAbilityLifecycleManager_->StartSpecifiedAbilityBySCB(want, userId);
}

int32_t AbilityManagerService::RegisterIAbilityManagerCollaborator(
    int32_t type, const sptr<IAbilityManagerCollaborator> &impl)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        HILOG_ERROR("The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckCollaboratorType(type)) {
        HILOG_ERROR("collaborator register failed, invalid type.");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        collaboratorMap_[type] = impl;
    }
    return ERR_OK;
}

int32_t AbilityManagerService::UnregisterIAbilityManagerCollaborator(int32_t type)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        HILOG_ERROR("The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckCollaboratorType(type)) {
        HILOG_ERROR("collaborator unregister failed, invalid type.");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        collaboratorMap_.erase(type);
    }
    return ERR_OK;
}

int32_t AbilityManagerService::MoveMissionToBackground(int32_t missionId)
{
    HILOG_INFO("call");
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        HILOG_ERROR("The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!currentMissionListManager_) {
        HILOG_ERROR("currentMissionListManager_ is null.");
        return ERR_INVALID_VALUE;
    }

    return currentMissionListManager_->MoveMissionToBackground(missionId);
}

int32_t AbilityManagerService::TerminateMission(int32_t missionId)
{
    HILOG_INFO("call");
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingUid = IPCSkeleton::GetCallingUid();
    if (!isSaCall || (callingUid != BROKER_UID && callingUid != BROKER_RESERVE_UID)) {
        HILOG_ERROR("The interface only support for broker");
        return CHECK_PERMISSION_FAILED;
    }
    if (!currentMissionListManager_) {
        HILOG_ERROR("currentMissionListManager_ is null.");
        return ERR_INVALID_VALUE;
    }

    return currentMissionListManager_->TerminateMission(missionId);
}

sptr<IAbilityManagerCollaborator> AbilityManagerService::GetCollaborator(int32_t type)
{
    if (!CheckCollaboratorType(type)) {
        HILOG_ERROR("collaborator: CheckCollaboratorType failed");
        return nullptr;
    }
    {
        std::lock_guard<ffrt::mutex> autoLock(collaboratorMapLock_);
        auto it = collaboratorMap_.find(type);
        if (it != collaboratorMap_.end()) {
            return it->second;
        }
    }
    return nullptr;
}

bool AbilityManagerService::CheckCollaboratorType(int32_t type)
{
    if (type != CollaboratorType::RESERVE_TYPE && type != CollaboratorType::OTHERS_TYPE) {
        HILOG_ERROR("type is invalid");
        return false;
    }
    return true;
}

void AbilityManagerService::GetConnectManagerAndUIExtensionBySessionInfo(const sptr<SessionInfo> &sessionInfo,
    std::shared_ptr<AbilityConnectManager> &connectManager, std::shared_ptr<AbilityRecord> &targetAbility)
{
    targetAbility = nullptr;
    int32_t userId = GetValidUserId(DEFAULT_INVAL_VALUE);
    HILOG_DEBUG("userId=%{public}d", userId);
    connectManager = GetConnectManagerByUserId(userId);
    if (connectManager) {
        targetAbility = connectManager->GetUIExtensioBySessionInfo(sessionInfo);
    } else {
        HILOG_WARN("connectManager is nullptr, userId: %{public}d", userId);
    }
    if (targetAbility == nullptr && userId != U0_USER_ID) {
        HILOG_DEBUG("try to find UIExtension in user0");
        connectManager = GetConnectManagerByUserId(U0_USER_ID);
        if (connectManager) {
            targetAbility = connectManager->GetUIExtensioBySessionInfo(sessionInfo);
        } else {
            HILOG_WARN("connectManager is nullptr, userId: 0");
        }
    }
}

int32_t AbilityManagerService::RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->RegisterAutoStartupSystemCallback(callback);
}

int32_t AbilityManagerService::UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->UnregisterAutoStartupSystemCallback(callback);
}

int32_t AbilityManagerService::SetApplicationAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->SetApplicationAutoStartup(info);
}

int32_t AbilityManagerService::CancelApplicationAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->CancelApplicationAutoStartup(info);
}

int32_t AbilityManagerService::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->QueryAllAutoStartupApplications(infoList);
}

int32_t AbilityManagerService::RegisterAutoStartupCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->RegisterAutoStartupCallback(callback);
}

int32_t AbilityManagerService::UnregisterAutoStartupCallback(const sptr<IRemoteObject> &callback)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->UnregisterAutoStartupCallback(callback);
}

int32_t AbilityManagerService::SetAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->SetAutoStartup(info);
}

int32_t AbilityManagerService::CancelAutoStartup(const AutoStartupInfo &info)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->CancelAutoStartup(info);
}

int32_t AbilityManagerService::IsAutoStartup(const AutoStartupInfo &info, bool &isAutoStartup)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->IsAutoStartup(info, isAutoStartup);
}

int AbilityManagerService::PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isTerminate)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }

    if (!uiAbilityLifecycleManager_) {
        HILOG_ERROR("failed, uiAbilityLifecycleManager is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = uiAbilityLifecycleManager_->GetUIAbilityRecordBySessionInfo(sessionInfo);
    isTerminate = uiAbilityLifecycleManager_->PrepareTerminateAbility(abilityRecord);

    return ERR_OK;
}

int AbilityManagerService::RegisterSessionHandler(const sptr<IRemoteObject> &object)
{
    HILOG_INFO("call");
    CHECK_POINTER_AND_RETURN(uiAbilityLifecycleManager_, ERR_NO_INIT);
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return ERR_WRONG_INTERFACE_CALL;
    }
    sptr<ISessionHandler> handler = iface_cast<ISessionHandler>(object);
    uiAbilityLifecycleManager_->SetSessionHandler(handler);
    return ERR_OK;
}

bool AbilityManagerService::CheckUserIdActive(int32_t userId)
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("QueryActiveOsAccountIds failed.");
        return false;
    }
    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("%{public}s, QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return false;
    }
    auto iter = std::find(osActiveAccountIds.begin(), osActiveAccountIds.end(), userId);
    if (iter == osActiveAccountIds.end()) {
        return false;
    }
    return true;
}

int32_t AbilityManagerService::RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        HILOG_ERROR("Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->RegisterAppDebugListener(listener);
}

int32_t AbilityManagerService::UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener)
{
    HILOG_DEBUG("Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        HILOG_ERROR("Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->UnregisterAppDebugListener(listener);
}

int32_t AbilityManagerService::AttachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    if (!system::GetBoolParameter(DEVELOPER_MODE_STATE, false)) {
        HILOG_ERROR("Developer Mode is false.");
        return ERR_NOT_DEVELOPER_MODE;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    if (abilityDebugDeal_ == nullptr) {
        HILOG_DEBUG("Creat ability debug deal object.");
        abilityDebugDeal_ = std::make_shared<AbilityDebugDeal>();
        if (abilityDebugDeal_ != nullptr) {
            abilityDebugDeal_->RegisterAbilityDebugResponse();
        }
    }
    return DelayedSingleton<AppScheduler>::GetInstance()->AttachAppDebug(bundleName);
}

int32_t AbilityManagerService::DetachAppDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    if (!AAFwk::PermissionVerification::GetInstance()->IsSACall() &&
        !AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    return DelayedSingleton<AppScheduler>::GetInstance()->DetachAppDebug(bundleName);
}

int32_t AbilityManagerService::ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const InsightIntentExecuteParam &param)
{
    HILOG_DEBUG("Called.");
    auto paramPtr = std::make_shared<InsightIntentExecuteParam>(param);
    int32_t ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(key, callerToken,
        paramPtr);
    if (ret != ERR_OK) {
        return ret;
    }

    Want want;
    ret = InsightIntentExecuteManager::GenerateWant(paramPtr, want);
    if (ret != ERR_OK) {
        return ret;
    }

    switch (param.executeMode_) {
        case AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND:
            HILOG_DEBUG("ExecuteMode UI_ABILITY_FOREGROUND.");
            ret = StartAbilityWithInsightIntent(want);
            break;
        case AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND: {
            HILOG_DEBUG("ExecuteMode UI_ABILITY_BACKGROUND.");
            ret = StartAbilityByCallWithInsightIntent(want, callerToken, param);
            break;
        }
        case AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY:
            HILOG_WARN("ExecuteMode UI_EXTENSION_ABILITY not supported.");
            ret = ERR_INVALID_OPERATION;
            break;
        case AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY:
            HILOG_DEBUG("ExecuteMode SERVICE_EXTENSION_ABILITY.");
            ret = StartExtensionAbilityWithInsightIntent(want, AppExecFwk::ExtensionAbilityType::SERVICE);
            break;
        default:
            HILOG_ERROR("Invalid ExecuteMode.");
            ret = ERR_INVALID_OPERATION;
            break;
    }
    if (ret == START_ABILITY_WAITING) {
        HILOG_DEBUG("Top ability is foregrounding. The intent will be queued for execution");
        ret = ERR_OK;
    }
    if (ret != ERR_OK) {
        DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->RemoveExecuteIntent(paramPtr->insightIntentId_);
    }

    HILOG_DEBUG("ExecuteIntent done, ret: %{public}d.", ret);
    return ret;
}

bool AbilityManagerService::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord, const int32_t oriValidUserId)
{
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        HILOG_INFO("scene board is enable");
        if (uiAbilityLifecycleManager_ == nullptr) {
            return false;
        }
        return uiAbilityLifecycleManager_->IsAbilityStarted(abilityRequest, targetRecord, oriValidUserId);
    }

    auto missionListMgr = GetListManagerByUserId(oriValidUserId);
    if (missionListMgr == nullptr) {
        return false;
    }
    return missionListMgr->IsAbilityStarted(abilityRequest, targetRecord);
}

int32_t AbilityManagerService::OnExecuteIntent(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    HILOG_INFO("OnExecuteIntent");
    if (targetRecord == nullptr || targetRecord->GetScheduler() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    targetRecord->GetScheduler()->OnExecuteIntent(abilityRequest.want);

    return ERR_OK;
}

int32_t AbilityManagerService::StartAbilityWithInsightIntent(const Want &want, int32_t userId, int requestCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool startWithAccount = want.GetBoolParam(START_ABILITY_TYPE, false);
    if (startWithAccount || IsCrossUserCall(userId)) {
        (const_cast<Want &>(want)).RemoveParam(START_ABILITY_TYPE);
        CHECK_CALLER_IS_SYSTEM_APP;
    }
    EventInfo eventInfo = BuildEventInfo(want, userId);
    EventReport::SendAbilityEvent(EventName::START_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    int32_t ret = StartAbilityWrap(want, nullptr, requestCode, userId);
    if (ret != ERR_OK) {
        eventInfo.errCode = ret;
        EventReport::SendAbilityEvent(EventName::START_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return ret;
}

int32_t AbilityManagerService::StartExtensionAbilityWithInsightIntent(const Want &want,
    AppExecFwk::ExtensionAbilityType extensionType)
{
    HILOG_DEBUG("called.");
    return StartExtensionAbilityInner(want, nullptr, DEFAULT_INVAL_VALUE, extensionType, true);
}

int32_t AbilityManagerService::StartAbilityByCallWithInsightIntent(const Want &want,
    const sptr<IRemoteObject> &callerToken, const InsightIntentExecuteParam &param)
{
    HILOG_INFO("call StartAbilityByCallWithInsightIntent.");
    sptr<IAbilityConnection> connect = sptr<AbilityBackgroundConnection>::MakeSptr();
    if (connect == nullptr) {
        HILOG_ERROR("Invalid connect.");
        return ERR_INVALID_VALUE;
    }

    AbilityRequest abilityRequest;
    abilityRequest.callType = AbilityCallType::CALL_REQUEST_TYPE;
    abilityRequest.callerUid = IPCSkeleton::GetCallingUid();
    abilityRequest.callerToken = callerToken;
    abilityRequest.startSetting = nullptr;
    abilityRequest.want = want;
    abilityRequest.connect = connect;
    int32_t result = GenerateAbilityRequest(want, -1, abilityRequest, callerToken, GetUserId());
    if (result != ERR_OK) {
        HILOG_ERROR("Generate ability request error.");
        return result;
    }
    std::shared_ptr<AbilityRecord> targetRecord;
    int32_t oriValidUserId = GetValidUserId(DEFAULT_INVAL_VALUE);
    auto missionListMgr = GetListManagerByUserId(oriValidUserId);
    if (IsAbilityStarted(abilityRequest, targetRecord, oriValidUserId)) {
        HILOG_INFO("ability has already started");
        result = OnExecuteIntent(abilityRequest, targetRecord);
    }  else {
        result = StartAbilityByCall(want, connect, callerToken);
    }

    HILOG_INFO("StartAbilityByCallWithInsightIntent %{public}d", result);
    return result;
}

bool AbilityManagerService::IsAbilityControllerStart(const Want &want)
{
    auto callingUid = IPCSkeleton::GetCallingUid();
    bool isBrokerCall = (callingUid == BROKER_UID || callingUid == BROKER_RESERVE_UID);
    if (isBrokerCall) {
        return IsAbilityControllerStart(want, want.GetBundle());
    }
    HILOG_ERROR("The interface only support for broker");
    return true;
}

int32_t AbilityManagerService::ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
    const InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, ERR_INVALID_VALUE, "Ability record is nullptr.");
    if (!JudgeSelfCalled(abilityRecord)) {
        return CHECK_PERMISSION_FAILED;
    }

    // check send by same bundleName.
    std::string bundleNameStored = "";
    auto ret = DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->GetBundleName(intentId, bundleNameStored);
    if (ret != ERR_OK) {
        HILOG_ERROR("Get matched bundleName failed, intentId: %{public}" PRIu64"", intentId);
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = abilityRecord->GetAbilityInfo().bundleName;
    if (bundleNameStored != bundleName) {
        HILOG_ERROR("BundleName %{public}s and %{public}s mismatch.", bundleName.c_str(), bundleNameStored.c_str());
        return ERR_INVALID_VALUE;
    }

    return DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->ExecuteIntentDone(
        intentId, result.innerErr, result);
}

int32_t AbilityManagerService::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->SetApplicationAutoStartupByEDM(info, flag);
}

int32_t AbilityManagerService::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    if (abilityAutoStartupService_ == nullptr) {
        HILOG_ERROR("abilityAutoStartupService_ is nullptr.");
        return ERR_NO_INIT;
    }
    return abilityAutoStartupService_->CancelApplicationAutoStartupByEDM(info, flag);
}

int32_t AbilityManagerService::GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list)
{
    HILOG_DEBUG("Called.");
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        HILOG_ERROR("Permission verification failed.");
        return CHECK_PERMISSION_FAILED;
    }

    std::vector<AbilityRunningInfo> abilityRunningInfos;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        uiAbilityLifecycleManager_->GetAbilityRunningInfos(abilityRunningInfos, isPerm, GetUserId());
    } else {
        if (currentMissionListManager_ == nullptr) {
            HILOG_ERROR("Current mission list manager is nullptr.");
            return ERR_NULL_OBJECT;
        }
        currentMissionListManager_->GetAbilityRunningInfos(abilityRunningInfos, isPerm);
    }

    for (auto &info : abilityRunningInfos) {
        if (info.abilityState != AbilityState::FOREGROUND) {
            continue;
        }

        AppExecFwk::AbilityStateData abilityData;
        abilityData.bundleName = info.ability.GetBundleName();
        abilityData.moduleName = info.ability.GetModuleName();
        abilityData.abilityName = info.ability.GetAbilityName();
        abilityData.abilityState = info.abilityState;
        abilityData.pid = info.pid;
        abilityData.uid = info.uid;
        abilityData.abilityType = static_cast<int32_t>(AppExecFwk::AbilityType::PAGE);
        list.push_back(abilityData);
    }
    HILOG_DEBUG("Get foreground ui abilities end, list.size = %{public}zu.", list.size());
    return ERR_OK;
}

void AbilityManagerService::HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid)
{
    auto userId = uid / BASE_USER_RANGE;
    auto connectManager = GetConnectManagerByUserId(userId);
    if (connectManager == nullptr) {
        HILOG_ERROR("can not find user connect manager");
        return;
    }
    connectManager->HandleProcessFrozen(pidList, uid);
}

void AbilityManagerService::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        HILOG_ERROR("collaborator GetCollaborator is nullptr.");
        return;
    }
    collaborator->UpdateConfiguration(config, userId);
}

int32_t AbilityManagerService::OpenFile(const Uri& uri, uint32_t flag)
{
    auto accessTokenId = IPCSkeleton::GetCallingTokenID();
    if (!AAFwk::UriPermissionManagerClient::GetInstance().VerifyUriPermission(uri, flag, accessTokenId)) {
        HILOG_ERROR("premission check failed");
        return -1;
    }
    auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
    if (collaborator == nullptr) {
        HILOG_ERROR("collaborator GetCollaborator is nullptr.");
        return ERR_COLLABORATOR_NOT_REGISTER;
    }
    return collaborator->OpenFile(uri, flag);
}

int AbilityManagerService::GetDialogSessionInfo(const std::string dialogSessionId,
    sptr<DialogSessionInfo> &dialogSessionInfo)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    CHECK_POINTER_AND_RETURN(dialogSessionRecord_, ERR_INVALID_VALUE);
    dialogSessionInfo = dialogSessionRecord_->GetDialogSessionInfo(dialogSessionId);
    if (dialogSessionInfo) {
        HILOG_DEBUG("success");
        return ERR_OK;
    }
    HILOG_DEBUG("fail");
    return INNER_ERR;
}

bool AbilityManagerService::GenerateDialogSessionRecord(AbilityRequest &abilityRequest, int32_t userId,
    std::string &dialogSessionId, std::vector<DialogAppInfo> &dialogAppInfos, bool isSelector)
{
    CHECK_POINTER_AND_RETURN(dialogSessionRecord_, ERR_INVALID_VALUE);
    if (!isSelector && dialogAppInfos.size() == 1) {
        dialogAppInfos.front().bundleName = abilityRequest.abilityInfo.bundleName;
        dialogAppInfos.front().moduleName = abilityRequest.abilityInfo.moduleName;
        dialogAppInfos.front().abilityName = abilityRequest.abilityInfo.name;
        dialogAppInfos.front().iconId = abilityRequest.abilityInfo.iconId;
        dialogAppInfos.front().labelId = abilityRequest.abilityInfo.labelId;
    }
    return dialogSessionRecord_->GenerateDialogSessionRecord(abilityRequest, userId,
        dialogSessionId, dialogAppInfos, OHOS::system::GetDeviceType());
}

int AbilityManagerService::CreateModalDialog(const Want &replaceWant, sptr<IRemoteObject> callerToken,
    std::string dialogSessionId)
{
    (const_cast<Want &>(replaceWant)).SetParam("dialogSessionId", dialogSessionId);
    auto connection = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
    if (callerToken == nullptr) {
        HILOG_DEBUG("create modal ui extension for system");
        return connection->CreateModalUIExtension(replaceWant) ? ERR_OK : INNER_ERR;
    }
    auto callerRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!callerRecord) {
        HILOG_ERROR("callerRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> token;
    int ret = IN_PROCESS_CALL(GetTopAbility(token));
    if (ret != ERR_OK || token == nullptr) {
        HILOG_ERROR("token is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (callerRecord->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE && token == callerToken) {
        HILOG_DEBUG("create modal ui extension for application");
        return callerRecord->CreateModalUIExtension(replaceWant);
    }
    HILOG_DEBUG("create modal ui extension for system");
    return connection->CreateModalUIExtension(replaceWant) ? ERR_OK : INNER_ERR;
}

int AbilityManagerService::SendDialogResult(const Want &want, const std::string dialogSessionId, bool isAllowed)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    if (!isAllowed) {
        HILOG_INFO("user refuse to jump");
        dialogSessionRecord_->ClearDialogContext(dialogSessionId);
        return ERR_OK;
    }
    CHECK_POINTER_AND_RETURN(dialogSessionRecord_, ERR_INVALID_VALUE);
    std::shared_ptr<DialogCallerInfo> dialogCallerInfo = dialogSessionRecord_->GetDialogCallerInfo(dialogSessionId);
    if (dialogCallerInfo == nullptr) {
        HILOG_ERROR("dialog caller info is nullptr");
        dialogSessionRecord_->ClearDialogContext(dialogSessionId);
        return ERR_INVALID_VALUE;
    }
    auto targetWant = dialogCallerInfo->targetWant;
    targetWant.SetElement(want.GetElement());
    targetWant.SetParam("isSelector", dialogCallerInfo->isSelector);
    targetWant.SetParam("dialogSessionId", dialogSessionId);
    sptr<IRemoteObject> callerToken = dialogCallerInfo->callerToken;
    return StartAbilityAsCaller(targetWant, callerToken, nullptr, dialogCallerInfo->userId,
        dialogCallerInfo->requestCode, true);
}

void AbilityManagerService::RemoveLauncherDeathRecipient(int32_t userId)
{
    auto connectManager = GetConnectManagerByUserId(userId);
    if (connectManager == nullptr) {
        HILOG_ERROR("connectManager is nullptr. userId=%{public}d", userId);
        return;
    }
    connectManager->RemoveLauncherDeathRecipient();
}

int32_t AbilityManagerService::GenerateEmbeddableUIAbilityRequest(
    const Want &want, AbilityRequest &request, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    int32_t result = ERR_OK;
    if (screenMode == AAFwk::HALF_SCREEN_MODE) {
        result = GenerateAbilityRequest(want, -1, request, callerToken, userId);
        request.abilityInfo.isModuleJson = true;
        request.abilityInfo.isStageBasedModel = true;
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
        request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
        struct timespec time = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &time);
        int64_t times = static_cast<int64_t>(time.tv_sec);
        request.abilityInfo.process = request.abilityInfo.bundleName + PROCESS_SUFFIX + std::to_string(times);
    } else {
        result = GenerateExtensionAbilityRequest(want, request, callerToken, userId);
    }
    return result;
}

void AbilityManagerService::UpdateSessionInfoBySCB(const std::vector<SessionInfo> &sessionInfos, int32_t userId)
{
    if (!CheckCallingTokenId(BUNDLE_NAME_SCENEBOARD)) {
        HILOG_ERROR("Not sceneboard called, not allowed.");
        return;
    }
    HILOG_INFO("The sceneboard is being restored.");
    uiAbilityLifecycleManager_->UpdateSessionInfoBySCB(sessionInfos, userId);
}
}  // namespace AAFwk
}  // namespace OHOS
