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
#include "common_func.h"

#include <vector>
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t NAPI_RETURN_ZERO = 0;
constexpr int32_t NAPI_RETURN_ONE = 1;
constexpr const char *BUNDLE_NAME = "bundleName";
constexpr const char *MODULE_NAME = "moduleName";
constexpr const char *URI = "uri";
constexpr const char *NAME = "name";
constexpr const char *IS_VISIBLE = "isVisible";
constexpr const char *EXPORTED = "exported";
constexpr const char *PERMISSIONS = "permissions";
constexpr const char *META_DATA = "metadata";
constexpr const char *ENABLED = "enabled";
constexpr const char *READ_PERMISSION = "readPermission";
constexpr const char *WRITE_PERMISSION = "writePermission";
constexpr const char *LABEL = "label";
constexpr const char *LABEL_ID = "labelId";
constexpr const char *DESCRIPTION = "description";
constexpr const char *DESCRIPTION_ID = "descriptionId";
constexpr const char *ICON = "icon";
constexpr const char *ICON_ID = "iconId";
constexpr const char *APPLICATION_INFO = "applicationInfo";
constexpr const char *DEBUG = "debug";
}

bool CommonFunc::ParsePropertyArray(napi_env env, napi_value args, const std::string &propertyName,
    std::vector<napi_value> &valueVec)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &type), false);
    if (type != napi_object) {
        return false;
    }

    bool hasKey = false;
    napi_has_named_property(env, args, propertyName.c_str(), &hasKey);
    if (!hasKey) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "%{public}s is not existed", propertyName.c_str());
        return true;
    }
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, args, propertyName.c_str(), &property);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "get name property failed, propertyName: %{public}s", propertyName.c_str());
        return false;
    }
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, property, &isArray), false);
    if (!isArray) {
        return false;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, property, &arrayLength), false);

    napi_value valueAry = 0;
    for (uint32_t j = 0; j < arrayLength; j++) {
        NAPI_CALL_BASE(env, napi_get_element(env, property, j, &valueAry), false);
        valueVec.emplace_back(valueAry);
    }
    return true;
}

void CommonFunc::ConvertExtensionInfos(napi_env env, const std::vector<ExtensionAbilityInfo> &extensionInfos,
    napi_value value)
{
    for (size_t index = 0; index < extensionInfos.size(); ++index) {
        napi_value objExtensionInfo = nullptr;
        napi_create_object(env, &objExtensionInfo);
        ConvertExtensionInfo(env, extensionInfos[index], objExtensionInfo);
        napi_set_element(env, value, index, objExtensionInfo);
    }
}

void CommonFunc::ConvertStringArrays(napi_env env, const std::vector<std::string> &strs, napi_value value)
{
    for (size_t index = 0; index < strs.size(); ++index) {
        napi_value nStr;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, strs[index].c_str(), NAPI_AUTO_LENGTH, &nStr));
        napi_set_element(env, value, index, nStr);
    }
}

void CommonFunc::ConvertExtensionInfo(napi_env env, const ExtensionAbilityInfo &extensionInfo,
    napi_value objExtensionInfo)
{
    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, extensionInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, BUNDLE_NAME, nBundleName));

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, extensionInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, MODULE_NAME, nModuleName));

    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, extensionInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, NAME, nName));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, extensionInfo.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, LABEL_ID, nLabelId));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, extensionInfo.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, DESCRIPTION_ID, nDescriptionId));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, extensionInfo.iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, ICON_ID, nIconId));

    napi_value nVisible;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, extensionInfo.visible, &nVisible));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, IS_VISIBLE, nVisible));

    napi_value nExported;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, extensionInfo.visible, &nExported));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, EXPORTED, nExported));

    napi_value nExtensionAbilityType;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, static_cast<int32_t>(extensionInfo.type), &nExtensionAbilityType));
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(env, objExtensionInfo, "extensionAbilityType", nExtensionAbilityType));

    napi_value nPermissions;
    size_t size = extensionInfo.permissions.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nPermissions));
    for (size_t i = 0; i < size; ++i) {
        napi_value permission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, extensionInfo.permissions[i].c_str(), NAPI_AUTO_LENGTH, &permission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPermissions, i, permission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, PERMISSIONS, nPermissions));

    napi_value nApplicationInfo;
    if (!extensionInfo.applicationInfo.name.empty()) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nApplicationInfo));
        ConvertApplicationInfo(env, nApplicationInfo, extensionInfo.applicationInfo);
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &nApplicationInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, APPLICATION_INFO, nApplicationInfo));

    napi_value nMetadata;
    size = extensionInfo.metadata.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nMetadata));
    for (size_t i = 0; i < size; ++i) {
        napi_value nMetaData;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nMetaData));
        ConvertMetadata(env, extensionInfo.metadata[i], nMetaData);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nMetadata, i, nMetaData));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, META_DATA, nMetadata));

    napi_value nEnabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, extensionInfo.enabled, &nEnabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, ENABLED, nEnabled));

    napi_value nReadPermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, extensionInfo.readPermission.c_str(), NAPI_AUTO_LENGTH, &nReadPermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, READ_PERMISSION, nReadPermission));

    napi_value nWritePermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, extensionInfo.writePermission.c_str(), NAPI_AUTO_LENGTH, &nWritePermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objExtensionInfo, WRITE_PERMISSION, nWritePermission));
}

