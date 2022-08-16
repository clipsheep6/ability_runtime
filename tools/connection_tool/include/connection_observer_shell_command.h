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

#ifndef OHOS_ABILITY_RUNTIME_CONNECTION_OBSERVER_SHELL_COMMAND_H
#define OHOS_ABILITY_RUNTIME_CONNECTION_OBSERVER_SHELL_COMMAND_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

#include "test_connection_observer.h"

namespace OHOS {
namespace AbilityRuntime {
class ConnectionObserverShellCommand {
public:
    using CommandFun = std::function<void(const std::vector<std::string>&)>;
    ConnectionObserverShellCommand();
    virtual ~ConnectionObserverShellCommand();

    void Run();

private:
    void RunRegister();
    void RunUnRegister();
    void InitCommand();
    void Process(const CommandFun &fun, const std::vector<std::string> &parts) const;
    std::vector<std::string> ParseInputs(std::istream &in) const;

private:
    std::shared_ptr<TestConnectionObserver> observer_;
    std::unordered_map<std::string, CommandFun> commands_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CONNECTION_OBSERVER_SHELL_COMMAND_H
