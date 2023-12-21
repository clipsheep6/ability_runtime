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

#include "ecological_rule/ability_ecological_rule_mgr_service.h"

#include "iservice_registry.h"
#include "iremote_broker.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace EcologicalRuleMgrService {

using namespace std::chrono;

static inline const std::u16string ERMS_INTERFACE_TOKEN =
    u"ohos.cloud.ecologicalrulemgrservice.IEcologicalRuleMgrService";
constexpr int32_t CYCLE_LIMIT = 1000;

std::mutex AbilityEcologicalRuleMgrServiceClient::instanceLock_;
sptr<AbilityEcologicalRuleMgrServiceClient> AbilityEcologicalRuleMgrServiceClient::instance_;
sptr<IAbilityEcologicalRuleMgrService> AbilityEcologicalRuleMgrServiceClient::ecologicalRuleMgrServiceProxy_;
sptr<IRemoteObject::DeathRecipient> AbilityEcologicalRuleMgrServiceClient::deathRecipient_;

std::string AbilityEcologicalRuleMgrServiceClient::ERMS_ORIGINAL_TARGET = "ecological_experience_original_target";

inline int64_t GetCurrentTimeMicro()
{
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

AbilityEcologicalRuleMgrServiceClient::~AbilityEcologicalRuleMgrServiceClient()
{
    if (ecologicalRuleMgrServiceProxy_ != nullptr) {
        auto remoteObj = ecologicalRuleMgrServiceProxy_->AsObject();
        if (remoteObj != nullptr) {
            remoteObj->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

sptr<AbilityEcologicalRuleMgrServiceClient> AbilityEcologicalRuleMgrServiceClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new AbilityEcologicalRuleMgrServiceClient;
        }
    }
    return instance_;
}

sptr<IAbilityEcologicalRuleMgrService> AbilityEcologicalRuleMgrServiceClient::ConnectService()
{
    sptr<ISystemAbilityManager> samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HILOG_ERROR("GetSystemAbilityManager error");
        return nullptr;
    }

    auto systemAbility = samgr->CheckSystemAbility(6105);
    if (systemAbility == nullptr) {
        HILOG_ERROR("CheckSystemAbility error, ECOLOGICALRULEMANAGERSERVICE_ID = 6105");
        return nullptr;
    }

    deathRecipient_ = new AbilityEcologicalRuleMgrServiceDeathRecipient();
    systemAbility->AddDeathRecipient(deathRecipient_);

    return iface_cast<IAbilityEcologicalRuleMgrService>(systemAbility);
}

bool AbilityEcologicalRuleMgrServiceClient::CheckConnectService()
{
    if (ecologicalRuleMgrServiceProxy_ == nullptr) {
        HILOG_WARN("redo ConnectService");
        ecologicalRuleMgrServiceProxy_ = ConnectService();
    }
    if (ecologicalRuleMgrServiceProxy_ == nullptr) {
        HILOG_ERROR("Connect SA Failed");
        return false;
    }
    return true;
}

void AbilityEcologicalRuleMgrServiceClient::OnRemoteSaDied(const wptr<IRemoteObject> &object)
{
    ecologicalRuleMgrServiceProxy_ = ConnectService();
}

int32_t AbilityEcologicalRuleMgrServiceClient::EvaluateResolveInfos(const AAFwk::Want &want,
    const AbilityCallerInfo &callerInfo, int32_t type, vector<AbilityInfo> &abilityInfos,
    const vector<AppExecFwk::ExtensionAbilityInfo> &extInfos)
{
    int64_t start = GetCurrentTimeMicro();
    HILOG_DEBUG("want: %{public}s, callerInfo: %{public}s, type: %{public}d", want.ToString().c_str(),
        callerInfo.ToString().c_str(), type);
    if (!CheckConnectService()) {
        return -1;
    }
    int32_t res = ecologicalRuleMgrServiceProxy_->EvaluateResolveInfos(want, callerInfo, type, abilityInfos);
    int64_t cost = GetCurrentTimeMicro() - start;
    HILOG_DEBUG("[ERMS-DFX] EvaluateResolveInfos interface cost %{public}lld mirco seconds.", cost);
    return res;
}

