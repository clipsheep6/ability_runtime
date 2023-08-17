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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_INNER_BUNDLE_INFO_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_INNER_BUNDLE_INFO_H

#include "nocopyable.h"
#include "ability_info.h"
#include "bundle_constants.h"
#include "common_profile.h"
#include "extension_ability_info.h"
#include "hap_module_info.h"
#include "json_util.h"

namespace OHOS {
namespace AppExecFwk {
struct Distro {
    bool deliveryWithInstall = false;
    std::string moduleName;
    std::string moduleType;
    bool installationFree = false;
};

struct InnerModuleInfo {
    std::string name;
    std::string modulePackage;
    std::string moduleName;
    std::string modulePath;
    std::string moduleDataDir;
    std::string moduleResPath;
    std::string label;
    std::string hapPath;
    int32_t labelId = 0;
    std::string description;
    int32_t descriptionId = 0;
    std::string icon;
    int32_t iconId = 0;
    std::string mainAbility; // config.json : mainAbility; module.json : mainElement
    std::string entryAbilityKey; // skills contains "action.system.home" and "entity.system.home"
    std::string srcPath;
    std::string hashValue;
    bool isEntry = false;
    bool installationFree = false;
    // all user's value of isRemovable
    // key:userId
    // value:isRemovable true or flase
    std::map<std::string, bool> isRemovable;
    MetaData metaData;
    ModuleColorMode colorMode = ModuleColorMode::AUTO;
    Distro distro;
    std::vector<std::string> reqCapabilities;
    std::vector<std::string> abilityKeys;
    std::vector<std::string> skillKeys;
    // new version fields
    std::string pages;
    std::string process;
    std::string srcEntrance;
    std::string uiSyntax;
    std::string virtualMachine;
    bool isModuleJson = false;
    bool isStageBasedModel = false;
    std::vector<std::string> deviceTypes;
    std::vector<std::string> extensionKeys;
    std::vector<std::string> extensionSkillKeys;
    std::vector<Metadata> metadata;
    int32_t upgradeFlag = 0;
    std::vector<Dependency> dependencies;
    std::string compileMode;
    bool isLibIsolated = false;
    std::string nativeLibraryPath;
    std::string cpuAbi;
    std::string targetModuleName;
    int32_t targetPriority;
    AtomicServiceModuleType atomicServiceModuleType;
    std::vector<std::string> preloads;
    BundleType bundleType = BundleType::SHARED;
    uint32_t versionCode = 0;
    std::string versionName;
    std::vector<ProxyData> proxyDatas;
    std::string buildHash;
    std::string isolationMode;
    bool compressNativeLibs = true;
    std::vector<std::string> nativeLibraryFileNames;
    AOTCompileStatus aotCompileStatus = AOTCompileStatus::NOT_COMPILED;
};

enum InstallExceptionStatus : int32_t {
    INSTALL_START = 1,
    INSTALL_FINISH,
    UPDATING_EXISTED_START,
    UPDATING_NEW_START,
    UPDATING_FINISH,
    UNINSTALL_BUNDLE_START,
    UNINSTALL_PACKAGE_START,
    UNKNOWN_STATUS,
};

class InnerBundleInfo {
public:
    enum class BundleStatus {
        ENABLED = 1,
        DISABLED,
    };

