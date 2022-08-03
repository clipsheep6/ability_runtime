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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H

#include <functional>
#include <vector>

#include "singleton.h"
#include "quick_fix_manager_interface.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void()>;
class QuickFixManagerClient : public DelayedSingleton<QuickFixManagerClient>,
                              public std::enable_shared_from_this<QuickFixManagerClient> {
public:
    QuickFixManagerClient() = default;
    ~QuickFixManagerClient() = default;

    /**
     * @brief Apply quick fix.
     *
     * @param quickFixFiles quick fix files need to apply, this value should include file path and file name.
     */
    void ApplyQuickFix(const std::vector<std::string> &quickFixFiles);

private:
    sptr<IQuickFixManager> ConnectQuickFixManagerService();
    void ClearProxy();
    DISALLOW_COPY_AND_MOVE(QuickFixManagerClient);

    class QfmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        QfmsDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        ~QfmsDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    sptr<IQuickFixManager> quickFixMgr_ = nullptr;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_CLIENT_H
