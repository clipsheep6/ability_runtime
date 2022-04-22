/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_ABILITY_STATE_DATA_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_ABILITY_STATE_DATA_H

#include <sys/types.h>

#include "parcel.h"

#include "app_mgr_constants.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
struct AbilityStateData : public Parcelable {
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
    static AbilityStateData *Unmarshalling(Parcel &parcel);

    std::string moduleName;
    std::string bundleName;
    std::string abilityName;
    int32_t abilityState = 0;
    pid_t pid = 0;
    int32_t uid = 0;
    sptr<IRemoteObject> token;
    int32_t abilityType = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_ABILITY_STATE_DATA_H
