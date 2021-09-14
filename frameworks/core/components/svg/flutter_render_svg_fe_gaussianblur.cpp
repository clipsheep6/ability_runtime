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

#include "frameworks/core/components/svg/flutter_render_svg_fe_gaussianblur.h"

#include "third_party/skia/include/effects/SkBlurImageFilter.h"

namespace OHOS::Ace {

RefPtr<RenderNode> RenderSvgFeGaussianBlur::Create()
{
    return AceType::MakeRefPtr<FlutterRenderSvgFeGaussianBlur>();
}

void FlutterRenderSvgFeGaussianBlur::OnAsImageFilter(sk_sp<SkImageFilter>& imageFilter) const
{
    imageFilter = SkBlurImageFilter::Make(deviationX_, deviationY_, imageFilter, nullptr);
}

} // namespace OHOS::Ace
