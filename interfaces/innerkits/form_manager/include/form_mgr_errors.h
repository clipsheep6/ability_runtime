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

#ifndef OHOS_AAFWK_FORM_MANAGER_ERRORS_H
#define OHOS_AAFWK_FORM_MANAGER_ERRORS_H

#include "errors.h"

namespace OHOS {
enum {
    AAFWK_MODULE_FORMMGR = 0x03,
};

// offset of aafwk error, only be used in this file.
constexpr ErrCode AAFWK_FORMMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, AAFWK_MODULE_FORMMGR);

enum {
    ERR_APPEXECFWK_FORM_COMMON_CODE = AAFWK_FORMMGR_ERR_OFFSET + 1,
    ERR_APPEXECFWK_FORM_PERMISSION_DENY,
    ERR_APPEXECFWK_FORM_PERMISSION_DENY_BUNDLE,
    ERR_APPEXECFWK_FORM_PERMISSION_DENY_SYS,
    ERR_APPEXECFWK_FORM_GET_INFO_FAILED,
    ERR_APPEXECFWK_FORM_GET_BUNDLE_FAILED,
    ERR_APPEXECFWK_FORM_INVALID_PARAM,
    ERR_APPEXECFWK_FORM_INVALID_FORM_ID,
    ERR_APPEXECFWK_FORM_FORM_ID_NUM_ERR,
    ERR_APPEXECFWK_FORM_FORM_ARRAY_ERR,
    ERR_APPEXECFWK_FORM_RELEASE_FLG_ERR,
    ERR_APPEXECFWK_FORM_REFRESH_TIME_NUM_ERR,
    ERR_APPEXECFWK_FORM_INVALID_BUNDLENAME,
    ERR_APPEXECFWK_FORM_INVALID_MODULENAME,
    ERR_APPEXECFWK_FORM_INVALID_PROVIDER_DATA,
    ERR_APPEXECFWK_FORM_INVALID_REFRESH_TIME,
    ERR_APPEXECFWK_FORM_FORM_ID_ARRAY_ERR,
    ERR_APPEXECFWK_FORM_SERVER_STATUS_ERR,
    ERR_APPEXECFWK_FORM_CFG_NOT_MATCH_ID,
    ERR_APPEXECFWK_FORM_NOT_EXIST_ID,
    ERR_APPEXECFWK_FORM_PROVIDER_DATA_EMPTY,
    ERR_APPEXECFWK_FORM_BIND_PROVIDER_FAILED,
    ERR_APPEXECFWK_FORM_MAX_SYSTEM_FORMS,
    ERR_APPEXECFWK_FORM_EXCEED_INSTANCES_PER_FORM,
    ERR_APPEXECFWK_FORM_OPERATION_NOT_SELF,
    ERR_APPEXECFWK_FORM_PROVIDER_DEL_FAIL,
    ERR_APPEXECFWK_FORM_MAX_FORMS_PER_CLIENT,
    ERR_APPEXECFWK_FORM_MAX_SYSTEM_TEMP_FORMS,
    ERR_APPEXECFWK_FORM_NO_SUCH_MODULE,
    ERR_APPEXECFWK_FORM_NO_SUCH_ABILITY,
    ERR_APPEXECFWK_FORM_NO_SUCH_DIMENSION,
    ERR_APPEXECFWK_FORM_FA_NOT_INSTALLED,
    ERR_APPEXECFWK_FORM_MAX_REQUEST,
    ERR_APPEXECFWK_FORM_MAX_REFRESH,
    ERR_APPEXECFWK_FORM_GET_BMS_FAILED,
    ERR_APPEXECFWK_FORM_GET_HOST_FAILED,
    // error code in sdk
    ERR_APPEXECFWK_FORM_GET_FMS_FAILED,
    ERR_APPEXECFWK_FORM_SEND_FMS_MSG,
    ERR_APPEXECFWK_FORM_FORM_DUPLICATE_ADDED,
    ERR_APPEXECFWK_FORM_IN_RECOVER,
    ERR_APPEXECFWK_FORM_GET_SYSMGR_FAILED
};
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_ERRORS_H
