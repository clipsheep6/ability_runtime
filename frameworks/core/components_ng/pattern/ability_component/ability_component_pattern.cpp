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

#include "core/components_ng/pattern/ability_component/ability_component_pattern.h"

#include "base/geometry/rect.h"
#include "base/utils/utils.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "core/pipeline_ng/ui_task_scheduler.h"

namespace OHOS::Ace::NG {

void AbilityComponentPattern::OnModifyDone()
{
    adapter_ = WindowExtensionConnectionProxy::CreateAdapter();
    if (adapter_) {
        adapter_->ConnectExtension(GetHost());
    }
    LOGI("connect to windows extension begin %{public}s", GetHost()->GetTag().c_str());
}

bool AbilityComponentPattern::OnDirtyLayoutWrapperSwap(
    const RefPtr<LayoutWrapper>& /*dirty*/, bool /*skipMeasure*/, bool /*skipLayout*/)
{
    if (hasConnectionToAbility_) {
        auto size = GetHost()->GetGeometryNode()->GetFrameSize();
        auto offset = GetHost()->GetGeometryNode()->GetFrameOffset();
        LOGI("ConnectExtension: %{public}f %{public}f %{public}f %{public}f", offset.GetX(), offset.GetY(),
            size.Width(), size.Height());
        Rect rect;
        rect.SetRect(offset.GetX(), offset.GetY(), size.Width(), size.Height());
        if (adapter_) {
            adapter_->UpdateRect(rect);
        }
    }
    return false;
}

void AbilityComponentPattern::FireConnect()
{
    hasConnectionToAbility_ = true;
    auto abilityComponentEventHub = GetEventHub<AbilityComponentEventHub>();
    CHECK_NULL_VOID(abilityComponentEventHub);
    abilityComponentEventHub->FireOnConnect();
}

void AbilityComponentPattern::FireDisConnect()
{
    hasConnectionToAbility_ = false;
    auto abilityComponentEventHub = GetEventHub<AbilityComponentEventHub>();
    CHECK_NULL_VOID(abilityComponentEventHub);
    abilityComponentEventHub->FireOnDisConnect();
}

} // namespace OHOS::Ace::NG