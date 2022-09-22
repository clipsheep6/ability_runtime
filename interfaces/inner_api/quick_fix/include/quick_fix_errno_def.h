/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_QUICK_FIX_ERRNO_DEF_H
#define OHOS_ABILITY_RUNTIME_QUICK_FIX_ERRNO_DEF_H

namespace OHOS {
namespace AAFwk {
enum {
    QUICK_FIX_OK = 0,
    QUICK_FIX_WRITE_PARCEL_FAILED = 1,
    QUICK_FIX_READ_PARCEL_FAILED = 2,
    QUICK_FIX_SEND_REQUEST_FAILED = 3,
    QUICK_FIX_COPY_FILES_FAILED = 4,
    QUICK_FIX_INVALID_PARAM = 5,
    QUICK_FIX_CONNECT_FAILED = 6,
    QUICK_FIX_VERIFY_PERMISSION_FAILED = 7,
    QUICK_FIX_GET_BUNDLE_INFO_FAILED = 8,
    QUICK_FIX_DEPLOY_FAILED = 9,
    QUICK_FIX_SWICH_FAILED = 10,
    QUICK_FIX_DELETE_FAILED = 11,
    QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED = 12,
    QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED = 13,
    QUICK_FIX_REGISTER_OBSERVER_FAILED = 14,
    QUICK_FIX_APPMGR_INVALID = 15,
    QUICK_FIX_BUNDLEMGR_INVALID = 16,
    QUICK_FIX_SET_INFO_FAILED = 17,
    QUICK_FIX_PROCESS_TIMEOUT = 18,
    QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED = 19,
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_QUICK_FIX_ERRNO_DEF_H
