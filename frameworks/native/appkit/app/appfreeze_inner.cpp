/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "appfreeze_inner.h"

#include <sys/time.h>

#include "ability_manager_client.h"
#include "ability_state.h"
#include "app_recovery.h"
#include "freeze_util.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "mix_stack_dumper.h"
#include "parameter.h"
#include "xcollie/watchdog.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
namespace {
constexpr char EVENT_UID[] = "UID";
constexpr char EVENT_PID[] = "PID";
constexpr char EVENT_MESSAGE[] = "MSG";
constexpr char EVENT_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_STACK[] = "STACK";
}
std::weak_ptr<EventHandler> AppfreezeInner::appMainHandler_;
std::shared_ptr<AppfreezeInner> AppfreezeInner::instance_ = nullptr;
std::mutex AppfreezeInner::singletonMutex_;

AppfreezeInner::AppfreezeInner()
{}

AppfreezeInner::~AppfreezeInner()
{}

void AppfreezeInner::SetMainHandler(const std::shared_ptr<EventHandler>& eventHandler)
{
    appMainHandler_ = eventHandler;
}

void AppfreezeInner::SetApplicationInfo(const std::shared_ptr<ApplicationInfo>& applicationInfo)
{
    applicationInfo_ = applicationInfo;
}

std::shared_ptr<AppfreezeInner> AppfreezeInner::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppfreezeInner>();
        }
    }
    return instance_;
}

void AppfreezeInner::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(singletonMutex_);
    if (instance_ != nullptr) {
        instance_.reset();
        instance_ = nullptr;
    }
}

bool AppfreezeInner::IsHandleAppfreeze()
{
    auto applicationInfo = applicationInfo_.lock();
    if (applicationInfo == nullptr) {
        HILOG_ERROR("applicationInfo_ is nullptr.");
        return false;
    }

    const int buffSize = 128;
    char paramOutBuff[buffSize] = {0};
    GetParameter("hiviewdfx.appfreeze.filter_bundle_name", "", paramOutBuff, buffSize - 1);

    std::string str(paramOutBuff);
    std::string& bundleName = applicationInfo->bundleName;
    if (bundleName.empty()) {
        return true;
    }
    if (str.find(bundleName) != std::string::npos) {
        HILOG_WARN("appfreeze filtration %{public}s.", bundleName.c_str());
        return false;
    }
    return true;
}

int AppfreezeInner::AppfreezeHandle(const FaultData& faultData, bool onlyMainThread)
{
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return -1;
    }
    auto reportFreeze = [faultData, onlyMainThread]() {
        if (faultData.errorObject.name == "") {
            HILOG_ERROR("name is nullptr, AppfreezeHandle failed.");
            return;
        }
        AppExecFwk::AppfreezeInner::GetInstance()->AcquireStack(faultData, onlyMainThread);
    };

    OHOS::HiviewDFX::Watchdog::GetInstance().RunOneShotTask("reportAppFreeze", reportFreeze);
    return 0;
}

bool AppfreezeInner::IsExitApp(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_6S) {
        return true;
    }

    if (name == AppFreezeType::APP_INPUT_BLOCK) {
        return true;
    }
    return false;
}

int AppfreezeInner::AcquireStack(const FaultData& faultInfo, bool onlyMainThread)
{
    HILOG_DEBUG("Start dump mixstack.");
    std::string stack = MixStackDumper::GetMixStack(onlyMainThread);

    HILOG_DEBUG("Start dump MainHandler message.");
    std::string msgContent = faultInfo.errorObject.message + "\n";
    if (faultInfo.state != 0) {
        FreezeUtil::LifecycleFlow flow = { faultInfo.token, static_cast<FreezeUtil::TimeoutState>(faultInfo.state) };
        msgContent = msgContent + "client:\n" + FreezeUtil::GetInstance().GetLifecycleEvent(flow) + "\n";
    }

    auto mainHandler = appMainHandler_.lock();
    if (mainHandler == nullptr) {
        msgContent += "mainHandler is destructed!";
    } else {
        MainHandlerDumper handlerDumper;
        msgContent += "mainHandler dump is:\n";
        mainHandler->Dump(handlerDumper);
        msgContent += handlerDumper.GetDumpInfo();
    }

    HILOG_DEBUG("end dump message is %{public}s", msgContent.c_str());

    FaultData faultData;
    faultData.errorObject.message = msgContent;
    faultData.errorObject.stack = stack;
    faultData.errorObject.name = faultInfo.errorObject.name;
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = faultInfo.timeoutMarkers;
    faultData.notifyApp = false;
    faultData.waitSaveState = false;
    faultData.forceExit = false;
    bool isExit = IsExitApp(faultInfo.errorObject.name);
    if (isExit) {
        faultData.forceExit = true;
        faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
    }
    int ret = NotifyANR(faultData);
    if (isExit) {
        AppFreezeRecovery();
    }
    HILOG_DEBUG("End notify appfreeze");
    return ret;
}

void AppfreezeInner::ThreadBlock(std::atomic_bool& isSixSecondEvent)
{
    FaultData faultData;
    faultData.errorObject.message = "App main thread is not response!";
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = "";
    bool onlyMainThread = false;

    if (isSixSecondEvent) {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
        onlyMainThread = true;
    } else {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
        isSixSecondEvent.store(true);
    }

    AppfreezeHandle(faultData, onlyMainThread);
}

int AppfreezeInner::NotifyANR(const FaultData& faultData)
{
    auto applicationInfo = applicationInfo_.lock();
    if (applicationInfo == nullptr) {
        HILOG_ERROR("reportEvent fail, applicationInfo_ is nullptr.");
        return -1;
    }

    int32_t pid = static_cast<int32_t>(getpid());
    HILOG_INFO("reportEvent:%{public}s, pid:%{public}d, bundleName:%{public}s. success",
        faultData.errorObject.name.c_str(), pid, applicationInfo->bundleName.c_str());

    // move this call before force stop app ? such as merge to NotifyAppFault ?
    DelayedSingleton<AbilityManagerClient>::GetInstance()->RecordAppExitReason(REASON_APP_FREEZE);
    int ret = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFault(faultData);
    if (ret != 0) {
        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_UID, applicationInfo->uid,
            EVENT_PID, pid, EVENT_PACKAGE_NAME, applicationInfo->bundleName,
            EVENT_PROCESS_NAME, applicationInfo->process, EVENT_MESSAGE,
            faultData.errorObject.message, EVENT_STACK, faultData.errorObject.stack);
    }
    return ret;
}

void AppfreezeInner::AppFreezeRecovery()
{
    AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
    AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::APP_FREEZE);
}

void MainHandlerDumper::Dump(const std::string &message)
{
    dumpInfo += message;
}

std::string MainHandlerDumper::GetTag()
{
    return "";
}

std::string MainHandlerDumper::GetDumpInfo()
{
    return dumpInfo;
}
}  // namespace AAFwk
}  // namespace OHOS