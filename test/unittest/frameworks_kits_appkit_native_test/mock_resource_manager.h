/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "resource_manager.h"

namespace OHOS {
namespace Global {
namespace Resource {
class MockResourceManager : public ResourceManager {
public:
MockResourceManager *CreateMockResourceManager()
{
    MockResourceManager *impl = new (std::nothrow) MockResourceManager;
    return impl;
}

~MockResourceManager() {}

bool AddResource(const char *path)
{
    return true;
}

bool AddResource(const std::string &path, const std::vector<std::string> &overlayPaths)
{
    return true;
}

RState UpdateResConfig(ResConfig &resConfig)
{
    return SUCCESS;
}

void GetResConfig(ResConfig &resConfig) {}

RState GetStringById(uint32_t id, std::string &outValue)
{
    outValue = "ENTRY";
    return SUCCESS;
}

RState GetStringByName(const char *name, std::string &outValue)
{
    outValue = "bgmode_test";
    return SUCCESS;
}

RState GetStringFormatById(std::string &outValue, uint32_t id, ...)
{
    return SUCCESS;
}

RState GetStringFormatByName(std::string &outValue, const char *name, ...)
{
    return SUCCESS;
}

RState GetStringArrayById(uint32_t id, std::vector<std::string> &outValue)
{
    return SUCCESS;
}

RState GetStringArrayByName(const char *name, std::vector<std::string> &outValue)
{
    return SUCCESS;
}

RState GetPatternById(uint32_t id, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState GetPatternByName(const char *name, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState GetPluralStringById(uint32_t id, int32_t quantity, std::string &outValue)
{
    return SUCCESS;
}

RState GetPluralStringByName(const char *name, int32_t quantity, std::string &outValue)
{
    return SUCCESS;
}

RState GetPluralStringByIdFormat(std::string &outValue, uint32_t id, int32_t quantity, ...)
{
    return SUCCESS;
}

RState GetPluralStringByNameFormat(std::string &outValue, const char *name, int32_t quantity, ...)
{
    return SUCCESS;
}

RState GetThemeById(uint32_t id, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState GetThemeByName(const char *name, std::map<std::string, std::string> &outValue)
{
    return SUCCESS;
}

RState GetBooleanById(uint32_t id, bool &outValue)
{
    return SUCCESS;
}

RState GetBooleanByName(const char *name, bool &outValue)
{
    return SUCCESS;
}

RState GetIntegerById(uint32_t id, int32_t &outValue)
{
    return SUCCESS;
}

RState GetIntegerByName(const char *name, int32_t &outValue)
{
    return SUCCESS;
}

RState GetFloatById(uint32_t id, float &outValue)
{
    return SUCCESS;
}

RState GetFloatById(uint32_t id, float &outValue, std::string &unit)
{
    return SUCCESS;
}

RState GetFloatByName(const char *name, float &outValue)
{
    return SUCCESS;
}

RState GetFloatByName(const char *name, float &outValue, std::string &unit)
{
    return SUCCESS;
}

RState GetIntArrayById(uint32_t id, std::vector<int32_t> &outValue)
{
    return SUCCESS;
}

RState GetIntArrayByName(const char *name, std::vector<int32_t> &outValue)
{
    return SUCCESS;
}

RState GetColorById(uint32_t id, uint32_t &outValue)
{
    return SUCCESS;
}

RState GetColorByName(const char *name, uint32_t &outValue)
{
    return SUCCESS;
}

RState GetProfileById(uint32_t id, std::string &outValue)
{
    return SUCCESS;
}

RState GetProfileByName(const char *name, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaById(uint32_t id, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaById(uint32_t id, uint32_t density, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaByName(const char *name, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaByName(const char *name, uint32_t density, std::string &outValue)
{
    return SUCCESS;
}

RState GetRawFilePathByName(const std::string &name, std::string &outValue)
{
    return SUCCESS;
}

RState GetRawFileDescriptor(const std::string &name, RawFileDescriptor &descriptor)
{
    return SUCCESS;
}

RState CloseRawFileDescriptor(const std::string &name)
{
    return SUCCESS;
}

RState GetMediaBase64ByIdData(uint32_t id, uint32_t density, std::string &base64Data)
{
    return SUCCESS;
}

RState GetMediaBase64ByNameData(const char *name, uint32_t density, std::string &base64Data)
{
    return SUCCESS;
}

RState GetMediaDataById(uint32_t id, size_t &len, std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetMediaDataByName(const char *name, size_t &len, std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetMediaDataById(uint32_t id, uint32_t density, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetMediaDataByName(const char *name, uint32_t density, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetMediaBase64DataById(uint32_t id, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaBase64DataByName(const char *name, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaBase64DataById(uint32_t id, uint32_t density, std::string &outValue)
{
    return SUCCESS;
}

RState GetMediaBase64DataByName(const char *name, uint32_t density, std::string &outValue)
{
    return SUCCESS;
}

RState GetProfileDataById(uint32_t id, size_t &len, std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetProfileDataByName(const char *name, size_t &len, std::unique_ptr<uint8_t[]> &outValue)
{
    return SUCCESS;
}

RState GetRawFileFromHap(const std::string& rawFileName, size_t& len,
    std::unique_ptr<uint8_t[]>& outValue)
{
    return SUCCESS;
}

RState GetRawFileDescriptorFromHap(const std::string &rawFileName, RawFileDescriptor &descriptor)
{
    return SUCCESS;
}

RState IsLoadHap(std::string& hapPath)
{
    return SUCCESS;
}

RState GetRawFileList(const std::string rawDirPath, std::vector<std::string>& rawfileList)
{
    return SUCCESS;
}

RState IsLoadHap()
{
    return SUCCESS;
}
};
}  // namespace Resource
}  // namespace Global
}  // namespace OHOS