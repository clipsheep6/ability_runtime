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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_SERVICE_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_SERVICE_UTIL_H

#include "ability_manager_service.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
constexpr const char* MARKET_BUNDLE_NAME = "com.huawei.hmsapp.appgallery";
constexpr const char* MARKET_CROWD_TEST_BUNDLE_PARAM = "crowd_test_bundle_name";

[[maybe_unused]] static int StartAppgallery(const std::string &bundleName, const int requestCode, const int32_t userId,
    const std::string &action)
{
    std::string appGalleryBundleName;
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr || !bundleMgrHelper->QueryAppGalleryBundleName(appGalleryBundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get bundle manager helper failed or QueryAppGalleryBundleName failed.");
        appGalleryBundleName = MARKET_BUNDLE_NAME;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "appGalleryBundleName:%{public}s", appGalleryBundleName.c_str());

    Want want;
    want.SetElementName(appGalleryBundleName, "");
    want.SetAction(action);
    want.SetParam(MARKET_CROWD_TEST_BUNDLE_PARAM, bundleName);
    return DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, userId, requestCode);
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif