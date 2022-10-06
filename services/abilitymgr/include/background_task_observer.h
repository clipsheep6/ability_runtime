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

#ifndef OHOS_ABILITY_RUNTIME_BACKGROUND_TASK_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_BACKGROUND_TASK_OBSERVER_H

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_mgr_helper.h"
#include "background_task_subscriber.h"
#include "iremote_object.h"
#include "app_mgr_interface.h"

namespace OHOS {
namespace AAFwk {
const int32_t SUBSCRIBE_BACKGROUND_TASK_TRY = 5;
const int32_t REPOLL_TIME_MICRO_SECONDS = 1000000;
class BackgroundTaskObserver : public BackgroundTaskMgr::BackgroundTaskSubscriber,
                               public std::enable_shared_from_this<BackgroundTaskObserver> {
public:
    BackgroundTaskObserver();
    virtual ~BackgroundTaskObserver();
    bool IsBackgroundTaskUid(const int uid);

private:
    void OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo);

    void OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>
        &continuousTaskCallbackInfo);

    void OnRemoteDied(const wptr<IRemoteObject> &object);

    sptr<AppExecFwk::IAppMgr> GetAppManager();

private:
    std::list<int> bgTaskUids_;
    std::mutex bgTaskMutex_;
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
#endif  // OHOS_ABILITY_RUNTIME_BACKGROUND_TASK_OBSERVER_H
