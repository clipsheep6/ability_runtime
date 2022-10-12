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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H

#include <mutex>

#include "ability_connect_callback_interface.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_scheduler_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityManagerClient : public AbilityManagerClient {
public:
    MockAbilityManagerClient();
    virtual ~MockAbilityManagerClient();

    ErrCode GetStartAbility();
    ErrCode GetTerminateAbility();
    ErrCode GetTerminateAbilityResult();

    void SetStartAbility(ErrCode tValue);
    void SetTerminateAbility(ErrCode tValue);
    void SetTerminateAbilityResult(ErrCode tValue);

    int GetTerminateAbilityValue();
    void SetTerminateAbilityValue(int nValue);

    static std::shared_ptr<MockAbilityManagerClient> mock_instance_;
    static bool mock_intanceIsNull_;

    static std::shared_ptr<MockAbilityManagerClient> GetInstance();
    static void SetInstanceNull(bool flag);

private:
    ErrCode startAbility_;
    ErrCode terminateAbility_;
    ErrCode terminateAbilityResult_;

    int terminateAbilityValue_;
};
}  // namespace AAFwk
}  // namespace OHOS

namespace OHOS {
namespace AppExecFwk {
class MockIBundleMgr : public IRemoteStub<IBundleMgr> {
public:
    MockIBundleMgr() {};
    virtual ~MockIBundleMgr() {};
};

class MockAbilityContextDeal : public ContextDeal {
public:
    MockAbilityContextDeal() {};
    virtual ~MockAbilityContextDeal() {};

    sptr<IBundleMgr> GetBundleManager() const override
    {
        return sptr<IBundleMgr>(new (std::nothrow) MockIBundleMgr());
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_INTERFACE1_H
