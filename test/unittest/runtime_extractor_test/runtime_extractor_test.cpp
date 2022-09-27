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

#include <fstream>
#include <gtest/gtest.h>
#include "runtime_extractor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string TEST_HAP_PATH("/system/app/com.ohos.settings/Settings.hap");
const std::string ERROR_HAP_PATH("/system/app/com.ohos.settings/XXX.hap");
const std::string MODULE_JSON_PATH("module.json");
const std::string OUT_PATH("/data/module.json");
const std::string MAIN_ABILITY_PATH("ets/MainAbility");
const std::string ERROR_PATH("ets/MainAbilityXXX");
const std::string MAIN_ABILITY_FILENAME("ets/MainAbility/MainAbility.abc");
const std::string ERROR_FILENAME("ets/MainAbility/XXX.abc");
}
class RuntimeExtractorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RuntimeExtractorTest::SetUpTestCase()
{}

void RuntimeExtractorTest::TearDownTestCase()
{}

void RuntimeExtractorTest::SetUp()
{}

void RuntimeExtractorTest::TearDown()
{}

/*
 * Feature: RuntimeExtractor
 * Function: Init
 * SubFunction: NA
 * FunctionPoints:Init runtime extractor
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call Init function.
 */
HWTEST_F(RuntimeExtractorTest, RuntimeExtractorInit_001, TestSize.Level1)
{
    std::string loadPath;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor1 = std::make_shared<RuntimeExtractor>(loadPath);
    EXPECT_FALSE(runtimeExtractor1->Init());

    loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor2 = std::make_shared<RuntimeExtractor>(loadPath);
    EXPECT_TRUE(runtimeExtractor2->Init());
}

/*
 * Feature: RuntimeExtractor
 * Function: Create
 * SubFunction: NA
 * FunctionPoints:Create runtime extractor
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor.
 */
HWTEST_F(RuntimeExtractorTest, RuntimeExtractorCreate_001, TestSize.Level1)
{
    std::string loadPath;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor1 = RuntimeExtractor::Create(loadPath);
    EXPECT_TRUE(runtimeExtractor1 == nullptr);

    loadPath = ERROR_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor2 = RuntimeExtractor::Create(loadPath);
    EXPECT_TRUE(runtimeExtractor2 == nullptr);

    loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor3 = RuntimeExtractor::Create(loadPath);
    EXPECT_TRUE(runtimeExtractor3 != nullptr);
}

/*
 * Feature: RuntimeExtractor
 * Function: GetFileBuffer
 * SubFunction: NA
 * FunctionPoints:Get file buffer
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call get file buffer function.
 */
HWTEST_F(RuntimeExtractorTest, GetFileBuffer_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::ostringstream outStream;
    std::string srcPath = MODULE_JSON_PATH;
    EXPECT_FALSE(runtimeExtractor->GetFileBuffer(srcPath, outStream));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->GetFileBuffer("", outStream));
    EXPECT_TRUE(runtimeExtractor->GetFileBuffer(srcPath, outStream));
    EXPECT_TRUE(sizeof(outStream) > 0);
}

/*
 * Feature: RuntimeExtractor
 * Function: GetFileList
 * SubFunction: NA
 * FunctionPoints:Get file list
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call get file list function.
 */
HWTEST_F(RuntimeExtractorTest, GetFileList_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::vector<std::string> fileList;
    std::string srcPath = MAIN_ABILITY_PATH;
    EXPECT_FALSE(runtimeExtractor->GetFileList(srcPath, fileList));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->GetFileList("", fileList));
    EXPECT_TRUE(runtimeExtractor->GetFileList(srcPath, fileList));
    EXPECT_TRUE(fileList.size() > 0);
}

/*
 * Feature: RuntimeExtractor
 * Function: HasEntry
 * SubFunction: NA
 * FunctionPoints:Has entry
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call has entry function.
 */
HWTEST_F(RuntimeExtractorTest, HasEntry_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::string fileName = MAIN_ABILITY_FILENAME;
    EXPECT_FALSE(runtimeExtractor->HasEntry(fileName));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->HasEntry(""));
    EXPECT_FALSE(runtimeExtractor->HasEntry(ERROR_FILENAME));
    EXPECT_TRUE(runtimeExtractor->HasEntry(fileName));
}

/*
 * Feature: RuntimeExtractor
 * Function: IsDirExist
 * SubFunction: NA
 * FunctionPoints:Is dir exist
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call is dir exist function.
 */
