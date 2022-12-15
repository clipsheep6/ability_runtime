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

#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define private public
#include "mix_stack_dumper.h"
#undef private
#include "mock_runtime.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class MixStackDumperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MixStackDumperTest::SetUpTestCase()
{}

void MixStackDumperTest::TearDownTestCase()
{}

void MixStackDumperTest::SetUp()
{}

void MixStackDumperTest::TearDown()
{}

static std::string GetCmdResultFromPopen(const std::string& cmd)
{
    if (cmd.empty()) {
        return "";
    }
    FILE* fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return "";
    }
    const int bufSize = 128; // 128 : cmd result buf size
    char buffer[bufSize];
    std::string result = "";
    while (!feof(fp)) {
        if (fgets(buffer, bufSize - 1, fp) != nullptr) {
            result += buffer;
        }
    }
    pclose(fp);
    return result;
}

static int GetServicePid(const std::string& serviceName)
{
    std::string cmd = "pidof " + serviceName;
    std::string pidStr = GetCmdResultFromPopen(cmd);
    int32_t pid = 0;
    std::stringstream pidStream(pidStr);
    pidStream >> pid;
    return pid;
}

static bool CheckMixStackKeyWords(const char *filePath, std::string *keywords, int length)
{
    std::ifstream file;
    file.open(filePath, std::ios::in);
    std::vector<std::string> buf(128); // 128 : buf size
    int cnt = 0;
    int i = 0;
    int j = 0;
    std::string::size_type idx;
    while (!file.eof()) {
        file >> buf.at(i);
        idx = buf.at(i).find(keywords[j]);
        if (idx != std::string::npos) {
            GTEST_LOG_(INFO) << buf.at(i);
            cnt++;
            j++;
            if (j == length) {
                break;
            }
            continue;
        }
        i++;
    }
    file.close();
    return cnt == length;
}

