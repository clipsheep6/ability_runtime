/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mission_index_info.h"

namespace OHOS {
namespace AAFwk {
void MissionIndexInfo::SetMissionIndexInfo(int32_t stackId, int32_t missionIndex)
{
    stackId_ = stackId;
    missionIndex_ = missionIndex;
}

int32_t MissionIndexInfo::GetStackId()
{
    return stackId_;
}
int32_t MissionIndexInfo::GetMissionIndex()
{
    return missionIndex_;
}
}  // namespace AAFwk
}  // namespace OHOS
