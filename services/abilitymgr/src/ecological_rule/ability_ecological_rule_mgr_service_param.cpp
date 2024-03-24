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

#include "ecological_rule/ability_ecological_rule_mgr_service_param.h"

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace EcologicalRuleMgrService {
#define TAG "ERMS_PARAM"

AbilityExperienceRule *AbilityExperienceRule::Unmarshalling(Parcel &in)
{
    auto *rule = new (std::nothrow) AbilityExperienceRule();
    if (rule == nullptr) {
        return nullptr;
    }

    if (!in.ReadBool(rule->isAllow)) {
        delete rule;
        return nullptr;
    }

    if (!in.ReadString(rule->sceneCode)) {
        delete rule;
        return nullptr;
    }

    rule->replaceWant = in.ReadParcelable<Want>();

    return rule;
}

bool AbilityExperienceRule::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(isAllow)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write isAllow failed");
        return false;
    }

    if (!parcel.WriteString(sceneCode)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write sceneCode failed");
        return false;
    }
    if (!parcel.WriteParcelable(replaceWant)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write replaceWant failed");
        return false;
    }

    return true;
}

bool AbilityCallerInfo::ReadFromParcel(Parcel &parcel)
{
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "read from parcel");
    return true;
}

AbilityCallerInfo *AbilityCallerInfo::Unmarshalling(Parcel &in)
{
    auto *info = new (std::nothrow) AbilityCallerInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "new callerInfo failed, return nullptr");
        return nullptr;
    }

    info->packageName = in.ReadString();
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "read packageName result: %{public}s", info->packageName.c_str());

    if (!in.ReadInt32(info->uid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read uid failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->pid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read pid failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->callerAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read callerAppType failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->targetAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read targetAppType failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->callerModelType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read callerModelType failed");
        delete info;
        return nullptr;
    }

    info->targetAppDistType = in.ReadString();
    info->targetLinkFeature = in.ReadString();

    if (!in.ReadInt32(info->targetLinkType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read targetLinkType failed");
        delete info;
        return nullptr;
    }

    if (!in.ReadInt32(info->callerAbilityType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "read callerAbilityType failed");
        delete info;
        return nullptr;
    }

    return info;
}

bool AbilityCallerInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(packageName)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write packageName failed");
        return false;
    }

    if (!parcel.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write uid failed");
        return false;
    }

    if (!parcel.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write pid failed");
        return false;
    }

    if (!parcel.WriteInt32(callerAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerAppType failed");
        return false;
    }

    if (!parcel.WriteInt32(targetAppType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAppType failed");
        return false;
    }

    if (!parcel.WriteInt32(callerModelType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerModelType failed");
        return false;
    }

    if (!parcel.WriteString(targetAppDistType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetAppDistType failed");
        return false;
    }

    if (!parcel.WriteString(targetLinkFeature)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetLinkFeature failed");
        return false;
    }

    if (!parcel.WriteInt32(targetLinkType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write targetLinkType failed");
        return false;
    }

    if (!parcel.WriteInt32(callerAbilityType)) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "write callerAbilityType failed");
        return false;
    }
    return true;
}

std::string AbilityCallerInfo::ToString() const
{
    std::string str = "CallerInfo{packageName:" + packageName + ",uid:" + std::to_string(uid) +
        ",pid:" + std::to_string(pid) + ",callerAppType:" + std::to_string(callerAppType) +
        ",targetAppType:" + std::to_string(targetAppType) + ",callerModelType:" + std::to_string(callerModelType) +
        ",targetAppDistType:" + targetAppDistType + ",targetLinkFeature:" + targetLinkFeature + ",targetLinkType:" +
        std::to_string(targetLinkType) + ",callerAbilityType:" + std::to_string(callerAbilityType) + "}";
    return str;
}
} // namespace EcologicalRuleMgrService
} // namespace OHOS
