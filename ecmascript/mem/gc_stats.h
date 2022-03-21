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

#ifndef ECMASCRIPT_MEM_GC_STATS_H
#define ECMASCRIPT_MEM_GC_STATS_H

#include "time.h"
#include "chrono"
#include "libpandabase/utils/logger.h"

namespace panda::ecmascript {
class Heap;
class GCStats {
    using Duration = std::chrono::duration<uint64_t, std::nano>;

public:
    explicit GCStats(const Heap *heap) : heap_(heap) {}
    ~GCStats() = default;

    void PrintStatisticResult(bool isForce = false);
    void PrintHeapStatisticResult(bool isForce = true);

    void StatisticSTWYoungGC(Duration time, size_t aliveSize, size_t promoteSize, size_t commitSize);
    void StatisticMixGC(bool concurrentMark, Duration time, size_t freeSize);
    void StatisticFullGC(Duration time, size_t youngAndOldAliveSize, size_t youngCommitSize,
                         size_t oldCommitSize, size_t nonMoveSpaceFreeSize, size_t nonMoveSpaceCommitSize);
    void StatisticConcurrentMark(Duration time);
    void StatisticConcurrentMarkWait(Duration time);
    void StatisticConcurrentRemark(Duration time);
    void StatisticConcurrentEvacuate(Duration time);

private:
    void PrintSemiStatisticResult(bool isForce);
    void PrintMixStatisticResult(bool isForce);
    void PrintCompressStatisticResult(bool isForce);

    size_t TimeToMicroseconds(Duration time)
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
    }

    float PrintTimeMilliseconds(uint64_t time)
    {
        return (float)time / MILLION_TIME;
    }

    float sizeToMB(size_t size)
    {
        return (float)size / MB;
    }

    size_t lastSemiGCCount_ = 0;
    size_t semiGCCount_ = 0;
    size_t semiGCMinPause_ = 0;
    size_t semiGCMAXPause_ = 0;
    size_t semiGCTotalPause_ = 0;
    size_t semiTotalAliveSize_ = 0;
    size_t semiTotalCommitSize_ = 0;
    size_t semiTotalPromoteSize_ = 0;

    size_t lastOldGCCount_ = 0;
    size_t mixGCCount_ = 0;
    size_t mixGCMinPause_ = 0;
    size_t mixGCMAXPause_ = 0;
    size_t mixGCTotalPause_ = 0;
    size_t mixOldSpaceFreeSize_ = 0;

    size_t lastOldConcurrentMarkGCCount_ = 0;
    size_t mixConcurrentMarkGCPauseTime_ = 0;
    size_t mixConcurrentMarkMarkPause_ = 0;
    size_t mixConcurrentMarkWaitPause_ = 0;
    size_t mixConcurrentMarkRemarkPause_ = 0;
    size_t mixConcurrentMarkEvacuatePause_ = 0;
    size_t mixConcurrentMarkGCCount_ = 0;
    size_t mixConcurrentMarkGCMinPause_ = 0;
    size_t mixConcurrentMarkGCMAXPause_ = 0;
    size_t mixConcurrentMarkGCTotalPause_ = 0;
    size_t mixOldSpaceConcurrentMarkFreeSize_ = 0;

    size_t lastFullGCCount_ = 0;
    size_t fullGCCount_ = 0;
    size_t fullGCMinPause_ = 0;
    size_t fullGCMaxPause_ = 0;
    size_t fullGCTotalPause_ = 0;
    size_t compressYoungAndOldAliveSize_ = 0;
    size_t compressYoungCommitSize_ = 0;
    size_t compressOldCommitSize_ = 0;
    size_t compressNonMoveTotalFreeSize_ = 0;
    size_t compressNonMoveTotalCommitSize_ = 0;

    const Heap *heap_;

    static constexpr uint32_t MILLION_TIME = 1000;
    static constexpr uint32_t MB = 1 * 1024 * 1024;

    NO_COPY_SEMANTIC(GCStats);
    NO_MOVE_SEMANTIC(GCStats);
};
}  // namespace panda::ecmascript

#endif  // ECMASCRIPT_MEM_GC_STATS_H
