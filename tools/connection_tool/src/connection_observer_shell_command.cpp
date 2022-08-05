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

#include "connection_observer_shell_command.h"

#include <sstream>
#include <iostream>

#include "hilog_wrapper.h"
#include "connection_observer_client.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
static const size_t MAX_LINE_INPUT = 256;
std::vector<std::string> SplitString(const std::string &str, const char* separators = " \t\r\n")
{
    std::vector<std::string> result;

    auto pos = str.find_first_not_of(separators);
    if (pos == std::string::npos) {
        return result;
    }

    do {
        auto start = pos;
        pos = str.find_first_of(separators, start);
        if (pos == std::string::npos) {
            result.emplace_back(str.substr(start));
            break;
        }
        result.emplace_back(str.substr(start, pos - start));
        pos = str.find_first_not_of(separators, pos);
    } while (pos != std::string::npos);

    return result;
}
}

ConnectionObserverShellCommand::ConnectionObserverShellCommand()
{
}

ConnectionObserverShellCommand::~ConnectionObserverShellCommand()
{
}

void ConnectionObserverShellCommand::Run()
{
    InitCommand();

    std::cout << "输入register注册" <<std::endl;
    std::cout << "输入unregister解注册" <<std::endl;

    while (!std::cin.eof()) {
        std::cout << "> " << std::flush;
        std::vector<std::string> inputs = ParseInputs(std::cin);
        if (inputs.empty()) {
            continue;
        }

        auto it = commands_.find(inputs[0]);
        if (it == commands_.end()) {
            std::cout << "invalid command" << std::endl;
            continue;
        }
        Process(it->second, inputs);
    }
}

void ConnectionObserverShellCommand::Process(const CommandFun &fun, const std::vector<std::string> &parts) const
{
    if (!fun) {
        return;
    }
    std::vector<std::string> newParts(parts.begin() + 1, parts.end());
    fun(newParts);
}

void ConnectionObserverShellCommand::InitCommand()
{
    commands_.emplace("register", std::bind(&ConnectionObserverShellCommand::RunRegister, this));
    commands_.emplace("unregister", std::bind(&ConnectionObserverShellCommand::RunUnRegister, this));
}

void ConnectionObserverShellCommand::RunRegister()
{
    if (!observer_) {
        observer_ = std::make_shared<TestConnectionObserver>();
    }

    auto result = ConnectionObserverClient::GetInstance().RegisterObserver(observer_);
    std::string registerResult = "RegiserObserver, result = " + std::to_string(result);
    HILOG_INFO("%{public}s", registerResult.c_str());
    std::cout<<registerResult<<std::endl;
}

void ConnectionObserverShellCommand::RunUnRegister()
{
    auto result = ConnectionObserverClient::GetInstance().UnregisterObserver(observer_);
    std::string registerResult = "UnregiserObserver, result = " + std::to_string(result);
    HILOG_INFO("%{public}s", registerResult.c_str());
    std::cout<<registerResult<<std::endl;
}

std::vector<std::string> ConnectionObserverShellCommand::ParseInputs(std::istream &in) const
{
    char buf[MAX_LINE_INPUT];
    in.getline(buf, sizeof(buf));
    return SplitString(buf);
}
} // namespace AbilityRuntime
} // namespace OHOS
