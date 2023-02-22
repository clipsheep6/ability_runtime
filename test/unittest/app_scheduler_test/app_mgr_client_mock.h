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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H

#include <gmock/gmock.h>
#include "app_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrClientMock : public AppMgrClient {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"AppMgrClientMock");
    AppMgrClientMock()
    {}
    virtual ~AppMgrClientMock()
    {}
    MOCK_METHOD0(ConnectAppMgrService, AppMgrResultCode());
    MOCK_METHOD1(RegisterAppStateCallback, AppMgrResultCode(const sptr<IAppStateCallback> &callback));
    MOCK_METHOD5(LoadAbility, AppMgrResultCode(const sptr<IRemoteObject>&, const sptr<IRemoteObject>&,
        const AbilityInfo&, const ApplicationInfo&, const AAFwk::Want&));
    MOCK_METHOD2(TerminateAbility, AppMgrResultCode(const sptr<IRemoteObject>&, bool));
    MOCK_METHOD2(UpdateExtensionState, AppMgrResultCode(const sptr<IRemoteObject> &token, const ExtensionState state));
    MOCK_METHOD2(UpdateApplicationInfoInstalled, AppMgrResultCode(const std::string &bundleName, const int uid));
    MOCK_METHOD1(KillApplication, AppMgrResultCode(const std::string&));
    MOCK_METHOD2(KillApplicationByUid, AppMgrResultCode(const std::string &bundleName, const int uid));
    MOCK_METHOD1(ClearUpApplicationData, AppMgrResultCode(const std::string&));
    MOCK_METHOD1(StartupResidentProcess, void(const std::vector<AppExecFwk::BundleInfo> &bundleInfos));
    MOCK_METHOD2(StartSpecifiedAbility, void(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo));
    MOCK_METHOD1(GetAllRunningProcesses, AppMgrResultCode(std::vector<RunningProcessInfo> &info));
    MOCK_METHOD2(GetProcessRunningInfosByUserId, AppMgrResultCode(
        std::vector<RunningProcessInfo> &info, int32_t userId));
    MOCK_METHOD4(StartUserTestProcess, int(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId));
    MOCK_METHOD3(FinishUserTest, int(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName));
    MOCK_METHOD1(UpdateConfiguration, AppMgrResultCode(const Configuration &config));
    MOCK_METHOD1(GetConfiguration, AppMgrResultCode(Configuration& config));
    MOCK_METHOD2(GetAbilityRecordsByProcessID, int(
        const int pid, std::vector<sptr<IRemoteObject>> &tokens));
    MOCK_METHOD0(BlockAppService, AppMgrResultCode());
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
