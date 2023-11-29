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

#ifndef OHOS_ABILITY_RUNTIME_AMS_CONFIGURATION_PARAMETER_H
#define OHOS_ABILITY_RUNTIME_AMS_CONFIGURATION_PARAMETER_H

#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include "nocopyable.h"
#include "parameters.h"

namespace OHOS {
namespace AAFwk {
namespace AmsConfig {
constexpr const char* AMS_CONFIG_FILE_PATH = "/system/etc/ams_service_config.json";
constexpr const char* PICKER_CONFIG_FILE_PATH = "/system/etc/uiextension_picker_config.json";
constexpr const char* SERVICE_ITEM_AMS = "service_startup_config";
constexpr const char* MISSION_SAVE_TIME = "mission_save_time";
constexpr const char* APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME = "app_not_response_process_timeout_time";
constexpr const char* AMS_TIMEOUT_TIME = "ams_timeout_time";
constexpr const char* DEVICE_TYPE = "device_type";
constexpr const char* SYSTEM_CONFIGURATION = "system_configuration";
constexpr const char* SYSTEM_ORIENTATION = "system_orientation";
constexpr const char* ROOT_LAUNCHER_RESTART_MAX = "root_launcher_restart_max";
constexpr const char* RESIDENT_RESTART_MAX = "resident_restart_max";
constexpr const char* RESTART_INTERVAL_TIME = "restart_interval_time";
constexpr const char* BOOT_ANIMATION_TIMEOUT_TIME = "boot_animation_timeout_time";
constexpr const char* TIMEOUT_UNIT_TIME = "timeout_unit_time";
constexpr const char* ABILITY_NAME = "ability_name";
constexpr const char* BUNDLE_NAME = "bundle_name";
constexpr const char* PICKER_CONFIGURATION = "picker_configuration";
constexpr const char* PICKER_TYPE = "picker_type";
constexpr const char* UIEATENSION = "uiextension";
constexpr const char* UIEATENSION_TYPE = "type";
constexpr const char* UIEATENSION_TYPE_PICKER = "typePicker";
}  // namespace AmsConfig

enum class SatrtUiMode { STATUSBAR = 1, NAVIGATIONBAR = 2, STARTUIBOTH = 3 };

enum class JsonValueType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING
};

class AmsConfigurationParameter final {
public:
    static AmsConfigurationParameter &GetInstance();
    /**
     * return true : ams no config file
     * return false : ams have config file
     */
    bool NonConfigFile() const;
    /**
     * Get profile information
     */
    void Parse();

    /**
     * Get the save time of the current content
     */
    int GetMissionSaveTime() const;

    /**
     * Get current system direction parameters, Temporary method.
     */
    std::string GetOrientation() const;

    /**
     * Get the max number of restart.
     */
    int GetMaxRestartNum(bool isRootLauncher) const;

    /**
     * Get the interval time after restart out of the max number of restart.
     */
    int GetRestartIntervalTime() const;

    /**
     * get the application not response process timeout time.
     */
    int GetANRTimeOutTime() const;

    /**
     * get ability manager service not response process timeout time.
     */
    int GetAMSTimeOutTime() const;

    /**
     * get device type.
     */
    std::string GetDeviceType() const;

    /**
     * get boot animation stared timout time.
     */
    int GetBootAnimationTimeoutTime() const;

    /**
     * get the application cold start timeout time.
     */
    int GetAppStartTimeoutTime() const;

    /**
     * set picker json object.
     */
    void SetPickerJsonObject(nlohmann::json jsonObject);

    /**
     * get picker json object.
     */
    nlohmann::json GetPickerJsonObject() const;

    const std::map<std::string, std::string>& GetPickerMap() const;

    enum { READ_OK = 0, READ_FAIL = 1, READ_JSON_FAIL = 2 };

private:
    AmsConfigurationParameter();
    ~AmsConfigurationParameter() = default;
    DISALLOW_COPY_AND_MOVE(AmsConfigurationParameter);
    /**
     * Read the configuration file of ams
     *
     */
    int LoadAmsConfiguration(const std::string &filePath);
    int LoadAppConfigurationForStartUpService(nlohmann::json& Object);
    int LoadAppConfigurationForMemoryThreshold(nlohmann::json& Object);
    int LoadSystemConfiguration(nlohmann::json& Object);
    void LoadPickerConfiguration(nlohmann::json& Object);
    bool CheckServiceConfigEnable(nlohmann::json& Object, const std::string &configName, JsonValueType type);
    void UpdateStartUpServiceConfigInteger(nlohmann::json& Object, const std::string &configName, int32_t &value);
    void UpdateStartUpServiceConfigString(nlohmann::json& Object, const std::string &configName, std::string &value);
    void UpdatePickerConfigurationString(nlohmann::json& Object, const std::string &configName, std::string &value);
    void LoadUIExtensionPickerConfig(const std::string &filePath);

private:
    bool nonConfigFile_ {false};
    int maxRootLauncherRestartNum_ = 0;
    int maxResidentRestartNum_ = 0;
    int restartIntervalTime_ {120000};
    std::string orientation_ {""};
    int missionSaveTime_ {12 * 60 * 60 * 1000};
    int anrTime_ {5000};
    int amsTime_ {5000};
    int bootAnimationTime_ {5};
    std::string deviceType_ {""};
    int timeoutUnitTime_ {1000};
    std::string bundleName_ {""};
    std::string abilityName_ {""};
    std::string pickerType_ {""};
    nlohmann::json pickerJsonObject_ = nlohmann::json::object();
    bool isPcDevice_ = false;
    std::map<std::string, std::string> picker_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_AMS_CONFIGURATION_PARAMETER_H
