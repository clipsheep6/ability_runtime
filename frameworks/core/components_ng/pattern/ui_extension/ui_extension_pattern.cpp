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

#include "core/components_ng/pattern/ui_extension/ui_extension_pattern.h"

#include "session/host/include/extension_session.h"
#include "session_manager/include/extension_session_manager.h"
#include "ui/rs_surface_node.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "core/components_ng/pattern/ui_extension/ui_extension_layout_algorithm.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {
UIExtensionPattern::UIExtensionPattern(const std::string& bundleName, const std::string& abilityName)
{
    auto container = AceType::DynamicCast<Platform::AceContainer>(Container::Current());
    CHECK_NULL_VOID(container);
    auto callerToken = container->GetToken();

    Rosen::ExtensionSessionManager::GetInstance().Init();
    Rosen::SessionInfo extensionSessionInfo = {
        .bundleName_ = bundleName,
        .abilityName_ = abilityName,
        .callerToken_ = callerToken,
    };
    session_ = Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSession(extensionSessionInfo);
    RegisterLifecycleListener();
    RequestExtensionSessionActivation();
}

UIExtensionPattern::~UIExtensionPattern()
{
    UnregisterLifecycleListener();
    RequestExtensionSessionDestruction();
}

void UIExtensionPattern::OnConnect()
{
    WindowPattern::OnConnect();
    CHECK_NULL_VOID(session_);
    auto surfaceNode = session_->GetSurfaceNode();
    CHECK_NULL_VOID(surfaceNode);
    surfaceNode->CreateNodeInRenderThread();
}

void UIExtensionPattern::OnWindowShow()
{
    RequestExtensionSessionActivation();
}

void UIExtensionPattern::OnWindowHide()
{
    RequestExtensionSessionBackground();
}

void UIExtensionPattern::RequestExtensionSessionActivation()
{
    CHECK_NULL_VOID(session_);
    sptr<Rosen::ExtensionSession> extensionSession(static_cast<Rosen::ExtensionSession*>(session_.GetRefPtr()));
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionActivation(extensionSession);
}

void UIExtensionPattern::RequestExtensionSessionBackground()
{
    CHECK_NULL_VOID(session_);
    sptr<Rosen::ExtensionSession> extensionSession(static_cast<Rosen::ExtensionSession*>(session_.GetRefPtr()));
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionBackground(extensionSession);
}

void UIExtensionPattern::RequestExtensionSessionDestruction()
{
    CHECK_NULL_VOID(session_);
    sptr<Rosen::ExtensionSession> extensionSession(static_cast<Rosen::ExtensionSession*>(session_.GetRefPtr()));
    Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSessionDestruction(extensionSession);
}

RefPtr<LayoutAlgorithm> UIExtensionPattern::CreateLayoutAlgorithm()
{
    return MakeRefPtr<UIExtensionLayoutAlgorithm>();
}

void UIExtensionPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID_NOLOG(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
}
} // namespace OHOS::Ace::NG
