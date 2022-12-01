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

#ifndef ECMASCRIPT_MEM_LINEAR_SPACE_H
#define ECMASCRIPT_MEM_LINEAR_SPACE_H

#include "ecmascript/mem/space-inl.h"

namespace panda::ecmascript {
class LinearSpace : public Space {
public:
    explicit LinearSpace(Heap *heap, MemSpaceType type, size_t initialCapacity, size_t maximumCapacity);
    NO_COPY_SEMANTIC(LinearSpace);
    NO_MOVE_SEMANTIC(LinearSpace);
    uintptr_t Allocate(size_t size, bool isPromoted = false);
    bool Expand(bool isPromoted);
    void Stop();
    void ResetAllocator();
    void IterateOverObjects(const std::function<void(TaggedObject *object)> &objectVisitor) const;
    void DecreaseSurvivalObjectSize(size_t objSize)
    {
        survivalObjectSize_ -= objSize;
    }
    const uintptr_t *GetAllocationTopAddress()
    {
        return allocator_.GetTopAddress();
    }
    const uintptr_t *GetAllocationEndAddress()
    {
        return allocator_.GetEndAddress();
    }
    void ResetNativeBindingSize()
    {
        newSpaceNativeBindingSize_ = 0;
    }
    void IncreaseNativeBindingSize(size_t size)
    {
        newSpaceNativeBindingSize_ += size;
    }
    size_t GetNativeBindingSize()
    {
        return newSpaceNativeBindingSize_;
    }

    bool NativeBindingSizeLargerThanLimit()
    {
        return newSpaceNativeBindingSize_ > newSpaceNativeLimit_;
    }
protected:
    Heap *heap_ {nullptr};

    BumpPointerAllocator allocator_;
    size_t overShootSize_ {0};
    size_t allocateAfterLastGC_ {0};
    size_t survivalObjectSize_ {0};
    uintptr_t waterLine_ {0};
    size_t newSpaceNativeLimit_;
    size_t newSpaceNativeBindingSize_ {0};
};

class SemiSpace : public LinearSpace {
public:
    explicit SemiSpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity);
    ~SemiSpace() override = default;
    NO_COPY_SEMANTIC(SemiSpace);
    NO_MOVE_SEMANTIC(SemiSpace);

    void Initialize() override;
    void Restart();

    uintptr_t AllocateSync(size_t size);

    void SetOverShootSize(size_t size);
    bool AdjustCapacity(size_t allocatedSizeSinceGC);
    void AdjustNativeLimit(size_t previousNativeSize);
    void SetWaterLine();

    uintptr_t GetWaterLine() const
    {
        return waterLine_;
    }
    uintptr_t GetTop() const
    {
        return allocator_.GetTop();
    }
    size_t GetHeapObjectSize() const;
    size_t GetSurvivalObjectSize() const;
    size_t GetAllocatedSizeSinceGC(uintptr_t top = 0) const;

    bool SwapRegion(Region *region, SemiSpace *fromSpace);

private:
    static constexpr int GROWING_FACTOR = 2;
    os::memory::Mutex lock_;
    size_t minimumCapacity_;
};

class SnapshotSpace : public LinearSpace {
public:
    explicit SnapshotSpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity);
    ~SnapshotSpace() override = default;
    NO_COPY_SEMANTIC(SnapshotSpace);
    NO_MOVE_SEMANTIC(SnapshotSpace);

    size_t GetHeapObjectSize() const
    {
        return liveObjectSize_;
    }

    void IncreaseLiveObjectSize(size_t size)
    {
        liveObjectSize_ += size;
    }

private:
    size_t liveObjectSize_ {0};
};

class ReadOnlySpace : public LinearSpace {
public:
    explicit ReadOnlySpace(Heap *heap, size_t initialCapacity, size_t maximumCapacity);
    ~ReadOnlySpace() override = default;
    void SetReadOnly()
    {
        auto cb = [](Region *region) {
            region->SetReadOnlyAndMarked();
        };
        EnumerateRegions(cb);
    }

    void ClearReadOnly()
    {
        auto cb = [](Region *region) {
            region->ClearReadOnly();
        };
        EnumerateRegions(cb);
    }

    NO_COPY_SEMANTIC(ReadOnlySpace);
    NO_MOVE_SEMANTIC(ReadOnlySpace);
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_MEM_LINEAR_SPACE_H
