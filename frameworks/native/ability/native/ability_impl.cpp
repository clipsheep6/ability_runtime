/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_impl.h"

#include "hitrace_meter.h"
#include "data_ability_predicates.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string PERMISSION_KEY = "ohos.user.grant.permission";
const std::string GRANTED_RESULT_KEY = "ohos.user.grant.permission.result";
}

void AbilityImpl::Init(std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &record,
    std::shared_ptr<Ability> &ability, std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token,
    std::shared_ptr<ContextDeal> &contextDeal)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("AbilityImpl::init begin");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        ability == nullptr || contextDeal == nullptr) {
        HILOG_ERROR("AbilityImpl::init failed, token is nullptr, application is nullptr, handler is nullptr, record is "
                 "nullptr, ability is nullptr, contextDeal is nullptr");
        return;
    }

    token_ = record->GetToken();
    record->SetAbilityImpl(shared_from_this());
    ability_ = ability;
    handler_ = handler;
    auto info = record->GetAbilityInfo();
    isStageBasedModel_ = info && info->isStageBasedModel;
#ifdef SUPPORT_GRAPHICS
    if (info && info->type == AbilityType::PAGE) {
        ability_->SetSceneListener(
            sptr<WindowLifeCycleImpl>(new WindowLifeCycleImpl(token_, shared_from_this())));
    }
#endif
    ability_->Init(record->GetAbilityInfo(), application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    contextDeal_ = contextDeal;
    HILOG_INFO("AbilityImpl::init end");
}

void AbilityImpl::Start(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Start ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AbilityType::PAGE) &&
        (!ability_->GetAbilityInfo()->isStageBasedModel)) {
        ability_->HandleCreateAsContinuation(want);
    }
#endif
    HILOG_INFO("AbilityImpl::Start");
    ability_->OnStart(want);
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
    } else {
#endif
        if (ability_->GetAbilityInfo()->type == AbilityType::DATA) {
            lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        } else {
            lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
        }
#ifdef SUPPORT_GRAPHICS
    }
#endif

    abilityLifecycleCallbacks_->OnAbilityStart(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::Stop()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Stop ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnStop();
#ifdef SUPPORT_GRAPHICS
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
    } else {
#endif
        lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#ifdef SUPPORT_GRAPHICS
    }
#endif
    abilityLifecycleCallbacks_->OnAbilityStop(ability_);
    ability_->DestroyInstance(); // Release window and ability.
    ability_ = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::Active()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Active ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnActive();
#ifdef SUPPORT_GRAPHICS
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_INACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(true);
        ability_->OnWindowFocusChanged(true);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::Inactive()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Inactive ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnInactive();
#ifdef SUPPORT_GRAPHICS
    if ((lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) && (ability_->GetAbilityInfo()->type == AbilityType::PAGE)) {
        ability_->OnTopActiveAbilityChanged(false);
        ability_->OnWindowFocusChanged(false);
    }
#endif
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    abilityLifecycleCallbacks_->OnAbilityInactive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

bool AbilityImpl::IsStageBasedModel() const
{
    return isStageBasedModel_;
}

void AbilityImpl::DispatchSaveAbilityState()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::DispatchSaveAbilityState ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    needSaveDate_ = true;
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::DispatchRestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::DispatchRestoreAbilityState ability_ is nullptr");
        return;
    }

    hasSaveData_ = true;
    restoreData_ = inState;
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{}

sptr<IRemoteObject> AbilityImpl::ConnectAbility(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ConnectAbility ability_ is nullptr");
        return nullptr;
    }

    HILOG_INFO("AbilityImpl:: ConnectAbility");
    sptr<IRemoteObject> object = ability_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);

    return object;
}

void AbilityImpl::DisconnectAbility(const Want &want)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("Disconnect ability error, ability_ is nullptr.");
        return;
    }
    HILOG_INFO("Disconnect ability begin, ability:%{public}s.", ability_->GetAbilityName().c_str());
    ability_->OnDisconnect(want);
}

