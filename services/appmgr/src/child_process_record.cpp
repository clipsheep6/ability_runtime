/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ChildProcessRecord::ChildProcessRecord(pid_t hostPid, const ChildProcessRequest &request,
    const std::shared_ptr<AppRunningRecord> hostRecord)
    : hostPid_(hostPid), childProcessCount_(request.childProcessCount), childProcessType_(request.childProcessType),
    srcEntry_(request.srcEntry), hostRecord_(hostRecord), isStartWithDebug_(request.isStartWithDebug)
{
    MakeProcessName(hostRecord);
}

ChildProcessRecord::ChildProcessRecord(pid_t hostPid, const std::string &libName,
    const std::shared_ptr<AppRunningRecord> hostRecord, const sptr<IRemoteObject> &mainProcessCb,
    int32_t childProcessCount, bool isStartWithDebug)
    : hostPid_(hostPid), childProcessCount_(childProcessCount), childProcessType_(CHILD_PROCESS_TYPE_NATIVE),
    srcEntry_(libName), hostRecord_(hostRecord), mainProcessCb_(mainProcessCb), isStartWithDebug_(isStartWithDebug)
{
    MakeProcessName(hostRecord);
}

ChildProcessRecord::~ChildProcessRecord()
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
}

std::shared_ptr<ChildProcessRecord> ChildProcessRecord::CreateChildProcessRecord(pid_t hostPid,
    const ChildProcessRequest &request, const std::shared_ptr<AppRunningRecord> hostRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "hostPid: %{public}d, srcEntry: %{priavte}s,", hostPid, request.srcEntry.c_str());
    if (hostPid <= 0 || request.srcEntry.empty() || !hostRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid parameter.");
        return nullptr;
    }
    return std::make_shared<ChildProcessRecord>(hostPid, request, hostRecord);
}

std::shared_ptr<ChildProcessRecord> ChildProcessRecord::CreateNativeChildProcessRecord(
    pid_t hostPid, const std::string &libName, const std::shared_ptr<AppRunningRecord> hostRecord,
    const sptr<IRemoteObject> &mainProcessCb, int32_t childProcessCount, bool isStartWithDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "hostPid: %{public}d, libName: %{public}s", hostPid, libName.c_str());
    if (hostPid <= 0 || libName.empty() || !hostRecord || !mainProcessCb) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid parameter.");
        return nullptr;
    }
    return std::make_shared<ChildProcessRecord>(hostPid, libName, hostRecord, mainProcessCb,
        childProcessCount, isStartWithDebug);
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
        TAG_LOGE(AAFwkTag::APPMGR, "scheduler_ or deathRecipient_ is null.");
        return;
    }
    auto obj = scheduler_->AsObject();
    if (!obj || !obj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AddDeathRecipient failed.");
    }
}

void ChildProcessRecord::RemoveDeathRecipient()
{
    if (!scheduler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "scheduler_ is null.");
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
        TAG_LOGE(AAFwkTag::APPMGR, "scheduler_ is null.");
        return;
    }
    scheduler_->ScheduleExitProcessSafely();
}

void ChildProcessRecord::MakeProcessName(const std::shared_ptr<AppRunningRecord> hostRecord)
{
    if (!hostRecord) {
        TAG_LOGW(AAFwkTag::APPMGR, "hostRecord empty.");
        return;
    }
    processName_ = hostRecord->GetBundleName();
    if (srcEntry_.empty()) {
        TAG_LOGW(AAFwkTag::APPMGR, "srcEntry empty.");
        return;
    }
    std::string filename = std::filesystem::path(srcEntry_).stem();
    if (!filename.empty()) {
        processName_.append(":");
        if (childProcessType_ == CHILD_PROCESS_TYPE_NATIVE) {
            processName_.append("Native_");
        }
        
        processName_.append(filename);
    }
    processName_.append(std::to_string(childProcessCount_));
    TAG_LOGD(AAFwkTag::APPMGR, "MakeSpawnForkProcessName processName is %{public}s", processName_.c_str());
}

bool ChildProcessRecord::isStartWithDebug()
{
    return isStartWithDebug_;
}

int32_t ChildProcessRecord::GetChildProcessType() const
{
    return childProcessType_;
}

sptr<IRemoteObject> ChildProcessRecord::GetMainProcessCallback() const
{
    return mainProcessCb_;
}

void ChildProcessRecord::ClearMainProcessCallback()
{
    mainProcessCb_.clear();
}

void ChildProcessRecord::SetEntryParams(const std::string &entryParams)
{
    entryParams_ = entryParams;
}

std::string ChildProcessRecord::GetEntryParams() const
{
    return entryParams_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
