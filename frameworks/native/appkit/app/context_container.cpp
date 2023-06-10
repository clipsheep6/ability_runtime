/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "constants.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "app_context.h"
#include "bundle_constants.h"
#include "hilog_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
// for api7 demo special
constexpr int CURRENT_ACCOUNT_ID = 100;

void ContextContainer::AttachBaseContext(const std::shared_ptr<ContextDeal> &base)
{
    if (base == nullptr) {
        HILOG_ERROR("ContextDeal::AttachBaseContext failed, base is nullptr");
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
        HILOG_ERROR("SetProcessInfo failed, info is empty");
        return;
    }
    processInfo_ = info;
}

std::shared_ptr<ApplicationInfo> ContextContainer::GetApplicationInfo() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetApplicationInfo();
    } else {
        HILOG_ERROR("ContextContainer::GetApplicationInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<Context> ContextContainer::GetApplicationContext() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetApplicationContext();
    } else {
        HILOG_ERROR("ContextContainer::GetApplicationContext baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetBundleCodePath()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleCodePath();
    } else {
        HILOG_ERROR("ContextContainer::GetBundleCodePath baseContext_ is nullptr");
        return "";
    }
}

const std::shared_ptr<AbilityInfo> ContextContainer::GetAbilityInfo()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAbilityInfo();
    } else {
        HILOG_ERROR("ContextContainer::GetAbilityInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<Context> ContextContainer::GetContext()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetContext();
    } else {
        HILOG_ERROR("ContextContainer::GetContext baseContext_ is nullptr");
        return nullptr;
    }
}

sptr<IBundleMgr> ContextContainer::GetBundleManager() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleManager();
    } else {
        HILOG_ERROR("ContextContainer::GetBundleManager baseContext_ is nullptr");
        return nullptr;
    }
}

std::shared_ptr<Global::Resource::ResourceManager> ContextContainer::GetResourceManager() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetResourceManager();
    } else {
        HILOG_ERROR("ContextContainer::GetResourceManager baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetDatabaseDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDatabaseDir();
    } else {
        HILOG_ERROR("ContextContainer::GetDatabaseDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetDataDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDataDir();
    } else {
        HILOG_ERROR("ContextContainer::GetDataDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetDir(const std::string &name, int mode)
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDir(name, mode);
    } else {
        HILOG_ERROR("ContextContainer::GetDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetFilesDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetFilesDir();
    } else {
        HILOG_ERROR("ContextContainer::GetFilesDir baseContext_ is nullptr");
        return "";
    }
}

std::string ContextContainer::GetBundleName() const
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetBundleName();
    } else {
        HILOG_ERROR("ContextContainer::GetBundleName baseContext_ is nullptr");
        return "";
    }
}

sptr<AAFwk::IAbilityManager> ContextContainer::GetAbilityManager()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAbilityManager();
    } else {
        HILOG_ERROR("ContextContainer::GetAbilityManager baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetAppType()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetAppType();
    } else {
        HILOG_ERROR("ContextContainer::GetAppType baseContext_ is nullptr");
        return "";
    }
}

std::shared_ptr<HapModuleInfo> ContextContainer::GetHapModuleInfo()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetHapModuleInfo();
    } else {
        HILOG_ERROR("ContextContainer::GetHapModuleInfo baseContext_ is nullptr");
        return nullptr;
    }
}

std::string ContextContainer::GetProcessName()
{
    return (processInfo_ != nullptr) ? processInfo_->GetProcessName() : "";
}

std::shared_ptr<Context> ContextContainer::CreateBundleContext(std::string bundleName, int accountId)
{
    if (bundleName.empty()) {
        HILOG_ERROR("ContextContainer::CreateBundleContext bundleName is empty");
        return nullptr;
    }

    if (strcmp(bundleName.c_str(), GetBundleName().c_str()) == 0) {
        return GetApplicationContext();
    }

    sptr<IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextContainer::CreateBundleContext GetBundleManager is nullptr");
        return nullptr;
    }

    BundleInfo bundleInfo;
    HILOG_INFO("CreateBundleContext length: %{public}zu, bundleName: %{public}s, accountId is %{public}d",
        bundleName.length(),
        bundleName.c_str(),
        accountId);
    int realAccountId = CURRENT_ACCOUNT_ID;
    if (accountId != DEFAULT_ACCOUNT_ID) {
        realAccountId = accountId;
    }
    bundleMgr->GetBundleInfo(bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, realAccountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("ContextContainer::CreateBundleContext GetBundleInfo is error");
        return nullptr;
    }

    std::shared_ptr<AppContext> appContext = std::make_shared<AppContext>();
    std::shared_ptr<ContextDeal> deal = std::make_shared<ContextDeal>();
    deal->SetSystemAppFlag(true);

    // init resourceManager.
    InitResourceManager(bundleInfo, deal);

    deal->SetApplicationInfo(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo));
    appContext->AttachBaseContext(deal);
    return appContext;
}

void ContextContainer::InitResourceManager(BundleInfo &bundleInfo, std::shared_ptr<ContextDeal> &deal)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (deal == nullptr || resourceManager == nullptr) {
        HILOG_ERROR("ContextContainer::InitResourceManager create resourceManager failed");
        return;
    }

    HILOG_DEBUG(
        "ContextContainer::InitResourceManager hapModuleInfos count: %{public}zu", bundleInfo.hapModuleInfos.size());
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
        HILOG_DEBUG("ContextContainer::InitResourceManager loadPath: %{public}s", loadPath.c_str());
        if (!resourceManager->AddResource(loadPath.c_str())) {
            HILOG_ERROR("ContextContainer::InitResourceManager AddResource failed");
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    resConfig->SetLocaleInfo("zh", "Hans", "CN");
#ifdef SUPPORT_GRAPHICS
    if (resConfig->GetLocaleInfo() != nullptr) {
        HILOG_INFO(
            "ContextContainer::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(),
            resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        HILOG_INFO("ContextContainer::InitResourceManager language: GetLocaleInfo is null.");
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

int ContextContainer::GetDisplayOrientation()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetDisplayOrientation();
    } else {
        HILOG_ERROR("ContextContainer::GetDisplayOrientation baseContext_ is nullptr");
        return static_cast<int>(DisplayOrientation::UNSPECIFIED);
    }
}

std::string ContextContainer::GetPreferencesDir()
{
    if (baseContext_ != nullptr) {
        return baseContext_->GetPreferencesDir();
    } else {
        HILOG_ERROR("ContextContainer::GetPreferencesDir baseContext_ is nullptr");
        return "";
    }
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
