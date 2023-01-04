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

#include "shell_command_config_loader.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "hilog_wrapper.h"

using json = nlohmann::json;
namespace OHOS {
namespace AAFwk {
namespace {
    constexpr const char* AA_TOOL_COMMAND_LIST = "command_list";
    constexpr static int  COMMANDS_MAX_SIZE = 100;
}

bool ShellCommandConfigLoder::configState_ = false;
std::set<std::string> ShellCommandConfigLoder::commands_ = {};

bool ShellCommandConfigLoder::ReadConfig(const std::string &filePath)
{
    HILOG_INFO("%{public}s", __func__);
    if (configState_) {
        HILOG_INFO("config has been read");
        return true;
    }

    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("read aa config error");
        return false;
    }

    json aaJson;
    inFile >> aaJson;
    inFile.close();
    if (aaJson.is_discarded()) {
        HILOG_INFO("json discarded error");
        return false;
    }

    if (aaJson.is_null() || aaJson.empty()) {
        HILOG_INFO("invalid jsonObj");
        return false;
    }

    if (!aaJson.contains(AA_TOOL_COMMAND_LIST)) {
        HILOG_INFO("json config not contains the key");
        return false;
    }

    if (aaJson[AA_TOOL_COMMAND_LIST].is_null() || !aaJson[AA_TOOL_COMMAND_LIST].is_array()
        || aaJson[AA_TOOL_COMMAND_LIST].empty()) {
        HILOG_INFO("invalid command obj size");
        return false;
    }

    if (aaJson[AA_TOOL_COMMAND_LIST].size() > COMMANDS_MAX_SIZE) {
        HILOG_INFO("command obj size overflow");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mtxRead_);
    for (int i = 0; i < aaJson[AA_TOOL_COMMAND_LIST].size(); i++) {
        if (aaJson[AA_TOOL_COMMAND_LIST][i].is_null() || !aaJson[AA_TOOL_COMMAND_LIST][i].is_string()) {
            continue;
        }
        commands_.emplace(aaJson[AA_TOOL_COMMAND_LIST][i]);
    }

    aaJson.clear();
    HILOG_INFO("read config success");
    configState_ = true;
    return true;
}

}  // namespace AAFwk
}  // namespace OHOS