/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "quick_fix_callback_proxy.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
void QuickFixCallbackProxy::OnLoadPatchDone(int32_t resultCode, int32_t recordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");

    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_AND_RETURN(InterfaceToken, data, QuickFixCallbackProxy::GetDescriptor());
    WRITE_PARCEL_AND_RETURN(Int32, data, resultCode);
    WRITE_PARCEL_AND_RETURN(Int32, data, recordId);
    if (!SendRequestWithCmd(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_LOAD_PATCH, data, reply)) {
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "function finished.");
    return;
}

void QuickFixCallbackProxy::OnUnloadPatchDone(int32_t resultCode, int32_t recordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");

    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_AND_RETURN(InterfaceToken, data, QuickFixCallbackProxy::GetDescriptor());
    WRITE_PARCEL_AND_RETURN(Int32, data, resultCode);
    WRITE_PARCEL_AND_RETURN(Int32, data, recordId);
    if (!SendRequestWithCmd(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_UNLOAD_PATCH, data, reply)) {
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "function finished.");
    return;
}

void QuickFixCallbackProxy::OnReloadPageDone(int32_t resultCode, int32_t recordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");

    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_AND_RETURN(InterfaceToken, data, QuickFixCallbackProxy::GetDescriptor());
    WRITE_PARCEL_AND_RETURN(Int32, data, resultCode);
    WRITE_PARCEL_AND_RETURN(Int32, data, recordId);
    if (!SendRequestWithCmd(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_RELOAD_PAGE, data, reply)) {
        return;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "function finished.");
    return;
}

void QuickFixCallbackProxy::OnUnloadHotloadDone(int32_t resultCode, int32_t recordId)
{
    HILOG_DEBUG("function called.");

    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_AND_RETURN(InterfaceToken, data, QuickFixCallbackProxy::GetDescriptor());
    WRITE_PARCEL_AND_RETURN(Int32, data, resultCode);
    WRITE_PARCEL_AND_RETURN(Int32, data, recordId);
    if (!SendRequestWithCmd(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_UNLOAD_HOT, data, reply)) {
        return;
    }

    HILOG_DEBUG("function finished.");
    return;
}

void QuickFixCallbackProxy::OnHotloadFormDone(int32_t resultCode, int32_t recordId)
{
    HILOG_DEBUG("function called.");

    MessageParcel data;
    MessageParcel reply;
    WRITE_PARCEL_AND_RETURN(InterfaceToken, data, QuickFixCallbackProxy::GetDescriptor());
    WRITE_PARCEL_AND_RETURN(Int32, data, resultCode);
    WRITE_PARCEL_AND_RETURN(Int32, data, recordId);
    if (!SendRequestWithCmd(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_HOTLOAD_FORM, data, reply)) {
        return;
    }

    HILOG_DEBUG("function finished.");
    return;
}

bool QuickFixCallbackProxy::SendRequestWithCmd(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return false;
    }

    MessageOption option(MessageOption::TF_SYNC);
    auto ret = remote->SendRequest(code, data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed with error %{public}d.", ret);
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
