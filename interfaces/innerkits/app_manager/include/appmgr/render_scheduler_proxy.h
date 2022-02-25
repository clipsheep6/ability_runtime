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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_PROXY_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_PROXY_H

#include "render_scheduler_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class RenderSchedulerProxy
 * RenderScheduler proxy.
 */
class RenderSchedulerProxy : public IRemoteProxy<IRenderScheduler> {
public:
    explicit RenderSchedulerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IRenderScheduler>(impl)
    {}
    virtual ~RenderSchedulerProxy()
    {}

private:
    static inline BrokerDelegator<RenderSchedulerProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_APPMGR_RENDER_SCHEDULER_PROXY_H
