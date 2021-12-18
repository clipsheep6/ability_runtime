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

#include "frameworks/bridge/declarative_frontend/jsview/js_image.h"

#if !defined(WINDOWS_PLATFORM) and !defined(MAC_PLATFORM)
#include <dlfcn.h>
#endif

#include "base/image/pixel_map.h"
#include "base/log/ace_trace.h"
#include "frameworks/bridge/declarative_frontend/engine/js_ref_ptr.h"
#include "frameworks/bridge/declarative_frontend/view_stack_processor.h"

namespace OHOS::Ace::Framework {

JSRef<JSVal> LoadImageSuccEventToJSValue(const LoadImageSuccessEvent& eventInfo)
{
    JSRef<JSObject> obj = JSRef<JSObject>::New();
    obj->SetProperty("width", eventInfo.GetWidth());
    obj->SetProperty("height", eventInfo.GetHeight());
    obj->SetProperty("componentWidth", eventInfo.GetComponentWidth());
    obj->SetProperty("componentHeight", eventInfo.GetComponentHeight());
    obj->SetProperty("loadingStatus", eventInfo.GetLoadingStatus());
    return JSRef<JSVal>::Cast(obj);
}

JSRef<JSVal> LoadImageFailEventToJSValue(const LoadImageFailEvent& eventInfo)
{
    JSRef<JSObject> obj = JSRef<JSObject>::New();
    obj->SetProperty("componentWidth", eventInfo.GetComponentWidth());
    obj->SetProperty("componentHeight", eventInfo.GetComponentHeight());
    return JSRef<JSVal>::Cast(obj);
}

void JSImage::SetAlt(const JSCallbackInfo& args)
{
    if (args.Length() < 1) {
        LOGE("The argv is wrong, it it supposed to have at least 1 argument");
        return;
    }

    std::string src;
    if (!ParseJsMedia(args[0], src)) {
        return;
    }

    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetAlt(src);
    }
}

void JSImage::SetObjectFit(int32_t value)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetImageFit(static_cast<ImageFit>(value));
    }
}

void JSImage::SetMatchTextDirection(bool value)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetMatchTextDirection(value);
    }
}

void JSImage::SetFitOriginalSize(bool value)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetFitMaxSize(!value);
    }
}

RefPtr<Decoration> JSImage::GetFrontDecoration()
{
    auto box = ViewStackProcessor::GetInstance()->GetBoxComponent();
    auto decoration = box->GetFrontDecoration();
    if (!decoration) {
        decoration = AceType::MakeRefPtr<Decoration>();
        box->SetFrontDecoration(decoration);
    }

    return decoration;
}

const Border& JSImage::GetBorder()
{
    return GetFrontDecoration()->GetBorder();
}

BorderEdge JSImage::GetLeftBorderEdge()
{
    return GetBorder().Left();
}

BorderEdge JSImage::GetTopBorderEdge()
{
    return GetBorder().Top();
}

BorderEdge JSImage::GetRightBorderEdge()
{
    return GetBorder().Right();
}

BorderEdge JSImage::GetBottomBorderEdge()
{
    return GetBorder().Bottom();
}

void JSImage::SetBorderEdge(const BorderEdge& edge)
{
    Border border = GetBorder();
    border.SetBorderEdge(edge);
    SetBorder(border);
}

void JSImage::SetLeftBorderEdge(const BorderEdge& edge)
{
    Border border = GetBorder();
    border.SetLeftEdge(edge);
    SetBorder(border);
}

void JSImage::SetTopBorderEdge(const BorderEdge& edge)
{
    Border border = GetBorder();
    border.SetTopEdge(edge);
    SetBorder(border);
}

void JSImage::SetRightBorderEdge(const BorderEdge& edge)
{
    Border border = GetBorder();
    border.SetRightEdge(edge);
    SetBorder(border);
}

void JSImage::SetBottomBorderEdge(const BorderEdge& edge)
{
    Border border = GetBorder();
    border.SetBottomEdge(edge);
    SetBorder(border);
}

void JSImage::SetBorder(const Border& border)
{
    GetFrontDecoration()->SetBorder(border);
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetBorder(border);
    }
}

void JSImage::SetBorderRadius(const Dimension& value)
{
    Border border = GetBorder();
    border.SetBorderRadius(Radius(value));
    SetBorder(border);
}

void JSImage::SetBorderStyle(int32_t style)
{
    BorderStyle borderStyle = BorderStyle::SOLID;

    if (static_cast<int32_t>(BorderStyle::SOLID) == style) {
        borderStyle = BorderStyle::SOLID;
    } else if (static_cast<int32_t>(BorderStyle::DASHED) == style) {
        borderStyle = BorderStyle::DASHED;
    } else if (static_cast<int32_t>(BorderStyle::DOTTED) == style) {
        borderStyle = BorderStyle::DOTTED;
    } else {
        borderStyle = BorderStyle::NONE;
    }

    BorderEdge edge = GetLeftBorderEdge();
    edge.SetStyle(borderStyle);
    auto stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    if (!stack->IsVisualStateSet()) {
        SetBorderEdge(edge);
    } else {
        box->SetBorderStyleForState(borderStyle, stack->GetVisualState());
    }
}

void JSImage::SetBorderColor(const Color& color)
{
    SetLeftBorderColor(color);
    SetTopBorderColor(color);
    SetRightBorderColor(color);
    SetBottomBorderColor(color);
}

void JSImage::SetLeftBorderColor(const Color& color)
{
    BorderEdge edge = GetLeftBorderEdge();
    edge.SetColor(color);
    SetLeftBorderEdge(edge);
}

void JSImage::SetTopBorderColor(const Color& color)
{
    BorderEdge edge = GetTopBorderEdge();
    edge.SetColor(color);
    SetTopBorderEdge(edge);
}

void JSImage::SetRightBorderColor(const Color& color)
{
    BorderEdge edge = GetRightBorderEdge();
    edge.SetColor(color);
    SetRightBorderEdge(edge);
}

void JSImage::SetBottomBorderColor(const Color& color)
{
    BorderEdge edge = GetBottomBorderEdge();
    edge.SetColor(color);
    SetBottomBorderEdge(edge);
}

void JSImage::SetBorderWidth(const Dimension& value)
{
    SetLeftBorderWidth(value);
    SetTopBorderWidth(value);
    SetRightBorderWidth(value);
    SetBottomBorderWidth(value);
}

void JSImage::SetLeftBorderWidth(const Dimension& value)
{
    BorderEdge edge = GetLeftBorderEdge();
    edge.SetWidth(value);
    SetLeftBorderEdge(edge);
}

void JSImage::SetTopBorderWidth(const Dimension& value)
{
    BorderEdge edge = GetTopBorderEdge();
    edge.SetWidth(value);
    SetTopBorderEdge(edge);
}

void JSImage::SetRightBorderWidth(const Dimension& value)
{
    BorderEdge edge = GetRightBorderEdge();
    edge.SetWidth(value);
    SetRightBorderEdge(edge);
}

void JSImage::SetBottomBorderWidth(const Dimension& value)
{
    BorderEdge edge = GetBottomBorderEdge();
    edge.SetWidth(value);
    SetBottomBorderEdge(edge);
}

void JSImage::JsBorderColor(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Color borderColor;
    if (!ParseJsColor(info[0], borderColor)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    if (!stack->IsVisualStateSet()) {
        SetBorderColor(borderColor);
    } else {
        AnimationOption option = ViewStackProcessor::GetInstance()->GetImplicitAnimationOption();
        box->SetBorderColorForState(borderColor, option, stack->GetVisualState());
    }
}

void JSImage::OnComplete(const JSCallbackInfo& args)
{
    LOGD("JSImage V8OnComplete");
    if (args[0]->IsFunction()) {
        auto jsLoadSuccFunc = AceType::MakeRefPtr<JsEventFunction<LoadImageSuccessEvent, 1>>(
            JSRef<JSFunc>::Cast(args[0]), LoadImageSuccEventToJSValue);
        auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
        image->SetLoadSuccessEvent(EventMarker(
            [execCtx = args.GetExecutionContext(), func = std::move(jsLoadSuccFunc)](const BaseEventInfo* info) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                auto eventInfo = TypeInfoHelper::DynamicCast<LoadImageSuccessEvent>(info);
                func->Execute(*eventInfo);
            }));
    } else {
        LOGE("args not function");
    }
}

void JSImage::OnError(const JSCallbackInfo& args)
{
    LOGD("JSImage V8OnError");
    if (args[0]->IsFunction()) {
        auto jsLoadFailFunc = AceType::MakeRefPtr<JsEventFunction<LoadImageFailEvent, 1>>(
            JSRef<JSFunc>::Cast(args[0]), LoadImageFailEventToJSValue);
        auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
        image->SetLoadFailEvent(EventMarker(
            [execCtx = args.GetExecutionContext(), func = std::move(jsLoadFailFunc)](const BaseEventInfo* info) {
                JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
                auto eventInfo = TypeInfoHelper::DynamicCast<LoadImageFailEvent>(info);
                func->Execute(*eventInfo);
            }));
    } else {
        LOGE("args not function");
    }
}

void JSImage::OnFinish(const JSCallbackInfo& info)
{
    LOGD("JSImage V8OnFinish");
    if (!info[0]->IsFunction()) {
        LOGE("info[0] is not a function.");
        return;
    }
    RefPtr<JsFunction> jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto eventMarker = EventMarker([execCtx = info.GetExecutionContext(), func = std::move(jsFunc)]() {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        func->Execute();
    });
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetSvgAnimatorFinishEvent(eventMarker);
}

void JSImage::Create(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    std::string src;
    auto usePixMap = ParseJsMedia(info[0], src);
    RefPtr<ImageComponent> imageComponent = AceType::MakeRefPtr<OHOS::Ace::ImageComponent>(src);
    imageComponent->SetUseSkiaSvg(false);
    ViewStackProcessor::GetInstance()->Push(imageComponent);
    if (usePixMap) {
        return;
    }

#if !defined(WINDOWS_PLATFORM) and !defined(MAC_PLATFORM)
    imageComponent->SetPixmap(CreatePixelMapFromNapiValue(info[0]));
#endif
}

void JSImage::JsBorderWidth(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension borderWidth;
    if (!ParseJsDimensionVp(info[0], borderWidth)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    if (!stack->IsVisualStateSet()) {
        SetBorderWidth(borderWidth);
    } else {
        AnimationOption option = ViewStackProcessor::GetInstance()->GetImplicitAnimationOption();
        box->SetBorderWidthForState(borderWidth, option, stack->GetVisualState());
    }
}

void JSImage::JsBorderRadius(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The argv is wrong, it is supposed to have at least 1 argument");
        return;
    }
    Dimension borderRadius;
    if (!ParseJsDimensionVp(info[0], borderRadius)) {
        return;
    }
    auto stack = ViewStackProcessor::GetInstance();
    auto box = stack->GetBoxComponent();
    if (!stack->IsVisualStateSet()) {
        SetBorderRadius(borderRadius);
    } else {
        AnimationOption option = ViewStackProcessor::GetInstance()->GetImplicitAnimationOption();
        box->SetBorderRadiusForState(borderRadius, option, stack->GetVisualState());
    }
}

void JSImage::JsBorder(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }
    if (!info[0]->IsObject()) {
        LOGE("arg is not a object.");
        return;
    }

    auto argsPtrItem = JsonUtil::ParseJsonString(info[0]->ToString());
    if (!argsPtrItem || argsPtrItem->IsNull()) {
        LOGE("Js Parse object failed. argsPtr is null. %s", info[0]->ToString().c_str());
        info.SetReturnValue(info.This());
        return;
    }
    Dimension width = Dimension(0.0, DimensionUnit::VP);
    Dimension radius = Dimension(0.0, DimensionUnit::VP);
    ParseJsonDimensionVp(argsPtrItem->GetValue("width"), width);
    ParseJsonDimensionVp(argsPtrItem->GetValue("radius"), radius);
    auto borderStyle = argsPtrItem->GetInt("style", static_cast<int32_t>(BorderStyle::SOLID));
    LOGD("JsBorder width = %lf unit = %d, radius = %lf unit = %d, borderStyle = %d", width.Value(), width.Unit(),
        radius.Value(), radius.Unit(), borderStyle);
    Color color;
    if (ParseJsonColor(argsPtrItem->GetValue("color"), color)) {
        SetBorderColor(color);
    }
    SetBorderStyle(borderStyle);
    SetBorderWidth(width);
    SetBorderRadius(radius);
    info.SetReturnValue(info.This());
}

void JSImage::SetSourceSize(const JSCallbackInfo& info)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetImageSourceSize(JSViewAbstract::ParseSize(info));
}

