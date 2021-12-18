/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATEPICKER_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATEPICKER_H

#include "bridge/declarative_frontend/jsview/js_interactable_view.h"
#include "bridge/declarative_frontend/jsview/js_view_abstract.h"
#include "core/components/picker/picker_base_component.h"

namespace OHOS::Ace::Framework {
class JSDatePicker : public JSViewAbstract {
public:
    static void Create(const JSCallbackInfo& info);

    static void JSBind(BindingTarget globalObj);
    static void SetLunar(bool isLunar);
    static void OnChange(const JSCallbackInfo& info);
    static void UseMilitaryTime(bool isUseMilitaryTime);

private:
    static void CreateDatePicker(const JSRef<JSObject>& paramObj);
    static void CreateTimePicker(const JSRef<JSObject>& paramObj);
    static PickerDate ParseDate(const JSRef<JSVal>& dateVal);
    static PickerTime ParseTime(const JSRef<JSVal>& timeVal);
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_JS_DATEPICKER_H
