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

#include "dialog_session_info.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr size_t MEMBER_NUM = 11;

std::string DialogAbilityInfo::GetURI() const
{
    return bundleName + "/" + moduleName + "/" + abilityName + "/" +
        std::to_string(bundleIconId) + "/" + std::to_string(bundleLabelId) + "/" +
        std::to_string(abilityIconId) + "/" + std::to_string(abilityLabelId) + "/" +
        std::to_string(visible) + "/" + std::to_string(appIndex) + "/" +
        std::to_string(static_cast<int32_t>(multiAppMode.multiAppModeType)) + "/" +
        std::to_string(multiAppMode.maxCount);
}

bool DialogAbilityInfo::ParseURI(const std::string &uri)
{
    if (std::count(uri.begin(), uri.end(), '/') != MEMBER_NUM - 1) {
        TAG_LOGE(AAFwkTag::DIALOG, "Invalid uri: %{public}s", uri.c_str());
        return false;
    }

    std::vector<std::string> uriVec;
    Split(uri, "/", uriVec);
    uriVec.resize(MEMBER_NUM);

    int index = 0;
    bundleName = uriVec[index++];
    moduleName = uriVec[index++];
    abilityName = uriVec[index++];
    bundleIconId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    bundleLabelId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    abilityIconId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    abilityLabelId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    visible = std::stoi(uriVec[index++]);
    appIndex = static_cast<int32_t>(std::stoi(uriVec[index++]));
    multiAppMode.multiAppModeType = static_cast<AppExecFwk::MultiAppModeType>(std::stoi(uriVec[index++]));
    multiAppMode.maxCount = static_cast<int32_t>(std::stoi(uriVec[index++]));
    return true;
}

void DialogAbilityInfo::Split(const std::string &str, const std::string &delim, std::vector<std::string> &vec)
{
    std::string::size_type posLeft = 0;
    std::string::size_type posRight = str.find(delim);
    while (std::string::npos != posRight) {
        vec.push_back(str.substr(posLeft, posRight - posLeft));
        posLeft = posRight + delim.size();
        posRight = str.find(delim, posLeft);
    }
    if (posLeft != str.size()) {
        vec.push_back(str.substr(posLeft));
    }
}

bool DialogSessionInfo::ReadFromParcel(Parcel &parcel)
{
    std::string callerAbilityInfoUri = Str16ToStr8(parcel.ReadString16());
    if (!callerAbilityInfo.ParseURI(callerAbilityInfoUri)) {
        TAG_LOGE(AAFwkTag::DIALOG, "parse callerAbilityInfo failed");
        return false;
    }
    int32_t targetAbilityInfoSize = 0;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, targetAbilityInfoSize);
    CONTAINER_SECURITY_VERIFY(parcel, targetAbilityInfoSize, &targetAbilityInfos);
    if (targetAbilityInfoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::DIALOG, "size too large");
        return false;
    }
    for (auto i = 0; i < targetAbilityInfoSize; i++) {
        std::string targetAbilityInfoUri = Str16ToStr8(parcel.ReadString16());
        DialogAbilityInfo targetAbilityInfo;
        if (!targetAbilityInfo.ParseURI(targetAbilityInfoUri)) {
            TAG_LOGE(AAFwkTag::DIALOG, "parse targetAbilityInfo failed");
            return false;
        }
        targetAbilityInfos.emplace_back(targetAbilityInfo);
    }
    std::unique_ptr<AAFwk::WantParams> params(parcel.ReadParcelable<AAFwk::WantParams>());
    if (!params) {
        APP_LOGE("ReadParcelable WantParams failed");
        return false;
    }
    parameters = *params;
    return true;
}

bool DialogSessionInfo::Marshalling(Parcel &parcel) const
{
    std::string callerAbilityInfoUri = callerAbilityInfo.GetURI();
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(callerAbilityInfoUri));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, targetAbilityInfos.size());
    for (const auto &targetAbilityInfo : targetAbilityInfos) {
        std::string targetAbilityInfoUri = targetAbilityInfo.GetURI();
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(targetAbilityInfoUri));
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Parcelable, parcel, &parameters);
    return true;
}

DialogSessionInfo *DialogSessionInfo::Unmarshalling(Parcel &parcel)
{
    DialogSessionInfo *info = new (std::nothrow) DialogSessionInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::DIALOG, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
}  // namespace AAFwk
}  // namespace OHOS
