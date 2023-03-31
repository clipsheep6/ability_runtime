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

#include "attachabilitythread_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_connect_manager.h"
#include "ability_record.h"
#include "ability_scheduler_stub.h"
#include "data_ability_manager.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr int32_t UID_TEST = 100;
constexpr int OFFSET_ZERO = 24;
}
class AbilitySchedulerFuzzTest : public AbilitySchedulerStub {
public:
    AbilitySchedulerFuzzTest() = default;
    virtual ~AbilitySchedulerFuzzTest()
    {};
    void ScheduleAbilityTransaction(const Want& want, const LifeCycleStateInfo& targetState,
        sptr<SessionInfo> sessionInfo = nullptr) override
    {}
    void SendResult(int requestCode, int resultCode, const Want& resultWant) override
    {}
    void ScheduleConnectAbility(const Want& want) override
    {}
    void ScheduleDisconnectAbility(const Want& want) override
    {}
    void ScheduleCommandAbility(const Want& want, bool restart, int startId) override
    {}
    void ScheduleSaveAbilityState() override
    {}
    void ScheduleRestoreAbilityState(const PacMap& inState) override
    {}
    std::vector<std::string> GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter) override
    {
        return {};
    }
    int OpenFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int OpenRawFile(const Uri& uri, const std::string& mode) override
    {
        return 0;
    }
    int Insert(const Uri& uri, const NativeRdb::ValuesBucket& value) override
    {
        return 0;
    }
    int Update(const Uri& uri, const NativeRdb::ValuesBucket& value,
        const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    int Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return 0;
    }
    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap) override
    {
        return {};
    }
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(const Uri& uri,
        std::vector<std::string>& columns, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return {};
    }
    std::string GetType(const Uri& uri) override
    {
        return {};
    }
    bool Reload(const Uri& uri, const PacMap& extras) override
    {
        return true;
    }
    int BatchInsert(const Uri& uri, const std::vector<NativeRdb::ValuesBucket>& values) override
    {
        return 0;
    }
    bool ScheduleRegisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleUnregisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return true;
    }
    bool ScheduleNotifyChange(const Uri& uri) override
    {
        return true;
    }
    Uri NormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }

    Uri DenormalizeUri(const Uri& uri) override
    {
        return Uri{ "abilityschedulerstub" };
    }
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations) override
    {
        return {};
    }
    void ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {}
    void NotifyContinuationResult(int32_t result) override
    {}
    void DumpAbilityInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {}
#ifdef ABILITY_COMMAND_FOR_TEST
    int BlockAbility() override
    {
        return 0;
    }
#endif
    void CallRequest() override
    {
        return;
    }
};
sptr<Token> GetFuzzAbilityToken(AbilityType type)
{
    sptr<Token> token = nullptr;

    AbilityRequest abilityRequest;
    abilityRequest.uid = UID_TEST;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = type;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }

    return token;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << OFFSET_ZERO) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    auto abilitymgr = AbilityManagerClient::GetInstance();
    int userId = static_cast<int>(GetU32Data(data));
    auto connectManager = new AbilityConnectManager(userId);
    auto dataManager = new DataAbilityManager();
    sptr<IAbilityScheduler> scheduler = new AbilitySchedulerFuzzTest();
    if (!abilitymgr) {
        return false;
    }

    // get token
    sptr<IRemoteObject> token = GetFuzzAbilityToken(AbilityType::PAGE);
    if (!token) {
        std::cout << "Get ability token failed." << std::endl;
        return false;
    }

    // get serviceToken
    sptr<IRemoteObject> serviceToken = GetFuzzAbilityToken(AbilityType::SERVICE);
    if (!serviceToken) {
        std::cout << "Get service ability token failed." << std::endl;
        return false;
    }

    // get dataToken
    sptr<IRemoteObject> dataToken = GetFuzzAbilityToken(AbilityType::DATA);
    if (!dataToken) {
        std::cout << "Get data ability token failed." << std::endl;
        return false;
    }

    if (connectManager) {
        connectManager->AttachAbilityThreadLocked(scheduler, serviceToken);
    }

    if (dataManager) {
        dataManager->AttachAbilityThread(scheduler, dataToken);
    }

    if (abilitymgr->AttachAbilityThread(scheduler, token) != 0) {
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

