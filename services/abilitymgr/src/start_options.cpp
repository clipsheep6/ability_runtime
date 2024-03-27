/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "start_options.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "process_options.h"

namespace OHOS {
namespace AAFwk {
StartOptions::StartOptions(const StartOptions &other)
{
    windowMode_ = other.windowMode_;
    displayId_ = other.displayId_;
    withAnimation_ = other.withAnimation_;
    windowLeft_ = other.windowLeft_;
    windowTop_ = other.windowTop_;
    windowWidth_ = other.windowWidth_;
    windowHeight_ = other.windowHeight_;
    windowLeftUsed_ = other.windowLeftUsed_;
    windowTopUsed_ = other.windowTopUsed_;
    windowWidthUsed_ = other.windowWidthUsed_;
    windowHeightUsed_ = other.windowHeightUsed_;
    processOptions = other.processOptions;
}

StartOptions &StartOptions::operator=(const StartOptions &other)
{
    if (this != &other) {
        windowMode_ = other.windowMode_;
        displayId_ = other.displayId_;
        withAnimation_ = other.withAnimation_;
        windowLeft_ = other.windowLeft_;
        windowTop_ = other.windowTop_;
        windowWidth_ = other.windowWidth_;
        windowHeight_ = other.windowHeight_;
        windowLeftUsed_ = other.windowLeftUsed_;
        windowTopUsed_ = other.windowTopUsed_;
        windowWidthUsed_ = other.windowWidthUsed_;
        windowHeightUsed_ = other.windowHeightUsed_;
        processOptions = other.processOptions;
    }
    return *this;
}

bool StartOptions::ReadFromParcel(Parcel &parcel)
{
    SetWindowMode(parcel.ReadInt32());
    SetDisplayID(parcel.ReadInt32());
    SetWithAnimation(parcel.ReadBool());
    SetWindowLeft(parcel.ReadInt32());
    SetWindowTop(parcel.ReadInt32());
    SetWindowWidth(parcel.ReadInt32());
    SetWindowHeight(parcel.ReadInt32());
    windowLeftUsed_ = parcel.ReadBool();
    windowTopUsed_ = parcel.ReadBool();
    windowWidthUsed_ = parcel.ReadBool();
    windowHeightUsed_ = parcel.ReadBool();
    processOptions.reset(parcel.ReadParcelable<ProcessOptions>());
    return true;
}

StartOptions *StartOptions::Unmarshalling(Parcel &parcel)
{
    StartOptions *option = new (std::nothrow) StartOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool StartOptions::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(GetWindowMode());
    parcel.WriteInt32(GetDisplayID());
    parcel.WriteBool(GetWithAnimation());
    parcel.WriteInt32(GetWindowLeft());
    parcel.WriteInt32(GetWindowTop());
    parcel.WriteInt32(GetWindowWidth());
    parcel.WriteInt32(GetWindowHeight());
    parcel.WriteBool(windowLeftUsed_);
    parcel.WriteBool(windowTopUsed_);
    parcel.WriteBool(windowWidthUsed_);
    parcel.WriteBool(windowHeightUsed_);
    if (!parcel.WriteParcelable(processOptions.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write processOptions failed.");
        return false;
    }
    return true;
}

void StartOptions::SetWindowMode(int32_t windowMode)
{
    windowMode_ = windowMode;
}

int32_t StartOptions::GetWindowMode() const
{
    return windowMode_;
}

void StartOptions::SetDisplayID(int32_t id)
{
    displayId_ = id;
}

int32_t StartOptions::GetDisplayID() const
{
    return displayId_;
}

void StartOptions::SetWithAnimation(bool withAnimation)
{
    withAnimation_ = withAnimation;
}

bool StartOptions::GetWithAnimation() const
{
    return withAnimation_;
}

void StartOptions::SetWindowLeft(int32_t windowLeft)
{
    windowLeft_ = windowLeft;
}

int32_t StartOptions::GetWindowLeft() const
{
    return windowLeft_;
}

void StartOptions::SetWindowTop(int32_t windowTop)
{
    windowTop_ = windowTop;
}

int32_t StartOptions::GetWindowTop() const
{
    return windowTop_;
}

void StartOptions::SetWindowWidth(int32_t windowWidth)
{
    windowWidth_ = windowWidth;
}

int32_t StartOptions::GetWindowWidth() const
{
    return windowWidth_;
}

void StartOptions::SetWindowHeight(int32_t windowHeight)
{
    windowHeight_ = windowHeight;
}

int32_t StartOptions::GetWindowHeight() const
{
    return windowHeight_;
}
}  // namespace AAFwk
}  // namespace OHOS
