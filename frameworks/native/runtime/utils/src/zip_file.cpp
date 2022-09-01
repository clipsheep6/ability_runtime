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

#include "zip_file.h"

#include <cassert>
#include <cstring>
#include <ostream>

#include "hilog_wrapper.h"
#include "securec.h"
#include "zlib.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint32_t MAX_FILE_NAME = 256;
constexpr uint32_t UNZIP_BUFFER_SIZE = 1024;
constexpr uint32_t UNZIP_BUF_IN_LEN = 160 * UNZIP_BUFFER_SIZE;   // in  buffer length: 160KB
constexpr uint32_t UNZIP_BUF_OUT_LEN = 320 * UNZIP_BUFFER_SIZE;  // out buffer length: 320KB
constexpr uint32_t LOCAL_HEADER_SIGNATURE = 0x04034b50;
constexpr uint32_t CENTRAL_SIGNATURE = 0x02014b50;
constexpr uint32_t EOCD_SIGNATURE = 0x06054b50;
constexpr uint32_t DATA_DESC_SIGNATURE = 0x08074b50;
constexpr uint32_t FLAG_DATA_DESC = 0x8;
constexpr size_t FILE_READ_COUNT = 1;
constexpr uint8_t INFLATE_ERROR_TIMES = 5;
const char FILE_SEPARATOR_CHAR = '/';
constexpr char EXT_NAME_ABC[] = ".abc";

inline bool StringEndWith(const std::string& str, const char* endStr, size_t endStrLen)
{
    size_t len = str.length();
    return ((len >= endStrLen) && (str.compare(len - endStrLen, endStrLen, endStr) == 0));
}
}  // namespace

ZipEntry::ZipEntry(const CentralDirEntry &centralEntry)
{
    compressionMethod = centralEntry.compressionMethod;
    uncompressedSize = centralEntry.uncompressedSize;
    compressedSize = centralEntry.compressedSize;
    localHeaderOffset = centralEntry.localHeaderOffset;
    crc = centralEntry.crc;
    flags = centralEntry.flags;
}

ZipFile::ZipFile(const std::string &pathName) : pathName_(pathName)
{}

ZipFile::~ZipFile()
{
    Close();
}

void ZipFile::SetContentLocation(const ZipPos start, const size_t length)
{
    fileStartPos_ = start;
    fileLength_ = length;
}

bool ZipFile::CheckEndDir(const EndDir &endDir) const
{
    size_t lenEndDir = sizeof(EndDir);
    if ((endDir.numDisk != 0) || (endDir.signature != EOCD_SIGNATURE) || (endDir.startDiskOfCentralDir != 0) ||
        (endDir.offset >= fileLength_) || (endDir.totalEntriesInThisDisk != endDir.totalEntries) ||
        (endDir.commentLen != 0) ||
        // central dir can't overlap end of central dir
        ((endDir.offset + endDir.sizeOfCentralDir + lenEndDir) > fileLength_)) {
        HILOG_ERROR("end dir format error");
        return false;
    }
    return true;
}

