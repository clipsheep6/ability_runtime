/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
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
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityRecord is nullptr");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo is nullptr");
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
        TAG_LOGE(AAFwkTag::ABILITY, "thread is nullptr");
        return;
    }
    thread->Attach(application, abilityRecord, mainRunner, stageContext);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityThread::AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityRecord is nullptr");
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo is nullptr");
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
        TAG_LOGE(AAFwkTag::ABILITY, "thread is nullptr");
        return;
    }
    thread->Attach(application, abilityRecord, stageContext);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &targetState, sptr<SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleShareData(const int32_t &requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

bool AbilityThread::SchedulePrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

void AbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::ScheduleRestoreAbilityState(const PacMap &state)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

std::vector<std::string> AbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::vector<std::string> types;
    return types;
}

int AbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::OpenRawFile(const Uri &uri, const std::string &mode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityThread::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return nullptr;
}

int AbilityThread::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

int AbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityThread::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return nullptr;
}

std::string AbilityThread::GetType(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return "";
}

bool AbilityThread::Reload(const Uri &uri, const PacMap &extras)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

int AbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

void AbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

Uri AbilityThread::NormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return uri;
}

Uri AbilityThread::DenormalizeUri(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return uri;
}

bool AbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

bool AbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

bool AbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

void AbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void AbilityThread::OnExecuteIntent(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> AbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

int AbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_INVALID_VALUE;
}

void AbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityThread::BlockAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (abilityHandler_) {
        auto task = []() {
            while (1) {
                std::this_thread::sleep_for(BLOCK_ABILITY_TIME * 1s);
            }
        };
        abilityHandler_->PostTask(task, "AbilityThread:BlockAbility");
        TAG_LOGD(AAFwkTag::ABILITY, "end");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}
#endif
} // namespace AppExecFwk
} // namespace OHOS
