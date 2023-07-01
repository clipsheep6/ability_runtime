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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_PERF_SOCPERF_CLIENT_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_PERF_SOCPERF_CLIENT_H

#include <string>

namespace OHOS::Ace {
class SocPerfClient {
public:
    SocPerfClient() = default;
    virtual ~SocPerfClient() = default;
    static SocPerfClient& GetInstance();
    virtual void PerfRequest(int32_t cmdId, const std::string& msg) {};
};
} // namespace OHOS::Ace

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_PERF_SOCPERF_CLIENT_H