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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_JS_INSPECTOR_MANAGER_H
#define FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_JS_INSPECTOR_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/pipeline/pipeline_context.h"
#include "frameworks/bridge/common/accessibility/js_accessibility_manager.h"

namespace OHOS::Ace::Framework {

class JsInspectorManager : public AccessibilityNodeManager {
    DECLARE_ACE_TYPE(JsInspectorManager, AccessibilityNodeManager);

public:
    JsInspectorManager() = default;
    ~JsInspectorManager() override = default;

    void InitializeCallback() override;

private:
    void AssembleJSONTree(std::string& jsonStr);
    void AssembleDefaultJSONTree(std::string& jsonStr);
    void GetNodeJSONStrMap();
    void GetAttrsAndStyles(std::unique_ptr<JsonValue>& jsonNode, const RefPtr<AccessibilityNode>& node);
    void GetAttrsAndStylesV2(std::unique_ptr<JsonValue>& jsonNode, const RefPtr<AccessibilityNode>& node);
    void ClearContainer();
    std::string UpdateNodeRectStrInfo(const RefPtr<AccessibilityNode> node);
    std::string UpdateNodeRectStrInfoV2(const RefPtr<AccessibilityNode> node);
    void DumpNodeTreeInfo(int32_t depth, NodeId nodeID);
    void GetChildrenJSONArray(
            int32_t depth, RefPtr<AccessibilityNode> node, std::unique_ptr<JsonValue>& childJSONArray);
    std::string ConvertStrToPropertyType(const std::string& typeValue);
    std::string ConvertPseudoClassStyle(const std::string pseudoClassValue);

    std::vector<std::pair<int32_t, int32_t>> depthNodeIdVec_;
    std::unordered_map<int32_t, std::vector<int32_t>> depthNodeIdMap_;
    std::unordered_map<int32_t, std::vector<std::pair<int32_t, std::string>>> nodeJSONInfoMap_;
};

} // namespace OHOS::Ace::Framework

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_INSPECTOR_JS_INSPECTOR_MANAGER_H
