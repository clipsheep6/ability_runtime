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

#include "ui_extension_callback.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
} // namespace
UIExtensionCallback::UIExtensionCallback(const std::string &autoFillType) : autoFillType_(autoFillType)
{}

void UIExtensionCallback::OnResult(int32_t number, const AAFwk::Want &want)
{
    HILOG_DEBUG("Called result code is %{public}d.", number);
    auto fillCallback = fillCallback_.lock();
    auto saveCallback = saveCallback_.lock();
    if (fillCallback != nullptr && saveCallback == nullptr) {
        HILOG_ERROR("saveCallback and saveCallback is nullptr.");
        return;
    }

    switch (number)
    {
    case ResultCode::FILL_SUCESS:
    {
        if (fillCallback == nullptr){
            HILOG_ERROR("sucess: fillCallback is nullptr.");
            return;
        }
        std::string dataStr = want.GetStringParam(WANT_PARAMS_VIEW_DATA);
        ViewData viewData;
        viewData = viewData.ToJsValue(dataStr.c_str());
        fillCallback->OnFillRequestSuccess(viewData);
        break;
    }
    case ResultCode::FILL_FAILED:
    case ResultCode::FILL_CANCEL:
    {
        if (fillCallback == nullptr){
            HILOG_ERROR("failed: fillCallback is nullptr.");
            return;
        }
        fillCallback->OnFillRequestFailed(number);
        break;
    }
    case ResultCode::SAVE_SUCESS:
    {
        if (saveCallback == nullptr){
            HILOG_ERROR("sucess: saveCallback is nullptr.");
            return;
        }
        saveCallback->OnSaveRequestSuccess();
        break;
    }
    case ResultCode::SAVE_FAILED:
    {
        if (saveCallback == nullptr){
            HILOG_ERROR("sucess: saveCallback is nullptr.");
            return;
        }
        saveCallback->OnSaveRequestFailed();
        break;
    }
    default:
        HILOG_ERROR("Result code is invalid.");
        break;
    }
}

void UIExtensionCallback::SetAutoRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback)
{
    fillCallback_ = callback;
}

void UIExtensionCallback::SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback)
{
    saveCallback_ = callback;
}
} // namespace AbilityRuntime
} // namespace OHOS