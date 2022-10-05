/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H

#include "base/geometry/ng/offset_t.h"
#include "core/common/clipboard/clipboard.h"
#include "core/common/ime/text_edit_controller.h"
#include "core/common/ime/text_input_action.h"
#include "core/common/ime/text_input_client.h"
#include "core/common/ime/text_input_configuration.h"
#include "core/common/ime/text_input_connection.h"
#include "core/common/ime/text_input_formatter.h"
#include "core/common/ime/text_input_proxy.h"
#include "core/common/ime/text_input_type.h"
#include "core/common/ime/text_selection.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/text_field/text_field_controller.h"
#include "core/components_ng/pattern/text_field/text_field_event_hub.h"
#include "core/components_ng/pattern/text_field/text_field_layout_algorithm.h"
#include "core/components_ng/pattern/text_field/text_field_layout_property.h"
#include "core/components_ng/pattern/text_field/text_field_paint_method.h"
#include "core/components_ng/pattern/text_field/text_field_paint_property.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/render/drawing.h"

#if defined(ENABLE_STANDARD_INPUT)
#include "commonlibrary/c_utils/base/include/refbase.h"

namespace OHOS::MiscServices {
class OnTextChangedListener;
}
#endif

namespace OHOS::Ace::NG {
class TextFieldPattern : public Pattern, public TextInputClient, public ValueChangeObserver {
    DECLARE_ACE_TYPE(TextFieldPattern, Pattern, TextInputClient, ValueChangeObserver);

public:
    TextFieldPattern();
    ~TextFieldPattern() override;

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override
    {
        return MakeRefPtr<TextFieldPaintMethod>(WeakClaim(this));
    }

    RefPtr<LayoutProperty> CreateLayoutProperty() override
    {
        return MakeRefPtr<TextFieldLayoutProperty>();
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<TextFieldEventHub>();
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        return MakeRefPtr<TextFieldPaintProperty>();
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<TextFieldLayoutAlgorithm>();
    }

    void OnModifyDone() override;
    int32_t GetInstanceId() const
    {
        return instanceId_;
    }
    // controller related
    void UpdateEditingValue(const std::shared_ptr<TextEditingValue>& value, bool needFireChangeEvent = true) override;

    void SetTextFieldController(const RefPtr<TextFieldController>& controller)
    {
        textFieldController_ = controller;
    }

    const RefPtr<TextFieldController>& GetTextFieldController()
    {
        return textFieldController_;
    }

    void SetTextEditController(const RefPtr<TextEditController>& textEditController)
    {
        textEditingController_ = textEditController;
    }

    const TextEditingValue& GetEditingValue() const;

    void SetEditingValue(const TextEditingValue& newValue, bool needFireChangeEvent = true);

    void UpdatePositionOfParagraph(int32_t pos);
    void UpdateCaretPositionByTouch(const Offset& offset);

    bool RequestKeyboard(bool isFocusViewChanged, bool needStartTwinkling, bool needShowSoftKeyboard);
    bool CloseKeyboard(bool forceClose);

    void UpdateConfiguration();
    void PerformAction(TextInputAction action, bool forceCloseKeyboard = false) override;
    void OnValueChanged(bool needFireChangeEvent = true, bool needFireSelectChangeEvent = true) override;

    void ClearEditingValue();

    ACE_DEFINE_PROPERTY_ITEM_FUNC_WITHOUT_GROUP(TextInputAction, TextInputAction)

    float GetBaseLineOffset() const
    {
        return baselineOffset_;
    }

    const std::shared_ptr<RSParagraph>& GetParagraph() const
    {
        return paragraph_;
    }

    bool GetCursorVisible() const
    {
        return cursorVisible_;
    }

    bool DisplayPlaceHolder();

    const Offset& GetLastTouchOffset()
    {
        return lastTouchOffset_;
    }

    TextDirection GetTextDirection()
    {
        return textDirection_;
    }

    float GetCaretOffsetX() const
    {
        return caretOffsetX_;
    }

    void SetCaretOffsetX(float offsetX)
    {
        caretOffsetX_ = offsetX;
    }

    bool GetTextModifiedByInputMethod() const
    {
        return textModifiedByInputMethod_;
    }

    void SetBasicPadding(float padding)
    {
        basicPadding_ = padding;
    }

    float GetBasicPadding() const
    {
        return basicPadding_;
    }

private:
    bool focusEventInitialized_ = false;
    void HandleFocusEvent();
    void HandleBlurEvent();
    bool HandleKeyEvent(const KeyEvent& keyEvent);
    void HandleTouchEvent(const TouchEventInfo& info);
    void HandleTouchDown(const Offset& offset);
    void HandleTouchUp();
    void InitFocusEvent();
    void InitTouchEvent();
    void CursorMoveOnClick(const Offset& offset);

    void ScheduleCursorTwinkling();
    void OnCursorTwinkling();
    void StartTwinkling();
    void StopTwinkling();

    void UpdateTextFieldManager(const Offset& offset);
    void OnTextInputActionUpdate(TextInputAction value);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    RectF textRect_;
    RectF imageRect_;
    std::shared_ptr<RSParagraph> paragraph_;

    RefPtr<ImageLoadingContext> ShowPasswordImageLoadingCtx_;
    RefPtr<ImageLoadingContext> HidePasswordImageLoadingCtx_;

    // password icon image related
    RefPtr<CanvasImage> ShowPasswordImageCanvas_;
    RefPtr<CanvasImage> HidePasswordImageCanvas_;

    RefPtr<ClickEvent> clickListener_;
    RefPtr<TouchEventImpl> touchListener_;
    CursorPositionType cursorPositionType_ = CursorPositionType::NORMAL;

    // What the keyboard should appears.
    TextInputType keyboard_ = TextInputType::UNSPECIFIED;
    // Action when "enter" pressed.
    TextInputAction action_ = TextInputAction::UNSPECIFIED;
    TextDirection textDirection_ = TextDirection::LTR;

    Offset lastTouchOffset_;
    float basicPadding_ = 0.0f;
    float baselineOffset_ = 0.0f;
    float caretOffsetX_ = 0.0f;
    bool cursorVisible_ = false;
    bool textModifiedByInputMethod_ = false;
    uint32_t twinklingInterval_ = 0;
    int32_t obscureTickCountDown_ = 0;

    Offset lastClickOffset_;

    CancelableCallback<void()> cursorTwinklingTask_;

    RefPtr<TextFieldController> textFieldController_;
    RefPtr<TextEditController> textEditingController_;
    ACE_DISALLOW_COPY_AND_MOVE(TextFieldPattern);

#if defined(ENABLE_STANDARD_INPUT)
    sptr<OHOS::MiscServices::OnTextChangedListener> textChangeListener_;
#endif
    int32_t instanceId_;
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_TEXT_FIELD_TEXT_FIELD_PATTERN_H