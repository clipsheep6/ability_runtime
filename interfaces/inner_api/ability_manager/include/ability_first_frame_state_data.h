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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_DATA_H
#define OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_DATA_H
#ifdef SUPPORT_GRAPHICS

#include <sys/types.h>

#include "parcel.h"
#include "app_mgr_constants.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
struct AbilityFirstFrameStateData : public Parcelable {
    /**
     * @brief read this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     * @return Returns true if read successed; returns false otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);

    /**
     * @brief Marshals this Sequenceable object into a Parcel.
     *
     * @param outParcel Indicates the Parcel object to which the Sequenceable object will be marshaled.
     */
    virtual bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshals this Sequenceable object from a Parcel.
     *
     * @param inParcel Indicates the Parcel object into which the Sequenceable object has been marshaled.
     */
    static AbilityFirstFrameStateData *Unmarshalling(Parcel &parcel);

    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    int32_t appIndex;
    bool coldStart = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // SUPPORT_GRAPHICS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_DATA_H
