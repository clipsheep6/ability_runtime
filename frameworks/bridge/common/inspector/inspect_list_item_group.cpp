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

#include "frameworks/bridge/common/inspector/inspect_list_item_group.h"

#include "frameworks/bridge/common/utils/utils.h"

namespace OHOS::Ace::Framework {
InspectListItemGroup::InspectListItemGroup(NodeId nodeId, const std::string& nodeName)
    : InspectNode(nodeId, nodeName) {}

void InspectListItemGroup::PackAttrAndStyle()
{
    // add for the attrs
    attrs_.insert(std::make_pair("type", "default"));
    attrs_.insert(std::make_pair("disabled", "false"));
    attrs_.insert(std::make_pair("focusable", "true"));

    // add for the styles
    styles_.insert(std::make_pair("flex-direction", "row"));
    styles_.insert(std::make_pair("justify-content", "flex-start"));
    styles_.erase("position");
}
} // namespace OHOS::Ace::Framework
