/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_MANAGER_RADAR_H
#define OHOS_ABILITY_MANAGER_RADAR_H

#include <cstdint>
#include <chrono>
#include <string>
#include <vector>

namespace OHOS {
namespace AAFWK {
const std::string ORG_PKG_NAME = "ohos.abilitymanagerservice";
const std::string APPLICATION_CONTINUE_BEHAVIOR = "APPLICATION_CONTINUE_BEHAVIOR";
const std::string DMS_PKG_NAME = "ohos.distributedschedule";
const std::string ORG_PKG = "ORG_PKG";
const std::string FUNC = "FUNC";
const std::string BIZ_SCENE = "BIZ_SCENE";
const std::string BIZ_STAGE = "BIZ_STAGE";
const std::string STAGE_RES = "STAGE_RES";
const std::string BIZ_STATE = "BIZ_STATE";
const std::string TO_CALL_PKG = "TO_CALL_PKG";
const std::string ERROR_CODE = "ERROR_CODE";
constexpr char APP_CONTINUE_DOMAIN[] = "APP_CONTINUE";

enum class BizScene : int32_t {
    CLICK_ICON = 10,
    SAVE_DATA = 11,
};

enum class StageRes : int32_t {
    STAGE_IDLE = 0,
    STAGE_SUCC = 1,
    STAGE_FAIL = 2,
    STAGE_CANCEL = 3,
    STAGE_UNKNOW = 4,
};

enum class BizState : int32_t {
    BIZ_STATE_START = 1,
    BIZ_STATE_END = 2,
};

enum class ClickIcon : int32_t {
    CLICKICON_CONTINUE = 3,
    CLICKICON_STARTABILITY = 6,
    CLICKICON_RECV_OVER = 9,
};

enum class SaveData : int32_t {
    SAVEDATA_CONTINUE = 2,
    SAVEDATA_RES = 3,
    SAVEDATA_REMOTE_WANT = 4,
};

class ContinueRadar {
public:
    static ContinueRadar &GetInstance();

    bool ClickIconContinue(const std::string& func);
    bool ClickIconStartAbility(const std::string& func, int32_t errCode);
    bool ClickIconRecvOver(const std::string& func);
    bool SaveDataContinue(const std::string& func);
    bool SaveDataRes(const std::string& func);
    bool SaveDataRemoteWant(const std::string& func);
};
} // namespace AAFWK
} // namespace OHOS
#endif // OHOS_ABILITY_MANAGER_RADAR_H