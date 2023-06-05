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

#ifndef OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_INTERFACE_H

#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
class IPrepareTerminateCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.prepareTerminateCallback");
    virtual void DoPrepareTerminate() {};
    enum {
        // ipc id for DoPrepareTerminate (1)
        ON_DO_PREPARE_TERMINATE = 1,
        CODE_MAX
    };
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PREPARE_TERMINATE_CALLBACK_INTERFACE_H
