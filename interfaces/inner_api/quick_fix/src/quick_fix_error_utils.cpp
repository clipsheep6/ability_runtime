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

#include "quick_fix_error_utils.h"

#include <map>

namespace OHOS {
namespace AAFwk {
namespace {
const std::map<int32_t, int32_t> INTERNAL_ERR_CODE_MAP = {
    { QUICK_FIX_OK,                         ERR_OK },
    { QUICK_FIX_WRITE_PARCEL_FAILED,        ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_READ_PARCEL_FAILED,         ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_SEND_REQUEST_FAILED,        ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_COPY_FILES_FAILED,          ERR_QUICKFIX_HQF_INVALID },
    { QUICK_FIX_INVALID_PARAM,              ERR_QUICKFIX_PARAM_INVALID },
    { QUICK_FIX_CONNECT_FAILED,             ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_VERIFY_PERMISSION_FAILED,   ERR_QUICKFIX_PERMISSION_DENIED },
    { QUICK_FIX_GET_BUNDLE_INFO_FAILED,     ERR_QUICKFIX_BUNDLE_NAME_INVALID },
    { QUICK_FIX_DEPLOY_FAILED,              ERR_QUICKFIX_HQF_DEPLOY_FAILED },
    { QUICK_FIX_SWICH_FAILED,               ERR_QUICKFIX_HQF_SWITCH_FAILED },
    { QUICK_FIX_DELETE_FAILED,              ERR_QUICKFIX_HQF_DELETE_FAILED },
    { QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED,   ERR_QUICKFIX_LOAD_PATCH_FAILED },
    { QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED,  ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_REGISTER_OBSERVER_FAILED,   ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_APPMGR_INVALID,             ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_BUNDLEMGR_INVALID,          ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_SET_INFO_FAILED,            ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_PROCESS_TIMEOUT,            ERR_QUICKFIX_INTERNAL_ERROR },
    { QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED, ERR_QUICKFIX_UNLOAD_PATCH_FAILED },
};

const std::map<int32_t, std::string> INTERNAL_ERR_MSG_MAP = {
    { QUICK_FIX_WRITE_PARCEL_FAILED,       "Write parcel failed." },
    { QUICK_FIX_READ_PARCEL_FAILED,        "Read parcel failed." },
    { QUICK_FIX_SEND_REQUEST_FAILED,       "Send request failed." },
    { QUICK_FIX_CONNECT_FAILED,            "Connect failed." },
    { QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED, "Reload page failed." },
    { QUICK_FIX_REGISTER_OBSERVER_FAILED,  "Register observer failed." },
    { QUICK_FIX_APPMGR_INVALID,            "AppMgr invalid." },
    { QUICK_FIX_BUNDLEMGR_INVALID,         "BundleMgr invalid." },
    { QUICK_FIX_SET_INFO_FAILED,           "Set quickfix info failed." },
    { QUICK_FIX_PROCESS_TIMEOUT,           "Process timeout." },
};

const std::map<int32_t, std::string> EXTERNAL_ERR_MSG_MAP = {
    { ERR_OK,                           "Success." },
    { ERR_QUICKFIX_PERMISSION_DENIED,   "The application does not have permission to call the interface." },
    { ERR_QUICKFIX_PARAM_INVALID,       "Invalid input parameter." },
    { ERR_QUICKFIX_BUNDLE_NAME_INVALID, "The specified bundleName is invalid." },
    { ERR_QUICKFIX_HQF_INVALID,         "The specified hqf is invalid. Hqf may not exist or inaccessible." },
    { ERR_QUICKFIX_HQF_DEPLOY_FAILED,   "Deploy hqf failed." },
    { ERR_QUICKFIX_HQF_SWITCH_FAILED,   "Switch hqf failed." },
    { ERR_QUICKFIX_HQF_DELETE_FAILED,   "Delete hqf failed." },
    { ERR_QUICKFIX_LOAD_PATCH_FAILED,   "Load patch failed." },
    { ERR_QUICKFIX_UNLOAD_PATCH_FAILED, "Unload patch failed." },
    { ERR_QUICKFIX_INTERNAL_ERROR,      "Internal error." },
};
} // namespace

int32_t QuickFixErrorUtil::GetErrorCode(int32_t errCode)
{
    // no need to convert is errCode is external error code.
    auto iter = EXTERNAL_ERR_MSG_MAP.find(errCode);
    if (iter != EXTERNAL_ERR_MSG_MAP.end()) {
        return errCode;
    }

    auto iterInternal = INTERNAL_ERR_CODE_MAP.find(errCode);
    if (iterInternal != INTERNAL_ERR_CODE_MAP.end()) {
        return iterInternal->second;
    }

    return ERR_QUICKFIX_INTERNAL_ERROR;
}

std::string QuickFixErrorUtil::GetErrorMessage(int32_t errCode)
{
    std::string errMsg;
    auto externalErrCode = GetErrorCode(errCode);
    auto iter = EXTERNAL_ERR_MSG_MAP.find(externalErrCode);
    if (iter != EXTERNAL_ERR_MSG_MAP.end()) {
        errMsg = iter->second;
    } else {
        errMsg = EXTERNAL_ERR_MSG_MAP.at(ERR_QUICKFIX_INTERNAL_ERROR);
    }

    auto iterInternal = INTERNAL_ERR_MSG_MAP.find(errCode);
    if (iterInternal != INTERNAL_ERR_MSG_MAP.end()) {
        errMsg += " " + iterInternal->second;
    }

    return errMsg;
}
} // namespace AAFwk
} // namespace OHOS