/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_COMPONENT_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_COMPONENT_H

#include <string>
#include <utility>

#include "base/geometry/size.h"
#include "base/utils/utils.h"
#include "core/components/box/drag_drop_event.h"
#include "core/components/declaration/common/declaration.h"
#include "core/components/declaration/web/web_client.h"
#include "core/components/declaration/web/web_declaration.h"
#include "core/components/web/resource/web_javascript_value.h"
#include "core/components/web/web_event.h"
#include "core/components/web/web_property.h"
#include "core/components_v2/common/common_def.h"
#include "core/event/key_event.h"
#include "core/focus/focus_node.h"
#include "core/pipeline/base/element.h"

namespace OHOS::Ace {

// A component can show HTML5 webpages.
class ACE_EXPORT WebComponent : public RenderComponent {
    DECLARE_ACE_TYPE(WebComponent, RenderComponent);

public:
    using CreatedCallback = std::function<void()>;
    using ReleasedCallback = std::function<void(bool)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;
    using MethodCall = std::function<void(const std::string&)>;
    using Method = std::string;

    WebComponent() = default;
    explicit WebComponent(const std::string& type);
    ~WebComponent() override = default;

    RefPtr<RenderNode> CreateRenderNode() override;
    RefPtr<Element> CreateElement() override;

    void SetType(const std::string& type)
    {
        type_ = type;
    }

    const std::string& GetType() const
    {
        return type_;
    }

    void SetSrc(const std::string& src)
    {
        declaration_->SetWebSrc(src);
    }

    const std::string& GetSrc() const
    {
        return declaration_->GetWebSrc();
    }

    void SetData(const std::string& data)
    {
        declaration_->SetWebData(data);
    }

    const std::string& GetData() const
    {
        return declaration_->GetWebData();
    }

    void SetPageStartedEventId(const EventMarker& pageStartedEventId)
    {
        declaration_->SetPageStartedEventId(pageStartedEventId);
    }

    const EventMarker& GetPageStartedEventId() const
    {
        return declaration_->GetPageStartedEventId();
    }

    void SetPageFinishedEventId(const EventMarker& pageFinishedEventId)
    {
        declaration_->SetPageFinishedEventId(pageFinishedEventId);
    }

    const EventMarker& GetPageFinishedEventId() const
    {
        return declaration_->GetPageFinishedEventId();
    }

    using OnProgressChangeImpl = std::function<void(const BaseEventInfo* info)>;
    void OnProgressChange(const BaseEventInfo* info) const
    {
        if (onProgressChangeImpl_) {
            onProgressChangeImpl_(info);
        }
    }
    void SetProgressChangeImpl(OnProgressChangeImpl&& onProgressChangeImpl)
    {
        if (onProgressChangeImpl == nullptr) {
            return;
        }
        onProgressChangeImpl_ = std::move(onProgressChangeImpl);
    }

    void SetTitleReceiveEventId(const EventMarker& titleReceiveEventId)
    {
        declaration_->SetTitleReceiveEventId(titleReceiveEventId);
    }

    const EventMarker& GetTitleReceiveEventId() const
    {
        return declaration_->GetTitleReceiveEventId();
    }

    void SetGeolocationHideEventId(const EventMarker& geolocationHideEventId)
    {
        declaration_->SetGeolocationHideEventId(geolocationHideEventId);
    }

    const EventMarker& GetGeolocationHideEventId() const
    {
        return declaration_->GetGeolocationHideEventId();
    }

    void SetGeolocationShowEventId(const EventMarker& geolocationShowEventId)
    {
        declaration_->SetGeolocationShowEventId(geolocationShowEventId);
    }

    const EventMarker& GetGeolocationShowEventId() const
    {
        return declaration_->GetGeolocationShowEventId();
    }

    void SetRequestFocusEventId(const EventMarker& requestFocusEventId)
    {
        declaration_->SetRequestFocusEventId(requestFocusEventId);
    }

    const EventMarker& GetRequestFocusEventId() const
    {
        return declaration_->GetRequestFocusEventId();
    }

    void SetDownloadStartEventId(const EventMarker& downloadStartEventId)
    {
        declaration_->SetDownloadStartEventId(downloadStartEventId);
    }

    const EventMarker& GetDownloadStartEventId() const
    {
        return declaration_->GetDownloadStartEventId();
    }

    void SetPageErrorEventId(const EventMarker& pageErrorEventId)
    {
        declaration_->SetPageErrorEventId(pageErrorEventId);
    }

    const EventMarker& GetPageErrorEventId() const
    {
        return declaration_->GetPageErrorEventId();
    }

