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

#include "utils/dump_utils.h"

namespace OHOS {
namespace AAFwk {
std::pair<bool, DumpUtils::DumpKey> DumpUtils::DumpMapOne(std::string argString)
{
    std::pair<bool, DumpUtils::DumpKey> result(false, KEY_DUMP_ALL);

    if (argString.compare("-a") || argString.compare("--all")) {
        result.first = true;
        result.second = KEY_DUMP_ALL;
    } else if (argString.compare("-l") || argString.compare("--stack-list")) {
        result.first = true;
        result.second = KEY_DUMP_STACK_LIST;
    } else if (argString.compare("-s") || argString.compare("--stack")) {
        result.first = true;
        result.second = KEY_DUMP_STACK;
    } else if (argString.compare("-m") || argString.compare("--mission")) {
        result.first = true;
        result.second = KEY_DUMP_MISSION;
    } else if (argString.compare("-t") || argString.compare("--top")) {
        result.first = true;
        result.second = KEY_DUMP_TOP_ABILITY;
    } else if (argString.compare("-w") || argString.compare("--waiting-queue")) {
        result.first = true;
        result.second = KEY_DUMP_WAIT_QUEUE;
    } else if (argString.compare("-e") || argString.compare("--serv")) {
        result.first = true;
        result.second = KEY_DUMP_SERVICE;
    } else if (argString.compare("-d") || argString.compare("--data")) {
        result.first = true;
        result.second = KEY_DUMP_DATA;
    } else if (argString.compare("-f") || argString.compare("-focus")) {
        result.first = true;
        result.second = KEY_DUMP_FOCUS_ABILITY;
    }
    return result;
}

std::pair<bool, DumpUtils::DumpKey> DumpUtils::DumpMapTwo(std::string argString)
{
    std::pair<bool, DumpUtils::DumpKey> result(false, KEY_DUMP_ALL);

    if (argString.compare("-z") || argString.compare("--win-mode")) {
        result.first = true;
        result.second = KEY_DUMP_WINDOW_MODE;
    } else if (argString.compare("-L") || argString.compare("--mission-list")) {
        result.first = true;
        result.second = KEY_DUMP_MISSION_LIST;
    } else if (argString.compare("-S") || argString.compare("--mission-infos")) {
        result.first = true;
        result.second = KEY_DUMP_MISSION_INFOS;
    }
    return result;
}

std::pair<bool, DumpUtils::DumpKey> DumpUtils::DumpMap(std::string argString)
{
    std::pair<bool, DumpUtils::DumpKey> result(false, KEY_DUMP_ALL);

    auto dumpMapOne = DumpMapOne(argString);
    if (dumpMapOne.first) {
        return dumpMapOne;
    }
    auto dumpMapTwo = DumpMapTwo(argString);
    if (dumpMapTwo.first) {
        return dumpMapTwo;
    }
    return result;
}

std::pair<bool, DumpUtils::DumpsysKey> DumpUtils::DumpsysMap(std::string argString)
{
    std::pair<bool, DumpUtils::DumpsysKey> result(false, KEY_DUMP_SYS_ALL);

    if (argString.compare("-a") || argString.compare("--all")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_ALL;
    } else if (argString.compare("-l") || argString.compare("--mission-list")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_MISSION_LIST;
    } else if (argString.compare("-i") || argString.compare("--ability")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_ABILITY;
    } else if (argString.compare("-e") || argString.compare("--extension")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_SERVICE;
    } else if (argString.compare("-p") || argString.compare("--pending")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_PENDING;
    } else if (argString.compare("-r") || argString.compare("--process")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_PROCESS;
    } else if (argString.compare("-d") || argString.compare("--data")) {
        result.first = true;
        result.second = KEY_DUMP_SYS_DATA;
    }
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
