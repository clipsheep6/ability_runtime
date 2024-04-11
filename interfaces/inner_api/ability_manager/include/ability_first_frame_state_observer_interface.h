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

#ifndef OHOS_ABILITY_FIRST_FRAME_STATE_OBSERVER_INTERFACE_H
#define OHOS_ABILITY_FIRST_FRAME_STATE_OBSERVER_INTERFACE_H
#ifdef SUPPORT_GRAPHICS

#include "ability_first_frame_state_data.h"
#include "iremote_broker.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityFirstFrameStateObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IAbilitFirstFrameState");

    virtual void OnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData) = 0;

    enum class Message {
        ON_ABILITY_FIRST_FRAME_STATE,
    };
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // SUPPORT_GRAPHICS
#endif // OHOS_ABILITY_FIRST_FRAME_STATE_OBSERVER_INTERFACE_H
