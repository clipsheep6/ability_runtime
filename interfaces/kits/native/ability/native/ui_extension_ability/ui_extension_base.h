/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_H

#include <string>

#include "extension.h"
#include "extension_base.h"
#include "iremote_object.h"
#include "ui_extension_context.h"
#include "ui_extension_base_impl.h"

namespace OHOS {
namespace AbilityRuntime {
template<class C = UIExtensionContext>
class UIExtensionBase : public ExtensionBase<C> {
public:
    UIExtensionBase() = default;
    virtual ~UIExtensionBase()
    {
        auto context = ExtensionBase<C>::GetContext();
        if (context != nullptr) {
            context->Unbind();
        }
    }

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override
    {
        ExtensionBase<C>::Init(record, application, handler, token);
        if (impl_ != nullptr) {
            impl_->SetAbilityInfo(Extension::abilityInfo_);
            impl_->SetContext(ExtensionBase<C>::GetContext());
            auto extensionCommon = impl_->Init(record, application, handler, token);
            ExtensionBase<C>::SetExtensionCommon(extensionCommon);
        }
    }

    void OnStart(const AAFwk::Want &want) override
    {
        Extension::OnStart(want);
        if (impl_ != nullptr) {
            auto launchParam = Extension::GetLaunchParam();
            impl_->OnStart(want, launchParam);
        }
    }

    void OnCommand(const AAFwk::Want &want, bool restart, int startId) override
    {
        Extension::OnCommand(want, restart, startId);
        if (impl_ != nullptr) {
            impl_->OnCommand(want, restart, startId);
        }
    }

    void OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd) override
    {
        Extension::OnCommandWindow(want, sessionInfo, winCmd);
        if (impl_ != nullptr) {
            impl_->OnCommandWindow(want, sessionInfo, winCmd);
        }
    }

    void OnStop() override
    {
        Extension::OnStop();
        if (impl_ != nullptr) {
            impl_->OnStop();
        }
    }

    void OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo) override
    {
        Extension::OnForeground(want, sessionInfo);
        if (impl_ != nullptr) {
            impl_->OnForeground(want, sessionInfo);
        }
    }

    void OnBackground() override
    {
        if (impl_ != nullptr) {
            impl_->OnBackground();
        }
        Extension::OnBackground();
    }

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override
    {
        Extension::OnConfigurationUpdated(configuration);
        if (impl_ != nullptr) {
            impl_->OnConfigurationUpdated(configuration);
        }
    }

    void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override
    {
        Extension::Dump(params, info);
        if (impl_ != nullptr) {
            impl_->Dump(params, info);
        }
    }

    void OnAbilityResult(int requestCode, int resultCode, const Want &resultData) override
    {
        Extension::OnAbilityResult(requestCode, resultCode, resultData);
        if (impl_ != nullptr) {
            impl_->OnAbilityResult(requestCode, resultCode, resultData);
        }
    }

    void SetUIExtensionBaseImpl(const std::shared_ptr<UIExtensionBaseImpl> &impl)
    {
        impl_ = impl;
    }

private:
    std::shared_ptr<UIExtensionBaseImpl> impl_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_H
