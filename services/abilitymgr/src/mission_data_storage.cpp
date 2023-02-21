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

#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "image_source.h"
#include "media_errors.h"
#include "mission_info_mgr.h"
#ifdef SUPPORT_GRAPHICS
#include <cstdio>
#include <setjmp.h>
#include "jpeglib.h"
#include "securec.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_GRAPHICS
constexpr int32_t RGB565_PIXEL_BYTES = 2;
constexpr int32_t RGB888_PIXEL_BYTES = 3;
constexpr int32_t RGBA8888_PIXEL_BYTES = 4;

constexpr uint8_t B_INDEX = 0;
constexpr uint8_t G_INDEX = 1;
constexpr uint8_t R_INDEX = 2;
constexpr uint8_t SHIFT_2_BIT = 2;
constexpr uint8_t SHITF_3_BIT = 3;
constexpr uint8_t SHIFT_5_BIT = 5;
constexpr uint8_t SHIFT_8_BIT = 8;
constexpr uint8_t SHIFT_11_BIT = 11;
constexpr uint8_t SHIFT_16_BIT = 16;

constexpr uint16_t RGB565_MASK_BLUE = 0xF800;
constexpr uint16_t RGB565_MASK_GREEN = 0x07E0;
constexpr uint16_t RGB565_MASK_RED = 0x001F;
constexpr uint32_t RGBA8888_MASK_BLUE = 0x000000FF;
constexpr uint32_t RGBA8888_MASK_GREEN = 0x0000FF00;
constexpr uint32_t RGBA8888_MASK_RED = 0x00FF0000;

const mode_t MODE = 0770;

struct mission_error_mgr : public jpeg_error_mgr {
    jmp_buf environment;
};

METHODDEF(void) mission_error_exit(j_common_ptr cinfo)
{
    if (cinfo == nullptr || cinfo->err == nullptr) {
        HILOG_ERROR("%{public}s param is invalid.", __func__);
        return;
    }
    auto err = static_cast<mission_error_mgr*>(cinfo->err);
    longjmp(err->environment, 1);
}
#endif

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
    std::vector<int32_t> tempMissions;
    std::string dirPath = GetMissionDataDirPath();
    OHOS::GetDirFiles(dirPath, fileNameVec);

    for (auto fileName : fileNameVec) {
        if (!CheckFileNameValid(fileName)) {
            HILOG_ERROR("load mission info: file name %{public}s invalid.", fileName.c_str());
            continue;
        }

        std::string content;
        bool loadFile = OHOS::LoadStringFromFile(fileName, content);
        if (!loadFile) {
            HILOG_ERROR("load string from file %{public}s failed.", fileName.c_str());
            continue;
        }

        InnerMissionInfo misssionInfo;
        if (!misssionInfo.FromJsonStr(content)) {
            HILOG_ERROR("parse mission info failed. file: %{public}s", fileName.c_str());
            continue;
        }
        if (misssionInfo.isTemporary) {
            tempMissions.push_back(misssionInfo.missionInfo.id);
            continue;
        }
        missionInfoList.push_back(misssionInfo);
    }

    for (auto missionId : tempMissions) {
        DeleteMissionInfo(missionId);
    }
    return true;
}

void MissionDataStorage::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::string filePath = GetMissionDataFilePath(missionInfo.missionInfo.id);
    std::string dirPath = OHOS::ExtractFilePath(filePath);
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("create dir %{public}s failed.", dirPath.c_str());
            return;
        }
        chmod(dirPath.c_str(), MODE);
    }

    std::string jsonStr = missionInfo.ToJsonStr();
    bool saveMissionFile = OHOS::SaveStringToFile(filePath, jsonStr, true);
    if (!saveMissionFile) {
        HILOG_ERROR("save mission file %{public}s failed.", filePath.c_str());
    }
}

