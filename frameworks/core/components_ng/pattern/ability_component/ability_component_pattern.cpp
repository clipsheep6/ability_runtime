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

#include "core/components_ng/pattern/ability_component/ability_component_pattern.h"

#include "session/host/include/extension_session.h"
#include "session_manager/include/extension_session_manager.h"

#include "adapter/ohos/entrance/ace_container.h"
#include "adapter/ohos/entrance/mmi_event_convertor.h"
#include "adapter/ohos/osal/want_wrap_ohos.h"
#include "base/utils/utils.h"
#include "core/common/container.h"
#include "core/pipeline_ng/pipeline_context.h"

namespace OHOS::Ace::NG {

void AbilityComponentPattern::OnModifyDone()
{
    auto container = Container::Current();
    if (container && container->IsScenceBoardWindow()) {
        Pattern::OnModifyDone();
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        auto hub = host->GetEventHub<EventHub>();
        CHECK_NULL_VOID(hub);
        auto gestureHub = hub->GetOrCreateGestureEventHub();
        CHECK_NULL_VOID(gestureHub);
        InitTouchEvent(gestureHub);
    }
    if (adapter_) {
        UpdateWindowRect();
    } else {
        auto container = AceType::DynamicCast<Platform::AceContainer>(Container::Current());
        CHECK_NULL_VOID_NOLOG(container);
        auto callerToken = container->GetToken();
        auto pipelineContext = PipelineContext::GetCurrentContext();
        CHECK_NULL_VOID(pipelineContext);
        auto windowId = pipelineContext->GetWindowId();
        auto host = GetHost();
        CHECK_NULL_VOID(host);
        adapter_ = WindowExtensionConnectionProxyNG::CreateAdapter();
        CHECK_NULL_VOID(adapter_);
        auto wantWrap = Ace::WantWrap::CreateWantWrap(bundleName_, abilityName_);
        auto want = AceType::DynamicCast<WantWrapOhos>(wantWrap)->GetWant();
        Rosen::SessionInfo extensionSessionInfo = {
            .bundleName_ = want.GetElement().GetBundleName(),
            .abilityName_ = want.GetElement().GetAbilityName(),
            .callerToken_ = callerToken,
            .want = new (std::nothrow) AAFwk::Want(want),
        };
        session_ = Rosen::ExtensionSessionManager::GetInstance().RequestExtensionSession(extensionSessionInfo);
        sptr<Rosen::ExtensionSession> extensionSession(static_cast<Rosen::ExtensionSession*>(session_.GetRefPtr()));
        adapter_->ConnectExtension(GetHost(), windowId, extensionSession);
        pipelineContext->AddOnAreaChangeNode(host->GetId());
        pipelineContext->AddWindowStateChangedCallback(host->GetId());
        LOGI("connect to windows extension begin %{public}s", GetHost()->GetTag().c_str());
    }
}

void AbilityComponentPattern::FireConnect()
{
    hasConnectionToAbility_ = true;
    UpdateWindowRect();

    auto abilityComponentEventHub = GetEventHub<AbilityComponentEventHub>();
    CHECK_NULL_VOID(abilityComponentEventHub);
    abilityComponentEventHub->FireOnConnect();
}

void AbilityComponentPattern::FireDisConnect()
{
    hasConnectionToAbility_ = false;
    auto abilityComponentEventHub = GetEventHub<AbilityComponentEventHub>();
    CHECK_NULL_VOID(abilityComponentEventHub);
    abilityComponentEventHub->FireOnDisConnect();
}

bool AbilityComponentPattern::OnDirtyLayoutWrapperSwap(
    const RefPtr<LayoutWrapper>& /*dirty*/, const DirtySwapConfig& config)
{
    if (config.frameSizeChange || config.frameOffsetChange) {
        UpdateWindowRect();
    }
    return false;
}

void AbilityComponentPattern::UpdateWindowRect()
{
    if (!hasConnectionToAbility_) {
        LOGD("AbilityComponent has not be connected");
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto size = host->GetGeometryNode()->GetFrameSize();
    auto offset = host->GetTransformRelativeOffset();
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    Rect rect = pipeline->GetDisplayWindowRectInfo();
    rect = Rect(offset.GetX() + rect.Left(), offset.GetY() + rect.Top(), size.Width(), size.Height());

    if (adapter_ && rect != lastRect_) {
        LOGI("ConnectExtension: %{public}f %{public}f %{public}f %{public}f", offset.GetX(), offset.GetY(),
            size.Width(), size.Height());
        adapter_->UpdateRect(rect);
        lastRect_ = rect;
    }
}

void AbilityComponentPattern::OnAreaChangedInner()
{
    UpdateWindowRect();
}

void AbilityComponentPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }
    auto callback = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleTouchEvent(info);
        }
    };
    if (touchEvent_) {
        gestureHub->RemoveTouchEvent(touchEvent_);
    }
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(callback));
    gestureHub->AddTouchEvent(touchEvent_);
}

void AbilityComponentPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    if (info.GetSourceDevice() != SourceType::TOUCH) {
        return;
    }
    const auto pointerEvent = info.GetPointerEvent();
    CHECK_NULL_VOID(pointerEvent);
    auto host = GetHost();
    CHECK_NULL_VOID_NOLOG(host);
    auto selfGlobalOffset = host->GetTransformRelativeOffset();
    auto scale = host->GetTransformScale();
    Platform::CalculatePointerEvent(selfGlobalOffset, pointerEvent, scale);
    WindowPattern::DispatchPointerEvent(pointerEvent);
}

} // namespace OHOS::Ace::NG
