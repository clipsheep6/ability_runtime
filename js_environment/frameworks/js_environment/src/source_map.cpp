/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "source_map.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace JsEnv {
namespace {
constexpr char DELIMITER_COMMA = ',';
constexpr char DELIMITER_SEMICOLON = ';';
constexpr char DOUBLE_SLASH = '\\';
constexpr char WEBPACK[] = "webpack:///";
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;
constexpr int32_t ANS_MAP_SIZE = 5;
constexpr int32_t DIGIT_NUM = 64;
const std::string MEGER_SOURCE_MAP_PATH = "ets/sourceMaps.map";
const std::string FLAG_SOURCES = "    \"sources\":";
const std::string FLAG_MAPPINGS = "    \"mappings\": \"";
const std::string FLAG_END = "  }";
static constexpr size_t FLAG_MAPPINGS_LEN = 17;
static constexpr size_t REAL_SOURCE_SIZE = 7;
static constexpr size_t REAL_URL_INDEX = 3;
static constexpr size_t REAL_SOURCE_INDEX = 7;
} // namespace
ReadSourceMapCallback SourceMap::readSourceMapFunc_ = nullptr;
GetHapPathCallback SourceMap::getHapPathFunc_ = nullptr;
std::mutex SourceMap::sourceMapMutex_;

int32_t StringToInt(const std::string& value)
{
    errno = 0;
    char* pEnd = nullptr;
    int64_t result = std::strtol(value.c_str(), &pEnd, 10);
    if (pEnd == value.c_str() || (result < INT_MIN || result > INT_MAX) || errno == ERANGE) {
        return 0;
    } else {
        return result;
    }
}

uint32_t Base64CharToInt(char charCode)
{
    if ('A' <= charCode && charCode <= 'Z') {
        // 0 - 25: ABCDEFGHIJKLMNOPQRSTUVWXYZ
        return charCode - 'A';
    } else if ('a' <= charCode && charCode <= 'z') {
        // 26 - 51: abcdefghijklmnopqrstuvwxyz
        return charCode - 'a' + 26;
    } else if ('0' <= charCode && charCode <= '9') {
        // 52 - 61: 0123456789
        return charCode - '0' + 52;
    } else if (charCode == '+') {
        // 62: +
        return 62;
    } else if (charCode == '/') {
        // 63: /
        return 63;
    }
    return DIGIT_NUM;
};

bool StringStartWith(const std::string& str, const std::string& startStr)
{
    size_t startStrLen = startStr.length();
    return ((str.length() >= startStrLen) && (str.compare(0, startStrLen, startStr) == 0));
}

void SourceMap::Init(bool isModular, const std::string& hapPath)
{
    isModular_ = isModular;
    hapPath_ = hapPath;
    if (isModular_) {
        std::string sourceMapData;
        ReadSourceMapData(hapPath_, MEGER_SOURCE_MAP_PATH, sourceMapData);
        SplitSourceMap(sourceMapData);
    } else {
        if (!nonModularMap_) {
            nonModularMap_ = std::make_shared<SourceMapData>();
        }
    }
}

std::string SourceMap::TranslateBySourceMap(const std::string& stackStr)
{
    std::string closeBrace = ")";
    std::string openBrace = "(";
    std::string ans = "";

    // find per line of stack
    std::vector<std::string> res;
    ExtractStackInfo(stackStr, res);

    // collect error info first
    uint32_t i = 0;
    std::string codeStart = "SourceCode (";
    std::string sourceCode = "";
    if (!res.empty()) {
        std::string fristLine = res[0];
        uint32_t codeStartLen = codeStart.length();
        if (fristLine.substr(0, codeStartLen).compare(codeStart) == 0) {
            sourceCode = fristLine.substr(codeStartLen, fristLine.length() - codeStartLen - 1);
            i = 1;  // 1 means Convert from the second line
        }
    }

    // collect error info first
    for (; i < res.size(); i++) {
        std::string temp = res[i];
        size_t start;
        size_t end;
        if (isModular_) {
            start = temp.find(openBrace);
            end = temp.find(":");
        } else {
            start = temp.find("/ets/");
            end = temp.rfind("_.js");
        }
        if (end <= start) {
            continue;
        }
        std::string key = temp.substr(start + 1, end - start - 1);
        auto closeBracePos = static_cast<int32_t>(temp.find(closeBrace));
        auto openBracePos = static_cast<int32_t>(temp.find(openBrace));
        std::string line;
        std::string column;
        GetPosInfo(temp, closeBracePos, line, column);
        if (line.empty() || column.empty()) {
            TAG_LOGW(AAFwkTag::JSENV, "the stack without line info");
            break;
        }
        std::string sourceInfo;
        if (isModular_) {
            auto iter = sourceMaps_.find(key);
            if (iter != sourceMaps_.end()) {
                sourceInfo = GetSourceInfo(line, column, *(iter->second));
            } else if (key.rfind(".js") != std::string::npos) {
                ans = ans + temp + "\n";
                continue;
            }
        } else {
            std::string url = key + ".js.map";
            std::string curSourceMap;
            if (!ReadSourceMapData(hapPath_, url, curSourceMap)) {
                TAG_LOGW(AAFwkTag::JSENV, "ReadSourceMapData fail");
                continue;
            }
            ExtractSourceMapData(curSourceMap, nonModularMap_);
            sourceInfo = GetSourceInfo(line, column, *nonModularMap_);
        }
        if (sourceInfo.empty()) {
            continue;
        }
        temp.replace(openBracePos, closeBracePos - openBracePos + 1, sourceInfo);
        replace(temp.begin(), temp.end(), '\\', '/');
        ans = ans + temp + "\n";
    }
    if (ans.empty()) {
        return (NOT_FOUNDMAP + stackStr);
    }
    return ans;
}

