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

#ifndef FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_IMAGE_MODEL_IMPL_H
#define FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_IMAGE_MODEL_IMPL_H

#include "core/components_ng/pattern/image/image_model.h"
#include "frameworks/bridge/declarative_frontend/jsview/js_interactable_view.h"

namespace OHOS::Ace::Framework {

// ImageModel is essentially the same class as ImageView
// there should be only one left in the final implementation

class ImageModelImpl : public OHOS::Ace::ImageModel {
public:
    void SetAlt(std::string src) override;
    void SetBlur(double blur) override;
    void SetImageFit(int32_t value) override;
    void SetMatchTextDirection(bool value) override;
    void SetFitMaxSize(bool value) override;
    void SetOnComplete(std::function<void(const LoadImageSuccessEvent& info)>&& callback) override;
    void SetOnError(std::function<void(const LoadImageFailEvent& info)>&& callback) override;
    void SetSvgAnimatorFinishEvent(std::function<void()>&& callback) override;
    void Create(std::string src, bool noPixMap, RefPtr<PixelMap>& pixMap) override;
    void SetImageSourceSize(std::pair<Dimension, Dimension> size) override;
    void SetImageFill(Color color) override;
    void SetImageInterpolation(ImageInterpolation iterpolation) override;
    void SetImageRepeat(ImageRepeat imageRepeat) override;
    void SetImageRenderMode(ImageRenderMode imageRenderMode) override;
    bool IsSrcSvgImage() override;
    void SetAutoResize(bool autoResize) override;
    void SetSyncMode(bool syncMode) override;
    void SetColorFilterMatrix(const std::vector<float>& matrix) override;
    void SetOnDragStartId(const OnDragFunc& onDragStartId) override;
    void SetOnDragEnterId(const OnDropFunc&  onDragStartId) override;
    void SetOnDragLeaveId(const OnDropFunc& onDragStartId) override;
    void SetOnDragMoveId(const OnDropFunc& onDragMoveId) override;
    void SetOnDropId(const OnDropFunc& onDropId) override;
    void SetCopyOption(const CopyOptions& copyOption) override;
    bool UpdateDragItemInfo(DragItemInfo& itemInfo) override;
};

} // namespace OHOS::Ace::Framework
#endif // FRAMEWORKS_BRIDGE_DECLARATIVE_FRONTEND_JS_VIEW_MODELS_IMAGE_MODEL_IMPL_H
