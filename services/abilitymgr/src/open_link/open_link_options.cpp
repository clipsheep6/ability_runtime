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

#include "open_link_options.h"

namespace OHOS {
namespace AAFwk {
OpenLinkOptions::OpenLinkOptions(const OpenLinkOptions &other)
{
    appLinkingOnly_ = other.appLinkingOnly_;
    parameters_ = other.parameters_;
}

OpenLinkOptions &OpenLinkOptions::operator=(const OpenLinkOptions &other)
{
    if (this != &other) {
        appLinkingOnly_ = other.appLinkingOnly_;
        parameters_ = other.parameters_;
    }
    return *this;
}

bool OpenLinkOptions::ReadParameters(Parcel &parcel)
{
    int empty = VALUE_NULL;
    if (!parcel.ReadInt32(empty)) {
        return false;
    }

    if (empty == VALUE_OBJECT) {
        auto params = parcel.ReadParcelable<WantParams>();
        if (params != nullptr) {
            SetParameters(*params);
            delete params;
            params = nullptr;
        } else {
            return false;
        }
    }

    return true;
}

bool OpenLinkOptions::ReadFromParcel(Parcel &parcel)
{
    bool appLinkingOnly;
    if (!parcel.ReadBool(appLinkingOnly)) {
        return false;
    }
    SetAppLinkingOnly(appLinkingOnly);

    if (!ReadParameters(parcel)) {
        return false;
    }

    return true;
}

OpenLinkOptions *OpenLinkOptions::Unmarshalling(Parcel &parcel)
{
    OpenLinkOptions *option = new (std::nothrow) OpenLinkOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool OpenLinkOptions::WriteParameters(const WantParams &parameters, Parcel &parcel) const
{
    if (parameters.Size() == 0) {
        if (!parcel.WriteInt32(VALUE_NULL)) {
            return false;
        }
    } else {
        if (!parcel.WriteInt32(VALUE_OBJECT)) {
            return false;
        }
        if (!parcel.WriteParcelable(&parameters)) {
            return false;
        }
    }

    return true;
}

bool OpenLinkOptions::Marshalling(Parcel &parcel) const
{
    // write GetAppLinkingOnly
    if (!parcel.WriteBool(GetAppLinkingOnly())) {
        return false;
    }
    // write parameters
    if (!WriteParameters(GetParameters(), parcel)) {
        return false;
    }

    return true;
}

void OpenLinkOptions::SetAppLinkingOnly(bool appLinkingOnly)
{
    appLinkingOnly_ = appLinkingOnly;
}

bool OpenLinkOptions::GetAppLinkingOnly() const
{
    return appLinkingOnly_;
}

void OpenLinkOptions::SetParameters(WantParams parameters)
{
    parameters_ = parameters;
}

WantParams OpenLinkOptions::GetParameters() const
{
    return parameters_;
}
}  // namespace AAFwk
}  // namespace OHOS
