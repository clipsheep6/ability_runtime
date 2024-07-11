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

#include "window_config.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AAFwk {

WindowConfig *WindowConfig::Unmarshalling(Parcel &parcel)
{
    WindowConfig *data = new (std::nothrow) WindowConfig();
    if (!data) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data is nullptr.");
        return nullptr;
    }
    data->windowType = parcel.ReadInt32();
    data->posx = parcel.ReadInt32();
    data->posy = parcel.ReadInt32();
    data->width = parcel.ReadUint32();
    data->height = parcel.ReadUint32();
    return data;
}

bool WindowConfig::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(windowType));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(posx));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(posy));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, static_cast<uint32_t>(width));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Uint32, parcel, static_cast<uint32_t>(height));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS