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

    if (argString.compare("-a") == 0 || argString.compare("--all") == 0) {
        result.first = true;
        result.second = KEY_DUMP_ALL;
    } else if (argString.compare("-l") == 0 || argString.compare("--stack-list") == 0) {
        result.first = true;
        result.second = KEY_DUMP_STACK_LIST;
    } else if (argString.compare("-s") == 0 || argString.compare("--stack") == 0) {
        result.first = true;
        result.second = KEY_DUMP_STACK;
    } else if (argString.compare("-m") == 0 || argString.compare("--mission") == 0) {
        result.first = true;
        result.second = KEY_DUMP_MISSION;
    } else if (argString.compare("-t") == 0 || argString.compare("--top") == 0) {
        result.first = true;
        result.second = KEY_DUMP_TOP_ABILITY;
    } else if (argString.compare("-w") == 0 || argString.compare("--waiting-queue") == 0) {
        result.first = true;
        result.second = KEY_DUMP_WAIT_QUEUE;
    } else if (argString.compare("-e") == 0 || argString.compare("--serv") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SERVICE;
    } else if (argString.compare("-d") == 0 || argString.compare("--data") == 0) {
        result.first = true;
        result.second = KEY_DUMP_DATA;
    } else if (argString.compare("-f") == 0 || argString.compare("-focus") == 0) {
        result.first = true;
        result.second = KEY_DUMP_FOCUS_ABILITY;
    }
    return result;
}

std::pair<bool, DumpUtils::DumpKey> DumpUtils::DumpMapTwo(std::string argString)
{
    std::pair<bool, DumpUtils::DumpKey> result(false, KEY_DUMP_ALL);

    if (argString.compare("-z") == 0 || argString.compare("--win-mode") == 0) {
        result.first = true;
        result.second = KEY_DUMP_WINDOW_MODE;
    } else if (argString.compare("-L") == 0 || argString.compare("--mission-list") == 0) {
        result.first = true;
        result.second = KEY_DUMP_MISSION_LIST;
    } else if (argString.compare("-S") == 0 || argString.compare("--mission-infos") == 0) {
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

    if (argString.compare("-a") == 0 || argString.compare("--all") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_ALL;
    } else if (argString.compare("-l") == 0 || argString.compare("--mission-list") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_MISSION_LIST;
    } else if (argString.compare("-i") == 0 || argString.compare("--ability") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_ABILITY;
    } else if (argString.compare("-e") == 0 || argString.compare("--extension") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_SERVICE;
    } else if (argString.compare("-p") == 0 || argString.compare("--pending") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_PENDING;
    } else if (argString.compare("-r") == 0 || argString.compare("--process") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_PROCESS;
    } else if (argString.compare("-d") == 0 || argString.compare("--data") == 0) {
        result.first = true;
        result.second = KEY_DUMP_SYS_DATA;
    }
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
