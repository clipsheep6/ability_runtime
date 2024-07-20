/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UI_HOLDER_H
#define OHOS_ABILITY_RUNTIME_UI_HOLDER_H

#include "context.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AbilityRuntime {
class UIHolder {
public:
    UIHolder() = default;
    virtual ~UIHolder() = default;

    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }

    static std::shared_ptr<UIHolder> ConvertToUIHolder(const std::shared_ptr<Context>& context);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_HOLDER_H
