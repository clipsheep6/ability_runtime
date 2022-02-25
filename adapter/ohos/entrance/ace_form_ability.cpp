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

#include "adapter/ohos/entrance/ace_form_ability.h"

#include "res_config.h"
#include "resource_manager.h"

#include "adapter/ohos/entrance/pa_container.h"
#include "adapter/ohos/entrance/pa_engine/pa_backend.h"
#include "adapter/ohos/entrance/platform_event_callback.h"
#include "adapter/ohos/entrance/utils.h"
#include "base/log/log.h"
#include "core/common/backend.h"

namespace OHOS::Ace {
namespace {
const int INSTANCE_NUM_MAX = 1000;
} // namespace
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

using FormPlatformFinish = std::function<void()>;
class FormPlatformEventCallback final : public Platform::PlatformEventCallback {
public:
    explicit FormPlatformEventCallback(FormPlatformFinish onFinish) : onFinish_(onFinish) {}

    ~FormPlatformEventCallback() = default;

    virtual void OnFinish() const
    {
        LOGI("FormPlatformEventCallback OnFinish");
        if (onFinish_) {
            onFinish_();
        }
    }

    virtual void OnStatusBarBgColorChanged(uint32_t color)
    {
        LOGI("FormPlatformEventCallback OnStatusBarBgColorChanged");
    }

private:
    FormPlatformFinish onFinish_;
};

int32_t AceFormAbility::instanceId_ = 300000;
const std::string AceFormAbility::START_PARAMS_KEY = "__startParams";
const std::string AceFormAbility::URI = "url";

REGISTER_AA(AceFormAbility)

AceFormAbility::AceFormAbility()
{
    abilityId_ = instanceId_;
    instanceId_ += INSTANCE_NUM_MAX;
}

OHOS::AppExecFwk::FormProviderInfo AceFormAbility::OnCreate(const OHOS::AAFwk::Want& want)
{
    LOGI("AceFormAbility::OnCreate called");

    // get url
    std::string parsedUrl;
    if (want.HasParameter(URI)) {
        parsedUrl = want.GetStringParam(URI);
    } else {
        parsedUrl = "form.js";
    }

    std::string formIdStr = want.GetStringParam(AppExecFwk::Constants::PARAM_FORM_IDENTITY_KEY);
    LOGI("AceFormAbility::formId = %{public}s.", formIdStr.c_str());
    int64_t formId = atoll(formIdStr.c_str());
    formMap_.emplace(formId, abilityId_++);

    // get asset
    auto packagePathStr = GetBundleCodePath();
    auto moduleInfo = GetHapModuleInfo();
    if (moduleInfo != nullptr) {
        packagePathStr += "/" + moduleInfo->name + "/";
    }

    // init form ability
    BackendType backendType = BackendType::FORM;
    bool isArkApp = GetIsArkFromConfig(packagePathStr);
    Platform::PaContainer::CreateContainer(formMap_.at(formId), backendType, isArkApp, this,
        std::make_unique<FormPlatformEventCallback>([this]() { TerminateAbility(); }));

    std::shared_ptr<AbilityInfo> info = GetAbilityInfo();
    if (info != nullptr && !info->srcPath.empty()) {
        LOGI("AceFormAbility::OnCreate assetBasePathStr: %{public}s, parsedUrl: %{public}s",
            info->srcPath.c_str(), parsedUrl.c_str());
        auto assetBasePathStr = { "assets/js/" + info->srcPath + "/" };
        Platform::PaContainer::AddAssetPath(formMap_.at(formId), packagePathStr, assetBasePathStr);
    } else {
        LOGI("AceFormAbility::OnCreate parsedUrl: %{public}s", parsedUrl.c_str());
        auto assetBasePathStr = { std::string("assets/js/default/"), std::string("assets/js/share/") };
        Platform::PaContainer::AddAssetPath(formMap_.at(formId), packagePathStr, assetBasePathStr);
    }
    std::shared_ptr<ApplicationInfo> appInfo = GetApplicationInfo();
    if (appInfo) {
        std::string nativeLibraryPath = appInfo->nativeLibraryPath;
        if (!nativeLibraryPath.empty()) {
            if (nativeLibraryPath.back() == '/') {
                nativeLibraryPath.pop_back();
            }
            std::string libPath = GetBundleCodePath();
            if (libPath.back() == '/') {
                libPath += nativeLibraryPath;
            } else {
                libPath += "/" + nativeLibraryPath;
            }
            LOGI("napi lib path = %{private}s", libPath.c_str());
            Platform::PaContainer::AddLibPath(abilityId_, libPath);
        }
    }

    // run form ability
    Platform::PaContainer::RunPa(formMap_.at(formId), parsedUrl, want);
    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    formProviderInfo.SetFormData(Platform::PaContainer::GetFormData(formMap_.at(formId)));
    std::string formData = formProviderInfo.GetFormData().GetDataString();
    LOGI("AceFormAbility::OnCreate return ok, formData: %{public}s", formData.c_str());
    return formProviderInfo;
}

void AceFormAbility::OnDelete(const int64_t formId)
{
    Platform::PaContainer::OnDelete(formMap_.at(formId), formId);
}

void AceFormAbility::OnTriggerEvent(const int64_t formId, const std::string& message)
{
    Platform::PaContainer::OnTriggerEvent(formMap_.at(formId), formId, message);
}

void AceFormAbility::OnUpdate(const int64_t formId)
{
    Platform::PaContainer::OnUpdate(formMap_.at(formId), formId);
}

void AceFormAbility::OnCastTemptoNormal(const int64_t formId)
{
    Platform::PaContainer::OnCastTemptoNormal(formMap_.at(formId), formId);
}

void AceFormAbility::OnVisibilityChanged(const std::map<int64_t, int32_t>& formEventsMap)
{
    for (const auto& form : formMap_) {
        Platform::PaContainer::OnVisibilityChanged(form.second, formEventsMap);
    }
}

void AceFormAbility::OnStart(const OHOS::AAFwk::Want& want)
{
    LOGI("AceFormAbility::OnStart start");
    Ability::OnStart(want);
}

void AceFormAbility::OnStop()
{
    LOGI("AceFormAbility::OnStop start ");
    Ability::OnStop();
}

sptr<IRemoteObject> AceFormAbility::OnConnect(const Want& want)
{
    LOGI("AceFormAbility::OnConnect start");
    Ability::OnConnect(want);
    return GetFormRemoteObject();
}

void AceFormAbility::OnDisconnect(const Want& want)
{
    LOGI("AceFormAbility::OnDisconnect start");
    Ability::OnDisconnect(want);
}

} // namespace OHOS::Ace
