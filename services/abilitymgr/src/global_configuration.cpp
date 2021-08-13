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

#include "global_configuration.h"

#include "string_ex.h"

namespace OHOS {
namespace AAFwk {

GlobalConfiguration::GlobalConfiguration(const std::string &name) : testInfostr_(name)
{}

bool GlobalConfiguration::ReadFromParcel(Parcel &parcel)
{
    testInfostr_ = Str16ToStr8(parcel.ReadString16());
    return true;
}

GlobalConfiguration *GlobalConfiguration::Unmarshalling(Parcel &parcel)
{
    GlobalConfiguration *globalConfiguration = new (std::nothrow) GlobalConfiguration();
    if (globalConfiguration && !globalConfiguration->ReadFromParcel(parcel)) {
        delete globalConfiguration;
        globalConfiguration = nullptr;
    }
    return globalConfiguration;
}

bool GlobalConfiguration::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(testInfostr_))) {
        return false;
    }
    return true;
}

}  // namespace AAFwk
}  // namespace OHOS