void SourceMap::SplitSourceMap(const std::string& sourceMapData)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    if (!isModular_) {
        if (!nonModularMap_) {
            nonModularMap_ = std::make_shared<SourceMapData>();
        }
        return ExtractSourceMapData(sourceMapData, nonModularMap_);
    }

    std::stringstream ss(sourceMapData);
    std::string tmp;
    std::string url;

    std::getline(ss, tmp);
    bool isUrl = true;
    while (std::getline(ss, tmp)) {
        if (isUrl && tmp.size() > REAL_SOURCE_SIZE) {
            url = tmp.substr(REAL_URL_INDEX, tmp.size() - REAL_SOURCE_SIZE);
            isUrl = false;
            continue;
        }
        if (StringStartWith(tmp.c_str(), FLAG_SOURCES)) {
            std::getline(ss, tmp);
            sources_.emplace(url, tmp);
            continue;
        }
        if (StringStartWith(tmp.c_str(), FLAG_MAPPINGS)) {
            mappings_.emplace(url, tmp);
            continue;
        }
        if (StringStartWith(tmp.c_str(), FLAG_END)) {
            isUrl = true;
        }
    }
    for (auto it = sources_.begin(); it != sources_.end(); ++it) {
        std::string mappings = mappings_[it->first];
        if (mappings.size() < FLAG_MAPPINGS_LEN + 1) {
            TAG_LOGE(AAFwkTag::JSENV, "Translate failed, url: %{public}s", it->first.c_str());
            continue;
        }
        std::shared_ptr<SourceMapData> modularMap = std::make_shared<SourceMapData>();
        ExtractSourceMapData(mappings.substr(FLAG_MAPPINGS_LEN, mappings.size() - FLAG_MAPPINGS_LEN - 1), modularMap);
        if (modularMap == nullptr) {
            TAG_LOGE(AAFwkTag::JSENV, "Extract mappings failed, url: %{public}s", it->first.c_str());
            continue;
        }
        modularMap->sources_.push_back(it->second);
        sourceMaps_[it->first] = modularMap;
    }
    mappings_.clear();
    sources_.clear();
}

void SourceMap::ExtractStackInfo(const std::string& stackStr, std::vector<std::string>& res)
{
    std::stringstream ss(stackStr);
    std::string tempStr;
    while (std::getline(ss, tempStr)) {
        res.push_back(tempStr);
    }
}

void SourceMap::ExtractSourceMapData(const std::string& allmappings, std::shared_ptr<SourceMapData>& curMapData)
{
    curMapData->mappings_ = HandleMappings(allmappings);
    // the first bit: the column after transferring.
    // the second bit: the source file.
    // the third bit: the row before transferring.
    // the fourth bit: the column before transferring.
    // the fifth bit: the variable name.
    for (const auto& mapping : curMapData->mappings_) {
        if (mapping == ";") {
            // plus a line for each semicolon
            curMapData->nowPos_.afterRow++,
            curMapData->nowPos_.afterColumn = 0;
            continue;
        }
        std::vector<int32_t> ans;

        if (!VlqRevCode(mapping, ans)) {
            return;
        }
        if (ans.empty()) {
            TAG_LOGE(AAFwkTag::JSENV, "decode sourcemap fail, mapping: %{public}s", mapping.c_str());
            break;
        }
        if (ans.size() == 1) {
            curMapData->nowPos_.afterColumn += ans[0];
            continue;
        }
        // after decode, assgin each value to the position
        curMapData->nowPos_.afterColumn += ans[0];
        curMapData->nowPos_.sourcesVal += ans[INDEX_ONE];
        curMapData->nowPos_.beforeRow += ans[INDEX_TWO];
        curMapData->nowPos_.beforeColumn += ans[INDEX_THREE];
        if (ans.size() == ANS_MAP_SIZE) {
            curMapData->nowPos_.namesVal += ans[INDEX_FOUR];
        }
        curMapData->afterPos_.push_back({
            curMapData->nowPos_.beforeRow,
            curMapData->nowPos_.beforeColumn,
            curMapData->nowPos_.afterRow,
            curMapData->nowPos_.afterColumn,
            curMapData->nowPos_.sourcesVal,
            curMapData->nowPos_.namesVal
        });
    }
    curMapData->mappings_.clear();
    curMapData->mappings_.shrink_to_fit();
}