    void SetHttpErrorEventId(const EventMarker& httpErrorEventId)
    {
        declaration_->SetHttpErrorEventId(httpErrorEventId);
    }

    const EventMarker& GetHttpErrorEventId() const
    {
        return declaration_->GetHttpErrorEventId();
    }

    void SetMessageEventId(const EventMarker& messageEventId)
    {
        declaration_->SetMessageEventId(messageEventId);
    }

    const EventMarker& GetMessageEventId() const
    {
        return declaration_->GetMessageEventId();
    }

    void SetRenderExitedId(const EventMarker& renderExitedId)
    {
        declaration_->SetRenderExitedId(renderExitedId);
    }

    const EventMarker& GetRenderExitedId() const
    {
        return declaration_->GetRenderExitedId();
    }

    void SetRefreshAccessedHistoryId(const EventMarker& refreshAccessedHistoryId)
    {
        declaration_->SetRefreshAccessedHistoryId(refreshAccessedHistoryId);
    }

    const EventMarker& GetRefreshAccessedHistoryId() const
    {
        return declaration_->GetRefreshAccessedHistoryId();
    }

    void SetResourceLoadId(const EventMarker& resourceLoadId)
    {
        declaration_->SetResourceLoadId(resourceLoadId);
    }

    const EventMarker& GetResourceLoadId() const
    {
        return declaration_->GetResourceLoadId();
    }

    void SetScaleChangeId(const EventMarker& scaleChangeId)
    {
        declaration_->SetScaleChangeId(scaleChangeId);
    }

    const EventMarker& GetScaleChangeId() const
    {
        return declaration_->GetScaleChangeId();
    }

    void SetScrollId(const EventMarker& scrollId)
    {
        declaration_->SetScrollId(scrollId);
    }

    const EventMarker& GetScrollId() const
    {
        return declaration_->GetScrollId();
    }

    void SetPermissionRequestEventId(const EventMarker& permissionRequestEventId)
    {
        declaration_->SetPermissionRequestEventId(permissionRequestEventId);
    }

    const EventMarker& GetPermissionRequestEventId() const
    {
        return declaration_->GetPermissionRequestEventId();
    }

    void SetDeclaration(const RefPtr<WebDeclaration>& declaration)
    {
        if (declaration) {
            declaration_ = declaration;
        }
    }

    RefPtr<WebController> GetController() const
    {
        return webController_;
    }

    void SetWebController(const RefPtr<WebController>& webController)
    {
        webController_ = webController;
    }

    bool GetJsEnabled() const
    {
        return isJsEnabled_;
    }

    void SetJsEnabled(bool isEnabled)
    {
        isJsEnabled_ = isEnabled;
    }

    std::string GetUserAgent() const
    {
        return userAgent_;
    }

    void SetUserAgent(std::string userAgent)
    {
        userAgent_ = std::move(userAgent);
    }

    bool GetContentAccessEnabled() const
    {
        return isContentAccessEnabled_;
    }

    void SetContentAccessEnabled(bool isEnabled)
    {
        isContentAccessEnabled_ = isEnabled;
    }

    bool GetFileAccessEnabled() const
    {
        return isFileAccessEnabled_;
    }

    void SetFileAccessEnabled(bool isEnabled)
    {
        isFileAccessEnabled_ = isEnabled;
    }
    bool GetOnLineImageAccessEnabled() const
    {
        return isOnLineImageAccessEnabled_;
    }

    void SetOnLineImageAccessEnabled(bool isEnabled)
    {
        isOnLineImageAccessEnabled_ = isEnabled;
    }

    bool GetDomStorageAccessEnabled() const
    {
        return isDomStorageAccessEnabled_;
    }

    void SetDomStorageAccessEnabled(bool isEnabled)
    {
        isDomStorageAccessEnabled_ = isEnabled;
    }

    bool GetImageAccessEnabled() const
    {
        return isImageAccessEnabled_;
    }

    void SetImageAccessEnabled(bool isEnabled)
    {
        isImageAccessEnabled_ = isEnabled;
    }

    MixedModeContent GetMixedMode() const
    {
        return mixedContentMode_;
    }

    void SetMixedMode(MixedModeContent mixedModeNum)
    {
        mixedContentMode_ = mixedModeNum;
    }

    bool GetZoomAccessEnabled() const
    {
        return isZoomAccessEnabled_;
    }

    void SetZoomAccessEnabled(bool isEnabled)
    {
        isZoomAccessEnabled_ = isEnabled;
    }

