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

#include "parameter.h"
#include "ability_manager_client.h"
#include "ability_state.h"
#include "app_recovery.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "mix_stack_dumper.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_STACK[] = "STACK";
}
std::weak_ptr<EventHandler> AppfreezeInner::appMainHandler_;
AppfreezeInner::AppfreezeInner()
{}

AppfreezeInner::~AppfreezeInner()
{}

void AppfreezeInner::SetMainHandler(std::shared_ptr<EventHandler> eventHandler)
{
    appMainHandler_ = eventHandler;
}

void AppfreezeInner::SetApplicationInfo(std::shared_ptr<ApplicationInfo> applicationInfo)
{
    applicationInfo_ = applicationInfo;
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
    GetParameter("appfreezeFiltration", "", paramOutBuff, buffSize - 1);

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
    auto reportFreeze = [this, faultData, onlyMainThread]() {
        if (faultData.errorObject.name == "") {
            HILOG_ERROR("name is nullptr, AppfreezeHandle failed.");
            return;
        }
        this->AcquireStack(faultData, onlyMainThread);
    };

    OHOS::HiviewDFX::Watchdog::GetInstance().RunOneShotTask("reportAppFreeze", reportFreeze);
    return 0;
}

bool AppfreezeInner::IsExit(const std::string& name)
{
    if (name == "THREAD_BLOCK_6S") {
        if (eventMarks_.find("APP_INPUT_BLOCK") != eventMarks_.end()) {
            return true;
        }
        eventMarks_.insert(name);
    }

    if (name == "APP_INPUT_BLOCK") {
        if (eventMarks_.size() > 0) {
            return true;
        }
        eventMarks_.insert(name);
    }
    return false;
}

int AppfreezeInner::AcquireStack(const FaultData& faultInfo, bool onlyMainThread)
{
    HILOG_DEBUG("Start dump mixstack.");
    std::string stack = "";
    stack = MixStackDumper::GetMixStack(onlyMainThread);

    HILOG_DEBUG("Start dump MainHandler message.");
    std::string msgContent = faultInfo.errorObject.message + "\n";
    MainHandlerDumper handlerDumper;
    auto mainHandler = appMainHandler_.lock();
    if (mainHandler == nullptr) {
        msgContent += "mainHandler is destructed!";
    } else {
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
    faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
    faultData.forceExit = false;
    bool isexit = IsExit(faultInfo.errorObject.name);

    int ret = 0;
    ret = NotifyANR(faultData);
    if (isexit) {
        AppFreezeRecovery();
    }
    HILOG_DEBUG("End notify ANR");
    return ret;
}

int AppfreezeInner::ThreadBlock(std::atomic_bool& isSixSecondEvent)
{
    FaultData faultData;
    faultData.errorObject.message = "App main thread is not response!";
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = "";
    bool onlyMainThread = false;

    if (isSixSecondEvent) {
        faultData.errorObject.name = "THREAD_BLOCK_6S";
        onlyMainThread = true;
    } else {
        faultData.errorObject.name = "THREAD_BLOCK_3S";
        isSixSecondEvent.store(true);
        onlyMainThread = false;
    }

    AppfreezeHandle(faultData, onlyMainThread);
    return 0;
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

    // timeout send
    if (ret != 0) {
        HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_UID, applicationInfo->uid,
            EVENT_KEY_PID, pid, EVENT_KEY_PACKAGE_NAME, applicationInfo->bundleName,
            EVENT_KEY_PROCESS_NAME, applicationInfo->process, EVENT_KEY_MESSAGE,
            faultData.errorObject.message, EVENT_KEY_STACK, faultData.errorObject.stack);
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