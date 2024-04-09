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

#ifndef OHOS_ABILITY_RUNTIME_OPEN_LINK_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_OPEN_LINK_OPTIONS_H

#include <string>

#include "ability_window_configuration.h"
#include "parcel.h"
#include "want.h"
namespace OHOS {
namespace AAFwk {
class OpenLinkOptions final : public Parcelable, public std::enable_shared_from_this<OpenLinkOptions> {
public:
    OpenLinkOptions() = default;
    virtual ~OpenLinkOptions() = default;
    OpenLinkOptions(const OpenLinkOptions &other);
    OpenLinkOptions &operator=(const OpenLinkOptions &other);

    virtual bool Marshalling(Parcel &parcel) const override;
    static OpenLinkOptions *Unmarshalling(Parcel &parcel);

    void SetAppLinkingOnly(bool appLinkingOnly);
    bool GetAppLinkingOnly() const;
    void SetParameters(WantParams parameters);
    WantParams GetParameters() const;
    bool WriteParameters(const WantParams &parameters, Parcel &parcel) const;

private:
    bool appLinkingOnly_ = false;
    WantParams parameters_;
    // no object in parcel
    static constexpr int VALUE_NULL = -1;
    // object exist in parcel
    static constexpr int VALUE_OBJECT = 1;

private:
    bool ReadParameters(Parcel &parcel);
    bool ReadFromParcel(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_OPEN_LINK_OPTIONS_H
