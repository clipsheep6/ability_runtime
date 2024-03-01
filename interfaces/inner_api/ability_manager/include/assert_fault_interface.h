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

#ifndef OHOS_ABILITY_RUNTIME_ASSERT_FAULT_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ASSERT_FAULT_INTERFACE_H

#include <iremote_broker.h>
#include "ability_state.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class IAssertFaultInterface
 * IAssertFaultInterface is used to notify caller ability user action result.
 */
class IAssertFaultInterface : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.IAssertFaultInterface");

    /**
     * Notify listeners of user operation results.
     *
     * @param status - User action result.
     */
    virtual void NotifyDebugAssertResult(AAFwk::UserStatus status) = 0;
protected:
    enum MessageCode {
        NOTIFY_USER_ACTION_RESULT,
    };
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ASSERT_FAULT_INTERFACE_H