    InnerBundleInfo();
    InnerBundleInfo &operator=(const InnerBundleInfo &info);
    ~InnerBundleInfo();
    /**
     * @brief Transform the InnerBundleInfo object to json.
     * @param jsonObject Indicates the obtained json object.
     * @return
     */
    void ToJson(nlohmann::json &jsonObject) const;
    /**
     * @brief Transform the json object to InnerBundleInfo object.
     * @param jsonObject Indicates the obtained json object.
     * @return Returns 0 if the json object parsed successfully; returns error code otherwise.
     */
    int32_t FromJson(const nlohmann::json &jsonObject);
    /**
     * @brief Add module info to old InnerBundleInfo object.
     * @param newInfo Indicates the new InnerBundleInfo object.
     * @return Returns true if the module successfully added; returns false otherwise.
     */
    bool AddModuleInfo(const InnerBundleInfo &newInfo);
    /**
     * @brief Update module info to old InnerBundleInfo object.
     * @param newInfo Indicates the new InnerBundleInfo object.
     * @return
     */
    void UpdateModuleInfo(const InnerBundleInfo &newInfo);
    /**
     * @brief Remove module info from InnerBundleInfo object.
     * @param modulePackage Indicates the module package to be remove.
     * @return
     */
    void RemoveModuleInfo(const std::string &modulePackage);
    /**
     * @brief Find hap module info by module package.
     * @param modulePackage Indicates the module package.
     * @param userId Indicates the user ID.
     * @return Returns the HapModuleInfo object if find it; returns null otherwise.
     */
    std::optional<HapModuleInfo> FindHapModuleInfo(
        const std::string &modulePackage, int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Find abilityInfo by bundle name and ability name.
     * @param moduleName Indicates the module name
     * @param abilityName Indicates the ability name.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo object if find it; returns null otherwise.
     */
    std::optional<AbilityInfo> FindAbilityInfo(
        const std::string &moduleName,
        const std::string &abilityName,
        int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Find abilityInfo by bundle name module name and ability name.
     * @param moduleName Indicates the module name
     * @param abilityName Indicates the ability name.
     * @return Returns ERR_OK if abilityInfo find successfully obtained; returns other ErrCode otherwise.
     */
    ErrCode FindAbilityInfo(
        const std::string &moduleName, const std::string &abilityName, AbilityInfo &info) const;
    /**
     * @brief Find abilityInfo of list by bundle name.
     * @param bundleName Indicates the bundle name.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo of list if find it; returns null otherwise.
     */
    std::optional<std::vector<AbilityInfo>> FindAbilityInfos(
        int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Find extensionInfo by bundle name and extension name.
     * @param moduleName Indicates the module name.
     * @param extensionName Indicates the extension name
     * @return Returns the ExtensionAbilityInfo object if find it; returns null otherwise.
     */
    std::optional<ExtensionAbilityInfo> FindExtensionInfo(
        const std::string &moduleName, const std::string &extensionName) const;
    /**
     * @brief Find extensionInfos by bundle name.
     * @param bundleName Indicates the bundle name.
     * @return Returns the ExtensionAbilityInfo array if find it; returns null otherwise.
     */
    std::optional<std::vector<ExtensionAbilityInfo>> FindExtensionInfos() const;
    /**
     * @brief Transform the InnerBundleInfo object to string.
     * @return Returns the string object
     */
    std::string ToString() const;
    /**
     * @brief Add ability infos to old InnerBundleInfo object.
     * @param abilityInfos Indicates the AbilityInfo object to be add.
     * @return
     */
    void AddModuleAbilityInfo(const std::map<std::string, AbilityInfo> &abilityInfos)
    {
        for (const auto &ability : abilityInfos) {
            baseAbilityInfos_.try_emplace(ability.first, ability.second);
        }
    }

    void AddModuleExtensionInfos(const std::map<std::string, ExtensionAbilityInfo> &extensionInfos)
    {
        for (const auto &extensionInfo : extensionInfos) {
            baseExtensionInfos_.try_emplace(extensionInfo.first, extensionInfo.second);
        }
    }
    /**
     * @brief Add innerModuleInfos to old InnerBundleInfo object.
     * @param innerModuleInfos Indicates the InnerModuleInfo object to be add.
     * @return
     */
    void AddInnerModuleInfo(const std::map<std::string, InnerModuleInfo> &innerModuleInfos)
    {
        for (const auto &info : innerModuleInfos) {
            innerModuleInfos_.try_emplace(info.first, info.second);
        }
    }
    /**
     * @brief Get application name.
     * @return Return application name
     */
    std::string GetApplicationName() const
    {
        return baseApplicationInfo_->name;
    }
    /**
     * @brief Set bundle status.
     * @param status Indicates the BundleStatus object to set.
     * @return
     */
    void SetBundleStatus(const BundleStatus &status)
    {
        bundleStatus_ = status;
    }
    /**
     * @brief Get bundle status.
     * @return Return the BundleStatus object
     */
    BundleStatus GetBundleStatus() const
    {
        return bundleStatus_;
    }
    /**
     * @brief Set bundle install time.
     * @param time Indicates the install time to set.
     * @param userId Indicates the user ID.
     * @return
     */
    void SetBundleInstallTime(
        const int64_t time, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Set bundle update time.
     * @param time Indicates the update time to set.
     * @param userId Indicates the user ID.
     * @return
     */
    void SetBundleUpdateTime(const int64_t time, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get bundle name.
     * @return Return bundle name
     */
    const std::string GetBundleName() const
    {
        return baseApplicationInfo_->bundleName;
    }
    /**
     * @brief Get baseApplicationInfo.
     * @return Return the ApplicationInfo object.
     */
    ApplicationInfo GetBaseApplicationInfo() const
    {
        return *baseApplicationInfo_;
    }
    /**
     * @brief Set baseApplicationInfo.
     * @param applicationInfo Indicates the ApplicationInfo object.
     */
    void SetBaseApplicationInfo(const ApplicationInfo &applicationInfo)
    {
        *baseApplicationInfo_ = applicationInfo;
    }
    /**
     * @brief Update baseApplicationInfo.
     * @param applicationInfo Indicates the ApplicationInfo object.
     */
    void UpdateBaseApplicationInfo(const ApplicationInfo &applicationInfo);
    /**
     * @brief Set application enabled.
     * @param userId Indicates the user ID.
     * @return Returns ERR_OK if the SetApplicationEnabled is successfully; returns error code otherwise.
     */
    ErrCode SetApplicationEnabled(bool enabled, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get application code path.
     * @return Return the string object.
     */
    const std::string GetAppCodePath() const
    {
        return baseApplicationInfo_->codePath;
    }
    /**
     * @brief Insert innerModuleInfos.
     * @param modulePackage Indicates the modulePackage object as key.
     * @param innerModuleInfo Indicates the InnerModuleInfo object as value.
     */
    void InsertInnerModuleInfo(const std::string &modulePackage, const InnerModuleInfo &innerModuleInfo)
    {
        innerModuleInfos_.try_emplace(modulePackage, innerModuleInfo);
    }
    /**
     * @brief Insert AbilityInfo.
     * @param key bundleName.moduleName.abilityName
     * @param abilityInfo value.
     */
    void InsertAbilitiesInfo(const std::string &key, const AbilityInfo &abilityInfo)
    {
        baseAbilityInfos_.emplace(key, abilityInfo);
    }
    /**
     * @brief Insert ExtensionAbilityInfo.
     * @param key bundleName.moduleName.extensionName
     * @param extensionInfo value.
     */
    void InsertExtensionInfo(const std::string &key, const ExtensionAbilityInfo &extensionInfo)
    {
        baseExtensionInfos_.emplace(key, extensionInfo);
    }
    /**
     * @brief Get application data dir.
     * @return Return the string object.
     */
    std::string GetAppDataDir() const
    {
        return baseApplicationInfo_->dataDir;
    }
    /**
     * @brief Set application data dir.
     * @param dataDir Indicates the data Dir to be set.
     */
    void SetAppDataDir(std::string dataDir)
    {
        baseApplicationInfo_->dataDir = dataDir;
    }
    /**
     * @brief Set application data base dir.
     * @param dataBaseDir Indicates the data base Dir to be set.
     */
    void SetAppDataBaseDir(std::string dataBaseDir)
    {
        baseApplicationInfo_->dataBaseDir = dataBaseDir;
    }
    /**
     * @brief Set application cache dir.
     * @param cacheDir Indicates the cache Dir to be set.
     */
    void SetAppCacheDir(std::string cacheDir)
    {
        baseApplicationInfo_->cacheDir = cacheDir;
    }
    /**
     * @brief Get application AppType.
     * @return Returns the AppType.
     */
    Constants::AppType GetAppType() const
    {
        return appType_;
    }
    /**
     * @brief Set application AppType.
     * @param gid Indicates the AppType to be set.
     */
    void SetAppType(Constants::AppType appType)
    {
        appType_ = appType;
        if (appType_ == Constants::AppType::SYSTEM_APP) {
            baseApplicationInfo_->isSystemApp = true;
        } else {
            baseApplicationInfo_->isSystemApp = false;
        }
    }
    // only used in install progress with newInfo
    std::string GetCurrentModulePackage() const
    {
        return currentPackage_;
    }
    void SetCurrentModulePackage(const std::string &modulePackage)
    {
        currentPackage_ = modulePackage;
    }
    void AddModuleSrcDir(const std::string &moduleSrcDir)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).modulePath = moduleSrcDir;
        }
    }
    void AddModuleDataDir(const std::string &moduleDataDir)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).moduleDataDir = moduleDataDir;
        }
    }

