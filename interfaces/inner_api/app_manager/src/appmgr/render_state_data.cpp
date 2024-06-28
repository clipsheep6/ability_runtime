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

#include "render_state_data.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool RenderStateData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteInt32(pid) && parcel.WriteInt32(uid) && parcel.WriteInt32(hostPid) &&
        parcel.WriteInt32(hostUid) && parcel.WriteInt32(state));
}

bool RenderStateData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "pid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(hostPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "hostPid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(hostUid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "hostUid read failed.");
        return false;
    }
    if (!parcel.ReadInt32(state)) {
        TAG_LOGE(AAFwkTag::APPMGR, "state read failed.");
        return false;
    }
    return true;
};

RenderStateData *RenderStateData::Unmarshalling(Parcel &parcel)
{
    RenderStateData *renderStateData = new (std::nothrow) RenderStateData();
    if (renderStateData && !renderStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "renderStateData failed, because ReadFromParcel failed");
        delete renderStateData;
        renderStateData = nullptr;
    }
    return renderStateData;
}
}
}