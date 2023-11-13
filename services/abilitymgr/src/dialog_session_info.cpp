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

#include "dialog_session_info.h"

#include <string>

#include "hilog_wrapper.h"
#include "parcel_macro.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t CYCLE_LIMIT = 1000;

std::string DialogAbilityInfo::GetURI() const
{
    return bundleName + "/" + moduleName + "/" + abilityName + "/" +
        std::to_string(bundleIconId) + "/" + std::to_string(bundleLabelId) + "/" +
        std::to_string(abilityIconId) + "/" + std::to_string(abilityLabelId);
}

bool DialogAbilityInfo::ParseURI(const std::string &uri)
{
    const size_t memberNum = 7;
    if (std::count(uri.begin(), uri.end(), '/') != memberNum - 1) {
        HILOG_ERROR("Invalid uri: %{public}s.", uri.c_str());
        return false;
    }

    std::vector<std::string> uriVec;
    Split(uri, "/", uriVec);
    uriVec.resize(memberNum);

    int index = 0;
    bundleName = uriVec[index++];
    moduleName = uriVec[index++];
    abilityName = uriVec[index++];
    bundleIconId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    bundleLabelId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    abilityIconId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    abilityLabelId = static_cast<int32_t>(std::stoi(uriVec[index++]));
    return true;
}

void DialogAbilityInfo::Split(const std::string &str, const std::string &delim, std::vector<std::string> &vec)
{
    std::string::size_type pos1 = 0;
    std::string::size_type pos2 = str.find(delim);
    while (std::string::npos != pos2) {
        vec.push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + delim.size();
        pos2 = str.find(delim, pos1);
    }
    if (pos1 != str.size()) {
        vec.push_back(str.substr(pos1));
    }
}

bool DialogSessionInfo::ReadFromParcel(Parcel &parcel)
{
    std::string callerAbilityInfoUri = Str16ToStr8(parcel.ReadString16());
    if (!callerAbilityInfo.ParseURI(callerAbilityInfoUri)) {
        HILOG_ERROR("parse callerAbilityInfo failed");
        return false;
    }
    int32_t targetAbilityInfoSize = 0;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, targetAbilityInfoSize);
    CONTAINER_SECURITY_VERIFY(parcel, targetAbilityInfoSize, &targetAbilityInfos);
    if (targetAbilityInfoSize > CYCLE_LIMIT) {
        HILOG_ERROR("size is too large.");
        return false;
    }
    for (auto i = 0; i < targetAbilityInfoSize; i++) {
        std::string targetAbilityInfoUri = Str16ToStr8(parcel.ReadString16());
        DialogAbilityInfo targetAbilityInfo;
        if (!targetAbilityInfo.ParseURI(targetAbilityInfoUri)) {
            HILOG_ERROR("parse targetAbilityInfo failed");
            return false;
        }
        targetAbilityInfos.emplace_back(targetAbilityInfo);
    }
    std::string paramsJsonString = Str16ToStr8(parcel.ReadString16());
    paramsJson = nlohmann::json::parse(paramsJsonString, nullptr, false);
    if (paramsJson.is_discarded()) {
        HILOG_ERROR("failed to parse json sting: %{private}s.", paramsJsonString.c_str());
        return false;
    }
    return true;
}

bool DialogSessionInfo::Marshalling(Parcel &parcel) const
{
    std::string callerAbilityInfoUri = callerAbilityInfo.GetURI();
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(callerAbilityInfoUri));

    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, targetAbilityInfos.size());
    for (const auto &targetAbilityInfo: targetAbilityInfos) {
        std::string targetAbilityInfoUri = targetAbilityInfo.GetURI();
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(targetAbilityInfoUri));
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(paramsJson.dump()));
    return true;
}

DialogSessionInfo *DialogSessionInfo::Unmarshalling(Parcel &parcel)
{
    DialogSessionInfo *info = new (std::nothrow) DialogSessionInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        HILOG_ERROR("read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
}  // namespace AAFwk
}  // namespace OHOS