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

#include "ams_st_service_ability_g1.h"
#include "hilog_wrapper.h"
#include "ability_context.h"
#include "ability.h"

using namespace OHOS::EventFwk;

namespace OHOS {
namespace AppExecFwk {
int AmsStServiceAbilityG1::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
std::map<std::string, AmsStServiceAbilityG1::func> AmsStServiceAbilityG1::funcMap_ = {
    {"StartOtherAbility", &AmsStServiceAbilityG1::StartOtherAbility},
    {"ConnectOtherAbility", &AmsStServiceAbilityG1::ConnectOtherAbility},
    {"DisConnectOtherAbility", &AmsStServiceAbilityG1::DisConnectOtherAbility},
};

AmsStServiceAbilityG1::~AmsStServiceAbilityG1()
{
    CommonEventManager::UnSubscribeCommonEvent(subscriber_);
}

std::vector<std::string> AmsStServiceAbilityG1::Split(std::string str, const std::string &token)
{
    HILOG_INFO("AmsStServiceAbilityG1::Split");

    std::vector<std::string> splitString;
    while (str.size()) {
        size_t index = str.find(token);
        if (index != std::string::npos) {
            splitString.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0) {
                splitString.push_back(str);
            }
        } else {
            splitString.push_back(str);
            str = "";
        }
    }
    return splitString;
}

void AmsStServiceAbilityG1::StartOtherAbility()
{
    HILOG_INFO("AmsStServiceAbilityG1::StartOtherAbility begin targetBundle=%{public}s, targetAbility=%{public}s",
        targetBundle_.c_str(),
        targetAbility_.c_str());
    HILOG_INFO("AmsStServiceAbilityG1::StartOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    if (!targetBundle_.empty() && !targetAbility_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundle_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbility_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("nextTargetBundleConn", nextTargetBundleConn_);
            want.SetParam("nextTargetAbilityConn", nextTargetAbilityConn_);
            StartAbility(want);
            sleep(1);
        }
    }
}

void AmsStServiceAbilityG1::ConnectOtherAbility()
{
    HILOG_INFO(
        "AmsStServiceAbilityG1::ConnectOtherAbility begin targetBundleConn=%{public}s, targetAbilityConn=%{public}s",
        targetBundleConn_.c_str(),
        targetAbilityConn_.c_str());
    HILOG_INFO("AmsStServiceAbilityG1::ConnectOtherAbility begin nextTargetBundleConn=%{public}s, "
             "nextTargetAbilityConn=%{public}s",
        nextTargetBundleConn_.c_str(),
        nextTargetAbilityConn_.c_str());

    // connect service ability
    if (!targetBundleConn_.empty() && !targetAbilityConn_.empty()) {
        std::vector<std::string> strtargetBundles = Split(targetBundleConn_, ",");
        std::vector<std::string> strTargetAbilitys = Split(targetAbilityConn_, ",");
        for (size_t i = 0; i < strtargetBundles.size() && i < strTargetAbilitys.size(); i++) {
            Want want;
            want.SetElementName(strtargetBundles[i], strTargetAbilitys[i]);
            want.SetParam("shouldReturn", shouldReturn_);
            want.SetParam("targetBundle", nextTargetBundle_);
            want.SetParam("targetAbility", nextTargetAbility_);
            want.SetParam("nextTargetBundleConn", nextTargetBundleConn_);
            want.SetParam("nextTargetAbilityConn", nextTargetAbilityConn_);
            stub_.push_back(new (std::nothrow) AbilityConnectCallback());
            connCallback_.push_back(new (std::nothrow) AbilityConnectionProxy(stub_[i]));
            HILOG_INFO("AmsStServiceAbilityG1::ConnectOtherAbility->ConnectAbility");
            bool ret = ConnectAbility(want, connCallback_[i]);
            sleep(1);
            if (!ret) {
                HILOG_ERROR("AmsStServiceAbilityG1::ConnectAbility failed!");
            }
        }
    }
    HILOG_INFO("AmsStServiceAbilityG1::ConnectOtherAbility end");
}

void AmsStServiceAbilityG1::OnStart(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityG1::OnStart");

    GetWantInfo(want);
    Ability::OnStart(want);
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnStart");
    SubscribeEvent();
}

void AmsStServiceAbilityG1::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HILOG_INFO("AmsStServiceAbilityG1::OnCommand");

    GetWantInfo(want);
    Ability::OnCommand(want, restart, startId);
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnCommand");
}

void AmsStServiceAbilityG1::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityG1::OnNewWant");

    GetWantInfo(want);
    Ability::OnNewWant(want);
}

void AmsStServiceAbilityG1::DisConnectOtherAbility()
{
    HILOG_INFO("AmsStServiceAbilityG1::DisConnectOtherAbility begin");
    for (auto &callback : connCallback_) {
        DisconnectAbility(callback);
        sleep(1);
    }
    HILOG_INFO("AmsStServiceAbilityG1::DisConnectOtherAbility end");
}

void AmsStServiceAbilityG1::OnStop()
{
    HILOG_INFO("AmsStServiceAbilityG1::onStop");

    Ability::OnStop();
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INITIAL, "OnStop");
}

