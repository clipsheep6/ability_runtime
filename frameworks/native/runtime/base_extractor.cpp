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

#include "base_extractor.h"

#include <fstream>

#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
BaseExtractor::BaseExtractor(const std::string &source) : sourceFile_(source), zipFile_(source)
{}

BaseExtractor::~BaseExtractor()
{}

bool BaseExtractor::Init()
{
    if (!zipFile_.Open()) {
        HILOG_ERROR("open zip file failed");
        return false;
    }
    ZipEntry zipEntry;
    initial_ = true;
    return true;
}

bool BaseExtractor::HasEntry(const std::string &fileName) const
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }

    return zipFile_.HasEntry(fileName);
}

bool BaseExtractor::IsDirExist(const std::string &dir) const
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }
    if (dir.empty()) {
        HILOG_ERROR("param dir empty");
        return false;
    }
    return zipFile_.IsDirExist(dir);
}

bool BaseExtractor::ExtractByName(const std::string &fileName, std::ostream &dest) const
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }
    if (!zipFile_.ExtractFile(fileName, dest)) {
        HILOG_ERROR("extractor is not ExtractFile");
        return false;
    }
    return true;
}

bool BaseExtractor::ExtractFile(const std::string &fileName, const std::string &targetPath) const
{
    std::ofstream fileStream;
    fileStream.open(targetPath, std::ios_base::out | std::ios_base::binary);
    if (!fileStream.is_open()) {
        HILOG_ERROR("fail to open %{private}s file to write", targetPath.c_str());
        return false;
    }
    if ((!ExtractByName(fileName, fileStream)) || (!fileStream.good())) {
        HILOG_ERROR("fail to extract %{public}s zip file into stream", fileName.c_str());
        fileStream.clear();
        fileStream.close();
        if (remove(targetPath.c_str()) != 0) {
            HILOG_ERROR("fail to remove %{private}s file which writes stream error", targetPath.c_str());
        }
        return false;
    }
    fileStream.clear();
    fileStream.close();
    return true;
}

bool BaseExtractor::GetZipFileNames(std::vector<std::string> &fileNames)
{
    auto &entryMap = zipFile_.GetAllEntries();
    for (auto &entry : entryMap) {
        fileNames.emplace_back(entry.first);
    }
    return true;
}

bool BaseExtractor::IsStageBasedModel(std::string abilityName)
{
    auto &entryMap = zipFile_.GetAllEntries();
    std::vector<std::string> splitStrs;
    OHOS::SplitStr(abilityName, ".", splitStrs);
    std::string name = splitStrs.empty() ? abilityName : splitStrs.back();
    std::string entry = "assets/js/" + name + "/" + name + ".js";
    bool isStageBasedModel = entryMap.find(entry) != entryMap.end();
    return isStageBasedModel;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
