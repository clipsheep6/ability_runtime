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

#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H

#include <sys/types.h>

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "fault_data.h"
#include "freeze_util.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
class AppfreezeManager : public std::enable_shared_from_this<AppfreezeManager> {
public:
    struct AppInfo {
        int pid;
        int uid;
        std::string bundleName;
        std::string processName;
    };

    enum TypeAttribute {
        NORMAL_TIMEOUT = 0,
        CRITICAL_TIMEOUT = 1,
    };

    enum AppFreezeState {
        APPFREEZE_STATE_IDLE = 0,
        APPFREEZE_STATE_FREEZE = 1,
        APPFREEZE_STATE_CANCELING = 2,
        APPFREEZE_STATE_CANCELED = 3,
    };

    struct AppFreezeInfo {
        int32_t pid = 0;
        int state = 0;
        int64_t occurTime = 0;
    };

    struct ParamInfo {
        int typeId = TypeAttribute::NORMAL_TIMEOUT;
        int32_t pid = 0;
        std::string eventName;
        std::string bundleName;
        std::string msg;
    };

    AppfreezeManager();
    ~AppfreezeManager();

    static std::shared_ptr<AppfreezeManager> GetInstance();
    static void DestroyInstance();
    int AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo);
    int LifecycleTimeoutHandle(const ParamInfo& info, std::unique_ptr<FreezeUtil::LifecycleFlow> flow = nullptr);
    std::string WriteToFile(const std::string& fileName, std::string& content);
    bool IsHandleAppfreeze(const std::string& bundleName);
    bool IsProcessDebug(int32_t pid, std::string processName);
    bool IsNeedIgnoreFreezeEvent(int32_t pid);
    void DeleteStack(int pid);
    bool CancelAppFreezeDetect(int32_t pid);
    void RemoveDeathProcess(std::string processName);
    void ResetAppfreezeState(int32_t pid);
    bool IsValidFreezeFilter(int32_t pid);

private:
    AppfreezeManager& operator=(const AppfreezeManager&) = delete;
    AppfreezeManager(const AppfreezeManager&) = delete;
    uint64_t GetMilliseconds();
    std::map<int, std::set<int>> BinderParser(std::ifstream& fin, std::string& stack) const;
    void ParseBinderPids(const std::map<int, std::set<int>>& binderInfo, std::set<int>& pids, int pid, int layer) const;
    std::set<int> GetBinderPeerPids(std::string& stack, int pid) const;
    void FindStackByPid(std::string& ret, int pid, const std::string& msg) const;
    std::string CatchJsonStacktrace(int pid, const std::string& faultType) const;
    std::string CatcherStacktrace(int pid) const;
    int AcquireStack(const FaultData& faultData, const AppInfo& appInfo);
    int NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo, const std::string& binderInfo);
    int64_t GetFreezeCurrentTime();
    void SetFreezeState(int32_t pid, int state);
    int GetFreezeState(int32_t pid);
    int64_t GetFreezeTime(int32_t pid);
    void ClearOldInfo();
    std::string FormatCmdLine(const std::string& cmdLine);
    std::string GetProcessName(int32_t pid);
    std::string GetBundleName(int32_t pid);

    static const inline std::string LOGGER_DEBUG_PROC_PATH = "/proc/transaction_proc";
    std::string name_;
    static ffrt::mutex singletonMutex_;
    static std::shared_ptr<AppfreezeManager> instance_;
    static ffrt::mutex freezeMutex_;
    std::map<int32_t, AppFreezeInfo> appfreezeInfo_;
    static ffrt::mutex catchStackMutex_;
    static std::map<int, std::string> catchStackMap_;
    static ffrt::mutex freezeFilterMutex_;
    std::map<std::string, AppFreezeInfo> appfreezeFilterMap_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPFREEZE_MAMAGER_H