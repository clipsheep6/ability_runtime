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

#include "abilityschedulerstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_scheduler_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityScheduler";

class AbilitySchedulerStubFuzzTest : public AbilitySchedulerStub {
public:
    AbilitySchedulerStubFuzzTest() = default;
    virtual ~AbilitySchedulerStubFuzzTest()
    {};
    virtual void ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState) override
    {}
    virtual void SendResult(int requestCode, int resultCode, const Want &resultWant) override
    {}
    virtual void ScheduleConnectAbility(const Want &want) override
    {}
    virtual void ScheduleDisconnectAbility(const Want &want) override
    {}
    virtual void ScheduleCommandAbility(const Want &want, bool restart, int startId) override
    {}
    virtual void ScheduleSaveAbilityState() override
    {}
    virtual void ScheduleRestoreAbilityState(const PacMap &inState) override
    {}
    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override
    {
        return {};
    }
    virtual int OpenFile(const Uri &uri, const std::string &mode) override
    {
        return 0;
    }
    virtual int OpenRawFile(const Uri &uri, const std::string &mode) override
    {
        return 0;
    }
    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override
    {
        return 0;
    }
    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return 0;
    }
    virtual std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap) override
    {
        return {};
    }
    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri &uri,
        std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return {};
    }
    virtual std::string GetType(const Uri &uri) override
    {
        return {};
    }
    virtual bool Reload(const Uri &uri, const PacMap &extras) override
    {
        return true;
    }
    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override
    {
        return 0;
    }
    virtual bool ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    virtual bool ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver) override
    {
        return true;
    }
    virtual bool ScheduleNotifyChange(const Uri &uri) override
    {
        return true;
    }
    virtual Uri NormalizeUri(const Uri &uri) override
    {
        return Uri{"abilityschedulerstub"};
    }

    virtual Uri DenormalizeUri(const Uri &uri) override
    {
        return Uri{"abilityschedulerstub"};
    }
    virtual std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override
    {
        return {};
    }
    virtual void ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {}
    virtual void NotifyContinuationResult(int32_t result) override
    {}
    virtual void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override
    {}
    virtual int BlockAbility() override
    {
        return 0;
    }
    virtual sptr<IRemoteObject> CallRequest() override
    {
        return {};
    }
};

uint32_t Convert2Uint32(const uint8_t* ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* rawData, size_t size)
{
    uint32_t code = Convert2Uint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::shared_ptr<AbilitySchedulerStub> abilityschedulerstub = std::make_shared<AbilitySchedulerStubFuzzTest>();

    if (abilityschedulerstub->OnRemoteRequest(code, data, reply, option) != 0) {
        return false;
    }

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
    if (size < OHOS::THRESHOLD) {
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(data, size);

    return 0;
}

