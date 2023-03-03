
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

#ifndef UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATAOBS_MGR_SERVICE_H
#define UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATAOBS_MGR_SERVICE_H

#include <gmock/gmock.h>
#define protected public
#define private public
#include "dataobs_mgr_stub.h"

namespace OHOS {
namespace AAFwk {
class MockDataObsMgrService : public DataObsManagerStub {
public:
    MockDataObsMgrService() = default;
    virtual ~MockDataObsMgrService() = default;

    int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override
    {
        onChangeCall_++;
        return NO_ERROR;
    }
    int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override
    {
        onChangeCall_++;
        return NO_ERROR;
    }
    int NotifyChange(const Uri &uri) override
    {
        onChangeCall_++;
        return NO_ERROR;
    }

    Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, bool isDe) override
    {
        onChangeCall_++;
        return SUCCESS;
    }

    Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) override
    {
        onChangeCall_++;
        return SUCCESS;
    }

    Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver) override
    {
        onChangeCall_++;
        return SUCCESS;
    }

    Status NotifyChangeExt(const ChangeInfo &changeInfo) override
    {
        onChangeCall_++;
        return SUCCESS;
    }

    void OnStart() {}
    void OnStop() {}

private:
    std::atomic_int onChangeCall_ = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // UNITTEST_OHOS_ABILITY_RUNTIME_MOCK_DATAOBS_MGR_SERVICE_H