/**
 * @tc.number: MixStackDumperTest001
 * @tc.name: dump com.ohos.systemui process
 * @tc.desc: try to dump com.ohos.systemui process, must be failed.
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest001, Function | MediumTest | Level3)
{
    char testFile[] = "/data/mix_stack_header_test01";
    int fd = open(testFile, O_RDWR | O_CREAT, 0755); // 0755 : -rwxr-xr-x
    if (fd == -1) {
        GTEST_LOG_(ERROR) << "Failed to create test file.";
        return;
    }
    MixStackDumper mixDumper;
    pid_t pid = GetServicePid("com.ohos.systemui");
    mixDumper.Init(pid);
    bool ret = mixDumper.DumpMixFrame(fd, pid, pid);
    mixDumper.Destroy();
    EXPECT_FALSE(ret);
    close(fd);
    std::string keywords[] = {
        "Tid:" + std::to_string(pid), "Failed", "suspend",
    };
    int length = sizeof(keywords) / sizeof(keywords[0]);
    EXPECT_TRUE(CheckMixStackKeyWords(testFile, keywords, length));
}

/**
 * @tc.number: MixStackDumperTest002
 * @tc.name: test DumpMixFrame Func
 * @tc.desc: dump current process which is not a applicaiton process
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest002, Function | MediumTest | Level3)
{
    char testFile[] = "/data/mix_stack_header_test02";
    int fd = open(testFile, O_RDWR | O_CREAT, 0755); // 0755 : -rwxr-xr-x
    if (fd == -1) {
        GTEST_LOG_(ERROR) << "Failed to create test file.";
        return;
    }
    MixStackDumper mixDumper;
    bool ret = mixDumper.DumpMixFrame(fd, getpid(), getpid());
    EXPECT_FALSE(ret);
    mixDumper.Init(getpid());
    ret = mixDumper.DumpMixFrame(fd, -1, -1);
    EXPECT_FALSE(ret);
    ret = mixDumper.DumpMixFrame(fd, getpid(), getpid());
    mixDumper.Destroy();
    EXPECT_TRUE(ret);
    close(fd);
    std::string keywords[] = {
        "Tid:-1", "Failed", "suspend", "Tid:" + std::to_string(getpid()), "#00", "pc",
        "libappkit_native.z.so", "mix_stack_dumper_test",
    };
    int length = sizeof(keywords) / sizeof(keywords[0]);
    EXPECT_TRUE(CheckMixStackKeyWords(testFile, keywords, length));
}

/**
 * @tc.number: MixStackDumperTest003
 * @tc.name: Call BuildJsStackInfoList Func
 * @tc.desc: test JsRuntime BuildJsStackInfoList Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest003, Function | MediumTest | Level3)
{
    AbilityRuntime::MockRuntime runtime;
    std::vector<JsFrames> frames;
    bool ret = runtime.BuildJsStackInfoList(gettid(), frames);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: MixStackDumperTest004
 * @tc.name: Call PrintNativeFrames Func
 * @tc.desc: test PrintNativeFrames Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest004, Function | MediumTest | Level3)
{
    std::shared_ptr<OHOS::HiviewDFX::DfxFrame> nativeFrame1 = std::make_shared<OHOS::HiviewDFX::DfxFrame>();
    nativeFrame1->SetFrameMapName("testmapname");
    std::shared_ptr<OHOS::HiviewDFX::DfxFrame> nativeFrame2 = std::make_shared<OHOS::HiviewDFX::DfxFrame>();
    std::vector<std::shared_ptr<OHOS::HiviewDFX::DfxFrame>> nativeFrames;
    nativeFrames.emplace_back(nativeFrame1);
    nativeFrames.emplace_back(nativeFrame2);
    nativeFrames.emplace_back(nullptr);
    MixStackDumper mixDumper;
    char testFile[] = "/data/mix_stack_header_test04";
    int fd = open(testFile, O_RDWR | O_CREAT, 0755); // 0755 : -rwxr-xr-x
    if (fd == -1) {
        GTEST_LOG_(ERROR) << "Failed to create test file.";
        mixDumper.PrintNativeFrames(1, nativeFrames);
        EXPECT_TRUE(true);
    } else {
        mixDumper.PrintNativeFrames(fd, nativeFrames);
        close(fd);
        std::string keywords[] = {
            "#00", "pc", "testmapname", "Unknown",
        };
        int length = sizeof(keywords) / sizeof(keywords[0]);
        EXPECT_TRUE(CheckMixStackKeyWords(testFile, keywords, length));
    }
}

/**
 * @tc.number: MixStackDumperTest005
 * @tc.name: Call GetThreadStackTraceLabel Func
 * @tc.desc: test GetThreadStackTraceLabel Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest005, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    std::string label = mixDumper.GetThreadStackTraceLabel(gettid());
    GTEST_LOG_(INFO) << label;
    std::string keyword = "mix_stack_dump";
    EXPECT_TRUE(label.find(keyword) != std::string::npos);
}

/**
 * @tc.number: MixStackDumperTest006
 * @tc.name: Call HandleMixDumpRequest Func
 * @tc.desc: test HandleMixDumpRequest Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest006, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    mixDumper.HandleMixDumpRequest();
    EXPECT_TRUE(true);
}

/**
 * @tc.number: MixStackDumperTest007
 * @tc.name: Call PrintProcessHeader Func
 * @tc.desc: test PrintProcessHeader Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest007, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    char testFile[] = "/data/mix_stack_header_test07";
    int fd = open(testFile, O_RDWR | O_CREAT, 0755); // 0755 : -rwxr-xr-x
    if (fd == -1) {
        GTEST_LOG_(ERROR) << "Failed to create test file.";
        mixDumper.PrintProcessHeader(1, getpid(), getuid());
        EXPECT_TRUE(true);
    } else {
        mixDumper.PrintProcessHeader(fd, getpid(), getuid());
        close(fd);
        std::string headerKeywords[] = {
            "Timestamp:", "Pid:" + std::to_string(getpid()), "Uid:" + std::to_string(getuid()), "mix_stack_dumper_test",
        };
        int length = sizeof(headerKeywords) / sizeof(headerKeywords[0]);
        EXPECT_TRUE(CheckMixStackKeyWords(testFile, headerKeywords, length));
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
