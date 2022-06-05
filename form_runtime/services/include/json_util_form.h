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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_JSON_UTIL_FORM_H
#define FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_JSON_UTIL_FORM_H

#include <string>

#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "json_serializer.h"

namespace OHOS {
namespace AppExecFwk {
enum class JsonType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING,
};

enum class ArrayType {
    NUMBER,
    OBJECT,
    STRING,
    NOT_ARRAY,
};

template<typename T, typename dataType>
void CheckArrayType(
    const nlohmann::json &jsonObject, const std::string &key, dataType &data, ArrayType arrayType, int32_t &parseResult)
{
    if (!jsonObject.contains(key)) {
        HILOG_ERROR("jsonObject doesn't contain key:%{public}s", key.c_str());
        return;
    }
    auto arrays = jsonObject.at(key);
    if (arrays.empty()) {
        HILOG_DEBUG("array is empty");
        return;
    }
    switch (arrayType) {
        case ArrayType::STRING:
            for (const auto &array : arrays) {
                if (!array.is_string()) {
                    HILOG_ERROR("array %{public}s is not string type", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                }
            }
            if (parseResult == ERR_OK) {
                data = jsonObject.at(key).get<T>();
            }
            break;
        case ArrayType::OBJECT:
            for (const auto &array : arrays) {
                if (!array.is_object()) {
                    HILOG_ERROR("array %{public}s is not object type", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
            }
            if (parseResult == ERR_OK) {
                data = jsonObject.at(key).get<T>();
            }
            break;
        case ArrayType::NUMBER:
            for (const auto &array : arrays) {
                if (!array.is_number()) {
                    HILOG_ERROR("array %{public}s is not number type", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                }
            }
            if (parseResult == ERR_OK) {
                data = jsonObject.at(key).get<T>();
            }
            break;
        case ArrayType::NOT_ARRAY:
            HILOG_ERROR("array %{public}s is not string type", key.c_str());
            break;
        default:
            HILOG_ERROR("array %{public}s type error", key.c_str());
            break;
    }
}

template<typename T, typename dataType>
void GetValueIfFindKey(const nlohmann::json &jsonObject, const nlohmann::detail::iter_impl<const nlohmann::json> &end,
    const std::string &key, dataType &data, JsonType jsonType, bool isNecessary, int32_t &parseResult,
    ArrayType arrayType)
{
    if (parseResult) {
        return;
    }
    if (jsonObject.find(key) != end) {
        switch (jsonType) {
            case JsonType::BOOLEAN:
                if (!jsonObject.at(key).is_boolean()) {
                    HILOG_ERROR("type is error %{public}s is not boolean", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::NUMBER:
                if (!jsonObject.at(key).is_number()) {
                    HILOG_ERROR("type is error %{public}s is not number", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::OBJECT:
                if (!jsonObject.at(key).is_object()) {
                    HILOG_ERROR("type is error %{public}s is not object", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::ARRAY:
                if (!jsonObject.at(key).is_array()) {
                    HILOG_ERROR("type is error %{public}s is not array", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
                CheckArrayType<T>(jsonObject, key, data, arrayType, parseResult);
                break;
            case JsonType::STRING:
                if (!jsonObject.at(key).is_string()) {
                    HILOG_ERROR("type is error %{public}s is not string", key.c_str());
                    parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::NULLABLE:
                HILOG_ERROR("type is error %{public}s is nullable", key.c_str());
                break;
            default:
                HILOG_ERROR("type is error %{public}s is not jsonType", key.c_str());
                parseResult = ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
        }
        return;
    }
    if (isNecessary) {
        HILOG_ERROR("profile prop %{public}s is mission", key.c_str());
        parseResult = ERR_APPEXECFWK_PARSE_PROFILE_MISSING_PROP;
    }
}

template<typename T>
const std::string GetJsonStrFromInfo(T &t)
{
    nlohmann::json json = t;
    return json.dump();
}

template<typename T>
bool ParseInfoFromJsonStr(const char *data, T &t)
{
    if (data == nullptr) {
        HILOG_ERROR("%{public}s faile due to data is nullptr", __func__);
        return false;
    }

    nlohmann::json jsonObject = nlohmann::json::parse(data, nullptr, false);
    if (jsonObject.is_discarded()) {
        HILOG_ERROR("%{public}s faile due to data is discarded", __func__);
        return false;
    }

    t = jsonObject.get<T>();
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_SERVICES_FORMMGR_INCLUDE_JSON_UTIL_FORM_H