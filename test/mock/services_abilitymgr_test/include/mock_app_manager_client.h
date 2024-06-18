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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MANAGER_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MANAGER_CLIENT_H

#include <gmock/gmock.h>

#include "app_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrClient : public AppMgrClient {
public:
    MockAppMgrClient()
    {}
    ~MockAppMgrClient()
    {}
    MOCK_METHOD6(LoadAbility, AppMgrResultCode(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
        const AbilityInfo& abilityInfo, const ApplicationInfo& appInfo, const AAFwk::Want& want,
        int32_t abilityRecordId));

    MOCK_METHOD1(TerminateAbility, AppMgrResultCode(const sptr<IRemoteObject>&));
    MOCK_METHOD2(UpdateAbilityState, AppMgrResultCode(const sptr<IRemoteObject>& token, const AbilityState state));
    MOCK_METHOD1(RegisterAppStateCallback, AppMgrResultCode(const sptr<IAppStateCallback>& callback));
    MOCK_METHOD0(ConnectAppMgrService, AppMgrResultCode());
    MOCK_METHOD2(KillApplication, AppMgrResultCode(const std::string&, const bool clearPageStack));
    MOCK_METHOD5(AbilityBehaviorAnalysis,
        AppMgrResultCode(const sptr<IRemoteObject>& token, const sptr<IRemoteObject>& preToken,
            const int32_t visibility, const int32_t perceptibility, const int32_t connectionState));
    MOCK_METHOD1(KillProcessByAbilityToken, AppMgrResultCode(const sptr<IRemoteObject>& token));
    MOCK_METHOD1(KillProcessesByUserId, AppMgrResultCode(int32_t userId));
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MANAGER_CLIENT_H
