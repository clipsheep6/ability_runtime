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

#include "core/components_ng/pattern/window_scene/scene/window_pattern.h"

#include "session_manager/include/scene_session_manager.h"
#include "ui/rs_surface_node.h"

#include "adapter/ohos/entrance/mmi_event_convertor.h"
#include "base/utils/system_properties.h"
#include "core/common/container.h"
#include "core/common/container_scope.h"
#include "core/components_ng/pattern/image/image_pattern.h"
#include "core/components_ng/render/adapter/rosen_render_context.h"
#include "core/components_v2/inspector/inspector_constants.h"
#include "core/components_ng/pattern/window_scene/scene/window_event_process.h"

namespace OHOS::Ace::NG {
namespace {
constexpr uint32_t COLOR_BLACK = 0xff000000;
constexpr uint32_t COLOR_WHITE = 0xffffffff;
} // namespace

class LifecycleListener : public Rosen::ILifecycleListener {
public:
    explicit LifecycleListener(const WeakPtr<WindowPattern>& windowPattern) : windowPattern_(windowPattern) {}
    virtual ~LifecycleListener() = default;

    void OnConnect() override
    {
        auto windowPattern = windowPattern_.Upgrade();
        CHECK_NULL_VOID(windowPattern);
        windowPattern->OnConnect();
    }

    void OnForeground() override
    {
        auto windowPattern = windowPattern_.Upgrade();
        CHECK_NULL_VOID(windowPattern);
        windowPattern->OnForeground();
    }

    void OnBackground() override
    {
        auto windowPattern = windowPattern_.Upgrade();
        CHECK_NULL_VOID(windowPattern);
        windowPattern->OnBackground();
    }

    void OnDisconnect() override
    {
        auto windowPattern = windowPattern_.Upgrade();
        CHECK_NULL_VOID(windowPattern);
        windowPattern->OnDisconnect();
    }

private:
    WeakPtr<WindowPattern> windowPattern_;
};

WindowPattern::WindowPattern()
{
    instanceId_ = Container::CurrentId();
}

void WindowPattern::RegisterLifecycleListener()
{
    CHECK_NULL_VOID(session_);
    lifecycleListener_ = std::make_shared<LifecycleListener>(WeakClaim(this));
    session_->RegisterLifecycleListener(lifecycleListener_);
}

void WindowPattern::UnregisterLifecycleListener()
{
    CHECK_NULL_VOID(session_);
    session_->UnregisterLifecycleListener(lifecycleListener_);
}

void WindowPattern::InitContent()
{
    contentNode_ = FrameNode::CreateFrameNode(
        V2::WINDOW_SCENE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<Pattern>());
    contentNode_->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    contentNode_->SetHitTestMode(HitTestMode::HTMNONE);
    CHECK_NULL_VOID(session_);
    auto surfaceNode = session_->GetSurfaceNode();
    if (surfaceNode) {
        auto context = AceType::DynamicCast<NG::RosenRenderContext>(contentNode_->GetRenderContext());
        CHECK_NULL_VOID(context);
        context->SetRSNode(surfaceNode);
    }

    auto state = session_->GetSessionState();
    LOGI("Session state is %{public}u.", state);
    switch (state) {
        case Rosen::SessionState::STATE_DISCONNECT: {
            CreateStartingNode();
            break;
        }
        case Rosen::SessionState::STATE_BACKGROUND: {
            CreateSnapshotNode();
            break;
        }
        default: {
            auto host = GetHost();
            CHECK_NULL_VOID(host);
            host->AddChild(contentNode_);
            break;
        }
    }
}

void WindowPattern::CreateStartingNode()
{
    if (!HasStartingPage() || CreatePersistentNode()) {
        return;
    }

    auto host = GetHost();
    CHECK_NULL_VOID(host);

    startingNode_ = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto imageLayoutProperty = startingNode_->GetLayoutProperty<ImageLayoutProperty>();
    imageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    host->AddChild(startingNode_);

    startingNode_->SetHitTestMode(HitTestMode::HTMNONE);
    std::string startPagePath;
    auto backgroundColor = SystemProperties::GetColorMode() == ColorMode::DARK ? COLOR_BLACK : COLOR_WHITE;
    auto sessionInfo = session_->GetSessionInfo();
    Rosen::SceneSessionManager::GetInstance().GetStartPage(sessionInfo, startPagePath, backgroundColor);
    LOGI("start page path %{public}s, background color %{public}x", startPagePath.c_str(), backgroundColor);

    startingNode_->GetRenderContext()->UpdateBackgroundColor(Color(backgroundColor));
    imageLayoutProperty->UpdateImageSourceInfo(
        ImageSourceInfo(startPagePath, sessionInfo.bundleName_, sessionInfo.moduleName_));
    imageLayoutProperty->UpdateImageFit(ImageFit::NONE);
    startingNode_->MarkModifyDone();
}

void WindowPattern::CreateSnapshotNode()
{
    snapshotNode_ = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto imageLayoutProperty = snapshotNode_->GetLayoutProperty<ImageLayoutProperty>();
    imageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto backgroundColor = SystemProperties::GetColorMode() == ColorMode::DARK ? COLOR_BLACK : COLOR_WHITE;
    snapshotNode_->GetRenderContext()->UpdateBackgroundColor(Color(backgroundColor));

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->AddChild(snapshotNode_);

    CHECK_NULL_VOID(session_);
    auto snapshot = session_->GetSnapshot();
    auto pixelMap = PixelMap::CreatePixelMap(&snapshot);

    CHECK_NULL_VOID(pixelMap);
    imageLayoutProperty->UpdateImageSourceInfo(ImageSourceInfo(pixelMap));
    imageLayoutProperty->UpdateImageFit(ImageFit::FILL);
    snapshotNode_->MarkModifyDone();
}

void WindowPattern::OnConnect()
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(session_);
    auto surfaceNode = session_->GetSurfaceNode();
    CHECK_NULL_VOID(surfaceNode);

