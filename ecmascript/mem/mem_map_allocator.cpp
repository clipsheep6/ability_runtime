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

#include "ecmascript/mem/mem_map_allocator.h"

namespace panda::ecmascript {
MemMap MemMapAllocator::Allocate(size_t size, size_t alignment, bool isRegular)
{
    if (UNLIKELY(memMapTotalSize_ + size > capacity_)) {
        LOG(ERROR, RUNTIME) << "mem map overflow";
        return MemMap();
    }
    MemMap mem;
    if (isRegular) {
        mem = memMapPool_.GetMemFromCache(size);
        if (mem.GetMem() != nullptr) {
            memMapTotalSize_ += size;
            PageTag(mem.GetMem(), size);
            return mem;
        }
        mem = PageMap(REGULAR_REGION_MMAP_SIZE, alignment);
        mem = memMapPool_.SplitMemToCache(mem);
    } else {
        mem = memMapFreeList_.GetMemFromList(size);
    }
    if (mem.GetMem() != nullptr) {
        PageTag(mem.GetMem(), mem.GetSize());
        memMapTotalSize_ += mem.GetSize();
    }
    return mem;
}

void MemMapAllocator::Free(void *mem, size_t size, bool isRegular)
{
    memMapTotalSize_ -= size;
    PageRelease(mem, size);
    if (isRegular) {
        memMapPool_.AddMemToCache(mem, size);
    } else {
        memMapFreeList_.AddMemToList(MemMap(mem, size));
    }
}

MemMap MemMapAllocator::PageMap(size_t size, size_t alignment)
{
    [[maybe_unused]]size_t allocSize = size + alignment;
    void *result = mmap(0, allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    LOG_IF(result == nullptr, FATAL, ECMASCRIPT);
    auto alignResult = AlignUp(reinterpret_cast<uintptr_t>(result), alignment);
    size_t leftSize = alignResult - reinterpret_cast<uintptr_t>(result);
    size_t rightSize = alignment - leftSize;
    void *alignEndResult = reinterpret_cast<void *>(alignResult + size);
    munmap(result, leftSize);
    munmap(alignEndResult, rightSize);
    return MemMap(reinterpret_cast<void *>(alignResult), size);
}
}  // namespace panda::ecmascript
