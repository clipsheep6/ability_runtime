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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_RENDER_SCHEDULER_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_RENDER_SCHEDULER_H

#include "gmock/gmock.h"
#include "irender_scheduler.h"

namespace OHOS {
namespace AppExecFwk {
class MockRenderScheduler : public IRenderScheduler {
public:
    MockRenderScheduler() = default;
    virtual ~MockRenderScheduler() = default;

    MOCK_METHOD3(NotifyBrowserFd,
                 void(int32_t ipcFd, int32_t sharedFd, int32_t crashFd));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_RENDER_SCHEDULER_H
