/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_IABILITY_CALLBACK_H

#ifdef SUPPORT_GRAPHICS
#include "pixel_map.h"
#endif

namespace OHOS {
namespace AppExecFwk {
class IAbilityCallback {
public:
    IAbilityCallback() = default;
    virtual ~IAbilityCallback() = default;
#ifdef SUPPORT_GRAPHICS
    /**
     * @brief Called back at ability context.
     *
     * @return current window mode of the ability.
     */
    virtual int GetCurrentWindowMode() = 0;

    /**
     * @brief Set mission label of this ability.
     *
     * @param label the label of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionLabel(const std::string &label) = 0;

    /**
     * @brief Set mission icon of this ability.
     *
     * @param icon the icon of this ability.
     * @return Returns ERR_OK if success.
     */
    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;
#endif
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IABILITY_CALLBACK_H
