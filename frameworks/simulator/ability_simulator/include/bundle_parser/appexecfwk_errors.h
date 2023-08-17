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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H

#include "errors.h"

namespace OHOS {
enum {
    APPEXECFWK_MODULE_BUNDLEMGR = 0x02,
};

// Error code for BundleMgr
constexpr ErrCode APPEXECFWK_BUNDLEMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_BUNDLEMGR);
enum {
    ERR_APPEXECFWK_PARSE_BAD_PROFILE = 8519884,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR = 8519885,
    ERR_APPEXECFWK_PARSE_PROFILE_MISSING_PROP = 8519886,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_CHECK_ERROR = 8519888,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_SIZE_CHECK_ERROR = 8519889,
    ERR_BUNDLE_MANAGER_ABILITY_NOT_EXIST = 8521221,
    ERR_BUNDLE_MANAGER_MODULE_NOT_EXIST = 8521222,
};
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H
