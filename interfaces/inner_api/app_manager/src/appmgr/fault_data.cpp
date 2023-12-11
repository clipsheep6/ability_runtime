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

#include "fault_data.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool FaultData::ReadFromParcel(Parcel &parcel)
{
    std::string strValue;
    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Name read string failed.");
        return false;
    }
    errorObject.name = strValue;

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Message read string failed.");
        return false;
    }
    errorObject.message = strValue;

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Stack read string failed.");
        return false;
    }
    errorObject.stack = strValue;

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        HILOG_ERROR("FaultType read int32 failed.");
        return false;
    }
    faultType = static_cast<FaultDataType>(type);

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("TimeoutMarkers read string failed.");
        return false;
    }
    timeoutMarkers = strValue;

    waitSaveState = parcel.ReadBool();
    notifyApp = parcel.ReadBool();
    forceExit = parcel.ReadBool();
    state = parcel.ReadUint32();
    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }
    return true;
}

FaultData *FaultData::Unmarshalling(Parcel &parcel)
{
    FaultData *info = new FaultData();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool FaultData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(errorObject.name)) {
        HILOG_ERROR("Name [%{public}s] write string failed.", errorObject.name.c_str());
        return false;
    }

    if (!parcel.WriteString(errorObject.message)) {
        HILOG_ERROR("Message [%{public}s] write string failed.", errorObject.message.c_str());
        return false;
    }
    
    if (!parcel.WriteString(errorObject.stack)) {
        HILOG_ERROR("Stack [%{public}s] write string failed.", errorObject.stack.c_str());
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        HILOG_ERROR("FaultType [%{public}d] write int32 failed.", static_cast<int32_t>(faultType));
        return false;
    }

    if (!parcel.WriteString(timeoutMarkers)) {
        HILOG_ERROR("TimeoutMarkers [%{public}s] write string failed.", timeoutMarkers.c_str());
        return false;
    }

    if (!parcel.WriteBool(waitSaveState)) {
        HILOG_ERROR("WaitSaveState [%{public}s] write bool failed.", waitSaveState ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(notifyApp)) {
        HILOG_ERROR("NotifyApp [%{public}s] write bool failed.", notifyApp ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(forceExit)) {
        HILOG_ERROR("ForceExit [%{public}s] write bool failed.", forceExit ? "true" : "false");
        return false;
    }

    if (!parcel.WriteUint32(state)) {
        HILOG_ERROR("State [%{public}u] write uint32 failed.", state);
        return false;
    }

    if (token == nullptr) {
        if (!parcel.WriteBool(false)) {
            HILOG_ERROR("Token falge [false] write bool failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(true) || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(token)) {
            HILOG_ERROR("Token falge [true] write bool failed.");
            return false;
        }
    }
    return true;
}

bool AppFaultDataBySA::ReadFromParcel(Parcel &parcel)
{
    std::string strValue;
    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Name read string failed.");
        return false;
    }
    errorObject.name = strValue;

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Message read string failed.");
        return false;
    }
    errorObject.message = strValue;

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("Stack read string failed.");
        return false;
    }
    errorObject.stack = strValue;

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        HILOG_ERROR("Type read int32 failed.");
        return false;
    }
    faultType = static_cast<FaultDataType>(type);

    if (!parcel.ReadInt32(pid)) {
        HILOG_ERROR("Pid read int32 failed.");
        return false;
    }

    if (!parcel.ReadString(strValue)) {
        HILOG_ERROR("TimeoutMarkers read string failed.");
        return false;
    }
    timeoutMarkers = strValue;

    waitSaveState = parcel.ReadBool();
    notifyApp = parcel.ReadBool();
    forceExit = parcel.ReadBool();
    state = parcel.ReadUint32();
    if (parcel.ReadBool()) {
        token = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    }
    return true;
}

AppFaultDataBySA *AppFaultDataBySA::Unmarshalling(Parcel &parcel)
{
    AppFaultDataBySA *info = new AppFaultDataBySA();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool AppFaultDataBySA::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(errorObject.name)) {
        HILOG_ERROR("Name [%{public}s] write string failed.", errorObject.name.c_str());
        return false;
    }

    if (!parcel.WriteString(errorObject.message)) {
        HILOG_ERROR("Message [%{public}s] write string failed.", errorObject.message.c_str());
        return false;
    }
    
    if (!parcel.WriteString(errorObject.stack)) {
        HILOG_ERROR("Stack [%{public}s] write string failed.", errorObject.stack.c_str());
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        HILOG_ERROR("FaultType [%{public}d] write int32 failed.", static_cast<int32_t>(faultType));
        return false;
    }

    if (!parcel.WriteInt32(pid)) {
        HILOG_ERROR("Pid [%{public}d] write int32 failed.", static_cast<int32_t>(pid));
        return false;
    }

    if (!parcel.WriteString(timeoutMarkers)) {
        HILOG_ERROR("TimeoutMarkers [%{public}s] write string failed.", timeoutMarkers.c_str());
        return false;
    }

    if (!parcel.WriteBool(waitSaveState)) {
        HILOG_ERROR("WaitSaveState [%{public}s] write bool failed.", waitSaveState ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(notifyApp)) {
        HILOG_ERROR("NotifyApp [%{public}s] write bool failed.", notifyApp ? "true" : "false");
        return false;
    }

    if (!parcel.WriteBool(forceExit)) {
        HILOG_ERROR("ForceExit [%{public}s] write bool failed.", forceExit ? "true" : "false");
        return false;
    }

    if (!parcel.WriteUint32(state)) {
        HILOG_ERROR("State [%{public}u] write uint32 failed.", state);
        return false;
    }

    if (token == nullptr) {
        if (!parcel.WriteBool(false)) {
            HILOG_ERROR("Token falge [false] write bool failed.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(true) || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(token)) {
            HILOG_ERROR("Token falge [true] write bool failed.");
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS