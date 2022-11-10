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

#include "appmgrrest_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "app_death_recipient.h"
#include "app_mgr_service_event_handler.h"
#include "app_process_manager.h"
#define private public
#include "app_spawn_client.h"
#include "app_spawn_msg_wrapper.h"
#undef private
#include "app_spawn_socket.h"
#include "remote_client_manager.h"
#include "window_focus_changed_listener.h"
#include "ability_record.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << OFFSET_ZERO) | (ptr[1] << OFFSET_ONE) | (ptr[2] << OFFSET_TWO) | ptr[3];
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
    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    wptr<IRemoteObject> remote;
    appDeathRecipient->OnRemoteDied(remote);
    std::shared_ptr<AMSEventHandler> handler;
    appDeathRecipient->SetEventHandler(handler);
    std::shared_ptr<AppMgrServiceInner> serviceInner;
    appDeathRecipient->SetAppMgrServiceInner(serviceInner);
    bool isRenderProcess = *data % ENABLE;
    appDeathRecipient->SetIsRenderProcess(isRenderProcess);
    AppProcessManager appProcessManager;
    std::shared_ptr<AppTaskInfo> appTaskInfo;
    appProcessManager.RemoveAppFromRecentList(appTaskInfo);
    std::string appName(data, size);
    std::string processName(data, size);
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    appProcessManager.AddAppToRecentList(appName, processName, pid, recordId);
    appProcessManager.PushAppFront(recordId);
    appProcessManager.GetAppTaskInfoById(recordId);
    std::string bundleName(data, size);
    appProcessManager.GetAppTaskInfoByProcessName(appName, bundleName);
    appProcessManager.GetRecentAppList();
    appProcessManager.RemoveAppFromRecentListById(recordId);
    appProcessManager.ClearRecentAppList();
    AppSpawnClient appSpawnClient;
    appSpawnClient.OpenConnection();
    std::shared_ptr<AppSpawnSocket> socket;
    appSpawnClient.SetSocket(socket);
    appSpawnClient.PreStartNWebSpawnProcess();
    appSpawnClient.PreStartNWebSpawnProcessImpl();
    AppSpawnStartMsg startMsg;
    appSpawnClient.StartProcess(startMsg, pid);
    int status = static_cast<int>(GetU32Data(data));
    appSpawnClient.GetRenderProcessTerminationStatus(startMsg, status);
    appSpawnClient.QueryConnectionState();
    appSpawnClient.CloseConnection();
    AppSpawnMsgWrapper appSpawnMsgWrapper;
    appSpawnMsgWrapper.AssembleMsg(startMsg);
    appSpawnMsgWrapper.VerifyMsg(startMsg);
    appSpawnMsgWrapper.DumpMsg();
    appSpawnMsgWrapper.FreeMsg();
    AppSpawnSocket appSpawnSocket;
    std::shared_ptr<OHOS::AppSpawn::ClientSocket> clientSocket;
    appSpawnSocket.SetClientSocket(clientSocket);
    appSpawnSocket.OpenAppSpawnConnection();
    int32_t len = appSpawnMsgWrapper.GetMsgLength();
    appSpawnSocket.WriteMessage(appSpawnMsgWrapper.GetMsgBuf(), len);
    appSpawnSocket.ReadMessage(reinterpret_cast<void *>(*data), len);
    appSpawnSocket.CloseAppSpawnConnection();
    RemoteClientManager remoteClientManager;
    sptr<IBundleMgr> bundleManager = nullptr;
    remoteClientManager.SetBundleManager(bundleManager);
    std::shared_ptr<AppSpawnClient> appSpawnClientptr;
    remoteClientManager.SetSpawnClient(appSpawnClientptr);
    remoteClientManager.GetSpawnClient();
    remoteClientManager.GetBundleManager();
    remoteClientManager.GetNWebSpawnClient();
    std::shared_ptr<AppMgrServiceInner> owner;
    WindowFocusChangedListener windowFocusChangedListener(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo = nullptr;
    windowFocusChangedListener.OnFocused(focusChangeInfo);
    windowFocusChangedListener.OnUnfocused(focusChangeInfo);
    return (appSpawnClient.StartProcessImpl(startMsg, pid) != 0);
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

