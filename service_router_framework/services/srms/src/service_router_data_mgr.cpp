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
#include "service_router_data_mgr.h"

#include "bundle_info_resolve_util.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "sr_constants.h"
#include "sr_samgr_helper.h"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string SCHEME_SEPARATOR = "://";
const std::string SCHEME_SERVICE_ROUTER = "servicerouter";
}

bool ServiceRouterDataMgr::LoadAllBundleInfos()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM LoadAllBundleInfos");
    ClearAllBundleInfos();
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "The bundleMgrHelper is nullptr.");
        return false;
    }
    auto flags = (BundleFlag::GET_BUNDLE_WITH_ABILITIES | BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO);
    std::vector<BundleInfo> bundleInfos;
    if (!bundleMgrHelper->GetBundleInfos(flags, bundleInfos, SrSamgrHelper::GetCurrentActiveUserId())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Return false.");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    for (const auto &bundleInfo : bundleInfos) {
        UpdateBundleInfoLocked(bundleInfo);
    }
    return true;
}

bool ServiceRouterDataMgr::LoadBundleInfo(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM LoadBundleInfo");
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "The bundleMgrHelper is nullptr.");
        return false;
    }
    BundleInfo bundleInfo;
    auto flags = (BundleFlag::GET_BUNDLE_WITH_ABILITIES | BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO);
    if (!bundleMgrHelper->GetBundleInfo(bundleName, flags, bundleInfo, SrSamgrHelper::GetCurrentActiveUserId())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Return false.");
        return false;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    UpdateBundleInfoLocked(bundleInfo);
    return true;
}

void ServiceRouterDataMgr::UpdateBundleInfoLocked(const BundleInfo &bundleInfo)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM UpdateBundleInfo");
    InnerServiceInfo innerServiceInfo;
    auto infoItem = innerServiceInfos_.find(bundleInfo.name);
    if (infoItem != innerServiceInfos_.end()) {
        innerServiceInfo = infoItem->second;
    }
    innerServiceInfo.UpdateAppInfo(bundleInfo.applicationInfo);

    std::vector<PurposeInfo> purposeInfos;
    std::vector<BusinessAbilityInfo> businessAbilityInfos;
    if (BundleInfoResolveUtil::ResolveBundleInfo(bundleInfo, purposeInfos, businessAbilityInfos,
        innerServiceInfo.GetAppInfo())) {
        innerServiceInfo.UpdateInnerServiceInfo(purposeInfos, businessAbilityInfos);
        innerServiceInfos_.try_emplace(bundleInfo.name, innerServiceInfo);
    }
}

void ServiceRouterDataMgr::DeleteBundleInfo(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM DeleteBundleInfo");
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    auto infoItem = innerServiceInfos_.find(bundleName);
    if (infoItem == innerServiceInfos_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SRDM inner service info not found by bundleName");
        return;
    }
    innerServiceInfos_.erase(bundleName);
}

int32_t ServiceRouterDataMgr::QueryBusinessAbilityInfos(const BusinessAbilityFilter &filter,
    std::vector<BusinessAbilityInfo> &businessAbilityInfos) const
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM QueryBusinessAbilityInfos");
    BusinessType validType = GetBusinessType(filter);
    if (validType == BusinessType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SRDM QueryBusinessAbilityInfos, businessType is empty");
        return ERR_BUNDLE_MANAGER_PARAM_ERROR;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    for (const auto &item : innerServiceInfos_) {
        item.second.FindBusinessAbilityInfos(validType, businessAbilityInfos);
    }
    return ERR_OK;
}

int32_t ServiceRouterDataMgr::QueryPurposeInfos(const Want &want, const std::string purposeName,
    std::vector<PurposeInfo> &purposeInfos) const
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SRDM QueryPurposeInfos");
    if (purposeName.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SRDM QueryPurposeInfos, purposeName is empty");
        return ERR_BUNDLE_MANAGER_PARAM_ERROR;
    }

    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    ElementName element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    if (bundleName.empty()) {
        for (const auto &item : innerServiceInfos_) {
            item.second.FindPurposeInfos(purposeName, purposeInfos);
        }
    } else {
        auto infoItem = innerServiceInfos_.find(bundleName);
        if (infoItem == innerServiceInfos_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "SRDM QueryPurposeInfos, not found by bundleName.");
            return ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST;
        }
        infoItem->second.FindPurposeInfos(purposeName, purposeInfos);
    }
    return ERR_OK;
}

BusinessType ServiceRouterDataMgr::GetBusinessType(const BusinessAbilityFilter &filter) const
{
    if (filter.businessType != BusinessType::UNSPECIFIED) {
        return filter.businessType;
    }

    if (filter.uri.empty()) {
        return BusinessType::UNSPECIFIED;
    }

    OHOS::Uri uri = OHOS::Uri(filter.uri);
    if (uri.GetScheme().empty() || uri.GetHost().empty() || uri.GetScheme() != SCHEME_SERVICE_ROUTER) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetExtensionServiceType, invalid uri: %{public}s", filter.uri.c_str());
        return BusinessType::UNSPECIFIED;
    }
    return BundleInfoResolveUtil::findBusinessType(uri.GetHost());
}

void ServiceRouterDataMgr::ClearAllBundleInfos()
{
    std::lock_guard<std::mutex> lock(bundleInfoMutex_);
    if (!innerServiceInfos_.empty()) {
        innerServiceInfos_.clear();
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
