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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_TOOL_COMMAND_H
#define OHOS_ABILITY_RUNTIME_ABILITY_TOOL_COMMAND_H

#include <getopt.h>
#include "ability_command.h"
#include "shell_command.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityToolCommand : public ShellCommand {
public:
    AbilityToolCommand(int argc, char* argv[]);
    ~AbilityToolCommand() override {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsStopService();
    ErrCode RunAsForceStop();
    ErrCode RunAsTestCommand();

    bool ParseStartAbilityArgsOtherOption(int optind, std::string& paramName, std::string& paramValue, std::smatch& sm,
        int32_t& windowMode);
    bool ParseStartAbilityArgsFlags(std::string& paramValue, std::smatch& sm, int& flags);
    ErrCode ParseStartAbilityArgsParameterCheck(const std::string& abilityName, const std::string& bundleName);
    void ParseStartAbilityArgsSetWant(bool isColdStart, bool isDebugApp, int flags, Want& want);
    ErrCode ParseStartAbilityArgsSetStartOptions(StartOptions& startOptions, int32_t windowMode);
    ErrCode ParseStartAbilityArgsFromCmd(Want& want, StartOptions& startOptions);
    ErrCode ParseStopServiceArgsFromCmd(Want& want);
    ErrCode ParseTestArgsFromCmd(std::map<std::string, std::string>& params);
    bool GetKeyAndValueByOpt(int optind, std::string& key, std::string& value);

    std::shared_ptr<AbilityManagerShellCommand> aaShellCmd_;
    struct option startAbilityLongOptions[9] = {
        {"help", no_argument, nullptr, 'h'},
        {"device", required_argument, nullptr, 'd'},
        {"ability", required_argument, nullptr, 'a'},
        {"bundle", required_argument, nullptr, 'b'},
        {"options", required_argument, nullptr, 'o'},
        {"flags", required_argument, nullptr, 'f'},
        {"cold-start", no_argument, nullptr, 'C'},
        {"debug", no_argument, nullptr, 'D'},
        {nullptr, 0, nullptr, 0},
    };
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_TOOL_COMMAND_H
