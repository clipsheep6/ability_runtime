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

#ifndef OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H
#define OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H

#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
class DumpRuntimeHelper : public std::enable_shared_from_this<DumpRuntimeHelper> {
public:
    explicit DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application);
    ~DumpRuntimeHelper() = default;
    void SetAppFreezeFilterCallback();
private:
    std::shared_ptr<OHOSApplication> application_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H
