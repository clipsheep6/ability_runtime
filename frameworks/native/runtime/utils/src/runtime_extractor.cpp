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

#include "runtime_extractor.h"

#include <fstream>
#include <regex>
#include <sstream>

#include "ability_constants.h"
#include "file_path_utils.h"
#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
RuntimeExtractor::RuntimeExtractor(const std::string &source) : sourceFile_(source), zipFile_(source)
{
    hapPath_ = source;
}

RuntimeExtractor::~RuntimeExtractor()
{}

bool RuntimeExtractor::Init()
{
    if (!zipFile_.Open()) {
        HILOG_ERROR("open zip file failed");
        return false;
    }
    ZipEntry zipEntry;
    initial_ = true;
    return true;
}

std::shared_ptr<RuntimeExtractor> RuntimeExtractor::Create(const std::string& hapPath)
{
    if (hapPath.empty()) {
        HILOG_ERROR("source is nullptr");
        return nullptr;
    }

    std::string loadPath;
    if (StringStartWith(hapPath, Constants::ABS_CODE_PATH, std::string(Constants::ABS_CODE_PATH).length())) {
        loadPath = GetLoadPath(hapPath);
    } else {
        loadPath = hapPath;
    }
    std::shared_ptr<RuntimeExtractor> runtimeExtractor = std::make_shared<RuntimeExtractor>(loadPath);
    if (!runtimeExtractor->Init()) {
        HILOG_ERROR("RuntimeExtractor create failed");
        return nullptr;
    }

    return runtimeExtractor;
}

bool RuntimeExtractor::GetFileBuffer(const std::string& srcPath, std::ostringstream& dest)
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }

    if (srcPath.empty()) {
        HILOG_ERROR("GetFileBuffer::srcPath is nullptr");
        return false;
    }

    std::string relativePath = GetRelativePath(srcPath);
    if (!ExtractByName(relativePath, dest)) {
        HILOG_ERROR("GetFileBuffer::Extract file failed");
        return false;
    }

    return true;
}

bool RuntimeExtractor::GetFileList(const std::string& srcPath, std::vector<std::string>& assetList)
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }

    if (srcPath.empty()) {
        HILOG_ERROR("GetFileList::srcPath is nullptr");
        return false;
    }

    std::vector<std::string> fileList;
    if (!GetZipFileNames(fileList)) {
        HILOG_ERROR("GetFileList::Get file list failed");
        return false;
    }

    std::regex replacePattern(srcPath);
    for (auto value : fileList) {
        if (StringStartWith(value, srcPath.c_str(), srcPath.length())) {
            std::string realpath = std::regex_replace(value, replacePattern, "");
            assetList.emplace_back(value);
        }
    }

    return true;
}

bool RuntimeExtractor::HasEntry(const std::string &fileName) const
{
    if (!initial_) {
        HILOG_ERROR("extractor is not initial");
        return false;
    }

    return zipFile_.HasEntry(fileName);
}

bool RuntimeExtractor::IsDirExist(const std::string &dir) const
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

bool RuntimeExtractor::ExtractByName(const std::string &fileName, std::ostream &dest) const
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

bool RuntimeExtractor::ExtractFile(const std::string &fileName, const std::string &targetPath) const
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

bool RuntimeExtractor::GetZipFileNames(std::vector<std::string> &fileNames)
{
    auto &entryMap = zipFile_.GetAllEntries();
    for (auto &entry : entryMap) {
        fileNames.emplace_back(entry.first);
    }
    return true;
}

void RuntimeExtractor::GetSpecifiedTypeFiles(std::vector<std::string> &fileNames, const std::string &suffix)
{
    auto &entryMap = zipFile_.GetAllEntries();
    for (auto &entry : entryMap) {
        std::string fileName = entry.first;
        auto position = fileName.rfind('.');
        if (position != std::string::npos) {
            std::string suffixStr = fileName.substr(position);
            if (LowerStr(suffixStr) == suffix) {
                fileNames.emplace_back(fileName);
            }
        }
    }
    return;
}

bool RuntimeExtractor::IsStageBasedModel(std::string abilityName)
{
    auto &entryMap = zipFile_.GetAllEntries();
    std::vector<std::string> splitStrs;
    OHOS::SplitStr(abilityName, ".", splitStrs);
    std::string name = splitStrs.empty() ? abilityName : splitStrs.back();
    std::string entry = "assets/js/" + name + "/" + name + ".js";
    bool isStageBasedModel = entryMap.find(entry) != entryMap.end();
    return isStageBasedModel;
}

bool RuntimeExtractor::IsSameHap(const std::string& hapPath) const
{
    return !hapPath_.empty() && !hapPath.empty() && hapPath_ == hapPath;
}

void RuntimeExtractor::SetRuntimeFlag(bool isRuntime)
{
    zipFile_.SetIsRuntime(isRuntime);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