    void SetModuleHapPath(const std::string &hapPath);

    const std::string &GetModuleHapPath(const std::string &modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).hapPath;
        }

        return Constants::EMPTY_STRING;
    }

    const std::string &GetModuleName(const std::string &modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).moduleName;
        }

        return Constants::EMPTY_STRING;
    }

    const std::string &GetCurModuleName() const;

    bool FindModule(std::string modulePackage) const
    {
        return (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end());
    }

    bool IsEntryModule(std::string modulePackage) const
    {
        if (FindModule(modulePackage)) {
            return innerModuleInfos_.at(modulePackage).isEntry;
        }
        return false;
    }

    std::string GetMainAbility() const;

    void GetMainAbilityInfo(AbilityInfo &abilityInfo) const;

    std::string GetModuleDir(std::string modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).modulePath;
        }
        return Constants::EMPTY_STRING;
    }

    std::string GetModuleDataDir(std::string modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).moduleDataDir;
        }
        return Constants::EMPTY_STRING;
    }

    void SetAppPrivilegeLevel(const std::string &appPrivilegeLevel)
    {
        if (appPrivilegeLevel.empty()) {
            return;
        }
        baseApplicationInfo_->appPrivilegeLevel = appPrivilegeLevel;
    }

    std::string GetAppPrivilegeLevel() const
    {
        return baseApplicationInfo_->appPrivilegeLevel;
    }

    bool HasEntry() const;

    /**
     * @brief Obtains configuration information about an application.
     * @param flags Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     */
    void GetApplicationInfo(int32_t flags, int32_t userId, ApplicationInfo &appInfo) const;

    std::optional<InnerModuleInfo> GetInnerModuleInfoByModuleName(const std::string &moduleName) const;

    /**
     * @brief Obtains all abilityInfos.
     */
    const std::map<std::string, AbilityInfo> &GetInnerAbilityInfos() const
    {
        return baseAbilityInfos_;
    }
    /**
     * @brief Fetch all extensionAbilityInfos, can be modify.
     */
    std::map<std::string, ExtensionAbilityInfo> &FetchInnerExtensionInfos()
    {
        return baseExtensionInfos_;
    }
    /**
     * @brief Obtains all extensionAbilityInfos.
     */
    const std::map<std::string, ExtensionAbilityInfo> &GetInnerExtensionInfos() const
    {
        return baseExtensionInfos_;
    }

    std::vector<std::string> GetModuleNameVec() const
    {
        std::vector<std::string> moduleVec;
        for (const auto &it : innerModuleInfos_) {
            moduleVec.emplace_back(it.first);
        }
        return moduleVec;
    }

    void SetIsNewVersion(bool flag)
    {
        isNewVersion_ = flag;
    }

    bool GetIsNewVersion() const
    {
        return isNewVersion_;
    }
    /**
     * @brief Add module removable info
     * @param info Indicates the innerModuleInfo of module.
     * @param stringUserId Indicates the string userId add to isRemovable map.
     * @param isEnable Indicates the value of enable module is removed.
     * @return Return add module isRemovable info result.
     */
    bool AddModuleRemovableInfo(InnerModuleInfo &info, const std::string &stringUserId, bool isEnable) const;

    void SetAppIndex(int32_t appIndex)
    {
        appIndex_ = appIndex;
    }

    int32_t GetAppIndex() const
    {
        return appIndex_;
    }

    void SetIsSandbox(bool isSandbox)
    {
        isSandboxApp_ = isSandbox;
    }

    bool GetIsSandbox() const
    {
        return isSandboxApp_;
    }

    std::string GetCertificateFingerprint() const
    {
        return baseApplicationInfo_->fingerprint;
    }

    void SetCertificateFingerprint(const std::string &fingerprint)
    {
        baseApplicationInfo_->fingerprint = fingerprint;
    }

    const std::string &GetNativeLibraryPath() const
    {
        return baseApplicationInfo_->nativeLibraryPath;
    }

    void SetNativeLibraryPath(const std::string &nativeLibraryPath)
    {
        baseApplicationInfo_->nativeLibraryPath = nativeLibraryPath;
    }

    const std::string &GetArkNativeFileAbi() const
    {
        return baseApplicationInfo_->arkNativeFileAbi;
    }

    void SetArkNativeFileAbi(const std::string &arkNativeFileAbi)
    {
        baseApplicationInfo_->arkNativeFileAbi = arkNativeFileAbi;
    }

    const std::string &GetArkNativeFilePath() const
    {
        return baseApplicationInfo_->arkNativeFilePath;
    }

    void SetArkNativeFilePath(const std::string &arkNativeFilePath)
    {
        baseApplicationInfo_->arkNativeFilePath = arkNativeFilePath;
    }

    const std::string &GetCpuAbi() const
    {
        return baseApplicationInfo_->cpuAbi;
    }

    void SetCpuAbi(const std::string &cpuAbi)
    {
        baseApplicationInfo_->cpuAbi = cpuAbi;
    }

    void SetRemovable(bool removable)
    {
        baseApplicationInfo_->removable = removable;
    }

    void SetKeepAlive(bool keepAlive)
    {
        baseApplicationInfo_->keepAlive = keepAlive;
    }

    void SetSingleton(bool singleton)
    {
        baseApplicationInfo_->singleton = singleton;
    }

    void SetRunningResourcesApply(bool runningResourcesApply)
    {
        baseApplicationInfo_->runningResourcesApply = runningResourcesApply;
    }

    void SetAssociatedWakeUp(bool associatedWakeUp)
    {
        baseApplicationInfo_->associatedWakeUp = associatedWakeUp;
    }

    void SetUserDataClearable(bool userDataClearable)
    {
        baseApplicationInfo_->userDataClearable = userDataClearable;
    }

    void SetHideDesktopIcon(bool hideDesktopIcon)
    {
        baseApplicationInfo_->hideDesktopIcon = hideDesktopIcon;
        if (hideDesktopIcon) {
            baseApplicationInfo_->needAppDetail = false;
            baseApplicationInfo_->appDetailAbilityLibraryPath = Constants::EMPTY_STRING;
        }
    }

    void SetFormVisibleNotify(bool formVisibleNotify)
    {
        baseApplicationInfo_->formVisibleNotify = formVisibleNotify;
    }

    std::string GetTargetBundleName() const
    {
        return baseApplicationInfo_->targetBundleName;
    }

    void SetTargetBundleName(const std::string &targetBundleName)
    {
        baseApplicationInfo_->targetBundleName = targetBundleName;
    }

    int32_t GetTargetPriority() const
    {
        return baseApplicationInfo_->targetPriority;
    }

    void SetTargetPriority(int32_t priority)
    {
        baseApplicationInfo_->targetPriority = priority;
    }

    void SetApplicationBundleType(BundleType type)
    {
        baseApplicationInfo_->bundleType = type;
    }

    BundleType GetApplicationBundleType() const
    {
        return baseApplicationInfo_->bundleType;
    }

    void SetAppProvisionMetadata(const std::vector<Metadata> &metadatas)
    {
        provisionMetadatas_ = metadatas;
    }

    std::vector<Metadata> GetAppProvisionMetadata() const
    {
        return provisionMetadatas_;
    }

    void SetAppProvisionType(const std::string &appProvisionType);

    std::string GetAppProvisionType() const;

    std::string GetModuleNameByPackage(const std::string &packageName) const;

    std::string GetModuleTypeByPackage(const std::string &packageName) const;

    void UpdatePrivilegeCapability(const ApplicationInfo &applicationInfo);

