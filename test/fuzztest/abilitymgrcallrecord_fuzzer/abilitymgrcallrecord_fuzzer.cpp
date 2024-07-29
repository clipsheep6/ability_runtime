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

#include "abilitymgrcallrecord_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "call_record.h"
#undef private
#include "inner_mission_info.h"
#include "parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    RemoteDiedHandler handler;
    std::shared_ptr<AbilityCallRecipient> infos = std::make_shared<AbilityCallRecipient>(handler);
    wptr<IRemoteObject> __attribute__((unused)) remotes;
    infos->OnRemoteDied(remotes);
    int32_t callerUid = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<AbilityRecord> targetService;
    sptr<IAbilityConnection> connCallback;
    sptr<IRemoteObject> callToken;
    std::shared_ptr<CallRecord> infos1 = std::make_shared<CallRecord>(
        callerUid, targetService, connCallback, callToken);
    std::shared_ptr<CallRecord> infos2 = CallRecord::CreateCallRecord(
        callerUid, targetService, connCallback, callToken);
    sptr<IRemoteObject> call;
    infos1->SetCallStub(call);
    infos1->GetCallStub();
    infos1->SetConCallBack(connCallback);
    infos1->GetConCallBack();
    std::vector<std::string> info;
    infos1->Dump(info);
    infos1->SchedulerConnectDone();
    infos1->SchedulerDisconnectDone();
    wptr<IRemoteObject> remote;
    infos1->OnCallStubDied(remote);
    infos1->GetCallerUid();
    CallState state = CallState::INIT;
    infos1->IsCallState(state);
    infos1->SetCallState(state);
    infos1->GetCallRecordId();
    infos1->GetTargetServiceName();
    infos1->GetCallerToken();
    return true;
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
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
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