MappingInfo SourceMap::Find(int32_t row, int32_t col, const SourceMapData& targetMap)
{
    if (row < 1 || col < 1 || targetMap.afterPos_.empty() || targetMap.sources_[0].empty()) {
        return MappingInfo {0, 0, ""};
    }
    row--;
    col--;
    // binary search
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(targetMap.afterPos_.size()) - 1;
    int32_t res = 0;
    std::string sources = targetMap.sources_[0].substr(REAL_SOURCE_INDEX,
                                                       targetMap.sources_[0].size() - REAL_SOURCE_SIZE - 1);
    if (row > targetMap.afterPos_[targetMap.afterPos_.size() - 1].afterRow) {
        return MappingInfo { row + 1, col + 1, sources };
    }
    while (right - left >= 0) {
        int32_t mid = (right + left) / 2;
        if ((targetMap.afterPos_[mid].afterRow == row && targetMap.afterPos_[mid].afterColumn > col) ||
             targetMap.afterPos_[mid].afterRow > row) {
            right = mid - 1;
        } else {
            res = mid;
            left = mid + 1;
        }
    }
    auto pos = sources.find(WEBPACK);
    if (pos != std::string::npos) {
        sources.replace(pos, sizeof(WEBPACK) - 1, "");
    }

    return MappingInfo {
        .row = targetMap.afterPos_[res].beforeRow + 1,
        .col = targetMap.afterPos_[res].beforeColumn + 1,
        .sources = sources,
    };
}

void SourceMap::ExtractKeyInfo(const std::string& sourceMap, std::vector<std::string>& sourceKeyInfo)
{
    uint32_t cnt = 0;
    std::string tempStr;
    for (uint32_t i = 0; i < sourceMap.size(); i++) {
        // reslove json file
        if (sourceMap[i] == DOUBLE_SLASH) {
            i++;
            tempStr += sourceMap[i];
            continue;
        }
        // cnt is used to represent a pair of double quotation marks: ""
        if (sourceMap[i] == '"') {
            cnt++;
        }
        if (cnt == INDEX_TWO) {
            sourceKeyInfo.push_back(tempStr);
            tempStr = "";
            cnt = 0;
        } else if (cnt == 1) {
            if (sourceMap[i] != '"') {
                tempStr += sourceMap[i];
            }
        }
    }
}

void SourceMap::GetPosInfo(const std::string& temp, int32_t start, std::string& line, std::string& column)
{
    // 0 for colum, 1 for row
    int32_t flag = 0;
    // find line, column
    for (int32_t i = start - 1; i > 0; i--) {
        if (temp[i] == ':') {
            flag += 1;
            continue;
        }
        if (flag == 0) {
            column = temp[i] + column;
        } else if (flag == 1) {
            line = temp[i] + line;
        } else {
            break;
        }
    }
}

std::string SourceMap::GetRelativePath(const std::string& sources)
{
    std::string temp = sources;
    std::size_t splitPos = std::string::npos;
    const static int pathLevel = 3;
    int i = 0;
    while (i < pathLevel) {
        splitPos = temp.find_last_of("/\\");
        if (splitPos != std::string::npos) {
            temp = temp.substr(0, splitPos - 1);
        } else {
            break;
        }
        i++;
    }
    if (i == pathLevel) {
        return sources.substr(splitPos);
    }
    return sources;
}

std::vector<std::string> SourceMap::HandleMappings(const std::string& mapping)
{
    std::vector<std::string> keyInfo;
    std::string tempStr;
    for (uint32_t i = 0; i < mapping.size(); i++) {
        if (mapping[i] == DELIMITER_COMMA) {
            keyInfo.push_back(tempStr);
            tempStr = "";
        } else if (mapping[i] == DELIMITER_SEMICOLON) {
            if (tempStr != "") {
                keyInfo.push_back(tempStr);
            }
            tempStr = "";
            keyInfo.push_back(";");
        } else {
            tempStr += mapping[i];
        }
    }
    if (tempStr != "") {
        keyInfo.push_back(tempStr);
    }
    return keyInfo;
};

