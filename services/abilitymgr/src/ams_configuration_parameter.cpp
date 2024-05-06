/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ams_configuration_parameter.h"
#include <unistd.h>
#include "app_utils.h"
#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int LOAD_CONFIGURATION_FAILED = -1;
const int LOAD_CONFIGURATION_SUCCESS = 0;
}

AmsConfigurationParameter::AmsConfigurationParameter() {}

AmsConfigurationParameter &AmsConfigurationParameter::GetInstance()
{
    static AmsConfigurationParameter amsConfiguration;
    return amsConfiguration;
}

using json = nlohmann::json;

void AmsConfigurationParameter::Parse()
{
    auto ref = LoadAmsConfiguration(AmsConfig::AMS_CONFIG_FILE_PATH);

    char buf[MAX_PATH_LEN] = { 0 };
    char *filePath = GetOneCfgFile(AmsConfig::PICKER_CONFIG_FILE_PATH, buf, MAX_PATH_LEN);
    if (filePath == nullptr || filePath[0] == '\0' || strlen(filePath) > MAX_PATH_LEN) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not get config file");
        LoadUIExtensionPickerConfig(AmsConfig::PICKER_CONFIG_FILE_PATH_DEFAULT);
    }
    std::string customConfig = filePath;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "The configuration file path is :%{private}s", customConfig.c_str());
    LoadUIExtensionPickerConfig(customConfig);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "load config ref : %{public}d", ref);
}

bool AmsConfigurationParameter::NonConfigFile() const
{
    return nonConfigFile_;
}

int AmsConfigurationParameter::GetMissionSaveTime() const
{
    return missionSaveTime_;
}

std::string AmsConfigurationParameter::GetOrientation() const
{
    return orientation_;
}

int AmsConfigurationParameter::GetANRTimeOutTime() const
{
    return anrTime_;
}

int AmsConfigurationParameter::GetAMSTimeOutTime() const
{
    return amsTime_;
}

int AmsConfigurationParameter::GetMaxRestartNum(bool isRootLauncher) const
{
    return (isRootLauncher ? maxRootLauncherRestartNum_ : maxResidentRestartNum_);
}

int AmsConfigurationParameter::GetRestartIntervalTime() const
{
    return restartIntervalTime_;
}

int AmsConfigurationParameter::GetBootAnimationTimeoutTime() const
{
    return bootAnimationTime_;
}

int AmsConfigurationParameter::GetAppStartTimeoutTime() const
{
    return timeoutUnitTime_ * AppUtils::GetInstance().GetTimeoutUnitTimeRatio();
}

void AmsConfigurationParameter::SetPickerJsonObject(nlohmann::json Object)
{
    if (Object.contains(AmsConfig::PICKER_CONFIGURATION)) {
        pickerJsonObject_ = Object.at(AmsConfig::PICKER_CONFIGURATION);
    }
}

nlohmann::json AmsConfigurationParameter::GetPickerJsonObject() const
{
    return pickerJsonObject_;
}

const std::map<std::string, std::string>& AmsConfigurationParameter::GetPickerMap() const
{
    return picker_;
}

void AmsConfigurationParameter::LoadUIExtensionPickerConfig(const std::string &filePath)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "the file is not existed due to empty file path.");
        return;
    }

    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not access the file: %{private}s.", filePath.c_str());
        return;
    }
    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read picker config error");
        return;
    }

    json pickerJson;
    inFile >> pickerJson;
    inFile.close();
    if (pickerJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "json discarded error");
        return;
    }

    if (pickerJson.is_null() || pickerJson.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid jsonObj");
        return;
    }

    if (!pickerJson.contains(AmsConfig::UIEATENSION)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "json config not contains the key");
        return;
    }

    if (pickerJson[AmsConfig::UIEATENSION].is_null() || !pickerJson[AmsConfig::UIEATENSION].is_array()
        || pickerJson[AmsConfig::UIEATENSION].empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid obj");
        return;
    }

    for (auto extension : pickerJson[AmsConfig::UIEATENSION]) {
        if (extension[AmsConfig::UIEATENSION_TYPE].is_null() || !extension[AmsConfig::UIEATENSION_TYPE].is_string()
            || extension[AmsConfig::UIEATENSION_TYPE_PICKER].is_null()
            || !extension[AmsConfig::UIEATENSION_TYPE_PICKER].is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid key or value");
            continue;
        }
        std::string type = extension[AmsConfig::UIEATENSION_TYPE].get<std::string>();
        std::string typePicker = extension[AmsConfig::UIEATENSION_TYPE_PICKER].get<std::string>();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "type: %{public}s, typePicker: %{public}s", type.c_str(), typePicker.c_str());
        picker_[type] = typePicker;
    }
    pickerJson.clear();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "read config success");
}

