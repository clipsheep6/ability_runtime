/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_DEAL_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_DEAL_H

#include "fa_context.h"
#include "lifecycle_state_info.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper;
class ContextDeal : public std::enable_shared_from_this<ContextDeal> {
public:
    ContextDeal() = default;
    explicit ContextDeal(bool isCreateBySystemApp);
    virtual ~ContextDeal() = default;

    /**
     * @brief Obtains information about the current application. The returned application information includes basic
     * information such as the application name and application permissions.
     *
     * @return Returns the ApplicationInfo for the current application.
     */
    std::shared_ptr<ApplicationInfo> GetApplicationInfo() const;

    /**
     * @brief Set ApplicationInfo
     *
     * @param info ApplicationInfo instance.
     */
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info);

    /**
     * @brief Obtains the Context object of the application.
     *
     * @return Returns the Context object of the application.
     */
    std::shared_ptr<Context> GetApplicationContext() const;

    /**
     * @brief Set ApplicationContext
     *
     * @param context ApplicationContext instance.
     */
    void SetApplicationContext(const std::shared_ptr<Context> &context);

    /**
     * @brief Obtains the path of the package containing the current ability. The returned path contains the resources,
     *  source code, and configuration files of a module.
     *
     * @return Returns the path of the package file.
     */
    std::string GetBundleCodePath();

    /**
     * @brief SetBundleCodePath
     *
     * @param Returns string path
     */
    void SetBundleCodePath(std::string &path);

    /**
     * @brief Obtains information about the current ability.
     * The returned information includes the class name, bundle name, and other information about the current ability.
     *
     * @return Returns the AbilityInfo object for the current ability.
     */
    const std::shared_ptr<AbilityInfo> GetAbilityInfo();

    /**
     * @brief Set AbilityInfo
     *
     * @param info AbilityInfo instance.
     */
    void SetAbilityInfo(const std::shared_ptr<AbilityInfo> &info);

    /**
     * @brief Obtains the Context object of the ability.
     *
     * @return Returns the Context object of the ability.
     */
    std::shared_ptr<Context> GetContext();

    /**
     * @brief Set Ability context
     *
     * @param context Ability object
     */
    void SetContext(const std::shared_ptr<Context> &context);

    /**
     * @brief Obtains a BundleMgrHelper instance.
     * You can use this instance to obtain information about the application bundle.
     *
     * @return Returns a BundleMgrHelper instance.
     */
    std::shared_ptr<BundleMgrHelper> GetBundleManager() const;

    /**
     * @brief Obtains a resource manager.
     *
     * @return Returns a ResourceManager object.
     */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const;

    /**
     * @brief Obtains the local database path.
     * If the local database path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the local database file.
     */
    std::string GetDatabaseDir();

    /**
     * @brief Obtains the absolute path where all private data files of this application are stored.
     *
     * @return Returns the absolute path storing all private data files of this application.
     */
    std::string GetDataDir();

    /**
     * @brief Obtains the directory for storing custom data files of the application.
     * You can use the returned File object to create and access files in this directory. The files
     * can be accessible only by the current application.
     *
     * @param name Indicates the name of the directory to retrieve. This directory is created as part
     * of your application data.
     * @param mode Indicates the file operating mode. The value can be 0 or a combination of MODE_PRIVATE.
     *
     * @return Returns a File object for the requested directory.
     */
    std::string GetDir(const std::string &name, int mode);

    /**
     * @brief Obtains the directory for storing files for the application on the device's internal storage.
     *
     * @return Returns the application file directory.
     */
    std::string GetFilesDir();

    /**
     * @brief Obtains the bundle name of the current ability.
     *
     * @return Returns the bundle name of the current ability.
     */
    std::string GetBundleName() const;

    /**
     * @brief Obtains the path of the OHOS Ability Package (HAP} containing this ability.
     *
     * @return Returns the path of the HAP containing this ability.
     */
    std::string GetBundleResourcePath();

    /**
     * @brief Obtains an ability manager.
     * The ability manager provides information about running processes and memory usage of an application.
     *
     * @return Returns an IAbilityManager instance.
     */
    sptr<AAFwk::IAbilityManager> GetAbilityManager();

    /**
     * @brief Obtains the type of this application.
     *
     * @return Returns system if this application is a system application;
     * returns normal if it is released in OHOS AppGallery;
     * returns other if it is released by a third-party vendor;
     * returns an empty string if the query fails.
     */
    std::string GetAppType();

    /**
     * @brief Sets the pattern of this Context based on the specified pattern ID.
     *
     * @param patternId Indicates the resource ID of the pattern to set.
     */
    void SetPattern(int patternId);

    /**
     * @brief Obtains the HapModuleInfo object of the application.
     *
     * @return Returns the HapModuleInfo object of the application.
     */
    std::shared_ptr<HapModuleInfo> GetHapModuleInfo();

    /**
     * @brief Obtains the name of the current process.
     *
     * @return Returns the current process name.
     */
    std::string GetProcessName();

    /**
     * @brief init the ResourceManager for ContextDeal.
     *
     * @param the ResourceManager has been inited.
     *
     */
    void initResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);

    /**
     * @brief Get the string of this Context based on the specified resource ID.
     *
     * @param resId Indicates the resource ID of the string to get.
     *
     * @return Returns the string of this Context.
     */
    std::string GetString(int resId);

    /**
     * @brief Get the string array of this Context based on the specified resource ID.
     *
     * @param resId Indicates the resource ID of the string array to get.
     *
     * @return Returns the string array of this Context.
     */
    std::vector<std::string> GetStringArray(int resId);

    /**
     * @brief Get the integer array of this Context based on the specified resource ID.
     *
     * @param resId Indicates the resource ID of the integer array to get.
     *
     * @return Returns the integer array of this Context.
     */
    std::vector<int> GetIntArray(int resId);

    /**
     * @brief Obtains the theme of this Context.
     *
     * @return theme Returns the theme of this Context.
     */
    std::map<std::string, std::string> GetTheme();

    /**
     * @brief Sets the theme of this Context based on the specified theme ID.
     *
     * @param themeId Indicates the resource ID of the theme to set.
     */
    void SetTheme(int themeId);

    /**
     * @brief Obtains the pattern of this Context.
     *
     * @return getPattern in interface Context
     */
    std::map<std::string, std::string> GetPattern();

    /**
     * @brief Get the color of this Context based on the specified resource ID.
     *
     * @param resId Indicates the resource ID of the color to get.
     *
     * @return Returns the color value of this Context.
     */
    int GetColor(int resId);

    /**
     * @brief Obtains the theme id of this Context.
     *
     * @return int Returns the theme id of this Context.
     */
    int GetThemeId();

    /**
     * @brief Obtains the current display orientation of this ability.
     *
     * @return Returns the current display orientation.
     */
    int GetDisplayOrientation();

    /**
     * @brief Obtains the path storing the preference file of the application.
     *        If the preference file path does not exist, the system creates one and returns the created path.
     *
     * @return Returns the preference file path .
     */
    std::string GetPreferencesDir();

    /**
     * @brief Set color mode
     *
     * @param the value of color mode.
     */
    void SetColorMode(int mode);

    /**
     * @brief Obtains color mode.
     *
     * @return Returns the color mode value.
     */
    int GetColorMode();

    /**
     * @brief Obtains the application base directory on the device's internal storage.
     *
     * @return Returns the application base directory.
     */
    std::string GetBaseDir() const;
