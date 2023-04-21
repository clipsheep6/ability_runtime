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

#include "frameworks/core/components_ng/svg/parse/svg_fe.h"

#include "include/effects/SkColorFilterImageFilter.h"
#include "include/effects/SkColorMatrix.h"

#include "base/utils/utils.h"

namespace OHOS::Ace::NG {

void InitFilterColor(const RefPtr<SvgFeDeclaration>& fe, ColorInterpolationType& currentColor)
{
    CHECK_NULL_VOID_NOLOG(fe);

    if (fe->GetIn() == FeInType::SOURCE_GRAPHIC) {
        currentColor = ColorInterpolationType::SRGB;
    } else {
        currentColor = fe->GetColorInterpolationType();
    }
}

SvgFe::SvgFe() : SvgNode()
{
    InitNoneFlag();
}

void SvgFe::GetImageFilter(sk_sp<SkImageFilter>& imageFilter, ColorInterpolationType& currentColor)
{
    ColorInterpolationType srcColor = currentColor;
    InitFilterColor(AceType::DynamicCast<SvgFeDeclaration>(declaration_), currentColor);
    OnAsImageFilter(imageFilter, srcColor, currentColor);
    currentColor = srcColor;
}

void SvgFe::ConverImageFilterColor(
    sk_sp<SkImageFilter>& imageFilter, const ColorInterpolationType& srcColor, const ColorInterpolationType& dst)
{
    if (dst == ColorInterpolationType::LINEAR_RGB && srcColor == ColorInterpolationType::SRGB) {
#ifdef USE_SYSTEM_SKIA
        imageFilter = SkColorFilterImageFilter::Make(SkColorFilter::MakeSRGBToLinearGamma(), imageFilter);
#else
        imageFilter = SkColorFilterImageFilter::Make(SkColorFilters::SRGBToLinearGamma(), imageFilter);
#endif
    } else if (dst == ColorInterpolationType::SRGB && srcColor == ColorInterpolationType::LINEAR_RGB) {
#ifdef USE_SYSTEM_SKIA
        imageFilter = SkColorFilterImageFilter::Make(SkColorFilter::MakeLinearToSRGBGamma(), imageFilter);
#else
        imageFilter = SkColorFilterImageFilter::Make(SkColorFilters::LinearToSRGBGamma(), imageFilter);
#endif
    }
}

sk_sp<SkImageFilter> SvgFe::MakeImageFilter(const FeInType& in, sk_sp<SkImageFilter>& imageFilter)
{
    switch (in) {
        case FeInType::SOURCE_GRAPHIC:
            return nullptr;
        case FeInType::SOURCE_ALPHA:
            SkColorMatrix m;
            m.setScale(0, 0, 0, 1.0f);
#ifdef USE_SYSTEM_SKIA
            return SkColorFilterImageFilter::Make(SkColorFilter::MakeMatrixFilterRowMajor255(m.fMat), nullptr);
#else
            return SkColorFilterImageFilter::Make(SkColorFilters::Matrix(m), nullptr);
#endif
        case FeInType::BACKGROUND_IMAGE:
            break;
        case FeInType::BACKGROUND_ALPHA:
            break;
        case FeInType::FILL_PAINT:
            break;
        case FeInType::STROKE_PAINT:
            break;
        case FeInType::PRIMITIVE:
            break;
        default:
            break;
    }
    return imageFilter;
}

} // namespace OHOS::Ace::NG
