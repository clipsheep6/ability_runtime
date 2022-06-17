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

#include "form_event_handler.h"
#include "hilog_wrapper.h"
#include "form_share_mgr.h"

namespace OHOS {
namespace AppExecFwk {
int64_t FormEventHandler::gEventId_ = 0;
FormEventHandler::FormEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
}

void FormEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (!event) {
        HILOG_ERROR("event is nullptr");
        return;
    }

    switch (event->GetInnerEventId()) {
        case MSG::FORM_SHARE_INFO_DELAY_MSG: {
            ProcessFormShareInfoTimeOut(event->GetParam());
            break;
        }
        case MSG::FORM_PACKAGE_FREE_INSTALL_DELAY_MSG: {
            ProcessFreeInstallTimeOut(event->GetParam());
            break;
        }
        default: {
            break;
        }
    }
}

int64_t FormEventHandler::GetEventId()
{
    gEventId_++;
    return gEventId_;
}

void FormEventHandler::ProcessFormShareInfoTimeOut(int64_t eventId)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    FormShareMgr::GetInstance().HandleFormShareInfoTimeOut(eventId);
}

void FormEventHandler::ProcessFreeInstallTimeOut(int64_t eventId)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    FormShareMgr::GetInstance().HandleFreeInstallTimeOut(eventId);
}
}  // namespace AppExecFwk
}  // namespace OHOS