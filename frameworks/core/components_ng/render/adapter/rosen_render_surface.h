/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_SURFACE_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_SURFACE_H

#ifdef OHOS_PLATFORM
#include "surface.h"
#include "surface_delegate.h"
#include "window.h"
#endif

#include "base/memory/referenced.h"
#include "base/utils/noncopyable.h"
#include "core/components_ng/render/render_surface.h"
#ifdef OHOS_PLATFORM
#include "foundation/graphic/graphic_2d/interfaces/inner_api/surface/surface_delegate.h"
#endif

namespace OHOS::Ace::NG {
class RosenRenderSurface : public RenderSurface {
    DECLARE_ACE_TYPE(RosenRenderSurface, NG::RenderSurface)
public:
    RosenRenderSurface() = default;
    ~RosenRenderSurface() override;

    void InitSurface() override;

    void UpdateXComponentConfig() override;

    void* GetNativeWindow() override;

    void SetRenderContext(const RefPtr<RenderContext>& renderContext) override;

    void ConfigSurface(uint32_t surfaceWidth, uint32_t surfaceHeight) override;

    bool IsSurfaceValid() const override;

    void CreateNativeWindow() override;

    void AdjustNativeWindowSize(uint32_t width, uint32_t height) override;

    std::string GetUniqueId() const override;

#ifdef OHOS_PLATFORM
    OHOS::sptr<OHOS::Surface> GetSurface() const
    {
        return producerSurface_;
    }
#endif

    void SetExtSurfaceBounds(int32_t left, int32_t top, int32_t width, int32_t height) override;

private:
#ifdef OHOS_PLATFORM
    OHOS::sptr<OHOS::Surface> producerSurface_ = nullptr;
    struct NativeWindow* nativeWindow_ = nullptr;
    sptr<OHOS::SurfaceDelegate> surfaceDelegate_;
#endif
    WeakPtr<NG::RenderContext> renderContext_ = nullptr;

    ACE_DISALLOW_COPY_AND_MOVE(RosenRenderSurface);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PAINTS_ADAPTER_ROSEN_RENDER_SURFACE_H
