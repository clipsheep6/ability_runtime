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

#ifndef FOUNDATION_ABILITY_RUNTIME_SOURCE_MAP_H
#define FOUNDATION_ABILITY_RUNTIME_SOURCE_MAP_H

#include <cstring>
#include <fstream>
#include <limits.h>
#include <map>
#include <utility>
#include <thread>
#include <vector>

namespace panda::ecmascript {
class EcmaVM;
} // namespace panda::ecmascript
namespace OHOS::AbilityRuntime {
using ErrorPos = std::pair<uint32_t, uint32_t>;
using panda::ecmascript::EcmaVM;
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

class ModSourceMap final {
public:
    explicit ModSourceMap() = default;
    explicit ModSourceMap(const bool isStageModel) : isStageModel(isStageModel) {};
    explicit ModSourceMap(const std::string& bundleCodeDir, const bool isStageModel) : isStageModel(isStageModel),
        bundleCodeDir_(bundleCodeDir) {};
    ~ModSourceMap() = default;

    static std::string TranslateBySourceMap(const std::string& stackStr, ModSourceMap& targetMaps,
        const std::string& hapPath);
    static std::string GetOriginalNames(std::shared_ptr<SourceMapData> targetMapData,
        const std::string& sourceCode, uint32_t& errorPos);
    static ErrorPos GetErrorPos(const std::string& rawStack);
    static void NonModularLoadSourceMap(ModSourceMap& targetMaps, const std::string& targetMap);

    bool isStageModel = true;

private:
    static void Init(const std::string& sourceMap, SourceMapData& curMap);
    static MappingInfo Find(int32_t row, int32_t col, const SourceMapData& targetMap, const std::string& key);
    static void ExtractKeyInfo(const std::string& sourceMap, std::vector<std::string>& sourceKeyInfo);
    static void GetPosInfo(const std::string& temp, int32_t start, std::string& line, std::string& column);
    static int32_t StringToInt(const std::string& value);
    static std::string GetRelativePath(const std::string& sources);
    static std::string GetSourceInfo(const std::string& line, const std::string& column,
        const SourceMapData& targetMap, const std::string& key);
    static bool ReadSourceMapData(const std::string& hapPath, std::string& content);
    static std::vector<std::string> HandleMappings(const std::string& mapping);
    static uint32_t Base64CharToInt(char charCode);
    static bool VlqRevCode(const std::string& vStr, std::vector<int32_t>& ans);

    std::string bundleCodeDir_;
    std::map<std::string, SourceMapData> sourceMaps_;
    std::shared_ptr<SourceMapData> nonModularMap_;
};
}   // namespace OHOS::AbilityRuntime

#endif // FOUNDATION_ACE_FRAMEWORKS_BRIDGE_COMMON_UTILS_SOURCE_MAP_H