void MissionDataStorage::DeleteMissionInfo(int missionId)
{
    std::string filePath = GetMissionDataFilePath(missionId);
    bool removeMissionFile = OHOS::RemoveFile(filePath);
    if (!removeMissionFile) {
        HILOG_ERROR("remove mission file %{public}s failed.", filePath.c_str());
        return;
    }
    DeleteMissionSnapshot(missionId);
}

void MissionDataStorage::SaveMissionSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("snapshot: save snapshot from cache, missionId = %{public}d", missionId);
    SaveCachedSnapshot(missionId, missionSnapshot);
    SaveSnapshotFile(missionId, missionSnapshot);
    HILOG_INFO("snapshot: delete snapshot from cache, missionId = %{public}d", missionId);
    DeleteCachedSnapshot(missionId);
#endif
}

void MissionDataStorage::DeleteMissionSnapshot(int32_t missionId)
{
#ifdef SUPPORT_GRAPHICS
    DeleteMissionSnapshot(missionId, false);
    DeleteMissionSnapshot(missionId, true);
#endif
}

bool MissionDataStorage::GetMissionSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot, bool isLowResolution)
{
#ifdef SUPPORT_GRAPHICS
    if (GetCachedSnapshot(missionId, missionSnapshot)) {
        if (isLowResolution) {
            missionSnapshot.snapshot = GetReducedPixelMap(missionSnapshot.snapshot);
        }
        HILOG_INFO("snapshot: GetMissionSnapshot from cache, missionId = %{public}d", missionId);
        return true;
    }

    auto pixelMap = GetPixelMap(missionId, isLowResolution);
    if (!pixelMap) {
        HILOG_ERROR("%{public}s: GetPixelMap failed.", __func__);
        return false;
    }
    missionSnapshot.snapshot = std::move(pixelMap);
#endif
    return true;
}

std::string MissionDataStorage::GetMissionDataDirPath() const
{
    return std::string(TASK_DATA_FILE_BASE_PATH) + "/" + std::to_string(userId_) + "/"
        + std::string(MISSION_DATA_FILE_PATH);
}

std::string MissionDataStorage::GetMissionDataFilePath(int missionId)
{
    return GetMissionDataDirPath() + "/"
        + MISSION_JSON_FILE_PREFIX + "_" + std::to_string(missionId) + JSON_FILE_SUFFIX;
}

std::string MissionDataStorage::GetMissionSnapshotPath(int32_t missionId, bool isLowResolution) const
{
    std::string filePath = GetMissionDataDirPath() + FILE_SEPARATOR + MISSION_JSON_FILE_PREFIX +
        UNDERLINE_SEPARATOR + std::to_string(missionId);
    if (isLowResolution) {
        filePath = filePath + UNDERLINE_SEPARATOR + LOW_RESOLUTION_FLAG;
    }
    filePath = filePath + JPEG_FILE_SUFFIX;
    return filePath;
}

bool MissionDataStorage::CheckFileNameValid(const std::string &fileName)
{
    std::string fileNameExcludePath = OHOS::ExtractFileName(fileName);
    if (fileNameExcludePath.find(MISSION_JSON_FILE_PREFIX) != 0) {
        return false;
    }

    if (fileNameExcludePath.find("_") != std::string(MISSION_JSON_FILE_PREFIX).length()) {
        return false;
    }

    if (fileNameExcludePath.find(JSON_FILE_SUFFIX) != fileNameExcludePath.length()
        - std::string(JSON_FILE_SUFFIX).length()) {
        return false;
    }

    size_t missionIdLength = fileNameExcludePath.find(JSON_FILE_SUFFIX) - fileNameExcludePath.find("_") - 1;
    std::string missionId = fileNameExcludePath.substr(fileNameExcludePath.find("_") + 1, missionIdLength);
    for (auto ch : missionId) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}

#ifdef SUPPORT_GRAPHICS
void MissionDataStorage::SaveSnapshotFile(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
    SaveSnapshotFile(missionId, missionSnapshot.snapshot, missionSnapshot.isPrivate, false);
    SaveSnapshotFile(missionId, GetReducedPixelMap(missionSnapshot.snapshot), missionSnapshot.isPrivate, true);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->CompleteSaveSnapshot(missionId);
}

