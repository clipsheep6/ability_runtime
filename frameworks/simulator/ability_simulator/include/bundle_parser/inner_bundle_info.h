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
     * @brief Transform the InnerBundleInfo object to string.
     * @return Returns the string object
     */
    std::string ToString() const;
    /**
     * @brief Get bundle name.
     * @return Return bundle name
     */
    const std::string GetBundleName() const
    {
        return baseApplicationInfo_->bundleName;
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
     * @brief Get application AppType.
     * @return Returns the AppType.
     */
    Constants::AppType GetAppType() const
    {
        return appType_;
    }

    void SetCurrentModulePackage(const std::string &modulePackage)
    {
        currentPackage_ = modulePackage;
    }

    /**
     * @brief Obtains configuration information about an application.
     * @param flags Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     */
    void GetApplicationInfo(int32_t flags, int32_t userId, ApplicationInfo &appInfo) const;

    void SetKeepAlive(bool keepAlive)
    {
        baseApplicationInfo_->keepAlive = keepAlive;
    }

    void SetTargetPriority(int32_t priority)
    {
        baseApplicationInfo_->targetPriority = priority;
    }

    void SetIsNewVersion(bool isNewVersion)
    {
        isNewVersion_ = isNewVersion;
    }

private:
    void RemoveDuplicateName(std::vector<std::string> &name) const;
    IsolationMode GetIsolationMode(const std::string &isolationMode) const;

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
