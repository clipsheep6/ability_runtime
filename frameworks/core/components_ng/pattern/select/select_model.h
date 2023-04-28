/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SELECT_SELECT_MODEL_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SELECT_SELECT_MODEL_H

#include <optional>
#include <string_view>

#include "base/utils/macros.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/pattern/select/select_event_hub.h"
#include "core/components_ng/pattern/text/text_styles.h"

namespace OHOS::Ace {
enum class ArrowPosition {
    END = 0,
    START,
};

enum class MenuAlignType {
    START = 0,
    CENTER,
    END,
};

struct MenuAlign {
    MenuAlignType alignType = MenuAlignType::START;
    DimensionOffset offset = DimensionOffset(Dimension(0, DimensionUnit::VP), Dimension(0, DimensionUnit::VP));
};

using SelectParam = std::pair<std::string, std::string>;

class SelectModel {
public:
    static SelectModel* GetInstance();
    virtual ~SelectModel() = default;

    virtual void Create(const std::vector<SelectParam>& params);
    virtual void SetSelected(int32_t idx);
    virtual void SetValue(const std::string& value);
    virtual void SetFontSize(const Dimension& value);
    virtual void SetFontWeight(const FontWeight& value);
    virtual void SetFontFamily(const std::vector<std::string>& value);
    virtual void SetItalicFontStyle(const Ace::FontStyle& value);
    virtual void SetFontColor(const Color& color);
    virtual void SetSelectedOptionBgColor(const Color& color);
    virtual void SetSelectedOptionFontSize(const Dimension& value);
    virtual void SetSelectedOptionFontWeight(const FontWeight& value);
    virtual void SetSelectedOptionFontFamily(const std::vector<std::string>& value);
    virtual void SetSelectedOptionItalicFontStyle(const Ace::FontStyle& value);
    virtual void SetSelectedOptionFontColor(const Color& color);
    virtual void SetOptionBgColor(const Color& color);
    virtual void SetOptionFontSize(const Dimension& value);
    virtual void SetOptionFontWeight(const FontWeight& value);
    virtual void SetOptionFontFamily(const std::vector<std::string>& value);
    virtual void SetOptionItalicFontStyle(const Ace::FontStyle& value);
    virtual void SetOptionFontColor(const Color& color);
    virtual void SetOnSelect(NG::SelectEvent&& onSelect);
    virtual void SetWidth(Dimension& value);
    virtual void SetHeight(Dimension& value);
    virtual void SetSize(Dimension& width, Dimension& height);
    virtual void SetPaddings(const std::optional<Dimension>& top, const std::optional<Dimension>& bottom,
        const std::optional<Dimension>& left, const std::optional<Dimension>& right);
    virtual void SetPadding(const Dimension& value);
    virtual void SetPaddingLeft(const Dimension& value);
    virtual void SetPaddingTop(const Dimension& value);
    virtual void SetPaddingRight(const Dimension& value);
    virtual void SetPaddingBottom(const Dimension& value);
    virtual void SetSpace(const Dimension& value);
    virtual void SetArrowPosition(const ArrowPosition value);
    virtual void SetMenuAlign(const MenuAlign& menuAlign);

private:
    static std::unique_ptr<SelectModel> instance_;
    static std::mutex mutex_;
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SELECT_SELECT_MODEL_H
