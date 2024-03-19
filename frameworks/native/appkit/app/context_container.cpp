/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "context_container.h"

#include <regex>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "app_context.h"
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "constants.h"
#include "hilog_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
// for api7 demo special
constexpr int CURRENT_ACCOUNT_ID = 100;
const int32_t TYPE_RESERVE = 1;
const int32_t TYPE_OTHERS = 2;

void ContextContainer::AttachBaseContext(const std::shared_ptr<ContextDeal> &base)
{
    if (base == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextDeal::AttachBaseContext failed, base is nullptr");
        return;
    }
    baseContext_ = base;
}

void ContextContainer::DetachBaseContext()
{
    if (baseContext_ != nullptr) {
        baseContext_.reset();
    }
    baseContext_ = nullptr;
}

std::shared_ptr<ProcessInfo> ContextContainer::GetProcessInfo() const
{
    return processInfo_;
}

void ContextContainer::SetProcessInfo(const std::shared_ptr<ProcessInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "SetProcessInfo failed, info is empty");
        return;
    }
    processInfo_ = info;
}

std::shared_ptr<ApplicationInfo> ContextContainer::GetApplicationInfo() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetApplicationInfo();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetApplicationInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<Context> ContextContainer::GetApplicationContext() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetApplicationContext();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetApplicationContext baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetBundleCodePath()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleCodePath();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetBundleCodePath baseContext_ is nullptr");
        return "";
    }
}

const std::shared_ptr<AbilityInfo> ContextContainer::GetAbilityInfo()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAbilityInfo();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetAbilityInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<Context> ContextContainer::GetContext()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetContext();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetContext baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<BundleMgrHelper> ContextContainer::GetBundleManager() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleManager();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "The baseContext_ is nullptr.");
        return nullptr;
    }
}

std::shared_ptr<Global::Resource::ResourceManager> ContextContainer::GetResourceManager() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetResourceManager();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetResourceManager baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetDatabaseDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDatabaseDir();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetDatabaseDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetDataDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDataDir();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetDataDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetDir(const std::string &name, int mode)
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDir(name, mode);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetFilesDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetFilesDir();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetFilesDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetBundleName() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleName();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetBundleName baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetBundleResourcePath()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleResourcePath();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetBundleResourcePath baseContext_ is nullptr");
        return "";
    }
}

sptr<AAFwk::IAbilityManager> ContextContainer::GetAbilityManager()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAbilityManager();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetAbilityManager baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetAppType()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAppType();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetAppType baseContext_ is nullptr");
        return "";
    }
}

void ContextContainer::SetPattern(int patternId)
{
    if (baseContext_ != nullptr) {
        baseContext_->SetPattern(patternId);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::SetPattern baseContext_ is nullptr");
    }
}

std::shared_ptr<HapModuleInfo> ContextContainer::GetHapModuleInfo()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetHapModuleInfo();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetHapModuleInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetProcessName()
{
    return (processInfo_ != nullptr) ? processInfo_->GetProcessName() : "";
}

std::shared_ptr<Context> ContextContainer::CreateBundleContext(std::string bundleName, int flag, int accountId)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "The bundleName is empty.");
        return nullptr;
    }

    if (strcmp(bundleName.c_str(), GetBundleName().c_str()) == 0) {
        return GetApplicationContext();
    }

    std::shared_ptr<BundleMgrHelper> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "The bundleMgr is nullptr.");
        return nullptr;
    }

    BundleInfo bundleInfo;
    TAG_LOGI(AAFwkTag::CONTEXT, "Length: %{public}zu, bundleName: %{public}s, accountId is %{public}d.",
        bundleName.length(),
        bundleName.c_str(),
        accountId);
    int realAccountId = CURRENT_ACCOUNT_ID;
    if (accountId != DEFAULT_ACCOUNT_ID) {
        realAccountId = accountId;
    }
    bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, realAccountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get Bundle Info.");
        return nullptr;
    }

    std::shared_ptr<AppContext> appContext = std::make_shared<AppContext>();
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>(true);

    // init resourceManager.
    InitResourceManager(bundleInfo, deal);

    deal->SetApplicationInfo(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo));
    appContext->AttachBaseContext(deal);
    return appContext;
}