int AmsConfigurationParameter::LoadAmsConfiguration(const std::string &filePath)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s", __func__);
    int ret[2] = {0};
    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "the file is not existed due to empty file path.");
        return READ_FAIL;
    }

    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not access the file: %{private}s.", filePath.c_str());
        return READ_FAIL;
    }
    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Read ability manager service config error.");
        nonConfigFile_ = true;
        return READ_FAIL;
    }

    json amsJson;
    inFile >> amsJson;
    if (amsJson.is_discarded()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "json discarded error ...");
        nonConfigFile_ = true;
        inFile.close();
        return READ_JSON_FAIL;
    }

    ret[0] = LoadAppConfigurationForStartUpService(amsJson);
    if (ret[0] != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "LoadAppConfigurationForStartUpService return error");
    }

    ret[1] = LoadAppConfigurationForMemoryThreshold(amsJson);
    if (ret[1] != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "LoadAppConfigurationForMemoryThreshold return error");
    }

    LoadSystemConfiguration(amsJson);
    SetPickerJsonObject(amsJson);
    amsJson.clear();
    inFile.close();

    for (const auto& i : ret) {
        if (i != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "json no have service item ...");
            return READ_JSON_FAIL;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Reading ability manager service config success.");
    return READ_OK;
}

int AmsConfigurationParameter::LoadAppConfigurationForStartUpService(nlohmann::json& Object)
{
    if (!Object.contains(AmsConfig::SERVICE_ITEM_AMS)) {
        return LOAD_CONFIGURATION_FAILED;
    }
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::MISSION_SAVE_TIME, missionSaveTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME, anrTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::AMS_TIMEOUT_TIME, amsTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::ROOT_LAUNCHER_RESTART_MAX, maxRootLauncherRestartNum_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::RESIDENT_RESTART_MAX, maxResidentRestartNum_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::RESTART_INTERVAL_TIME, restartIntervalTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::BOOT_ANIMATION_TIMEOUT_TIME, bootAnimationTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::TIMEOUT_UNIT_TIME, timeoutUnitTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::MULTI_USER_TYPE, multiUserType_);
    return LOAD_CONFIGURATION_SUCCESS;
}

int AmsConfigurationParameter::LoadAppConfigurationForMemoryThreshold(nlohmann::json &Object)
{
    int ret = 0;
    if (!Object.contains("memorythreshold")) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "LoadAppConfigurationForMemoryThreshold return error");
        ret = -1;
    }

    return ret;
}

int AmsConfigurationParameter::LoadSystemConfiguration(nlohmann::json& Object)
{
    if (Object.contains(AmsConfig::SYSTEM_CONFIGURATION) &&
        Object.at(AmsConfig::SYSTEM_CONFIGURATION).contains(AmsConfig::SYSTEM_ORIENTATION) &&
        Object.at(AmsConfig::SYSTEM_CONFIGURATION).at(AmsConfig::SYSTEM_ORIENTATION).is_string()) {
        orientation_ = Object.at(AmsConfig::SYSTEM_CONFIGURATION).at(AmsConfig::SYSTEM_ORIENTATION).get<std::string>();
        return READ_OK;
    }

    return READ_FAIL;
}

bool AmsConfigurationParameter::CheckServiceConfigEnable(nlohmann::json& Object, const std::string &configName,
    JsonValueType type)
{
    if (Object.contains(AmsConfig::SERVICE_ITEM_AMS) &&
        Object.at(AmsConfig::SERVICE_ITEM_AMS).contains(configName)) {
        switch (type) {
            case JsonValueType::NUMBER: {
                return Object.at(AmsConfig::SERVICE_ITEM_AMS).at(configName).is_number();
            }
            case JsonValueType::STRING: {
                return Object.at(AmsConfig::SERVICE_ITEM_AMS).at(configName).is_string();
            }
            case JsonValueType::BOOLEAN: {
                return Object.at(AmsConfig::SERVICE_ITEM_AMS).at(configName).is_boolean();
            }
            default: {
                return false;
            }
        }
    }
    return false;
}

void AmsConfigurationParameter::UpdateStartUpServiceConfigInteger(nlohmann::json& Object,
    const std::string &configName, int32_t &value)
{
    if (CheckServiceConfigEnable(Object, configName, JsonValueType::NUMBER)) {
        value = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(configName).get<int>();
    }
}

void AmsConfigurationParameter::UpdateStartUpServiceConfigString(nlohmann::json& Object,
    const std::string &configName, std::string &value)
{
    if (CheckServiceConfigEnable(Object, configName, JsonValueType::STRING)) {
        value = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(configName).get<std::string>();
    }
}

int AmsConfigurationParameter::MultiUserType() const
{
    return multiUserType_;
}
}  // namespace AAFwk
}  // namespace OHOS
