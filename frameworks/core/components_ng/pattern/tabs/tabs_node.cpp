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

#include "core/components_ng/pattern/tabs/tabs_node.h"

namespace OHOS::Ace::NG {

void TabsNode::AddChildToGroup(const RefPtr<UINode>& child)
{
    if (swiperChildren_.find(child->GetId()) != swiperChildren_.end()) {
        LOGW("Child has already exist.");
        return;
    }

    swiperChildren_.emplace(child->GetId());
    auto swiperNode = GetChildren().back();
    if (swiperNode) {
        child->MountToParent(swiperNode);
    }
}

} // namespace OHOS::Ace::NG