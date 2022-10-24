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

#ifndef ABILITY_RUNTIME_CONNECTION_DATA_H
#define ABILITY_RUNTIME_CONNECTION_DATA_H

#include "extension_ability_info.h"
#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {
struct ConnectionData : public Parcelable {
    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param parcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param parcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param parcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static ConnectionData *Unmarshalling(Parcel &parcel);

    int32_t extensionPid = -1;
    int32_t extensionUid = 0;
    std::string extensionBundleName;
    std::string extensionModuleName;
    std::string extensionName;
    AppExecFwk::ExtensionAbilityType extensionType;
    int32_t callerUid;
    int32_t callerPid;
    std::string callerName;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_CONNECTION_DATA_H
