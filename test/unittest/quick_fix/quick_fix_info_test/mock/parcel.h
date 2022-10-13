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

#ifndef MOCK_OHOS_UTILS_PARCEL_H
#define MOCK_OHOS_UTILS_PARCEL_H

#include <string>

namespace OHOS {
void MockWriteUint32(bool state);

void MockWriteString(bool state, uint8_t count);

void MockWriteParcelable(bool state);

void MockReadParcelable(bool state);

void ResetParcelState();

bool GetMockReadParcelableState();

class Parcelable;
class Parcel {
public:
    Parcel() {}
    virtual ~Parcel() = default;

    bool WriteUint32(uint32_t) const;

    bool WriteString(const std::string &) const;

    bool WriteParcelable(const Parcelable *) const;

    uint32_t ReadUint32() const;

    std::string ReadString() const;

    template <typename T>
    T *ReadParcelable() const
    {
        if (GetMockReadParcelableState()) {
            return new T();
        }
        return nullptr;
    }
};

class Parcelable {
public:
    Parcelable() = default;
    virtual ~Parcelable() = default;
    virtual bool Marshalling(Parcel &parcel) const
    {
        return true;
    }
};
} // namespace OHOS
#endif
