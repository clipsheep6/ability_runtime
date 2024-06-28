/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "inner_bundle_info.h"

#include <algorithm>
#include <deque>
#include <regex>
#include <unistd.h>

#include "common_profile.h"
#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string APP_TYPE = "appType";
const std::string UID = "uid";
const std::string GID = "gid";
const std::string BUNDLE_STATUS = "bundleStatus";
const std::string BASE_APPLICATION_INFO = "baseApplicationInfo";
const std::string BASE_BUNDLE_INFO = "baseBundleInfo";
const std::string BASE_ABILITY_INFO = "baseAbilityInfos";
const std::string INNER_MODULE_INFO = "innerModuleInfos";
const std::string USER_ID = "userId_";
const std::string APP_FEATURE = "appFeature";
const std::string CAN_UNINSTALL = "canUninstall";
const std::string NAME = "name";
const std::string MODULE_PACKAGE = "modulePackage";
const std::string MODULE_PATH = "modulePath";
const std::string MODULE_NAME = "moduleName";
const std::string MODULE_DESCRIPTION = "description";
const std::string MODULE_DESCRIPTION_ID = "descriptionId";
const std::string MODULE_ICON = "icon";
const std::string MODULE_ICON_ID = "iconId";
const std::string MODULE_LABEL = "label";
const std::string MODULE_LABEL_ID = "labelId";
const std::string MODULE_DESCRIPTION_INSTALLATION_FREE = "installationFree";
const std::string MODULE_IS_REMOVABLE = "isRemovable";
const std::string MODULE_UPGRADE_FLAG = "upgradeFlag";
const std::string MODULE_IS_ENTRY = "isEntry";
const std::string MODULE_METADATA = "metaData";
const std::string MODULE_COLOR_MODE = "colorMode";
const std::string MODULE_DISTRO = "distro";
const std::string MODULE_REQ_CAPABILITIES = "reqCapabilities";
const std::string MODULE_DATA_DIR = "moduleDataDir";
const std::string MODULE_RES_PATH = "moduleResPath";
const std::string MODULE_HAP_PATH = "hapPath";
const std::string MODULE_ABILITY_KEYS = "abilityKeys";
const std::string MODULE_MAIN_ABILITY = "mainAbility";
const std::string MODULE_ENTRY_ABILITY_KEY = "entryAbilityKey";
const std::string MODULE_DEPENDENCIES = "dependencies";
const std::string MODULE_IS_LIB_ISOLATED = "isLibIsolated";
const std::string MODULE_NATIVE_LIBRARY_PATH = "nativeLibraryPath";
const std::string MODULE_CPU_ABI = "cpuAbi";
const std::string NEW_BUNDLE_NAME = "newBundleName";
const std::string MODULE_SRC_PATH = "srcPath";
const std::string MODULE_HASH_VALUE = "hashValue";
const std::string SCHEME_SEPARATOR = "://";
const std::string PORT_SEPARATOR = ":";
const std::string PATH_SEPARATOR = "/";
const std::string PARAM_SEPARATOR = "?";
const std::string INSTALL_MARK = "installMark";
const std::string TYPE_WILDCARD = "*/*";
const std::string MODULE_PROCESS = "process";
const std::string MODULE_SRC_ENTRANCE = "srcEntrance";
const std::string MODULE_DEVICE_TYPES = "deviceTypes";
const std::string MODULE_VIRTUAL_MACHINE = "virtualMachine";
const std::string MODULE_UI_SYNTAX = "uiSyntax";
const std::string MODULE_PAGES = "pages";
const std::string MODULE_META_DATA = "metadata";
const std::string MODULE_EXTENSION_KEYS = "extensionKeys";
const std::string MODULE_IS_MODULE_JSON = "isModuleJson";
const std::string MODULE_IS_STAGE_BASED_MODEL = "isStageBasedModel";
const std::string BUNDLE_IS_NEW_VERSION = "isNewVersion";
const std::string BUNDLE_IS_NEED_UPDATE = "upgradeFlag";
const std::string BUNDLE_BASE_EXTENSION_INFOS = "baseExtensionInfos";
const std::string ALLOWED_ACLS = "allowedAcls";
const std::string META_DATA_SHORTCUTS_NAME = "ohos.ability.shortcuts";
const std::string APP_INDEX = "appIndex";
const std::string BUNDLE_IS_SANDBOX_APP = "isSandboxApp";
const std::string BUNDLE_SANDBOX_PERSISTENT_INFO = "sandboxPersistentInfo";
const std::string MODULE_COMPILE_MODE = "compileMode";
const std::string BUNDLE_HQF_INFOS = "hqfInfos";
const std::string MODULE_TARGET_MODULE_NAME = "targetModuleName";
const std::string MODULE_TARGET_PRIORITY = "targetPriority";
const std::string MODULE_ATOMIC_SERVICE_MODULE_TYPE = "atomicServiceModuleType";
const std::string MODULE_PRELOADS = "preloads";
const std::string MODULE_BUNDLE_TYPE = "bundleType";
const std::string MODULE_VERSION_CODE = "versionCode";
const std::string MODULE_VERSION_NAME = "versionName";
const std::string MODULE_PROXY_DATAS = "proxyDatas";
const std::string MODULE_BUILD_HASH = "buildHash";
const std::string MODULE_ISOLATION_MODE = "isolationMode";
const std::string MODULE_COMPRESS_NATIVE_LIBS = "compressNativeLibs";
const std::string MODULE_NATIVE_LIBRARY_FILE_NAMES = "nativeLibraryFileNames";
const std::string MODULE_AOT_COMPILE_STATUS = "aotCompileStatus";
const std::string DATA_GROUP_INFOS = "dataGroupInfos";
const std::map<std::string, IsolationMode> ISOLATION_MODE_MAP = {
    {"isolationOnly", IsolationMode::ISOLATION_ONLY},
    {"nonisolationOnly", IsolationMode::NONISOLATION_ONLY},
    {"isolationFirst", IsolationMode::ISOLATION_FIRST},
};
const std::string NATIVE_LIBRARY_PATH_SYMBOL = "!/";

