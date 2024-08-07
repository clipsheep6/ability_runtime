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

#include "shell_command_executor.h"

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "shell_command_config_loader.h"

using namespace std::chrono_literals;
namespace OHOS {
namespace AAFwk {
ShellCommandExecutor::ShellCommandExecutor(const std::string& cmd, const int64_t timeoutSec)
    : cmd_(cmd), timeoutSec_(timeoutSec)
{
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
}

ShellCommandResult ShellCommandExecutor::WaitWorkDone()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter");

    if (!DoWork()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "Failed to execute command : \"%{public}s\"", cmd_.data());
        return cmdResult_;
    }

    std::unique_lock<std::mutex> workLock(mtxWork_);

    auto condition = [this]() { return isDone_; };
    if (timeoutSec_ <= 0) {
        cvWork_.wait(workLock, condition);
    } else if (!cvWork_.wait_for(workLock, timeoutSec_ * 1s, condition)) {
        TAG_LOGW(AAFwkTag::AA_TOOL, "Command execution timed out! cmd : \"%{public}s\", timeoutSec : %{public}" PRId64,
            cmd_.data(), timeoutSec_);
        std::cout << "Warning! Command execution timed out! cmd : " << cmd_ << ", timeoutSec : " << timeoutSec_
            << std::endl;

        ShellCommandResult realResult;
        realResult.exitCode = -1;
        {
            std::lock_guard<std::mutex> copyLock(mtxCopy_);
            realResult.stdResult = cmdResult_.stdResult;
        }
        return realResult;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "Command execution complete, cmd : \"%{public}s\", exitCode : %{public}d",
        cmd_.data(), cmdResult_.exitCode);
    return cmdResult_;
}

bool ShellCommandExecutor::DoWork()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "enter");

    if (cmd_.empty()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid command");
        return false;
    }

    if (!handler_) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid event handler");
        return false;
    }
    
    if (!CheckCommand()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid command");
        return false;
    }

    auto self(shared_from_this());
    handler_->PostTask([this, self]() {
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork async task begin, cmd : \"%{public}s\"", cmd_.data());

        FILE* file = popen(cmd_.c_str(), "r");
        HILOG_INFO("DoWork async task 93 line, cmd : \"%{public}s\"", cmd_.data());
        if (!file) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "Failed to call popen, cmd : \"%{public}s\"", cmd_.data());

            {
                std::unique_lock<std::mutex> workLock(mtxWork_);
                isDone_ = true;
            }
            cvWork_.notify_one();
            TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork async task end, cmd : \"%{public}s\"", cmd_.data());
            return;
        }
        HILOG_INFO("DoWork async task 105 line, cmd : \"%{public}s\"", cmd_.data());
        char commandResult[1024] = { '\0' };
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            {
                HILOG_INFO("DoWork async task 109 line");
                std::lock_guard<std::mutex> copyLock(mtxCopy_);
                HILOG_INFO("DoWork async task 111 line");
                cmdResult_.stdResult.append(commandResult);
                HILOG_INFO("DoWork async task 113 line");
                
            }
            HILOG_INFO("DoWork async task 116 line");
            std::cout << commandResult;
            HILOG_INFO("DoWork async task 118 line");
        }
        HILOG_INFO("DoWork async task 120 line");
        cmdResult_.exitCode = pclose(file);
        file = nullptr;
        HILOG_INFO("DoWork async task 123 line");
        {
            std::unique_lock<std::mutex> workLock(mtxWork_);
            isDone_ = true;
        }
        HILOG_INFO("DoWork async task 128 line");
        cvWork_.notify_one();
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork async task end, cmd : \"%{public}s\"", cmd_.data());
    });

    return true;
}

bool ShellCommandExecutor::CheckCommand()
{
    std::istringstream iss(cmd_);
    std::string firstCommand = "";
    iss >> firstCommand;
    if (ShellCommandConfigLoader::commands_.find(firstCommand) != ShellCommandConfigLoader::commands_.end()) {
        return true;
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS
