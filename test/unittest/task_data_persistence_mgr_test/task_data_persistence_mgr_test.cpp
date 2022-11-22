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
#include "task_data_persistence_mgr.h"
#undef private

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class TaskDataPersistenceMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TaskDataPersistenceMgrTest::SetUpTestCase(void)
{}
void TaskDataPersistenceMgrTest::TearDownTestCase(void)
{}
void TaskDataPersistenceMgrTest::SetUp(void)
{}
void TaskDataPersistenceMgrTest::TearDown(void)
{}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: LoadAllMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage LoadAllMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify LoadAllMissionInfo
 */
HWTEST_F(TaskDataPersistenceMgrTest, LoadAllMissionInfo_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    std::list<InnerMissionInfo> missionInfoList;
    bool res = taskDataPersistenceMgr->LoadAllMissionInfo(missionInfoList);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: SaveMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionInfo
 */
HWTEST_F(TaskDataPersistenceMgrTest, SaveMissionInfo_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    InnerMissionInfo missionInfo;
    bool res = taskDataPersistenceMgr->SaveMissionInfo(missionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: SaveMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionInfo
 */
HWTEST_F(TaskDataPersistenceMgrTest, SaveMissionInfo_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int userId = 0;
    taskDataPersistenceMgr->Init(userId);
    InnerMissionInfo missionInfo;
    bool res = taskDataPersistenceMgr->SaveMissionInfo(missionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: DeleteMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionInfo
 */
HWTEST_F(TaskDataPersistenceMgrTest, DeleteMissionInfo_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int missionId = 0;
    bool res = taskDataPersistenceMgr->DeleteMissionInfo(missionId);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: DeleteMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionInfo
 */
HWTEST_F(TaskDataPersistenceMgrTest, DeleteMissionInfo_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int userId = 0;
    taskDataPersistenceMgr->Init(userId);
    int missionId = 0;
    bool res = taskDataPersistenceMgr->DeleteMissionInfo(missionId);
    EXPECT_TRUE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: RemoveUserDir
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage RemoveUserDir
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUserDir
 */
HWTEST_F(TaskDataPersistenceMgrTest, RemoveUserDir_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int32_t userId = 0;
    bool res = taskDataPersistenceMgr->RemoveUserDir(userId);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: RemoveUserDir
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage RemoveUserDir
 * EnvConditions: NA
 * CaseDescription: Verify RemoveUserDir
 */
HWTEST_F(TaskDataPersistenceMgrTest, RemoveUserDir_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int32_t userId = -1;
    bool res = taskDataPersistenceMgr->RemoveUserDir(userId);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: SaveMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, SaveMissionSnapshot_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int missionId = 0;
    MissionSnapshot snapshot;
    bool res = taskDataPersistenceMgr->SaveMissionSnapshot(missionId, snapshot);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: SaveMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, SaveMissionSnapshot_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int userId = 0;
    taskDataPersistenceMgr->Init(userId);
    int missionId = 0;
    MissionSnapshot snapshot;
    bool res = taskDataPersistenceMgr->SaveMissionSnapshot(missionId, snapshot);
    EXPECT_TRUE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: GetSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, GetSnapshot_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int missionId = 0;
    std::shared_ptr<Media::PixelMap> res = taskDataPersistenceMgr->GetSnapshot(missionId);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: GetSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, GetSnapshot_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int userId = 0;
    taskDataPersistenceMgr->Init(userId);
    int missionId = 0;
    std::shared_ptr<Media::PixelMap> res = taskDataPersistenceMgr->GetSnapshot(missionId);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, GetMissionSnapshot_001, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int missionId = 0;
    MissionSnapshot snapshot;
    bool isLowResolution = true;
    bool res = taskDataPersistenceMgr->GetMissionSnapshot(missionId, snapshot, isLowResolution);
    EXPECT_FALSE(res);
}

/*
 * Feature: TaskDataPersistenceMgr
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(TaskDataPersistenceMgrTest, GetMissionSnapshot_002, TestSize.Level1)
{
    auto taskDataPersistenceMgr = std::make_shared<TaskDataPersistenceMgr>();
    int userId = 0;
    taskDataPersistenceMgr->Init(userId);
    int missionId = 0;
    MissionSnapshot snapshot;
    bool isLowResolution = true;
    bool res = taskDataPersistenceMgr->GetMissionSnapshot(missionId, snapshot, isLowResolution);
    EXPECT_FALSE(res);
}
}  // namespace AAFwk
}  // namespace OHOS
