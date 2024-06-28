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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_IMPL_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_IMPL_H

#include <string>

#include "extension_common.h"
#include "iremote_object.h"
#include "ui_extension_context.h"
#include "ui_extension_base_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionBaseImpl {
public:
    UIExtensionBaseImpl() = default;
    virtual ~UIExtensionBaseImpl() = default;

    virtual std::shared_ptr<ExtensionCommon> Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) = 0;

    virtual void OnStart(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam) = 0;

    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) = 0;

    virtual void OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd) = 0;

    virtual void OnStop() = 0;

    virtual void OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo) = 0;

    virtual void OnBackground() = 0;

    virtual void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) = 0;

    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) = 0;

    virtual void OnAbilityResult(int requestCode, int resultCode, const Want &resultData) = 0;

    virtual void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo) = 0;

    virtual void SetContext(const std::shared_ptr<UIExtensionContext> &context) = 0;

    virtual void BindContext() = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_BASE_IMPL_H