    bool GetGeolocationAccessEnabled() const
    {
        return isGeolocationAccessEnabled_;
    }

    void SetGeolocationAccessEnabled(bool isEnabled)
    {
        isGeolocationAccessEnabled_ = isEnabled;
    }

    WebCacheMode GetCacheMode()
    {
        return cacheMode_;
    }

    void SetCacheMode(WebCacheMode mode)
    {
        cacheMode_ = mode;
    }

    bool GetOverviewModeAccessEnabled() const
    {
        return isOverviewModeAccessEnabled_;
    }

    void SetOverviewModeAccessEnabled(bool isEnabled)
    {
        isOverviewModeAccessEnabled_ = isEnabled;
    }

    bool GetFileFromUrlAccessEnabled() const
    {
        return isFileFromUrlAccessEnabled_;
    }

    void SetFileFromUrlAccessEnabled(bool isEnabled)
    {
        isFileFromUrlAccessEnabled_ = isEnabled;
    }

    bool GetDatabaseAccessEnabled() const
    {
        return isDatabaseAccessEnabled_;
    }

    void SetDatabaseAccessEnabled(bool isEnabled)
    {
        isDatabaseAccessEnabled_ = isEnabled;
    }

    bool GetWebDebuggingAccessEnabled() const
    {
        return isWebDebuggingAccessEnabled_;
    }

    void SetWebDebuggingAccessEnabled(bool isEnabled)
    {
        isWebDebuggingAccessEnabled_ = isEnabled;
    }

    bool GetIsInitialScaleSet() const
    {
        return isInitialScaleSet_;
    }

    float GetInitialScale() const
    {
        return initialScale_;
    }

    void SetInitialScale(float scale)
    {
        initialScale_ = scale;
        isInitialScaleSet_ = true;
    }

    bool GetBackgroundColorEnabled() const
    {
        return isBackgroundColor_;
    }

    int32_t GetBackgroundColor() const
    {
        return backgroundColor_;
    }

    void SetBackgroundColor(int32_t backgroundColor)
    {
        backgroundColor_ = backgroundColor;
        isBackgroundColor_ = true;
    }

    int32_t GetTextZoomRatio() const
    {
        return textZoomRatioNum_;
    }

    void SetTextZoomRatio(int32_t ratio)
    {
        textZoomRatioNum_ = ratio;
    }

    using OnCommonDialogImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnCommonDialog(const BaseEventInfo* info, DialogEventType dialogEventType) const
    {
        if (dialogEventType == DialogEventType::DIALOG_EVENT_ALERT && onAlertImpl_) {
            return onAlertImpl_(info);
        }
        if (dialogEventType == DialogEventType::DIALOG_EVENT_CONFIRM && onConfirmImpl_) {
            return onConfirmImpl_(info);
        }
        if (dialogEventType == DialogEventType::DIALOG_EVENT_PROMPT && onPromptImpl_) {
            return onPromptImpl_(info);
        }
        if (dialogEventType == DialogEventType::DIALOG_EVENT_BEFORE_UNLOAD && onBeforeUnloadImpl_) {
            return onBeforeUnloadImpl_(info);
        }
        return false;
    }
    void SetOnCommonDialogImpl(OnCommonDialogImpl&& onCommonDialogImpl, DialogEventType dialogEventType)
    {
        if (onCommonDialogImpl == nullptr) {
            return;
        }

        switch (dialogEventType) {
            case DialogEventType::DIALOG_EVENT_ALERT:
                onAlertImpl_ = std::move(onCommonDialogImpl);
                break;
            case DialogEventType::DIALOG_EVENT_CONFIRM:
                onConfirmImpl_ = std::move(onCommonDialogImpl);
                break;
            case DialogEventType::DIALOG_EVENT_PROMPT:
                onPromptImpl_ = std::move(onCommonDialogImpl);
                break;
            case DialogEventType::DIALOG_EVENT_BEFORE_UNLOAD:
                onBeforeUnloadImpl_ = std::move(onCommonDialogImpl);
                break;
            default:
                break;
        }
    }

    using OnHttpAuthRequestImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnHttpAuthRequest(const BaseEventInfo* info) const
    {
        if (onHttpAuthRequestImpl_) {
            return onHttpAuthRequestImpl_(info);
        }
        return false;
    }
    void SetOnHttpAuthRequestImpl(OnHttpAuthRequestImpl&& onHttpAuthRequestImpl)
    {
        if (onHttpAuthRequestImpl == nullptr) {
            return;
        }
        onHttpAuthRequestImpl_ = std::move(onHttpAuthRequestImpl);
    }

