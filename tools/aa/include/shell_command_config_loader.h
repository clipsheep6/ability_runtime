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

#ifndef OHOS_ABILITY_RUNTIME_SHELL_COMMAND_CONFIG_LOADER_H
#define OHOS_ABILITY_RUNTIME_SHELL_COMMAND_CONFIG_LOADER_H

#include <mutex>
#include <set>

namespace OHOS {
namespace AAFwk {
class ShellCommandConfigLoder final {
public:
    ShellCommandConfigLoder() = default;
    ~ShellCommandConfigLoder() = default;

    /**
     * @brief Read configuration file.
     *
     * @param filePath Configuration directory.
     * @return true Read successfully.
     * @return false Read failed.
     */
    bool ReadConfig(const std::string &filePath);

    /**
     * @brief Read the configuration file only once,
     * true indicates that the config has been read.
     * Otherwise, it has not been read.
     */
    static bool configState_;

    static std::set<std::string> commands_;
private:
    std::mutex mtxRead_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif