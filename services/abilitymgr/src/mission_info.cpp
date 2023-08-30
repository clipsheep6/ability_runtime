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

#include "mission_info.h"

namespace OHOS {
namespace AAFwk {
bool MissionInfo::ReadFromParcel(Parcel &parcel)
{
    id = parcel.ReadInt32();
    runningState = parcel.ReadInt32();
    lockedState = parcel.ReadBool();
    continuable = parcel.ReadBool();
    time = Str16ToStr8(parcel.ReadString16());
    label = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());
    std::unique_ptr<Want> parcelWant(parcel.ReadParcelable<Want>());
    if (parcelWant == nullptr) {
        return false;
    }
    want = *parcelWant;
    abilityState = parcel.ReadInt32();
    unclearable = parcel.ReadBool();
    continueState = static_cast<AAFwk::ContinueState>(parcel.ReadInt32());
    return true;
}

MissionInfo *MissionInfo::Unmarshalling(Parcel &parcel)
{
    MissionInfo *info = new MissionInfo();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool MissionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(id)) {
        return false;
    }

    if (!parcel.WriteInt32(runningState)) {
        return false;
    }

    if (!parcel.WriteBool(lockedState)) {
        return false;
    }

    if (!parcel.WriteBool(continuable)) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(time))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(label))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(iconPath))) {
        return false;
    }

    if (!parcel.WriteParcelable(&want)) {
        return false;
    }

    if (!parcel.WriteInt32(abilityState)) {
        return false;
    }

    if (!parcel.WriteBool(unclearable)) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(continueState))) {
        return false;
    }
    return true;
}

bool MissionValidResult::ReadFromParcel(Parcel &parcel)
{
    missionId = parcel.ReadInt32();
    isValid = parcel.ReadBool();
    return true;
}

bool MissionValidResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(missionId)) {
        return false;
    }

    if (!parcel.WriteBool(isValid)) {
        return false;
    }

    return true;
}

MissionValidResult *MissionValidResult::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<MissionValidResult> info = std::make_unique<MissionValidResult>();
    if (!info->ReadFromParcel(parcel)) {
        return nullptr;
    }
    return info.release();
}

bool InnerMissionInfoDto::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<MissionInfo> missionInfoPtr(parcel.ReadParcelable<MissionInfo>());
    if (!missionInfoPtr) {
        return false;
    }
    missionInfo = *missionInfoPtr;
    missionName = Str16ToStr8(parcel.ReadString16());
    missionAffinity = Str16ToStr8(parcel.ReadString16());
    launchMode = parcel.ReadInt32();
    startMethod = parcel.ReadInt32();
    bundleName = Str16ToStr8(parcel.ReadString16());
    uid = parcel.ReadInt32();
    isTemporary = parcel.ReadBool();
    specifiedFlag = parcel.ReadInt32();
    hasRecoverInfo = parcel.ReadBool();
    collaboratorType = parcel.ReadInt32();
    return true;
}

InnerMissionInfoDto *InnerMissionInfoDto::Unmarshalling(Parcel &parcel)
{
    InnerMissionInfoDto *info = new InnerMissionInfoDto();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool InnerMissionInfoDto::Marshalling(Parcel &parcel) const
{
    if (!(parcel).WriteParcelable(&missionInfo)) {
        return false;
    }

    if (!(parcel).WriteString16(Str8ToStr16(missionName))) {
        return false;
    }

    if (!(parcel).WriteString16(Str8ToStr16(missionAffinity))) {
        return false;
    }

    if (!(parcel).WriteInt32(launchMode)) {
        return false;
    }

    if (!(parcel).WriteInt32(startMethod)) {
        return false;
    }

    if (!(parcel).WriteString16(Str8ToStr16(bundleName))) {
        return false;
    }

    if (!(parcel).WriteInt32(uid)) {
        return false;
    }

    if (!(parcel).WriteBool(isTemporary)) {
        return false;
    }

    if (!(parcel).WriteString16(Str8ToStr16(specifiedFlag))) {
        return false;
    }

    if (!(parcel).WriteBool(hasRecoverInfo)) {
        return false;
    }

    if (!(parcel).WriteInt32(collaboratorType)) {
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