int32_t AbilityEcologicalRuleMgrServiceClient::QueryStartExperience(const OHOS::AAFwk::Want &want,
    const AbilityCallerInfo &callerInfo, AbilityExperienceRule &rule)
{
    int64_t start = GetCurrentTimeMicro();
    HILOG_DEBUG("callerInfo: %{public}s, want: %{public}s", callerInfo.ToString().c_str(), want.ToString().c_str());
    if (callerInfo.packageName.find_first_not_of(' ') == std::string::npos) {
        rule.isAllow = true;
        HILOG_DEBUG("callerInfo packageName is empty, allow = true");
        return 0;
    }

    if (!CheckConnectService()) {
        return -1;
    }
    int32_t res = ecologicalRuleMgrServiceProxy_->QueryStartExperience(want, callerInfo, rule);
    if (rule.replaceWant != nullptr) {
        rule.replaceWant->SetParam(ERMS_ORIGINAL_TARGET, want.ToString());
        HILOG_DEBUG("queryStart finish: rule.isAllow = %{public}d, rule.sceneCode = %{public}s, replaceWant = %{public}s",
            rule.isAllow, rule.sceneCode.c_str(), (*(rule.replaceWant)).ToString().c_str());
    }
    int64_t cost = GetCurrentTimeMicro() - start;
    HILOG_DEBUG("[ERMS-DFX] QueryStartExperience interface cost %{public}lld mirco seconds.", cost);
    return res;
}

void AbilityEcologicalRuleMgrServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    AbilityEcologicalRuleMgrServiceClient::GetInstance()->OnRemoteSaDied(object);
}

AbilityEcologicalRuleMgrServiceProxy::AbilityEcologicalRuleMgrServiceProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IAbilityEcologicalRuleMgrService>(object)
{}

int32_t AbilityEcologicalRuleMgrServiceProxy::EvaluateResolveInfos(const Want &want, const AbilityCallerInfo &callerInfo,
    int32_t type, std::vector<AbilityInfo> &abilityInfos)
{
    HILOG_DEBUG("called");
    MessageParcel data;

    if (!data.WriteInterfaceToken(ERMS_INTERFACE_TOKEN)) {
        HILOG_ERROR("write token failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("write want failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&callerInfo)) {
        HILOG_ERROR("write callerInfo failed");
        return ERR_FAILED;
    }

    if (!data.WriteInt32(type)) {
        HILOG_ERROR("write type failed");
        return ERR_FAILED;
    }

    if (!data.WriteInt32(abilityInfos.size())) {
        HILOG_ERROR("write abilityInfos size failed");
        return ERR_FAILED;
    }

    for (auto &abilityInfo : abilityInfos) {
        if (!data.WriteParcelable(&abilityInfo)) {
            HILOG_ERROR("write abilityInfo failed");
            return ERR_FAILED;
        }
    }

    MessageOption option = { MessageOption::TF_SYNC };
    MessageParcel reply;

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("get Remote failed.");
        return ERR_FAILED;
    }

    int32_t ret = remote->SendRequest(EVALUATE_RESOLVE_INFO_CMD, data, reply, option);
    if (ret != ERR_NONE) {
        HILOG_ERROR("SendRequest error, ret = %{public}d", ret);
        return ERR_FAILED;
    }

    if (!ReadParcelableVector(abilityInfos, reply)) {
        HILOG_ERROR("GetParcelableInfos fail");
    }
    HILOG_DEBUG("end");
    return ERR_OK;
}

template <typename T>
bool AbilityEcologicalRuleMgrServiceProxy::ReadParcelableVector(std::vector<T> &parcelableVector, MessageParcel &reply)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("size is too large.");
        return false;
    }
    parcelableVector.clear();
    for (int32_t i = 0; i < infoSize; i++) {
        sptr<T> info = reply.ReadParcelable<T>();
        if (info == nullptr) {
            HILOG_ERROR("read Parcelable infos failed");
            return false;
        }
        parcelableVector.emplace_back(*info);
    }
    return true;
}

int32_t AbilityEcologicalRuleMgrServiceProxy::QueryStartExperience(const Want &want, const AbilityCallerInfo &callerInfo,
    AbilityExperienceRule &rule)
{
    HILOG_DEBUG("called");
    MessageParcel data;

    if (!data.WriteInterfaceToken(ERMS_INTERFACE_TOKEN)) {
        HILOG_ERROR("write token failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("write want failed");
        return ERR_FAILED;
    }

    if (!data.WriteParcelable(&callerInfo)) {
        HILOG_ERROR("write callerInfo failed");
        return ERR_FAILED;
    }

    MessageOption option = { MessageOption::TF_SYNC };
    MessageParcel reply;

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("get Remote failed");
        return ERR_FAILED;
    }

    int32_t ret = remote->SendRequest(QUERY_START_EXPERIENCE_CMD, data, reply, option);
    if (ret != ERR_NONE) {
        HILOG_ERROR("SendRequest error, ret = %{public}d", ret);
        return ERR_FAILED;
    }

    sptr<AbilityExperienceRule> sptrRule = reply.ReadParcelable<AbilityExperienceRule>();
    if (sptrRule == nullptr) {
        HILOG_ERROR("ReadParcelable sptrRule error");
        return ERR_FAILED;
    }

    rule = *sptrRule;
    HILOG_DEBUG("end");
    return ERR_OK;
}
} // namespace EcologicalRuleMgrService
} // namespace OHOS