bool SourceMap::VlqRevCode(const std::string& vStr, std::vector<int32_t>& ans)
{
    const int32_t VLQ_BASE_SHIFT = 5;
    // binary: 100000
    uint32_t VLQ_BASE = 1 << VLQ_BASE_SHIFT;
    // binary: 011111
    uint32_t VLQ_BASE_MASK = VLQ_BASE - 1;
    // binary: 100000
    uint32_t VLQ_CONTINUATION_BIT = VLQ_BASE;
    uint32_t result = 0;
    uint32_t shift = 0;
    bool continuation = 0;
    for (uint32_t i = 0; i < vStr.size(); i++) {
        uint32_t digit = Base64CharToInt(vStr[i]);
        if (digit == DIGIT_NUM) {
            return false;
        }
        continuation = digit & VLQ_CONTINUATION_BIT;
        digit &= VLQ_BASE_MASK;
        result += digit << shift;
        if (continuation) {
            shift += VLQ_BASE_SHIFT;
        } else {
            bool isNegate = result & 1;
            result >>= 1;
            ans.push_back(isNegate ? -result : result);
            result = 0;
            shift = 0;
        }
    }
    if (continuation) {
        return false;
    }
    return true;
};

std::string SourceMap::GetSourceInfo(const std::string& line, const std::string& column,
    const SourceMapData& targetMap)
{
    int32_t offSet = 0;
    std::string sourceInfo;
    MappingInfo mapInfo;
#if defined(WINDOWS_PLATFORM) || defined(MAC_PLATFORM)
        mapInfo = Find(StringToInt(line) - offSet + OFFSET_PREVIEW, StringToInt(column), targetMap);
#else
        mapInfo = Find(StringToInt(line) - offSet, StringToInt(column), targetMap);
#endif
    if (mapInfo.row == 0 || mapInfo.col == 0) {
        return "";
    }
    std::string sources = isModular_ ? mapInfo.sources : GetRelativePath(mapInfo.sources);
    sourceInfo = "(" + sources + ":" + std::to_string(mapInfo.row) + ":" + std::to_string(mapInfo.col) + ")";
    return sourceInfo;
}

ErrorPos SourceMap::GetErrorPos(const std::string& rawStack)
{
    size_t findLineEnd = rawStack.find("\n");
    if (findLineEnd == std::string::npos) {
        return std::make_pair(0, 0);
    }
    int32_t lineEnd = (int32_t)findLineEnd - 1;
    if (lineEnd < 1 || rawStack[lineEnd - 1] == '?') {
        return std::make_pair(0, 0);
    }

    uint32_t secondPos = rawStack.rfind(':', lineEnd);
    uint32_t fristPos = rawStack.rfind(':', secondPos - 1);

    std::string lineStr = rawStack.substr(fristPos + 1, secondPos - 1 - fristPos);
    std::string columnStr = rawStack.substr(secondPos + 1, lineEnd - 1 - secondPos);

    return std::make_pair(StringToInt(lineStr), StringToInt(columnStr));
}

void SourceMap::RegisterReadSourceMapCallback(ReadSourceMapCallback readFunc)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    readSourceMapFunc_ = readFunc;
}

bool SourceMap::ReadSourceMapData(const std::string& hapPath, const std::string& sourceMapPath, std::string& content)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    if (readSourceMapFunc_) {
        return readSourceMapFunc_(hapPath, sourceMapPath, content);
    }
    return false;
}

bool SourceMap::TranslateUrlPositionBySourceMap(std::string& url, int& line, int& column)
{
    if (isModular_) {
        auto iter = sourceMaps_.find(url);
        if (iter != sourceMaps_.end()) {
            return GetLineAndColumnNumbers(line, column, *(iter->second), url);
        }
        TAG_LOGE(AAFwkTag::JSENV, "stageMode sourceMaps find fail");
        return false;
    }
    return false;
}

bool SourceMap::GetLineAndColumnNumbers(int& line, int& column, SourceMapData& targetMap, std::string& url)
{
    int32_t offSet = 0;
    MappingInfo mapInfo;
#if defined(WINDOWS_PLATFORM) || defined(MAC_PLATFORM)
        mapInfo = Find(line - offSet + OFFSET_PREVIEW, column, targetMap);
#else
        mapInfo = Find(line - offSet, column, targetMap);
#endif
    if (mapInfo.row == 0 || mapInfo.col == 0) {
        return false;
    } else {
        line = mapInfo.row;
        column = mapInfo.col;
        url = mapInfo.sources;
        return true;
    }
}

void SourceMap::RegisterGetHapPathCallback(GetHapPathCallback getFunc)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    getHapPathFunc_ = getFunc;
}

void SourceMap::GetHapPath(const std::string &bundleName, std::vector<std::string> &hapList)
{
    std::lock_guard<std::mutex> lock(sourceMapMutex_);
    if (getHapPathFunc_) {
        getHapPathFunc_(bundleName, hapList);
    }
}
}   // namespace JsEnv
}   // namespace OHOS
