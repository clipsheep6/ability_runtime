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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PAINT_METHOD_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PAINT_METHOD_H

#include "base/geometry/ng/size_t.h"
#include "base/memory/ace_type.h"
#include "base/memory/referenced.h"
#include "base/subwindow/subwindow_manager.h"
#include "base/utils/macros.h"
#include "base/utils/utils.h"
#include "core/components_ng/render/node_paint_method.h"
#include "core/components_v2/list/list_component.h"

namespace OHOS::Ace::NG {
class ACE_EXPORT MenuWrapperPaintMethod : public NodePaintMethod {
    DECLARE_ACE_TYPE(MenuWrapperPaintMethod, NodePaintMethod)
public:
    MenuWrapperPaintMethod() = default;
    ~MenuWrapperPaintMethod() override = default;
    
    CanvasDrawFunction GetOverlayDrawFunction(PaintWrapper* paintWrapper) override;
private:
    float horizonOffsetFirst_;
    float horizonOffsetSecond_;
    float horizonOffsetThird_;
    float horizonOffsetForth_;
    float verticalOffsetFirst_;
    float verticalOffsetSecond_;
    float verticalOffsetThird_;
    void UpdateArrowPath(Placement placement, float arrowX, float arrowY,
        RSPath& path);
    void GetTopPath(float arrowX, float arrowY, RSPath& path);
    void GetBottomPath(float arrowX, float arrowY, RSPath& path);
    void GetRightPath(float arrowX, float arrowY, RSPath& path);
    void GetLeftPath(float arrowX, float arrowY, RSPath& path);
    ACE_DISALLOW_COPY_AND_MOVE(MenuWrapperPaintMethod);

};
} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_MENU_MENU_WRAPPER_PAINT_METHOD_H