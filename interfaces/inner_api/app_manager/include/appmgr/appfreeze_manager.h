/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H

#include <sys/types.h>

#include <fstream>
#include <string>
#include <set>
#include <map>
#include <memory>
#include <vector>

#include "refbase.h"
#include "singleton.h"
#include "app_mgr_client.h"
namespace OHOS {
namespace AppExecFwk {
class AppfreezeManager : public Singleton<AppfreezeManager> {
    DECLARE_SINGLETON(AppfreezeManager)
public:    
    struct AppInfo {
        int pid;
        int uid;
        std::string bundleName;
        std::string processName;
    };
    int AppfreezeHandle(const FaultData& faultData,const AppfreezeManager::AppInfo& appInfo);
    int TimeoutHandle(const FaultData& faultData,const AppfreezeManager::AppInfo& appInfo);
    bool IsHandleAppfreeze(const std::string bundleName);
private:


    enum {
        LOGGER_BINDER_STACK_ONE = 0,
        LOGGER_BINDER_STACK_ALL = 1,
    };
    static constexpr mode_t DEFAULT_LOG_FILE_MODE = 0664;
    static const inline std::string PEERBINDER_DIR = "/data/log/eventlog";

    static const inline std::string LOGGER_BINDER_DEBUG_PROC_PATH = "/proc/transaction_proc";
    static constexpr int BP_CMD_LAYER_INDEX = 1;
    static constexpr int BP_CMD_PERF_TYPE_INDEX = 2;
    static constexpr int PERF_LOG_EXPIRE_TIME = 60;
    static constexpr size_t BP_CMD_SZ = 3;

    uint64_t GetMilliseconds();
    std::map<int, std::set<int>> BinderParser(std::ifstream& fin, std::string& stack) const;
    void ParseBinderPids(const std::map<int, std::set<int>>& binderInfo, std::set<int>& pids, int pid) const;
    std::set<int> GetBinderPeerPids(std::string& stack, int pid) const;
    std::string CatcherStacktrace(int pid) const;
    void ForkToDumpHiperf(const std::set<int>& pids, int pid);
    void DoExecHiperf(const std::string& fileName, const std::set<int>& pids, int pid);
    int AcquireStack(const FaultData& faultData, const AppInfo& appInfo);
    int NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H
