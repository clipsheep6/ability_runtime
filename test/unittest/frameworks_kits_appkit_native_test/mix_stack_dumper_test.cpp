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
#define protected public
#include "mix_stack_dumper.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "mock_runtime.h"
#include "runtime.h"
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int32_t ZERO = 0;
    constexpr int32_t ONE = 1;
    constexpr int32_t NATIVE_DUMP = -1;
    constexpr int32_t MIX_DUMP = -2;
}
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
        "Tid:" + std::to_string(getpid()), "#00", "pc", "mix_stack_dumper_test",
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

/**
 * @tc.number: MixStackDumperTest008
 * @tc.name: dump com.ohos.systemui process
 * @tc.desc: try to dump com.ohos.systemui process, must be failed.
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest008, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    mixDumper.Destroy();
    pid_t pid = GetServicePid("com.ohos.systemui");
    mixDumper.Init(pid);
    bool ret = mixDumper.DumpMixFrame(1, pid, pid);
    mixDumper.Destroy();
    mixDumper.HandleMixDumpRequest();
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: MixStackDumperTest009
 * @tc.name: test DumpMixFrame Func
 * @tc.desc: dump current process which is not a applicaiton process
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest009, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    bool ret = mixDumper.DumpMixFrame(1, getpid(), getpid());
    EXPECT_FALSE(ret);
    mixDumper.Init(getpid());
    ret = mixDumper.DumpMixFrame(1, -1, -1);
    EXPECT_FALSE(ret);
    ret = mixDumper.DumpMixFrame(1, getpid(), getpid());
    mixDumper.Destroy();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: MixStackDumperTest0010
 * @tc.name: Call BuildJsStackInfoList Func
 * @tc.desc: test JsRuntime BuildJsStackInfoList Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest010, Function | MediumTest | Level3)
{
    AbilityRuntime::MockRuntime runtime;
    std::vector<JsFrames> frames;
    bool ret = runtime.BuildJsStackInfoList(gettid(), frames);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: MixStackDumperTest011
 * @tc.name: Call GetThreadStackTraceLabel Func
 * @tc.desc: test GetThreadStackTraceLabel Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest011, Function | MediumTest | Level3)
{
    MixStackDumper mixDumper;
    std::string label = mixDumper.GetThreadStackTraceLabel(gettid());
    EXPECT_TRUE(label != "");
}

/**
 * @tc.number: MixStackDumperTest013
 * @tc.name: Call DumpMixFrame Func
 * @tc.desc: test DumpMixFrame Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest013, Function | MediumTest | Level1)
{
    std::shared_ptr<MixStackDumper> mixDumper = std::make_shared<MixStackDumper>();
    std::shared_ptr<OHOSApplication> ohosApplication = std::make_shared<OHOSApplication>();
    mixDumper->application_ = ohosApplication;
    auto application = mixDumper->application_.lock();
    EXPECT_TRUE(application);
    mixDumper->Init(getpid());
    mixDumper->DumpMixFrame(ONE, getpid(), getpid());
    std::unique_ptr<AbilityRuntime::MockRuntime> mockRuntime = std::make_unique<AbilityRuntime::MockRuntime>();
    application->runtime_ = std::move(mockRuntime);
    EXPECT_TRUE(application->GetRuntime());
    mixDumper->Init(getpid());
    bool ret = mixDumper->DumpMixFrame(ONE, getpid(), getpid());
    EXPECT_TRUE(ret);
    mixDumper->catcher_->procInfo_.pid = ZERO;
    EXPECT_EQ(mixDumper->catcher_->procInfo_.pid, ZERO);
    mixDumper->DumpMixFrame(ONE, ZERO, getpid());
    mixDumper->application_.reset();
}

/**
 * @tc.number: MixStackDumperTest014
 * @tc.name: Call BuildJsNativeMixStack Func
 * @tc.desc: test BuildJsNativeMixStack Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest014, Function | MediumTest | Level1)
{
    MixStackDumper mixDumper;
    std::vector<OHOS::HiviewDFX::DfxFrame> v_nativeFrames;
    struct JsFrames jsFrames;
    jsFrames.fileName = "fileName";
    jsFrames.functionName = "functionName";
    jsFrames.pos = "pos";
    jsFrames.nativePointer = nullptr;
    std::vector<JsFrames> v_jsFrames;
    EXPECT_TRUE(v_jsFrames.size() == ZERO);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
    v_jsFrames.push_back(jsFrames);
    EXPECT_TRUE(v_jsFrames.size() > ZERO);
    EXPECT_TRUE(v_jsFrames[ZERO].nativePointer == nullptr);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
    v_jsFrames.clear();
    unsigned int data = 10;
    jsFrames.nativePointer = reinterpret_cast<uintptr_t*>(&data);
    v_jsFrames.push_back(jsFrames);
    EXPECT_TRUE(v_jsFrames.size() > ZERO);
    EXPECT_TRUE(v_jsFrames[ZERO].nativePointer != nullptr);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
    EXPECT_TRUE(v_nativeFrames.size() == ZERO);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
    HiviewDFX::DfxFrame nativeFrame1;
    nativeFrame1.mapName = "testmapname";
    nativeFrame1.pc = (uint64_t)(v_jsFrames[ZERO].nativePointer);
    v_nativeFrames.push_back(nativeFrame1);
    bool ret1 = mixDumper.IsJsNativePcEqual(v_jsFrames[ZERO].nativePointer, v_nativeFrames[ZERO].pc,
        v_nativeFrames[ZERO].funcOffset);
    EXPECT_TRUE(ret1);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
    v_nativeFrames[ZERO].pc = ZERO;
    v_nativeFrames.push_back(nativeFrame1);
    bool ret2 = mixDumper.IsJsNativePcEqual(v_jsFrames[ZERO].nativePointer, v_nativeFrames[ZERO].pc,
        v_nativeFrames[ZERO].funcOffset);
    EXPECT_FALSE(ret2);
    mixDumper.BuildJsNativeMixStack(ONE, v_jsFrames, v_nativeFrames);
}

/**
 * @tc.number: MixStackDumperTest015
 * @tc.name: Call Dump_SignalHandler Func
 * @tc.desc: test Dump_SignalHandler Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest015, Function | MediumTest | Level1)
{
    MixStackDumper mixDumper;
    siginfo_t sign;
    sign.si_code = NATIVE_DUMP;
    mixDumper.Dump_SignalHandler(ZERO, &sign, nullptr);
    sign.si_code = ZERO;
    mixDumper.Dump_SignalHandler(ZERO, &sign, nullptr);
    sign.si_code = MIX_DUMP;
    mixDumper.Dump_SignalHandler(ZERO, &sign, nullptr);
    std::shared_ptr<EventHandler> eventHandler = std::make_shared<EventHandler>();
    mixDumper.signalHandler_ = eventHandler;
    auto handler = mixDumper.signalHandler_.lock();
    EXPECT_TRUE(handler);
    mixDumper.Dump_SignalHandler(ZERO, &sign, nullptr);
    mixDumper.signalHandler_.reset();
}

/**
 * @tc.number: MixStackDumperTest016
 * @tc.name: Call Dump_SignalHandler Func
 * @tc.desc: test Dump_SignalHandler Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest016, Function | MediumTest | Level1)
{
    MixStackDumper mixDumper;
    siginfo_t sign;
    sign.si_code = MIX_DUMP;
    mixDumper.signalHandler_.reset();
    bool ret = mixDumper.Dump_SignalHandler(ZERO, &sign, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: MixStackDumperTest017
 * @tc.name: Call GetMixStack
 * @tc.desc: test GetMixStack Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest017, Function | MediumTest | Level1)
{
    std::string stack0 = MixStackDumper::GetMixStack(false);
    EXPECT_TRUE(stack0.size() > ZERO);

    std::string stack1 = MixStackDumper::GetMixStack(true);
    EXPECT_TRUE(stack1.size() > ZERO);
}

/**
 * @tc.number: MixStackDumperTest018
 * @tc.name: Call InstallDumpHandler
 * @tc.desc: test InstallDumpHandler Func
 */
HWTEST_F(MixStackDumperTest, MixStackDumperTest018, Function | MediumTest | Level1)
{
    MixStackDumper mixDumepr;
    EXPECT_FALSE(mixDumepr.IsInstalled());
    mixDumepr.InstallDumpHandler(nullptr, nullptr);
    EXPECT_TRUE(mixDumepr.IsInstalled());
    mixDumepr.InstallDumpHandler(nullptr, nullptr); // install twice should not trigger abort crash
    EXPECT_TRUE(mixDumepr.IsInstalled());
}
}  // namespace AppExecFwk
}  // namespace OHOS
