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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H

#include <cstdint>
#include <optional>

#include "render_service_client/core/ui/rs_node.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkRefCnt.h"

#include "base/geometry/dimension_offset.h"
#include "base/geometry/ng/offset_t.h"
#include "base/utils/noncopyable.h"
#include "core/components/common/properties/color.h"
#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/property/measure_property.h"
#include "core/components_ng/render/render_context.h"

namespace OHOS::Ace::NG {
class RosenRenderContext : public RenderContext {
    DECLARE_ACE_TYPE(RosenRenderContext, NG::RenderContext)
public:
    RosenRenderContext() = default;
    ~RosenRenderContext() override;

    void InitContext(bool isRoot, const std::optional<std::string>& surfaceName) override;

    void SyncGeometryProperties(GeometryNode* geometryNode) override;

    void RebuildFrame(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void AddFrameChildren(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void RemoveFrameChildren(FrameNode* self, const std::list<RefPtr<FrameNode>>& children) override;

    void MoveFrame(FrameNode* self, const RefPtr<FrameNode>& child, int32_t index) override;

    void OnModifyDone() override;

    void ResetBlendBgColor() override;

    void BlendBgColor(const Color& color) override;

    RefPtr<Canvas> GetCanvas() override;
    void Restore() override;

    const std::shared_ptr<Rosen::RSNode>& GetRSNode();

    void SetRSNode(const std::shared_ptr<Rosen::RSNode>& rsNode)
    {
        rsNode_ = rsNode;
    }

    void StartRecording() override;

    void StopRecordingIfNeeded() override;

    bool IsRecording()
    {
        return !!recordingCanvas_;
    }

    void StartPictureRecording(float x, float y, float width, float height);
    sk_sp<SkPicture> FinishRecordingAsPicture();

    void SetDrawContentAtLast(bool useDrawContentLastOrder) override
    {
        if (rsNode_) {
            rsNode_->SetPaintOrder(useDrawContentLastOrder);
        }
    }

    void SetClipToFrame(bool useClip) override
    {
        if (rsNode_) {
            rsNode_->SetClipToFrame(useClip);
        }
    }

    void FlushContentDrawFunction(CanvasDrawFunction&& contentDraw) override;

    void FlushForegroundDrawFunction(CanvasDrawFunction&& foregroundDraw) override;

    void FlushOverlayDrawFunction(CanvasDrawFunction&& overlayDraw) override;

    void AnimateHoverEffectScale(bool isHovered) override;
    void AnimateHoverEffectBoard(bool isHovered) override;
    void UpdateBackBlurRadius(const Dimension& radius) override;
    void UpdateFrontBlurRadius(const Dimension& radius) override;
    void UpdateBackShadow(const Shadow& shadow) override;

    void UpdateTransition(const TransitionOptions& options) override;
    bool HasAppearingTransition() const
    {
        return transitionAppearingEffect_ != nullptr;
    }
    bool HasDisappearingTransition() const
    {
        return transitionDisappearingEffect_ != nullptr;
    }

    bool TriggerPageTransition(PageTransitionType type) const override;

    static std::list<std::shared_ptr<Rosen::RSNode>> GetChildrenRSNodes(
        const std::list<RefPtr<FrameNode>>& frameChildren);

    static std::shared_ptr<Rosen::RSTransitionEffect> GetRSTransitionWithoutType(const TransitionOptions& options);

    void FlushModifier(const RefPtr<Modifier>& modifier) override;

private:
    void OnBackgroundColorUpdate(const Color& value) override;
    void OnBackgroundImageUpdate(const ImageSourceInfo& imageSourceInfo) override;
    void OnBackgroundImageRepeatUpdate(const ImageRepeat& imageRepeat) override;
    void OnBackgroundImageSizeUpdate(const BackgroundImageSize& bgImgSize) override;
    void OnBackgroundImagePositionUpdate(const BackgroundImagePosition& bgImgPosition) override;
    void OnBackgroundBlurStyleUpdate(const BlurStyle& bgBlurStyle) override;

    void OnBorderRadiusUpdate(const BorderRadiusProperty& value) override;
    void OnBorderColorUpdate(const BorderColorProperty& value) override;
    void UpdateBorderWidth(const BorderWidthPropertyF& value) override;
    void OnBorderStyleUpdate(const BorderStyleProperty& value) override;
    void OnOpacityUpdate(double opacity) override;

    void OnTransformScaleUpdate(const VectorF& value) override;
    void OnTransformCenterUpdate(const DimensionOffset& value) override;
    void OnTransformTranslateUpdate(const Vector3F& value) override;
    void OnTransformRotateUpdate(const Vector4F& value) override;

    void OnPositionUpdate(const OffsetT<Dimension>& value) override;
    void OnOffsetUpdate(const OffsetT<Dimension>& value) override;
    void OnAnchorUpdate(const OffsetT<Dimension>& value) override;
    void OnZIndexUpdate(int32_t value) override;

    void OnClipShapeUpdate(const ClipPathNG& clipPath) override;
    void OnClipEdgeUpdate(bool isClip) override;

    void OnLinearGradientUpdate(const NG::Gradient& value) override;
    void OnSweepGradientUpdate(const NG::Gradient& value) override;
    void OnRadialGradientUpdate(const NG::Gradient& value) override;

    void ReCreateRsNodeTree(const std::list<RefPtr<FrameNode>>& children);
    bool GetRSNodeTreeDiff(const std::list<std::shared_ptr<Rosen::RSNode>>& nowRSNodes,
        std::list<std::shared_ptr<Rosen::RSNode>>& toRemoveRSNodes,
        std::list<std::pair<std::shared_ptr<Rosen::RSNode>, int>>& toAddRSNodesAndIndex);

    RectF AdjustPaintRect();

    DataReadyNotifyTask CreateBgImageDataReadyCallback();
    LoadSuccessNotifyTask CreateBgImageLoadSuccessCallback();
    void PaintBackground();
    void PaintClip(const SizeF& size);
    void PaintGradient(const SizeF& frameSize);

    std::shared_ptr<Rosen::RSNode> rsNode_;
    SkPictureRecorder* recorder_ = nullptr;
    RefPtr<ImageLoadingContext> bgLoadingCtx_;
    RefPtr<Canvas> recordingCanvas_;
    RefPtr<Canvas> rosenCanvas_;
    bool isHoveredScale_ = false;
    bool isHoveredBoard_ = false;
    bool isPositionChanged_ = false;
    Color blendColor_ = Color::TRANSPARENT;
    Color hoveredColor_ = Color::TRANSPARENT;
    std::shared_ptr<Rosen::RSTransitionEffect> transitionAppearingEffect_ = nullptr;
    std::shared_ptr<Rosen::RSTransitionEffect> transitionDisappearingEffect_ = nullptr;

    ACE_DISALLOW_COPY_AND_MOVE(RosenRenderContext);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_CONTEXT_H
