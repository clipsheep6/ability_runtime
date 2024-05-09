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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H

#include <memory>
#include <string>
#include <sys/types.h>

#include "app_death_recipient.h"
#include "child_scheduler_interface.h"

namespace OHOS {
namespace AppExecFwk {
class AppRunningRecord;

class ChildProcessRecord {
public:
    ChildProcessRecord(pid_t hostPid, const std::string &srcEntry, const std::shared_ptr<AppRunningRecord> hostRecord,
        int32_t childProcessCount, bool isStartWithDebug);
    virtual ~ChildProcessRecord();

    static std::shared_ptr<ChildProcessRecord> CreateChildProcessRecord(pid_t hostPid, const std::string &srcEntry,
        const std::shared_ptr<AppRunningRecord> hostRecord, int32_t childProcessCount, bool isStartWithDebug);

    void SetPid(pid_t pid);
    pid_t GetPid() const;
    pid_t GetHostPid() const;
    void SetUid(int32_t uid);
    int32_t GetUid() const;
    std::string GetProcessName() const;
    std::string GetSrcEntry() const;
    std::shared_ptr<AppRunningRecord> GetHostRecord() const;
    void SetScheduler(const sptr<IChildScheduler> &scheduler);
    sptr<IChildScheduler> GetScheduler() const;
    void SetDeathRecipient(const sptr<AppDeathRecipient> recipient);
    void RegisterDeathRecipient();
    void RemoveDeathRecipient();
    void ScheduleExitProcessSafely();
    bool isStartWithDebug();
private:
    void MakeProcessName(const std::shared_ptr<AppRunningRecord> hostRecord);

    pid_t pid_ = 0;
    pid_t hostPid_ = 0;
    int32_t uid_ = 0;
    int32_t childProcessCount_ = 0;
    std::string processName_;
    std::string srcEntry_;
    std::weak_ptr<AppRunningRecord> hostRecord_;
    sptr<IChildScheduler> scheduler_ = nullptr;
    sptr<AppDeathRecipient> deathRecipient_ = nullptr;
    bool isStartWithDebug_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_RECORD_H
