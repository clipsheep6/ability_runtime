/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_lifecycle_observer_interface.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
LifeCycle::Event LifeCycle::GetLifecycleState()
{
    HILOG_DEBUG("LifeCycle::GetLifecycleState: called");
    return state_;
}

void LifeCycle::AddObserver(const std::shared_ptr<ILifecycleObserver> &observer)
{
    HILOG_DEBUG("LifeCycle::AddObserver: called");

    if (observer == nullptr) {
        HILOG_ERROR("LifeCycle::AddObserver: observer is null");
        return;
    }

    callbacks_.emplace_back(observer);
}

void LifeCycle::DispatchLifecycle(const LifeCycle::Event &event, const Want &want)
{
    HILOG_DEBUG("LifeCycle::DispatchLifecycle: event:%{public}d", event);
    if ((event != LifeCycle::Event::ON_FOREGROUND) && (event != LifeCycle::Event::ON_START)) {
        HILOG_ERROR("event value error: event is %{public}d", event);
        return;
    }

    state_ = event;
    if (callbacks_.size() != 0) {
        for (auto &callback : callbacks_) {
            switch (event) {
#ifdef SUPPORT_GRAPHICS
                case ON_FOREGROUND: {
                    if (callback != nullptr) {
                        callback->OnForeground(want);
                    }
                    break;
                }
#endif
                case ON_START: {
                    if (callback != nullptr) {
                        callback->OnStart(want);
                    }
                    break;
                }
                default:
                    break;
            }
            if (callback != nullptr) {
                callback->OnStateChanged(event, want);
            }
        }
    }
}

void LifeCycle::DispatchLifecycle(const LifeCycle::Event &event)
{
    HILOG_DEBUG("LifeCycle::DispatchLifecycle: event:%{public}d", event);
    if ((event != LifeCycle::Event::ON_ACTIVE) && (event != LifeCycle::Event::ON_BACKGROUND) &&
        (event != LifeCycle::Event::ON_INACTIVE) && (event != LifeCycle::Event::ON_STOP)) {
        HILOG_ERROR("event value error: event is %{public}d", event);
        return;
    }

    state_ = event;
    if (callbacks_.size() != 0) {
        for (auto &callback : callbacks_) {
            switch (event) {
                case ON_ACTIVE: {
                    if (callback != nullptr) {
                        callback->OnActive();
                    }
                    break;
                }
#ifdef SUPPORT_GRAPHICS
                case ON_BACKGROUND: {
                    if (callback != nullptr) {
                        callback->OnBackground();
                    }
                    break;
                }
#endif
                case ON_INACTIVE: {
                    if (callback != nullptr) {
                        callback->OnInactive();
                    }
                    break;
                }
                case ON_STOP: {
                    if (callback != nullptr) {
                        callback->OnStop();
                    }
                    break;
                }
                default:
                    break;
            }
            if (callback != nullptr) {
                callback->OnStateChanged(event);
            }
        }
    }
}

void LifeCycle::RemoveObserver(const std::shared_ptr<ILifecycleObserver> &observer)
{
    HILOG_DEBUG("LifeCycle::RemoveObserver: called");

    if (observer == nullptr) {
        HILOG_ERROR("LifeCycle::RemoveObserver: observer is null");
        return;
    }

    callbacks_.remove(observer);
}
}  // namespace AppExecFwk
}  // namespace OHOS
