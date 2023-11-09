/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int LOAD_CONFIGURATION_FAILED = -1;
const int LOAD_CONFIGURATION_SUCCESS = 0;
const int32_t TIME_OUT_UNIT_TIME_RATIO = 1000;
}

AmsConfigurationParameter::AmsConfigurationParameter()
{
    std::string deviceType = OHOS::system::GetParameter("const.product.devicetype", "unknown");
    isPcDevice_ = (deviceType == "tablet" || deviceType == "pc" || deviceType == "2in1");
}

AmsConfigurationParameter &AmsConfigurationParameter::GetInstance()
{
    static AmsConfigurationParameter amsConfiguration;
    return amsConfiguration;
}

using json = nlohmann::json;

void AmsConfigurationParameter::Parse()
{
    auto ref = LoadAmsConfiguration(AmsConfig::AMS_CONFIG_FILE_PATH);
    HILOG_INFO("load config ref : %{public}d", ref);
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

std::string AmsConfigurationParameter::GetDeviceType() const
{
    return deviceType_;
}

int AmsConfigurationParameter::GetBootAnimationTimeoutTime() const
{
    return bootAnimationTime_;
}

int AmsConfigurationParameter::GetAppStartTimeoutTime() const
{
    if (isPcDevice_) {
        return timeoutUnitTime_ * TIME_OUT_UNIT_TIME_RATIO;
    }
    return timeoutUnitTime_;
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

int AmsConfigurationParameter::LoadAmsConfiguration(const std::string &filePath)
{
    HILOG_DEBUG("%{public}s", __func__);
    int ret[2] = {0};
    std::ifstream inFile;
    inFile.open(filePath, std::ios::in);
    if (!inFile.is_open()) {
        HILOG_INFO("read ams config error ...");
        nonConfigFile_ = true;
        return READ_FAIL;
    }

    json amsJson;
    inFile >> amsJson;
    if (amsJson.is_discarded()) {
        HILOG_INFO("json discarded error ...");
        nonConfigFile_ = true;
        inFile.close();
        return READ_JSON_FAIL;
    }

    ret[0] = LoadAppConfigurationForStartUpService(amsJson);
    if (ret[0] != 0) {
        HILOG_ERROR("LoadAppConfigurationForStartUpService return error");
    }

    ret[1] = LoadAppConfigurationForMemoryThreshold(amsJson);
    if (ret[1] != 0) {
        HILOG_ERROR("LoadAppConfigurationForMemoryThreshold return error");
    }

    LoadSystemConfiguration(amsJson);
    SetPickerJsonObject(amsJson);
    amsJson.clear();
    inFile.close();

    for (const auto& i : ret) {
        if (i != 0) {
            HILOG_ERROR("json no have service item ...");
            return READ_JSON_FAIL;
        }
    }

    HILOG_INFO("read ams config success!");
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
    UpdateStartUpServiceConfigString(Object, AmsConfig::DEVICE_TYPE, deviceType_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::BOOT_ANIMATION_TIMEOUT_TIME, bootAnimationTime_);
    UpdateStartUpServiceConfigInteger(Object, AmsConfig::TIMEOUT_UNIT_TIME, timeoutUnitTime_);
    return LOAD_CONFIGURATION_SUCCESS;
}

int AmsConfigurationParameter::LoadAppConfigurationForMemoryThreshold(nlohmann::json &Object)
{
    int ret = 0;
    if (!Object.contains("memorythreshold")) {
        HILOG_ERROR("LoadAppConfigurationForMemoryThreshold return error");
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
}  // namespace AAFwk
}  // namespace OHOS
