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

#include "amsstabilityq4.h"

namespace OHOS {
namespace AppExecFwk {
void AmsStAbilityQ4::OnStart(const Want &want)
{
    GetWantInfo(want);

    HILOG_INFO("AmsStAbilityQ4::onStart");
    pageAbilityEvent.SubscribeEvent(STEventName::g_eventList, shared_from_this());
    Ability::OnStart(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStart;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStartCount(), eventData);
}

void AmsStAbilityQ4::OnNewWant(const Want &want)
{
    HILOG_INFO("AmsStAbilityQ4::OnNewWant");
    Ability::OnNewWant(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnNewWant;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnNewWantCount(), eventData);
}

void AmsStAbilityQ4::OnForeground(const Want &want)
{
    HILOG_INFO("AmsStAbilityQ4::OnForeground");
    Ability::OnForeground(want);
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnForeground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnForegroundCount(), eventData);
}

void AmsStAbilityQ4::OnStop()
{
    HILOG_INFO("AmsStAbilityQ4::onStop");
    Ability::OnStop();
    pageAbilityEvent.UnsubscribeEvent();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnStop;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnStopCount(), eventData);
}

void AmsStAbilityQ4::OnActive()
{
    HILOG_INFO("AmsStAbilityQ4::OnActive");
    Ability::OnActive();
    std::string startBundleName = this->Split(targetBundle, ",");
    std::string startAbilityName = this->Split(targetAbility, ",");
    if (!startBundleName.empty() && !startAbilityName.empty()) {
        Want want;
        want.SetElementName(startBundleName, startAbilityName);
        want.SetParam("shouldReturn", shouldReturn);
        if (!targetBundle.empty() && !targetAbility.empty()) {
            want.SetParam("targetBundle", targetBundle);
            want.SetParam("targetAbility", targetAbility);
        }
        StartAbility(want);
    }
    if (std::string::npos != shouldReturn.find(GetAbilityName())) {
        TerminateAbility();
    }
    Clear();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnActive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnActiveCount(), eventData);
}

void AmsStAbilityQ4::OnInactive()
{
    HILOG_INFO("AmsStAbilityQ4::OnInactive");
    Ability::OnInactive();
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnInactive;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnInactiveCount(), eventData);
}

void AmsStAbilityQ4::OnBackground()
{
    HILOG_INFO("AmsStAbilityQ4::OnBackground");
    Ability::OnBackground();
    std::this_thread::sleep_for(std::chrono::milliseconds(STEventName::TERMINATE_TIMEOUT));
    std::string eventData = GetAbilityName() + STEventName::g_abilityStateOnBackground;
    pageAbilityEvent.PublishEvent(STEventName::g_eventName, pageAbilityEvent.GetOnBackgroundCount(), eventData);
}

void AmsStAbilityQ4::Clear()
{
    shouldReturn = "";
    targetBundle = "";
    targetAbility = "";
}

void AmsStAbilityQ4::GetWantInfo(const Want &want)
{
    Want mWant(want);
    shouldReturn = mWant.GetStringParam("shouldReturn");
    targetBundle = mWant.GetStringParam("targetBundle");
    targetAbility = mWant.GetStringParam("targetAbility");
}

std::string AmsStAbilityQ4::Split(std::string &str, std::string delim)
{
    std::string result;
    if (!str.empty()) {
        size_t index = str.find(delim);
        if (index != std::string::npos) {
            result = str.substr(0, index);
            str = str.substr(index + delim.size());
        } else {
            result = str;
            str = "";
        }
    }
    return result;
}

REGISTER_AA(AmsStAbilityQ4);
}  // namespace AppExecFwk
}  // namespace OHOS