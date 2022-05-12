/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_OHOS_SHELL_CMD_RESULT_H
#define FOUNDATION_APPEXECFWK_OHOS_SHELL_CMD_RESULT_H

#include <string>
#include "shell_command_result.h"

namespace OHOS {
namespace AppExecFwk {
class ShellCmdResult {
public:
    ShellCmdResult() = default;
    ShellCmdResult(const int32_t exitCode, const std::string &stdResult);
    ShellCmdResult(const AAFwk::ShellCommandResult &result);
    ~ShellCmdResult() = default;

    void SetExitCode(const int32_t exitCode);
    int32_t GetExitCode() const;

    void SetStdResult(const std::string &stdResult);
    std::string GetStdResult() const;

    std::string Dump();

private:
    int32_t exitCode_ {-1};
    std::string stdResult_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_SHELL_CMD_RESULT_H
