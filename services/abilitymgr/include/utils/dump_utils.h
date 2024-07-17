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

#ifndef OHOS_ABILITY_RUNTIME_DUMP_UTILS_H
#define OHOS_ABILITY_RUNTIME_DUMP_UTILS_H

#include <string>

namespace OHOS {
namespace AAFwk {
class DumpUtils final {
public:
    enum DumpKey {
        KEY_DUMP_ALL = 0,
        KEY_DUMP_STACK_LIST,
        KEY_DUMP_STACK,
        KEY_DUMP_MISSION,
        KEY_DUMP_TOP_ABILITY,
        KEY_DUMP_WAIT_QUEUE,
        KEY_DUMP_SERVICE,
        KEY_DUMP_DATA,
        KEY_DUMP_FOCUS_ABILITY,
        KEY_DUMP_WINDOW_MODE,
        KEY_DUMP_MISSION_LIST,
        KEY_DUMP_MISSION_INFOS,
    };

    enum DumpsysKey {
        KEY_DUMP_SYS_ALL = 0,
        KEY_DUMP_SYS_MISSION_LIST,
        KEY_DUMP_SYS_ABILITY,
        KEY_DUMP_SYS_SERVICE,
        KEY_DUMP_SYS_PENDING,
        KEY_DUMP_SYS_PROCESS,
        KEY_DUMP_SYS_DATA,
    };

    static std::pair<bool, DumpUtils::DumpKey> DumpMapOne(std::string argString);
    static std::pair<bool, DumpUtils::DumpKey> DumpMapTwo(std::string argString);
    static std::pair<bool, DumpUtils::DumpKey> DumpMap(std::string argString);
    static std::pair<bool, DumpUtils::DumpsysKey> DumpsysMap(std::string argString);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_DUMP_UTILS_H
