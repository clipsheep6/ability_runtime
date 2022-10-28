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

#include "frameworks/bridge/declarative_frontend/jsview/js_text_clock.h"

#include <regex>
#include <string>

#include "base/log/ace_scoring_log.h"
#include "bridge/declarative_frontend/jsview/js_text.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "bridge/declarative_frontend/jsview/js_view_common_def.h"
#include "bridge/declarative_frontend/jsview/models/text_clock_model_impl.h"
#include "core/components_ng/pattern/text_clock/text_clock_model.h"
#include "core/components_ng/pattern/text_clock/text_clock_model_ng.h"

namespace OHOS::Ace {

std::unique_ptr<TextClockModel> TextClockModel::instance_ = nullptr;

TextClockModel* TextClockModel::GetInstance()
{
    if (!instance_) {
#ifdef NG_BUILD
        instance_.reset(new NG::TextClockModelNG());
#else
        if (Container::IsCurrentUseNewPipeline()) {
            instance_.reset(new NG::TextClockModelNG());
        } else {
            instance_.reset(new Framework::TextClockModelImpl());
        }
#endif
    }
    return instance_.get();
}
} // namespace OHOS::Ace

namespace OHOS::Ace::Framework {

namespace {
const std::vector<FontStyle> FONT_STYLES = { FontStyle::NORMAL, FontStyle::ITALIC };
const int32_t TWENTY_FOUR_HOUR_BASE = 24;
constexpr int32_t HOURS_WEST_LOWER_LIMIT = -14;
constexpr int32_t HOURS_WEST_UPPER_LIMIT = 12;
constexpr int32_t HOURS_WEST_GEOGRAPHICAL_LOWER_LIMIT = -12;

bool HoursWestIsValid(int32_t hoursWest)
{
    if (hoursWest < HOURS_WEST_LOWER_LIMIT || hoursWest > HOURS_WEST_UPPER_LIMIT) {
        return false;
    }
    if (hoursWest < HOURS_WEST_GEOGRAPHICAL_LOWER_LIMIT) {
        hoursWest += TWENTY_FOUR_HOUR_BASE;
    }
    return true;
}
} // namespace

void JSTextClock::Create(const JSCallbackInfo& info)
{
    auto controller = TextClockModel::GetInstance()->Create();
    if (info.Length() < 1 || !info[0]->IsObject()) {
        LOGD("TextClock Info is non-valid");
    } else {
        JSRef<JSObject> optionsObject = JSRef<JSObject>::Cast(info[0]);
        JSRef<JSVal> hourWestVal = optionsObject->GetProperty("timeZoneOffset");
        if (hourWestVal->IsNumber()) {
            auto hourWest_ = hourWestVal->ToNumber<int32_t>();
            if (HoursWestIsValid(hourWest_)) {
                TextClockModel::GetInstance()->SetHoursWest(hourWest_);
            } else {
                LOGE("hourWest args is invalid");
            }
        } else {
            LOGE("hourWest args is not number,args is invalid");
        }
        auto controllerObj = optionsObject->GetProperty("controller");
        if (!controllerObj->IsUndefined() && !controllerObj->IsNull()) {
            auto* jsController = JSRef<JSObject>::Cast(controllerObj)->Unwrap<JSTextClockController>();
            if (jsController != nullptr) {
                if (controller) {
                    jsController->SetController(controller);
                } else {
                    LOGE("TextClockController is nullptr");
                }
            }
        } else {
            LOGE("controllerObj is nullptr or undefined");
        }
    }
}

void JSTextClock::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextClock>::Declare("TextClock");
    MethodOptions opt = MethodOptions::NONE;
    JSClass<JSTextClock>::StaticMethod("create", &JSTextClock::Create, opt);
    JSClass<JSTextClock>::StaticMethod("format", &JSTextClock::SetFormat, opt);
    JSClass<JSTextClock>::StaticMethod("onDateChange", &JSTextClock::JsOnDateChange, opt);
    JSClass<JSTextClock>::StaticMethod("onClick", &JSInteractableView::JsOnClick);
    JSClass<JSTextClock>::StaticMethod("onTouch", &JSInteractableView::JsOnTouch);
    JSClass<JSTextClock>::StaticMethod("onKeyEvent", &JSInteractableView::JsOnKey);
    JSClass<JSTextClock>::StaticMethod("onDeleteEvent", &JSInteractableView::JsOnDelete);
    JSClass<JSTextClock>::StaticMethod("onAppear", &JSInteractableView::JsOnAppear);
    JSClass<JSTextClock>::StaticMethod("onDisAppear", &JSInteractableView::JsOnDisAppear);
    JSClass<JSTextClock>::StaticMethod("fontColor", &JSText::SetTextColor, opt);
    JSClass<JSTextClock>::StaticMethod("fontSize", &JSText::SetFontSize, opt);
    JSClass<JSTextClock>::StaticMethod("fontStyle", &JSText::SetFontStyle, opt);
    JSClass<JSTextClock>::StaticMethod("fontWeight", &JSText::SetFontWeight, opt);
    JSClass<JSTextClock>::StaticMethod("fontFamily", &JSText::SetFontFamily, opt);
    JSClass<JSTextClock>::Inherit<JSViewAbstract>();
    JSClass<JSTextClock>::Bind<>(globalObj);
}

void JSTextClock::SetFormat(const JSCallbackInfo& info)
{
    if (info.Length() < 1) {
        LOGE("The arg is wrong, it is supposed to have atleast 1 argument.");
        return;
    }
    std::string value;
    if (!ParseJsString(info[0], value)) {
        return;
    }
    std::regex pattern(
        R"(^([Yy]*[_|\W\s]*[M]*[_|\W\s]*[d]*[_|\W\s]*[D]*[_|\W\s]*[Hh]*[_|\W\s]*[m]*[_|\W\s]*[s]*[_|\W\s]*[S]*)$)");
    if (!std::regex_match(value, pattern)) {
        LOGE("The arg is wrong, because of format matching error.");
        return;
    }
    TextClockModel::GetInstance()->SetFormat(value);
}

void JSTextClock::JsOnDateChange(const JSCallbackInfo& info)
{
    if (!info[0]->IsFunction()) {
        return;
    }

    auto jsFunc = AceType::MakeRefPtr<JsFunction>(JSRef<JSObject>(), JSRef<JSFunc>::Cast(info[0]));
    auto onChange = [execCtx = info.GetExecutionContext(), func = std::move(jsFunc)](const std::string& value) {
        JAVASCRIPT_EXECUTION_SCOPE_WITH_CHECK(execCtx);
        ACE_SCORING_EVENT("TextClock.onDateChange");
        auto newJSVal = JSRef<JSVal>::Make(ToJSValue(value));
        func->ExecuteJS(1, &newJSVal);
    };
    TextClockModel::GetInstance()->SetOnDateChange(std::move(onChange));
}

void JSTextClockController::JSBind(BindingTarget globalObj)
{
    JSClass<JSTextClockController>::Declare("TextClockController");
    JSClass<JSTextClockController>::Method("start", &JSTextClockController::Start);
    JSClass<JSTextClockController>::Method("stop", &JSTextClockController::Stop);
    JSClass<JSTextClockController>::Bind(
        globalObj, JSTextClockController::Constructor, JSTextClockController::Destructor);
}

void JSTextClockController::Constructor(const JSCallbackInfo& args)
{
    auto scroller = Referenced::MakeRefPtr<JSTextClockController>();
    scroller->IncRefCount();
    args.SetReturnValue(Referenced::RawPtr(scroller));
}

void JSTextClockController::Destructor(JSTextClockController* scroller)
{
    if (scroller != nullptr) {
        scroller->DecRefCount();
    }
}

void JSTextClockController::Start()
{
    if (controller_) {
        controller_->Start();
    }
}

void JSTextClockController::Stop()
{
    if (controller_) {
        controller_->Stop();
    }
}
} // namespace OHOS::Ace::Framework
