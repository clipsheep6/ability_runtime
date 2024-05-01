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
#include "tokenid_permission.h"

#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {

bool TokenIdPermission::VerifyProxyAuthorizationUriPermission()
{
    if (!initProxyAuthorizationUriPermission_) {
        initProxyAuthorizationUriPermission_ = true;
        haveProxyAuthorizationUriPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    }
    return haveProxyAuthorizationUriPermission_;
}

bool TokenIdPermission::VerifyFileAccessManagerPermission()
{
    if (!initFileAccessManagerPermission_) {
        initFileAccessManagerPermission_ = true;
        haveFileAccessManagerPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_FILE_ACCESS_MANAGER);
    }
    return haveFileAccessManagerPermission_;
}

bool TokenIdPermission::VerifyReadImageVideoPermission()
{
    if (!initReadImageVideoPermission_) {
        initReadImageVideoPermission_ = true;
        haveReadImageVideoPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_READ_IMAGEVIDEO);
    }
    return haveReadImageVideoPermission_;
}

bool TokenIdPermission::VerifyWriteImageVideoPermission()
{
    if (!initWriteImageVideoPermission_) {
        initWriteImageVideoPermission_ = true;
        haveWriteImageVideoPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_WRITE_IMAGEVIDEO);
    }
    return haveWriteImageVideoPermission_;
}

bool TokenIdPermission::VerifyReadAudioPermission()
{
    if (!initReadAudioPermission_) {
        initReadAudioPermission_ = true;
        haveReadAudioPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_READ_AUDIO);
    }
    return haveReadAudioPermission_;
}

bool TokenIdPermission::VerifyWriteAudioPermission()
{
    if (!initWriteAudioPermission_) {
        initWriteAudioPermission_ = true;
        haveWriteAudioPermission_ = PermissionVerification::GetInstance()->VerifyPermissionByTokenId(
            tokenId_, PermissionConstants::PERMISSION_WRITE_AUDIO);
    }
    return haveWriteAudioPermission_;
}
} // OHOS
} // AAFwk