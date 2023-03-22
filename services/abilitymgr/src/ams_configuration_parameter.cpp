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
    return timeoutUnitTime_;
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
    int ret = -1;
    if (Object.contains(AmsConfig::SERVICE_ITEM_AMS)) {
        missionSaveTime_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::MISSION_SAVE_TIME).get<int>();
        anrTime_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::APP_NOT_RESPONSE_PROCESS_TIMEOUT_TIME).get<int>();
        amsTime_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::AMS_TIMEOUT_TIME).get<int>();
        maxRootLauncherRestartNum_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::ROOT_LAUNCHER_RESTART_MAX).get<int>();
        if (Object.at(AmsConfig::SERVICE_ITEM_AMS).contains(AmsConfig::RESIDENT_RESTART_MAX)) {
            maxResidentRestartNum_ =
                Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::RESIDENT_RESTART_MAX).get<int>();
        }
        if (Object.at(AmsConfig::SERVICE_ITEM_AMS).contains(AmsConfig::RESTART_INTERVAL_TIME)) {
            restartIntervalTime_ =
                Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::RESTART_INTERVAL_TIME).get<int>();
        }
        deviceType_ = Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::DEVICE_TYPE).get<std::string>();
        bootAnimationTime_ =
            Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::BOOT_ANIMATION_TIMEOUT_TIME).get<int>();
        if (Object.at(AmsConfig::SERVICE_ITEM_AMS).contains(AmsConfig::TIMEOUT_UNIT_TIME)) {
            timeoutUnitTime_ =
                Object.at(AmsConfig::SERVICE_ITEM_AMS).at(AmsConfig::TIMEOUT_UNIT_TIME).get<int>();
        }
        HILOG_INFO("get ams service config success!");
        ret = 0;
    }

    return ret;
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
    if (Object.contains(AmsConfig::SYSTEM_CONFIGURATION)) {
        orientation_ = Object.at(AmsConfig::SYSTEM_CONFIGURATION).at(AmsConfig::SYSTEM_ORIENTATION).get<std::string>();
        return READ_OK;
    }

    return READ_FAIL;
}
}  // namespace AAFwk
}  // namespace OHOS
