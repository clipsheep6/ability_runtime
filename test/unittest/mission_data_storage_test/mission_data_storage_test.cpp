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
#define protected public
#include "mission_data_storage.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MissionDataStorageTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MissionDataStorageTest::SetUpTestCase(void)
{}
void MissionDataStorageTest::TearDownTestCase(void)
{}
void MissionDataStorageTest::SetUp(void)
{}
void MissionDataStorageTest::TearDown(void)
{}

/*
 * Feature: MissionListManager
 * Function: SetEventHandler
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SetEventHandler
 * EnvConditions: NA
 * CaseDescription: Verify SetEventHandler
 */
HWTEST_F(MissionDataStorageTest, SetEventHandler_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    missionDataStorage->SetEventHandler(handler);
}

/*
 * Feature: MissionListManager
 * Function: SaveMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionInfo
 */
HWTEST_F(MissionDataStorageTest, SaveMissionInfo_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 0;
    missionDataStorage->SaveMissionInfo(missionInfo);
}

/*
 * Feature: MissionListManager
 * Function: SaveMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionInfo
 */
HWTEST_F(MissionDataStorageTest, SaveMissionInfo_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    missionDataStorage->userId_ = 10;
    InnerMissionInfo missionInfo;
    missionInfo.missionInfo.id = 0;
    missionDataStorage->SaveMissionInfo(missionInfo);
}

/*
 * Feature: MissionListManager
 * Function: DeleteMissionInfo
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteMissionInfo
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionInfo
 */
HWTEST_F(MissionDataStorageTest, DeleteMissionInfo_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    missionDataStorage->DeleteMissionInfo(missionId);
    missionId = 1;
    missionDataStorage->DeleteMissionInfo(missionId);
}

/*
 * Feature: MissionListManager
 * Function: SaveMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify SaveMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, SaveMissionSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    MissionSnapshot missionSnapshot;
    missionDataStorage->SaveMissionSnapshot(missionId, missionSnapshot);
}

/*
 * Feature: MissionListManager
 * Function: DeleteMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, DeleteMissionSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    MissionSnapshot missionSnapshot;
    missionDataStorage->SaveMissionSnapshot(missionId, missionSnapshot);
    missionId = 0;
    missionDataStorage->DeleteMissionSnapshot(missionId);
}

/*
 * Feature: MissionListManager
 * Function: DeleteMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify DeleteMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, DeleteMissionSnapshot_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    missionDataStorage->DeleteMissionSnapshot(missionId);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, GetMissionSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = true;
    missionDataStorage->SaveCachedSnapshot(missionId, missionSnapshot);
    bool res = missionDataStorage->GetMissionSnapshot(missionId, missionSnapshot, isLowResolution);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, GetMissionSnapshot_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = false;
    missionDataStorage->SaveCachedSnapshot(missionId, missionSnapshot);
    bool res = missionDataStorage->GetMissionSnapshot(missionId, missionSnapshot, isLowResolution);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshot
 */
