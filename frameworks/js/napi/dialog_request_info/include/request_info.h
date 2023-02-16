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

#ifndef OHOS_ABILITY_RUNTIME_REQUEST_INFO_H
#define OHOS_ABILITY_RUNTIME_REQUEST_INFO_H

#include "iremote_object.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class RequestInfo {
public:
    explicit RequestInfo(const sptr<IRemoteObject> &token);
    ~RequestInfo();

    static NativeValue* WrapRequestInfo(NativeEngine &engine, RequestInfo *request);
    static std::shared_ptr<RequestInfo> UnwrapRequestInfo(NativeEngine &engine, NativeValue *jsParam);

    sptr<IRemoteObject> GetToken();
private:
    sptr<IRemoteObject> callerToken_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_REQUEST_INFO_H
