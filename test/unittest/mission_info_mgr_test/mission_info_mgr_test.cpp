/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#define private public
#include "mission_info_mgr.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionInfoMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionInfoMgrTest::SetUpTestCase(void)
{}
void MissionInfoMgrTest::TearDownTestCase(void)
{}
void MissionInfoMgrTest::SetUp(void)
{}
void MissionInfoMgrTest::TearDown(void)
{}

/*
 * Feature: MissionInfoMgr
 * Function: GenerateMissionId
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GenerateMissionId
 * EnvConditions: NA
 * CaseDescription: Verify GenerateMissionId
 */
HWTEST_F(MissionInfoMgrTest, GenerateMissionId_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->currentMissionId_ = MAX_MISSION_ID;
    int32_t missionId = 1;
    bool res = missionInfoMgr->GenerateMissionId(missionId);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: GenerateMissionId
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GenerateMissionId
 * EnvConditions: NA
 * CaseDescription: Verify GenerateMissionId
 */
HWTEST_F(MissionInfoMgrTest, GenerateMissionId_002, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->missionIdMap_[missionInfoMgr->currentMissionId_] = true;
    int32_t missionId = 1;
    bool res = missionInfoMgr->GenerateMissionId(missionId);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage AddMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, AddMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage AddMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, AddMissionInfo_002, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.time = 'a';
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
    missionInfo.missionInfo.time = 'b';
    missionInfo.missionInfo.id = 1;
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage AddMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, AddMissionInfo_003, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    InnerMissionInfo missionInfo2;
    missionInfo.missionInfo.time = 'a';
    missionInfo2.missionInfo.time = 'b';
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo2);
    missionInfo.missionInfo.id = 1;
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage UpdateMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    bool res = missionInfoMgr->UpdateMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage UpdateMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionInfo_002, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->missionIdMap_[1] = false;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    bool res = missionInfoMgr->UpdateMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage UpdateMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionInfo_003, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
    bool res = missionInfoMgr->UpdateMissionInfo(missionInfo);
    EXPECT_TRUE(res);
}
}  // namespace AAFwk
}  // namespace OHOS
