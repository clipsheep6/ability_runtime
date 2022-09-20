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

#include "quick_fix_manager_proxy.h"

#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "message_parcel.h"
#include "permission_verification.h"
#include "quick_fix_errno_def.h"
#include "quick_fix_util.h"

namespace OHOS {
namespace AAFwk {
int32_t QuickFixManagerProxy::ApplyQuickFix(const std::vector<std::string> &quickFixFiles)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    if (!AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission()) {
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    auto bundleQuickFixMgr = QuickFixUtil::GetBundleQuickFixMgrProxy();
    if (bundleQuickFixMgr == nullptr) {
        return QUICK_FIX_CONNECT_FAILED;
    }

    HILOG_DEBUG("hqf file number need to apply: %{public}zu.", quickFixFiles.size());
    std::vector<std::string> destFiles;
    if (bundleQuickFixMgr->CopyFiles(quickFixFiles, destFiles) != 0) {
        HILOG_ERROR("Copy files failed.");
        return QUICK_FIX_COPY_FILES_FAILED;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AAFwk::IQuickFixManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return QUICK_FIX_WRITE_PARCEL_FAILED;
    }

    if (!data.WriteStringVector(destFiles)) {
        HILOG_ERROR("Write quick fix files failed.");
        return QUICK_FIX_WRITE_PARCEL_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = remote->SendRequest(QuickFixMgrCmd::ON_APPLY_QUICK_FIX, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("Send request failed with error %{public}d.", ret);
        return QUICK_FIX_SEND_REQUEST_FAILED;
    }

    HILOG_DEBUG("function finished.");
    return reply.ReadInt32();
}

int32_t QuickFixManagerProxy::GetApplyedQuickFixInfo(const std::string &bundleName,
    ApplicationQuickFixInfo &quickFixInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    if (!AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission()) {
        return QUICK_FIX_VERIFY_PERMISSION_FAILED;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(AAFwk::IQuickFixManager::GetDescriptor())) {
        HILOG_ERROR("GetApplyedQuickFixInfo, Write interface token failed.");
        return QUICK_FIX_WRITE_PARCEL_FAILED;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("GetApplyedQuickFixInfo, Write quick fix files failed.");
        return QUICK_FIX_WRITE_PARCEL_FAILED;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("GetApplyedQuickFixInfo, Remote is nullptr.");
        return QUICK_FIX_CONNECT_FAILED;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = remote->SendRequest(QuickFixMgrCmd::ON_GET_APPLYED_QUICK_FIX_INFO, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("GetApplyedQuickFixInfo, Send request failed with error %{public}d.", ret);
        return QUICK_FIX_SEND_REQUEST_FAILED;
    }

    auto result = reply.ReadInt32();
    if (result == QUICK_FIX_OK) {
        std::unique_ptr<ApplicationQuickFixInfo> info(reply.ReadParcelable<ApplicationQuickFixInfo>());
        if (info != nullptr) {
            quickFixInfo = *info;
        }
    }

    HILOG_DEBUG("function finished with %{public}d.", result);
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
