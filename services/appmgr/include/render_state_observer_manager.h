/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_MANAGER_H

#include <vector>
#include "app_running_record.h"
#include "irender_state_observer.h"
#include "singleton.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class RenderStateObserverManager : public std::enable_shared_from_this<RenderStateObserverManager> {
    DECLARE_DELAYED_SINGLETON(RenderStateObserverManager);
public:
    void Init();
    int32_t RegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);
    int32_t UnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);
    int32_t OnRenderStateChanged(const std::shared_ptr<RenderRecord> &renderRecord, int32_t state);
private:
    void OnObserverDied(const wptr<IRemoteObject> &remote);
    void HandleRegisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);
    void HandleUnregisterRenderStateObserver(const sptr<IRenderStateObserver> &observer);
    void HandleOnRenderStateChanged(const std::shared_ptr<RenderRecord> &renderRecord, int32_t state);
    RenderStateData WrapRenderStateData(const std::shared_ptr<RenderRecord> &renderRecord,
        int32_t state);

    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::vector<sptr<IRenderStateObserver>> observerList_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler_;
};

class RenderStateObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit RenderStateObserverRecipient(RemoteDiedHandler handler);
    virtual ~RenderStateObserverRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);
private:
    RemoteDiedHandler handler_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RENDER_STATE_OBSERVER_MANAGER_H
