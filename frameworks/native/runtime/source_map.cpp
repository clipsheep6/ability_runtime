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

#include "source_map.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "extractor.h"
#include "hilog_wrapper.h"

using namespace OHOS::AbilityBase;
using Extractor = OHOS::AbilityBase::Extractor;

namespace OHOS {
namespace AbilityRuntime {

constexpr char SOURCES[] = "sources";
constexpr char NAMES[] = "names";
constexpr char MAPPINGS[] = "mappings";
constexpr char FILE[] = "file";
constexpr char SOURCE_CONTENT[] = "sourceContent";
constexpr char SOURCE_ROOT[] = "sourceRoot";
constexpr char DELIMITER_COMMA = ',';
constexpr char DELIMITER_SEMICOLON = ';';
constexpr char DOUBLE_SLASH = '\\';
constexpr char WEBPACK[] = "webpack:///";
const std::string REALPATH_FLAG = "/temprary/";
const std::string MEGER_SOURCE_MAP_PATH = "ets/sourceMaps.map";
const std::string NOT_FOUNDMAP = "Cannot get SourceMap info, dump raw stack:\n";
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;
constexpr int32_t ANS_MAP_SIZE = 5;
constexpr int32_t NUM_TWENTY = 20;
constexpr int32_t NUM_TWENTYSIX = 26;
constexpr int32_t DIGIT_NUM = 64;

bool ModSourceMap::ReadSourceMapData(const std::string& hapPath, std::string& content)
{
    if (hapPath.empty()) {
        HILOG_ERROR("hapPath is empty");
        return false;
    }
    bool newCreate = false;
    std::shared_ptr<Extractor> extractor = ExtractorUtil::GetExtractor(
        ExtractorUtil::GetLoadFilePath(hapPath), newCreate);
    if (extractor == nullptr) {
        HILOG_ERROR("hapPath %{public}s GetExtractor failed", hapPath.c_str());
        return false;
    }
    std::unique_ptr<uint8_t[]> dataPtr = nullptr;
    size_t len = 0;
    if (!extractor->ExtractToBufByName(MEGER_SOURCE_MAP_PATH, dataPtr, len)) {
        HILOG_ERROR("get mergeSourceMapData fileBuffer failed");
        return false;
    }
    content = reinterpret_cast<char *>(dataPtr.get());
    return true;
}

MappingInfo ModSourceMap::Find(int32_t row, int32_t col, const SourceMapData& targetMap, const std::string& key)
{
    if (row < 1 || col < 1) {
        HILOG_ERROR("the input pos is wrong");
        return MappingInfo {};
    }
    row--;
    col--;
    // binary search
    int32_t left = 0;
    int32_t right = static_cast<int32_t>(targetMap.afterPos_.size()) - 1;
    int32_t res = 0;
    if (row > targetMap.afterPos_[targetMap.afterPos_.size() - 1].afterRow) {
        return MappingInfo { row + 1, col + 1, targetMap.files_[0] };
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
    std::string sources = key;
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

void ModSourceMap::ExtractKeyInfo(const std::string& sourceMap, std::vector<std::string>& sourceKeyInfo)
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

void ModSourceMap::GetPosInfo(const std::string& temp, int32_t start, std::string& line, std::string& column)
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

int32_t ModSourceMap::StringToInt(const std::string& value)
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

std::string ModSourceMap::GetRelativePath(const std::string& sources)
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

void ModSourceMap::Init(const std::string& sourceMap, SourceMapData& curMapData)
{
    std::vector<std::string> sourceKeyInfo;
    std::string mark = "";

    ExtractKeyInfo(sourceMap, sourceKeyInfo);

    // first: find the key info and record the temp key info
    // second: add the detail into the keyinfo
    for (auto keyInfo : sourceKeyInfo) {
        if (keyInfo == SOURCES || keyInfo == NAMES || keyInfo == MAPPINGS || keyInfo == FILE ||
            keyInfo == SOURCE_CONTENT ||  keyInfo == SOURCE_ROOT) {
            // record the temp key info
            mark = keyInfo;
        } else if (mark == SOURCES) {
            curMapData.sources_.push_back(keyInfo);
        } else if (mark == NAMES) {
            curMapData.names_.push_back(keyInfo);
        } else if (mark == MAPPINGS) {
            curMapData.mappings_.push_back(keyInfo);
        } else if (mark == FILE) {
            curMapData.files_.push_back(keyInfo);
        } else {
            continue;
        }
    }

    // transform to vector for mapping easily
    curMapData.mappings_ = HandleMappings(curMapData.mappings_[0]);

    // the first bit: the column after transferring.
    // the second bit: the source file.
    // the third bit: the row before transferring.
    // the fourth bit: the column before transferring.
    // the fifth bit: the variable name.
    for (const auto& mapping : curMapData.mappings_) {
        if (mapping == ";") {
            // plus a line for each semicolon
            curMapData.nowPos_.afterRow++,
            curMapData.nowPos_.afterColumn = 0;
            continue;
        }
        // decode each mapping ";QAABC"
        std::vector<int32_t> ans;
        if (!VlqRevCode(mapping, ans)) {
            HILOG_ERROR("decode code fail");
            return;
        }
        if (ans.size() == 0) {
            HILOG_ERROR("decode sourcemap fail, mapping: %{public}s", mapping.c_str());
            break;
        }
        if (ans.size() == 1) {
            curMapData.nowPos_.afterColumn += ans[0];
            continue;
        }
        // after decode, assgin each value to the position
        curMapData.nowPos_.afterColumn += ans[0];
        curMapData.nowPos_.sourcesVal += ans[1];
        curMapData.nowPos_.beforeRow += ans[INDEX_TWO];
        curMapData.nowPos_.beforeColumn += ans[INDEX_THREE];
        if (ans.size() == ANS_MAP_SIZE) {
            curMapData.nowPos_.namesVal += ans[INDEX_FOUR];
        }
        curMapData.afterPos_.push_back({
            curMapData.nowPos_.beforeRow,
            curMapData.nowPos_.beforeColumn,
            curMapData.nowPos_.afterRow,
            curMapData.nowPos_.afterColumn,
            curMapData.nowPos_.sourcesVal,
            curMapData.nowPos_.namesVal
        });
    }
    curMapData.mappings_.clear();
    curMapData.mappings_.shrink_to_fit();
    sourceKeyInfo.clear();
    sourceKeyInfo.shrink_to_fit();
};

std::vector<std::string> ModSourceMap::HandleMappings(const std::string& mapping)
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

uint32_t ModSourceMap::Base64CharToInt(char charCode)
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

bool ModSourceMap::VlqRevCode(const std::string& vStr, std::vector<int32_t>& ans)
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

std::string ModSourceMap::TranslateBySourceMap(const std::string& stackStr, ModSourceMap& bindSourceMaps,
    const std::string& hapPath)
{
    const std::string closeBrace = ")";
    const std::string openBrace = "(";
    std::string ans = "";
    std::string tempStack = stackStr;

    // find per line of stack
    std::vector<std::string> res;
    std::string tempStr = "";
    for (uint32_t i = 0; i < tempStack.length(); i++) {
        if (tempStack[i] == '\n') {
            res.push_back(tempStr);
            tempStr = "";
        } else {
            tempStr += tempStack[i];
        }
    }
    if (!tempStr.empty()) {
        res.push_back(tempStr);
    }

    // collect error info first
    bool needGetErrorPos = false;
    uint32_t i = 0;
    std::string codeStart = "SourceCode (";
    std::string sourceCode = "";
    if (res.size() >= 1) {
        std::string fristLine = res[0];
        uint32_t codeStartLen = codeStart.length();
        if (fristLine.substr(0, codeStartLen).compare(codeStart) == 0) {
            sourceCode = fristLine.substr(codeStartLen, fristLine.length() - codeStartLen - 1);
            i = 1;  // 1 means Convert from the second line
            needGetErrorPos = true;
        }
    }
    std::string curSourceMap;
    if (!ReadSourceMapData(hapPath, curSourceMap)) {
        HILOG_ERROR("ReadSourceMapData fail");
        return stackStr;
    }
    std::size_t s = 0;
    std::size_t j = 0;
    std::string value;
    std::string key;
    std::map<std::string, std::string> MapData;
    while ((s = curSourceMap.find(": {", j)) != std::string::npos) {
        j = curSourceMap.find("},", s);
        uint32_t q = s;
        uint32_t jj = j;
        value = curSourceMap.substr(q + 1, jj - q + INDEX_TWO);
        uint32_t sources = value.find("\"sources\": [");
        uint32_t names = value.find("],");
        key = value.substr(sources + NUM_TWENTY, names - sources - NUM_TWENTYSIX);
        MapData.insert(std::pair<std::string, std::string>(key, value));
    }

    for (; i < res.size(); i++) {
        std::string temp = res[i];
        uint32_t start = temp.find("(");
        uint32_t end  = temp.find(":");
        std::string key = temp.substr(start + 1, end - start - 1);
        int32_t closeBracePos = static_cast<int32_t>(temp.find(closeBrace));
        int32_t openBracePos = static_cast<int32_t>(temp.find(openBrace));

        std::string line = "";
        std::string column = "";
        GetPosInfo(temp, closeBracePos, line, column);
        if (needGetErrorPos) {
            needGetErrorPos = false;
        }
        if (line.empty() || column.empty()) {
            break;
        }
        static SourceMapData curMapData;
        if (!bindSourceMaps.isStageModel) {
            if (i == 1) {   // The non module scenario initializes curmapdata only at the first traversal
                if (!bindSourceMaps.nonModularMap_) {
                    return NOT_FOUNDMAP + stackStr;
                }
                curMapData = *bindSourceMaps.nonModularMap_;
            }
        } else {
            auto iter = MapData.find(key);
            if (iter != MapData.end()) {
                Init(iter->second, curMapData);
            } else {
                ans += NOT_FOUNDMAP + temp + "\n";
                continue;
            }
        }
        const std::string sourceInfo = GetSourceInfo(line, column, curMapData, key);
        if (sourceInfo.empty()) {
            break;
        }
        temp.replace(openBracePos, closeBracePos - openBracePos + 1, sourceInfo);
        ans += temp + "\n";
    }

    if (ans.empty()) {
        return tempStack;
    }
    return ans;
}

std::string ModSourceMap::GetSourceInfo(const std::string& line, const std::string& column,
    const SourceMapData& targetMap, const std::string& key)
{
    int32_t offSet = 0;
    std::string sourceInfo;
    MappingInfo mapInfo;
#if defined(WINDOWS_PLATFORM) || defined(MAC_PLATFORM)
        mapInfo = Find(StringToInt(line) - offSet + OFFSET_PREVIEW, StringToInt(column), targetMap, key);
#else
        mapInfo = Find(StringToInt(line) - offSet, StringToInt(column), targetMap, key);
#endif
    if (mapInfo.row == 0 || mapInfo.col == 0) {
        return "";
    }
    std::string sources = GetRelativePath(mapInfo.sources);
    sourceInfo = "(" + sources + ":" + std::to_string(mapInfo.row) + ":" + std::to_string(mapInfo.col) + ")";
    return sourceInfo;
}

void ModSourceMap::NonModularLoadSourceMap(ModSourceMap& targetMaps, const std::string& targetMap)
{
    if (!targetMaps.nonModularMap_) {
        targetMaps.nonModularMap_ = std::make_shared<SourceMapData>();
    }
    Init(targetMap, *targetMaps.nonModularMap_);
}

std::string ModSourceMap::GetOriginalNames(std::shared_ptr<SourceMapData> targetMapData,
    const std::string& sourceCode, uint32_t& errorPos)
{
    if (sourceCode.empty() || sourceCode.find("SourceCode:\n") == std::string::npos) {
        return sourceCode;
    }
    std::vector<std::string> names = targetMapData->names_;
    if (names.size() % INDEX_TWO != 0) {
        HILOG_ERROR("Names in sourcemap is wrong.");
        return sourceCode;
    }

    std::string jsCode = sourceCode;
    int32_t posDiff = 0;
    for (uint32_t i = 0; i < names.size(); i += INDEX_TWO) {
        auto found = jsCode.find(names[i]);
        while (found != std::string::npos) {
            // names_[i + 1] is the original name of names_[i]
            jsCode.replace(found, names[i].length(), names[i + 1]);
            if (static_cast<uint32_t>(found) < errorPos) {
                // sum the errorPos differences to adjust position of ^
                posDiff += static_cast<int32_t>(names[i + 1].length()) - static_cast<int32_t>(names[i].length());
            }
            // In case there are other variable names not replaced.
            // example:var e = process.a.b + _ohos_process_1.a.b;
            found = jsCode.find(names[i], found + names[i + 1].length());
        }
    }
    auto lineBreakPos = jsCode.rfind('\n', jsCode.length() - 2);
    if (lineBreakPos == std::string::npos) {
        HILOG_WARN("There is something wrong in source code of summaryBody.");
        return jsCode;
    }
    // adjust position of ^ in dump file
    if (posDiff < 0) {
        int32_t flagPos = static_cast<int32_t>(lineBreakPos) + static_cast<int32_t>(errorPos);
        if (lineBreakPos > 0 && errorPos > 0 && flagPos < 0) {
            HILOG_WARN("Add overflow of sourceCode.");
            return jsCode;
        }
        if (flagPos < static_cast<int32_t>(jsCode.length()) && jsCode[flagPos] == '^' && flagPos + posDiff - 1 > 0) {
            jsCode.erase(flagPos + posDiff - 1, -posDiff);
        }
    } else if (posDiff > 0) {
        if (lineBreakPos + 1 < jsCode.length() - 1) {
            jsCode.insert(lineBreakPos + 1, posDiff, ' ');
        }
    }
    return jsCode;
}

ErrorPos ModSourceMap::GetErrorPos(const std::string& rawStack)
{
    size_t findLineEnd = rawStack.find("\n");
    if (findLineEnd == std::string::npos) {
        return std::make_pair(0, 0);
    }
    uint32_t lineEnd = findLineEnd - 1;
    if (lineEnd < 1 || rawStack[lineEnd - 1] == '?') {
        return std::make_pair(0, 0);
    }

    uint32_t secondPos = rawStack.rfind(':', lineEnd);
    uint32_t fristPos = rawStack.rfind(':', secondPos - 1);

    std::string lineStr = rawStack.substr(fristPos + 1, secondPos - 1 - fristPos);
    std::string columnStr = rawStack.substr(secondPos + 1, lineEnd - 1 - secondPos);

    return std::make_pair(StringToInt(lineStr), StringToInt(columnStr));
}
}   // namespace AbilityRuntime
}   // namespace OHOS
