/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ability_thread.h"

#include "extension_ability_thread.h"
#include "fa_ability_thread.h"
#include "ui_ability_thread.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
#ifdef ABILITY_COMMAND_FOR_TEST
const int32_t BLOCK_ABILITY_TIME = 20;
#endif
void AbilityThread::AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    sptr<AbilityThread> thread = nullptr;
    if (abilityInfo->type == AbilityType::PAGE && abilityInfo->isStageBasedModel) {
        thread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        thread = new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    } else {
        thread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    }
    if (thread == nullptr) {
        HILOG_ERROR("thread is nullptr");
        return;
    }
    thread->Attach(application, abilityRecord, mainRunner, stageContext);
    HILOG_DEBUG("end");
}

void AbilityThread::AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    sptr<AbilityThread> thread = nullptr;
    if (abilityInfo->type == AbilityType::PAGE && abilityInfo->isStageBasedModel) {
        thread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        thread = new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    } else {
        thread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    }
    if (thread == nullptr) {
        HILOG_ERROR("thread is nullptr");
        return;
    }
    thread->Attach(application, abilityRecord, stageContext);
    HILOG_DEBUG("end");
}

void AbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &targetState, sptr<SessionInfo> sessionInfo)
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleShareData(const int32_t &requestCode)
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleConnectAbility(const Want &want)
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("called");
}

bool AbilityThread::SchedulePrepareTerminateAbility()
{
    HILOG_DEBUG("called");
    return false;
}

void AbilityThread::ScheduleSaveAbilityState()
{
    HILOG_DEBUG("called");
}

void AbilityThread::ScheduleRestoreAbilityState(const PacMap &state)
{
    HILOG_DEBUG("called");
}

void AbilityThread::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("called");
}

std::vector<std::string> AbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_DEBUG("called");
    std::vector<std::string> types;
    return types;
}

int AbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityThread::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    HILOG_DEBUG("called");
    return nullptr;
}

int AbilityThread::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityThread::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("called");
    return nullptr;
}

std::string AbilityThread::GetType(const Uri &uri)
{
    HILOG_DEBUG("called");
    return "";
}

bool AbilityThread::Reload(const Uri &uri, const PacMap &extras)
{
    HILOG_DEBUG("called");
    return false;
}

int AbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

void AbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    HILOG_DEBUG("called");
}

void AbilityThread::NotifyContinuationResult(int32_t result)
{
    HILOG_DEBUG("called");
}

Uri AbilityThread::NormalizeUri(const Uri &uri)
{
    HILOG_DEBUG("called");
    return uri;
}

Uri AbilityThread::DenormalizeUri(const Uri &uri)
{
    HILOG_DEBUG("called");
    return uri;
}

bool AbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_DEBUG("called");
    return false;
}

bool AbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_DEBUG("called");
    return false;
}

bool AbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    HILOG_DEBUG("called");
    return false;
}

void AbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("called");
}

void AbilityThread::CallRequest()
{
    HILOG_DEBUG("called");
}

void AbilityThread::OnExecuteIntent(const Want &want)
{
    HILOG_DEBUG("called");
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> AbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_DEBUG("called");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

int AbilityThread::CreateModalUIExtension(const Want &want)
{
    HILOG_DEBUG("called");
    return ERR_INVALID_VALUE;
}

void AbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    HILOG_DEBUG("called");
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityThread::BlockAbility()
{
    HILOG_DEBUG("begin");
    if (abilityHandler_) {
        auto task = []() {
            while (1) {
                std::this_thread::sleep_for(BLOCK_ABILITY_TIME * 1s);
            }
        };
        abilityHandler_->PostTask(task, "AbilityThread:BlockAbility");
        HILOG_DEBUG("end");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}
#endif
} // namespace AppExecFwk
} // namespace OHOS