const std::string STR_PHONE = "phone";
const std::string STR_DEFAULT = "default";

inline CompileMode ConvertCompileMode(const std::string &compileMode)
{
    if (compileMode == Profile::COMPILE_MODE_ES_MODULE) {
        return CompileMode::ES_MODULE;
    } else {
        return CompileMode::JS_BUNDLE;
    }
}
} // namespace

InnerBundleInfo::InnerBundleInfo()
{
    baseApplicationInfo_ = std::make_shared<ApplicationInfo>();
    if (baseApplicationInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "baseApplicationInfo_ is nullptr, create failed");
    }
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "inner bundle info instance is created");
}

InnerBundleInfo &InnerBundleInfo::operator=(const InnerBundleInfo &info)
{
    if (this == &info) {
        return *this;
    }
    this->appType_ = info.appType_;
    this->userId_ = info.userId_;
    this->bundleStatus_ = info.bundleStatus_;
    this->appFeature_ = info.appFeature_;
    this->allowedAcls_ = info.allowedAcls_;
    this->appIndex_ = info.appIndex_;
    this->isSandboxApp_ = info.isSandboxApp_;
    this->currentPackage_ = info.currentPackage_;
    this->onlyCreateBundleUser_ = info.onlyCreateBundleUser_;
    this->innerModuleInfos_ = info.innerModuleInfos_;
    this->baseAbilityInfos_ = info.baseAbilityInfos_;
    this->isNewVersion_ = info.isNewVersion_;
    this->baseExtensionInfos_= info.baseExtensionInfos_;
    this->baseApplicationInfo_ = std::make_shared<ApplicationInfo>();
    if (info.baseApplicationInfo_ != nullptr) {
        *(this->baseApplicationInfo_) = *(info.baseApplicationInfo_);
    }
    this->provisionMetadatas_ = info.provisionMetadatas_;
    return *this;
}

InnerBundleInfo::~InnerBundleInfo()
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "inner bundle info instance is destroyed");
}

void to_json(nlohmann::json &jsonObject, const Distro &distro)
{
    jsonObject = nlohmann::json {
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL, distro.deliveryWithInstall},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME, distro.moduleName},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE, distro.moduleType},
            {ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE, distro.installationFree}
    };
}

