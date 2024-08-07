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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_XCOLLIE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_XCOLLIE_H

#include <stdint.h>
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace AbilityRuntime {

static const uint32_t TIME_OUT_SECONDS = 30;

class AbilityManagerXCollie {
public:
    AbilityManagerXCollie(const std::string &tag, uint32_t timeoutSeconds = TIME_OUT_SECONDS,
        std::function<void(void *)> func = nullptr, void* arg = nullptr,
        uint32_t flag = HiviewDFX::XCOLLIE_FLAG_LOG);

    ~AbilityManagerXCollie();

    void CancelAbilityManagerXCollie();
private:
    int32_t id_ = -1;
    std::string tag_;
    bool isCanceled_ = true;
};
}
}
#endif //OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_XCOLLIE_H