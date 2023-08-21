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

#include "core/components/text/flutter_render_text.h"

#include <cmath>

#ifndef USE_GRAPHIC_TEXT_GINE
#include "txt/paragraph_builder.h"
#include "txt/paragraph_txt.h"
#else
#include "rosen_text/typography_create.h"
#include "rosen_text/typography.h"
#endif
#include "unicode/uchar.h"

#include "base/geometry/dimension.h"
#include "base/i18n/localization.h"
#include "base/utils/string_utils.h"
#include "base/utils/utils.h"
#include "core/common/font_manager.h"
#include "core/components/calendar/flutter_render_calendar.h"
#include "core/components/font/constants_converter.h"
#include "core/components/font/flutter_font_collection.h"
#include "core/components/text_span/flutter_render_text_span.h"
#include "core/pipeline/base/flutter_render_context.h"
#include "core/pipeline/base/scoped_canvas_state.h"
#include "core/pipeline/pipeline_context.h"

namespace OHOS::Ace {
namespace {

const std::u16string ELLIPSIS = u"\u2026";
constexpr Dimension ADAPT_UNIT = 1.0_fp;
constexpr int32_t COMPATIBLE_VERSION = 6;

} // namespace

double FlutterRenderText::GetBaselineDistance(TextBaseline textBaseline)
{
    if (textBaseline == TextBaseline::IDEOGRAPHIC) {
        return paragraph_->GetIdeographicBaseline() + std::max(NormalizeToPx(textStyle_.GetBaselineOffset()), 0.0);
    }
    return paragraph_->GetAlphabeticBaseline() + std::max(NormalizeToPx(textStyle_.GetBaselineOffset()), 0.0);
}

void FlutterRenderText::Paint(RenderContext& context, const Offset& offset)
{
    if (!NeedPaint()) {
        return;
    }

    if (needMeasure_) {
        LOGW("Text can not paint before measure.");
        return;
    }
    auto canvas = ScopedCanvas::Create(context);
    if (!canvas || !paragraph_) {
        LOGE("Paint canvas or paragraph is null");
        return;
    }

    RenderNode::Paint(context, offset);

    if (textStyle_.GetTextOverflow() == TextOverflow::CLIP) {
        const auto& clipRect = Rect(offset, GetLayoutSize());
        canvas->clipRect(clipRect.Left(), clipRect.Top(), clipRect.Right(), clipRect.Bottom(), SkClipOp::kIntersect);
    }
    auto baselineOffset = std::min(NormalizeToPx(textStyle_.GetBaselineOffset()), 0.0);
    auto paintOffset = offset - Offset(0.0, baselineOffset);
    auto textRealWidth = paragraph_->GetMaxWidth();
    auto textRealHeight = paragraph_->GetHeight();
    SetParagraphWidth(textRealWidth);
    SetParagraphHeight(textRealHeight);
    float newX = paintOffset.GetX();
    float newY = paintOffset.GetY();

    if (text_->GetDeclarationHeight().IsValid()) {
        switch (textStyle_.GetTextVerticalAlign()) {
            case VerticalAlign::TOP:
                break;
            case VerticalAlign::BOTTOM:
                newY = offset.GetY() + (GetLayoutSize().Height() - textRealHeight) -
                       std::max(NormalizeToPx(textStyle_.GetBaselineOffset()), 0.0);
                break;
            case VerticalAlign::CENTER:
                newY = offset.GetY() - NormalizeToPx(textStyle_.GetBaselineOffset()) +
                       (GetLayoutSize().Height() - textRealHeight) / 2.0;
                break;
            default:
                break;
        }
    }

    switch (textStyle_.GetTextAlign()) {
        case TextAlign::LEFT:
            break;
        case TextAlign::START:
            if (TextDirection::RTL == defaultTextDirection_) {
                newX = paintOffset.GetX() + (GetLayoutSize().Width() - textRealWidth);
            }
            break;
        case TextAlign::RIGHT:
            newX = paintOffset.GetX() + (GetLayoutSize().Width() - textRealWidth);
            break;
        case TextAlign::END:
            if (TextDirection::RTL != defaultTextDirection_) {
                newX = paintOffset.GetX() + (GetLayoutSize().Width() - textRealWidth);
            }
            break;
        case TextAlign::CENTER:
            newX = paintOffset.GetX() + (GetLayoutSize().Width() - textRealWidth) / 2.0;
            break;
        case TextAlign::JUSTIFY:
            break;
        default:
            break;
    }
    paragraph_->Paint(canvas->canvas(), newX, newY);
}

bool FlutterRenderText::NeedPaint()
{
    // If font is custom font, paint text until font is ready.
    auto pipelineContext = context_.Upgrade();
    if (pipelineContext && pipelineContext->GetFontManager()) {
        auto fontNames = pipelineContext->GetFontManager()->GetFontNames();
        for (const auto& familyName : textStyle_.GetFontFamilies()) {
            if (std::find(std::begin(fontNames), std::end(fontNames), familyName) != std::end(fontNames) &&
                !isCallbackCalled_) {
                return false;
            }
        }
        for (const auto& child : GetChildren()) {
            auto span = AceType::DynamicCast<RenderTextSpan>(child);
            if (!span) {
                continue;
            }
            for (const auto& familyName : span->GetSpanStyle().GetFontFamilies()) {
                if (std::find(std::begin(fontNames), std::end(fontNames), familyName) != std::end(fontNames) &&
                    !span->IsCallbackCalled()) {
                    return false;
                }
            }
        }
    }
    return true;
}

Size FlutterRenderText::Measure()
{
    if (!text_ || CheckMeasureFlag()) {
        return GetSize();
    }

    textStyle_.SetMaxLines(maxLines_);
    lastLayoutMaxWidth_ = GetLayoutParam().GetMaxSize().Width();
    lastLayoutMinWidth_ = GetLayoutParam().GetMinSize().Width();
    lastLayoutMaxHeight_ = GetLayoutParam().GetMaxSize().Height();
    lastLayoutMinHeight_ = GetLayoutParam().GetMinSize().Height();
    if (!textStyle_.GetAdaptTextSize()) {
        if (!UpdateParagraph()) {
            LOGE("fail to initialize text paragraph");
            return {};
        }
        paragraph_->Layout(lastLayoutMaxWidth_);
    } else {
        if (!AdaptTextSize(lastLayoutMaxWidth_)) {
            LOGE("fail to initialize text paragraph in adapt text size step");
            return {};
        }
    }
    needMeasure_ = false;
    // If you need to lay out the text according to the maximum layout width given by the parent, use it.
    if (text_->GetMaxWidthLayout()) {
        paragraphNewWidth_ = GetLayoutParam().GetMaxSize().Width();
        return GetSize();
    }
    // The reason for the second layout is because the TextAlign property needs the width of the layout,
    // and the width of the second layout is used as the width of the TextAlign layout.
    if (!NearEqual(lastLayoutMinWidth_, lastLayoutMaxWidth_)) {
        paragraphNewWidth_ = std::clamp(GetTextWidth(), lastLayoutMinWidth_, lastLayoutMaxWidth_);
        if (!NearEqual(paragraphNewWidth_, paragraph_->GetMaxWidth())) {
            ApplyIndents(paragraphNewWidth_);
            paragraph_->Layout(std::ceil(paragraphNewWidth_));
        }
    }
    EffectAutoMaxLines();
    return GetSize();
}

bool FlutterRenderText::IsCompatibleVersion()
{
    auto context = context_.Upgrade();
    if (!context) {
        return false;
    }
    return context->GetMinPlatformVersion() >= COMPATIBLE_VERSION;
}

bool FlutterRenderText::CheckMeasureFlag()
{
    if (isCallbackCalled_) {
        needMeasure_ = true;
    }
    for (const auto& child : GetChildren()) {
        auto span = AceType::DynamicCast<RenderTextSpan>(child);
        if (span && (span->IsCallbackCalled() || span->NeedLayout())) {
            paragraph_.reset();
            needMeasure_ = true;
            break;
        }
    }

    double paragraphMaxWidth = GetLayoutParam().GetMaxSize().Width();
    double paragraphMinWidth = GetLayoutParam().GetMinSize().Width();
    double paragraphMaxHeight = GetLayoutParam().GetMaxSize().Height();
    double paragraphMinHeight = GetLayoutParam().GetMinSize().Height();

    if (!needMeasure_) {
        // Layout when constrains of height is changed.
        if (!NearEqual(paragraphMaxHeight, lastLayoutMaxHeight_) ||
            !NearEqual(paragraphMinHeight, lastLayoutMinHeight_)) {
            return false;
        }
        bool constrainsAffect = true;
        auto layoutWidth = GetSize().Width();
        if (NearEqual(paragraphMaxWidth, lastLayoutMaxWidth_) && NearEqual(paragraphMinWidth, lastLayoutMinWidth_)) {
            // Width constrains not changed.
            constrainsAffect = false;
        } else if (!IsCompatibleVersion() && GreatOrEqual(layoutWidth, paragraphMinWidth) &&
                   LessOrEqual(layoutWidth, paragraphMaxWidth) && (lastLayoutMaxWidth_ - layoutWidth > 1.0)) {
            // Constrains changed but has no effect. For example, text width is 100 when constrains [0, 200].
            // When constrains changed to [100, 300], there's no need to do layout.
            // An exception is that given [0, 100], resulting in layout 100. We assume the actual layout size is more
            // than 100 due to soft-wrap.
            if (!textStyle_.GetAdaptTextSize()) {
                constrainsAffect = false;
            }
        }
        if (!constrainsAffect) {
            LOGD("Text content and constrains not affect, no need measure.");
            return true;
        }
    }
    return false;
}

void FlutterRenderText::EffectAutoMaxLines()
{
    // Effect when max-lines set auto.
    if (!paragraph_ || !text_ || !text_->GetAutoMaxLines()) {
        return;
    }

    auto lineCount = GetTextLines();
    while (lineCount > 0 && GreatNotEqual(paragraph_->GetHeight(), GetLayoutParam().GetMaxSize().Height())) {
        textStyle_.SetMaxLines(--lineCount);
        UpdateParagraphAndLayout(lastLayoutMaxWidth_);
    }
}

bool FlutterRenderText::AdaptTextSize(double paragraphMaxWidth)
{
    const auto& preferTextSizeGroups = textStyle_.GetPreferTextSizeGroups();
    if (!preferTextSizeGroups.empty()) {
        return AdaptPreferTextSizeGroup(paragraphMaxWidth);
    }
    const auto& preferFontSizes = textStyle_.GetPreferFontSizes();
    if (!preferFontSizes.empty()) {
        return AdaptPreferTextSize(paragraphMaxWidth);
    }
    return AdaptMinTextSize(paragraphMaxWidth);
}

bool FlutterRenderText::AdaptMinTextSize(double paragraphMaxWidth)
{
    double maxFontSize = NormalizeToPx(textStyle_.GetAdaptMaxFontSize());
    double minFontSize = NormalizeToPx(textStyle_.GetAdaptMinFontSize());
    if (LessNotEqual(maxFontSize, minFontSize) || LessOrEqual(textStyle_.GetAdaptMinFontSize().Value(), 0.0)) {
        if (!UpdateParagraph()) {
            LOGE("fail to initialize text paragraph when adapt min text size.");
            return false;
        }
        paragraph_->Layout(lastLayoutMaxWidth_);
        return true;
    }
    Dimension step = ADAPT_UNIT;
    if (GreatNotEqual(textStyle_.GetAdaptFontSizeStep().Value(), 0.0)) {
        step = textStyle_.GetAdaptFontSizeStep();
    }
    double stepSize = NormalizeToPx(step);
    while (GreatOrEqual(maxFontSize, minFontSize)) {
        textStyle_.SetFontSize(Dimension(maxFontSize));
        if (!UpdateParagraphAndLayout(paragraphMaxWidth)) {
            return false;
        }
        if (!DidExceedMaxLines(paragraphMaxWidth)) {
            break;
        }
        maxFontSize -= stepSize;
    }
    return true;
}

bool FlutterRenderText::AdaptPreferTextSize(double paragraphMaxWidth)
{
    // Use preferFontSizes to adapt lines.
    const auto& preferFontSizes = textStyle_.GetPreferFontSizes();
    for (const auto& fontSize : preferFontSizes) {
        textStyle_.SetFontSize(fontSize);
        if (!UpdateParagraphAndLayout(paragraphMaxWidth)) {
            return false;
        }
        if (!DidExceedMaxLines(paragraphMaxWidth)) {
            break;
        }
    }
    return true;
}

bool FlutterRenderText::AdaptPreferTextSizeGroup(double paragraphMaxWidth)
{
    // Use preferTextSizeGroup.
    const auto& preferTextSizeGroups = textStyle_.GetPreferTextSizeGroups();
    for (const auto& preferTextSizeGroup : preferTextSizeGroups) {
        textStyle_.SetFontSize(preferTextSizeGroup.fontSize);
        textStyle_.SetMaxLines(preferTextSizeGroup.maxLines);
        textStyle_.SetTextOverflow(preferTextSizeGroup.textOverflow);
        if (!UpdateParagraphAndLayout(paragraphMaxWidth)) {
            return false;
        }
        if ((preferTextSizeGroup.textOverflow == TextOverflow::NONE) || (!DidExceedMaxLines(paragraphMaxWidth))) {
            break;
        }
    }
    return true;
}

bool FlutterRenderText::UpdateParagraphAndLayout(double paragraphMaxWidth)
{
    if (!UpdateParagraph()) {
        return false;
    }
    if (paragraph_) {
        paragraph_->Layout(paragraphMaxWidth);
    }
    return true;
}

uint32_t FlutterRenderText::GetTextLines()
{
    uint32_t textLines = 0;
#ifndef USE_GRAPHIC_TEXT_GINE
    auto paragraphTxt = static_cast<txt::ParagraphTxt*>(paragraph_.get());
    if (paragraphTxt != nullptr) {
        textLines = paragraphTxt->GetLineCount();
#else
    if (paragraph_ != nullptr) {
        textLines = paragraph_->GetLineCount();
#endif
    }
    return textLines;
}

int32_t FlutterRenderText::GetTouchPosition(const Offset& offset)
{
    if (!paragraph_) {
        return 0;
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    return static_cast<int32_t>(paragraph_->GetGlyphPositionAtCoordinate(offset.GetX(), offset.GetY()).position);
#else
    return static_cast<int32_t>(paragraph_->GetGlyphIndexByCoordinate(offset.GetX(), offset.GetY()).index);
#endif
}

Size FlutterRenderText::GetSize()
{
    double height = paragraph_ ? paragraph_->GetHeight() : 0.0;
    double heightFinal = std::min(
        height + std::fabs(NormalizeToPx(textStyle_.GetBaselineOffset())), GetLayoutParam().GetMaxSize().Height());
    const auto& VerticalAlign = textStyle_.GetTextVerticalAlign();
    if (((VerticalAlign == VerticalAlign::TOP || VerticalAlign == VerticalAlign::CENTER ||
            VerticalAlign == VerticalAlign::BOTTOM)) &&
        text_->GetDeclarationHeight().IsValid()) {
        heightFinal = GetLayoutParam().GetMaxSize().Height();
    }

    return Size(text_->GetMaxWidthLayout() ? paragraphNewWidth_ : std::ceil(paragraphNewWidth_), heightFinal);
}

std::string FlutterRenderText::ApplyWhiteSpace()
{
    std::string data = text_->GetData();
    switch (textStyle_.GetWhiteSpace()) {
        case WhiteSpace::NORMAL:
            StringUtils::ReplaceTabAndNewLine(data);
            break;
        case WhiteSpace::PRE:
            break;
        case WhiteSpace::NOWRAP:
            textStyle_.SetMaxLines(1);
            StringUtils::ReplaceTabAndNewLine(data);
            break;
        case WhiteSpace::PRE_WRAP:
            break;
        case WhiteSpace::PRE_LINE:
            StringUtils::ReplaceSpace(data);
            break;
        default:
            break;
    }
    return data;
}

void FlutterRenderText::ApplyIndents(double width)
{
    std::vector<float> indents;
    double indent;
    if (textStyle_.GetTextIndent().Unit() != DimensionUnit::PERCENT) {
        indent = NormalizeToPx(textStyle_.GetTextIndent());
    } else {
        indent = width * textStyle_.GetTextIndent().Value();
    }

    if (indent > 0.0) {
        indents.push_back(indent);
        indents.push_back(0.0);
    } else {
        indents.push_back(0.0);
        indents.push_back(-indent);
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(paragraph_.get());
    if (paragraphTxt != nullptr) {
        paragraphTxt->SetIndents(indents);
    }
#else
    if (paragraph_ != nullptr) {
        paragraph_->SetIndents(indents);
    }
#endif
}

bool FlutterRenderText::UpdateParagraph()
{
    if (!text_) {
        return false;
    }

    using namespace Constants;

#ifndef USE_GRAPHIC_TEXT_GINE
    txt::ParagraphStyle style;
#else
    Rosen::TypographyStyle style;
#endif
    if (alignment_.has_value()) {
        textStyle_.SetTextAlign(alignment_.value());
    }
    const auto& textAlign = textStyle_.GetTextAlign();
    if (textAlign == TextAlign::START || textAlign == TextAlign::END) {
        std::string data = text_->GetData();
        if (!GetChildren().empty()) {
            for (const auto& child : GetChildren()) {
                auto span = DynamicCast<RenderTextSpan>(child);
                if (span && !span->GetSpanData().empty()) {
                    data = span->GetSpanData();
                    break;
                }
            }
        }
        if (!ChangeDirectionIfNeeded(data)) {
            defaultTextDirection_ = text_->GetTextDirection();
        }
    }
    std::string displayData = ApplyWhiteSpace();
#ifndef USE_GRAPHIC_TEXT_GINE
    style.text_direction = ConvertTxtTextDirection(defaultTextDirection_);
    style.text_align = ConvertTxtTextAlign(textAlign);
    style.max_lines = textStyle_.GetMaxLines();
#else
    style.textDirection = ConvertTxtTextDirection(defaultTextDirection_);
    style.textAlign = ConvertTxtTextAlign(textAlign);
    style.maxLines = textStyle_.GetMaxLines();
#endif
    style.locale = Localization::GetInstance()->GetFontLocale();
    if (textStyle_.GetTextOverflow() == TextOverflow::ELLIPSIS) {
        if (!IsCompatibleVersion() && textStyle_.GetMaxLines() == UINT32_MAX && !text_->GetAutoMaxLines()) {
#ifndef USE_GRAPHIC_TEXT_GINE
            style.max_lines = 1;
#else
            style.maxLines = 1;
#endif
        }
        style.ellipsis = ELLIPSIS;
        auto context = GetContext().Upgrade();
        if (context && context->UseLiteStyle()) {
#ifndef USE_GRAPHIC_TEXT_GINE
            style.max_lines = 1;
#else
            style.maxLines = 1;
#endif
        }
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    style.word_break_type = static_cast<minikin::WordBreakType>(textStyle_.GetWordBreak());

    std::unique_ptr<txt::ParagraphBuilder> builder;
#else
    style.wordBreakType = static_cast<Rosen::WordBreakType>(textStyle_.GetWordBreak());
#endif
    auto fontCollection = FlutterFontCollection::GetInstance().GetFontCollection();
    if (!fontCollection) {
        LOGW("UpdateParagraph: fontCollection is null");
        return false;
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    builder = txt::ParagraphBuilder::CreateTxtBuilder(style, fontCollection);
#else
    auto builder = Rosen::TypographyCreate::Create(style, fontCollection);
#endif
    std::string textValue = "";

#ifndef USE_GRAPHIC_TEXT_GINE
    txt::TextStyle txtStyle;
#else
    Rosen::TextStyle txtStyle;
#endif
    ConvertTxtStyle(textStyle_, context_, txtStyle);
    builder->PushStyle(txtStyle);
    const auto& children = GetChildren();
    if (!children.empty()) {
        touchRegions_.clear();
        for (const auto& child : children) {
            auto textSpan = AceType::DynamicCast<FlutterRenderTextSpan>(child);
            if (textSpan) {
                textSpan->UpdateText(*builder, touchRegions_, textValue);
            }
        }
    } else {
        StringUtils::TransformStrCase(displayData, (int32_t)textStyle_.GetTextCase());
#ifndef USE_GRAPHIC_TEXT_GINE
        builder->AddText(StringUtils::Str8ToStr16(displayData));
#else
        builder->AppendText(StringUtils::Str8ToStr16(displayData));
#endif
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    paragraph_ = builder->Build();
#else
    paragraph_ = builder->CreateTypography();
#endif
    ApplyIndents(GetLayoutParam().GetMaxSize().Width());
    return true;
}

double FlutterRenderText::GetTextWidth()
{
    if (!paragraph_) {
        return 0.0;
    }
    if (!IsCompatibleVersion()) {
        return paragraph_->GetMaxIntrinsicWidth();
    }
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(paragraph_.get());
    if (paragraphTxt != nullptr && paragraphTxt->GetLineCount() == 1) {
        return std::max(paragraph_->GetLongestLine(), paragraph_->GetMaxIntrinsicWidth());
    }
    return paragraph_->GetLongestLine();
#else
    if (paragraph_ != nullptr && paragraph_->GetLineCount() == 1) {
        return std::max(paragraph_->GetActualWidth(), paragraph_->GetMaxIntrinsicWidth());
    }
    return paragraph_->GetActualWidth();
#endif
}

bool FlutterRenderText::DidExceedMaxLines(double paragraphMaxWidth)
{
#ifndef USE_GRAPHIC_TEXT_GINE
    auto* paragraphTxt = static_cast<txt::ParagraphTxt*>(paragraph_.get());
    if (paragraphTxt != nullptr) {
        bool didExceedMaxLines = paragraphTxt->DidExceedMaxLines() ||
                                 (textStyle_.GetAdaptHeight() &&
                                     GreatNotEqual(paragraph_->GetHeight(), GetLayoutParam().GetMaxSize().Height()));
#else
    if (paragraph_ != nullptr) {
        bool didExceedMaxLines = paragraph_->DidExceedMaxLines() ||
                                 (textStyle_.GetAdaptHeight() &&
                                     GreatNotEqual(paragraph_->GetHeight(), GetLayoutParam().GetMaxSize().Height()));
#endif
        if (textStyle_.GetMaxLines() == 1) {
            return didExceedMaxLines || GreatNotEqual(GetTextWidth(), paragraphMaxWidth);
        }
        return didExceedMaxLines;
    }
    return false;
}

bool FlutterRenderText::ChangeDirectionIfNeeded(const std::string& data)
{
    auto declaration = text_->GetDeclaration();
    if (!declaration) {
        return false;
    }
    auto& commonAttr = static_cast<CommonAttribute&>(declaration->GetAttribute(AttributeTag::COMMON_ATTR));
    if (!commonAttr.IsValid() || commonAttr.direction != TextDirection::AUTO) {
        return false;
    }
    auto showingTextForWString = StringUtils::ToWstring(data);
    for (const auto& charOfShowingText : showingTextForWString) {
        if (u_charDirection(charOfShowingText) == UCharDirection::U_LEFT_TO_RIGHT) {
            defaultTextDirection_ = TextDirection::LTR;
            return true;
        } else if (u_charDirection(charOfShowingText) == UCharDirection::U_RIGHT_TO_LEFT ||
                   u_charDirection(charOfShowingText) == UCharDirection::U_RIGHT_TO_LEFT_ARABIC) {
            defaultTextDirection_ = TextDirection::RTL;
            return true;
        } else if (!IsCompatibleVersion() && u_charDirection(charOfShowingText) == UCharDirection::U_ARABIC_NUMBER) {
            defaultTextDirection_ = TextDirection::RTL;
            return true;
        }
    }
    return false;
}

bool FlutterRenderText::MaybeRelease()
{
    auto context = GetContext().Upgrade();
    if (context && context->GetRenderFactory() && context->GetRenderFactory()->GetRenderTextFactory()->Recycle(this)) {
        ClearRenderObject();
        return false;
    }
    return true;
}

void FlutterRenderText::ClearRenderObject()
{
    RenderText::ClearRenderObject();
    LOGD("FlutterRenderText ClearRenderObject");
    paragraph_ = nullptr;
    paragraphNewWidth_ = 0.0;
    lastLayoutMaxWidth_ = 0.0;
    lastLayoutMinWidth_ = 0.0;
    lastLayoutMaxHeight_ = 0.0;
    lastLayoutMinHeight_ = 0.0;
}

Offset FlutterRenderText::GetHandleOffset(int32_t extend)
{
    Rect result;
    GetCaretRect(extend, result);
    selectHeight_ = result.Bottom() - result.Top();
    Offset handleLocalOffset = Offset((result.Left() + result.Right()) / 2.0, result.Bottom());
    Offset handleOffset = handleLocalOffset + GetPaintRect().GetOffset() + GetOffsetToPage() + textOffsetForShowCaret_;
    return handleOffset;
}

} // namespace OHOS::Ace
