/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "sender_info.h"

#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
bool SenderInfo::ReadFromParcel(Parcel &parcel)
{
    HILOG_INFO("call");

    code = parcel.ReadInt32();
    std::unique_ptr<Want> wantResquest(parcel.ReadParcelable<Want>());
    if (wantResquest == nullptr) {
        HILOG_ERROR("wantResquest is nullptr.");
        return false;
    }
    want = *wantResquest;
    resolvedType = Str16ToStr8(parcel.ReadString16());

    if (parcel.ReadBool()) {
        sptr<IRemoteObject> finishedReceiverResquest = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        if (finishedReceiverResquest == nullptr) {
            HILOG_ERROR("remote object is nullptr.");
            return false;
        }
        finishedReceiver = iface_cast<IWantReceiver>(finishedReceiverResquest);
        if (!finishedReceiver) {
            HILOG_ERROR("receiver is nullptr.");
            return false;
        }
    }
    requiredPermission = Str16ToStr8(parcel.ReadString16());
    return true;
}

SenderInfo *SenderInfo::Unmarshalling(Parcel &parcel)
{
    HILOG_INFO("call");

    SenderInfo *info = new (std::nothrow) SenderInfo();
    if (info == nullptr) {
        HILOG_ERROR("senderInfo is nullptr.");
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        HILOG_ERROR("ReadFromParcel failed.");
        delete info;
        info = nullptr;
    }
    return info;
}

bool SenderInfo::Marshalling(Parcel &parcel) const
{
    HILOG_INFO("call");

    if (!parcel.WriteInt32(code)) {
        HILOG_ERROR("Failed to write code");
        return false;
    }
    if (!parcel.WriteParcelable(&want)) {
        HILOG_ERROR("Failed to write want");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(resolvedType))) {
        HILOG_ERROR("Failed to write resolvedType");
        return false;
    }
    if (!parcel.WriteBool(finishedReceiver != nullptr)) {
        HILOG_ERROR("Failed to write the flag which indicate whether receiver is null");
        return false;
    }
    if (finishedReceiver) {
        if (finishedReceiver->AsObject() == nullptr) {
            HILOG_ERROR("finishedReceiver->AsObject is null");
            return false;
        }
        if (!(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(finishedReceiver->AsObject())) {
            HILOG_ERROR("Failed to write receiver");
            return false;
        }
    }
    if (!parcel.WriteString16(Str8ToStr16(requiredPermission))) {
        HILOG_ERROR("Failed to write requiredPermission");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
