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

#include "extension_ability_info.h"
#include "foundation/ability/ability_base/interfaces/kits/native/view_data/include/view_data.h"
#include "hilog_wrapper.h"
#include "auto_fill_extension_callback.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *TEMP_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char *TEMP_BUNDLE_NAME = "com.ohos.launcher";
const std::string WANT_PARAMS_AUTO_FILL_CMD_FILL = "fill";
const std::string WANT_PARAMS_AUTO_FILL_CMD_SAVE = "save";
const std::string WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
const std::string WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
const std::string WANT_PARAMS_EXTENSION_TYPE_KEY = "ability.want.params.ExtensionType";
const std::string WANT_PARAMS_AUTO_FILL_TYPE_KEY = "ability.want.params.AutoFillType";
} // namespace

int32_t AutoFillManager::RequestAutoFill(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<IFillRequestCallback> &fillCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr) {
        HILOG_ERROR("uiContent is nullptr.");
        return ERR_INVALID_VALUE;
    }
    AAFwk::Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::AUTOFILL;
    want.SetElementName(TEMP_BUNDLE_NAME, TEMP_ABILITY_NAME);
    want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, static_cast<int32_t>(extensionType));
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(autoFillType));
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD, WANT_PARAMS_AUTO_FILL_CMD_FILL);
    want.SetParam(WANT_PARAMS_VIEW_DATA, viewdata.ToJsonString());

    auto autoFillExtensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (autoFillExtensionCallback == nullptr) {
        HILOG_ERROR("autoFillExtensionCallback is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    autoFillExtensionCallback->SetFillRequestCallback(fillCallback);

    Ace::ModalUIExtensionCallbacks callback;
    callback.onResult = std::bind(
        &AutoFillExtensionCallback::OnResult, autoFillExtensionCallback, std::placeholders::_1, std::placeholders::_2);
    callback.onRelease = std::bind(
        &AutoFillExtensionCallback::OnRelease, autoFillExtensionCallback, std::placeholders::_1);

    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("CreateModalUIExtension is failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AutoFillManager::RequestAutoSave(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr) {
        HILOG_ERROR("uiContent is nullptr.");
        return ERR_INVALID_VALUE;
    }

    AAFwk::Want want;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    want.SetElementName(TEMP_BUNDLE_NAME, TEMP_ABILITY_NAME);
    want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, static_cast<int32_t>(extensionType));
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(autoFillType));
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD, WANT_PARAMS_AUTO_FILL_CMD_SAVE);
    want.SetParam(WANT_PARAMS_VIEW_DATA, viewdata.ToJsonString());

    auto autoFillExtensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (autoFillExtensionCallback == nullptr) {
        HILOG_ERROR("autoFillExtensionCallback is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    autoFillExtensionCallback->SetSaveRequestCallback(saveCallback);

    Ace::ModalUIExtensionCallbacks callback;
    callback.onResult = std::bind(
        &AutoFillExtensionCallback::OnResult, autoFillExtensionCallback, std::placeholders::_1, std::placeholders::_2);
    callback.onRelease = std::bind(
        &AutoFillExtensionCallback::OnRelease, autoFillExtensionCallback, std::placeholders::_1);

    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("CreateModalUIExtension is failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
