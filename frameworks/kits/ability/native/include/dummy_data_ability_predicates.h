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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DATAABILITYPREDICATES_H
#define FOUNDATION_APPEXECFWK_OHOS_DATAABILITYPREDICATES_H

#include <string>
#include <unistd.h>

#include "nocopyable.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
class DataAbilityPredicates : public Parcelable {
public:
    DataAbilityPredicates() = default;
    DataAbilityPredicates(const std::string &testInf);
    ~DataAbilityPredicates() = default;
    DataAbilityPredicates(const Parcel &parcel) {};

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static DataAbilityPredicates *Unmarshalling(Parcel &parcel);

private:
    std::string testInf_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DATAABILITYPREDICATES_H