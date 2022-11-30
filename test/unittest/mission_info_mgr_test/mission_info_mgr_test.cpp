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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo2);
    missionInfo.missionInfo.id = 1;
    missionInfoMgr->DeleteMissionInfo(missionInfo.missionInfo.id);
    bool res = missionInfoMgr->AddMissionInfo(missionInfo);
    EXPECT_TRUE(res);
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
    bool res = missionInfoMgr->UpdateMissionInfo(missionInfo);
    EXPECT_FALSE(res);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
    bool res = missionInfoMgr->UpdateMissionInfo(missionInfo);
    EXPECT_TRUE(res);
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
    int userId = 0;
    missionInfoMgr->Init(userId);
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
    EXPECT_TRUE(missionInfoMgr->DeleteMissionInfo(-1));
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
    std::shared_ptr<MissionListenerController> listenerController;
    EXPECT_FALSE(missionInfoMgr->DeleteAllMissionInfos(listenerController));
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    std::vector<MissionInfo> missionInfos;
    auto ret1 = missionInfoMgr->GetMissionInfos(-1, missionInfos);
    EXPECT_EQ(ret1, -1);

    auto ret2 = missionInfoMgr->GetMissionInfos(1, missionInfos);
    EXPECT_EQ(ret2, 0);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    MissionInfo myMissionInfo;
    auto ret1 = missionInfoMgr->GetMissionInfoById(-1, myMissionInfo);
    EXPECT_EQ(ret1, -1);

    auto ret2 = missionInfoMgr->GetMissionInfoById(1, myMissionInfo);
    EXPECT_EQ(ret2, 0);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    InnerMissionInfo innerMissionInfo;
    auto ret2 = missionInfoMgr->GetInnerMissionInfoById(1, innerMissionInfo);
    EXPECT_EQ(ret2, 0);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    std::string label = "test";
    auto ret1 = missionInfoMgr->UpdateMissionLabel(-1, label);
    EXPECT_EQ(ret1, -1);

    auto ret2 = missionInfoMgr->UpdateMissionLabel(1, label);
    EXPECT_EQ(ret2, 0);
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
    missionInfoMgr->missionIdMap_[1] = true;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 1;
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
    std::list<int32_t> missions;
    missionInfoMgr->HandleUnInstallApp("", 1, missions);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    sptr<ISnapshotHandler> handler = nullptr;
    missionInfoMgr->RegisterSnapshotHandler(handler);
    EXPECT_EQ(missionInfoMgr->snapshotHandler_, nullptr);
}

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
    auto ret = missionInfoMgr->GetMissionSnapshot(1, abilityToken, missionSnapshot, true, true);
    EXPECT_FALSE(ret);
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
    auto listIter = missionInfoMgr->missionInfoList_.begin();
    missionInfoMgr->missionInfoList_.insert(listIter, missionInfo);

    Snapshot snapshot;
    missionInfoMgr->CreateWhitePixelMap(snapshot);
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
