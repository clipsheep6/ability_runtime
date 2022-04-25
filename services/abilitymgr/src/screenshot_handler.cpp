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

#include "screenshot_handler.h"
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

ScreenshotHandler::ScreenshotHandler()
{
}

void ScreenshotHandler::StartScreenshot(int32_t missionId, int32_t winId)
{
    HILOG_INFO("StartScreenshot");
    auto response = std::make_shared<ScreenShotResponse>();
}

void ScreenshotHandler::RemoveImageInfo(int32_t missionId)
{
    HILOG_DEBUG("%{public}s begin", __func__);
}

}  // namespace AAFwk
}  // namespace OHOS
