/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONTEXT_H

#include "ability_info.h"
#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief context supply for extension
 *
 */
class ExtensionContext : public ContextImpl {
public:
    ExtensionContext() = default;
    virtual ~ExtensionContext() = default;

    /**
     * @brief Obtains information about the current ability.
     * The returned information includes the class name, bundle name, and other information about the current ability.
     *
     * @return Returns the AbilityInfo object for the current ability.
     */
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> GetAbilityInfo() const;

    /**
     * @brief Set AbilityInfo when init.
     *
     */
    void SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo);

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodeDir() override;

    /**
     * @brief Obtains the application-specific cache directory on the device's internal storage. The system
     * automatically deletes files from the cache directory if disk space is required elsewhere on the device.
     * Older files are always deleted first.
     *
     * @return Returns the application-specific cache directory.
     */
    std::string GetCacheDir() override;
    /**
     * @brief Obtains the temporary directory.
     *
     * @return Returns the application temporary directory.
     */
    std::string GetTempDir() override;

    std::string GetResourceDir() override;

    /**
     * @brief Obtains the directory for storing files for the application on the device's internal storage.
     *
     * @return Returns the application file directory.
     */
    std::string GetFilesDir() override;

    /**
     * @brief Obtains the local database path.
     * If the local database path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the local database file.
     */
    std::string GetDatabaseDir() override;

    /**
     * @brief Obtains the path storing the storage file of the application.
     *
     * @return Returns the local storage file.
     */
    std::string GetPreferencesDir() override;

    /**
     * @brief Obtains the path storing the group file of the application by the groupId.
     *
     * @return Returns the local group file.
     */
    std::string GetGroupDir(std::string groupId) override;

    /**
     * @brief Obtains the path distributed file of the application
     *
     * @return Returns the distributed file.
     */
    std::string GetDistributedFilesDir() override;

    /**
     * @brief Obtains the application base directory on the device's internal storage.
     *
     * @return Returns the application base directory.
     */
    std::string GetBaseDir() const override;

    using SelfType = ExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }
private:
    int32_t GetDatabaseDirWithCheck(bool checkExist, std::string &databaseDir);
    int32_t GetGroupDirWithCheck(const std::string &groupId, bool checkExist, std::string &groupDir);
    int32_t GetPreferencesDirWithCheck(bool checkExist, std::string &preferencesDir);

    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
    static const std::string CONTEXT_EXTENSION;
    static const int64_t CONTEXT_CREATE_BY_SYSTEM_APP;
    static const std::string CONTEXT_DATA_APP;
    static const std::string CONTEXT_BUNDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE;
    static const std::string CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE;
    static const std::string CONTEXT_DISTRIBUTEDFILES;
    static const std::string CONTEXT_FILE_SEPARATOR;
    static const std::string CONTEXT_DATA;
    static const std::string CONTEXT_DATA_STORAGE;
    static const std::string CONTEXT_BASE;
    static const std::string CONTEXT_CACHE;
    static const std::string CONTEXT_PREFERENCES;
    static const std::string CONTEXT_GROUP;
    static const std::string CONTEXT_DATABASE;
    static const std::string CONTEXT_TEMP;
    static const std::string CONTEXT_FILES;
    static const std::string CONTEXT_HAPS;
    static const std::string CONTEXT_ELS[];
    static const std::string CONTEXT_RESOURCE_END;

};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EXTENSION_CONTEXT_H
