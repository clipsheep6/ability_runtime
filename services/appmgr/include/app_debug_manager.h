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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_APP_DEBUG_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_APP_DEBUG_MANAGER_H

#include <mutex>
#include <set>

#include "app_debug_listener_interface.h"

namespace OHOS {
namespace AppExecFwk {
class AppDebugManager {
public:
    AppDebugManager() = default;
    ~AppDebugManager() = default;

    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener);
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener);
    void StartDebug(const std::vector<AppDebugInfo> &infos);
    void StopDebug(const std::vector<AppDebugInfo> &infos);
    bool IsAttachDebug(const std::string &bundleName);
    void RemoveAppDebugInfo(const AppDebugInfo &info);

private:
    void GetIncrementAppDebugInfos(const std::vector<AppDebugInfo> &infos, std::vector<AppDebugInfo> &incrementInfos);
    std::mutex mutex_;
    std::set<sptr<IAppDebugListener>> listeners_;
    std::vector<AppDebugInfo> debugInfos_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_MGR_APP_DEBUG_MANAGER_H
