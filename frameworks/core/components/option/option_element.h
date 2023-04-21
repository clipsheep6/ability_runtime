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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_OPTION_OPTION_ELEMENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_OPTION_OPTION_ELEMENT_H

#include "core/components/text/render_text.h"
#include "core/components/text/text_component.h"
#include "core/pipeline/base/component_group_element.h"

namespace OHOS::Ace {

class OptionElement : public ComponentGroupElement, public FocusGroup {
    DECLARE_ACE_TYPE(OptionElement, ComponentGroupElement, FocusGroup);

public:
    void PerformBuild() override;

protected:
    bool OnKeyEvent(const KeyEvent& keyEvent) override;
    void OnFocus() override;
    void OnClick() override;
    void OnBlur() override;
    bool RequestNextFocus(bool vertical, bool reverse, const Rect& rect) override;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_OPTION_OPTION_ELEMENT_H