HWTEST_F(MissionDataStorageTest, GetMissionSnapshot_003, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    MissionSnapshot missionSnapshot;
    bool isLowResolution = true;
    bool res = missionDataStorage->GetMissionSnapshot(missionId, missionSnapshot, isLowResolution);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: GetMissionDataFilePath
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionDataFilePath
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionDataFilePath
 */
HWTEST_F(MissionDataStorageTest, GetMissionDataFilePath_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    std::string res = missionDataStorage->GetMissionDataFilePath(missionId);
    EXPECT_EQ(res, "/data/service/el1/public/AbilityManagerService/0/MissionInfo/mission_0.json");
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshotPath
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshotPath
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshotPath
 */
HWTEST_F(MissionDataStorageTest, GetMissionSnapshotPath_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    bool isLowResolution = true;
    std::string res = missionDataStorage->GetMissionSnapshotPath(missionId, isLowResolution);
    EXPECT_EQ(res, "/data/service/el1/public/AbilityManagerService/0/MissionInfo/mission_0_little.jpg");
}

/*
 * Feature: MissionListManager
 * Function: GetMissionSnapshotPath
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetMissionSnapshotPath
 * EnvConditions: NA
 * CaseDescription: Verify GetMissionSnapshotPath
 */
HWTEST_F(MissionDataStorageTest, GetMissionSnapshotPath_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int missionId = 0;
    bool isLowResolution = false;
    std::string res = missionDataStorage->GetMissionSnapshotPath(missionId, isLowResolution);
    EXPECT_EQ(res, "/data/service/el1/public/AbilityManagerService/0/MissionInfo/mission_0.jpg");
}

/*
 * Feature: MissionListManager
 * Function: SaveSnapshotFile
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveSnapshotFile
 * EnvConditions: NA
 * CaseDescription: Verify SaveSnapshotFile
 */
HWTEST_F(MissionDataStorageTest, SaveSnapshotFile_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    missionDataStorage->SaveSnapshotFile(missionId, missionSnapshot);
}

/*
 * Feature: MissionListManager
 * Function: SaveSnapshotFile
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveSnapshotFile
 * EnvConditions: NA
 * CaseDescription: Verify SaveSnapshotFile
 */
HWTEST_F(MissionDataStorageTest, SaveSnapshotFile_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    missionSnapshot.isPrivate = true;
    missionSnapshot.snapshot = std::shared_ptr<Media::PixelMap>();
    missionDataStorage->SaveSnapshotFile(missionId, missionSnapshot);
}

/*
 * Feature: MissionListManager
 * Function: SaveSnapshotFile
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveSnapshotFile
 * EnvConditions: NA
 * CaseDescription: Verify SaveSnapshotFile
 */
HWTEST_F(MissionDataStorageTest, SaveSnapshotFile_003, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    missionSnapshot.isPrivate = false;
    missionSnapshot.snapshot = std::shared_ptr<Media::PixelMap>();
    missionDataStorage->SaveSnapshotFile(missionId, missionSnapshot);
}

/*
 * Feature: MissionListManager
 * Function: GetReducedPixelMap
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetReducedPixelMap
 * EnvConditions: NA
 * CaseDescription: Verify GetReducedPixelMap
 */
HWTEST_F(MissionDataStorageTest, GetReducedPixelMap_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    std::shared_ptr<OHOS::Media::PixelMap> snapshot = nullptr;
    auto res = missionDataStorage->GetReducedPixelMap(snapshot);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetReducedPixelMap
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetReducedPixelMap
 * EnvConditions: NA
 * CaseDescription: Verify GetReducedPixelMap
 */
HWTEST_F(MissionDataStorageTest, GetReducedPixelMap_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    std::shared_ptr<OHOS::Media::PixelMap> snapshot = std::shared_ptr<Media::PixelMap>();
    auto res = missionDataStorage->GetReducedPixelMap(snapshot);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetCachedSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetCachedSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetCachedSnapshot
 */
HWTEST_F(MissionDataStorageTest, GetCachedSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    bool res = missionDataStorage->GetCachedSnapshot(missionId, missionSnapshot);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: SaveCachedSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveCachedSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify SaveCachedSnapshot
 */
HWTEST_F(MissionDataStorageTest, SaveCachedSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    bool res = missionDataStorage->SaveCachedSnapshot(missionId, missionSnapshot);
    EXPECT_TRUE(res);
}

/*
 * Feature: MissionListManager
 * Function: SaveCachedSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage SaveCachedSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify SaveCachedSnapshot
 */
HWTEST_F(MissionDataStorageTest, SaveCachedSnapshot_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    missionDataStorage->SaveCachedSnapshot(missionId, missionSnapshot);
    bool res = missionDataStorage->SaveCachedSnapshot(missionId, missionSnapshot);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: DeleteCachedSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage DeleteCachedSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify DeleteCachedSnapshot
 */
HWTEST_F(MissionDataStorageTest, DeleteCachedSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    bool res = missionDataStorage->DeleteCachedSnapshot(missionId);
    EXPECT_FALSE(res);
}

/*
 * Feature: MissionListManager
 * Function: GetSnapshot
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetSnapshot
 * EnvConditions: NA
 * CaseDescription: Verify GetSnapshot
 */
HWTEST_F(MissionDataStorageTest, GetSnapshot_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    bool isLowResolution = true;
    std::shared_ptr<Media::PixelMap> res = missionDataStorage->GetSnapshot(missionId, isLowResolution);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetPixelMap
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetPixelMap
 * EnvConditions: NA
 * CaseDescription: Verify GetPixelMap
 */
HWTEST_F(MissionDataStorageTest, GetPixelMap_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    bool isLowResolution = true;
    std::unique_ptr<Media::PixelMap> res = missionDataStorage->GetPixelMap(missionId, isLowResolution);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: GetPixelMap
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage GetPixelMap
 * EnvConditions: NA
 * CaseDescription: Verify GetPixelMap
 */
HWTEST_F(MissionDataStorageTest, GetPixelMap_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    int32_t missionId = 0;
    MissionSnapshot missionSnapshot;
    missionSnapshot.snapshot = std::shared_ptr<Media::PixelMap>();
    missionDataStorage->SaveMissionSnapshot(missionId, missionSnapshot);
    bool isLowResolution = true;
    std::unique_ptr<Media::PixelMap> res = missionDataStorage->GetPixelMap(missionId, isLowResolution);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: MissionListManager
 * Function: WriteRgb888ToJpeg
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage WriteRgb888ToJpeg
 * EnvConditions: NA
 * CaseDescription: Verify WriteRgb888ToJpeg
 */
HWTEST_F(MissionDataStorageTest, WriteRgb888ToJpeg_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    char* fileName;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t* data;
    missionDataStorage->WriteRgb888ToJpeg(fileName, width, height, data);
}

/*
 * Feature: MissionListManager
 * Function: WriteRgb888ToJpeg
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage WriteRgb888ToJpeg
 * EnvConditions: NA
 * CaseDescription: Verify WriteRgb888ToJpeg
 */
HWTEST_F(MissionDataStorageTest, WriteRgb888ToJpeg_002, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    char* fileName;
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t* data = new uint8_t[height];
    missionDataStorage->WriteRgb888ToJpeg(fileName, width, height, data);
}

/*
 * Feature: MissionListManager
 * Function: RGB565ToRGB888
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage RGB565ToRGB888
 * EnvConditions: NA
 * CaseDescription: Verify RGB565ToRGB888
 */
HWTEST_F(MissionDataStorageTest, RGB565ToRGB888_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    uint16_t* rgb565Buf = nullptr;
    int32_t rgb565Size = 0;
    uint8_t* rgb888Buf = nullptr;
    int32_t rgb888Size = 0;
    missionDataStorage->RGB565ToRGB888(rgb565Buf, rgb565Size, rgb888Buf, rgb888Size);
}

/*
 * Feature: MissionListManager
 * Function: RGBA8888ToRGB888
 * SubFunction: NA
 * FunctionPoints: MissionDataStorage RGBA8888ToRGB888
 * EnvConditions: NA
 * CaseDescription: Verify RGBA8888ToRGB888
 */
HWTEST_F(MissionDataStorageTest, RGBA8888ToRGB888_001, TestSize.Level1)
{
    auto missionDataStorage = std::make_shared<MissionDataStorage>();
    uint32_t* rgba8888Buf = nullptr;
    int32_t rgba8888Size = 0;
    uint8_t* rgb888Buf = nullptr;
    int32_t rgb888Size = 0;
    missionDataStorage->RGBA8888ToRGB888(rgba8888Buf, rgba8888Size, rgb888Buf, rgb888Size);
}
}  // namespace AAFwk
}  // namespace OHOS
