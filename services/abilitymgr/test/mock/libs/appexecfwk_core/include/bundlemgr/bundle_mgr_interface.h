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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_INTERFACE_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_INTERFACE_H

#include "ability_info.h"
#include "application_info.h"
#include "bundle_info.h"
#include "hap_module_info.h"
#include "permission_def.h"
#include "bundle_installer_interface.h"
#include "bundle_status_callback_interface.h"
#include "clean_cache_callback_interface.h"
#include "ohos/aafwk/content/want.h"

namespace OHOS {
namespace AppExecFwk {

enum class DumpFlag {
    DUMP_BUNDLE_LIST = 1,  // corresponse to option "-bundle-list"
    DUMP_ALL_BUNDLE_INFO,  // corresponse to option "-bundle"
    DUMP_BUNDLE_INFO,      // corresponse to option "-bundle [name]"
};

class IBundleMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.BundleMgr");

    using Want = OHOS::AAFwk::Want;
    /**
     * @brief Obtains the ApplicationInfo based on a given bundle name.
     * @param appName Indicates the application bundle name to be queried.
     * @param flag Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     * @return Returns true if the application is successfully obtained; returns false otherwise.
     */
    virtual bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) = 0;
    /**
     * @brief Obtains information about all installed applications of a specified user.
     * @param flag Indicates the flag used to specify information contained
     *             in the ApplicationInfo objects that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfos Indicates all of the obtained ApplicationInfo objects.
     * @return Returns true if the application is successfully obtained; returns false otherwise.
     */
    virtual bool GetApplicationInfos(
        const ApplicationFlag flag, const int userId, std::vector<ApplicationInfo> &appInfos) = 0;
    /**
     * @brief Obtains the BundleInfo based on a given bundle name.
     * @param bundleName Indicates the application bundle name to be queried.
     * @param flag Indicates the information contained in the BundleInfo object to be returned.
     * @param bundleInfo Indicates the obtained BundleInfo object.
     * @return Returns true if the BundleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo) = 0;
    /**
     * @brief Obtains BundleInfo of all bundles available in the system.
     * @param flag Indicates the flag used to specify information contained in the BundleInfo that will be returned.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos) = 0;
    /**
     * @brief Obtains the application UID based on the given bundle name and user ID.
     * @param bundleName Indicates the bundle name of the application.
     * @param userId Indicates the user ID.
     * @return Returns the uid if successfully obtained; returns -1 otherwise.
     */
    virtual int GetUidByBundleName(const std::string &bundleName, const int userId) = 0;
    /**
     * @brief Obtains the bundle name of a specified application based on the given UID.
     * @param uid Indicates the uid.
     * @param bundleName Indicates the obtained bundle name.
     * @return Returns true if the bundle name is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) = 0;
    /**
     * @brief Obtains an array of all group IDs associated with a specified bundle.
     * @param bundleName Indicates the bundle name.
     * @param gids Indicates the group IDs associated with the specified bundle.
     * @return Returns true if the gids is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleGids(const std::string &bundleName, std::vector<int> &gids) = 0;
    /**
     * @brief Obtains the type of a specified application based on the given bundle name.
     * @param bundleName Indicates the bundle name.
     * @return Returns "system" if the bundle is a system application; returns "third-party" otherwise.
     */
    virtual std::string GetAppType(const std::string &bundleName) = 0;
    /**
     * @brief Check whether the app is system app by it's UID.
     * @param uid Indicates the uid.
     * @return Returns true if the bundle is a system application; returns false otherwise.
     */
    virtual bool CheckIsSystemAppByUid(const int uid) = 0;
    /**
     * @brief Obtains the BundleInfo of application bundles based on the specified metaData.
     * @param metaData Indicates the metadata to get in the bundle.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleInfosByMetaData(const std::string &metaData, std::vector<BundleInfo> &bundleInfos) = 0;
    /**
     * @brief Query the AbilityInfo by the given Want.
     * @param want Indicates the information of the ability.
     * @param abilityInfo Indicates the obtained AbilityInfo object.
     * @return Returns true if the AbilityInfo is successfully obtained; returns false otherwise.
     */
    virtual bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo) = 0;
    /**
     * @brief Query the AbilityInfo by ability.uri in config.json.
     * @param abilityUri Indicates the uri of the ability.
     * @param abilityInfo Indicates the obtained AbilityInfo object.
     * @return Returns true if the AbilityInfo is successfully obtained; returns false otherwise.
     */
    virtual bool QueryAbilityInfoByUri(const std::string &abilityUri, AbilityInfo &abilityInfo) = 0;
    /**
     * @brief Obtains the BundleInfo of all keep-alive applications in the system.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @return Returns true if the BundleInfos is successfully obtained; returns false otherwise.
     */
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo> &bundleInfos) = 0;
    /**
     * @brief Obtains the label of a specified ability.
     * @param bundleName Indicates the bundle name.
     * @param className Indicates the ability class name.
     * @return Returns the label of the ability if exist; returns empty string otherwise.
     */
    virtual std::string GetAbilityLabel(const std::string &bundleName, const std::string &className) = 0;
    /**
     * @brief Obtains information about an application bundle contained in a ohos Ability Package (HAP).
     * @param hapFilePath Indicates the absolute file path of the HAP.
     * @param flag Indicates the information contained in the BundleInfo object to be returned.
     * @param bundleInfo Indicates the obtained BundleInfo object.
     * @return Returns true if the BundleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleArchiveInfo(
        const std::string &hapFilePath, const BundleFlag flag, BundleInfo &bundleInfo) = 0;
    /**
     * @brief Obtain the HAP module info of a specific ability.
     * @param abilityInfo Indicates the ability.
     * @param hapModuleInfo Indicates the obtained HapModuleInfo object.
     * @return Returns true if the HapModuleInfo is successfully obtained; returns false otherwise.
     */
    virtual bool GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo) = 0;
    /**
     * @brief Obtains the Want for starting the main ability of an application based on the given bundle name.
     * @param bundleName Indicates the bundle name.
     * @param want Indicates the obtained launch Want object.
     * @return Returns true if the launch Want object is successfully obtained; returns false otherwise.
     */
    virtual bool GetLaunchWantForBundle(const std::string &bundleName, Want &want) = 0;
    /**
     * @brief Checks whether the publickeys of two bundles are the same.
     * @param firstBundleName Indicates the first bundle name.
     * @param secondBundleName Indicates the second bundle name.
     * @return Returns SIGNATURE_UNKNOWN_BUNDLE if at least one of the given bundles is not found;
     *         returns SIGNATURE_NOT_MATCHED if their publickeys are different;
     *         returns SIGNATURE_MATCHED if their publickeys are the same.
     */
    virtual int CheckPublicKeys(const std::string &firstBundleName, const std::string &secondBundleName) = 0;
    /**
     * @brief Checks whether a specified bundle has been granted a specific permission.
     * @param bundleName Indicates the name of the bundle to check.
     * @param permission Indicates the permission to check.
     * @return Returns 0 if the bundle has the permission; returns -1 otherwise.
     */
    virtual int CheckPermission(const std::string &bundleName, const std::string &permission) = 0;
    /**
     * @brief Obtains detailed information about a specified permission.
     * @param permissionName Indicates the name of the ohos permission.
     * @param permissionDef Indicates the object containing detailed information about the given ohos permission.
     * @return Returns true if the PermissionDef object is successfully obtained; returns false otherwise.
     */
    virtual bool GetPermissionDef(const std::string &permissionName, PermissionDef &permissionDef) = 0;
    /**
     * @brief Obtains all known permission groups in the system.
     * @param permissionDefs Indicates the list of objects containing the permission group information.
     * @return Returns true if the PermissionDef objects is successfully obtained; returns false otherwise.
     */
    virtual bool GetAllPermissionGroupDefs(std::vector<PermissionDef> &permissionDefs) = 0;
    /**
     * @brief Obtains all known permission groups in the system.
     * @param permissions Indicates the permission array.
     * @param appNames Indicates the list of application names that have the specified permissions.
     * @return Returns true if the application names is successfully obtained; returns false otherwise.
     */
    virtual bool GetAppsGrantedPermissions(
        const std::vector<std::string> &permissions, std::vector<std::string> &appNames) = 0;
    /**
     * @brief Checks whether the system has a specified capability.
     * @param capName Indicates the name of the system feature to check.
     * @return Returns true if the given feature specified by name is available in the system; returns false otherwise.
     */
    virtual bool HasSystemCapability(const std::string &capName) = 0;
    /**
     * @brief Obtains the capabilities that are available in the system.
     * @param systemCaps Indicates the list of capabilities available in the system.
     * @return Returns true if capabilities in the system are successfully obtained; returns false otherwise.
     */
    virtual bool GetSystemAvailableCapabilities(std::vector<std::string> &systemCaps) = 0;
    /**
     * @brief Checks whether the current device has been started in safe mode.
     * @return Returns true if the device is in safe mode; returns false otherwise.
     */
    virtual bool IsSafeMode() = 0;
    /**
     * @brief Clears cache data of a specified application.
     * @param bundleName Indicates the bundle name of the application whose cache data is to be cleared.
     * @param cleanCacheCallback Indicates the callback to be invoked for returning the operation result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool CleanBundleCacheFiles(
        const std::string &bundleName, const sptr<ICleanCacheCallback> &cleanCacheCallback) = 0;
    /**
     * @brief Clears application running data of a specified application.
     * @param bundleName Indicates the bundle name of the application whose data is to be cleared.
     * @return Returns true if the data cleared successfully; returns false otherwise.
     */
    virtual bool CleanBundleDataFiles(const std::string &bundleName) = 0;
    /**
     * @brief Register the specific bundle status callback.
     * @param bundleStatusCallback Indicates the callback to be invoked for returning the bundle status changed result.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool RegisterBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) = 0;
    /**
     * @brief Clear the specific bundle status callback.
     * @param bundleStatusCallback Indicates the callback to be cleared.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool ClearBundleStatusCallback(const sptr<IBundleStatusCallback> &bundleStatusCallback) = 0;
    /**
     * @brief Unregister all the callbacks of status changed.
     * @return Returns true if this function is successfully called; returns false otherwise.
     */
    virtual bool UnregisterBundleStatusCallback() = 0;
    /**
     * @brief Dump the bundle informations with specific flags.
     * @param flag Indicates the information contained in the dump result.
     * @param bundleName Indicates the bundle name if needed.
     * @param result Indicates the dump information result.
     * @return Returns true if the dump result is successfully obtained; returns false otherwise.
     */
    virtual bool DumpInfos(const DumpFlag flag, const std::string &bundleName, std::string &result) = 0;
    /**
     * @brief Checks whether a specified application is enabled.
     * @param bundleName Indicates the bundle name of the application.
     * @return Returns true if the application is enabled; returns false otherwise.
     */
    virtual bool IsApplicationEnabled(const std::string &bundleName) = 0;
    /**
     * @brief Sets whether to enable a specified application.
     * @param bundleName Indicates the bundle name of the application.
     * @param isEnable Specifies whether to enable the application.
     *                 The value true means to enable it, and the value false means to disable it.
     * @return Returns true if the application is enabled; returns false otherwise.
     */
    virtual bool SetApplicationEnabled(const std::string &bundleName, bool isEnable) = 0;
    /**
     * @brief Obtains the interface used to install and uninstall bundles.
     * @return Returns a pointer to IBundleInstaller class if exist; returns nullptr otherwise.
     */
    virtual sptr<IBundleInstaller> GetBundleInstaller() = 0;

    /**
     * @brief Requests a certain permission from user.
     * @param bundleName Indicates the name of the bundle to request permission.
     * @param permission Indicates the permission to request permission.
     * @param userId Indicates the user id.
     * @return Returns true if the permission request successfully; returns false otherwise.
     */
    virtual bool RequestPermissionFromUser(
        const std::string &bundleName, const std::string &permission, const int userId)
    {
        return true;
    }

    enum class Message {
        GET_APPLICATION_INFO,
        GET_APPLICATION_INFOS,
        GET_BUNDLE_INFO,
        GET_BUNDLE_INFOS,
        GET_UID_BY_BUNDLE_NAME,
        GET_BUNDLE_NAME_FOR_UID,
        GET_BUNDLE_GIDS,
        GET_APP_TYPE,
        CHECK_IS_SYSTEM_APP_BY_UID,
        GET_BUNDLE_INFOS_BY_METADATA,
        QUERY_ABILITY_INFO,
        QUERY_ABILITY_INFO_BY_URI,
        QUERY_KEEPALIVE_BUNDLE_INFOS,
        GET_ABILITY_LABEL,
        GET_BUNDLE_ARCHIVE_INFO,
        GET_HAP_MODULE_INFO,
        GET_LAUNCH_WANT_FOR_BUNDLE,
        CHECK_PUBLICKEYS,
        CHECK_PERMISSION,
        GET_PERMISSION_DEF,
        GET_ALL_PERMISSION_GROUP_DEFS,
        GET_APPS_GRANTED_PERMISSIONS,
        HAS_SYSTEM_CAPABILITY,
        GET_SYSTEM_AVAILABLE_CAPABILITIES,
        IS_SAFE_MODE,
        CLEAN_BUNDLE_CACHE_FILES,
        CLEAN_BUNDLE_DATA_FILES,
        REGISTER_BUNDLE_STATUS_CALLBACK,
        CLEAR_BUNDLE_STATUS_CALLBACK,
        UNREGISTER_BUNDLE_STATUS_CALLBACK,
        DUMP_INFOS,
        IS_APPLICATION_ENABLED,
        SET_APPLICATION_ENABLED,
        GET_BUNDLE_INSTALLER,
    };
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_CORE_INCLUDE_BUNDLEMGR_BUNDLE_MGR_INTERFACE_H
