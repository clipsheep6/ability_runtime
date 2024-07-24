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

#include "ui_extension_session_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
UIExtensionSessionInfo *UIExtensionSessionInfo::Unmarshalling(Parcel &parcel)
{
    UIExtensionSessionInfo *info = new (std::nothrow) UIExtensionSessionInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create ui extension session info failed.");
        return nullptr;
    }
    info->persistentId = parcel.ReadInt32();
    info->hostWindowId = parcel.ReadUint32();
    info->uiExtensionUsage = static_cast<AAFwk::UIExtensionUsage>(parcel.ReadUint32());
    return info;
}

bool UIExtensionSessionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(persistentId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write persistent id failed.");
        return false;
    }

    if (!parcel.WriteUint32(hostWindowId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write host window id failed.");
        return false;
    }

    if (!parcel.WriteUint32(static_cast<uint32_t>(uiExtensionUsage))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write uiExtensionUsage failed.");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
