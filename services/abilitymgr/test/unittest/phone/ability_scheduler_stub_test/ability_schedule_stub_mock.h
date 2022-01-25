/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ABILITY_UNITTEST_ABILITY_SCHEDULE_STUB_MOCK_H
#define ABILITY_UNITTEST_ABILITY_SCHEDULE_STUB_MOCK_H
#include "ability_scheduler_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilitySchedulerStubMock : public AbilitySchedulerStub {
public:
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
    virtual void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config) override
    {}

    virtual std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override
    {
        std::vector<std::string> types;
        return types;
    }

    virtual int OpenFile(const Uri &uri, const std::string &mode) override
    {
        return -1;
    }

    virtual int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override
    {
        return -1;
    }

    virtual int Update(const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return -1;
    }

    virtual int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return -1;
    }

    virtual std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override
    {
        return nullptr;
    }

    virtual std::string GetType(const Uri &uri) override
    {
        return " ";
    }

    virtual int OpenRawFile(const Uri &uri, const std::string &mode) override
    {
        return -1;
    }

    virtual bool Reload(const Uri &uri, const PacMap &extras) override
    {
        return false;
    }

    virtual int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override
    {
        return -1;
    }

    virtual Uri NormalizeUri(const Uri &uri) override
    {
        Uri urivalue("");
        return urivalue;
    }

    virtual Uri DenormalizeUri(const Uri &uri) override
    {
        Uri urivalue("");
        return urivalue;
    }

    void NotifyMultiWinModeChanged(int32_t winModeKey, bool flag) override
    {}

    void NotifyTopActiveAbilityChanged(bool flag) override
    {}
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
    virtual std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override
    {
        return std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>();
    }
    virtual void NotifyContinuationResult(const int32_t result) override
    {}
    virtual void ContinueAbility(const std::string& deviceId) override
    {}
    virtual void DumpAbilityInfo(std::vector<std::string> &info) override
    {}
};
}  // namespace AAFwk
}  // namespace OHOS

#endif