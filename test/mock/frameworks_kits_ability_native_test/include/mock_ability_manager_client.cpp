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

#include "mock_ability_manager_client.h"
#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_manager_client.h"
#undef private
#undef protected

namespace OHOS {
namespace AAFwk {
std::shared_ptr<AbilityManagerClient> mockInstance_ = nullptr;
std::mutex mockMutex_;

std::shared_ptr<AbilityManagerClient> AbilityManagerClient::GetInstance()
{
    if (mockInstance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mockMutex_);
        if (mockInstance_ == nullptr) {
            mockInstance_ = std::make_shared<AbilityManagerClient>();
        }
    }
    return mockInstance_;
}

AbilityManagerClient::AbilityManagerClient()
{
    // log
}

AbilityManagerClient::~AbilityManagerClient()
{}

ErrCode AbilityManagerClient::AttachAbilityThread(
    sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token)
{
    GTEST_LOG_(INFO) << "Mock AbilityManagerClient::AttachAbilityThread called";
    return -1;
}

ErrCode AbilityManagerClient::AbilityTransitionDone(sptr<IRemoteObject> token, int state, const PacMap& saveData)
{
    return -1;
}

ErrCode AbilityManagerClient::ScheduleConnectAbilityDone(
    sptr<IRemoteObject> token, sptr<IRemoteObject> remoteObject)
{
    return -1;
}

ErrCode AbilityManagerClient::ScheduleDisconnectAbilityDone(sptr<IRemoteObject> token)
{
    return -1;
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityDone(sptr<IRemoteObject> token)
{
    return -1;
}

ErrCode AbilityManagerClient::ScheduleCommandAbilityWindowDone(
    sptr<IRemoteObject> token,
    sptr<SessionInfo> sessionInfo,
    WindowCommand winCmd,
    AbilityCommand abilityCmd)
{
    return -1;
}

ErrCode AbilityManagerClient::StartAbility(const Want& want, int requestCode, int32_t userId)
{
    return -1;
}

ErrCode AbilityManagerClient::TerminateAbility(sptr<IRemoteObject> token, int resultCode, const Want* resultWant)
{
    return -1;
}

ErrCode AbilityManagerClient::ConnectAbility(
    const Want& want, sptr<IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    return -1;
}

ErrCode AbilityManagerClient::DisconnectAbility(sptr<IAbilityConnection> connect)
{
    return -1;
}

sptr<IAbilityScheduler> AbilityManagerClient::AcquireDataAbility(
    const Uri& uri, bool tryBind, sptr<IRemoteObject> callerToken)
{
    GTEST_LOG_(INFO) << "Mock AcquireDataAbility called";
    sptr<IAbilityScheduler> dataScheduler = new (std::nothrow) OHOS::AppExecFwk::MockAbilityThread();
    return dataScheduler;
}

ErrCode AbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, sptr<IRemoteObject> callerToken)
{
    GTEST_LOG_(INFO) << "Mock ReleaseDataAbility called";
    return ERR_OK;
}

ErrCode AbilityManagerClient::DumpState(const std::string& args, std::vector<std::string>& state)
{
    return -1;
}

ErrCode AbilityManagerClient::Connect()
{
    return -1;
}

ErrCode AbilityManagerClient::StopServiceAbility(const Want& want, sptr<IRemoteObject> token)
{
    return -1;
}
}  // namespace AAFwk
}  // namespace OHOS