void AbilityImpl::CommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CommandAbility ability_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl:: CommandAbility");
    ability_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    abilityLifecycleCallbacks_->OnAbilityActive(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

int AbilityImpl::GetCurrentState()
{
    return lifecycleState_;
}

void AbilityImpl::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SendResult ability_ is nullptr");
        return;
    }

    if (resultData.HasParameter(PERMISSION_KEY)) {
        std::vector<std::string> permissions = resultData.GetStringArrayParam(PERMISSION_KEY);
        std::vector<int> grantedResult(permissions.size(), -1);
        if (resultCode > 0) {
            grantedResult = resultData.GetIntArrayParam(GRANTED_RESULT_KEY);
            HILOG_INFO("%{public}s Get user granted result.", __func__);
        }
        ability_->OnRequestPermissionsFromUserResult(requestCode, permissions, grantedResult);
    } else {
        ability_->OnAbilityResult(requestCode, resultCode, resultData);
    }

    // for api5 FeatureAbility::startAbilityForResult
    ability_->OnFeatureAbilityResult(requestCode, resultCode, resultData);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::NewWant(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::NewWant ability_ is nullptr");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_GRAPHICS
    ability_->ContinuationRestore(want);
#endif
    HILOG_INFO("%{public}s end.", __func__);
}

std::vector<std::string> AbilityImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("AbilityImpl::GetFileTypes");
    std::vector<std::string> types;
    return types;
}

int AbilityImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("AbilityImpl::OpenFile");
    return -1;
}

int AbilityImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("AbilityImpl::OpenRawFile");
    return -1;
}

int AbilityImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("AbilityImpl::Insert");
    return -1;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    HILOG_INFO("AbilityImpl::Call");
    return nullptr;
}

int AbilityImpl::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityImpl::Update");
    return -1;
}

int AbilityImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityImpl::Delete");
    return -1;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityImpl::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("AbilityImpl::Query");
    return nullptr;
}

std::string AbilityImpl::GetType(const Uri &uri)
{
    HILOG_INFO("AbilityImpl::GetType");
    return "";
}

bool AbilityImpl::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

int AbilityImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("AbilityImpl::BatchInsert");
    return -1;
}

void AbilityImpl::SerUriString(const std::string &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (contextDeal_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SerUriString contextDeal_ is nullptr");
        return;
    }
    contextDeal_->SerUriString(uri);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (contextDeal_ == nullptr) {
        HILOG_ERROR("AbilityImpl::SetLifeCycleStateInfo contextDeal_ is nullptr");
        return;
    }
    contextDeal_->SetLifeCycleStateInfo(info);
}

bool AbilityImpl::CheckAndRestore()
{
    HILOG_INFO("AbilityImpl::CheckAndRestore called start");
    if (!hasSaveData_) {
        HILOG_ERROR("AbilityImpl::CheckAndRestore hasSaveData_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CheckAndRestore ability_ is nullptr");
        return false;
    }

    HILOG_INFO("AbilityImpl::CheckAndRestore ready to restore");
    ability_->OnRestoreAbilityState(restoreData_);

    HILOG_INFO("AbilityImpl::CheckAndRestore called end");
    return true;
}

bool AbilityImpl::CheckAndSave()
{
    HILOG_INFO("AbilityImpl::CheckAndSave called start");
    if (!needSaveDate_) {
        HILOG_ERROR("AbilityImpl::CheckAndSave needSaveDate_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CheckAndSave ability_ is nullptr");
        return false;
    }

    HILOG_INFO("AbilityImpl::CheckAndSave ready to save");
    ability_->OnSaveAbilityState(restoreData_);
    abilityLifecycleCallbacks_->OnAbilitySaveState(restoreData_);

    needSaveDate_ = false;

    HILOG_INFO("AbilityImpl::CheckAndSave called end");
    return true;
}

PacMap &AbilityImpl::GetRestoreData()
{
    return restoreData_;
}

void AbilityImpl::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    if (ability_ != nullptr) {
        ability_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    }
}

Uri AbilityImpl::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("AbilityImpl::NormalizeUri");
    return uri;
}

Uri AbilityImpl::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("AbilityImpl::DenormalizeUri");
    return uri;
}

void AbilityImpl::ScheduleUpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ScheduleUpdateConfiguration ability_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        HILOG_INFO("ability name: [%{public}s]", ability_->GetAbilityName().c_str());
        ability_->OnConfigurationUpdatedNotify(config);
    }

    HILOG_INFO("%{public}s end.", __func__);
}

