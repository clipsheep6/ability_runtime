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

#ifndef OHOS_ABILITY_RUNTIME_URI_UTILS_H
#define OHOS_ABILITY_RUNTIME_URI_UTILS_H

#include <string>
#include <vector>

#include "nocopyable.h"
#include "uri.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class UriUtils {
public:
    static UriUtils &GetInstance();

    void FilterUriWithPermissionDms(Want &want, uint32_t tokenId);

private:
    UriUtils();
    ~UriUtils();

    std::vector<std::string> GetUriListFromWantDms(const Want &want);

    DISALLOW_COPY_AND_MOVE(UriUtils);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_URI_UTILS_H