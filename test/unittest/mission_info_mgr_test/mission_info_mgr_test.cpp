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

#include "ability_info.h"
#include "ability_manager_errors.h"

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
 * FunctionPoints: MissionInfoMgr GenerateMissionId
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
 * FunctionPoints: MissionInfoMgr GenerateMissionId
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

    missionInfoMgr->currentMissionId_ = 1;
    EXPECT_TRUE(missionInfoMgr->GenerateMissionId(missionId));
}

/*
 * Feature: MissionInfoMgr
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr AddMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, AddMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    EXPECT_FALSE(missionInfoMgr->AddMissionInfo(missionInfo));

    missionInfoMgr->missionIdMap_[1] = false;
    missionInfoMgr->DeleteMissionInfo(1);
    EXPECT_TRUE(missionInfoMgr->AddMissionInfo(missionInfo));
}

/*
 * Feature: MissionListManager
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr AddMissionInfo
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
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    missionInfo.missionInfo.time = 'b';
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->DeleteMissionInfo(missionInfo.missionInfo.id);
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr AddMissionInfo
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
    missionInfoMgr->missionInfoList_.push_back(missionInfo2);
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->DeleteMissionInfo(missionInfo.missionInfo.id);
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionInfoMgr
 * Function: AddMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr AddMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify AddMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, AddMissionInfo_004, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->DeleteMissionInfo(missionInfo.missionInfo.id);

    auto temp = missionInfoMgr->taskDataPersistenceMgr_->handler_;
    missionInfoMgr->taskDataPersistenceMgr_->handler_.reset();
    EXPECT_FALSE(missionInfoMgr->AddMissionInfo(missionInfo));
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr UpdateMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    EXPECT_FALSE(missionInfoMgr->UpdateMissionInfo(missionInfo));

    missionInfoMgr->missionIdMap_[1] = false;
    EXPECT_FALSE(missionInfoMgr->UpdateMissionInfo(missionInfo));

    missionInfoMgr->missionIdMap_[1] = true;
    EXPECT_FALSE(missionInfoMgr->UpdateMissionInfo(missionInfo));
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr UpdateMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionInfo_002, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    missionInfoMgr->missionIdMap_[2] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 2;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);

    auto temp = missionInfoMgr->taskDataPersistenceMgr_->handler_;
    missionInfoMgr->taskDataPersistenceMgr_->handler_.reset();
    EXPECT_FALSE(missionInfoMgr->UpdateMissionInfo(missionInfo));
    missionInfoMgr->taskDataPersistenceMgr_->handler_ = temp;

    missionInfo.missionInfo.time = 'b';
    EXPECT_TRUE(missionInfoMgr->UpdateMissionInfo(missionInfo));
    missionInfoMgr->DeleteMissionInfo(missionInfo.missionInfo.id);
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr UpdateMissionInfo
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
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_TRUE(missionInfoMgr->UpdateMissionInfo(missionInfo));
}

/*
 * Feature: MissionInfoMgr
 * Function: DeleteMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr DeleteMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, DeleteMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    missionInfoMgr->missionIdMap_[1] = false;
    EXPECT_TRUE(missionInfoMgr->DeleteMissionInfo(-1));
    EXPECT_TRUE(missionInfoMgr->DeleteMissionInfo(1));

    missionInfoMgr->missionIdMap_[1] = true;
    EXPECT_FALSE(missionInfoMgr->DeleteMissionInfo(1));

    missionInfoMgr->Init(0);
    auto temp = missionInfoMgr->taskDataPersistenceMgr_->handler_;
    missionInfoMgr->taskDataPersistenceMgr_->handler_ = nullptr;
    EXPECT_FALSE(missionInfoMgr->DeleteMissionInfo(1));
    missionInfoMgr->taskDataPersistenceMgr_->handler_ = temp;

    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    InnerMissionInfo missionInfo2;
    missionInfo2.missionInfo.id = 2;
    missionInfoMgr->missionInfoList_.push_back(missionInfo2);
    missionInfoMgr->missionIdMap_[2] = true;
    EXPECT_TRUE(missionInfoMgr->DeleteMissionInfo(1));
}

/*
 * Feature: MissionInfoMgr
 * Function: DeleteAllMissionInfos
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr DeleteAllMissionInfos
 * EnvConditions: NA
 * CaseDescription: Verify DeleteAllMissionInfos
 */
HWTEST_F(MissionInfoMgrTest, DeleteAllMissionInfos_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    EXPECT_FALSE(missionInfoMgr->DeleteAllMissionInfos(nullptr));
}

/*
 * Feature: MissionInfoMgr
 * Function: GetMissionInfos
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetMissionInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionInfos
 */
HWTEST_F(MissionInfoMgrTest, GetMissionInfos_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    std::vector<MissionInfo> missionInfos;
    EXPECT_EQ(missionInfoMgr->GetMissionInfos(-1, missionInfos), -1);

    EXPECT_EQ(missionInfoMgr->GetMissionInfos(1, missionInfos), 0);

    EXPECT_EQ(missionInfoMgr->GetMissionInfos(0, missionInfos), 0);
}

