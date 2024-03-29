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

#ifndef OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class IStatusBarDelegate
 */
class IStatusBarDelegate : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ability.StatusBarDelegate");

    enum class StatusBarDelegateCmd : uint32_t {
        CHECK_IF_STATUS_BAR_ITEM_EXISTS = 0,
        ATTACH_PID_TO_STATUS_BAR_ITEM,
        END
    };

    virtual int32_t CheckIfStatusBarItemExists(uint32_t accessTokenId, bool& isExist) = 0;
    virtual int32_t AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_INTERFACE_H
