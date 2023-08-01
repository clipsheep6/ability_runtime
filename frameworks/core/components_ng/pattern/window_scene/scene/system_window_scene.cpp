/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "core/components_ng/pattern/window_scene/scene/system_window_scene.h"

#include "ui/rs_surface_node.h"

#include "core/components_ng/render/adapter/rosen_render_context.h"

namespace OHOS::Ace::NG {
void SystemWindowScene::OnAttachToFrameNode()
{
    CHECK_NULL_VOID(session_);
    auto sessionInfo = session_->GetSessionInfo();
    auto name = sessionInfo.bundleName_;
    auto pos = name.find_last_of('.');
    name = (pos == std::string::npos) ? name : name.substr(pos + 1); // skip '.'

    Rosen::RSSurfaceNodeConfig config;
    config.SurfaceNodeName = name;
    auto surfaceNode = Rosen::RSSurfaceNode::Create(config, Rosen::RSSurfaceNodeType::APP_WINDOW_NODE);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto context = AceType::DynamicCast<NG::RosenRenderContext>(host->GetRenderContext());
    CHECK_NULL_VOID(context);
    context->SetRSNode(surfaceNode);

    auto frameNode = frameNode_.Upgrade();
    CHECK_NULL_VOID(frameNode);
    auto gestureHub = frameNode->GetOrCreateGestureEventHub();
    auto touchCallback = [this](TouchEventInfo& info) {
        const auto pointerEvent = info.GetPointerEvent();
        CHECK_NULL_VOID(session_);
        CHECK_NULL_VOID(pointerEvent);
        session_->TransferPointerEvent(pointerEvent);
    };
    gestureHub->SetTouchEvent(std::move(touchCallback));

    auto mouseEventHub = frameNode->GetOrCreateInputEventHub();
    auto mouseCallback = [this](MouseInfo& info) {
        const auto pointerEvent = info.GetPointerEvent();
        CHECK_NULL_VOID(session_);
        CHECK_NULL_VOID(pointerEvent);
        session_->TransferPointerEvent(pointerEvent);
    };
    mouseEventHub->SetMouseEvent(std::move(mouseCallback));
}

bool SystemWindowScene::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    CHECK_NULL_RETURN(dirty, false);
    auto host = dirty->GetHostNode();
    CHECK_NULL_RETURN(host, false);
    auto globalOffsetWithTranslate = host->GetPaintRectGlobalOffsetWithTranslate();
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);
    auto frameRect = geometryNode->GetFrameRect();
    Rosen::WSRect windowRect {
        .posX_ = std::round(globalOffsetWithTranslate.GetX()),
        .posY_ = std::round(globalOffsetWithTranslate.GetY()),
        .width_ = std::round(frameRect.Width()),
        .height_ = std::round(frameRect.Height())
    };

    CHECK_NULL_RETURN(session_, false);
    session_->UpdateRect(windowRect, Rosen::SizeChangeReason::UNDEFINED);
    return false;
}

void SystemWindowScene::OnSetDepth(const int32_t depth)
{
    CHECK_NULL_VOID(session_);
    session_->SetZOrder(static_cast<uint32_t>(depth));
}
} // namespace OHOS::Ace::NG
