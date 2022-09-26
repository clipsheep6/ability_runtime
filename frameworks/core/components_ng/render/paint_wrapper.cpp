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

#include "core/components_ng/render/paint_wrapper.h"

#include "base/utils/utils.h"
#include "core/components_ng/base/frame_node.h"
#include "core/components_ng/base/modifier.h"
#include "core/components_ng/render/node_paint_method.h"

namespace OHOS::Ace::NG {

PaintWrapper::PaintWrapper(
    WeakPtr<RenderContext> renderContext, RefPtr<GeometryNode> geometryNode, RefPtr<PaintProperty> layoutProperty)
    : renderContext_(std::move(renderContext)), geometryNode_(std::move(geometryNode)),
      paintProperty_(std::move(layoutProperty))
{}

PaintWrapper::~PaintWrapper() = default;

void PaintWrapper::SetNodePaintMethod(const RefPtr<NodePaintMethod>& nodePaintImpl)
{
    nodePaintImpl_ = nodePaintImpl;
    CHECK_NULL_VOID(nodePaintImpl_);
    auto renderContext = renderContext_.Upgrade();
    CHECK_NULL_VOID(renderContext);
    auto modifier = nodePaintImpl_->GetModifier(this);
    if (modifier) {
        renderContext->FlushModifier(modifier);
    }
}

void PaintWrapper::FlushRender()
{
    CHECK_NULL_VOID(nodePaintImpl_);

    auto renderContext = renderContext_.Upgrade();
    CHECK_NULL_VOID(renderContext);

    auto modifier = nodePaintImpl_->GetModifier(this);
    if (modifier) {
        nodePaintImpl_->UpdateModifier(this);
        return;
    }

    renderContext->StartRecording();

    // first set content paint function.
    auto contentDraw = nodePaintImpl_->GetContentDrawFunction(this);
    if (contentDraw) {
        renderContext->FlushContentDrawFunction(std::move(contentDraw));
    }

    // then set foreground paint function.
    auto foregroundDraw = nodePaintImpl_->GetForegroundDrawFunction(this);
    if (foregroundDraw) {
        renderContext->FlushForegroundDrawFunction(std::move(foregroundDraw));
    }

    // at last, set overlay paint function.
    auto overlayDraw = nodePaintImpl_->GetOverlayDrawFunction(this);
    if (overlayDraw) {
        renderContext->FlushOverlayDrawFunction(std::move(overlayDraw));
    }

    renderContext->StopRecordingIfNeeded();
}
} // namespace OHOS::Ace::NG
