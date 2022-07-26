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

#ifndef OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H
#define OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H

#include <mutex>

#include "ability_manager_interface.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class WantAgentClient
 * WantAgentClient is used to access want agent in ability manager service.
 */
class WantAgentClient {
public:
    static WantAgentClient &GetInstance();

    sptr<IWantSender> GetWantSender(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken);

    ErrCode SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo);

    void CancelWantSender(const sptr<IWantSender> &sender);

    ErrCode GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid);

    ErrCode GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId);

    ErrCode GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName);

    ErrCode GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code);

    ErrCode GetPendingWantType(const sptr<IWantSender> &target, int32_t &type);

    void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);

    void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier);

    ErrCode GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want);

    ErrCode GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info);
private:
    class AbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        AbilityMgrDeathRecipient() = default;
        ~AbilityMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(AbilityMgrDeathRecipient);
    };
    WantAgentClient();
    virtual ~WantAgentClient();
    DISALLOW_COPY_AND_MOVE(WantAgentClient);

    sptr<IAbilityManager> GetAbilityManager();
    void ResetProxy(const wptr<IRemoteObject>& remote);

    std::recursive_mutex mutex_;
    sptr<IAbilityManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WANT_AGENT_CLIENT_H
