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

#include "missioninfomgrc_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "mission_info_mgr.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
} // namespace

uint32_t GetU32Data(const char *ptr)
{
    // convert fuzz input data to an integer
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    bool boolParam = *data % ENABLE;
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data));
    std::string stringParam(data, size);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::vector<std::string> info;
    MissionSnapshot missionSnapshot;

    // fuzz for MissionInfoMgr
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->UpdateMissionLabel(int32Param, stringParam);
    missionInfoMgr->LoadAllMissionInfo();
    std::list<int32_t> missions;
    missionInfoMgr->HandleUnInstallApp(stringParam, int32Param, missions);
    missionInfoMgr->GetMatchedMission(stringParam, int32Param, missions);
    missionInfoMgr->Dump(info);
    sptr<ISnapshotHandler> snapshotHandler;
    missionInfoMgr->RegisterSnapshotHandler(snapshotHandler);
    missionInfoMgr->UpdateMissionSnapshot(int32Param, token, missionSnapshot,
                                          boolParam);
    missionInfoMgr->GetSnapshot(int32Param);
    missionInfoMgr->GetMissionSnapshot(int32Param, token, missionSnapshot,
                                       boolParam, boolParam);
    Snapshot missionInfoMgrSnapshot;
    missionInfoMgr->CreateWhitePixelMap(missionInfoMgrSnapshot);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    char *ch = (char *)malloc(size + 1);
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}