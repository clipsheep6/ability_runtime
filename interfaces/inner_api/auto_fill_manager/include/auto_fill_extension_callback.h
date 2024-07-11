/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H

#include <atomic>

#include "auto_fill_custom_config.h"
#include "fill_request_callback_interface.h"
#include "save_request_callback_interface.h"
#ifdef SUPPORT_GRAPHICS
#include "ui_content.h"
#endif // SUPPORT_GRAPHICS
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace AutoFill {
enum AutoFillWindowType {
    MODAL_WINDOW,
    POPUP_WINDOW
};
}
#ifdef SUPPORT_GRAPHICS
class AutoFillExtensionCallback : public std::enable_shared_from_this<AutoFillExtensionCallback> {
public:
    AutoFillExtensionCallback();
    ~AutoFillExtensionCallback() = default;

    void OnResult(int32_t errCode, const AAFwk::Want &want);
    void OnRelease(int32_t errCode);
    void OnError(int32_t errCode, const std::string &name, const std::string &message);
    void OnReceive(const AAFwk::WantParams &wantParams);
    void onRemoteReady(const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy);
    void onDestroy();

    void SetFillRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback);
    void SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback);

    void SetSessionId(int32_t sessionId);
    void SetInstanceId(int32_t instanceId);
    int32_t GetInstanceId();
    void SetWindowType(const AutoFill::AutoFillWindowType &autoFillWindowType);
    AutoFill::AutoFillWindowType GetWindowType() const;
    void SetExtensionType(bool isSmartAutoFill);
    void SetAutoFillType(const AbilityBase::AutoFillType &autoFillType);
    void SetViewData(const AbilityBase::ViewData &viewData);
    void SetAutoFillRequestConfig(const AutoFill::AutoFillCustomConfig &config);
    uint32_t GetCallbackId() const;
    void HandleTimeOut();
    void UpdateCustomPopupUIExtension(const AbilityBase::ViewData &viewData);
    void CloseUIExtension();

private:
    void SendAutoFillSuccess(const AAFwk::Want &want);
    void SendAutoFillFailed(int32_t errCode, const AAFwk::Want &want = AAFwk::Want());
    void HandleReloadInModal(const AAFwk::WantParams &wantParams);
    int32_t ReloadInModal(const AAFwk::WantParams &wantParams);
    void UpdateCustomPopupConfig(const AAFwk::WantParams &wantParams);
    void SetModalUIExtensionProxy(const std::shared_ptr<Ace::ModalUIExtensionProxy>& proxy);
    std::shared_ptr<Ace::ModalUIExtensionProxy> GetModalUIExtensionProxy();
    uint32_t GenerateCallbackId();
    Ace::UIContent* GetUIContent();

    std::mutex requestCallbackMutex_;
    std::shared_ptr<IFillRequestCallback> fillCallback_;
    std::shared_ptr<ISaveRequestCallback> saveCallback_;
    int32_t sessionId_ = -1;
    std::atomic<int32_t> instanceId_ {-1};
    uint32_t callbackId_ = 0;
    AutoFill::AutoFillWindowType autoFillWindowType_ = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    AbilityBase::ViewData viewData_;
    AutoFill::AutoFillCustomConfig autoFillCustomConfig_;
    bool isReloadInModal_ = false;
    bool isSmartAutoFill_ = false;
    bool isOnResult_ = false;
    AAFwk::Want want_;
    int32_t errCode_ = 0;
    AbilityBase::AutoFillType autoFillType_ = AbilityBase::AutoFillType::UNSPECIFIED;
    std::mutex proxyMutex_;
    std::shared_ptr<Ace::ModalUIExtensionProxy> modalUIExtensionProxy_;
    std::mutex closeMutex_;
};
#endif // SUPPORT_GRAPHICS
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H
