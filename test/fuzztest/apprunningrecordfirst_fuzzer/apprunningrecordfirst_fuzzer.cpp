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

#include "apprunningrecordfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_running_record.h"
#undef private

#include "ability_record.h"
#include "app_running_manager.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    pid_t hostPid = static_cast<pid_t>(GetU32Data(data));
    std::string renderParam(data, size);
    int32_t ipcFd = static_cast<int32_t>(GetU32Data(data));
    int32_t sharedFd = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord = new RenderRecord(hostPid, renderParam, ipcFd, sharedFd, host);
    renderRecord->RegisterDeathRecipient();
    renderRecord->CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, host);
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    renderRecord->SetPid(pid);
    sptr<IRenderScheduler> scheduler;
    renderRecord->SetScheduler(scheduler);
    sptr<AppDeathRecipient> recipient;
    renderRecord->SetDeathRecipient(recipient);
    renderRecord->SetHostUid(hostPid);
    std::string hostBundleName(data, size);
    renderRecord->SetHostBundleName(hostBundleName);
    renderRecord->GetPid();
    renderRecord->GetHostPid();
    renderRecord->GetHostUid();
    renderRecord->GetHostBundleName();
    renderRecord->GetRenderParam();
    renderRecord->GetIpcFd();
    renderRecord->GetSharedFd();
    renderRecord->GetHostRecord();
    renderRecord->GetScheduler();
    std::shared_ptr<ApplicationInfo> appInfo;
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    std::string processName(data, size);
    AppRunningRecord* appRecord = new AppRunningRecord(appInfo, recordId, processName);
    appRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    sptr<IAppScheduler> thread;
    appRecord->SetApplicationClient(thread);
    std::string signCode(data, size);
    appRecord->SetSignCode(signCode);
    std::string jointUserId(data, size);
    appRecord->SetJointUserId(jointUserId);
    int32_t uid = static_cast<int32_t>(GetU32Data(data));
    appRecord->SetUid(uid);
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    appRecord->SetState(state);
    std::weak_ptr<AppMgrServiceInner> inner;
    appRecord->SetAppMgrServiceInner(inner);
    std::shared_ptr<AMSEventHandler> handler;
    appRecord->SetEventHandler(handler);
    bool isKeepAlive = *data % ENABLE;
    bool isEmptyKeepAliveApp = *data % ENABLE;
    appRecord->SetKeepAliveAppState(isKeepAlive, isEmptyKeepAliveApp);
    bool isStageBasedModel = *data % ENABLE;
    appRecord->SetStageModelState(isStageBasedModel);
    int count = static_cast<int>(GetU32Data(data));
    appRecord->SetRestartResidentProcCount(count);
    std::shared_ptr<UserTestRecord> testRecord;
    appRecord->SetUserTestInfo(testRecord);
    std::shared_ptr<RenderRecord> record;
    appRecord->SetRenderRecord(record);
    AppSpawnStartMsg appMsg;
    appRecord->SetStartMsg(appMsg);
    bool isDebugApp = *data % ENABLE;
    appRecord->SetDebugApp(isDebugApp);
    int32_t appIndex = static_cast<int32_t>(GetU32Data(data));
    appRecord->SetAppIndex(appIndex);
    bool securityFlag = *data % ENABLE;
    appRecord->SetSecurityFlag(securityFlag);
    appRecord->SetTerminating();
    appRecord->SetKilling();
    appRecord->GetSignCode();
    appRecord->GetJointUserId();
    appRecord->GetUid();
    appRecord->GetState();
    appRecord->GetApplicationClient();
    appRecord->IsTerminating();
    appRecord->IsKeepAliveApp();
    appRecord->IsEmptyKeepAliveApp();
    appRecord->GetRestartResidentProcCount();
    appRecord->GetUserTestInfo();
    appRecord->GetRenderRecord();
    appRecord->GetStartMsg();
    appRecord->GetAppIndex();
    appRecord->GetSecurityFlag();
    return appRecord->IsKilling();
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size > OHOS::FOO_MAX_LEN || size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = (char*)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
