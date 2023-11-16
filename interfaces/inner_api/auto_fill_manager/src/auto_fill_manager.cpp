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

#include "auto_fill_manager.h"

#include "auto_fill_error.h"

namespace OHOS {
namespace AbilityRuntime {
AutoFillManager &AutoFillManager::GetInstance()
{
    static AutoFillManager instance;
    return instance;
}

int32_t AutoFillManager::RequestAutoFill(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<IFillRequestCallback> &fillCallback)
{
    return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
}

int32_t AutoFillManager::RequestAutoSave(
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    return AutoFiil::AUTO_FILL_OBJECT_IS_NULL;
}
} // namespace AbilityRuntime
} // namespace OHOS
