/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "app_mgr_service_event_handler.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AMSEventHandler::AMSEventHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler,
    const std::weak_ptr<AppMgrServiceInner> &appMgr)
    : AAFwk::EventHandlerWrap(taskHandler), appMgr_(appMgr)
{
    TAG_LOGI(AAFwkTag::APPMGR, "instance created");
}

AMSEventHandler::~AMSEventHandler()
{
    TAG_LOGI(AAFwkTag::APPMGR, "instance destroyed");
}

void AMSEventHandler::ProcessEvent(const AAFwk::EventWrap &event)
{
    auto appManager = appMgr_.lock();
    if (!appManager) {
        TAG_LOGE(AAFwkTag::APPMGR, "app manager is nullptr");
        return;
    }
    appManager->HandleTimeOut(event);
}
}  // namespace AppExecFwk
}  // namespace OHOS
