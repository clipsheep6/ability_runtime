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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_APPMGR_TEST_UT_MOCK_ABILITY_TOKEN_H
#define FOUNDATION_APPEXECFWK_SERVICES_APPMGR_TEST_UT_MOCK_ABILITY_TOKEN_H

#include "nocopyable.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AbilityToken");
};
DECLARE_INTERFACE_DESCRIPTOR(u"IAbilityToken");

class MockAbilityToken : public IRemoteStub<IAbilityToken> {
public:
    MockAbilityToken() = default;
    virtual ~MockAbilityToken() = default;

    virtual int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    }

private:
    DISALLOW_COPY_AND_MOVE(MockAbilityToken);
};

class AbilityTokenProxy : public IRemoteProxy<IAbilityToken> {
public:
    explicit AbilityTokenProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityToken>(impl)
    {}

    virtual ~AbilityTokenProxy() = default;

private:
    DISALLOW_COPY_AND_MOVE(AbilityTokenProxy);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_APPMGR_TEST_UT_MOCK_ABILITY_TOKEN_H