void CommonFunc::ConvertMetadata(napi_env env, const Metadata &metadata, napi_value value)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, NAME, nName));

    napi_value nValue;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.value.c_str(), NAPI_AUTO_LENGTH, &nValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "value", nValue));

    napi_value nResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.resource.c_str(), NAPI_AUTO_LENGTH, &nResource));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "resource", nResource));
}

bool CommonFunc::ParseString(napi_env env, napi_value value, std::string &result)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_string) {
        return false;
    }
    size_t size = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, NAPI_RETURN_ZERO, &size) != napi_ok) {
        return false;
    }
    result.reserve(size + 1);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + 1), &size) != napi_ok) {
        return false;
    }
    return true;
}

bool CommonFunc::ParseAbilityInfo(napi_env env, napi_value param, AbilityInfo &abilityInfo)
{
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, param, &valueType), false);
    if (valueType != napi_object) {
        return false;
    }

    napi_value prop = nullptr;
    // parse bundleName
    napi_get_named_property(env, param, "bundleName", &prop);
    std::string bundleName;
    if (!ParseString(env, prop, bundleName)) {
        return false;
    }
    abilityInfo.bundleName = bundleName;

    // parse moduleName
    napi_get_named_property(env, param, "moduleName", &prop);
    std::string moduleName;
    if (!ParseString(env, prop, moduleName)) {
        return false;
    }
    abilityInfo.moduleName = moduleName;

    // parse abilityName
    napi_get_named_property(env, param, "name", &prop);
    std::string abilityName;
    if (!ParseString(env, prop, abilityName)) {
        return false;
    }
    abilityInfo.name = abilityName;
    return true;
}

void CommonFunc::ConvertAbilityInfos(napi_env env, const std::vector<AbilityInfo> &abilityInfos, napi_value value)
{
    for (size_t index = 0; index < abilityInfos.size(); ++index) {
        napi_value objAbilityInfo = nullptr;
        napi_create_object(env, &objAbilityInfo);
        ConvertAbilityInfo(env, abilityInfos[index], objAbilityInfo);
        napi_set_element(env, value, index, objAbilityInfo);
    }
}

void CommonFunc::ConvertPreloadItem(napi_env env, const PreloadItem &preloadItem, napi_value value)
{
    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env,
        preloadItem.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "moduleName", nModuleName));
}

void CommonFunc::ConvertDependency(napi_env env, const Dependency &dependency, napi_value value)
{
    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(
        env, dependency.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, BUNDLE_NAME, nBundleName));

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(
        env, dependency.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, MODULE_NAME, nModuleName));

    napi_value nVersionCode;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, dependency.versionCode, &nVersionCode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "versionCode", nVersionCode));
}

std::string CommonFunc::GetStringFromNAPI(napi_env env, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType != napi_string) {
        return "";
    }
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, value, nullptr, NAPI_RETURN_ZERO, &size) != napi_ok) {
        return "";
    }
    result.reserve(size + NAPI_RETURN_ONE);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), (size + NAPI_RETURN_ONE), &size) != napi_ok) {
        return "";
    }
    return result;
}

