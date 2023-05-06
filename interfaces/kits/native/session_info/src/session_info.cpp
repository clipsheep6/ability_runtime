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

#include "session_info.h"

#include "ability_base_log_wrapper.h"
#include "session/host/include/zidl/session_interface.h"
#include "session/host/include/zidl/session_proxy.h"
#include "ui/rs_surface_node.h"

namespace OHOS {
namespace AAFwk {
bool SessionInfo::Marshalling(Parcel& parcel) const
{
    if (sessionToken) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(sessionToken->AsObject())) {
            ABILITYBASE_LOGE("Write session token failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (callerSession) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerSession->AsObject())) {
            ABILITYBASE_LOGE("Write caller session failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (callerToken) {
        if (!parcel.WriteBool(true) ||
            !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerToken)) {
            ABILITYBASE_LOGE("Write caller token failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            return false;
        }
    }

    if (!parcel.WriteUint64(persistentId)) {
        ABILITYBASE_LOGE("Write persistent id failed.");
        return false;
    }
    return true;
}

SessionInfo* SessionInfo::Unmarshalling(Parcel& parcel)
{
    SessionInfo* info = new SessionInfo();
    if (parcel.ReadBool()) {
        sptr<IRemoteObject> remoteObject = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        info->sessionToken = iface_cast<Rosen::ISession>(remoteObject);
    } else {
        ABILITYBASE_LOGE("Read session token failed.");
    }

    if (parcel.ReadBool()) {
        sptr<IRemoteObject> remoteObject = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        info->callerSession = iface_cast<Rosen::ISession>(remoteObject);
    } else {
        ABILITYBASE_LOGE("Read caller session failed.");
    }

    if (parcel.ReadBool()) {
        info->callerToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    } else {
        ABILITYBASE_LOGE("Read session token failed.");
    }

    info->persistentId = parcel.ReadUint64();
    return info;
}
}  // namespace AAFwk
}  // namespace OHOS
