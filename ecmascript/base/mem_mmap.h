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

#ifndef ECMASCRIPT_BASE_MEM_MMAP_H
#define ECMASCRIPT_BASE_MEM_MMAP_H

#include <cstdlib>
#include "ecmascript/common.h"

#ifdef PANDA_TARGET_WINDOWS
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0
#endif

namespace panda::ecmascript::base {
class MemMmap {
public:
    MemMmap() = default;
    ~MemMmap() = default;

    static void* PUBLIC_API Mmap(size_t allocSize, bool executable = true);
    static int PUBLIC_API Munmap(void *addr, size_t allocSize);
};
}  // namespace panda::ecmascript::base

#endif  // ECMASCRIPT_BASE_MEM_MMAP_H