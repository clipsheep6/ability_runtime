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

#include "apprunningmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_running_manager.h"
#undef private

#include "ability_record.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr uint8_t ENABLE = 2;

uint32_t GetU32Data(const char* ptr)
{
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }

    return token;
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AppRunningManager* apprunningmanager = new AppRunningManager();
    if (!apprunningmanager) {
        return false;
    }
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName(data, size);
    BundleInfo bundleInfo;
    apprunningmanager->CreateAppRunningRecord(appInfo, processName, bundleInfo);

    std::string appName(data, size);
    int uid = static_cast<int>(GetU32Data(data));
    apprunningmanager->CheckAppRunningRecordIsExist(appName, processName, uid, bundleInfo);

    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    apprunningmanager->GetAppRunningRecordByPid(pid);

    sptr<IRemoteObject> abilityToken = GetFuzzAbilityToken();
    apprunningmanager->GetAppRunningRecordByAbilityToken(abilityToken);

    std::string bundleName(data, size);
    std::list<pid_t> pids;
    apprunningmanager->ProcessExitByBundleName(bundleName, pids);

    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    apprunningmanager->GetPidsByUserId(userId, pids);

    apprunningmanager->ProcessExitByBundleNameAndUid(bundleName, uid, pids);
    apprunningmanager->ProcessExitByPid(pid);

    wptr<IRemoteObject> remote = nullptr;
    apprunningmanager->OnRemoteDied(remote);
    apprunningmanager->GetAppRunningRecordMap();

    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    apprunningmanager->RemoveAppRunningRecordById(recordId);
    apprunningmanager->ClearAppRunningRecordMap();

    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    apprunningmanager->HandleTerminateTimeOut(eventId);
    apprunningmanager->GetTerminatingAppRunningRecord(abilityToken);
    apprunningmanager->GetAbilityRunningRecord(eventId);
    apprunningmanager->GetAppRunningRecord(eventId);

    sptr<IRemoteObject> token = nullptr;
    apprunningmanager->HandleAbilityAttachTimeOut(token);
    apprunningmanager->PrepareTerminate(token);

    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner = nullptr;
    bool clearMissionFlag = *data % ENABLE;
    apprunningmanager->TerminateAbility(token, clearMissionFlag, appMgrServiceInner);

    RunningProcessInfo info;
    apprunningmanager->GetRunningProcessInfoByToken(token, info);

    std::vector<AppStateData> list;
    apprunningmanager->GetForegroundApplications(list);
    apprunningmanager->HandleAddAbilityStageTimeOut(eventId);
    apprunningmanager->HandleStartSpecifiedAbilityTimeOut(eventId);

    Configuration config;
    apprunningmanager->UpdateConfiguration(config);

    int32_t level = static_cast<int32_t>(GetU32Data(data));
    apprunningmanager->NotifyMemoryLevel(level);
    apprunningmanager->GetAppRunningRecordByRenderPid(pid);
    apprunningmanager->OnRemoteRenderDied(remote);
    apprunningmanager->GetAppRunningStateByBundleName(bundleName);

    sptr<IQuickFixCallback> callback = nullptr;
    apprunningmanager->NotifyLoadRepairPatch(bundleName, callback);
    apprunningmanager->NotifyHotReloadPage(bundleName, callback);
    apprunningmanager->NotifyUnLoadRepairPatch(bundleName, callback);

    std::shared_ptr<ApplicationInfo> infoPtr = std::make_shared<ApplicationInfo>();
    AppRunningRecord apprunningrecord(infoPtr, recordId, processName);
    apprunningmanager->IsApplicationFirstForeground(apprunningrecord);
    apprunningmanager->IsApplicationBackground(bundleName);
    apprunningmanager->IsApplicationFirstFocused(apprunningrecord);
    apprunningmanager->IsApplicationUnfocused(bundleName);

    return apprunningmanager->IsApplicationUnfocused(bundleName);
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
    if (size == 0 || size > OHOS::FOO_MAX_LEN) {
        std::cout << "invalid size" << std::endl;
        return 0;
    }

    char* ch = (char *)malloc(size + 1);
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