void AmsStServiceAbilityG1::GetDataByDataAbility()
{
    std::shared_ptr<DataAbilityHelper> helper = DataAbilityHelper::Creator(std::shared_ptr<Context>(this));
    if (helper == nullptr) {
        HILOG_ERROR("AmsStServiceAbilityG1::GetDataByDataAbility:helper == nullptr");
        return;
    }

    Uri uri2("dataability:///com.ohos.amsst.service.appF.dataability");
    std::string mimeTypeFilter("mimeTypeFiltertest");
    std::vector<std::string> result = helper->GetFileTypes(uri2, mimeTypeFilter);

    int count = result.size();
    if (count > 0) {
        HILOG_INFO("AmsStServiceAbilityG1::OnBackground get data ability data info result > 0!");
        PublishEvent(APP_G1_RESP_EVENT_NAME, 1, "GetDataByDataAbility");
    } else {
        HILOG_INFO("AmsStServiceAbilityG1::OnBackground get data ability data info result = 0!");
        PublishEvent(APP_G1_RESP_EVENT_NAME, 0, "GetDataByDataAbility");
    }
}

void AmsStServiceAbilityG1::OnActive()
{
    HILOG_INFO("AmsStServiceAbilityG1::OnActive");

    Ability::OnActive();
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnActive");
}

void AmsStServiceAbilityG1::OnInactive()
{
    HILOG_INFO("AmsStServiceAbilityG1::OnInactive");

    Ability::OnInactive();
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::INACTIVE, "OnInactive");
}

void AmsStServiceAbilityG1::OnBackground()
{
    HILOG_INFO("AmsStServiceAbilityG1::OnBackground");

    Ability::OnBackground();
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnBackground");
}

sptr<IRemoteObject> AmsStServiceAbilityG1::OnConnect(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityG1::OnConnect");

    Ability::OnConnect(want);
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::ACTIVE, "OnConnect");
    return nullptr;
}

void AmsStServiceAbilityG1::OnDisconnect(const Want &want)
{
    HILOG_INFO("AmsStServiceAbilityG1::OnDisconnect");

    Ability::OnDisconnect(want);
    PublishEvent(APP_G1_RESP_EVENT_NAME, AbilityLifecycleExecutor::LifecycleState::BACKGROUND, "OnDisconnect");
}

void AmsStServiceAbilityG1::Clear()
{
    shouldReturn_ = "";
    targetBundle_ = "";
    targetAbility_ = "";
    targetBundleConn_ = "";
    targetAbilityConn_ = "";
    nextTargetBundle_ = "";
    nextTargetAbility_ = "";
    nextTargetBundleConn_ = "";
    nextTargetAbilityConn_ = "";
    AmsStServiceAbilityG1::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}

void AmsStServiceAbilityG1::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn_ = mWant.GetStringParam("shouldReturn");
    targetBundle_ = mWant.GetStringParam("targetBundle");
    targetAbility_ = mWant.GetStringParam("targetAbility");
    targetBundleConn_ = mWant.GetStringParam("targetBundleConn");
    targetAbilityConn_ = mWant.GetStringParam("targetAbilityConn");
    nextTargetBundle_ = mWant.GetStringParam("nextTargetBundle");
    nextTargetAbility_ = mWant.GetStringParam("nextTargetAbility");
    nextTargetBundleConn_ = mWant.GetStringParam("nextTargetBundleConn");
    nextTargetAbilityConn_ = mWant.GetStringParam("nextTargetAbilityConn");
    AmsStServiceAbilityG1::AbilityConnectCallback::onAbilityConnectDoneCount = 0;
}

bool AmsStServiceAbilityG1::PublishEvent(const std::string &eventName, const int &code, const std::string &data)
{
    HILOG_INFO("AmsStServiceAbilityG1::PublishEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        code,
        data.c_str());

    Want want;
    want.SetAction(eventName);
    CommonEventData commonData;
    commonData.SetWant(want);
    commonData.SetCode(code);
    commonData.SetData(data);
    return CommonEventManager::PublishCommonEvent(commonData);
}

bool AmsStServiceAbilityG1::SubscribeEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(APP_G1_REQ_EVENT_NAME);
    CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetPriority(1);
    subscriber_ = std::make_shared<AppEventSubscriber>(subscribeInfo);
    subscriber_->mainAbility_ = this;
    return CommonEventManager::SubscribeCommonEvent(subscriber_);
}

void AmsStServiceAbilityG1::AppEventSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    auto eventName = data.GetWant().GetAction();
    auto dataContent = data.GetData();
    HILOG_INFO("AmsStServiceAbilityG1::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
        eventName.c_str(),
        data.GetCode(),
        dataContent.c_str());
    if (APP_G1_REQ_EVENT_NAME.compare(eventName) == 0) {
        if (funcMap_.find(dataContent) == funcMap_.end()) {
            HILOG_INFO(
                "AmsStServiceAbilityG1::OnReceiveEvent eventName = %{public}s, code = %{public}d, data = %{public}s",
                eventName.c_str(),
                data.GetCode(),
                dataContent.c_str());
        } else {
            if (mainAbility_ != nullptr) {
                (mainAbility_->*funcMap_[dataContent])();
            }
        }
    }
}
REGISTER_AA(AmsStServiceAbilityG1);
}  // namespace AppExecFwk
}  // namespace OHOS