private:
    void RemoveDuplicateName(std::vector<std::string> &name) const;
    IsolationMode GetIsolationMode(const std::string &isolationMode) const;
    void UpdateIsCompressNativeLibs();

    // using for get
    Constants::AppType appType_ = Constants::AppType::THIRD_PARTY_APP;
    int userId_ = Constants::DEFAULT_USERID;
    BundleStatus bundleStatus_ = BundleStatus::ENABLED;
    std::shared_ptr<ApplicationInfo> baseApplicationInfo_;
    std::string appFeature_;
    std::vector<std::string> allowedAcls_;
    int32_t appIndex_ = Constants::INITIAL_APP_INDEX;
    bool isSandboxApp_ = false;
    std::string currentPackage_;
    bool onlyCreateBundleUser_ = false;
    std::map<std::string, InnerModuleInfo> innerModuleInfos_;
    std::map<std::string, AbilityInfo> baseAbilityInfos_;
    bool isNewVersion_ = false;
    std::map<std::string, ExtensionAbilityInfo> baseExtensionInfos_;
    std::vector<Metadata> provisionMetadatas_;
};

void from_json(const nlohmann::json &jsonObject, InnerModuleInfo &info);
void from_json(const nlohmann::json &jsonObject, Distro &distro);
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_INNER_BUNDLE_INFO_H