napi_value CommonFunc::ParseStringArray(napi_env env, std::vector<std::string> &stringArray, napi_value args)
{
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, args, &isArray));
    if (!isArray) {
        return nullptr;
    }
    uint32_t arrayLength = 0;
    NAPI_CALL(env, napi_get_array_length(env, args, &arrayLength));
    for (uint32_t j = 0; j < arrayLength; j++) {
        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_element(env, args, j, &value));
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, value, &valueType));
        if (valueType != napi_string) {
            stringArray.clear();
            return nullptr;
        }
        stringArray.push_back(GetStringFromNAPI(env, value));
    }
    // create result code
    napi_value result;
    napi_status status = napi_create_int32(env, NAPI_RETURN_ONE, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

void CommonFunc::ConvertWindowSize(napi_env env, const AbilityInfo &abilityInfo, napi_value value)
{
    napi_value nMaxWindowRatio;
    NAPI_CALL_RETURN_VOID(env, napi_create_double(env, abilityInfo.maxWindowRatio, &nMaxWindowRatio));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowRatio", nMaxWindowRatio));

    napi_value mMinWindowRatio;
    NAPI_CALL_RETURN_VOID(env, napi_create_double(env, abilityInfo.minWindowRatio, &mMinWindowRatio));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowRatio", mMinWindowRatio));

    napi_value nMaxWindowWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.maxWindowWidth, &nMaxWindowWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowWidth", nMaxWindowWidth));

    napi_value nMinWindowWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.minWindowWidth, &nMinWindowWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowWidth", nMinWindowWidth));

    napi_value nMaxWindowHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.maxWindowHeight, &nMaxWindowHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowHeight", nMaxWindowHeight));

    napi_value nMinWindowHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.minWindowHeight, &nMinWindowHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowHeight", nMinWindowHeight));
}

void CommonFunc::ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo, napi_value objAbilityInfo)
{
    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, BUNDLE_NAME, nBundleName));

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, MODULE_NAME, nModuleName));

    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, NAME, nName));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, LABEL, nLabel));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, LABEL_ID, nLabelId));

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, DESCRIPTION, nDescription));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, DESCRIPTION_ID, nDescriptionId));

    napi_value nIconPath;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, ICON, nIconPath));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, abilityInfo.iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, ICON_ID, nIconId));

    napi_value nProcess;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &nProcess));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "process", nProcess));

    napi_value nVisible;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, abilityInfo.visible, &nVisible));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, IS_VISIBLE, nVisible));

    napi_value nExported;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, abilityInfo.visible, &nExported));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, EXPORTED, nExported));

    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.type), &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "type", nType));

    napi_value nOrientation;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &nOrientation));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "orientation", nOrientation));

    napi_value nLaunchType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &nLaunchType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "launchType", nLaunchType));

    napi_value nPermissions;
    size_t size = abilityInfo.permissions.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nPermissions));
    for (size_t idx = 0; idx < size; ++idx) {
        napi_value nPermission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, abilityInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &nPermission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPermissions, idx, nPermission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, PERMISSIONS, nPermissions));

    napi_value nReadPermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.readPermission.c_str(), NAPI_AUTO_LENGTH, &nReadPermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, READ_PERMISSION, nReadPermission));

    napi_value nWritePermission;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.writePermission.c_str(), NAPI_AUTO_LENGTH, &nWritePermission));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, WRITE_PERMISSION, nWritePermission));

    napi_value nUri;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.uri.c_str(), NAPI_AUTO_LENGTH, &nUri));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, URI, nUri));

    napi_value nDeviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nDeviceTypes));
    for (size_t idx = 0; idx < abilityInfo.deviceTypes.size(); ++idx) {
        napi_value nDeviceType;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, abilityInfo.deviceTypes[idx].c_str(), NAPI_AUTO_LENGTH, &nDeviceType));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDeviceTypes, idx, nDeviceType));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "deviceTypes", nDeviceTypes));

    napi_value nApplicationInfo;
    if (!abilityInfo.applicationInfo.name.empty()) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nApplicationInfo));
        ConvertApplicationInfo(env, nApplicationInfo, abilityInfo.applicationInfo);
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &nApplicationInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, APPLICATION_INFO, nApplicationInfo));

    napi_value nMetadata;
    size = abilityInfo.metadata.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nMetadata));
    for (size_t index = 0; index < size; ++index) {
        napi_value nMetaData;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nMetaData));
        ConvertMetadata(env, abilityInfo.metadata[index], nMetaData);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nMetadata, index, nMetaData));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, META_DATA, nMetadata));

    napi_value nEnabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, abilityInfo.enabled, &nEnabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, ENABLED, nEnabled));

    napi_value nSupportWindowModes;
    size = abilityInfo.windowModes.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nSupportWindowModes));
    for (size_t index = 0; index < size; ++index) {
        napi_value innerMode;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_int32(env, static_cast<int32_t>(abilityInfo.windowModes[index]), &innerMode));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nSupportWindowModes, index, innerMode));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "supportWindowModes", nSupportWindowModes));

    napi_value nWindowSize;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nWindowSize));
    ConvertWindowSize(env, abilityInfo, nWindowSize);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "windowSize", nWindowSize));
}

