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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCORING_RENDER_SCORING_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCORING_RENDER_SCORING_H

#include "core/components/proxy/render_proxy.h"
#include "core/components/scoring/scoring_component.h"

namespace OHOS::Ace {

class RenderScoring final : public RenderProxy {
    DECLARE_ACE_TYPE(RenderScoring, RenderProxy);

public:
    RenderScoring() = default;
    ~RenderScoring() override = default;

    static RefPtr<RenderNode> Create();

    void Update(const RefPtr<Component>& component) override;
    void PerformLayout() override;
    void Paint(RenderContext& context, const Offset& offset) override;

    void SetPageName(const std::string& pageName)
    {
        pageName_ = pageName;
    }

private:
    std::string name_;
    std::string pageName_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_SCORING_RENDER_SCORING_H