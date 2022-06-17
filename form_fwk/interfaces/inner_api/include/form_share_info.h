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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_SHARE_INFO_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_SHARE_INFO_H

#include <string>
#include "parcel.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @struct FormShareInfo
 * Defines form share info.
 */
struct FormShareInfo : public Parcelable {
    int64_t formId;
    std::string formName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    bool formTempFlag = false;
    int dimensionId;
    OHOS::AAFwk::WantParams providerData;
    std::string deviceId;
    std::string deviceName;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static FormShareInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_SHARE_INFO_H