void MissionDataStorage::SaveSnapshotFile(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap>& snapshot,
    bool isPrivate, bool isLowResolution)
{
    if (!snapshot) {
        return;
    }

    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    std::string dirPath = OHOS::ExtractFilePath(filePath);
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("snapshot: create dir %{public}s failed.", dirPath.c_str());
            return;
        }
        chmod(dirPath.c_str(), MODE);
    }

    if (isPrivate) {
        HILOG_DEBUG("snapshot: the param isPrivate is true.");
        ssize_t dataLength = snapshot->GetWidth() * snapshot->GetHeight() * RGB888_PIXEL_BYTES;
        uint8_t* data = (uint8_t*) malloc(dataLength);
        if (memset_s(data, dataLength, 0xff, dataLength) == EOK) {
            WriteRgb888ToJpeg(filePath.c_str(), snapshot->GetWidth(), snapshot->GetHeight(), data);
        }
        free(data);
    } else {
        if (snapshot->GetPixelFormat() == Media::PixelFormat::RGB_565) {
            SaveRGB565Image(snapshot, filePath.c_str());
        } else if (snapshot->GetPixelFormat() == Media::PixelFormat::RGBA_8888) {
            SaveRGBA8888Image(snapshot, filePath.c_str());
        } else if (snapshot->GetPixelFormat() == Media::PixelFormat::RGB_888) {
            WriteRgb888ToJpeg(filePath.c_str(), snapshot->GetWidth(), snapshot->GetHeight(), snapshot->GetPixels());
        } else {
            HILOG_ERROR("snapshot: invalid pixel format.");
        }
    }
}

std::shared_ptr<OHOS::Media::PixelMap> MissionDataStorage::GetReducedPixelMap(
    const std::shared_ptr<OHOS::Media::PixelMap>& snapshot)
{
    if (!snapshot) {
        return nullptr;
    }

    OHOS::Media::InitializationOptions options;
    options.size.width = snapshot->GetWidth() / SCALE;
    options.size.height = snapshot->GetHeight() / SCALE;
    std::unique_ptr<OHOS::Media::PixelMap> reducedPixelMap = OHOS::Media::PixelMap::Create(*snapshot, options);
    return std::shared_ptr<OHOS::Media::PixelMap>(reducedPixelMap.release());
}

bool MissionDataStorage::GetCachedSnapshot(int32_t missionId, MissionSnapshot& missionSnapshot)
{
    std::lock_guard<std::mutex> lock(cachedPixelMapMutex_);
    auto pixelMap = cachedPixelMap_.find(missionId);
    if (pixelMap != cachedPixelMap_.end()) {
        missionSnapshot.snapshot = pixelMap->second;
        return true;
    }
    return false;
}

bool MissionDataStorage::SaveCachedSnapshot(int32_t missionId, const MissionSnapshot& missionSnapshot)
{
    std::lock_guard<std::mutex> lock(cachedPixelMapMutex_);
    auto result = cachedPixelMap_.insert_or_assign(missionId, missionSnapshot.snapshot);
    if (!result.second) {
        HILOG_ERROR("snapshot: save snapshot cache failed, missionId = %{public}d", missionId);
        return false;
    }
    return true;
}

bool MissionDataStorage::DeleteCachedSnapshot(int32_t missionId)
{
    std::lock_guard<std::mutex> lock(cachedPixelMapMutex_);
    auto result = cachedPixelMap_.erase(missionId);
    if (result != 1) {
        HILOG_ERROR("snapshot: delete snapshot cache failed, missionId = %{public}d", missionId);
        return false;
    }
    return true;
}

void MissionDataStorage::DeleteMissionSnapshot(int32_t missionId, bool isLowResolution)
{
    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    std::string dirPath = OHOS::ExtractFilePath(filePath);
    if (!OHOS::FileExists(filePath)) {
        HILOG_WARN("snapshot: remove snapshot file %{public}s failed, file not exists", filePath.c_str());
        return;
    }
    bool removeResult = OHOS::RemoveFile(filePath);
    if (!removeResult) {
        HILOG_ERROR("snapshot: remove snapshot file %{public}s failed.", filePath.c_str());
    }
}

