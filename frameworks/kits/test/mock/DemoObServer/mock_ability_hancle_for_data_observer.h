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
#ifndef MOCK_ABILIT_HANCLDE_FOR_DATA_OBSERVER_H
#define MOCK_ABILIT_HANCLDE_FOR_DATA_OBSERVER_H

#include <string>

#include "gtest/gtest.h"

#include "event_handler.h"

namespace OHOS {
namespace AppExecFwk {
bool EventHandler::SendEvent(InnerEvent::Pointer &event, int64_t delayTime, Priority priority)
{
    auto callback = event->GetTaskCallback();
    callback();
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_ABILIT_HANCLDE_FOR_DATA_OBSERVER_H