std::shared_ptr<AbilityPostEventTimeout> AbilityImpl::CreatePostEventTimeouter(std::string taskstr)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::CreatePostEventTimeouter ability_ is nullptr");
        return nullptr;
    }

    return ability_->CreatePostEventTimeouter(taskstr);
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_INFO("AbilityImpl::ExecuteBatch");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    return results;
}

void AbilityImpl::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::ContinueAbility ability_ is nullptr");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void AbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::NotifyContinuationResult ability_ is nullptr");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void AbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("AbilityImpl::NotifyMemoryLevel ability_ is nullptr");
        return;
    }
    ability_->OnMemoryLevel(level);
}

#ifdef SUPPORT_GRAPHICS
void AbilityImpl::AfterUnFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (!ability_ || !ability_->GetAbilityInfo() || !contextDeal_ || !handler_) {
        HILOG_ERROR("AbilityImpl::AfterUnFocused failed");
        return;
    }

    if (ability_->GetAbilityInfo()->isStageBasedModel) {
        HILOG_INFO("new version ability, do nothing when after unfocused.");
        return;
    }

    HILOG_INFO("old version ability, window after unfocused.");
    auto task = [abilityImpl = shared_from_this(), want = *(ability_->GetWant()), contextDeal = contextDeal_]() {
        auto info = contextDeal->GetLifeCycleStateInfo();
        info.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
        info.isNewWant = false;
        abilityImpl->HandleAbilityTransaction(want, info);
    };
    handler_->PostTask(task);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::AfterFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (!ability_ || !ability_->GetAbilityInfo() || !contextDeal_ || !handler_) {
        HILOG_ERROR("AbilityImpl::AfterFocused failed");
        return;
    }

    if (ability_->GetAbilityInfo()->isStageBasedModel) {
        HILOG_INFO("new version ability, do nothing when after focused.");
        return;
    }

    HILOG_INFO("fa mode ability, window after focused.");
    auto task = [abilityImpl = shared_from_this(), want = *(ability_->GetWant()), contextDeal = contextDeal_]() {
        auto info = contextDeal->GetLifeCycleStateInfo();
        info.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
        info.isNewWant = false;
        abilityImpl->HandleAbilityTransaction(want, info);
    };
    handler_->PostTask(task);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("Stage mode ability, window after foreground, notify ability manager service.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("new version ability, window after background.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::WindowLifeCycleImpl::ForegroundFailed()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("The ability is stage mode, schedule foreground failed.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_FAILED, restoreData);
}

void AbilityImpl::WindowLifeCycleImpl::ForegroundInvalidMode()
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto owner = owner_.lock();
    if (owner && !owner->IsStageBasedModel()) {
        return;
    }

    HILOG_INFO("The ability is stage mode, schedule foreground invalid mode.");
    PacMap restoreData;
    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_,
        AbilityLifeCycleState::ABILITY_STATE_INVALID_WINDOW_MODE, restoreData);
}

void AbilityImpl::Foreground(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Foreground ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl::Foreground");
    ability_->OnForeground(want);
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::Background()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("AbilityImpl::Background ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_INFO("AbilityImpl::Background");
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    if ((ability_->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE) &&
        (ability_->GetAbilityInfo()->isStageBasedModel)) {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    } else {
        lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND;
    }
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    HILOG_INFO("%{public}s end.", __func__);
}

void AbilityImpl::DoKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_INFO("AbilityImpl::DoKeyDown called");
}

void AbilityImpl::DoKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_INFO("AbilityImpl::DoKeyUp called");
}

void AbilityImpl::DoPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    HILOG_INFO("AbilityImpl::DoPointerEvent called");
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    int32_t code = keyEvent->GetKeyAction();
    if (code == MMI::KeyEvent::KEY_ACTION_DOWN) {
        abilityImpl_->DoKeyDown(keyEvent);
        HILOG_INFO("AbilityImpl::OnKeyDown keyAction: %{public}d.", code);
    } else if (code == MMI::KeyEvent::KEY_ACTION_UP) {
        abilityImpl_->DoKeyUp(keyEvent);
        HILOG_INFO("AbilityImpl::DoKeyUp keyAction: %{public}d.", code);
    }
}

void AbilityImpl::InputEventConsumerImpl::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    HILOG_INFO("AbilityImpl::DoPointerEvent called.");
    abilityImpl_->DoPointerEvent(pointerEvent);
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
