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
#include "child_process_record.h"

#include <filesystem>

#include "app_running_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ChildProcessRecord::ChildProcessRecord(pid_t hostPid, const std::string &srcEntry,
    const std::shared_ptr<AppRunningRecord> hostRecord)
    : hostPid_(hostPid), srcEntry_(srcEntry), hostRecord_(hostRecord)
{
    MakeProcessName(hostRecord);
}

ChildProcessRecord::~ChildProcessRecord()
{
    HILOG_DEBUG("Called.");
}

std::shared_ptr<ChildProcessRecord> ChildProcessRecord::CreateChildProcessRecord(pid_t hostPid,
    const std::string &srcEntry, const std::shared_ptr<AppRunningRecord> hostRecord)
{
    HILOG_DEBUG("hostPid: %{public}d, srcEntry: %{public}s", hostPid, srcEntry.c_str());
    if (hostPid <= 0 || srcEntry.empty() || !hostRecord) {
        HILOG_ERROR("Invalid parameter.");
        return nullptr;
    }
    return std::make_shared<ChildProcessRecord>(hostPid, srcEntry, hostRecord);
}

void ChildProcessRecord::SetPid(pid_t pid)
{
    pid_ = pid;
}

pid_t ChildProcessRecord::GetPid() const
{
    return pid_;
}

pid_t ChildProcessRecord::GetHostPid() const
{
    return hostPid_;
}

void ChildProcessRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t ChildProcessRecord::GetUid() const
{
    return uid_;
}

std::string ChildProcessRecord::GetProcessName() const
{
    return processName_;
}

std::string ChildProcessRecord::GetSrcEntry() const
{
    return srcEntry_;
}

std::shared_ptr<AppRunningRecord> ChildProcessRecord::GetHostRecord() const
{
    return hostRecord_.lock();
}

void ChildProcessRecord::SetScheduler(const sptr<IChildScheduler> &scheduler)
{
    scheduler_ = scheduler;
}

sptr<IChildScheduler> ChildProcessRecord::GetScheduler() const
{
    return scheduler_;
}

void ChildProcessRecord::SetDeathRecipient(const sptr<AppDeathRecipient> recipient)
{
    deathRecipient_ = recipient;
}

void ChildProcessRecord::RegisterDeathRecipient()
{
    if (scheduler_ == nullptr || deathRecipient_ == nullptr) {
        HILOG_ERROR("scheduler_ or deathRecipient_ is null.");
        return;
    }
    auto obj = scheduler_->AsObject();
    if (!obj || !obj->AddDeathRecipient(deathRecipient_)) {
        HILOG_ERROR("AddDeathRecipient failed.");
    }
}

void ChildProcessRecord::RemoveDeathRecipient()
{
    if (!scheduler_) {
        HILOG_ERROR("scheduler_ is null.");
        return;
    }
    auto object = scheduler_->AsObject();
    if (object) {
        object->RemoveDeathRecipient(deathRecipient_);
    }
}

void ChildProcessRecord::ScheduleExitProcessSafely()
{
    if (!scheduler_) {
        HILOG_ERROR("scheduler_ is null.");
        return;
    }
    scheduler_->ScheduleExitProcessSafely();
}

void ChildProcessRecord::MakeProcessName(const std::shared_ptr<AppRunningRecord> hostRecord)
{
    if (!hostRecord) {
        HILOG_WARN("hostRecord empty.");
        return;
    }
    processName_ = hostRecord->GetBundleName();
    if (srcEntry_.empty()) {
        HILOG_WARN("srcEntry empty.");
        return;
    }
    std::string filename = std::filesystem::path(srcEntry_).stem();
    if (!filename.empty()) {
        processName_.append(":");
        processName_.append(filename);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