public:
    static const int64_t CONTEXT_CREATE_BY_SYSTEM_APP;
    static const std::string CONTEXT_DEAL_FILE_SEPARATOR;
    static const std::string CONTEXT_DEAL_Files;
    static const std::string CONTEXT_DISTRIBUTED;
    static const std::string CONTEXT_DATA_STORAGE;
    static const std::string CONTEXT_DEAL_DATA_APP;
    static const std::string CONTEXT_DEAL_BASE;
    static const std::string CONTEXT_DEAL_DATABASE;
    static const std::string CONTEXT_DEAL_PREFERENCES;
    static const std::string CONTEXT_DEAL_DATA;
    int flags_ = 0x00000000;

protected:
    bool HapModuleInfoRequestInit();

private:
    bool IsCreateBySystemApp() const;
    int GetCurrentAccountId() const;
    void CreateDirIfNotExist(const std::string& dirPath) const;

    std::shared_ptr<ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<Context> appContext_ = nullptr;
    std::shared_ptr<Context> abilityContext_ = nullptr;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_ = nullptr;
    std::string path_ = "";
    std::map<std::string, std::string> pattern_;
    std::map<std::string, std::string> theme_;
    AAFwk::LifeCycleStateInfo lifeCycleStateInfo_;
    std::shared_ptr<HapModuleInfo> hapModuleInfoLocal_ = nullptr;
    bool isCreateBySystemApp_ = false;
    std::string currArea_ = "el2";
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_DEAL_H