void CommonFunc::ConvertResource(napi_env env, const Resource &resource, napi_value objResource)
{
    napi_value nBundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, resource.bundleName.c_str(), NAPI_AUTO_LENGTH, &nBundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, BUNDLE_NAME, nBundleName));

    napi_value nModuleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, resource.moduleName.c_str(), NAPI_AUTO_LENGTH, &nModuleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, MODULE_NAME, nModuleName));

    napi_value nId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, resource.id, &nId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, "id", nId));
}

void CommonFunc::ConvertApplicationInfo(napi_env env, napi_value objAppInfo, const ApplicationInfo &appInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, NAME, nName));

    napi_value nBundleType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(appInfo.bundleType), &nBundleType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "bundleType", nBundleType));

    napi_value nDebug;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.debug, &nDebug));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, DEBUG, nDebug));

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, DESCRIPTION, nDescription));

    napi_value nDescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.descriptionId, &nDescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, DESCRIPTION_ID, nDescriptionId));

    napi_value nEnabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.enabled, &nEnabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, ENABLED, nEnabled));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, LABEL, nLabel));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, LABEL_ID, nLabelId));

    napi_value nIconPath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, ICON, nIconPath));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, ICON_ID, nIconId));

    napi_value nProcess;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &nProcess));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "process", nProcess));

    napi_value nPermissions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nPermissions));
    for (size_t idx = 0; idx < appInfo.permissions.size(); idx++) {
        napi_value nPermission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, appInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &nPermission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPermissions, idx, nPermission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, PERMISSIONS, nPermissions));

    napi_value nEntryDir;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.entryDir.c_str(), NAPI_AUTO_LENGTH, &nEntryDir));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "entryDir", nEntryDir));

    napi_value nCodePath;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.codePath.c_str(), NAPI_AUTO_LENGTH, &nCodePath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "codePath", nCodePath));

    napi_value nMetaData;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nMetaData));
    for (const auto &item : appInfo.metadata) {
        napi_value nmetaDataArray;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nmetaDataArray));
        for (size_t j = 0; j < item.second.size(); j++) {
            napi_value nmetaData;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nmetaData));
            ConvertMetadata(env, item.second[j], nmetaData);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nmetaDataArray, j, nmetaData));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, nMetaData, item.first.c_str(), nmetaDataArray));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, META_DATA, nMetaData));

    napi_value nRemovable;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.removable, &nRemovable));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "removable", nRemovable));

    napi_value nAccessTokenId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, appInfo.accessTokenId, &nAccessTokenId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "accessTokenId", nAccessTokenId));

    napi_value nUid;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.uid, &nUid));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "uid", nUid));

    napi_value nIconResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nIconResource));
    ConvertResource(env, appInfo.iconResource, nIconResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "iconResource", nIconResource));

    napi_value nLabelResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nLabelResource));
    ConvertResource(env, appInfo.labelResource, nLabelResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "labelResource", nLabelResource));

    napi_value nDescriptionResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nDescriptionResource));
    ConvertResource(env, appInfo.descriptionResource, nDescriptionResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "descriptionResource", nDescriptionResource));

    napi_value nAppDistributionType;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.appDistributionType.c_str(), NAPI_AUTO_LENGTH,
        &nAppDistributionType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "appDistributionType", nAppDistributionType));

    napi_value nAppProvisionType;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.appProvisionType.c_str(), NAPI_AUTO_LENGTH,
        &nAppProvisionType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "appProvisionType", nAppProvisionType));

    napi_value nIsSystemApp;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.isSystemApp, &nIsSystemApp));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "systemApp", nIsSystemApp));
}