HWTEST_F(RuntimeExtractorTest, IsDirExist_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::string srcPath = MAIN_ABILITY_PATH;
    EXPECT_FALSE(runtimeExtractor->IsDirExist(srcPath));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->IsDirExist(""));
    EXPECT_FALSE(runtimeExtractor->IsDirExist(ERROR_PATH));
    EXPECT_TRUE(runtimeExtractor->IsDirExist(srcPath));
}

/*
 * Feature: RuntimeExtractor
 * Function: ExtractByName
 * SubFunction: NA
 * FunctionPoints:Extract by name
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call extract by name function.
 */
HWTEST_F(RuntimeExtractorTest, ExtractByName_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::ostringstream outStream;
    std::string srcPath = MODULE_JSON_PATH;
    EXPECT_FALSE(runtimeExtractor->ExtractByName(srcPath, outStream));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->ExtractByName("", outStream));
    EXPECT_TRUE(runtimeExtractor->ExtractByName(srcPath, outStream));
    EXPECT_TRUE(sizeof(outStream) > 0);
}

/*
 * Feature: RuntimeExtractor
 * Function: ExtractFile
 * SubFunction: NA
 * FunctionPoints:Extract file
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call extract file function.
 */
HWTEST_F(RuntimeExtractorTest, ExtractFile_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::string outPath = OUT_PATH;
    std::string srcPath = MODULE_JSON_PATH;
    EXPECT_FALSE(runtimeExtractor->ExtractFile(srcPath, outPath));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->ExtractFile("", outPath));
    EXPECT_TRUE(runtimeExtractor->ExtractFile(srcPath, outPath));
    std::ifstream f(outPath.c_str());
    EXPECT_TRUE(f.good());
}

/*
 * Feature: RuntimeExtractor
 * Function: GetZipFileNames
 * SubFunction: NA
 * FunctionPoints:Get zip file names
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call get zip file names function.
 */
HWTEST_F(RuntimeExtractorTest, GetZipFileNames_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::vector<std::string> fileList;
    EXPECT_TRUE(runtimeExtractor->GetZipFileNames(fileList));
    EXPECT_TRUE(fileList.size() == 0);

    runtimeExtractor->Init();
    EXPECT_TRUE(runtimeExtractor->GetZipFileNames(fileList));
    EXPECT_TRUE(fileList.size() > 0);
}

/*
 * Feature: RuntimeExtractor
 * Function: GetSpecifiedTypeFiles
 * SubFunction: NA
 * FunctionPoints:Get specified type files
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call get specified type files function.
 */
HWTEST_F(RuntimeExtractorTest, GetSpecifiedTypeFiles_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::vector<std::string> fileList;
    runtimeExtractor->GetSpecifiedTypeFiles(fileList, ".abc");
    EXPECT_TRUE(fileList.size() == 0);
    runtimeExtractor->Init();
    runtimeExtractor->GetSpecifiedTypeFiles(fileList, ".abc");
    EXPECT_TRUE(fileList.size() > 0);
}

/*
 * Feature: RuntimeExtractor
 * Function: IsStageBasedModel
 * SubFunction: NA
 * FunctionPoints:Is stage based model
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call is stage based model function.
 */
HWTEST_F(RuntimeExtractorTest, IsStageBasedModel_001, TestSize.Level1)
{
    std::string loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    std::vector<std::string> fileList;
    EXPECT_FALSE(runtimeExtractor->IsStageBasedModel("MainAbility"));

    runtimeExtractor->Init();
    EXPECT_FALSE(runtimeExtractor->IsStageBasedModel("MainAbility"));
}

/*
 * Feature: RuntimeExtractor
 * Function: IsSameHap
 * SubFunction: NA
 * FunctionPoints:Is same hap
 * EnvConditions: NA
 * CaseDescription: Create runtime extractor, call is same hap function.
 */
HWTEST_F(RuntimeExtractorTest, IsSameHap_001, TestSize.Level1)
{
    std::string loadPath;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    EXPECT_FALSE(runtimeExtractor->IsSameHap(""));

    loadPath = TEST_HAP_PATH;
    std::shared_ptr<RuntimeExtractor> runtimeExtractor1 = std::make_shared<RuntimeExtractor>(loadPath);
    runtimeExtractor1->Init();
    EXPECT_FALSE(runtimeExtractor1->IsSameHap(""));
    EXPECT_FALSE(runtimeExtractor1->IsSameHap(ERROR_HAP_PATH));
    EXPECT_TRUE(runtimeExtractor1->IsSameHap(TEST_HAP_PATH));
}
}  // namespace AbilityRuntime
}  // namespace OHOS