    using OnSslErrorRequestImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnSslErrorRequest(const BaseEventInfo* info) const
    {
        if (onSslErrorRequestImpl_) {
            return onSslErrorRequestImpl_(info);
        }
        return false;
    }
    void SetOnSslErrorRequestImpl(OnSslErrorRequestImpl && onSslErrorRequestImpl)
    {
        if (onSslErrorRequestImpl == nullptr) {
            return;
        }
        onSslErrorRequestImpl_ = std::move(onSslErrorRequestImpl);
    }

    using OnSslSelectCertRequestImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnSslSelectCertRequest(const BaseEventInfo* info) const
    {
        LOGI("OnSslSelectCertRequest");
        if (onSslSelectCertRequestImpl_) {
            LOGI("web_component.h OnSslSelectCertRequest onSslSelectCertRequestImpl_");
            return onSslSelectCertRequestImpl_(info);
        }
        return false;
    }
    void SetOnSslSelectCertRequestImpl(OnSslSelectCertRequestImpl && impl)
    {
        if (impl == nullptr) {
            return;
        }
        onSslSelectCertRequestImpl_ = std::move(impl);
    }

    void RequestFocus();

    using OnConsoleImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnConsole(const BaseEventInfo* info) const
    {
        if (consoleImpl_) {
            return consoleImpl_(info);
        }
        return false;
    }

    void SetOnConsoleImpl(OnConsoleImpl&& consoleImpl)
    {
        consoleImpl_ = std::move(consoleImpl);
    }

    void SetFocusElement(const WeakPtr<FocusNode>& focusElement)
    {
        focusElement_ = focusElement;
    }

    using OnFileSelectorShowImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnFileSelectorShow(const BaseEventInfo* info) const
    {
        if (onFileSelectorShowImpl_) {
            return onFileSelectorShowImpl_(info);
        }
        return false;
    }
    void SetOnFileSelectorShow(OnFileSelectorShowImpl&& onFileSelectorShowImpl)
    {
        if (onFileSelectorShowImpl == nullptr) {
            return;
        }

        onFileSelectorShowImpl_ = onFileSelectorShowImpl;
    }

    using OnContextMenuImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnContextMenuShow(const BaseEventInfo* info) const
    {
        if (onContextMenuImpl_) {
            return onContextMenuImpl_(info);
        }
        return false;
    }
    void SetOnContextMenuShow(OnContextMenuImpl&& onContextMenuImpl)
    {
        if (onContextMenuImpl == nullptr) {
            return;
        }
        onContextMenuImpl_ = std::move(onContextMenuImpl);
    }

    using OnUrlLoadInterceptImpl = std::function<bool(const BaseEventInfo* info)>;
    bool OnUrlLoadIntercept(const BaseEventInfo* info) const
    {
        if (onUrlLoadInterceptImpl_) {
            return onUrlLoadInterceptImpl_(info);
        }
        return false;
    }
    void SetOnUrlLoadIntercept(OnUrlLoadInterceptImpl&& onUrlLoadInterceptImpl)
    {
        if (onUrlLoadInterceptImpl == nullptr) {
            return;
        }

        onUrlLoadInterceptImpl_ = onUrlLoadInterceptImpl;
    }

    using OnInterceptRequestImpl = std::function<RefPtr<WebResponse>(const BaseEventInfo* info)>;
    RefPtr<WebResponse> OnInterceptRequest(const BaseEventInfo* info) const
    {
        if (onInterceptRequestImpl_) {
            return onInterceptRequestImpl_(info);
        }
        return nullptr;
    }

    bool IsEmptyOnInterceptRequest() const
    {
        return onInterceptRequestImpl_ == nullptr;
    }

    void SetOnInterceptRequest(OnInterceptRequestImpl&& onInterceptRequestImpl)
    {
        if (onInterceptRequestImpl == nullptr) {
            return;
        }
        onInterceptRequestImpl_ = std::move(onInterceptRequestImpl);
    }

    void SetOnMouseEventCallback(const OnMouseCallback& onMouseId)
    {
        onMouseEvent_ = onMouseId;
    }

    OnMouseCallback GetOnMouseEventCallback() const
    {
        return onMouseEvent_;
    }

    void SetOnKeyEventCallback(const OnKeyEventCallback& onKeyEventId)
    {
        onKeyEvent_ = onKeyEventId;
    }

    OnKeyEventCallback GetOnKeyEventCallback() const
    {
        return onKeyEvent_;
    }