void to_json(nlohmann::json &jsonObject, const Dependency &dependency)
{
    jsonObject = nlohmann::json {
        {Profile::DEPENDENCIES_MODULE_NAME, dependency.moduleName},
        {Profile::DEPENDENCIES_BUNDLE_NAME, dependency.bundleName},
        {Profile::APP_VERSION_CODE, dependency.versionCode}
    };
}

void to_json(nlohmann::json &jsonObject, const InnerModuleInfo &info)
{
    jsonObject = nlohmann::json {
        {NAME, info.name},
        {MODULE_PACKAGE, info.modulePackage},
        {MODULE_NAME, info.moduleName},
        {MODULE_PATH, info.modulePath},
        {MODULE_DATA_DIR, info.moduleDataDir},
        {MODULE_RES_PATH, info.moduleResPath},
        {MODULE_IS_ENTRY, info.isEntry},
        {MODULE_METADATA, info.metaData},
        {MODULE_COLOR_MODE, info.colorMode},
        {MODULE_DISTRO, info.distro},
        {MODULE_DESCRIPTION, info.description},
        {MODULE_DESCRIPTION_ID, info.descriptionId},
        {MODULE_ICON, info.icon},
        {MODULE_ICON_ID, info.iconId},
        {MODULE_LABEL, info.label},
        {MODULE_LABEL_ID, info.labelId},
        {MODULE_DESCRIPTION_INSTALLATION_FREE, info.installationFree},
        {MODULE_IS_REMOVABLE, info.isRemovable},
        {MODULE_UPGRADE_FLAG, info.upgradeFlag},
        {MODULE_REQ_CAPABILITIES, info.reqCapabilities},
        {MODULE_ABILITY_KEYS, info.abilityKeys},
        {MODULE_MAIN_ABILITY, info.mainAbility},
        {MODULE_ENTRY_ABILITY_KEY, info.entryAbilityKey},
        {MODULE_SRC_PATH, info.srcPath},
        {MODULE_HASH_VALUE, info.hashValue},
        {MODULE_PROCESS, info.process},
        {MODULE_SRC_ENTRANCE, info.srcEntrance},
        {MODULE_DEVICE_TYPES, info.deviceTypes},
        {MODULE_VIRTUAL_MACHINE, info.virtualMachine},
        {MODULE_UI_SYNTAX, info.uiSyntax},
        {MODULE_PAGES, info.pages},
        {MODULE_META_DATA, info.metadata},
        {MODULE_EXTENSION_KEYS, info.extensionKeys},
        {MODULE_IS_MODULE_JSON, info.isModuleJson},
        {MODULE_IS_STAGE_BASED_MODEL, info.isStageBasedModel},
        {MODULE_DEPENDENCIES, info.dependencies},
        {MODULE_IS_LIB_ISOLATED, info.isLibIsolated},
        {MODULE_NATIVE_LIBRARY_PATH, info.nativeLibraryPath},
        {MODULE_CPU_ABI, info.cpuAbi},
        {MODULE_HAP_PATH, info.hapPath},
        {MODULE_COMPILE_MODE, info.compileMode},
        {MODULE_TARGET_MODULE_NAME, info.targetModuleName},
        {MODULE_TARGET_PRIORITY, info.targetPriority},
        {MODULE_ATOMIC_SERVICE_MODULE_TYPE, info.atomicServiceModuleType},
        {MODULE_PRELOADS, info.preloads},
        {MODULE_BUNDLE_TYPE, info.bundleType},
        {MODULE_VERSION_CODE, info.versionCode},
        {MODULE_VERSION_NAME, info.versionName},
        {MODULE_PROXY_DATAS, info.proxyDatas},
        {MODULE_BUILD_HASH, info.buildHash},
        {MODULE_ISOLATION_MODE, info.isolationMode},
        {MODULE_COMPRESS_NATIVE_LIBS, info.compressNativeLibs},
        {MODULE_NATIVE_LIBRARY_FILE_NAMES, info.nativeLibraryFileNames},
        {MODULE_AOT_COMPILE_STATUS, info.aotCompileStatus},
    };
}

