/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "bundle_container.h"

#include <nlohmann/json.hpp>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "json_serializer.h"
#include "module_profile.h"

namespace OHOS {
namespace AppExecFwk {
BundleContainer& BundleContainer::GetInstance()
{
    static BundleContainer instance;
    return instance;
}

void BundleContainer::LoadBundleInfos(const std::vector<uint8_t> &buffer)
{
    bundleInfo_ = std::make_shared<InnerBundleInfo>();
    if (!bundleInfo_) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "bundleInfo_ is nullptr");
        return;
    }

    bundleInfo_->SetIsNewVersion(true);
    ModuleProfile moduleProfile;
    moduleProfile.TransformTo(buffer, *bundleInfo_);
}

std::shared_ptr<ApplicationInfo> BundleContainer::GetApplicationInfo() const
{
    if (bundleInfo_ != nullptr) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        bundleInfo_->GetApplicationInfo(0, Constants::UNSPECIFIED_USERID, *appInfo);
        return appInfo;
    }
    return nullptr;
}

std::shared_ptr<HapModuleInfo> BundleContainer::GetHapModuleInfo(const std::string &modulePackage) const
{
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        TAG_LOGI(AAFwkTag::ABILITY_SIM,
            "BundleContainer GetHapModuleInfo by modulePackage %{public}s", modulePackage.c_str());
        std::optional<HapModuleInfo> hapMouduleInfo = bundleInfo_->FindHapModuleInfo(modulePackage, uid);
        if (hapMouduleInfo) {
            auto hapInfo = std::make_shared<HapModuleInfo>();
            *hapInfo = *hapMouduleInfo;
            return hapInfo;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityInfo> BundleContainer::GetAbilityInfo(
    const std::string &moduleName, const std::string &abilityName) const
{
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        std::optional<AbilityInfo> ablilityInfo = bundleInfo_->FindAbilityInfo(moduleName, abilityName, uid);
        if (ablilityInfo) {
            auto aInfo = std::make_shared<AbilityInfo>();
            *aInfo = *ablilityInfo;
            return aInfo;
        }
    }
    return nullptr;
}
} // namespace AppExecFwk
} // namespace OHOS