void ContextContainer::InitResourceManager(BundleInfo &bundleInfo, std::shared_ptr<ContextDeal> &deal)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "InitResourceManager begin, bundleName:%{public}s, codePath:%{public}s",
        bundleInfo.name.c_str(), bundleInfo.applicationInfo.codePath.c_str());
    if (deal == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "InitResourceManager deal is nullptr");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    std::string moduleName;
    std::string hapPath;
    std::vector<std::string> overlayPaths;
    int32_t appType;
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE)) {
        appType = TYPE_RESERVE;
    } else if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        appType = TYPE_OTHERS;
    } else {
        appType = 0;
    }
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
            bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
        if (resourceManager == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "ContextImpl::InitResourceManager failed to create resourceManager");
            return;
        }
        deal->initResourceManager(resourceManager);
        return;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::InitResourceManager create resourceManager failed");
        return;
    }

    TAG_LOGD(AAFwkTag::CONTEXT, "ContextContainer::InitResourceManager hapModuleInfos count: %{public}zu",
        bundleInfo.hapModuleInfos.size());
    std::regex pattern(AbilityBase::Constants::ABS_CODE_PATH);
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        std::string loadPath;
        if (!hapModuleInfo.hapPath.empty()) {
            loadPath = hapModuleInfo.hapPath;
        } else {
            loadPath = hapModuleInfo.resourcePath;
        }
        if (loadPath.empty()) {
            continue;
        }
        loadPath = std::regex_replace(loadPath, pattern, AbilityBase::Constants::LOCAL_BUNDLES);
        TAG_LOGD(AAFwkTag::CONTEXT, "ContextContainer::InitResourceManager loadPath: %{public}s", loadPath.c_str());
        if (!resourceManager->AddResource(loadPath.c_str())) {
            TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::InitResourceManager AddResource failed");
        }
    }

    resConfig->SetLocaleInfo("zh", "Hans", "CN");
#ifdef SUPPORT_GRAPHICS
    if (resConfig->GetLocaleInfo() != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT,
            "ContextContainer::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        TAG_LOGI(AAFwkTag::CONTEXT, "ContextContainer::InitResourceManager language: GetLocaleInfo is null.");
    }
#endif
    resourceManager->UpdateResConfig(*resConfig);
    deal->initResourceManager(resourceManager);
}

Uri ContextContainer::GetCaller()
{
    Uri uri(uriString_);
    return uri;
}

void ContextContainer::SetUriString(const std::string &uri)
{
    uriString_ = uri;
}

std::string ContextContainer::GetString(int resId)
{
    if (baseContext_ != nullptr) {
        std::string ret = baseContext_->GetString(resId);
        return ret;
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetString baseContext_ is nullptr");
        return "";
    }
}

std::vector<std::string> ContextContainer::GetStringArray(int resId)
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetStringArray(resId);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetStringArray baseContext_ is nullptr");
        return std::vector<std::string>();
    }
}

std::vector<int> ContextContainer::GetIntArray(int resId)
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetIntArray(resId);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetIntArray baseContext_ is nullptr");
        return std::vector<int>();
    }
}

std::map<std::string, std::string> ContextContainer::GetTheme()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetTheme();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetTheme baseContext_ is nullptr");
        return std::map<std::string, std::string>();
    }
}

void ContextContainer::SetTheme(int themeId)
{
    if (baseContext_ != nullptr) {
        baseContext_->SetTheme(themeId);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::SetTheme baseContext_ is nullptr");
    }
}

std::map<std::string, std::string> ContextContainer::GetPattern()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetPattern();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetPattern baseContext_ is nullptr");
        return std::map<std::string, std::string>();
    }
}

int ContextContainer::GetColor(int resId)
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetColor(resId);
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetColor baseContext_ is nullptr");
        return INVALID_RESOURCE_VALUE;
    }
}

int ContextContainer::GetThemeId()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetThemeId();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetThemeId baseContext_ is nullptr");
        return -1;
    }
}

int ContextContainer::GetDisplayOrientation()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDisplayOrientation();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetDisplayOrientation baseContext_ is nullptr");
        return static_cast<int>(DisplayOrientation::UNSPECIFIED);
    }
}

std::string ContextContainer::GetPreferencesDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetPreferencesDir();
    } else {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetPreferencesDir baseContext_ is nullptr");
        return "";
    }
}

void ContextContainer::SetColorMode(int mode)
{
    if (baseContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::SetColorMode baseContext_ is nullptr");
        return;
    }

    baseContext_->SetColorMode(mode);
}

int ContextContainer::GetColorMode()
{
    if (baseContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ContextContainer::GetColorMode baseContext_ is nullptr");
        return -1;
    }

    return baseContext_->GetColorMode();
}

int ContextContainer::GetMissionId()
{
    return lifeCycleStateInfo_.missionId;
}

void ContextContainer::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    lifeCycleStateInfo_ = info;
}

AAFwk::LifeCycleStateInfo ContextContainer::GetLifeCycleStateInfo() const
{
    return lifeCycleStateInfo_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
