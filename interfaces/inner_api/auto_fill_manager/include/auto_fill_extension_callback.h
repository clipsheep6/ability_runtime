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

#include "fill_request_callback_interface.h"
#include "save_request_callback_interface.h"
#include "ui_content.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace AutoFill {
enum AutoFillWindowType {
    MODAL_WINDOW,
    POPUP_WINDOW
};
}
class AutoFillExtensionCallback : public std::enable_shared_from_this<AutoFillExtensionCallback> {
public:
    AutoFillExtensionCallback() = default;
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
    void SetUIContent(Ace::UIContent *uiContent);
    void SetEventId(uint32_t eventId);
    void SetWindowType(const AutoFill::AutoFillWindowType &autoFillWindowType);
    void SetExtensionType(bool isSmartAutoFill);
    void SetAutoFillType(const AbilityBase::AutoFillType &autoFillType);
    void SetViewData(const AbilityBase::ViewData &viewData);
    AbilityBase::ViewData GetViewData();
    void HandleTimeOut();

private:
    void SendAutoFillSucess(const AAFwk::Want &want);
    void SendAutoFillFailed(int32_t errCode);
    void CloseModalUIExtension();
    void HandleReloadInModal(const AAFwk::WantParams &wantParams);

    std::shared_ptr<IFillRequestCallback> fillCallback_;
    std::shared_ptr<ISaveRequestCallback> saveCallback_;
    int32_t sessionId_;
    Ace::UIContent *uiContent_ = nullptr;
    uint32_t eventId_ = 0;
    AutoFill::AutoFillWindowType autoFillWindowType_ = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    AbilityBase::ViewData viewData_;
    bool isReloadInModal_ = false;
    bool isSmartAutoFill_ = false;
    bool isOnResult_ = false;
    AAFwk::Want want_;
    int32_t errCode_ = 0;
    AbilityBase::AutoFillType autoFillType_ = AbilityBase::AutoFillType::UNSPECIFIED;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H
