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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_REQUEST_CALLBACK_IMPL_H
#define OHOS_ABILITY_RUNTIME_DIALOG_REQUEST_CALLBACK_IMPL_H

#include "ability_context.h"
#include "dialog_request_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class DialogRequestCallbackImpl : public DialogRequestCallbackStub {
public:
    explicit DialogRequestCallbackImpl(RequestDialogResultTask &&task) : task_(task) {}
    virtual ~DialogRequestCallbackImpl() = default;

    void SendResult(int32_t resultCode) override;

private:
    RequestDialogResultTask task_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTHORIZATION_RESULT_H
