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

#ifndef PREPARE_TERMINATE_CALLBACK_H
#define PREPARE_TERMINATE_CALLBACK_H

#include "iprepare_terminate_callback_interface.h"

namespace OHOS {
namespace AAFwk {
class PrepareTerminateCallback : public IPrepareTerminateCallback {
public:
    MOCK_METHOD0(DoPrepareTerminate, void());
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
}
}
#endif // PREPARE_TERMINATE_CALLBACK_H