void CommonFunc::ConvertHapModuleInfo(napi_env env, const HapModuleInfo &hapModuleInfo, napi_value objHapModuleInfo)
{
    napi_value nName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &nName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, NAME, nName));

    napi_value nIcon;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &nIcon));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, ICON, nIcon));

    napi_value nIconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, hapModuleInfo.iconId, &nIconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, ICON_ID, nIconId));

    napi_value nLabel;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &nLabel));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, LABEL, nLabel));

    napi_value nLabelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, hapModuleInfo.labelId, &nLabelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, LABEL_ID, nLabelId));

    napi_value nDescription;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &nDescription));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, DESCRIPTION, nDescription));

    napi_value ndescriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, hapModuleInfo.descriptionId, &ndescriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, DESCRIPTION_ID, ndescriptionId));

    napi_value nMainElementName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.mainElementName.c_str(), NAPI_AUTO_LENGTH,
        &nMainElementName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "mainElementName", nMainElementName));

    napi_value nAbilityInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nAbilityInfos));
    for (size_t idx = 0; idx < hapModuleInfo.abilityInfos.size(); idx++) {
        napi_value objAbilityInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAbilityInfo));
        ConvertAbilityInfo(env, hapModuleInfo.abilityInfos[idx], objAbilityInfo);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nAbilityInfos, idx, objAbilityInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "abilitiesInfo", nAbilityInfos));

    napi_value nExtensionAbilityInfos;
    napi_create_array_with_length(env, hapModuleInfo.extensionInfos.size(), &nExtensionAbilityInfos);
    ConvertExtensionInfos(env, hapModuleInfo.extensionInfos, nExtensionAbilityInfos);
    NAPI_CALL_RETURN_VOID(env,
        napi_set_named_property(env, objHapModuleInfo, "extensionAbilitiesInfo", nExtensionAbilityInfos));

    napi_value nMetadata;
    size_t size = hapModuleInfo.metadata.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nMetadata));
    for (size_t index = 0; index < size; ++index) {
        napi_value innerMeta;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &innerMeta));
        ConvertMetadata(env, hapModuleInfo.metadata[index], innerMeta);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nMetadata, index, innerMeta));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "metadata", nMetadata));

    napi_value nDeviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &nDeviceTypes));
    for (size_t idx = 0; idx < hapModuleInfo.deviceTypes.size(); idx++) {
        napi_value nDeviceType;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, hapModuleInfo.deviceTypes[idx].c_str(), NAPI_AUTO_LENGTH, &nDeviceType));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDeviceTypes, idx, nDeviceType));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "deviceTypes", nDeviceTypes));

    napi_value nInstallationFree;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, hapModuleInfo.installationFree, &nInstallationFree));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "installationFree", nInstallationFree));

    napi_value nHashValue;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.hashValue.c_str(), NAPI_AUTO_LENGTH, &nHashValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "hashValue", nHashValue));

    napi_value nModuleSourceDir;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.moduleSourceDir.c_str(), NAPI_AUTO_LENGTH,
        &nModuleSourceDir));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "moduleSourceDir", nModuleSourceDir));

    napi_value nType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(hapModuleInfo.moduleType), &nType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "type", nType));

    napi_value nDependencies;
    size = hapModuleInfo.dependencies.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nDependencies));
    for (size_t index = 0; index < size; ++index) {
        napi_value nDependency;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nDependency));
        ConvertDependency(env, hapModuleInfo.dependencies[index], nDependency);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nDependencies, index, nDependency));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "dependencies", nDependencies));

    napi_value nPreloads;
    size = hapModuleInfo.preloads.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &nPreloads));
    for (size_t index = 0; index < size; ++index) {
        napi_value nPreload;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &nPreload));
        ConvertPreloadItem(env, hapModuleInfo.preloads[index], nPreload);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, nPreloads, index, nPreload));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "preloads", nPreloads));
}
} // AppExecFwk
} // OHOS