/*
 * Feature: MissionInfoMgr
 * Function: GetMissionInfoById
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetMissionInfoById
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionInfoById
 */
HWTEST_F(MissionInfoMgrTest, GetMissionInfoById_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    MissionInfo myMissionInfo;
    auto ret1 = missionInfoMgr->GetMissionInfoById(-1, myMissionInfo);
    EXPECT_EQ(ret1, -1);

    auto ret2 = missionInfoMgr->GetMissionInfoById(1, myMissionInfo);
    EXPECT_EQ(ret2, 0);

    missionInfo.missionInfo.id = 100;
    EXPECT_EQ(missionInfoMgr->GetMissionInfoById(1, myMissionInfo), 0);

    missionInfo.startMethod = 100;
    EXPECT_EQ(missionInfoMgr->GetMissionInfoById(1, myMissionInfo), 0);

    missionInfoMgr->missionIdMap_[2] = true;
    EXPECT_EQ(missionInfoMgr->GetMissionInfoById(2, myMissionInfo), -1);
}

/*
 * Feature: MissionInfoMgr
 * Function: GetInnerMissionInfoById
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetInnerMissionInfoById
 * EnvConditions: NA
 * CaseDescription: Verify GetInnerMissionInfoById
 */
HWTEST_F(MissionInfoMgrTest, GetInnerMissionInfoById_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_EQ(missionInfoMgr->GetInnerMissionInfoById(1, missionInfo), 0);

    missionInfoMgr->missionIdMap_[2] = true;
    EXPECT_EQ(missionInfoMgr->GetInnerMissionInfoById(2, missionInfo), MISSION_NOT_FOUND);
}

/*
 * Feature: MissionInfoMgr
 * Function: FindReusedMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr FindReusedMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify FindReusedMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, FindReusedMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("", "", false, missionInfo));
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfo.missionInfo.id = 1;
    missionInfo.launchMode = static_cast<int32_t>(AppExecFwk::LaunchMode::STANDARD);
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.launchMode = static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON);
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));
    missionInfoMgr->missionInfoList_.clear();
    missionInfo.missionName = "test";
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_TRUE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.launchMode = static_cast<int32_t>(AppExecFwk::LaunchMode::SPECIFIED);
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_TRUE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.missionName = "test1";
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.missionName = "test";
    missionInfo.specifiedFlag = "flag";
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.specifiedFlag = "";
    missionInfo.launchMode = 1000;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_FALSE(missionInfoMgr->FindReusedMissionInfo("test", "", false, missionInfo));

    missionInfoMgr->missionInfoList_.clear();
    missionInfo.missionName = "test";
    missionInfo.launchMode = static_cast<int32_t>(AppExecFwk::LaunchMode::STANDARD);
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    EXPECT_TRUE(missionInfoMgr->FindReusedMissionInfo("test", "", true, missionInfo));
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionLabel
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr UpdateMissionLabel
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionLabel
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionLabel_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);

    std::string label = "test";
    auto ret1 = missionInfoMgr->UpdateMissionLabel(-1, label);
    EXPECT_EQ(ret1, -1);

    auto ret2 = missionInfoMgr->UpdateMissionLabel(1, label);
    EXPECT_EQ(ret2, 0);

    auto temp = missionInfoMgr->taskDataPersistenceMgr_;
    missionInfoMgr->taskDataPersistenceMgr_.reset();
    EXPECT_EQ(missionInfoMgr->UpdateMissionLabel(1, label), -1);
    missionInfoMgr->taskDataPersistenceMgr_ = temp;

    auto temp1 = missionInfoMgr->taskDataPersistenceMgr_->handler_;
    missionInfoMgr->taskDataPersistenceMgr_->handler_.reset();
    EXPECT_EQ(missionInfoMgr->UpdateMissionLabel(1, label), -1);
    missionInfoMgr->taskDataPersistenceMgr_->handler_ = temp1;
}

/*
 * Feature: MissionInfoMgr
 * Function: LoadAllMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr LoadAllMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify LoadAllMissionInfo
 */
HWTEST_F(MissionInfoMgrTest, LoadAllMissionInfo_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    EXPECT_FALSE(missionInfoMgr->LoadAllMissionInfo());

    int userId = 0;
    missionInfoMgr->Init(userId);
    auto temp = missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_;
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_.reset();
    EXPECT_FALSE(missionInfoMgr->LoadAllMissionInfo());
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_ = temp;
}

/*
 * Feature: MissionInfoMgr
 * Function: HandleUnInstallApp
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr HandleUnInstallApp
 * EnvConditions: NA
 * CaseDescription: Verify HandleUnInstallApp
 */
HWTEST_F(MissionInfoMgrTest, HandleUnInstallApp_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    std::list<int32_t> missions;
    missionInfoMgr->HandleUnInstallApp("", 1, missions);

    missions.push_back(1);
    missionInfoMgr->HandleUnInstallApp("", 1, missions);
}