bool ZipFile::ParseEndDirectory()
{
    size_t endDirLen = sizeof(EndDir);
    size_t endFilePos = fileStartPos_ + fileLength_;

    if (fileLength_ <= endDirLen) {
        HILOG_ERROR("parse EOCD file length(%{public}llu) <= end dir length(%{public}llu)", fileStartPos_, fileLength_);
        return false;
    }

    size_t eocdPos = endFilePos - endDirLen;
    if (fseek(file_, eocdPos, SEEK_SET) != 0) {
        HILOG_ERROR("locate EOCD seek failed, error: %{public}d", errno);
        return false;
    }

    if (fread(&endDir_, sizeof(EndDir), FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
        HILOG_ERROR("read EOCD struct failed, error: %{public}d", errno);
        return false;
    }

    centralDirPos_ = endDir_.offset + fileStartPos_;

    return CheckEndDir(endDir_);
}

bool ZipFile::ParseAllEntries()
{
    bool ret = true;
    ZipPos currentPos = centralDirPos_;
    CentralDirEntry directoryEntry = {0};
    size_t fileLength = 0;

    for (uint16_t i = 0; i < endDir_.totalEntries; i++) {
        std::string fileName;
        fileName.reserve(MAX_FILE_NAME);
        fileName.resize(MAX_FILE_NAME - 1);

        if (fseek(file_, currentPos, SEEK_SET) != 0) {
            HILOG_ERROR("parse entry(%{public}d) seek zipEntry failed, error: %{public}d", i, errno);
            ret = false;
            break;
        }

        if (fread(&directoryEntry, sizeof(CentralDirEntry), FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
            HILOG_ERROR("parse entry(%{public}d) read ZipEntry failed, error: %{public}d", i, errno);
            ret = false;
            break;
        }

        if (directoryEntry.signature != CENTRAL_SIGNATURE) {
            HILOG_ERROR("parse entry(%{public}d) check signature(0x%08x) at pos(0x%08llx) failed",
                i,
                directoryEntry.signature,
                currentPos);
            ret = false;
            break;
        }

        fileLength = (directoryEntry.nameSize >= MAX_FILE_NAME) ? (MAX_FILE_NAME - 1) : directoryEntry.nameSize;
        if (fread(&(fileName[0]), fileLength, FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
            HILOG_ERROR("parse entry(%{public}d) read file name failed, error: %{public}d", i, errno);
            ret = false;
            break;
        }
        fileName.resize(fileLength);

        if (isRuntime_ && !StringEndWith(fileName, EXT_NAME_ABC, sizeof(EXT_NAME_ABC) - 1)) {
            currentPos += sizeof(directoryEntry);
            currentPos += directoryEntry.nameSize + directoryEntry.extraSize + directoryEntry.commentSize;
            continue;
        }

        ZipEntry currentEntry(directoryEntry);
        currentEntry.fileName = fileName;
        entriesMap_[fileName] = currentEntry;

        currentPos += sizeof(directoryEntry);
        currentPos += directoryEntry.nameSize + directoryEntry.extraSize + directoryEntry.commentSize;
    }
    return ret;
}

bool ZipFile::Open()
{
    if (isOpen_) {
        HILOG_ERROR("has already opened");
        return true;
    }

    if (pathName_.length() > PATH_MAX) {
        HILOG_ERROR("path length(%{public}u) longer than max path length(%{public}d)",
            static_cast<unsigned int>(pathName_.length()),
            PATH_MAX);
        return false;
    }
    std::string realPath;
    realPath.reserve(PATH_MAX);
    realPath.resize(PATH_MAX - 1);
    if (realpath(pathName_.c_str(), &(realPath[0])) == nullptr) {
        HILOG_ERROR("transform real path error: %{public}d", errno);
        return false;
    }

    FILE *tmpFile = fopen(realPath.c_str(), "rb");
    if (tmpFile == nullptr) {
        HILOG_ERROR("open file(%{private}s) failed, error: %{public}d", pathName_.c_str(), errno);
        return false;
    }

    if (fileLength_ == 0) {
        if (fseek(tmpFile, 0, SEEK_END) != 0) {
            HILOG_ERROR("file seek failed, error: %{public}d", errno);
            fclose(tmpFile);
            return false;
        }
        int64_t fileLength = ftell(tmpFile);
        if (fileLength == -1) {
            HILOG_ERROR("open file %{private}s failed", pathName_.c_str());
            fclose(tmpFile);
            return false;
        }
        fileLength_ = static_cast<ZipPos>(fileLength);
        if (fileStartPos_ >= fileLength_) {
            HILOG_ERROR("open start pos > length failed");
            fclose(tmpFile);
            return false;
        }

        fileLength_ -= fileStartPos_;
    }

    file_ = tmpFile;
    bool result = ParseEndDirectory();
    if (result) {
        result = ParseAllEntries();
    }
    // it means open file success.
    isOpen_ = true;
    return result;
}

void ZipFile::Close()
{
    if (!isOpen_ || file_ == nullptr) {
        HILOG_WARN("file is not opened");
        return;
    }

    entriesMap_.clear();
    pathName_ = "";
    isOpen_ = false;

    if (fclose(file_) != 0) {
        HILOG_WARN("close failed, error: %{public}d", errno);
    }
    file_ = nullptr;
}

// Get all file zipEntry in this file
const ZipEntryMap &ZipFile::GetAllEntries() const
{
    return entriesMap_;
}

bool ZipFile::HasEntry(const std::string &entryName) const
{
    return entriesMap_.find(entryName) != entriesMap_.end();
}

bool ZipFile::IsDirExist(const std::string &dir) const
{
    if (dir.empty()) {
        HILOG_ERROR("target dir is empty");
        return false;
    }

    auto tempDir = dir;
    if (tempDir.back() != FILE_SEPARATOR_CHAR) {
        tempDir.push_back(FILE_SEPARATOR_CHAR);
    }

    for (const auto &item : entriesMap_) {
        if (item.first.find(tempDir) == 0) {
            return true;
        }
    }
    HILOG_WARN("target dir not found, dir : %{private}s", dir.c_str());
    return false;
}

bool ZipFile::GetEntry(const std::string &entryName, ZipEntry &resultEntry) const
{
    auto iter = entriesMap_.find(entryName);
    if (iter != entriesMap_.end()) {
        resultEntry = iter->second;
        return true;
    }
    HILOG_ERROR("get entry %{public}s failed", entryName.c_str());
    return false;
}

size_t ZipFile::GetLocalHeaderSize(const uint16_t nameSize, const uint16_t extraSize) const
{
    return sizeof(LocalHeader) + nameSize + extraSize;
}

bool ZipFile::CheckDataDesc(const ZipEntry &zipEntry, const LocalHeader &localHeader) const
{
    uint32_t crcLocal = 0;
    uint32_t compressedLocal = 0;
    uint32_t uncompressedLocal = 0;

    if (localHeader.flags & FLAG_DATA_DESC) {  // use data desc
        DataDesc dataDesc;
        auto descPos = zipEntry.localHeaderOffset + GetLocalHeaderSize(localHeader.nameSize, localHeader.extraSize);
        descPos += fileStartPos_ + zipEntry.compressedSize;

        if (fseek(file_, descPos, SEEK_SET) != 0) {
            HILOG_ERROR("check local header seek datadesc failed, error: %{public}d", errno);
            return false;
        }

        if (fread(&dataDesc, sizeof(DataDesc), FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
            HILOG_ERROR("check local header read datadesc failed, error: %{public}d", errno);
            return false;
        }

        if (dataDesc.signature != DATA_DESC_SIGNATURE) {
            HILOG_ERROR("check local header check datadesc signature failed");
            return false;
        }

        crcLocal = dataDesc.crc;
        compressedLocal = dataDesc.compressedSize;
        uncompressedLocal = dataDesc.uncompressedSize;
    } else {
        crcLocal = localHeader.crc;
        compressedLocal = localHeader.compressedSize;
        uncompressedLocal = localHeader.uncompressedSize;
    }

    if ((zipEntry.crc != crcLocal) || (zipEntry.compressedSize != compressedLocal) ||
        (zipEntry.uncompressedSize != uncompressedLocal)) {
        HILOG_ERROR("check local header compressed size corrupted");
        return false;
    }

    return true;
}

bool ZipFile::CheckCoherencyLocalHeader(const ZipEntry &zipEntry, uint16_t &extraSize) const
{
    LocalHeader localHeader = {0};

    if (zipEntry.localHeaderOffset >= fileLength_) {
        HILOG_ERROR("check local file header offset is overflow %{public}d", zipEntry.localHeaderOffset);
        return false;
    }

    if (fseek(file_, fileStartPos_ + zipEntry.localHeaderOffset, SEEK_SET) != 0) {
        HILOG_ERROR("check local header seek failed, error: %{public}d", errno);
        return false;
    }

    if (fread(&localHeader, sizeof(LocalHeader), FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
        HILOG_ERROR("check local header read localheader failed, error: %{public}d", errno);
        return false;
    }

    if ((localHeader.signature != LOCAL_HEADER_SIGNATURE) ||
        (zipEntry.compressionMethod != localHeader.compressionMethod)) {
        HILOG_ERROR("check local header signature or compressionMethod failed");
        return false;
    }

    // current only support store and Z_DEFLATED method
    if ((zipEntry.compressionMethod != Z_DEFLATED) && (zipEntry.compressionMethod != 0)) {
        HILOG_ERROR("check local header compressionMethod(%{public}d) not support", zipEntry.compressionMethod);
        return false;
    }

    std::string fileName;
    fileName.reserve(MAX_FILE_NAME);
    fileName.resize(MAX_FILE_NAME - 1);
    size_t fileLength = (localHeader.nameSize >= MAX_FILE_NAME) ? (MAX_FILE_NAME - 1) : localHeader.nameSize;
    if (fileLength != zipEntry.fileName.length()) {
        HILOG_ERROR("check local header file name size failed");
        return false;
    }
    if (fread(&(fileName[0]), fileLength, FILE_READ_COUNT, file_) != FILE_READ_COUNT) {
        HILOG_ERROR("check local header read file name failed, error: %{public}d", errno);
        return false;
    }
    fileName.resize(fileLength);
    if (zipEntry.fileName != fileName) {
        HILOG_ERROR("check local header file name corrupted");
        return false;
    }

    if (!CheckDataDesc(zipEntry, localHeader)) {
        HILOG_ERROR("check data desc failed");
        return false;
    }

    extraSize = localHeader.extraSize;
    return true;
}

bool ZipFile::SeekToEntryStart(const ZipEntry &zipEntry, const uint16_t extraSize) const
{
    ZipPos startOffset = zipEntry.localHeaderOffset;
    // get data offset, add signature+localheader+namesize+extrasize
    startOffset += GetLocalHeaderSize(zipEntry.fileName.length(), extraSize);
    if (startOffset + zipEntry.compressedSize > fileLength_) {
        HILOG_ERROR("startOffset(%{public}lld)+entryCompressedSize(%{public}ud) > fileLength(%{public}llu)",
            startOffset,
            zipEntry.compressedSize,
            fileLength_);
        return false;
    }
    startOffset += fileStartPos_;  // add file start relative to file stream

    if (fseek(file_, startOffset, SEEK_SET) != 0) {
        HILOG_ERROR("seek failed, error: %{public}d", errno);
        return false;
    }
    return true;
}

bool ZipFile::UnzipWithStore(const ZipEntry &zipEntry, const uint16_t extraSize, std::ostream &dest) const
{
    if (!SeekToEntryStart(zipEntry, extraSize)) {
        HILOG_ERROR("seek to entry start failed");
        return false;
    }

    uint32_t remainSize = zipEntry.compressedSize;
    std::string readBuffer;
    readBuffer.reserve(UNZIP_BUF_OUT_LEN);
    readBuffer.resize(UNZIP_BUF_OUT_LEN - 1);
    while (remainSize > 0) {
        size_t readBytes;
        size_t readLen = (remainSize > UNZIP_BUF_OUT_LEN) ? UNZIP_BUF_OUT_LEN : remainSize;
        readBytes = fread(&(readBuffer[0]), sizeof(Byte), readLen, file_);
        if (readBytes == 0) {
            HILOG_ERROR("unzip store read failed, error: %{public}d", ferror(file_));
            return false;
        }
        remainSize -= readBytes;
        dest.write(&(readBuffer[0]), readBytes);
    }

    return true;
}

bool ZipFile::InitZStream(z_stream &zstream) const
{
    // init zlib stream
    if (memset_s(&zstream, sizeof(z_stream), 0, sizeof(z_stream))) {
        HILOG_ERROR("unzip stream buffer init failed");
        return false;
    }
    int32_t zlibErr = inflateInit2(&zstream, -MAX_WBITS);
    if (zlibErr != Z_OK) {
        HILOG_ERROR("unzip inflated init failed");
        return false;
    }

    BytePtr bufOut = new (std::nothrow) Byte[UNZIP_BUF_OUT_LEN];
    if (bufOut == nullptr) {
        HILOG_ERROR("unzip inflated new out buffer failed");
        return false;
    }

    BytePtr bufIn = new (std::nothrow) Byte[UNZIP_BUF_IN_LEN];
    if (bufIn == nullptr) {
        HILOG_ERROR("unzip inflated new in buffer failed");
        delete[] bufOut;
        return false;
    }
    zstream.next_out = bufOut;
    zstream.next_in = bufIn;
    zstream.avail_out = UNZIP_BUF_OUT_LEN;
    return true;
}

bool ZipFile::ReadZStream(const BytePtr &buffer, z_stream &zstream, uint32_t &remainCompressedSize) const
{
    if (zstream.avail_in == 0) {
        size_t readBytes;
        size_t remainBytes = (remainCompressedSize > UNZIP_BUF_IN_LEN) ? UNZIP_BUF_IN_LEN : remainCompressedSize;
        readBytes = fread(buffer, sizeof(Byte), remainBytes, file_);
        if (readBytes == 0) {
            HILOG_ERROR("unzip inflated read failed, error: %{public}d", ferror(file_));
            return false;
        }

        remainCompressedSize -= readBytes;
        zstream.avail_in = readBytes;
        zstream.next_in = buffer;
    }
    return true;
}

bool ZipFile::UnzipWithInflated(const ZipEntry &zipEntry, const uint16_t extraSize, std::ostream &dest) const
{
    z_stream zstream;
    if (!SeekToEntryStart(zipEntry, extraSize) || !InitZStream(zstream)) {
        return false;
    }

    BytePtr bufIn = zstream.next_in;
    BytePtr bufOut = zstream.next_out;

    bool ret = true;
    int32_t zlibErr = Z_OK;
    uint32_t remainCompressedSize = zipEntry.compressedSize;
    size_t inflateLen = 0;
    uint8_t errorTimes = 0;
    while ((remainCompressedSize > 0) || (zstream.avail_in > 0)) {
        if (!ReadZStream(bufIn, zstream, remainCompressedSize)) {
            ret = false;
            break;
        }

        zlibErr = inflate(&zstream, Z_SYNC_FLUSH);
        if ((zlibErr >= Z_OK) && (zstream.msg != nullptr)) {
            HILOG_ERROR("unzip inflated inflate, error: %{public}d, err msg: %{public}s", zlibErr, zstream.msg);
            ret = false;
            break;
        }

        inflateLen = UNZIP_BUF_OUT_LEN - zstream.avail_out;
        if (inflateLen > 0) {
            dest.write((const char *)bufOut, inflateLen);
            zstream.next_out = bufOut;
            zstream.avail_out = UNZIP_BUF_OUT_LEN;
            errorTimes = 0;
        } else {
            errorTimes++;
        }
        if (errorTimes >= INFLATE_ERROR_TIMES) {
            HILOG_ERROR("unzip inflated data is abnormal!");
            ret = false;
            break;
        }
    }

    // free all dynamically allocated data structures except the next_in and next_out for this stream.
    zlibErr = inflateEnd(&zstream);
    if (zlibErr != Z_OK) {
        HILOG_ERROR("unzip inflateEnd error, error: %{public}d", zlibErr);
        ret = false;
    }

    delete[] bufOut;
    delete[] bufIn;
    return ret;
}

ZipPos ZipFile::GetEntryDataOffset(const ZipEntry &zipEntry, const uint16_t extraSize) const
{
    // get entry data offset relative file
    ZipPos offset = zipEntry.localHeaderOffset;

    offset += GetLocalHeaderSize(zipEntry.fileName.length(), extraSize);
    offset += fileStartPos_;

    return offset;
}

bool ZipFile::GetDataOffsetRelative(const std::string &file, ZipPos &offset, uint32_t &length) const
{
    ZipEntry zipEntry;
    if (!GetEntry(file, zipEntry)) {
        HILOG_ERROR("extract file: not find file");
        return false;
    }

    uint16_t extraSize = 0;
    if (!CheckCoherencyLocalHeader(zipEntry, extraSize)) {
        HILOG_ERROR("check coherency local header failed");
        return false;
    }

    offset = GetEntryDataOffset(zipEntry, extraSize);
    length = zipEntry.compressedSize;
    return true;
}

bool ZipFile::ExtractFile(const std::string &file, std::ostream &dest) const
{
    ZipEntry zipEntry;
    if (!GetEntry(file, zipEntry)) {
        HILOG_ERROR("extract file: not find file");
        return false;
    }

    uint16_t extraSize = 0;
    if (!CheckCoherencyLocalHeader(zipEntry, extraSize)) {
        HILOG_ERROR("check coherency local header failed");
        return false;
    }

    bool ret = true;
    if (zipEntry.compressionMethod == 0) {
        ret = UnzipWithStore(zipEntry, extraSize, dest);
    } else {
        ret = UnzipWithInflated(zipEntry, extraSize, dest);
    }

    return ret;
}

void ZipFile::SetIsRuntime(const bool isRuntime)
{
    isRuntime_ = isRuntime;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
