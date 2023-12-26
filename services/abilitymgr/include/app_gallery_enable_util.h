/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_APP_GALLERY_ENABLE_UTIL_H
#define OHOS_ABILITY_APP_GALLERY_ENABLE_UTIL_H

#include <string>
#include "hilog_wrapper.h"
#include "parameters.h"

namespace OHOS {
namespace AAFwk {
namespace AppGalleryEnableUtil {
const std::string ENABLE_APP_GALLERY_SELECTOR_UTIL = "abilitymanagerservice.support.appgallery.selector";

inline bool IsEnableAppGallerySelector()
{
    HILOG_DEBUG("call");
    std::string ret = OHOS::system::GetParameter(ENABLE_APP_GALLERY_SELECTOR_UTIL, "true");
    if (ret == "true") {
        return true;
    }
    return false;
}
}  // namespace AppGalleryEnableUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_APP_GALLERY_ENABLE_UTIL_H
