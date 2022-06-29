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

#ifndef OHOS_AAFWK_BACKGROUND_TASK_OBSERVER_H
#define OHOS_AAFWK_BACKGROUND_TASK_OBSERVER_H

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_subscriber.h"

namespace OHOS {
namespace AAFwk {
class BackgroundTaskObserver : public BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    BackgroundTaskObserver();
    virtual ~BackgroundTaskObserver();
    bool IsBackgroundTaskUid(const int uid);

private:
    void OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo);

    void OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo);

private:
    std::list<int> bgTaskUids_;
    std::mutex bgTaskMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
#endif  // OHOS_AAFWK_BACKGROUND_TASK_OBSERVER_H