    void SetSearchResultReceiveEventId(const EventMarker& searchResultReceiveEventId)
    {
        declaration_->SetSearchResultReceiveEventId(searchResultReceiveEventId);
    }

    const EventMarker& GetSearchResultReceiveEventId() const
    {
        return declaration_->GetSearchResultReceiveEventId();
    }

    void SetMediaPlayGestureAccess(bool isNeedGestureAccess)
    {
        isNeedGestureAccess_ = isNeedGestureAccess;
    }

    bool IsMediaPlayGestureAccess() const
    {
        return isNeedGestureAccess_;
    }

    const OnDragFunc& GetOnDragStartId() const
    {
        return onDragStartId_;
    }

    void SetOnDragStartId(const OnDragFunc& onDragStartId)
    {
        onDragStartId_ = onDragStartId;
    }

    const OnDropFunc& GetOnDragEnterId() const
    {
        return onDragEnterId_;
    }

    void SetOnDragEnterId(const OnDropFunc& onDragEnterId)
    {
        onDragEnterId_ = onDragEnterId;
    }

    const OnDropFunc& GetOnDragMoveId() const
    {
        return onDragMoveId_;
    }

    void SetOnDragMoveId(const OnDropFunc& onDragMoveId)
    {
        onDragMoveId_ = onDragMoveId;
    }

    const OnDropFunc& GetOnDragLeaveId() const
    {
        return onDragLeaveId_;
    }

    void SetOnDragLeaveId(const OnDropFunc& onDragLeaveId)
    {
        onDragLeaveId_ = onDragLeaveId;
    }

    const OnDropFunc& GetOnDropId() const
    {
        return onDropId_;
    }

    void SetOnDropId(const OnDropFunc& onDropId)
    {
        onDropId_ = onDropId;
    }

private:
    RefPtr<WebDeclaration> declaration_;
    CreatedCallback createdCallback_ = nullptr;
    ReleasedCallback releasedCallback_ = nullptr;
    ErrorCallback errorCallback_ = nullptr;
    RefPtr<WebDelegate> delegate_;
    RefPtr<WebController> webController_;
    OnCommonDialogImpl onAlertImpl_;
    OnCommonDialogImpl onConfirmImpl_;
    OnCommonDialogImpl onPromptImpl_;
    OnCommonDialogImpl onBeforeUnloadImpl_;
    OnConsoleImpl consoleImpl_;
    OnFileSelectorShowImpl onFileSelectorShowImpl_;
    OnUrlLoadInterceptImpl onUrlLoadInterceptImpl_;
    OnHttpAuthRequestImpl onHttpAuthRequestImpl_;
    OnSslErrorRequestImpl onSslErrorRequestImpl_;
    OnSslSelectCertRequestImpl onSslSelectCertRequestImpl_;
    OnContextMenuImpl onContextMenuImpl_;
    OnInterceptRequestImpl onInterceptRequestImpl_ = nullptr;
    OnProgressChangeImpl onProgressChangeImpl_ = nullptr;

    std::string type_;
    bool isJsEnabled_ = true;
    bool isContentAccessEnabled_ = true;
    bool isFileAccessEnabled_ = true;
    std::string userAgent_;
    WeakPtr<FocusNode> focusElement_;
    bool isOnLineImageAccessEnabled_ = false;
    bool isDomStorageAccessEnabled_ = false;
    bool isImageAccessEnabled_ = true;
    MixedModeContent mixedContentMode_ = MixedModeContent::MIXED_CONTENT_NEVER_ALLOW;
    bool isZoomAccessEnabled_ = true;
    bool isGeolocationAccessEnabled_ = true;
    bool isOverviewModeAccessEnabled_ = true;
    bool isFileFromUrlAccessEnabled_ = false;
    bool isDatabaseAccessEnabled_ = false;
    int32_t textZoomRatioNum_ = DEFAULT_TEXT_ZOOM_RATIO;
    WebCacheMode cacheMode_ = WebCacheMode::DEFAULT;
    bool isWebDebuggingAccessEnabled_ = false;
    OnMouseCallback onMouseEvent_;
    OnKeyEventCallback onKeyEvent_;
    float initialScale_;
    bool isInitialScaleSet_ = false;
    int32_t backgroundColor_;
    bool isBackgroundColor_ = false;
    bool isNeedGestureAccess_ = true;
    OnDragFunc onDragStartId_;
    OnDropFunc onDragEnterId_;
    OnDropFunc onDragMoveId_;
    OnDropFunc onDragLeaveId_;
    OnDropFunc onDropId_;
};

} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_WEB_WEB_COMPONENT_H
