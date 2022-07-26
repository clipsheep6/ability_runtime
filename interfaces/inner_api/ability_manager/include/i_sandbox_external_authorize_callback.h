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

#ifndef OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_H

#include "iremote_broker.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
enum AuthorType {
    DENY_START_ABILITY,
    ALLOW_START_ABILITY,
};

/**
 * @class ISandboxExternalAuthorizeCallback
 * sandbox external authorize callback.
 */
class ISandboxExternalAuthorizeCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.SandboxExternalAuthorizeCallback");

    virtual AuthorType GetAuthorization(int sandboxUid, const Want& want) = 0;
    enum SandboxExternalAuthorizeCallbackCmd {
        // ipc id for GetAuthorization
        GET_SANDBOX_EXTERNAL_AUTH = 0,

        // maximum of enum
        CMD_MAX
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_H
