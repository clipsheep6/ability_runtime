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

#ifndef OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_STUB_H
#define OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_STUB_H

#include "iremote_stub.h"
#include "icomponent_interception.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Interface to monitor what is happening in component manager.
 */
class ComponentInterceptionStub : public IRemoteStub<IComponentInterception> {
public:
    ComponentInterceptionStub();
    virtual ~ComponentInterceptionStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * The system is trying to start an component.
     *
     * @param want The want of component to start.
     * @param callerToken Caller component token.
     * @param requestCode the requestCode of the component to start.
     * @param componentStatus the status of component.
     * @param extraParam The extra param of component to start.
     * @return Return true to allow component to start, or false to reject.
     */
    virtual bool AllowComponentStart(const Want &want, const sptr<IRemoteObject> &callerToken,
        int requestCode, int componentStatus, sptr<Want> &extraParam) override;

private:
    using ComponentInterceptionFunc = int32_t (ComponentInterceptionStub::*)
        (MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, ComponentInterceptionFunc> requestFuncMap_;
    int32_t HandleAllowComponentStart(MessageParcel &data, MessageParcel &reply);

    DISALLOW_COPY_AND_MOVE(ComponentInterceptionStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_STUB_H
