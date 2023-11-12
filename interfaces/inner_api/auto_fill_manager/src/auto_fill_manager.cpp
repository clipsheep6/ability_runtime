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

#include "auto_fill_manager.h"

#include "auto_fill_error.h"
#include "auto_fill_extension_callback.h"
#include "extension_ability_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string WANT_PARAMS_AUTO_FILL_CMD = "fill";
const std::string WANT_PARAMS_AUTO_SAVE_CMD = "save";
const std::string WANT_PARAMS_EXTENSION_TYPE = "autoFill/password";
constexpr static char AUTO_FILL_BUNDLE_NAME[] = "com.ohos.passwordbox";
constexpr static char AUTO_FILL_MODULE_NAME[] = "entry";
constexpr static char AUTO_FILL_ABILITY_NAME[] = "AutoFillAbility";
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr static char WANT_PARAMS_AUTO_FILL_CMD_KEY[] = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_EXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr static char WANT_PARAMS_AUTO_FILL_TYPE_KEY[] = "ability.want.params.AutoFillType";
} // namespace
AutoFillManager &AutoFillManager::GetInstance()
{
    static AutoFillManager instance;
    return instance;
}

int32_t AutoFillManager::RequestAutoFill(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<IFillRequestCallback> &fillCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr || fillCallback == nullptr) {
        HILOG_ERROR("UIContent or fillCallback is nullptr.");
        return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
    }

    AAFwk::Want want;
    want.SetElementName(AUTO_FILL_BUNDLE_NAME, AUTO_FILL_ABILITY_NAME);
    want.SetModuleName(AUTO_FILL_MODULE_NAME);
    want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_EXTENSION_TYPE);
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(autoFillType));
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, WANT_PARAMS_AUTO_FILL_CMD);
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, viewdata.ToJsonString());

    auto extensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (extensionCallback == nullptr) {
        HILOG_ERROR("Extension callback is nullptr.");
        return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
    }
    extensionCallback->SetFillRequestCallback(fillCallback);

    Ace::ModalUIExtensionCallbacks callback;
    callback.onResult = std::bind(
        &AutoFillExtensionCallback::OnResult, extensionCallback, std::placeholders::_1, std::placeholders::_2);
    callback.onRelease = std::bind(
        &AutoFillExtensionCallback::OnRelease, extensionCallback, std::placeholders::_1);
    callback.onError = std::bind(&AutoFillExtensionCallback::OnError,
        extensionCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    Ace::ModalUIExtensionConfig config;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("Create modal ui extension is failed.");
        return AutoFiil::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    extensionCallback->SetUIContent(uiContent);
    extensionCallback->SetSessionId(sessionId);
    return AutoFiil::AUTO_FILL_SUCCESS;
}

int32_t AutoFillManager::RequestAutoSave(
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr || saveCallback == nullptr) {
        HILOG_ERROR("UIContent or save callback is nullptr.");
        return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
    }

    AAFwk::Want want;
    want.SetElementName(AUTO_FILL_BUNDLE_NAME, AUTO_FILL_ABILITY_NAME);
    want.SetModuleName(AUTO_FILL_MODULE_NAME);
    want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_EXTENSION_TYPE);
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, WANT_PARAMS_AUTO_SAVE_CMD);
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, viewdata.ToJsonString());

    auto extensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (extensionCallback == nullptr) {
        HILOG_ERROR("Extension callback is nullptr.");
        return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
    }
    extensionCallback->SetSaveRequestCallback(saveCallback);

    Ace::ModalUIExtensionCallbacks callback;
    callback.onResult = std::bind(
        &AutoFillExtensionCallback::OnResult, extensionCallback, std::placeholders::_1, std::placeholders::_2);
    callback.onRelease = std::bind(
        &AutoFillExtensionCallback::OnRelease, extensionCallback, std::placeholders::_1);
    callback.onError = std::bind(&AutoFillExtensionCallback::OnError,
        extensionCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    Ace::ModalUIExtensionConfig config;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("Create modal ui extension is failed.");
        return AutoFiil::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    extensionCallback->SetUIContent(uiContent);
    extensionCallback->SetSessionId(sessionId);
    return AutoFiil::AUTO_FILL_SUCCESS;
}
} // namespace AbilityRuntime
} // namespace OHOS
