/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_UTILS_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_UTILS_H
#include <climits>
#include <list>
#include <map>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

#include "iremote_object.h"
#include "message_parcel.h"
namespace OHOS {
template<class T>
struct is_container : std::false_type {
};
template<class T>
struct is_container<std::vector<T>> : std::true_type {
};
template<class T>
struct is_container<std::list<T>> : std::true_type {
};
namespace DataObsUtils {
static inline bool Marshal(MessageParcel &data)
{
    return true;
}

static inline bool Unmarshal(MessageParcel &data)
{
    return true;
}

static inline bool Marshalling(int64_t input, MessageParcel &data)
{
    return data.WriteInt64(input);
}

static inline bool Unmarshalling(int64_t &output, MessageParcel &data)
{
    return data.ReadInt64(output);
}

static inline bool Marshalling(double input, MessageParcel &data)
{
    return data.WriteDouble(input);
}

static inline bool Unmarshalling(double &output, MessageParcel &data)
{
    return data.ReadDouble(output);
}

static inline bool Marshalling(bool input, MessageParcel &data)
{
    return data.WriteBool(input);
}

static inline bool Unmarshalling(bool &output, MessageParcel &data)
{
    return data.ReadBool(output);
}

static inline bool Marshalling(const std::monostate &input, MessageParcel &data)
{
    return true;
}

static inline bool Unmarshalling(std::monostate &output, MessageParcel &data)
{
    return true;
}

static inline bool Marshalling(const std::string &input, MessageParcel &data)
{
    return data.WriteString(input);
}

static inline bool Unmarshalling(std::string &output, MessageParcel &data)
{
    return data.ReadString(output);
}

static inline bool Marshalling(const std::vector<uint8_t> &input, MessageParcel &data)
{
    return data.WriteUInt8Vector(input);
}

static inline bool Unmarshalling(std::vector<uint8_t> &output, MessageParcel &data)
{
    return data.ReadUInt8Vector(&output);
}

template<typename _OutTp>
bool ReadVariant(uint32_t step, uint32_t index, const _OutTp &output, MessageParcel &data);
template<typename _OutTp, typename _First, typename... _Rest>
bool ReadVariant(uint32_t step, uint32_t index, const _OutTp &output, MessageParcel &data);

template<typename _InTp>
bool WriteVariant(uint32_t step, const _InTp &input, MessageParcel &data);
template<typename _InTp, typename _First, typename... _Rest>
bool WriteVariant(uint32_t step, const _InTp &input, MessageParcel &data);

template<typename... _Types>
bool Marshalling(const std::variant<_Types...> &input, MessageParcel &data);
template<typename... _Types>
bool Unmarshalling(std::variant<_Types...> &output, MessageParcel &data);

template<class K, class V>
bool Marshalling(const std::map<K, V> &result, MessageParcel &parcel);
template<class K, class V>
bool Unmarshalling(std::map<K, V> &val, MessageParcel &parcel);

template<class T>
bool Marshalling(const std::vector<T> &val, MessageParcel &parcel);
template<class T>
bool Unmarshalling(std::vector<T> &val, MessageParcel &parcel);

template<typename T>
bool Marshalling(const T &input, MessageParcel &data);
template<typename T>
bool Unmarshalling(T &output, MessageParcel &data);

template<class T, typename std::enable_if<is_container<T>{}, int>::type = 0>
bool MarshalToContainer(const T &val, MessageParcel &parcel);
template<class T, typename std::enable_if<is_container<T>{}, int>::type = 0>
bool UnmarshalFromContainer(T &val, MessageParcel &parcel);

template<typename T, typename... Types>
bool Marshal(MessageParcel &parcel, const T &first, const Types &...others);

template<typename T, typename... Types>
bool Unmarshal(MessageParcel &parcel, T &first, Types &...others);
} // namespace DataObsUtils

template<typename _OutTp>
bool DataObsUtils::ReadVariant(uint32_t step, uint32_t index, const _OutTp &output, MessageParcel &data)
{
    return false;
}

template<typename _OutTp, typename _First, typename... _Rest>
bool DataObsUtils::ReadVariant(uint32_t step, uint32_t index, const _OutTp &output, MessageParcel &data)
{
    if (step == index) {
        _First value{};
        auto success = DataObsUtils::Unmarshalling(value, data);
        output = value;
        return success;
    }
    return DataObsUtils::ReadVariant<_OutTp, _Rest...>(step + 1, index, output, data);
}

template<typename _InTp>
bool DataObsUtils::WriteVariant(uint32_t step, const _InTp &input, MessageParcel &data)
{
    return false;
}

template<typename _InTp, typename _First, typename... _Rest>
bool DataObsUtils::WriteVariant(uint32_t step, const _InTp &input, MessageParcel &data)
{
    if (step == input.index()) {
        return DataObsUtils::Marshalling(std::get<_First>(input), data);
    }
    return DataObsUtils::WriteVariant<_InTp, _Rest...>(step + 1, input, data);
}

template<typename... _Types>
bool DataObsUtils::Marshalling(const std::variant<_Types...> &input, MessageParcel &data)
{
    uint32_t index = static_cast<uint32_t>(input.index());
    if (!data.WriteUint32(index)) {
        return false;
    }

    return DataObsUtils::WriteVariant<decltype(input), _Types...>(0, input, data);
}

template<typename... _Types>
bool DataObsUtils::Unmarshalling(std::variant<_Types...> &output, MessageParcel &data)
{
    uint32_t index = data.ReadUint32();
    if (index >= sizeof...(_Types)) {
        return false;
    }

    return DataObsUtils::ReadVariant<decltype(output), _Types...>(0, index, output, data);
}

template<class K, class V>
bool DataObsUtils::Marshalling(const std::map<K, V> &result, MessageParcel &parcel)
{
    if (!parcel.WriteInt32(static_cast<int32_t>(result.size()))) {
        return false;
    }
    for (const auto &entry : result) {
        if (!DataObsUtils::Marshalling(entry.first, parcel)) {
            return false;
        }
        if (!DataObsUtils::Marshalling(entry.second, parcel)) {
            return false;
        }
    }
    return true;
}

template<class K, class V>
bool DataObsUtils::Unmarshalling(std::map<K, V> &val, MessageParcel &parcel)
{
    int32_t size = 0;
    if (!parcel.ReadInt32(size)) {
        return false;
    }
    if (size < 0) {
        return false;
    }

    size_t readAbleSize = parcel.GetReadableBytes();
    if ((static_cast<size_t>(size) > readAbleSize) || static_cast<size_t>(size) > val.max_size()) {
        return false;
    }

    for (int32_t i = 0; i < size; i++) {
        K key;
        if (!DataObsUtils::Unmarshalling(key, parcel)) {
            return false;
        }
        if (!DataObsUtils::Unmarshalling(val[key], parcel)) {
            return false;
        }
    }
    return true;
}

template<class T>
bool DataObsUtils::Marshalling(const std::vector<T> &val, MessageParcel &parcel)
{
    return DataObsUtils::MarshalToContainer(val, parcel);
}

template<class T>
bool DataObsUtils::Unmarshalling(std::vector<T> &val, MessageParcel &parcel)
{
    return DataObsUtils::UnmarshalFromContainer(val, parcel);
}

template<class T, typename std::enable_if<is_container<T>{}, int>::type>
bool DataObsUtils::MarshalToContainer(const T &val, MessageParcel &parcel)
{
    if (val.size() > INT_MAX) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(val.size()))) {
        return false;
    }

    for (auto &v : val) {
        if (!DataObsUtils::Marshalling(v, parcel)) {
            return false;
        }
    }
    return true;
}

