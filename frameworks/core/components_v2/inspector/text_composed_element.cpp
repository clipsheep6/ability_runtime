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

#include "core/components_v2/inspector/text_composed_element.h"

#include "base/log/dump_log.h"
#include "core/components/common/layout/constants.h"
#include "core/components/text/text_element.h"
#include "core/components_v2/inspector/utils.h"

namespace OHOS::Ace::V2 {

namespace {

const std::unordered_map<std::string, std::function<std::string(const TextComposedElement&)>> CREATE_JSON_MAP {
    { "content", [](const TextComposedElement& inspector) { return inspector.GetData(); } },
    { "textAlign", [](const TextComposedElement& inspector) { return inspector.GetTextAlign(); } },
    { "textOverflow", [](const TextComposedElement& inspector) { return inspector.GetTextOverflow(); } },
    { "maxLines", [](const TextComposedElement& inspector) { return inspector.GetMaxLines(); } },
    { "lineHeight", [](const TextComposedElement& inspector) { return inspector.GetLineHeight(); } },
    { "decoration", [](const TextComposedElement& inspector) { return inspector.GetTextDecoration(); } },
    { "baselineOffset", [](const TextComposedElement& inspector) { return inspector.GetBaseLineOffset(); } },
    { "textCase", [](const TextComposedElement& inspector) { return inspector.GetTextCase(); } },
    { "fontColor", [](const TextComposedElement& inspector) { return inspector.GetTextFontColor(); } },
    { "fontSize", [](const TextComposedElement& inspector) { return inspector.GetTextFontSize(); } },
    { "fontStyle", [](const TextComposedElement& inspector) { return inspector.GetTextFontStyle(); } },
    { "fontWeight", [](const TextComposedElement& inspector) { return inspector.GetTextFontWeight(); } },

};

} // namespace

void TextComposedElement::Dump()
{
    InspectorComposedElement::Dump();
    DumpLog::GetInstance().AddDesc(std::string("content: ").append(GetData()));
    DumpLog::GetInstance().AddDesc(std::string("textAlign: ").append(GetTextAlign()));
    DumpLog::GetInstance().AddDesc(std::string("textOverflow: ").append(GetTextOverflow()));
    DumpLog::GetInstance().AddDesc(std::string("maxLines: ").append(GetMaxLines()));
    DumpLog::GetInstance().AddDesc(std::string("lineHeight: ").append(GetLineHeight()));
    DumpLog::GetInstance().AddDesc(std::string("decoration: ").append(GetTextDecoration()));
    DumpLog::GetInstance().AddDesc(std::string("baselineOffset: ").append(GetBaseLineOffset()));
    DumpLog::GetInstance().AddDesc(std::string("textCase: ").append(GetTextCase()));
    DumpLog::GetInstance().AddDesc(std::string("fontColor: ").append(GetTextFontColor()));
    DumpLog::GetInstance().AddDesc(std::string("fontSize: ").append(GetTextFontSize()));
    DumpLog::GetInstance().AddDesc(std::string("fontStyle: ").append(GetTextFontStyle()));
    DumpLog::GetInstance().AddDesc(std::string("fontWeight: ").append(GetTextFontWeight()));
}

std::unique_ptr<JsonValue> TextComposedElement::ToJsonObject() const
{
    auto resultJson = InspectorComposedElement::ToJsonObject();
    for (const auto& value : CREATE_JSON_MAP) {
        resultJson->Put(value.first.c_str(), value.second(*this).c_str());
    }
    return resultJson;
}

double TextComposedElement::GetWidth() const
{
    double width = InspectorComposedElement::GetWidth();
    if (width != -1.0) {
        return width;
    }

    auto renderText = GetRenderText();
    if (renderText) {
        return renderText->GetPaintRect().Width();
    }
    return -1.0;
}

double TextComposedElement::GetHeight() const
{
    double height = InspectorComposedElement::GetHeight();
    if (height != -1.0) {
        return height;
    }

    auto renderText = GetRenderText();
    if (renderText) {
        return renderText->GetPaintRect().Height();
    }
    return -1.0;
}

std::string TextComposedElement::GetTextAlign() const
{
    auto renderText = GetRenderText();
    auto textAlign =
        renderText ? renderText->GetTextStyle().GetTextAlign() : TextAlign::START;
    return ConvertWrapTextAlignToString(textAlign);
}

std::string TextComposedElement::GetTextOverflow() const
{
    auto renderText = GetRenderText();
    auto textOverflow =
        renderText ? renderText->GetTextStyle().GetTextOverflow() : TextOverflow::CLIP;
    return ConvertWrapTextOverflowToString(textOverflow);
}

std::string TextComposedElement::GetMaxLines() const
{
    auto renderText = GetRenderText();
    auto maxLines =
        renderText ? renderText->GetTextStyle().GetMaxLines() : INT32_MAX;
    return std::to_string(static_cast<int32_t>(maxLines));
}

std::string TextComposedElement::GetLineHeight() const
{
    auto renderText = GetRenderText();
    auto lineHeight =
        renderText ? renderText->GetTextStyle().GetLineHeight() : Dimension();
    return std::to_string(static_cast<int32_t>(lineHeight.Value()));
}

std::string TextComposedElement::GetTextDecoration() const
{
    auto renderText = GetRenderText();
    auto textDecoration =
        renderText ? renderText->GetTextStyle().GetTextDecoration() : TextDecoration::NONE;
    return ConvertWrapTextDecorationToStirng(textDecoration);
}

std::string TextComposedElement::GetBaseLineOffset() const
{
    auto renderText = GetRenderText();
    auto baseLineOffset =
        renderText ? renderText->GetTextStyle().GetBaselineOffset() : Dimension();
    return std::to_string(static_cast<int32_t>(baseLineOffset.Value()));
}

std::string TextComposedElement::GetTextCase() const
{
    auto renderText = GetRenderText();
    auto textCase =
        renderText ? renderText->GetTextStyle().GetTextCase() : TextCase::NORMAL;
    return ConvertWrapTextCaseToStirng(textCase);
}

std::string TextComposedElement::GetTextFontColor() const
{
    auto renderText = GetRenderText();
    auto fontColor =
        renderText ? renderText->GetTextStyle().GetTextColor() : Color::BLACK;
    return std::to_string(fontColor.GetValue());
}

std::string TextComposedElement::GetTextFontSize() const
{
    auto renderText = GetRenderText();
    auto fontSize =
        renderText ? renderText->GetTextStyle().GetFontSize() : Dimension();
    return std::to_string(static_cast<int32_t>(fontSize.Value()));
}

std::string TextComposedElement::GetTextFontStyle() const
{
    auto renderText = GetRenderText();
    auto fontStyle =
        renderText ? renderText->GetTextStyle().GetFontStyle() : FontStyle::NORMAL;
    return ConvertWrapFontStyleToStirng(fontStyle);
}

std::string TextComposedElement::GetTextFontWeight() const
{
    auto renderText = GetRenderText();
    auto fontWeight =
        renderText ? renderText->GetTextStyle().GetFontWeight() : FontWeight::NORMAL;
    return ConvertWrapFontWeightToStirng(fontWeight);
}

std::string TextComposedElement::GetData() const
{
    auto renderText = GetRenderText();
    return renderText ? "" : renderText->GetTextData();
}

RefPtr<RenderText> TextComposedElement::GetRenderText() const
{
    auto node = GetInspectorNode(TextElement::TypeId());
    if (node) {
        return AceType::DynamicCast<RenderText>(node);
    }
    return nullptr;
}

} // namespace OHOS::Ace::V2
