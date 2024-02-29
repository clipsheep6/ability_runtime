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

#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include "app_mgr_client.h"
#include "render_state_observer_stub.h"
#include "mock_native_token.h"
#include "singleton.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

volatile int32_t g_count = 0;

class MockRenderStateObserver : public RenderStateObserverStub {
public:
    MockRenderStateObserver() = default;
    virtual ~MockRenderStateObserver() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {
        g_count++;
        cout << "OnRenderStateChanged:" << endl;
        cout << "    pid: " << renderStateData.pid << endl;
        cout << "    uid: " << renderStateData.uid << endl;
        cout << "    hostPid: " << renderStateData.hostPid << endl;
        cout << "    hostUid: " << renderStateData.hostUid << endl;
        cout << "    state: " << renderStateData.state << endl;
    }
};

int main()
{
    MockNativeToken::SetNativeToken();

    sptr<IRenderStateObserver> observer_ = new MockRenderStateObserver();
    int32_t ret = DelayedSingleton<AppMgrClient>::GetInstance()->RegisterRenderStateObserver(observer_);
    if (ret != ERR_OK) {
        cout << "注册Render进程状态监控失败，error:" << ret << endl;
        return 0;
    }
    while (g_count < 10) {}
    
    return 0;
}