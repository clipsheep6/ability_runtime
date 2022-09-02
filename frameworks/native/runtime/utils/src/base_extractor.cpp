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
#include <regex>
#include <sstream>

#include "ability_constants.h"
#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
inline bool StringStartWith(const std::string& str, const char* startStr, size_t startStrLen)
{
    return ((str.length() >= startStrLen) && (str.compare(0, startStrLen, startStr) == 0));
}
} // namespace

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

std::shared_ptr<BaseExtractor> BaseExtractor::Create()
{
    if (sourceFile_.empty()) {
        HILOG_ERROR("source is nullptr");
        return std::shared_ptr<BaseExtractor>();
    }

    std::string loadPath;
    if (StringStartWith(sourceFile_, Constants::ABS_CODE_PATH, std::string(Constants::ABS_CODE_PATH).length())) {
        loadPath = GetLoadPath(sourceFile_);
    } else {
        loadPath = sourceFile_;
    }
    std::shared_ptr<BaseExtractor> baseExtractor = std::make_shared<BaseExtractor>(loadPath);
    if (!baseExtractor->Init()) {
        HILOG_ERROR("BaseExtractor create failed");
        return std::shared_ptr<BaseExtractor>();
    }

    return baseExtractor;
}

bool BaseExtractor::GetFileBuffer(const std::string& srcPath, std::ostringstream& dest)
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

bool BaseExtractor::GetFileList(const std::string& srcPath, std::vector<std::string>& assetList)
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
        if (StringStartWith(value, srcPath.c_str(), sizeof(srcPath.c_str()) - 1)) {
            std::string realpath = std::regex_replace(value, replacePattern, "");
            if (realpath.find(Constants::FILE_SEPARATOR) != std::string::npos) {
                continue;
            }
            assetList.emplace_back(value);
        }
    }

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

std::string BaseExtractor::GetLoadPath(const std::string& hapPath)
{
    std::regex hapPattern(std::string(Constants::ABS_CODE_PATH) + std::string(Constants::FILE_SEPARATOR));
    std::string loadPath = std::regex_replace(hapPath, hapPattern, "");
    loadPath = std::string(Constants::LOCAL_CODE_PATH) + std::string(Constants::FILE_SEPARATOR) +
        loadPath.substr(loadPath.find(std::string(Constants::FILE_SEPARATOR)) + 1);
    return loadPath;
}

std::string BaseExtractor::GetRelativePath(const std::string& srcPath)
{
    std::regex srcPattern(Constants::LOCAL_CODE_PATH);
    std::string relativePath = std::regex_replace(srcPath, srcPattern, "");
    if (relativePath.find(Constants::FILE_SEPARATOR) == 0) {
        relativePath = relativePath.substr(1);
        relativePath = relativePath.substr(relativePath.find(std::string(Constants::FILE_SEPARATOR)) + 1);
    }
    return relativePath;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
