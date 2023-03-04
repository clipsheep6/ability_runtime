/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULE_STUB_MOCK_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULE_STUB_MOCK_H
#include "ability_scheduler_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilitySchedulerStubMock : public AbilitySchedulerStub {
public:
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
        std::vector<std::string> types;
        return types;
    }

    int OpenFile(const Uri& uri, const std::string& mode) override
    {
        return -1;
    }

    int Insert(const Uri& uri, const NativeRdb::ValuesBucket& value) override
    {
        return -1;
    }

    int Update(const Uri& uri,
        const NativeRdb::ValuesBucket& value, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return -1;
    }

    int Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return -1;
    }

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri& uri, std::vector<std::string>& columns, const NativeRdb::DataAbilityPredicates& predicates) override
    {
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap) override
    {
        return nullptr;
    }

    std::string GetType(const Uri& uri) override
    {
        return " ";
    }

    int OpenRawFile(const Uri& uri, const std::string& mode) override
    {
        return -1;
    }

    bool Reload(const Uri& uri, const PacMap& extras) override
    {
        return false;
    }

    int BatchInsert(const Uri& uri, const std::vector<NativeRdb::ValuesBucket>& values) override
    {
        return -1;
    }

    Uri NormalizeUri(const Uri& uri) override
    {
        Uri urivalue("");
        return urivalue;
    }

    Uri DenormalizeUri(const Uri& uri) override
    {
        Uri urivalue("");
        return urivalue;
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
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations) override
    {
        return std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>();
    }
    void NotifyContinuationResult(int32_t result) override
    {}
    void ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {}
    void DumpAbilityInfo(const std::vector<std::string>& params, std::vector<std::string>& info) override
    {}
    void CallRequest() override
    {
        return;
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    int BlockAbility() override
    {
        return 0;
    }
#endif
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
