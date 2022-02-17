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

#ifndef OHOS_ZIDL_FILE_H
#define OHOS_ZIDL_FILE_H

#include <cstddef>
#include <cstdio>

#include "util/string.h"

namespace OHOS {
namespace Zidl {

class File {
public:
    File(
        /* [in] */ const String& path,
        /* [in] */ int mode);

    ~File();

    inline bool IsValid()
    {
        return fd_ != nullptr;
    }

    inline String GetPath()
    {
        return path_;
    }

    char GetChar();

    char PeekChar();

    bool IsEof() const;

    inline int GetCharLineNumber() const
    {
        return lineNo_;
    }

    inline int GetCharColumnNumber() const
    {
        return columnNo_;
    }

    bool ReadData(
        /* [out] */ void* data,
        /* [in] */ size_t size);

    bool WriteData(
        /* [in] */ const void* data,
        /* [in] */ size_t size);

    void Flush();

    bool Reset();

    bool Skip(
        /* [in] */ long size);

    void Close();

    static constexpr unsigned int READ = 0x1;
    static constexpr unsigned int WRITE = 0x2;
    static constexpr unsigned int APPEND = 0x4;

private:
    int Read();

    static constexpr int BUFFER_SIZE = 1024;

    char buffer_[BUFFER_SIZE] = {0};
    size_t size_ = 0;
    size_t position_ = 0;
    size_t columnNo_ = 1;
    size_t lineNo_ = 1;
    bool isEof_ = false;
    bool isError_ = false;

    FILE* fd_ = nullptr;
    String path_;
    unsigned int mode_ = 0;
};

}
}

#endif // OHOS_ZIDL_STRING_H
