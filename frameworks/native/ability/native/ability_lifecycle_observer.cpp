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

#include "ability_lifecycle_observer.h"

namespace OHOS {
namespace AppExecFwk {
void LifecycleObserver::OnActive() {}

void LifecycleObserver::OnInactive() {}

void LifecycleObserver::OnStart(const Want &want) {}

void LifecycleObserver::OnStop() {}

void LifecycleObserver::OnStateChanged(Lifecycle::Event event, const Want &want) {}

virtual void LifecycleObserver::OnStateChanged(LifeCycle::Event event) {}

#ifdef SUPPORT_GRAPHICS
void LifecycleObserver::OnBackground() {}

void LifecycleObserver::OnForeground(const Want &want) {}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
