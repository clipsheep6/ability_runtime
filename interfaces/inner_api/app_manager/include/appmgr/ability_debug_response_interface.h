/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityDebugResponse : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AbilityDebugResponse");

    /**
     * @brief Set ability attach debug flag to ability manager service.
     * @param tokens The token of ability token.
     */
    virtual void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) = 0;

    /**
     * @brief Cancel ability attach debug flag to ability manager service.
     * @param tokens The token of ability token.
     */
    virtual void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) = 0;

    /**
     * @brief Change ability assert debug flag.
     * @param tokens The token of ability records.
     * @param isAssertDebug Assert debug flag.
     */
    virtual void OnAbilitysAssertDebugChange(const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug) = 0;

    enum class Message {
        ON_ABILITYS_DEBUG_STARTED = 0,
        ON_ABILITYS_DEBUG_STOPED,
        ON_ABILITYS_ASSERT_DEBUG,
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_INTERFACE_H
