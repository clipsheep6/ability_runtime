/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "test_observer.h"

#include <cinttypes>
#include <iostream>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "shell_command_config_loader.h"
#include "shell_command_executor.h"
#include "system_time.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr const char* AA_TOOL_COMMAND_CONFIG = "/system/etc/shell_command_executor_config.json";
}

TestObserver::TestObserver() : isFinished_(false)
{}

TestObserver::~TestObserver()
{}

void TestObserver::TestStatus(const std::string& msg, const int64_t& resultCode)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);
    printf("%s\n", msg.data());
    fflush(stdout);
}

void TestObserver::TestFinished(const std::string& msg, const int64_t& resultCode)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);
    std::cout << "TestFinished-ResultCode: " + std::to_string(resultCode) << std::endl;
    std::cout << "TestFinished-ResultMsg: " + msg << std::endl;
    isFinished_ = true;
}

ShellCommandResult TestObserver::ExecuteShellCommand(const std::string& cmd, const int64_t timeoutSec)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter, cmd : \"%{public}s\", timeoutSec : %{public}" PRId64, cmd.data(), timeoutSec);

    auto cmdExecutor = std::make_shared<ShellCommandExecutor>(cmd, timeoutSec);
    if (!cmdExecutor) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to create ShellCommandExecutor intance");
        return {};
    }

    if (!std::make_shared<ShellCommandConfigLoder>()->ReadConfig(AA_TOOL_COMMAND_CONFIG)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to read config");
        return {};
    }
    
    return cmdExecutor->WaitWorkDone();
}

bool TestObserver::WaitForFinish(const int64_t& timeoutMs)
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter");

    auto realTime = timeoutMs > 0 ? timeoutMs : 0;
    int64_t startTime = SystemTime::GetNowSysTime();
    while (!isFinished_) {
        int64_t nowSysTime = SystemTime::GetNowSysTime();
        if (realTime && (nowSysTime - startTime > realTime)) {
            return false;
        }
        sleep(1);
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "User test finished");
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
