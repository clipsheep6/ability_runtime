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

#include "mission_data_storage.h"

#include <cctype>

#include "file_util.h"
#include "hilog_wrapper.h"
#include "image_source.h"
#include "media_errors.h"
#include "png.h"

namespace OHOS {
namespace AAFwk {
MissionDataStorage::MissionDataStorage(int userId)
{
    userId_ = userId;
}

MissionDataStorage::~MissionDataStorage()
{}

void MissionDataStorage::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

bool MissionDataStorage::LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList)
{
    std::vector<std::string> fileNameVec;
    std::string dirPath = GetMissionDataDirPath();
    OHOS::HiviewDFX::FileUtil::GetDirFiles(dirPath, fileNameVec);

    for (auto fileName : fileNameVec) {
        if (!CheckFileNameValid(fileName)) {
            HILOG_ERROR("load mission info: file name %{public}s invalid.", fileName.c_str());
            continue;
        }

        std::string content;
        bool loadFile = OHOS::HiviewDFX::FileUtil::LoadStringFromFile(fileName, content);
        if (!loadFile) {
            HILOG_ERROR("load string from file %{public}s failed.", fileName.c_str());
            continue;
        }

        InnerMissionInfo misssionInfo;
        misssionInfo.FromJsonStr(content);
        missionInfoList.push_back(misssionInfo);
    }
    return true;
}

void MissionDataStorage::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::string filePath = GetMissionDataFilePath(missionInfo.missionInfo.id);
    std::string dirPath = OHOS::HiviewDFX::FileUtil::ExtractFilePath(filePath);
    if (!OHOS::HiviewDFX::FileUtil::FileExists(dirPath)) {
        bool createDir = OHOS::HiviewDFX::FileUtil::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("create dir %{public}s failed.", dirPath.c_str());
            return;
        }
    }

    std::string jsonStr = missionInfo.ToJsonStr();
    bool saveMissionFile = OHOS::HiviewDFX::FileUtil::SaveStringToFile(filePath, jsonStr, true);
    if (!saveMissionFile) {
        HILOG_ERROR("save mission file %{public}s failed.", filePath.c_str());
    }
}

void MissionDataStorage::DeleteMissionInfo(int missionId)
{
    std::string filePath = GetMissionDataFilePath(missionId);
    bool removeMissionFile = OHOS::HiviewDFX::FileUtil::RemoveFile(filePath);
    if (!removeMissionFile) {
        HILOG_ERROR("remove mission file %{public}s failed.", filePath.c_str());
    }
}

std::string MissionDataStorage::GetMissionDataDirPath()
{
    return TASK_DATA_FILE_BASE_PATH + "/" + std::to_string(userId_) + "/" + MISSION_DATA_FILE_PATH;
}

std::string MissionDataStorage::GetMissionDataFilePath(int missionId)
{
    return GetMissionDataDirPath() + "/"
        + MISSION_JSON_FILE_PREFIX + "_" + std::to_string(missionId) + JSON_FILE_SUFFIX;
}

bool MissionDataStorage::CheckFileNameValid(const std::string &fileName)
{
    std::string fileNameExcludePath = OHOS::HiviewDFX::FileUtil::ExtractFileName(fileName);
    if (fileNameExcludePath.find(MISSION_JSON_FILE_PREFIX) != 0) {
        return false;
    }

    if (fileNameExcludePath.find("_") != MISSION_JSON_FILE_PREFIX.length()) {
        return false;
    }

    if (fileNameExcludePath.find(JSON_FILE_SUFFIX) != fileNameExcludePath.length() - JSON_FILE_SUFFIX.length()) {
        return false;
    }

    int missionIdLength = fileNameExcludePath.find(JSON_FILE_SUFFIX) - fileNameExcludePath.find("_") - 1;
    std::string missionId = fileNameExcludePath.substr(fileNameExcludePath.find("_") + 1, missionIdLength);
    for (auto ch : missionId) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}

void MissionDataStorage::SaveMissionSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
    std::string filePath = GetMissionSnapshotPath(missionId);
    std::string dirPath = OHOS::HiviewDFX::FileUtil::ExtractFilePath(filePath);
    if (!OHOS::HiviewDFX::FileUtil::FileExists(dirPath)) {
        bool createDir = OHOS::HiviewDFX::FileUtil::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("snapshot: create dir %{public}s failed.", dirPath.c_str());
            return;
        }
    }
    const uint8_t* data = missionSnapshot.snapshot->GetPixels();
    bool saveMissionFile = WriteToPng(filePath.c_str(), missionSnapshot.snapshot->GetWidth(),
        missionSnapshot.snapshot->GetHeight(), data);
    if (!saveMissionFile) {
        HILOG_ERROR("snapshot: save mission snapshot failed, path = %{public}s.", filePath.c_str());
    }
}

bool MissionDataStorage::GetMissionSnapshot(int missionId, MissionSnapshot& missionSnapshot)
{
    std::string filePath = GetMissionSnapshotPath(missionId);
    if (!OHOS::HiviewDFX::FileUtil::FileExists(filePath)) {
        HILOG_INFO("snapshot: storage snapshot not exists, missionId = %{public}d", missionId);
        return false;
    }
    uint32_t errCode = 0;
    Media::SourceOptions sourceOptions;
    auto imageSource = Media::ImageSource::CreateImageSource(filePath, sourceOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS) {
        HILOG_ERROR("snapshot: CreateImageSource failed, errCode = %{public}d", errCode);
        return false;
    }
    Media::DecodeOptions decodeOptions;
    auto pixelMap = imageSource->CreatePixelMap(decodeOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS) {
        HILOG_ERROR("snapshot: CreatePixelMap failed, errCode = %{public}d", errCode);
        return false;
    }
    missionSnapshot.snapshot = std::move(pixelMap);
    return true;
}

std::string MissionDataStorage::GetMissionSnapshotPath(int missionId)
{
    return GetMissionDataDirPath() + "/"
        + MISSION_JSON_FILE_PREFIX + "_" + std::to_string(missionId) + PNG_FILE_SUFFIX;
}

bool MissionDataStorage::WriteToPng(const char* fileName, uint32_t width, uint32_t height, const uint8_t* data)
{
    const int BITMAP_DEPTH = 8; // color depth
    const int BPP = 4; // bytes per pixel
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr) {
        HILOG_ERROR("snapshot: png_create_write_struct error, nullptr!\n");
        return false;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (png_ptr == nullptr) {
        HILOG_ERROR("snapshot: png_create_info_struct error, nullptr!\n");
        png_destroy_write_struct(&png_ptr, nullptr);
        return false;
    }
    FILE *fp = fopen(fileName,"wb");
    if (fp == nullptr) {
        HILOG_ERROR("snapshot: open file [%s] error, nullptr!\n", fileName);
        png_destroy_write_struct(&png_ptr,&info_ptr);
        return false;
    }
    png_init_io(png_ptr,fp);

    // set png header
    png_set_IHDR(png_ptr,info_ptr,
        width, height,
        BITMAP_DEPTH,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);
    png_set_packing(png_ptr);
    png_write_info(png_ptr, info_ptr);

    for (uint32_t i = 0; i < height; i++) {
        png_write_row(png_ptr, data + (i * width * BPP));
    }
    png_write_end(png_ptr,info_ptr);

    // free memory
    png_destroy_write_struct(&png_ptr,&info_ptr);
    fclose(fp);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