template<class T, typename std::enable_if<is_container<T>{}, int>::type>
bool DataObsUtils::UnmarshalFromContainer(T &val, MessageParcel &parcel)
{
    int32_t len = parcel.ReadInt32();
    if (len < 0) {
        return false;
    }

    size_t readAbleSize = parcel.GetReadableBytes();
    size_t size = static_cast<size_t>(len);
    if ((size > readAbleSize) || (size > val.max_size())) {
        return false;
    }

    val.clear();
    for (size_t i = 0; i < size; i++) {
        typename T::value_type value;
        if (!DataObsUtils::Unmarshalling(value, parcel)) {
            return false;
        }
        val.emplace_back(std::move(value));
    }
    return true;
}

template<typename T, typename... Types>
bool DataObsUtils::Marshal(MessageParcel &parcel, const T &first, const Types &...others)
{
    if (!DataObsUtils::Marshalling(first, parcel)) {
        return false;
    }
    return DataObsUtils::Marshal(parcel, others...);
}

template<typename T, typename... Types>
bool DataObsUtils::Unmarshal(MessageParcel &parcel, T &first, Types &...others)
{
    if (!DataObsUtils::Unmarshalling(first, parcel)) {
        return false;
    }
    return DataObsUtils::Unmarshal(parcel, others...);
}
} // namespace OHOS
#endif //  OHOS_ABILITY_RUNTIME_DATAOBS_UTILS_H