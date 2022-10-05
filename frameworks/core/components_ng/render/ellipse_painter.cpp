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

#include "core/components_ng/render/ellipse_painter.h"

#include "base/geometry/ng/rect_t.h"
#include "core/components_ng/render/canvas_image.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/shape_painter.h"

namespace OHOS::Ace::NG {
void EllipsePainter::DrawEllipse(RSCanvas& canvas, const RectF& rect, const ShapePaintProperty& shapePaintProperty)
{
    RSPen pen;
    RSBrush brush;
    ShapePainter::SetPan(pen, shapePaintProperty);
    ShapePainter::SetBrush(brush, shapePaintProperty);
    canvas.AttachPen(pen);
    canvas.AttachBrush(brush);
    canvas.DrawOval(RSRRect(rect.GetX(), rect.GetY(), rect.GetX() + rect.Width(), rect.GetY() + rect.Height()));
}
} // namespace OHOS::Ace::NG