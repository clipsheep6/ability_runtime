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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_EVENT_HANDLER_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_EVENT_HANDLER_H

#include <memory>

#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace AppExecFwk {
namespace MSG {
const int64_t FORM_SHARE_INFO_DELAY_MSG = 1;
const int64_t FORM_PACKAGE_FREE_INSTALL_DELAY_MSG = 2;
}
/**
 * @class FormEventHandler
 * FormEventHandler handling the form event.
 */
class FormEventHandler : public EventHandler {
public:
    FormEventHandler(const std::shared_ptr<EventRunner> &runner);
    virtual ~FormEventHandler() = default;

    /**
     * ProcessEvent with request.
     *
     * @param event, inner event loop.
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    static int64_t GetEventId();
private:
    void ProcessFormShareInfoTimeOut(int64_t eventId);
    void ProcessFreeInstallTimeOut(int64_t eventId);

private:
    static int64_t gEventId_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_FORM_EVENT_HANDLER_H