std::shared_ptr<Media::PixelMap> MissionDataStorage::GetSnapshot(int missionId, bool isLowResolution) const
{
    auto pixelMapPtr = GetPixelMap(missionId, isLowResolution);
    if (!pixelMapPtr) {
        HILOG_ERROR("%{public}s: GetPixelMap failed.", __func__);
        return nullptr;
    }
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}

std::unique_ptr<Media::PixelMap> MissionDataStorage::GetPixelMap(int missionId, bool isLowResolution) const
{
    std::string filePath = GetMissionSnapshotPath(missionId, isLowResolution);
    if (!OHOS::FileExists(filePath)) {
        HILOG_INFO("snapshot: storage snapshot not exists, missionId = %{public}d", missionId);
        return nullptr;
    }
    uint32_t errCode = 0;
    Media::SourceOptions sourceOptions;
    auto imageSource = Media::ImageSource::CreateImageSource(filePath, sourceOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS) {
        HILOG_ERROR("snapshot: CreateImageSource failed, errCode = %{public}d", errCode);
        return nullptr;
    }
    Media::DecodeOptions decodeOptions;
    decodeOptions.allocatorType = Media::AllocatorType::SHARE_MEM_ALLOC;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOptions, errCode);
    if (errCode != OHOS::Media::SUCCESS) {
        HILOG_ERROR("snapshot: CreatePixelMap failed, errCode = %{public}d", errCode);
        return nullptr;
    }
    return pixelMapPtr;
}

void MissionDataStorage::WriteRgb888ToJpeg(const char* fileName, uint32_t width, uint32_t height, const uint8_t* data)
{
    if (data == nullptr) {
        HILOG_ERROR("snapshot: data error, nullptr!\n");
        return;
    }

    FILE *file = fopen(fileName, "wb");
    if (file == nullptr) {
        HILOG_ERROR("snapshot: open file [%s] error, nullptr!\n", fileName);
        return;
    }

    struct jpeg_compress_struct jpeg;
    struct mission_error_mgr jerr;
    jpeg.err = jpeg_std_error(&jerr);
    jerr.error_exit = mission_error_exit;
    if (setjmp(jerr.environment)) {
        jpeg_destroy_compress(&jpeg);
        (void)fclose(file);
        file = nullptr;
        HILOG_ERROR("snapshot: lib jpeg exit with error!");
        return;
    }

    jpeg_create_compress(&jpeg);
    jpeg.image_width = width;
    jpeg.image_height = height;
    jpeg.input_components = RGB888_PIXEL_BYTES;
    jpeg.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jpeg);

    constexpr int32_t quality = 75;
    jpeg_set_quality(&jpeg, quality, TRUE);

    jpeg_stdio_dest(&jpeg, file);
    jpeg_start_compress(&jpeg, TRUE);
    JSAMPROW rowPointer[1];
    for (uint32_t i = 0; i < jpeg.image_height; i++) {
        rowPointer[0] = const_cast<uint8_t *>(data + i * jpeg.image_width * RGB888_PIXEL_BYTES);
        (void)jpeg_write_scanlines(&jpeg, rowPointer, 1);
    }

    jpeg_finish_compress(&jpeg);
    (void)fclose(file);
    file = nullptr;
    jpeg_destroy_compress(&jpeg);
}

