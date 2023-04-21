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

#include "core/components/page_transition/page_transition_component.h"

#ifndef NEW_SKIA
#include "core/components/stack/flutter_render_stack.h"
#endif
#include "core/components/stack/rosen_render_stack.h"

namespace OHOS::Ace {

RefPtr<RenderNode> PageTransitionComponent::CreateRenderNode()
{
    auto renderNode = StackComponent::CreateRenderNode();

    if (SystemProperties::GetRosenBackendEnabled()) {
        auto rosenRenderNode = AceType::DynamicCast<RosenRenderStack>(renderNode);
        if (rosenRenderNode) {
            rosenRenderNode->SyncRSNodeBoundary(true, true);
            rosenRenderNode->SetBoundary();
        }
    } else {
#ifndef NEW_SKIA
        auto flutterRenderNode = AceType::DynamicCast<FlutterRenderStack>(renderNode);
        if (flutterRenderNode) {
            flutterRenderNode->SetBoundary();
        }
#else
        return nullptr;
#endif
    }

    return renderNode;
}

} // namespace OHOS::Ace
