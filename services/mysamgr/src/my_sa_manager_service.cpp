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

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"

#include "my_sa_manager_service.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {

class MyConnection : public AbilityConnectionStub {
pulbic:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        std::string uri = ElementName.GetURI();
        HILOG_INFO("zhoujun MyConnection::OnAbilityConnectDone uri:%{public}s.", uri.c_str());
    }

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        std::string uri = ElementName.GetURI();
        HILOG_INFO("zhoujun MyConnection::OnAbilityDisconnectDone uri:%{public}s.", uri.c_str());
    }
};

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<MySaManagerService>::GetInstance().get());

MySaManagerService::MySaManagerService() : SystemAbility(MY_SA_MGR_SERVICE_ID, true) {}

MySaManagerService::~MySaManagerService()
{
    if (impl_ != nullptr) {
        impl_ = nullptr;
    }
}

void MySaManagerService::OnStart()
{
    HILOG_INFO("MySaManagerService start is triggered.");
    if (!Init()) {
        HILOG_ERROR("init failed.");
        return;
    }

    if (!registerToService_) {
        auto systemAabilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAabilityMgr || systemAabilityMgr->AddSystemAbility(MY_SA_MGR_SERVICE_ID, impl_) != 0) {
            HILOG_ERROR("fail to register to system ability manager");
            return;
        }
        HILOG_INFO("register to system ability manager success");
        registerToService_ = true;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    HILOG_INFO("zhoujun SA test begin");

    Want want1;
    want1.SetElementName("ohos.com.application.server", "ServiceAbility");
    sptr<IAbilityConnection> connect = new MyConnection;

    HILOG_INFO("zhoujun SA test: ConnectAbility");
    AbilityManagerClient::GetInstance().ConnectAbility(want1, connect, -1);
    AbilityManagerClient::GetInstance().DisconnectAbility(connect);

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    HILOG_INFO("zhoujun SA test: StartAbilityByCall");

    Want want2;
    want2.SetElementName("ohos.com.application.server", "ServiceAbility");
    sptr<IAbilityConnection> connect2 = new MyConnection;

    AbilityManagerClient::GetInstance().StartAbilityByCall(want2, -1);
    AbilityManagerClient::GetInstance().DisconnectAbility(connect2);
    HILOG_INFO("zhoujun SA test end");
}

void MySaManagerService::OnStop()
{
    HILOG_INFO("OnStop is called.");
    SelfClean();
}

bool MySaManagerService::IsServiceReady() const
{
    return ready_;
}

bool MySaManagerService::Init()
{
    if (ready_) {
        HILOG_WARN("init more than one time.");
        return true;
    }

    if (impl_ == nullptr) {
        impl_ = new MySaManagerStubImpl();
    }
    ready_ = true;
    return true;
}

void MySaManagerService::SelfClean()
{
    if (ready_) {
        ready_ = false;
        if (registerToService_) {
            registerToService_ = false;
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS
