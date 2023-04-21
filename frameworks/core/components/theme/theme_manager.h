/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_THEME_THEME_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_THEME_THEME_MANAGER_H

#include "base/memory/ace_type.h"
#include "base/resource/asset_manager.h"
#include "core/components/theme/theme.h"
#include "core/components/theme/theme_constants.h"

namespace OHOS::Ace {
class ACE_EXPORT ThemeManager : public AceType {
    DECLARE_ACE_TYPE(ThemeManager, AceType);

public:
    ThemeManager() = default;
    virtual ~ThemeManager() = default;

    virtual void InitResource(const ResourceInfo& resourceInfo) {}

    virtual void ReloadResource() {}

    virtual void UpdateConfig(const ResourceConfiguration& config) {}

    virtual void LoadSystemTheme(int32_t themeId) {}

    virtual void ParseSystemTheme() {}

    virtual void LoadCustomTheme(const RefPtr<AssetManager>& assetManager) {}

    virtual void SetColorScheme(ColorScheme colorScheme) {}

    virtual Color GetBackgroundColor() const = 0;

    virtual RefPtr<ThemeConstants> GetThemeConstants(
        const std::string& bundleName, const std::string& moduleName) const = 0;

    virtual RefPtr<ThemeConstants> GetThemeConstants() const = 0;

    virtual RefPtr<Theme> GetTheme(ThemeType type) = 0;

    virtual void LoadResourceThemes() {}

    template<typename T>
    RefPtr<T> GetTheme()
    {
        return AceType::DynamicCast<T>(GetTheme(T::TypeId()));
    }
};
} // namespace OHOS::Ace
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_THEME_THEME_MANAGER_H
