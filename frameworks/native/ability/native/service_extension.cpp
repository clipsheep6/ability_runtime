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

#include "service_extension.h"

#include "ability_handler.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_service_extension.h"
#include "runtime.h"
#include "service_extension_context.h"
#ifdef SUPPORT_GRAPHICS
#include "window_scene.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

CreatorFunc ServiceExtension::creator_ = nullptr;
void ServiceExtension::SetCreator(const CreatorFunc& creator)
{
    creator_ = creator;
}

ServiceExtension* ServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new ServiceExtension();
    }

    if (creator_) {
        return creator_(runtime);
    }

    HILOG_INFO("ServiceExtension::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsServiceExtension::Create(runtime);

        default:
            return new ServiceExtension();
    }
}

void ServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<ServiceExtensionContext>::Init(record, application, handler, token);
    HILOG_INFO("ServiceExtension begin init context");
    handler_ = handler;
#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("RegisterDisplayListener");
    std::shared_ptr<ServiceExtension> serviceExtension = std::static_pointer_cast<ServiceExtension>(shared_from_this());
    displayListener_ = new ServiceExtensionDisplayListener(serviceExtension);
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(displayListener_);
#endif
}

std::shared_ptr<ServiceExtensionContext> ServiceExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<ServiceExtensionContext> context =
        ExtensionBase<ServiceExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        HILOG_ERROR("ServiceExtension::CreateAndInitContext context is nullptr");
        return context;
    }

    auto appContext = Context::GetApplicationContext();
    if (appContext != nullptr) {
        auto config = appContext->GetConfiguration();
        if (config != nullptr) {
            context->SetConfiguration(std::make_shared<Configuration>(*config));
        }
    }

    return context;
}

void ServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    Extension::OnConfigurationUpdated(configuration);

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Context is invalid.");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        HILOG_DEBUG("Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        HILOG_DEBUG("Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
}

void ServiceExtension::ConfigurationUpdated()
{
    HILOG_DEBUG("called.");
    return;
}

void ServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Context is invalid.");
        return;
    }

    int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, Rosen::WindowScene::DEFAULT_DISPLAY_ID);
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
}

#ifdef SUPPORT_GRAPHICS
void ServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("OnCreate.");
}

void ServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("OnDestroy.");
}

void ServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("OnChange, displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        HILOG_ERROR("Configuration is invalid.");
        return;
    }

    HILOG_DEBUG("Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    HILOG_DEBUG("Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto serviceExtension = std::static_pointer_cast<ServiceExtension>(shared_from_this());
        auto task = [serviceExtension]() {
            if (serviceExtension) {
                serviceExtension->ConfigurationUpdated();
            }
        };
        handler_->PostTask(task);
    }

    HILOG_DEBUG("OnChange finished.");
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
