/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "screenshot_response.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

void ScreenShotResponse::OnWindowShot(const OHOS::WMImageInfo &info)
{
    HILOG_INFO("On screen shot call back.");
    std::unique_lock<std::mutex> lock(mutex_);
    info_ = std::make_shared<OHOS::WMImageInfo>();
    if (!info_) {
        return;
    }
    info_->width = info.width;
    info_->size = info.size;
    info_->height = info.height;
    info_->format = info.format;
    info_->data = info.data;
    condition_.notify_all();
}

OHOS::WMImageInfo ScreenShotResponse::GetImageInfo()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (info_ == nullptr) {
        if (condition_.wait_for(lock, std::chrono::milliseconds(TIME_OUT)) == std::cv_status::timeout) {
            return OHOS::WMImageInfo();
        }
    }

    OHOS::WMImageInfo info = *info_;
    info_.reset();
    return info;
}
}  // namespace AAFwk
}  // namespace OHOS
