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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_IMPL_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_IMPL_H

#include <functional>
#include <map>

#include "bundlemgr/bundle_mgr_interface.h"
#include "quick_fix_manager_stub.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;

class QuickFixManagerStubImpl : public QuickFixManagerStub,
                                public std::enable_shared_from_this<QuickFixManagerStubImpl> {
public:
    QuickFixManagerStubImpl() = default;
    virtual ~QuickFixManagerStubImpl() = default;

    void ApplyQuickFix(const std::vector<std::string> &quickFixFiles) override;

private:
    sptr<AppExecFwk::IBundleMgr> ConnectBundleManager();
    void ClearProxy();

    class BMSDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        BMSDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        ~BMSDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    std::mutex bmsMutex_;
    sptr<AppExecFwk::IBundleMgr> bundleManager_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_QUICK_FIX_MANAGER_STUB_IMPL_H
