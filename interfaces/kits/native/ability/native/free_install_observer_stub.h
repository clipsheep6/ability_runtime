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

#ifndef OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "free_install_observer_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {
class FreeInstallObserverStub : public IRemoteStub<IFreeInstallObserver> {
public:
    FreeInstallObserverStub();
    virtual ~FreeInstallObserverStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(FreeInstallObserverStub);
    int OnInstallFinishedInner(MessageParcel &data, MessageParcel &reply);
    using FreeInstallObserverFunc = int32_t (FreeInstallObserverStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, FreeInstallObserverFunc> memberFuncMap_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_STUB_H