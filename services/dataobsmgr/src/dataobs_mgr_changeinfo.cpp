/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <limits>
#include "dataobs_mgr_changeinfo.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {
bool ChangeInfo::Marshalling(const ChangeInfo &input, MessageParcel &data)
{
    if (!data.WriteUint32(static_cast<uint32_t>(input.changeType_))) {
        return false;
    }

    if (input.uris_.size() > std::numeric_limits<uint32_t>::max() ||
        !data.WriteUint32(static_cast<uint32_t>(input.uris_.size()))) {
        return false;
    }

    for (auto const &uri : input.uris_) {
        if (!data.WriteString(uri.ToString())) {
            return false;
        }
    }

    if (!data.WriteUint32(input.size_)) {
        return false;
    }

    return data.WriteBuffer(input.data_, input.size_);
}

bool ChangeInfo::Unmarshalling(ChangeInfo &output, MessageParcel &parcel)
{
    uint32_t changeType;
    if (!parcel.ReadUint32(changeType) || changeType >= INVAILD) {
        return false;
    }

    uint32_t len = 0;
    if (!parcel.ReadUint32(len)) {
        return false;
    }

    std::list<Uri> uris;
    for (uint32_t i = 0; i < len; i++) {
        Uri uri = Uri(parcel.ReadString());
        if (uri.ToString().empty()) {
            return false;
        }
        uris.emplace_back(std::move(uri));
    }

    uint32_t size = 0;
    if (!parcel.ReadUint32(size)) {
        return false;
    }

    auto data = parcel.ReadBuffer(size);
    if (data == nullptr) {
        return false;
    }
    output.changeType_ = static_cast<ChangeType>(changeType);
    std::swap(output.uris_, uris);
    output.data_ = static_cast<const void *>(data);
    output.size_ = size;
    return true;
}
} // namespace AAFwk
} // namespace OHOS