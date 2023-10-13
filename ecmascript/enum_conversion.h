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

#ifndef ECMASCRIPT_ENUM_CONVERSION_H
#define ECMASCRIPT_ENUM_CONVERSION_H

#include <optional>
#include "ecmascript/global_env_fields.h"
#include "ecmascript/ts_types/builtin_type_id.h"

namespace panda::ecmascript {
inline constexpr std::optional<GlobalEnvField> ToGlobelEnvPrototypeField(BuiltinTypeId type)
{
    // case BuiltinTypeId::INT8_ARRAY ...
    if (IsTypedArrayType(type)) {
        return GlobalEnvField::TYPED_ARRAY_PROTOTYPE_INDEX;
    }
    switch (type) {
        case BuiltinTypeId::ARRAY:
            return GlobalEnvField::ARRAY_PROTOTYPE_INDEX;
        case BuiltinTypeId::DATA_VIEW:
            return GlobalEnvField::DATA_VIEW_PROTOTYPE_INDEX;
        case BuiltinTypeId::DATE:
            return GlobalEnvField::DATE_PROTOTYPE_INDEX;
        case BuiltinTypeId::FUNCTION:
            return GlobalEnvField::FUNCTION_PROTOTYPE_INDEX;
        case BuiltinTypeId::GENERATOR_FUNCTION:
            return GlobalEnvField::GENERATOR_FUNCTION_PROTOTYPE_OFFSET;
        case BuiltinTypeId::MAP:
            return GlobalEnvField::MAP_PROTOTYPE_INDEX;
        case BuiltinTypeId::OBJECT:
            return GlobalEnvField::OBJECT_FUNCTION_PROTOTYPE_INDEX;
        case BuiltinTypeId::SET:
            return GlobalEnvField::SET_PROTOTYPE_INDEX;
        case BuiltinTypeId::STRING:
            return GlobalEnvField::STRING_PROTOTYPE_INDEX;
        default: // No corresponding entry in either BuiltinTypeId or GlobalEnvField
            return std::nullopt;
    }
}
} // namespace panda::ecmascript
#endif // ECMASCRIPT_ENUM_CONVERSION_H
