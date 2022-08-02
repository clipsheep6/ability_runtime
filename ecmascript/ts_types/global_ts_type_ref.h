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

#ifndef ECMASCRIPT_TS_TYPES_GLOBAL_TS_TYPE_REF_H
#define ECMASCRIPT_TS_TYPES_GLOBAL_TS_TYPE_REF_H

#include "ecmascript/ecma_macros.h"
#include "libpandabase/utils/bit_field.h"

namespace panda::ecmascript {
enum class TSTypeKind : int {
    PRIMITIVE = 0,
    CLASS,
    CLASS_INSTANCE,
    FUNCTION,
    UNION,
    ARRAY,
    OBJECT,
    IMPORT,
    INTERFACE_KIND
};

enum class TSPrimitiveType : int {
    ANY = 0,
    NUMBER,
    BOOLEAN,
    VOID_TYPE,
    STRING,
    SYMBOL,
    NULL_TYPE,
    UNDEFINED,
    INT,
    BIG_INT,
    END
};

class GlobalTSTypeRef {
public:
    explicit GlobalTSTypeRef(uint32_t type = 0) : type_(type) {}
    explicit GlobalTSTypeRef(int moduleId, int localId) : type_(0)
    {
        SetLocalId(static_cast<uint16_t>(localId));
        SetModuleId(static_cast<uint16_t>(moduleId));
    }

    ~GlobalTSTypeRef() = default;

    static constexpr int LOCAL_ID_BITS = 16;
    static constexpr int MODULE_ID_BITS = 13;
    static constexpr int GC_TYPE_BITS = 2;
    static constexpr int FLAG_BITS = 1;
    FIRST_BIT_FIELD(Type, LocalId, uint16_t, LOCAL_ID_BITS);            // 0 ~ 15
    NEXT_BIT_FIELD(Type, ModuleId, uint16_t, MODULE_ID_BITS, LocalId);  // 16 ~ 28
    NEXT_BIT_FIELD(Type, GCType, uint8_t, GC_TYPE_BITS, ModuleId);      // 29 ~ 30
    NEXT_BIT_FIELD(Type, Flag, bool, FLAG_BITS, GCType);                // 31: 0: TS type, 1: MIR type

    static GlobalTSTypeRef Default()
    {
        return GlobalTSTypeRef(0u);
    }

    uint32_t GetType() const
    {
        return type_;
    }

    void SetType(uint32_t type)
    {
        type_ = type;
    }

    void Clear()
    {
        type_ = 0;
    }

    bool IsDefault() const
    {
        return type_ == 0;
    }

    bool operator <(const GlobalTSTypeRef &other) const
    {
        return type_ < other.type_;
    }

    bool operator ==(const GlobalTSTypeRef &other) const
    {
        return type_ == other.type_;
    }

    bool operator !=(const GlobalTSTypeRef &other) const
    {
        return type_ != other.type_;
    }

    void Dump() const
    {
        uint32_t gcType = GetGCType();
        uint32_t moduleId = GetModuleId();
        uint32_t localId = GetLocalId();
        LOG_ECMA(ERROR) << " gcType: " << gcType
                        << " moduleId: " << moduleId
                        << " localId: " << localId;
    }

private:
     uint32_t type_ {0};
};
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_TS_TYPES_GLOBAL_TS_TYPE_REF_H
