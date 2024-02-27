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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_H
#define OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_H

#include <cstring>
#include <fstream>
#include <limits.h>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <thread>
#include <vector>

namespace OHOS {
namespace JsEnv {
using ErrorPos = std::pair<uint32_t, uint32_t>;
struct SourceMapInfo {
    int32_t beforeRow = 0;
    int32_t beforeColumn = 0;
    int32_t afterRow = 0;
    int32_t afterColumn = 0;
    int32_t sourcesVal = 0;
    int32_t namesVal = 0;
};

struct MappingInfo {
    int32_t row = 0;
    int32_t col = 0;
    std::string sources;
};

class SourceMapData final {
public:
    SourceMapData() = default;
    ~SourceMapData() = default;

    std::string url_;
    SourceMapInfo nowPos_;
    std::vector<std::string> files_;
    std::vector<std::string> sources_;
    std::vector<std::string> names_;
    std::vector<std::string> mappings_;
    std::vector<SourceMapInfo> afterPos_;

    inline SourceMapData GetSourceMapData() const
    {
        return *this;
    }
};

using ReadSourceMapCallback = std::function<bool(const std::string& hapPath,
    const std::string& sourceMapPath, std::string& content)>;
using GetHapPathCallback = std::function<void(const std::string &bundleName, std::vector<std::string> &hapList)>;
class SourceMap final {
public:
    SourceMap() = default;
    ~SourceMap() = default;

    void Init(bool isModular, const std::string& hapPath);
    std::string TranslateBySourceMap(const std::string& stackStr);
    bool TranslateUrlPositionBySourceMap(std::string& url, int& line, int& column);
    static ErrorPos GetErrorPos(const std::string& rawStack);
    static void RegisterReadSourceMapCallback(ReadSourceMapCallback readFunc);
    static bool ReadSourceMapData(const std::string& hapPath, const std::string& sourceMapPath, std::string& content);
    static void RegisterGetHapPathCallback(GetHapPathCallback getFunc);
    static void GetHapPath(const std::string &bundleName, std::vector<std::string> &hapList);
    bool GetLineAndColumnNumbers(int& line, int& column, SourceMapData& targetMap);
    static void ExtractStackInfo(const std::string& stackStr, std::vector<std::string>& res);

private:
    void SplitSourceMap(const std::string& sourceMapData);
    void ExtractSourceMapData(const std::string& sourceMapData, std::shared_ptr<SourceMapData>& curMapData);
    void ExtractKeyInfo(const std::string& sourceMap, std::vector<std::string>& sourceKeyInfo);
    std::vector<std::string> HandleMappings(const std::string& mapping);
    bool VlqRevCode(const std::string& vStr, std::vector<int32_t>& ans);
    MappingInfo Find(int32_t row, int32_t col, const SourceMapData& targetMap);
    void GetPosInfo(const std::string& temp, int32_t start, std::string& line, std::string& column);
    std::string GetRelativePath(const std::string& sources);
    std::string GetSourceInfo(const std::string& line, const std::string& column, const SourceMapData& targetMap);

private:
    bool isModular_ = true;
    std::string hapPath_;
    std::unordered_map<std::string, std::shared_ptr<SourceMapData>> sourceMaps_;
    std::shared_ptr<SourceMapData> nonModularMap_;
    static ReadSourceMapCallback readSourceMapFunc_;
    static std::mutex sourceMapMutex_;
    static GetHapPathCallback getHapPathFunc_;
};
} // namespace JsEnv
} // namespace OHOS

#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_H