void InnerBundleInfo::ToJson(nlohmann::json &jsonObject) const
{
    jsonObject[APP_TYPE] = appType_;
    jsonObject[BUNDLE_STATUS] = bundleStatus_;
    jsonObject[ALLOWED_ACLS] = allowedAcls_;
    jsonObject[BASE_APPLICATION_INFO] = *baseApplicationInfo_;
    jsonObject[BASE_ABILITY_INFO] = baseAbilityInfos_;
    jsonObject[INNER_MODULE_INFO] = innerModuleInfos_;
    jsonObject[USER_ID] = userId_;
    jsonObject[APP_FEATURE] = appFeature_;
    jsonObject[BUNDLE_IS_NEW_VERSION] = isNewVersion_;
    jsonObject[BUNDLE_BASE_EXTENSION_INFOS] = baseExtensionInfos_;
    jsonObject[APP_INDEX] = appIndex_;
    jsonObject[BUNDLE_IS_SANDBOX_APP] = isSandboxApp_;
}

void from_json(const nlohmann::json &jsonObject, InnerModuleInfo &info)
{
    // these are not required fields.
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        NAME,
        info.name,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PACKAGE,
        info.modulePackage,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_NAME,
        info.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PATH,
        info.modulePath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_DATA_DIR,
        info.moduleDataDir,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_HAP_PATH,
        info.hapPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_RES_PATH,
        info.moduleResPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_ENTRY,
        info.isEntry,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<MetaData>(jsonObject,
        jsonObjectEnd,
        MODULE_METADATA,
        info.metaData,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ModuleColorMode>(jsonObject,
        jsonObjectEnd,
        MODULE_COLOR_MODE,
        info.colorMode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Distro>(jsonObject,
        jsonObjectEnd,
        MODULE_DISTRO,
        info.distro,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION,
        info.description,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION_ID,
        info.descriptionId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_ICON,
        info.icon,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_ICON_ID,
        info.iconId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_LABEL,
        info.label,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_LABEL_ID,
        info.labelId,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_MAIN_ABILITY,
        info.mainAbility,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_ENTRY_ABILITY_KEY,
        info.entryAbilityKey,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_SRC_PATH,
        info.srcPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_HASH_VALUE,
        info.hashValue,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_DESCRIPTION_INSTALLATION_FREE,
        info.installationFree,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, bool>>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_REMOVABLE,
        info.isRemovable,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_UPGRADE_FLAG,
        info.upgradeFlag,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_REQ_CAPABILITIES,
        info.reqCapabilities,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_ABILITY_KEYS,
        info.abilityKeys,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PROCESS,
        info.process,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_SRC_ENTRANCE,
        info.srcEntrance,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_DEVICE_TYPES,
        info.deviceTypes,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_VIRTUAL_MACHINE,
        info.virtualMachine,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_UI_SYNTAX,
        info.uiSyntax,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_PAGES,
        info.pages,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject,
        jsonObjectEnd,
        MODULE_META_DATA,
        info.metadata,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_EXTENSION_KEYS,
        info.extensionKeys,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_MODULE_JSON,
        info.isModuleJson,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_STAGE_BASED_MODEL,
        info.isStageBasedModel,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Dependency>>(jsonObject,
        jsonObjectEnd,
        MODULE_DEPENDENCIES,
        info.dependencies,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_COMPILE_MODE,
        info.compileMode,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_IS_LIB_ISOLATED,
        info.isLibIsolated,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_NATIVE_LIBRARY_PATH,
        info.nativeLibraryPath,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_CPU_ABI,
        info.cpuAbi,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_TARGET_MODULE_NAME,
        info.targetModuleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_TARGET_PRIORITY,
        info.targetPriority,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<AtomicServiceModuleType>(jsonObject,
        jsonObjectEnd,
        MODULE_ATOMIC_SERVICE_MODULE_TYPE,
        info.atomicServiceModuleType,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_PRELOADS,
        info.preloads,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<BundleType>(jsonObject,
        jsonObjectEnd,
        MODULE_BUNDLE_TYPE,
        info.bundleType,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject,
        jsonObjectEnd,
        MODULE_VERSION_CODE,
        info.versionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_VERSION_NAME,
        info.versionName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<ProxyData>>(jsonObject,
        jsonObjectEnd,
        MODULE_PROXY_DATAS,
        info.proxyDatas,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::OBJECT);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_BUILD_HASH,
        info.buildHash,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        MODULE_ISOLATION_MODE,
        info.isolationMode,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        MODULE_COMPRESS_NATIVE_LIBS,
        info.compressNativeLibs,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        MODULE_NATIVE_LIBRARY_FILE_NAMES,
        info.nativeLibraryFileNames,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<AOTCompileStatus>(jsonObject,
        jsonObjectEnd,
        MODULE_AOT_COMPILE_STATUS,
        info.aotCompileStatus,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        TAG_LOGE(
            AAFwkTag::ABILITY_SIM, "read InnerModuleInfo from database error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json &jsonObject, Dependency &dependency)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        Profile::DEPENDENCIES_MODULE_NAME,
        dependency.moduleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        Profile::DEPENDENCIES_BUNDLE_NAME,
        dependency.bundleName,
        JsonType::STRING,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        Profile::APP_VERSION_CODE,
        dependency.versionCode,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Dependency from_json error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json &jsonObject, Distro &distro)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "from_json start.");
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL,
        distro.deliveryWithInstall,
        JsonType::BOOLEAN,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME,
        distro.moduleName,
        JsonType::STRING,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE,
        distro.moduleType,
        JsonType::STRING,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    // mustFlag decide by distro.moduleType
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        ProfileReader::BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE,
        distro.installationFree,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Distro from_json error, error code : %{public}d", parseResult);
    }
}

int32_t InnerBundleInfo::FromJson(const nlohmann::json &jsonObject)
{
    const auto &jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<Constants::AppType>(jsonObject,
        jsonObjectEnd,
        APP_TYPE,
        appType_,
        JsonType::NUMBER,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        ALLOWED_ACLS,
        allowedAcls_,
        JsonType::ARRAY,
        false,
        parseResult,
        ArrayType::STRING);
    GetValueIfFindKey<BundleStatus>(jsonObject,
        jsonObjectEnd,
        BUNDLE_STATUS,
        bundleStatus_,
        JsonType::NUMBER,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ApplicationInfo>(jsonObject,
        jsonObjectEnd,
        BASE_APPLICATION_INFO,
        *baseApplicationInfo_,
        JsonType::OBJECT,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, AbilityInfo>>(jsonObject,
        jsonObjectEnd,
        BASE_ABILITY_INFO,
        baseAbilityInfos_,
        JsonType::OBJECT,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, InnerModuleInfo>>(jsonObject,
        jsonObjectEnd,
        INNER_MODULE_INFO,
        innerModuleInfos_,
        JsonType::OBJECT,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        USER_ID,
        userId_,
        JsonType::NUMBER,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject,
        jsonObjectEnd,
        APP_FEATURE,
        appFeature_,
        JsonType::STRING,
        true,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_IS_NEW_VERSION,
        isNewVersion_,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, ExtensionAbilityInfo>>(jsonObject,
        jsonObjectEnd,
        BUNDLE_BASE_EXTENSION_INFOS,
        baseExtensionInfos_,
        JsonType::OBJECT,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject,
        jsonObjectEnd,
        APP_INDEX,
        appIndex_,
        JsonType::NUMBER,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject,
        jsonObjectEnd,
        BUNDLE_IS_SANDBOX_APP,
        isSandboxApp_,
        JsonType::BOOLEAN,
        false,
        parseResult,
        ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        TAG_LOGE(
            AAFwkTag::ABILITY_SIM, "read InnerBundleInfo from database error, error code : %{public}d", parseResult);
    }
    return parseResult;
}

std::optional<HapModuleInfo> InnerBundleInfo::FindHapModuleInfo(const std::string &modulePackage, int32_t userId) const
{
    auto it = innerModuleInfos_.find(modulePackage);
    if (it == innerModuleInfos_.end()) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "can not find module %{public}s", modulePackage.c_str());
        return std::nullopt;
    }
    HapModuleInfo hapInfo;
    hapInfo.name = it->second.name;
    hapInfo.package = it->second.modulePackage;
    hapInfo.moduleName = it->second.moduleName;
    hapInfo.description = it->second.description;
    hapInfo.descriptionId = it->second.descriptionId;
    hapInfo.label = it->second.label;
    hapInfo.labelId = it->second.labelId;
    hapInfo.iconPath = it->second.icon;
    hapInfo.iconId = it->second.iconId;
    hapInfo.mainAbility = it->second.mainAbility;
    hapInfo.srcPath = it->second.srcPath;
    hapInfo.hapPath = it->second.hapPath;
    hapInfo.supportedModes = baseApplicationInfo_->supportedModes;
    hapInfo.reqCapabilities = it->second.reqCapabilities;
    hapInfo.colorMode = it->second.colorMode;
    hapInfo.isRemovable = it->second.isRemovable;
    hapInfo.upgradeFlag = it->second.upgradeFlag;
    hapInfo.isLibIsolated = it->second.isLibIsolated;
    hapInfo.nativeLibraryPath = it->second.nativeLibraryPath;
    hapInfo.cpuAbi = it->second.cpuAbi;

    hapInfo.bundleName = baseApplicationInfo_->bundleName;
    hapInfo.mainElementName = it->second.mainAbility;
    hapInfo.pages = it->second.pages;
    hapInfo.process = it->second.process;
    hapInfo.resourcePath = it->second.moduleResPath;
    hapInfo.srcEntrance = it->second.srcEntrance;
    hapInfo.uiSyntax = it->second.uiSyntax;
    hapInfo.virtualMachine = it->second.virtualMachine;
    hapInfo.deliveryWithInstall = it->second.distro.deliveryWithInstall;
    hapInfo.installationFree = it->second.distro.installationFree;
    hapInfo.isModuleJson = it->second.isModuleJson;
    hapInfo.isStageBasedModel = it->second.isStageBasedModel;
    std::string moduleType = it->second.distro.moduleType;
    if (moduleType == Profile::MODULE_TYPE_ENTRY) {
        hapInfo.moduleType = ModuleType::ENTRY;
    } else if (moduleType == Profile::MODULE_TYPE_FEATURE) {
        hapInfo.moduleType = ModuleType::FEATURE;
    } else if (moduleType == Profile::MODULE_TYPE_SHARED) {
        hapInfo.moduleType = ModuleType::SHARED;
    } else {
        hapInfo.moduleType = ModuleType::UNKNOWN;
    }
    std::string key;
    key.append(".").append(modulePackage).append(".");
    for (const auto &extension : baseExtensionInfos_) {
        if (extension.first.find(key) != std::string::npos) {
            hapInfo.extensionInfos.emplace_back(extension.second);
        }
    }
    hapInfo.metadata = it->second.metadata;
    bool first = false;
    for (auto &ability : baseAbilityInfos_) {
        if (ability.second.name == Constants::APP_DETAIL_ABILITY) {
            continue;
        }
        if (ability.first.find(key) != std::string::npos) {
            if (!first) {
                hapInfo.deviceTypes = ability.second.deviceTypes;
                first = true;
            }
            auto &abilityInfo = hapInfo.abilityInfos.emplace_back(ability.second);
            GetApplicationInfo(ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMISSION |
                ApplicationFlag::GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT, userId,
                abilityInfo.applicationInfo);
        }
    }
    hapInfo.dependencies = it->second.dependencies;
    hapInfo.compileMode = ConvertCompileMode(it->second.compileMode);
    hapInfo.atomicServiceModuleType = it->second.atomicServiceModuleType;
    for (const auto &item : it->second.preloads) {
        PreloadItem preload(item);
        hapInfo.preloads.emplace_back(preload);
    }
    for (const auto &item : it->second.proxyDatas) {
        ProxyData proxyData(item);
        hapInfo.proxyDatas.emplace_back(proxyData);
    }
    hapInfo.buildHash = it->second.buildHash;
    hapInfo.isolationMode = GetIsolationMode(it->second.isolationMode);
    hapInfo.compressNativeLibs = it->second.compressNativeLibs;
    hapInfo.nativeLibraryFileNames = it->second.nativeLibraryFileNames;
    hapInfo.aotCompileStatus = it->second.aotCompileStatus;
    return hapInfo;
}

std::optional<AbilityInfo> InnerBundleInfo::FindAbilityInfo(
    const std::string &moduleName,
    const std::string &abilityName,
    int32_t userId) const
{
    for (const auto &ability : baseAbilityInfos_) {
        auto abilityInfo = ability.second;
        if ((abilityInfo.name == abilityName) &&
            (moduleName.empty() || (abilityInfo.moduleName == moduleName))) {
            GetApplicationInfo(ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMISSION |
                ApplicationFlag::GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT, userId,
                abilityInfo.applicationInfo);
            return abilityInfo;
        }
    }

    return std::nullopt;
}

ErrCode InnerBundleInfo::FindAbilityInfo(
    const std::string &moduleName, const std::string &abilityName, AbilityInfo &info) const
{
    bool isModuleFind = false;
    for (const auto &ability : baseAbilityInfos_) {
        auto abilityInfo = ability.second;
        if ((abilityInfo.moduleName == moduleName)) {
            isModuleFind = true;
            if (abilityInfo.name == abilityName) {
                info = abilityInfo;
                return ERR_OK;
            }
        }
    }
    TAG_LOGE(AAFwkTag::ABILITY_SIM,
        "bundleName:%{public}s not find moduleName:%{public}s, abilityName:%{public}s, isModuleFind:%{public}d",
        GetBundleName().c_str(), moduleName.c_str(), abilityName.c_str(), isModuleFind);
    if (isModuleFind) {
        return ERR_BUNDLE_MANAGER_ABILITY_NOT_EXIST;
    } else {
        return ERR_BUNDLE_MANAGER_MODULE_NOT_EXIST;
    }
}

std::string InnerBundleInfo::ToString() const
{
    nlohmann::json j;
    ToJson(j);
    return j.dump();
}

void InnerBundleInfo::GetApplicationInfo(int32_t flags, int32_t userId, ApplicationInfo &appInfo) const
{
    appInfo = *baseApplicationInfo_;

    for (const auto &info : innerModuleInfos_) {
        bool deCompress = info.second.hapPath.empty();
        ModuleInfo moduleInfo;
        moduleInfo.moduleName = info.second.moduleName;
        if (deCompress) {
            moduleInfo.moduleSourceDir = info.second.modulePath;
            appInfo.moduleSourceDirs.emplace_back(info.second.modulePath);
        }
        moduleInfo.preloads = info.second.preloads;
        appInfo.moduleInfos.emplace_back(moduleInfo);
        if (deCompress && info.second.isEntry) {
            appInfo.entryDir = info.second.modulePath;
        }
        if ((static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_METADATA) == GET_APPLICATION_INFO_WITH_METADATA) {
            bool isModuleJson = info.second.isModuleJson;
            if (!isModuleJson && info.second.metaData.customizeData.size() > 0) {
                appInfo.metaData[info.second.moduleName] = info.second.metaData.customizeData;
            }
            if (isModuleJson && info.second.metadata.size() > 0) {
                appInfo.metadata[info.second.moduleName] = info.second.metadata;
            }
        }
        if ((static_cast<uint32_t>(flags) & GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT) !=
            GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT) {
            appInfo.fingerprint.clear();
        }
    }
    if (!appInfo.permissions.empty()) {
        RemoveDuplicateName(appInfo.permissions);
    }
}

void InnerBundleInfo::RemoveDuplicateName(std::vector<std::string> &name) const
{
    std::sort(name.begin(), name.end());
    auto iter = std::unique(name.begin(), name.end());
    name.erase(iter, name.end());
}

IsolationMode InnerBundleInfo::GetIsolationMode(const std::string &isolationMode) const
{
    auto isolationModeRes = ISOLATION_MODE_MAP.find(isolationMode);
    if (isolationModeRes != ISOLATION_MODE_MAP.end()) {
        return isolationModeRes->second;
    } else {
        return IsolationMode::NONISOLATION_FIRST;
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