/*
 * Feature: MissionInfoMgr
 * Function: GetMatchedMission
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetMatchedMission
 * EnvConditions: NA
 * CaseDescription: Verify GetMatchedMission
 */
HWTEST_F(MissionInfoMgrTest, GetMatchedMission_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    missionInfo.uid = 1;
    missionInfo.bundleName = "test";
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    std::list<int32_t> missions;
    missionInfoMgr->GetMatchedMission("test", 1, missions);
}

/*
 * Feature: MissionInfoMgr
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr Dump
 * EnvConditions: NA
 * CaseDescription: Verify Dump
 */
HWTEST_F(MissionInfoMgrTest, Dump_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    std::vector<std::string> info;
    missionInfoMgr->Dump(info);
}

/*
 * Feature: MissionInfoMgr
 * Function: RegisterSnapshotHandler
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr RegisterSnapshotHandler
 * EnvConditions: NA
 * CaseDescription: Verify RegisterSnapshotHandler
 */
HWTEST_F(MissionInfoMgrTest, RegisterSnapshotHandler_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    sptr<ISnapshotHandler> handler = nullptr;
    missionInfoMgr->RegisterSnapshotHandler(handler);
    EXPECT_EQ(missionInfoMgr->snapshotHandler_, nullptr);
}

/*
 * Feature: MissionInfoMgr
 * Function: UpdateMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr UpdateMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify UpdateMissionSnapshot
 */
HWTEST_F(MissionInfoMgrTest, UpdateMissionSnapshot_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    MissionSnapshot missionSnapshot;
    auto temp = missionInfoMgr->snapshotHandler_;
    missionInfoMgr->snapshotHandler_ = nullptr;
    EXPECT_FALSE(missionInfoMgr->UpdateMissionSnapshot(1, nullptr, missionSnapshot, true));
    missionInfoMgr->snapshotHandler_ = temp;
    EXPECT_FALSE(missionInfoMgr->UpdateMissionSnapshot(1, nullptr, missionSnapshot, true));
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: MissionInfoMgr
 * Function: GetSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetSnapshot
 */
HWTEST_F(MissionInfoMgrTest, GetSnapshot_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    MissionSnapshot missionSnapshot;
    EXPECT_EQ(missionInfoMgr->GetSnapshot(2), nullptr);

    auto temp = missionInfoMgr->taskDataPersistenceMgr_;
    missionInfoMgr->taskDataPersistenceMgr_.reset();
    EXPECT_EQ(missionInfoMgr->GetSnapshot(2), nullptr);
    missionInfoMgr->taskDataPersistenceMgr_ = temp;

    auto temp1 = missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_;
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_.reset();
    EXPECT_EQ(missionInfoMgr->GetSnapshot(2), nullptr);
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_ = temp1;
}
#endif

/*
 * Feature: MissionInfoMgr
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionInfoMgrTest, GetMissionSnapshot_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    sptr<IRemoteObject> abilityToken = nullptr;
    MissionSnapshot missionSnapshot;
    EXPECT_FALSE(missionInfoMgr->GetMissionSnapshot(1, abilityToken, missionSnapshot, true, true));

    int userId = 0;
    missionInfoMgr->Init(userId);
    InnerMissionInfo info;
    info.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(info);
    auto temp = missionInfoMgr->taskDataPersistenceMgr_;
    missionInfoMgr->taskDataPersistenceMgr_.reset();
    EXPECT_FALSE(missionInfoMgr->GetMissionSnapshot(1, abilityToken, missionSnapshot, true, true));
    missionInfoMgr->taskDataPersistenceMgr_ = temp;

    // force
    auto temp2 = missionInfoMgr->snapshotHandler_;
    missionInfoMgr->snapshotHandler_ = nullptr;
    EXPECT_FALSE(missionInfoMgr->GetMissionSnapshot(1, abilityToken, missionSnapshot, true, true));
    missionInfoMgr->snapshotHandler_ = temp2;

    // taskDataPersistenceMgr_ GetMissionSnapshot return false
    auto temp3 = missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_;
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_ = nullptr;
    auto temp4 = missionInfoMgr->snapshotHandler_;
    missionInfoMgr->snapshotHandler_ = nullptr;
    EXPECT_FALSE(missionInfoMgr->GetMissionSnapshot(1, abilityToken, missionSnapshot, true, false));
    missionInfoMgr->snapshotHandler_ = temp4;
    missionInfoMgr->taskDataPersistenceMgr_->currentMissionDataStorage_ = temp3;
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: MissionInfoMgr
 * Function: CreateWhitePixelMap
 * SubFunction: NA
 * FunctionPoints: MissionInfoMgr CreateWhitePixelMap
 * EnvConditions: NA
 * CaseDescription: Verify CreateWhitePixelMap
 */
HWTEST_F(MissionInfoMgrTest, CreateWhitePixelMap_001, TestSize.Level1)
{
    auto missionInfoMgr = std::make_shared<MissionInfoMgr>();
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->missionInfoList_.push_back(missionInfo);
    Snapshot snapshot;
    missionInfoMgr->CreateWhitePixelMap(snapshot);
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
