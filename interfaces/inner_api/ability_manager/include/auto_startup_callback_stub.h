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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_STARTUP_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_AUTO_STARTUP_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include <map>

#include "auto_startup_interface.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class AutoStartupCallBackStub
 * AutoStartupCallBack Stub.
 */
class AutoStartupCallBackStub : public IRemoteStub<IAutoStartupCallBack> {
public:
    AutoStartupCallBackStub();
    virtual ~AutoStartupCallBackStub();

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void Init();
    int32_t OnAutoStartupOnInner(MessageParcel &data, MessageParcel &reply);
    int32_t OnAutoStartupOffInner(MessageParcel &data, MessageParcel &reply);

    using RequestFuncType = int (AutoStartupCallBackStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
    DISALLOW_COPY_AND_MOVE(AutoStartupCallBackStub);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_STARTUP_CALLBACK_STUB_H
