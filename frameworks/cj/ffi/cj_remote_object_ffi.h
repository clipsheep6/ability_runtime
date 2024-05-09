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

#include "ffi_remote_data.h"
#include "iremote_object.h"

#ifndef ABILITY_RUNTIME_CJ_REMOTE_OBJECT_FFI_H
#define ABILITY_RUNTIME_CJ_REMOTE_OBJECT_FFI_H

namespace OHOS {
namespace AppExecFwk {
class CJRemoteObject : public FFI::FFIData {
public:
    explicit CJRemoteObject(const sptr<IRemoteObject>& remoteObject) : remoteObject_(remoteObject) {}

private:
    sptr<IRemoteObject> remoteObject_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // ABILITY_RUNTIME_CJ_REMOTE_OBJECT_FFI_H
