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

#include <singleton.h>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "mock_ability_manager_client_interface1.h"

namespace OHOS {
namespace AAFwk {
MockAbilityManagerClient::MockAbilityManagerClient()
{
    startAbility_ = ERR_INVALID_OPERATION;
    terminateAbility_ = ERR_INVALID_OPERATION;
    terminateAbilityValue_ = 0;
}
MockAbilityManagerClient::~MockAbilityManagerClient()
{}

std::shared_ptr<MockAbilityManagerClient> MockAbilityManagerClient::mock_instance_ = nullptr;
bool MockAbilityManagerClient::mock_intanceIsNull_ = true;

std::shared_ptr<MockAbilityManagerClient> MockAbilityManagerClient::GetInstance()
{
    if (mock_instance_ == nullptr) {
        mock_instance_ = std::make_shared<MockAbilityManagerClient>();
    }

    return mock_instance_;
}

void MockAbilityManagerClient::SetInstanceNull(bool flag)
{
    mock_intanceIsNull_ = flag;
}

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = MockAbilityManagerClient::GetInstance();
        }
    }
    if (MockAbilityManagerClient::mock_intanceIsNull_)
        return instance_;
    else
        return nullptr;
}

AbilityManagerClient::AbilityManagerClient()
{}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    return ERR_OK;
}

ErrCode ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want* resultWant)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want& want, sptr<IAbilityConnection> connect, sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    return ERR_OK;
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri& uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    return nullptr;
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpState(const std::string& args, std::vector<std::string>& state)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::Connect()
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want& want, sptr<IRemoteObject> callerToken)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::KillProcess(const std::string& bundleName, const bool clearPageStack)
{
    return ERR_OK;
}

ErrCode AbilityManagerClient::StartAbility(const Want& want, int requestCode)
{
    return MockAbilityManagerClient::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::StartAbility(const Want& want, sptr<IRemoteObject> callerToken, int requestCode)
{
    return MockAbilityManagerClient::GetInstance()->GetStartAbility();
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> callerToken, int requestCode)
{
    MockAbilityManagerClient::GetInstance()->SetTerminateAbilityValue(requestCode);
    return MockAbilityManagerClient::GetInstance()->GetTerminateAbility();
}

ErrCode MockAbilityManagerClient::GetStartAbility()
{
    return startAbility_;
}
ErrCode MockAbilityManagerClient::GetTerminateAbility()
{
    return terminateAbility_;
}

void MockAbilityManagerClient::SetStartAbility(ErrCode tValue)
{
    startAbility_ = tValue;
}
void MockAbilityManagerClient::SetTerminateAbility(ErrCode tValue)
{
    terminateAbility_ = tValue;
}

int MockAbilityManagerClient::GetTerminateAbilityValue()
{
    return terminateAbilityValue_;
}
void MockAbilityManagerClient::SetTerminateAbilityValue(int nValue)
{
    terminateAbilityValue_ = nValue;
}
}  // namespace AAFwk
}  // namespace OHOS
