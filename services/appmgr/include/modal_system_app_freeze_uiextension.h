/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_MODAL_SYSTEM_APP_FREEZE_UIEXTENSION_H
#define OHOS_ABILITY_RUNTIME_MODAL_SYSTEM_APP_FREEZE_UIEXTENSION_H

#ifdef APP_NO_RESPONSE_DIALOG

#include <iremote_broker.h>
#include <semaphore.h>
#include <functional>

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ability_state.h"
#include "fault_data.h"
#include "iremote_stub.h"
#include "task_handler_wrap.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {

constexpr const char* APP_NO_RESPONSE_ABILITY = "AppAbnormalAbility";

class ModalSystemAppFreezeUIExtension {
public:
    static ModalSystemAppFreezeUIExtension &GetInstance();
    ModalSystemAppFreezeUIExtension() = default;
    virtual ~ModalSystemAppFreezeUIExtension();

    void ProcessAppFreeze(bool focusFlag, const FaultData &faultData, std::string pid, std::string bundleName,
        std::function<void()> callback, bool isDialogExist);

private:
    bool CreateModalUIExtension(std::string pid, std::string bundleName);
    AAFwk::Want CreateSystemDialogWant(std::string pid, std::string bundleName);

private:
    class AppFreezeDialogConnection : public AAFwk::AbilityConnectionStub {
    public:
        AppFreezeDialogConnection() = default;
        virtual ~AppFreezeDialogConnection() = default;

        void SetReqeustAppFreezeDialogWant(const AAFwk::Want &want);
        void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
            int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    private:
        AAFwk::Want want_;
    };

private:
    sptr<AppFreezeDialogConnection> GetConnection();

    std::mutex appFreezeResultMutex_;
    std::mutex dialogConnectionMutex_;
    std::string lastFreezePid;
    sptr<AppFreezeDialogConnection> dialogConnectionCallback_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // APP_NO_RESPONSE_DIALOG
#endif // OHOS_ABILITY_RUNTIME_MODAL_SYSTEM_APP_FREEZE_UIEXTENSION_H