// only valid for little-endian order.
bool MissionDataStorage::RGB565ToRGB888(const uint16_t *rgb565Buf, int32_t rgb565Size,
    uint8_t *rgb888Buf, int32_t rgb888Size)
{
    if (rgb565Buf == nullptr || rgb565Size <= 0 || rgb888Buf == nullptr || rgb888Size <= 0) {
        HILOG_ERROR("%{public}s: params are invalid.", __func__);
        return false;
    }

    if (rgb888Size < rgb565Size * RGB888_PIXEL_BYTES) {
        HILOG_ERROR("%{public}s: rgb888Size are invalid.", __func__);
        return false;
    }

    for (int32_t i = 0; i < rgb565Size; i++) {
        rgb888Buf[i * RGB888_PIXEL_BYTES + R_INDEX] = (rgb565Buf[i] & RGB565_MASK_RED);
        rgb888Buf[i * RGB888_PIXEL_BYTES + G_INDEX] = (rgb565Buf[i] & RGB565_MASK_GREEN) >> SHIFT_5_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + B_INDEX] = (rgb565Buf[i] & RGB565_MASK_BLUE) >> SHIFT_11_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + R_INDEX] <<= SHITF_3_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + G_INDEX] <<= SHIFT_2_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + B_INDEX] <<= SHITF_3_BIT;
    }

    return true;
}

bool MissionDataStorage::RGBA8888ToRGB888(const uint32_t *rgba8888Buf, int32_t rgba8888Size,
    uint8_t *rgb888Buf, int32_t rgb888Size)
{
    if (rgba8888Buf == nullptr || rgba8888Size <= 0 || rgb888Buf == nullptr || rgb888Size <= 0) {
        HILOG_ERROR("%{public}s: params are invalid.", __func__);
        return false;
    }

    if (rgb888Size < rgba8888Size * RGB888_PIXEL_BYTES) {
        HILOG_ERROR("%{public}s: rgb888Size are invalid.", __func__);
        return false;
    }

    for (int32_t i = 0; i < rgba8888Size; i++) {
        rgb888Buf[i * RGB888_PIXEL_BYTES + R_INDEX] = (rgba8888Buf[i] & RGBA8888_MASK_RED) >> SHIFT_16_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + G_INDEX] = (rgba8888Buf[i] & RGBA8888_MASK_GREEN) >> SHIFT_8_BIT;
        rgb888Buf[i * RGB888_PIXEL_BYTES + B_INDEX] = rgba8888Buf[i] & RGBA8888_MASK_BLUE;
    }

    return true;
}

void MissionDataStorage::SaveRGB565Image(const std::shared_ptr<Media::PixelMap> &frame, const char* fileName)
{
    HILOG_DEBUG("%{public}s was called.", __func__);
    int32_t rgb888Size = (frame->GetByteCount() / RGB565_PIXEL_BYTES) * RGB888_PIXEL_BYTES;
    uint8_t *rgb888 = new uint8_t[rgb888Size];
    const uint16_t *rgb565Data = reinterpret_cast<const uint16_t *>(frame->GetPixels());
    auto ret = RGB565ToRGB888(rgb565Data, frame->GetByteCount() / RGB565_PIXEL_BYTES, rgb888, rgb888Size);
    if (ret) {
        HILOG_DEBUG("snapshot: convert rgb565 to rgb888 successfully.");
        WriteRgb888ToJpeg(fileName, frame->GetWidth(), frame->GetHeight(), rgb888);
    }
    delete [] rgb888;
}

void MissionDataStorage::SaveRGBA8888Image(const std::shared_ptr<Media::PixelMap> &frame, const char* fileName)
{
    HILOG_DEBUG("%{public}s was called.", __func__);
    int32_t rgb888Size = (frame->GetByteCount() / RGBA8888_PIXEL_BYTES) * RGB888_PIXEL_BYTES;
    uint8_t *rgb888 = new uint8_t[rgb888Size];
    const uint32_t *rgba8888Data = reinterpret_cast<const uint32_t *>(frame->GetPixels());
    auto ret = RGBA8888ToRGB888(rgba8888Data, frame->GetByteCount() / RGBA8888_PIXEL_BYTES, rgb888, rgb888Size);
    if (ret) {
        HILOG_DEBUG("snapshot: convert rgba8888 to rgb888 successfully.");
        WriteRgb888ToJpeg(fileName, frame->GetWidth(), frame->GetHeight(), rgb888);
    }
    delete [] rgb888;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