    CHECK_NULL_VOID(contentNode_);
    auto context = AceType::DynamicCast<NG::RosenRenderContext>(contentNode_->GetRenderContext());
    CHECK_NULL_VOID(context);
    context->SetRSNode(surfaceNode);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->AddChild(contentNode_, 0);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);

    if (!HasStartingPage()) {
        return;
    }
    surfaceNode->SetBufferAvailableCallback([weak = WeakClaim(this)]() {
        LOGI("RSSurfaceNode buffer available callback");
        auto windowPattern = weak.Upgrade();
        CHECK_NULL_VOID(windowPattern);
        windowPattern->BufferAvailableCallback();
    });
}

void WindowPattern::BufferAvailableCallback()
{
    ContainerScope scope(instanceId_);

    auto host = GetHost();
    CHECK_NULL_VOID(host);

    host->RemoveChild(startingNode_);
    startingNode_.Reset();
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

void WindowPattern::OnAttachToFrameNode()
{
    InitContent();

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->MarkDirtyNode(PROPERTY_UPDATE_MEASURE);
}

bool WindowPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    CHECK_NULL_RETURN(dirty, false);
    auto host = dirty->GetHostNode();
    CHECK_NULL_RETURN(host, false);
    auto globalOffsetWithTranslate = host->GetPaintRectGlobalOffsetWithTranslate();
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_RETURN(geometryNode, false);
    auto frameRect = geometryNode->GetFrameRect();
    Rosen::WSRect windowRect {
        .posX_ = std::round(globalOffsetWithTranslate.GetX()),
        .posY_ = std::round(globalOffsetWithTranslate.GetY()),
        .width_ = std::round(frameRect.Width()),
        .height_ = std::round(frameRect.Height())
    };

    CHECK_NULL_RETURN(session_, false);
    session_->UpdateRect(windowRect, Rosen::SizeChangeReason::UNDEFINED);
    return false;
}

void WindowPattern::DispatchPointerEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    CHECK_NULL_VOID(session_);
    CHECK_NULL_VOID(pointerEvent);
    session_->TransferPointerEvent(pointerEvent);
}

void WindowPattern::DispatchKeyEvent(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    CHECK_NULL_VOID(session_);
    session_->TransferKeyEvent(keyEvent);
}

bool WindowPattern::CreatePersistentNode()
{
    CHECK_NULL_RETURN(session_, false);
    if (session_->GetScenePersistence() == nullptr || !session_->GetScenePersistence()->IsSnapshotExisted()) {
        return false;
    }
    startingNode_ = FrameNode::CreateFrameNode(
        V2::IMAGE_ETS_TAG, ElementRegister::GetInstance()->MakeUniqueId(), AceType::MakeRefPtr<ImagePattern>());
    auto imageLayoutProperty = startingNode_->GetLayoutProperty<ImageLayoutProperty>();
    imageLayoutProperty->UpdateMeasureType(MeasureType::MATCH_PARENT);

    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    host->AddChild(startingNode_);
    imageLayoutProperty->UpdateImageSourceInfo(
        ImageSourceInfo(std::string("file:/").append(session_->GetScenePersistence()->GetSnapshotFilePath())));
    imageLayoutProperty->UpdateImageFit(ImageFit::COVER);
    startingNode_->MarkModifyDone();
    return true;
}

