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

#ifndef OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H
#define OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H

#include <string>

#include "zip_file.h"

namespace OHOS {
namespace AbilityRuntime {
class RuntimeExtractor {
public:
    explicit RuntimeExtractor(const std::string &source);
    virtual ~RuntimeExtractor();
    static std::shared_ptr<RuntimeExtractor> Create(const std::string& hapPath);

    /**
     * @brief Open compressed file.
     * @return Returns true if the file is successfully opened; returns false otherwise.
     */
    virtual bool Init();

    /**
     * @brief Extract to dest stream by file name.
     * @param fileName Indicates the file name.
     * @param dest Indicates the obtained std::ostream object.
     * @return Returns true if the file extracted successfully; returns false otherwise.
     */
    bool ExtractByName(const std::string &fileName, std::ostream &dest) const;
    /**
     * @brief Extract to dest path on filesystem.
     * @param fileName Indicates the file name.
     * @param targetPath Indicates the target Path.
     * @return Returns true if the file extracted to filesystem successfully; returns false otherwise.
     */
    bool ExtractFile(const std::string &fileName, const std::string &targetPath) const;
    /**
     * @brief Get all file names in a hap file.
     * @param fileName Indicates the obtained file names in hap.
     * @return Returns true if the file names obtained successfully; returns false otherwise.
     */
    bool GetZipFileNames(std::vector<std::string> &fileNames);
    /**
     * @brief Get specified type names in a zip file.
     * @param fileNames Indicates the obtained file names in zip.
     * @param suffix Indicates the suffix of file.
     */
    void GetSpecifiedTypeFiles(std::vector<std::string> &fileNames, const std::string &suffix);
    /**
     * @brief Has entry by name.
     * @param entryName Indicates the entry name.
     * @return Returns true if the ZipEntry is successfully finded; returns false otherwise.
     */
    bool HasEntry(const std::string &fileName) const;
    bool IsDirExist(const std::string &dir) const;
    bool IsStageBasedModel(std::string abilityName);
    bool GetFileBuffer(const std::string& srcPath, std::ostringstream& dest);
    bool GetFileList(const std::string& srcPath, std::vector<std::string>& assetList);
    bool IsSameHap(const std::string& hapPath) const;
    void SetRuntimeFlag(bool isRuntime);

private:
    const std::string sourceFile_;
    ZipFile zipFile_;
    bool initial_ = false;
    std::string hapPath_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RUNTIME_EXTRACTOR_H
