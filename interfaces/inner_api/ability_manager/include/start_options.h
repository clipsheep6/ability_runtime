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

#ifndef OHOS_ABILITY_RUNTIME_START_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_START_OPTIONS_H

#include <string>

#include "ability_window_configuration.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
class ProcessOptions;

class StartOptions final : public Parcelable, public std::enable_shared_from_this<StartOptions> {
public:
    const int32_t DEFAULT_DISPLAY_ID {0};
    bool windowLeftUsed_ = false;
    bool windowTopUsed_ = false;
    bool windowWidthUsed_ = false;
    bool windowHeightUsed_ = false;
    std::shared_ptr<ProcessOptions> processOptions = nullptr;

    StartOptions() = default;
    ~StartOptions() = default;
    StartOptions(const StartOptions &other);
    StartOptions &operator=(const StartOptions &other);

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static StartOptions *Unmarshalling(Parcel &parcel);

    void SetWindowMode(int32_t windowMode);
    int32_t GetWindowMode() const;

    void SetDisplayID(int32_t displayId);
    int32_t GetDisplayID() const;

    void SetWithAnimation(bool withAnimation);
    bool GetWithAnimation() const;

    void SetWindowLeft(int32_t windowLeft);
    int32_t GetWindowLeft() const;

    void SetWindowTop(int32_t windowTop);
    int32_t GetWindowTop() const;

    void SetWindowWidth(int32_t windowWidth);
    int32_t GetWindowWidth() const;

    void SetWindowHeight(int32_t windowHeight);
    int32_t GetWindowHeight() const;
private:
    int32_t windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    int32_t displayId_ = DEFAULT_DISPLAY_ID;
    bool withAnimation_ = true;
    int32_t windowLeft_ = 0;
    int32_t windowTop_ = 0;
    int32_t windowWidth_ = 0;
    int32_t windowHeight_ = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_START_OPTIONS_H
