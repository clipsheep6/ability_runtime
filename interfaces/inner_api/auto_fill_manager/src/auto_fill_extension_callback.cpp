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

#include "auto_fill_extension_callback.h"

#include "auto_fill_error.h"
#include "foundation/ability/ability_base/interfaces/kits/native/view_data/include/view_data.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
} // namespace
void AutoFillExtensionCallback::OnResult(int32_t number, const AAFwk::Want &want)
{
    HILOG_DEBUG("Called result code is %{public}d.", number);
    auto fillCallback = fillCallback_.lock();
    auto saveCallback = saveCallback_.lock();
    if (fillCallback != nullptr && saveCallback == nullptr) {
        HILOG_ERROR("saveCallback and saveCallback is nullptr.");
        return;
    }

    if (isFillCallback_) {
        if (fillCallback == nullptr) {
            HILOG_ERROR("sucess: fillCallback is nullptr.");
            return;
        }
        std::string dataStr = want.GetStringParam(WANT_PARAMS_VIEW_DATA);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(dataStr.c_str());
        fillCallback->OnFillRequestSuccess(viewData);
    } else {
        if (saveCallback == nullptr) {
            HILOG_ERROR("sucess: saveCallback is nullptr.");
            return;
        }
        saveCallback->OnSaveRequestSuccess();
    }
}

void AutoFillExtensionCallback::OnRelease(int32_t number)
{
    HILOG_DEBUG("Called result code is %{public}d.", number);
    auto fillCallback = fillCallback_.lock();
    auto saveCallback = saveCallback_.lock();
    if (fillCallback != nullptr && saveCallback == nullptr) {
        HILOG_ERROR("saveCallback and saveCallback is nullptr.");
        return;
    }

    if (isFillCallback_) {
        if (fillCallback == nullptr){
            HILOG_ERROR("failed: fillCallback is nullptr.");
            return;
        }
        fillCallback->OnFillRequestFailed(number);
    } else {
        if (saveCallback == nullptr){
            HILOG_ERROR("failed: fillCallback is nullptr.");
            return;
        }
        saveCallback->OnSaveRequestFailed();
    }
}

void AutoFillExtensionCallback::SetFillRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback)
{
    fillCallback_ = callback;
    isFillCallback_ = true;
}

void AutoFillExtensionCallback::SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback)
{
    saveCallback_ = callback;
    isFillCallback_ = false;
}
} // namespace AbilityRuntime
} // namespace OHOS