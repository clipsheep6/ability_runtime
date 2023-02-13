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

#include <gtest/gtest.h>
#include <iremote_object.h>
#include <parcel.h>
#include <pixel_map.h>
#include <refbase.h>
#include "window.h"
#include "wm_common.h"
#include "window_option.h"

namespace OHOS {
namespace Rosen {
using NotifyNativeWinDestroyFunc = std::function<void(std::string windowName)>;
class RSSurfaceNode;
Rect g_Rect;
SystemBarProperty g_SystemBarProperty;
Transform g_Transform;
class MockWindow : public Window {
public:
    MockWindow() = default;
    virtual ~MockWindow() = default;

    virtual WMError SetAPPWindowLabel(const std::string& label) {return WMError::WM_OK;}
    virtual WMError SetAPPWindowIcon(const std::shared_ptr<Media::PixelMap>& icon) {return WMError::WM_OK;}
    virtual std::shared_ptr<RSSurfaceNode> GetSurfaceNode() const {return nullptr;}
    virtual const std::shared_ptr<AbilityRuntime::Context> GetContext() const {return nullptr;}
    virtual Rect GetRect() const {return g_Rect;}
    virtual Rect GetRequestRect() const {return g_Rect;}
    virtual WindowType GetType() const {return WindowType::APP_WINDOW_BASE;}
    virtual WindowMode GetMode() const {return WindowMode::WINDOW_MODE_UNDEFINED;}
    virtual float GetAlpha() const {return 0;}
    virtual const std::string& GetWindowName() const {return "";}
    virtual uint32_t GetWindowId() const {return 0;}
    virtual uint32_t GetWindowFlags() const {return 0;}
    virtual WindowState GetWindowState() const {return WindowState::STATE_INITIAL;}
    virtual WMError SetFocusable(bool isFocusable) {return WMError::WM_OK;}
    virtual bool GetFocusable() const {return false;}
    virtual WMError SetTouchable(bool isTouchable) {return WMError::WM_OK;}
    virtual bool GetTouchable() const {return false;}
    virtual SystemBarProperty GetSystemBarPropertyByType(WindowType type) const {return g_SystemBarProperty;}
    virtual bool IsFullScreen() const {return false;}
    virtual bool IsLayoutFullScreen() const {return false;}
    virtual WMError SetWindowType(WindowType type) {return WMError::WM_OK;}
    virtual WMError SetWindowMode(WindowMode mode) {return WMError::WM_OK;}
    virtual WMError SetAlpha(float alpha) {return WMError::WM_OK;}
    virtual WMError SetTransform(const Transform& trans) {return WMError::WM_OK;}
    virtual const Transform& GetTransform() const {return g_Transform;}
    virtual WMError AddWindowFlag(WindowFlag flag) {return WMError::WM_OK;}
    virtual WMError RemoveWindowFlag(WindowFlag flag) {return WMError::WM_OK;}
    virtual WMError SetWindowFlags(uint32_t flags) {return WMError::WM_OK;}
    virtual WMError SetSystemBarProperty(WindowType type, const SystemBarProperty& property) {return WMError::WM_OK;}
    virtual WMError GetAvoidAreaByType(AvoidAreaType type, AvoidArea& avoidArea) {return WMError::WM_OK;}
    virtual WMError SetLayoutFullScreen(bool status) {return WMError::WM_OK;}
    virtual WMError SetFullScreen(bool status) {return WMError::WM_OK;}
    virtual WMError Destroy() {return WMError::WM_OK;}
    virtual WMError Show(uint32_t reason = 0, bool withAnimation = false) {return WMError::WM_OK;}
    virtual WMError Hide(uint32_t reason = 0, bool withAnimation = false) {return WMError::WM_OK;}
    virtual WMError MoveTo(int32_t x, int32_t y) {return WMError::WM_OK;}
    virtual WMError Resize(uint32_t width, uint32_t height) {return WMError::WM_OK;}
    virtual WMError SetKeepScreenOn(bool keepScreenOn) {return WMError::WM_OK;}
    virtual bool IsKeepScreenOn() const {return false;}
    virtual WMError SetTurnScreenOn(bool turnScreenOn) {return WMError::WM_OK;}
    virtual bool IsTurnScreenOn() const {return false;}
    virtual WMError SetBackgroundColor(const std::string& color) {return WMError::WM_OK;}
    virtual WMError SetTransparent(bool isTransparent) {return WMError::WM_OK;}
    virtual bool IsTransparent() const {return false;}
    virtual WMError SetBrightness(float brightness) {return WMError::WM_OK;}
    virtual float GetBrightness() const {return 0;}
    virtual WMError SetCallingWindow(uint32_t windowId) {return WMError::WM_OK;}
    virtual WMError SetPrivacyMode(bool isPrivacyMode) {return WMError::WM_OK;}
    virtual bool IsPrivacyMode() const {return false;}
    virtual void SetSystemPrivacyMode(bool isSystemPrivacyMode) {}
    virtual WMError BindDialogTarget(sptr<IRemoteObject> targetToken) {return WMError::WM_OK;}
    virtual WMError SetSnapshotSkip(bool isSkip) {return WMError::WM_OK;}
    virtual WMError SetCornerRadius(float cornerRadius) {return WMError::WM_OK;}
    virtual WMError SetShadowRadius(float radius) {return WMError::WM_OK;}
    virtual WMError SetShadowColor(std::string color) {return WMError::WM_OK;}
    virtual WMError SetShadowOffsetX(float offsetX) {return WMError::WM_OK;}
    virtual WMError SetShadowOffsetY(float offsetY) {return WMError::WM_OK;}
    virtual WMError SetBlur(float radius) {return WMError::WM_OK;}
    virtual WMError SetBackdropBlur(float radius) {return WMError::WM_OK;}
    virtual WMError SetBackdropBlurStyle(WindowBlurStyle blurStyle) {return WMError::WM_OK;}
    virtual WMError RequestFocus() const {return WMError::WM_OK;}
    virtual bool IsFocused() const {return false;}
    virtual WMError UpdateSurfaceNodeAfterCustomAnimation(bool isAdd) {return WMError::WM_OK;}
    virtual void SetInputEventConsumer(const std::shared_ptr<IInputEventConsumer>& inputEventConsumer) {}
    virtual void ConsumeKeyEvent(std::shared_ptr<MMI::KeyEvent>& inputEvent) {}
    virtual void ConsumePointerEvent(const std::shared_ptr<MMI::PointerEvent>& inputEvent) {}
    virtual void RequestVsync(const std::shared_ptr<VsyncCallback>& vsyncCallback) {}
    virtual void UpdateConfiguration(const std::shared_ptr<AppExecFwk::Configuration>& configuration) {}
    virtual WMError RegisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterLifeCycleListener(const sptr<IWindowLifeCycle>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterWindowChangeListener(const sptr<IWindowChangeListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterAvoidAreaChangeListener(sptr<IAvoidAreaChangedListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterDragListener(const sptr<IWindowDragListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterDragListener(const sptr<IWindowDragListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterDisplayMoveListener(sptr<IDisplayMoveListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual void RegisterWindowDestroyedListener(const NotifyNativeWinDestroyFunc& func) {}
    virtual WMError RegisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterOccupiedAreaChangeListener(const sptr<IOccupiedAreaChangeListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterTouchOutsideListener(const sptr<ITouchOutsideListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterAnimationTransitionController(const sptr<IAnimationTransitionController>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterScreenshotListener(const sptr<IScreenshotListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterScreenshotListener(const sptr<IScreenshotListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError RegisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual WMError UnregisterDialogTargetTouchListener(const sptr<IDialogTargetTouchListener>& listener) {return WMError::WM_ERROR_NULLPTR;}
    virtual void RegisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}
    virtual void UnregisterDialogDeathRecipientListener(const sptr<IDialogDeathRecipientListener>& listener) {}
    virtual void NotifyTouchDialogTarget() {}
    virtual void SetAceAbilityHandler(const sptr<IAceAbilityHandler>& handler) {}
    virtual WMError SetUIContent(const std::string& contentInfo, NativeEngine* engine,
        NativeValue* storage, bool isDistributed = false, AppExecFwk::Ability* ability = nullptr) {return WMError::WM_OK;}
    virtual std::string GetContentInfo() {return "";}
    virtual Ace::UIContent* GetUIContent() const {return nullptr;}
    virtual void OnNewWant(const AAFwk::Want& want) {}
    virtual void SetRequestedOrientation(Orientation) {}
    virtual Orientation GetRequestedOrientation() {return Orientation::BEGIN;}
    virtual void SetRequestModeSupportInfo(uint32_t modeSupportInfo) {}
    virtual uint32_t GetRequestModeSupportInfo() const {return 0;}
    virtual WMError SetTouchHotAreas(const std::vector<Rect>& rects) {return WMError::WM_OK;}
    virtual void GetRequestedTouchHotAreas(std::vector<Rect>& rects) const {}
    virtual bool IsMainHandlerAvailable() const {return false;}
    virtual WMError DisableAppWindowDecor() {return WMError::WM_OK;}
    virtual bool IsDecorEnable() const {return false;}
    virtual WMError Maximize() {return WMError::WM_OK;}
    virtual WMError Minimize() {return WMError::WM_OK;}
    virtual WMError Recover() {return WMError::WM_OK;}
    virtual WMError Close() {return WMError::WM_OK;}
    virtual void StartMove() {}
    virtual void SetNeedRemoveWindowInputChannel(bool needRemoveWindowInputChannel) {}
    virtual bool IsSupportWideGamut() {return false;}
    virtual void SetColorSpace(ColorSpace colorSpace) {}
    virtual ColorSpace GetColorSpace() {return ColorSpace::COLOR_SPACE_DEFAULT;}
    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) {}
    virtual std::shared_ptr<Media::PixelMap> Snapshot() {return nullptr;}
    virtual WMError NotifyMemoryLevel(int32_t level) const {return WMError::WM_OK;}
    virtual bool IsAllowHaveSystemSubWindow() {return false;}
    virtual WmErrorCode RaiseToAppTop() {return WmErrorCode::WM_OK;}
    virtual WMError SetAspectRatio(float ratio) {return WMError::WM_OK;}
    virtual WMError ResetAspectRatio() {return WMError::WM_OK;}
};
}
}
