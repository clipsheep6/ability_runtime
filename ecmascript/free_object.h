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

#ifndef ECMASCRIPT_FREE_OBJECT_H
#define ECMASCRIPT_FREE_OBJECT_H

#include "ecmascript/js_hclass.h"
#include "ecmascript/mem/barriers.h"
#include "ecmascript/mem/tagged_object-inl.h"

#define INVALID_OBJECT ((FreeObject *)0x06)

namespace panda::ecmascript {
class FreeObject : public TaggedObject {
public:
    static FreeObject *Cast(uintptr_t object)
    {
        return reinterpret_cast<FreeObject *>(object);
    }
    static FreeObject *FillFreeObject(EcmaVM *vm, uintptr_t address, size_t size);

    inline bool IsEmpty() const
    {
        return Available() == 0;
    }

    inline uintptr_t GetBegin() const
    {
        return reinterpret_cast<uintptr_t>(this);
    }

    inline uintptr_t GetEnd() const
    {
        return reinterpret_cast<uintptr_t>(this) + Available();
    }

    inline void SetAvailable(uint32_t size)
    {
        if (size >= SIZE) {
            SetSize(JSTaggedValue(size));
        }
    }

    inline uint32_t Available() const
    {
        auto hclass = GetClass();
        if (hclass->IsFreeObjectWithShortField()) {
            return hclass->GetObjectSize();
        }
        ASSERT(GetSize().IsInt());
        return GetSize().GetInt();
    }

    inline bool IsFreeObject() const
    {
        return GetClass()->IsFreeObject();
    }

    static constexpr size_t NEXT_OFFSET = TaggedObjectSize();
    ACCESSORS_FIXED_SIZE_FIELD(Next, FreeObject *, JSTaggedType, NEXT_OFFSET, SIZE_OFFSET)
    // TaggedArray visitor may be error while concurrent marking
    ACCESSORS(Size, SIZE_OFFSET, SIZE)
};

static_assert((FreeObject::SIZE % static_cast<uint8_t>(MemAlignment::MEM_ALIGN_OBJECT)) == 0);
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_MEM_FREE_OBJECT_H