void WindowPattern::DispatchKeyEventForConsumed(const std::shared_ptr<MMI::KeyEvent>& keyEvent, bool& isConsumed)
{
    CHECK_NULL_VOID(session_);
    session_->TransferKeyEventForConsumed(keyEvent, isConsumed);
}

void WindowPattern::DisPatchFocusActiveEvent(bool isFocusActive)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFocusActiveEvent(isFocusActive);
}

void WindowPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
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

void WindowPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (mouseEvent_) {
        return;
    }
    auto callback = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        if (pattern) {
            pattern->HandleMouseEvent(info);
        }
    };
    if (mouseEvent_) {
        inputHub->RemoveOnMouseEvent(mouseEvent_);
    }
    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(callback));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void WindowPattern::HandleTouchEvent(const TouchEventInfo& info)
{
    LOGD("WindowPattern HandleTouchEvent enter");
    const auto pointerEvent = info.GetPointerEvent();
    CHECK_NULL_VOID(pointerEvent);
    if (IsFilterTouchEvent(pointerEvent)) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID_NOLOG(host);
    auto selfGlobalOffset = host->GetTransformRelativeOffset();
    auto scale = host->GetTransformScale();
    Platform::CalculateWindowCoordinate(selfGlobalOffset, pointerEvent, scale);
    DispatchPointerEvent(pointerEvent);
}

bool WindowPattern::IsFilterTouchEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    return pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE &&
        ((pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) ||
        (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) ||
        (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_UP) ||
        (pointerEvent->GetButtonId() == MMI::PointerEvent::BUTTON_NONE));
}

void WindowPattern::HandleMouseEvent(const MouseInfo& info)
{
    LOGD("WindowPattern HandleMouseEvent enter");
    const auto pointerEvent = info.GetPointerEvent();
    CHECK_NULL_VOID(pointerEvent);
    if (IsFilterMouseEvent(pointerEvent)) {
        return;
    }
    auto host = GetHost();
    CHECK_NULL_VOID_NOLOG(host);
    auto selfGlobalOffset = host->GetTransformRelativeOffset();
    auto scale = host->GetTransformScale();
    Platform::CalculateWindowCoordinate(selfGlobalOffset, pointerEvent, scale);
    int32_t action = pointerEvent->GetPointerAction();
    if (action == MMI::PointerEvent::POINTER_ACTION_MOVE &&
        pointerEvent->GetButtonId() == MMI::PointerEvent::BUTTON_NONE) {
        DelayedSingleton<WindowEventProcess>::GetInstance()->ProcessWindowMouseEvent(
            AceType::DynamicCast<WindowNode>(host), pointerEvent);
    }
    if (action == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
        DelayedSingleton<WindowEventProcess>::GetInstance()->ProcessWindowDragEvent(
            AceType::DynamicCast<WindowNode>(host), pointerEvent);
    }
    DispatchPointerEvent(pointerEvent);
}

bool WindowPattern::IsFilterMouseEvent(const std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    int32_t pointerAction = pointerEvent->GetPointerAction();
    if ((pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (pointerAction != MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) &&
        (pointerAction != MMI::PointerEvent::POINTER_ACTION_PULL_UP)) {
        return true;
    }
    return pointerEvent->GetButtonId() != MMI::PointerEvent::BUTTON_NONE &&
        (pointerAction == MMI::PointerEvent::POINTER_ACTION_MOVE ||
        pointerAction == MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
}

void WindowPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto hub = host->GetEventHub<EventHub>();
    CHECK_NULL_VOID(hub);
    auto gestureHub = hub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);
    InitTouchEvent(gestureHub);
    auto inputHub = hub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    InitMouseEvent(inputHub);
}

void WindowPattern::TransferFocusWindowId(uint32_t focusWindowId)
{
    CHECK_NULL_VOID(session_);
    session_->TransferFocusWindowIdEvent(focusWindowId);
}
} // namespace OHOS::Ace::NG
