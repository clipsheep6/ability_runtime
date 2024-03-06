/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H
#define OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H

#include <queue>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "ability_connect_callback_stub.h"
#include "assert_fault_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultProxy : public IRemoteProxy<IAssertFaultInterface> {
public:
    explicit AssertFaultProxy(const sptr<IRemoteObject>& impl);
    /**
     * Notify listeners of user operation results.
     *
     * @param status - User action result.
     */
    void NotifyDebugAssertResult(AAFwk::UserStatus status) override;

private:
    static inline BrokerDelegator<AssertFaultProxy> delegator_;
};

class AssertFaultRemoteDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit AssertFaultRemoteDeathRecipient(RemoteDiedHandler handler);
    ~AssertFaultRemoteDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

private:
    RemoteDiedHandler handler_;
};

class ModalSystemAssertUIExtension : public std::enable_shared_from_this<ModalSystemAssertUIExtension> {
public:
    ModalSystemAssertUIExtension() = default;
    virtual~ModalSystemAssertUIExtension();

    bool CreateModalUIExtension(const AAFwk::Want& want);

private:
    class AssertDialogConnection : public OHOS::AAFwk::AbilityConnectionStub {
        public:
            AssertDialogConnection() = default;
            virtual ~AssertDialogConnection();

            bool RequestShowDialog(const AAFwk::Want& want);
            void CleanUp();

            void OnAbilityConnectDone(const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject,
                int resultCode) override;
            void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

        private:
            std::mutex mutex_;
            std::atomic_bool isDialogShow_ = false;
            std::queue<AAFwk::Want> consumptionList_;
            sptr<IRemoteObject> remoteObject_;
            sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    };

    sptr<AssertDialogConnection> GetConnection();

    std::mutex dialogConnectionMutex_;
    sptr<AssertDialogConnection> dialogConnectionCallback_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ASSERT_FAULT_PROXY_H