void JSImage::SetImageFill(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 arguments");
        return;
    }

    Color color;
    if (!ParseJsColor(info[0], color)) {
        return;
    }
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetImageFill(color);
}

void JSImage::SetImageRenderMode(int32_t imageRenderMode)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetImageRenderMode(static_cast<ImageRenderMode>(imageRenderMode));
}

void JSImage::SetImageInterpolation(int32_t imageInterpolation)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetImageInterpolation(static_cast<ImageInterpolation>(imageInterpolation));
}

void JSImage::SetImageRepeat(int32_t imageRepeat)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    image->SetImageRepeat(static_cast<ImageRepeat>(imageRepeat));
}

void JSImage::JsTransition(const JSCallbackInfo& info)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image && image->IsSrcSvgImage()) {
        JSViewAbstract::JsTransition(info);
    } else {
        JSViewAbstract::JsTransitionPassThrough(info);
    }
}

void JSImage::JsOpacity(const JSCallbackInfo& info)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image && image->IsSrcSvgImage()) {
        JSViewAbstract::JsOpacity(info);
    } else {
        JSViewAbstract::JsOpacityPassThrough(info);
    }
}

void JSImage::SetAutoResize(bool autoResize)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetAutoResize(autoResize);
    }
}

void JSImage::SetSyncLoad(const JSCallbackInfo& info)
{
    auto image = AceType::DynamicCast<ImageComponent>(ViewStackProcessor::GetInstance()->GetMainComponent());
    if (image) {
        image->SetSyncMode(info[0]->ToBoolean());
    }
}

void JSImage::JSBind(BindingTarget globalObj)
{
    JSClass<JSImage>::Declare("Image");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSImage>::StaticMethod("create", &JSImage::Create, opt);
    JSClass<JSImage>::StaticMethod("alt", &JSImage::SetAlt, opt);
    JSClass<JSImage>::StaticMethod("objectFit", &JSImage::SetObjectFit, opt);
    JSClass<JSImage>::StaticMethod("matchTextDirection", &JSImage::SetMatchTextDirection, opt);
    JSClass<JSImage>::StaticMethod("fitOriginalSize", &JSImage::SetFitOriginalSize, opt);
    JSClass<JSImage>::StaticMethod("sourceSize", &JSImage::SetSourceSize, opt);
    JSClass<JSImage>::StaticMethod("fillColor", &JSImage::SetImageFill, opt);
    JSClass<JSImage>::StaticMethod("renderMode", &JSImage::SetImageRenderMode, opt);
    JSClass<JSImage>::StaticMethod("objectRepeat", &JSImage::SetImageRepeat, opt);
    JSClass<JSImage>::StaticMethod("interpolation", &JSImage::SetImageInterpolation, opt);
    JSClass<JSImage>::StaticMethod("borderStyle", &JSImage::SetBorderStyle, opt);
    JSClass<JSImage>::StaticMethod("borderColor", &JSImage::JsBorderColor);
    JSClass<JSImage>::StaticMethod("border", &JSImage::JsBorder);
    JSClass<JSImage>::StaticMethod("borderWidth", &JSImage::JsBorderWidth);
    JSClass<JSImage>::StaticMethod("borderRadius", &JSImage::JsBorderRadius);
    JSClass<JSImage>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSImage>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSImage>::StaticMethod("autoResize", &JSImage::SetAutoResize);

    JSClass<JSImage>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSImage>::StaticMethod("onHover", &JSInteractableView::JsOnHover);
    JSClass<JSImage>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSImage>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSImage>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSImage>::StaticMethod("onComplete", &JSImage::OnComplete);
    JSClass<JSImage>::StaticMethod("onError", &JSImage::OnError);
    JSClass<JSImage>::StaticMethod("onFinish", &JSImage::OnFinish);
    JSClass<JSImage>::StaticMethod("syncLoad", &JSImage::SetSyncLoad);
    JSClass<JSImage>::Inherit<JSViewAbstract>();
    // override method
    JSClass<JSImage>::StaticMethod("opacity", &JSImage::JsOpacity);
    JSClass<JSImage>::StaticMethod("transition", &JSImage::JsTransition);
    JSClass<JSImage>::Bind<>(globalObj);
}

} // namespace OHOS::Ace::Framework
