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

#include "page_state_data.h"

#include "hilog_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool PageStateData::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    abilityName = parcel.ReadString();
    pageName = parcel.ReadString();
    targetBundleName = parcel.ReadString();
    targetModuleName = parcel.ReadString();

    return true;
}

bool PageStateData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteString(bundleName) &&
        parcel.WriteString(moduleName) &&
        parcel.WriteString(abilityName) &&
        parcel.WriteString(pageName) &&
        parcel.WriteString(targetBundleName) &&
        parcel.WriteString(targetModuleName));
}

PageStateData *PageStateData::Unmarshalling(Parcel &parcel)
{
    PageStateData *pageStateData = new (std::nothrow) PageStateData();
    if (pageStateData && !pageStateData->ReadFromParcel(parcel)) {
        HILOG_WARN("failed, because ReadFromParcel failed");
        delete pageStateData;
        pageStateData = nullptr;
    }
    return pageStateData;
}
}
}