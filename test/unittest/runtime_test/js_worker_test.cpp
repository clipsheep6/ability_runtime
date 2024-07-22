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

#include <gtest/gtest.h>
#include <cstdarg>
#include <string>

#include "js_environment_impl.h"
#define private public
#define protected public
#include "js_worker.h"
#undef private
#undef protected
#include "native_engine.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class JsWorkerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void TestSetGetAssetFunc(GetAssetFunc func);
    GetAssetFunc TestGetGetAssetFunc() const;

private:
    GetAssetFunc getAssetFunc_ = nullptr;
};

void JsWorkerTest::SetUpTestCase()
{}

void JsWorkerTest::TearDownTestCase()
{}

void JsWorkerTest::SetUp()
{}

void JsWorkerTest::TearDown()
{}

void JsWorkerTest::TestSetGetAssetFunc(GetAssetFunc func)
{
    getAssetFunc_ = func;
}

GetAssetFunc JsWorkerTest::TestGetGetAssetFunc() const
{
    return getAssetFunc_;
}

/**
 * @tc.name: AssetHelper_0100
 * @tc.desc: Asset helper.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, AssetHelper_0100, TestSize.Level1)
{
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect("/data/test/codePath");
    workerInfo->packagePathStr = "/data/test/packagePath";
    workerInfo->hapPath = panda::panda_file::StringPacProtect("/data/test/hapPath");
    workerInfo->moduleName = "moduleName";
    TestSetGetAssetFunc(AssetHelper(workerInfo));

    std::string uri = "/data";
    uint8_t *buff = nullptr;
    size_t buffSize;
    std::vector<uint8_t> content;
    std::string ami;
    bool useSecureMem;
    bool isRestricted = false;
    auto func = TestGetGetAssetFunc();
    func("/data", &buff, &buffSize, content, ami, useSecureMem, isRestricted);
    EXPECT_EQ(useSecureMem, false);
}

/**
 * @tc.name: AssetHelper_0200
 * @tc.desc: Asset helper GetSafeData.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, AssetHelper_0200, TestSize.Level1)
{
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect("/data/test/codePath");
    workerInfo->packagePathStr = "/data/test/packagePath";
    workerInfo->hapPath = panda::panda_file::StringPacProtect("/data/test/hapPath");
    workerInfo->moduleName = "moduleName";
    AssetHelper helper = AssetHelper(workerInfo);

    FILE *fp = nullptr;
    fp = fopen("test.txt", "w+");
    ASSERT_NE(fp, nullptr);
    fclose(fp);

    uint8_t *buff = nullptr;
    size_t buffSize;
    auto ret = helper.GetSafeData("test.txt", &buff, &buffSize);
    EXPECT_EQ(ret, false);
}
} // namespace AbilityRuntime
} // namespace OHOS
