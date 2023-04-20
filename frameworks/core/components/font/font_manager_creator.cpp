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

#include "core/common/font_manager.h"

#ifndef NG_BUILD
#ifndef NEW_SKIA
#include "core/components/font/flutter_font_manager.h"
#endif
#endif
#ifdef ENABLE_ROSEN_BACKEND
#include "core/components/font/rosen_font_manager.h"
#endif

namespace OHOS::Ace {

RefPtr<FontManager> FontManager::Create()
{
    if (SystemProperties::GetRosenBackendEnabled()) {
#ifdef ENABLE_ROSEN_BACKEND
        return AceType::MakeRefPtr<RosenFontManager>();
#else
        return nullptr;
#endif
    } else {
#ifdef NG_BUILD
        // TODO: adapt to flutter fontmanager for ng
        return nullptr;
#else
#ifndef NEW_SKIA
        return AceType::MakeRefPtr<FlutterFontManager>();
#else
        return nullptr;
#endif
#endif
    }
}

} // namespace OHOS::Ace
