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

#ifndef OHOS_ABILITY_RUNTIME_EXIT_RESIDENT_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_EXIT_RESIDENT_PROCESS_MANAGER_H

#include <string>
#include <vector>

#include "bundle_info.h"
#include "cpp/mutex.h"
#include "nocopyable.h"
#include "refbase.h"


namespace OHOS {
namespace AppExecFwk {
enum class MemorySizeState {
    MEMORY_SIZE_SUFFICENT = 0,
    MEMORY_SIZE_INSUFFICENT = 1
};

class ExitResidentProcessManager {
public:
    static ExitResidentProcessManager &GetInstance();
    ~ExitResidentProcessManager();
    bool IsMemorySizeSufficent() const;
    bool RecordExitResidentBundleName(const std::string &bundleName);
    int32_t HandleMemorySizeInSufficent();
    int32_t HandleMemorySizeSufficent(std::vector<std::string>& bundleNames);
    void QueryExitBundleInfos(const std::vector<std::string>& exitBundleNames,
        std::vector<AppExecFwk::BundleInfo>& exitBundleInfos);

private:
    ExitResidentProcessManager();
    MemorySizeState currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    std::vector<std::string> exitResidentBundleNames_;
    mutable ffrt::mutex mutexLock_;
    DISALLOW_COPY_AND_MOVE(ExitResidentProcessManager);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EXIT_RESIDENT_PROCESS_MANAGER_H
