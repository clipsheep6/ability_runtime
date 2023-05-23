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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H

#include "extension_context.h"

#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief context supply for UIExtension
 *
 */
class UIExtensionContext : public ExtensionContext {
public:
    UIExtensionContext() = default;
    virtual ~UIExtensionContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.EXTENSION or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode StartAbility(const AAFwk::Want &want) const;
    virtual ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const;

    /**
     * @brief Destroys the current ui extension ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateSelf();

    using SelfType = UIExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

private:
    static int ILLEGAL_REQUEST_CODE;

    /**
     * @brief Get Current Ability Type
     *
     * @return Current Ability Type
     */
    OHOS::AppExecFwk::AbilityType GetAbilityInfoType() const;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_CONTEXT_H
