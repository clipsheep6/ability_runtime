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

#include "abilitymanagerservicefirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "ability_manager_service.h"
#undef protected
#undef private

#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"
#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}

uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
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

void DoSomethingInterestingWithMyAPI1(AbilityManagerService &abilityms, Want& want,
    sptr<IRemoteObject> token)
{
    sptr<IAbilityConnection> connect(new MyAbilityConnection());
    abilityms.InitStartupFlag();
    abilityms.QueryServiceState();
    AbilityRequest abilityRequest;
    AppExecFwk::ExtensionAbilityType extensionType = ExtensionAbilityType::SERVICE;
    abilityms.CheckOptExtensionAbility(want, abilityRequest, int32Param, extensionType);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityms.ReportAbilitStartInfoToRSS(abilityInfo);
    abilityms.ReportEventToSuspendManager(abilityInfo);
    abilityms.StartExtensionAbility(want, token, int32Param, extensionType);
    abilityms.StopExtensionAbility(want, token, int32Param, extensionType);
    abilityms.TerminateAbility(token, intParam, want);
    abilityms.CloseAbility(token, intParam, want);
    abilityms.TerminateAbilityWithFlag(token, intParam, &want, boolParam);
    abilityms.SendResultToAbility(intParam, intParam, want);
    abilityms.StartRemoteAbility(want, intParam, int32Param, token);
    abilityms.CheckIfOperateRemote(want);
    abilityms.AnonymizeDeviceId(stringParam);
    abilityms.MinimizeAbility(token, boolParam);
    abilityms.ConnectAbility(want, connect, token, int32Param);
    abilityms.ConnectAbilityCommon(want, connect, token, extensionType, int32Param);
    abilityms.BuildEventInfo(want, int32Param);
    abilityms.DisconnectAbility(connect);
    abilityms.ConnectLocalAbility(want, int32Param, connect, token, extensionType);
    abilityms.ConnectRemoteAbility(want, token, token);
    abilityms.DisconnectLocalAbility(connect);
    abilityms.DisconnectRemoteAbility(token);
    AAFwk::WantParams wantParams;
    abilityms.ContinueMission(stringParam, stringParam, int32Param, token, wantParams);
    abilityms.ContinueAbility(stringParam, int32Param, uint32Param);
    abilityms.StartContinuation(want, token, int32Param);
    abilityms.NotifyCompleteContinuation(stringParam, int32Param, boolParam);
    abilityms.NotifyContinuationResult(int32Param, int32Param);
    abilityms.StartSyncRemoteMissions(stringParam, boolParam, int64Param);
    abilityms.StopSyncRemoteMissions(stringParam);
}

void DoSomethingInterestingWithMyAPI2(AbilityManagerService &abilityms, Want& want,
    sptr<IRemoteObject> token)
{
    sptr<AbilityRuntime::IConnectionObserver> observer;
    abilityms.RegisterObserver(observer);
    abilityms.UnregisterObserver(observer);
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    abilityms.GetDlpConnectionInfos(infos);
    std::vector<AbilityRuntime::ConnectionData> connectionData;
    abilityms.GetConnectionData(connectionData);

    sptr<SessionInfo> extensionSessionInfo;
    abilityms.StartUIExtensionAbility(extensionSessionInfo, int32Param);
    extensionSessionInfo = sptr<SessionInfo>(new (std::nothrow) SessionInfo());
    extensionSessionInfo->sessionToken = token;
    extensionSessionInfo->callerToken = token;
    extensionSessionInfo->persistentId = int32Param;
    extensionSessionInfo->resultCode = int32Param;
    extensionSessionInfo->requestCode = int32Param;
    extensionSessionInfo->errorReason = stringParam;
    extensionSessionInfo->errorCode = int32Param;
    extensionSessionInfo->uiAbilityId = int64Param;
    extensionSessionInfo->userId = int32Param;
    extensionSessionInfo->state = CallToState::FOREGROUND;
    extensionSessionInfo->want = want;
    abilityms.StartUIExtensionAbility(extensionSessionInfo, int32Param);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    int64_t int64Param = static_cast<int64_t>(GetU32Data(data));
    uint32_t uint32Param = GetU32Data(data);
    std::string stringParam(data, size);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
        if (!want) {
            return false;
        }
    }
    sptr<IRemoteObject> token = GetFuzzAbilityToken();

    // fuzz for AbilityManagerService
    auto abilityms = std::make_shared<AbilityManagerService>();
    DoSomethingInterestingWithMyAPI1(*abilityms, *want, token);
    DoSomethingInterestingWithMyAPI2(*abilityms, *want, token);
    if (!want) {
        delete want;
        want = nullptr;
    }

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
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

