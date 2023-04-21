/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "core/components_v2/inspector/counter_composed_element.h"

#include "base/log/dump_log.h"
#include "core/components/counter/render_counter.h"

namespace OHOS::Ace::V2 {
std::string CounterComposedElement::GetWidth() const
{
    auto renderCounter = GetContentRender<RenderCounter>(CounterElement::TypeId());
    if (!renderCounter) {
        return "";
    }
    auto component = AceType::DynamicCast<CounterComponent>(renderCounter->GetComponent());
    if (!component) {
        return "";
    }
    return component->GetWidth().ToString();
}

std::string CounterComposedElement::GetHeight() const
{
    auto renderCounter = GetContentRender<RenderCounter>(CounterElement::TypeId());
    if (!renderCounter) {
        return "";
    }
    auto component = AceType::DynamicCast<CounterComponent>(renderCounter->GetComponent());
    if (!component) {
        return "";
    }
    return component->GetHeight().ToString();
}
}
