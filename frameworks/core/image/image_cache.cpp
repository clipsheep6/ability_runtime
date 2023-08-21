/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "core/image/image_cache.h"

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>

#include "core/components_ng/image_provider/image_object.h"
#include "core/image/image_object.h"

namespace OHOS::Ace {

std::shared_mutex ImageCache::cacheFilePathMutex_;
std::string ImageCache::cacheFilePath_;

std::atomic<size_t> ImageCache::fileLimit_ = 100 * 1024 * 1024; // the capacity is 100MB

std::atomic<float> ImageCache::clearCacheFileRatio_ = 0.5f; // default clear ratio is 0.5

bool ImageCache::hasSetCacheFileInfo_ = false;

std::mutex ImageCache::cacheFileSizeMutex_;
int64_t ImageCache::cacheFileSize_ = 0;

std::mutex ImageCache::cacheFileInfoMutex_;
std::list<FileInfo> ImageCache::cacheFileInfo_;

// TODO: Create a real ImageCache later
#ifdef FLUTTER_2_5
class MockImageCache : public ImageCache {
    void Clear() override {};
    RefPtr<NG::ImageData> GetDataFromCacheFile(const std::string& filePath) override
    {
        return nullptr;
    }
};

RefPtr<ImageCache> ImageCache::Create()
{
    return AceType::MakeRefPtr<MockImageCache>();
}

void ImageCache::Purge() {}
#endif

template<typename T>
void ImageCache::CacheWithCountLimitLRU(const std::string& key, const T& cacheObj, std::list<CacheNode<T>>& cacheList,
    std::unordered_map<std::string, typename std::list<CacheNode<T>>::iterator>& cache,
    const std::atomic<size_t>& capacity)
{
    auto iter = cache.find(key);
    if (iter == cache.end()) {
        if (cache.size() == capacity) {
            cache.erase(cacheList.back().cacheKey);
            cacheList.pop_back();
        }
        cacheList.emplace_front(key, cacheObj);
        cache.emplace(key, cacheList.begin());
    } else {
        iter->second->cacheObj = cacheObj;
        cacheList.splice(cacheList.begin(), cacheList, iter->second);
        iter->second = cacheList.begin();
    }
}

template<typename T>
T ImageCache::GetCacheObjWithCountLimitLRU(const std::string& key, std::list<CacheNode<T>>& cacheList,
    std::unordered_map<std::string, typename std::list<CacheNode<T>>::iterator>& cache)
{
    auto iter = cache.find(key);
    if (iter != cache.end()) {
        cacheList.splice(cacheList.begin(), cacheList, iter->second);
        iter->second = cacheList.begin();
        return iter->second->cacheObj;
    }
    return nullptr;
}

bool ImageCache::GetFromCacheFile(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(cacheFileInfoMutex_);
    return GetFromCacheFileInner(filePath);
}

bool ImageCache::GetFromCacheFileInner(const std::string& filePath)
{
    auto iter = std::find_if(cacheFileInfo_.begin(), cacheFileInfo_.end(),
        [&filePath](const FileInfo& fileInfo) { return fileInfo.filePath == filePath; });
    if (iter == cacheFileInfo_.end()) {
        return false;
    }
    iter->accessTime = time(nullptr);
    cacheFileInfo_.splice(cacheFileInfo_.end(), cacheFileInfo_, iter);
    return true;
}

void ImageCache::CacheImage(const std::string& key, const std::shared_ptr<CachedImage>& image)
{
    if (key.empty() || capacity_ == 0) {
        return;
    }
    std::scoped_lock lock(imageCacheMutex_);
    CacheWithCountLimitLRU<std::shared_ptr<CachedImage>>(key, image, cacheList_, imageCache_, capacity_);
}

std::shared_ptr<CachedImage> ImageCache::GetCacheImage(const std::string& key)
{
    std::scoped_lock lock(imageCacheMutex_);
    return GetCacheObjWithCountLimitLRU<std::shared_ptr<CachedImage>>(key, cacheList_, imageCache_);
}

void ImageCache::CacheImgObjNG(const std::string& key, const RefPtr<NG::ImageObject>& imgObj)
{
    if (key.empty() || imgObjCapacity_ == 0) {
        return;
    }
    std::scoped_lock lock(imgObjMutex_);
    CacheWithCountLimitLRU<RefPtr<NG::ImageObject>>(key, imgObj, cacheImgObjListNG_, imgObjCacheNG_, imgObjCapacity_);
}

RefPtr<NG::ImageObject> ImageCache::GetCacheImgObjNG(const std::string& key)
{
    std::scoped_lock lock(imgObjMutex_);
    return GetCacheObjWithCountLimitLRU<RefPtr<NG::ImageObject>>(key, cacheImgObjListNG_, imgObjCacheNG_);
}

void ImageCache::CacheImgObj(const std::string& key, const RefPtr<ImageObject>& imgObj)
{
    if (key.empty() || imgObjCapacity_ == 0) {
        return;
    }
    std::scoped_lock lock(imgObjMutex_);
    CacheWithCountLimitLRU<RefPtr<ImageObject>>(key, imgObj, cacheImgObjList_, imgObjCache_, imgObjCapacity_);
}

RefPtr<ImageObject> ImageCache::GetCacheImgObj(const std::string& key)
{
    std::scoped_lock lock(imgObjMutex_);
    return GetCacheObjWithCountLimitLRU<RefPtr<ImageObject>>(key, cacheImgObjList_, imgObjCache_);
}

void ImageCache::CacheImageData(const std::string& key, const RefPtr<NG::ImageData>& imageData)
{
    if (key.empty() || !imageData || dataSizeLimit_ == 0) {
        return;
    }
    std::scoped_lock lock(dataCacheMutex_);
    auto dataSize = imageData->GetSize();
    if (dataSize > (dataSizeLimit_ >> 1)) { // if data is longer than half limit, do not cache it.
        LOGW("data is %{public}d, bigger than half limit %{public}d, do not cache it", static_cast<int32_t>(dataSize),
            static_cast<int32_t>(dataSizeLimit_ >> 1));
        return;
    }
    auto iter = imageDataCache_.find(key);
    if (iter == imageDataCache_.end()) {
        if (!ProcessImageDataCacheInner(dataSize)) {
            return;
        }
        dataCacheList_.emplace_front(key, imageData);
        imageDataCache_.emplace(key, dataCacheList_.begin());
    } else {
        auto oldSize = iter->second->cacheObj->GetSize();
        if (oldSize != dataSize) {
            curDataSize_ -= oldSize;
            if (!ProcessImageDataCacheInner(dataSize)) {
                return;
            }
        }
        iter->second->cacheObj = imageData;
        dataCacheList_.splice(dataCacheList_.begin(), dataCacheList_, iter->second);
        iter->second = dataCacheList_.begin();
    }
}

bool ImageCache::ProcessImageDataCacheInner(size_t dataSize)
{
    while (dataSize + curDataSize_ > dataSizeLimit_ && !dataCacheList_.empty()) {
        curDataSize_ -= dataCacheList_.back().cacheObj->GetSize();
        imageDataCache_.erase(dataCacheList_.back().cacheKey);
        dataCacheList_.pop_back();
    }
    if (dataSize + curDataSize_ > dataSizeLimit_) {
        return false;
    }
    curDataSize_ += dataSize;
    return true;
}

RefPtr<NG::ImageData> ImageCache::GetCacheImageData(const std::string& key)
{
    std::scoped_lock lock(dataCacheMutex_);
    auto iter = imageDataCache_.find(key);
    if (iter != imageDataCache_.end()) {
        dataCacheList_.splice(dataCacheList_.begin(), dataCacheList_, iter->second);
        iter->second = dataCacheList_.begin();
        return iter->second->cacheObj;
    }
    return nullptr;
}

void ImageCache::WriteCacheFile(const std::string& url, const void* const data, size_t size, const std::string& suffix)
{
    if (size > fileLimit_) {
        LOGW("file size is %{public}d, greater than limit %{public}d, cannot cache", static_cast<int32_t>(size),
            static_cast<int32_t>(fileLimit_));
        return;
    }
    std::vector<std::string> removeVector;
    std::string cacheNetworkFilePath = GetImageCacheFilePath(url) + suffix;

    std::lock_guard<std::mutex> lock(cacheFileInfoMutex_);
    // 1. first check if file has been cached.
    if (ImageCache::GetFromCacheFileInner(cacheNetworkFilePath)) {
        LOGI("file has been wrote %{private}s", cacheNetworkFilePath.c_str());
        return;
    }

    // 2. if not in dist, write file into disk.
#ifdef WINDOWS_PLATFORM
    std::ofstream outFile(cacheNetworkFilePath, std::ios::binary);
#else
    std::ofstream outFile(cacheNetworkFilePath, std::fstream::out);
#endif
    if (!outFile.is_open()) {
        LOGW("open cache file failed, cannot write.");
        return;
    }
    outFile.write(reinterpret_cast<const char*>(data), size);
    LOGI("write image cache: %{public}s %{private}s", url.c_str(), cacheNetworkFilePath.c_str());

    cacheFileSize_ += size;
    cacheFileInfo_.emplace_back(cacheNetworkFilePath, size, time(nullptr));
    // check if cache files too big.
    if (cacheFileSize_ > static_cast<int32_t>(fileLimit_)) {
        auto removeCount = static_cast<int32_t>(cacheFileInfo_.size() * clearCacheFileRatio_);
        int32_t removeSize = 0;
        auto iter = cacheFileInfo_.begin();
        int32_t count = 0;
        while (count < removeCount) {
            removeSize += static_cast<int32_t>(iter->fileSize);
            removeVector.push_back(iter->filePath);
            ++iter;
            ++count;
        }
        cacheFileInfo_.erase(cacheFileInfo_.begin(), iter);
        cacheFileSize_ -= static_cast<int32_t>(removeSize);
    }
    // 3. clear files removed from cache list.
    ClearCacheFile(removeVector);
}

void ImageCache::ClearCacheFile(const std::vector<std::string>& removeFiles)
{
    LOGD("begin to clear %{public}zu files: ", removeFiles.size());
    for (auto&& iter : removeFiles) {
        if (remove(iter.c_str()) != 0) {
            LOGW("remove file %{private}s failed.", iter.c_str());
            continue;
        }
    }
}

void ImageCache::ClearCacheImage(const std::string& key)
{
    {
        std::scoped_lock lock(imageCacheMutex_);
        auto iter = imageCache_.find(key);
        if (iter != imageCache_.end()) {
            cacheList_.erase(iter->second);
            imageCache_.erase(iter);
        }
    }

    {
        std::scoped_lock lock(dataCacheMutex_);
        auto iter = imageDataCache_.find(key);
        if (iter != imageDataCache_.end()) {
            dataCacheList_.erase(iter->second);
            imageDataCache_.erase(iter);
        }
    }
}

void ImageCache::SetCacheFileInfo()
{
    std::lock_guard<std::mutex> lock(cacheFileInfoMutex_);
    // Set cache file information only once.
    if (hasSetCacheFileInfo_) {
        return;
    }
    std::string cacheFilePath = GetImageCacheFilePath();
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(cacheFilePath.c_str()), closedir);
    if (dir == nullptr) {
        LOGW("cache file path wrong! maybe it is not set.");
        return;
    }
    int64_t cacheFileSize = 0;
    dirent* filePtr = readdir(dir.get());
    while (filePtr != nullptr) {
        // skip . or ..
        if (filePtr->d_name[0] != '.') {
            std::string filePath = cacheFilePath + "/" + std::string(filePtr->d_name);
            struct stat fileStatus;
            if (stat(filePath.c_str(), &fileStatus) == -1) {
                filePtr = readdir(dir.get());
                continue;
            }
            cacheFileInfo_.emplace_back(filePath, fileStatus.st_size, fileStatus.st_atime);
            cacheFileSize += static_cast<int64_t>(fileStatus.st_size);
        }
        filePtr = readdir(dir.get());
    }
    cacheFileInfo_.sort();
    cacheFileSize_ = cacheFileSize;
    hasSetCacheFileInfo_ = true;
}

void ImageCache::Clear()
{
    {
        std::scoped_lock lock(imageCacheMutex_);
        cacheList_.clear();
        imageCache_.clear();
    }
    {
        std::scoped_lock lock(dataCacheMutex_);
        dataCacheList_.clear();
        imageDataCache_.clear();
    }
    {
        std::scoped_lock lock(imgObjMutex_);
        cacheImgObjListNG_.clear();
        imgObjCacheNG_.clear();
        cacheImgObjList_.clear();
        imgObjCache_.clear();
    }
}
} // namespace OHOS::Ace
