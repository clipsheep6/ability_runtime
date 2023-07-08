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

#ifndef ECMASCRIPT_PGO_PROFILER_ENCODER_H
#define ECMASCRIPT_PGO_PROFILER_ENCODER_H

#include "ecmascript/pgo_profiler/pgo_profiler_info.h"
#include "macros.h"

namespace panda::ecmascript {
class PGOProfilerEncoder {
public:
    PGOProfilerEncoder(const std::string &outDir, uint32_t hotnessThreshold)
        : outDir_(outDir), hotnessThreshold_(hotnessThreshold) {}

    NO_COPY_SEMANTIC(PGOProfilerEncoder);
    NO_MOVE_SEMANTIC(PGOProfilerEncoder);

    static void AddChecksum(std::fstream& fileStream);

    bool PUBLIC_API InitializeData();

    void PUBLIC_API Destroy();

    bool IsInitialized() const
    {
        return isInitialized_;
    }

    void SamplePandaFileInfo(uint32_t checksum);
    void Merge(const PGORecordDetailInfos &recordInfos);
    void TerminateSaveTask();
    void PostSaveTask();

    bool PUBLIC_API Save();

    bool PUBLIC_API LoadAPTextFile(const std::string &inPath);

private:
    void StartSaveTask(const SaveTask *task);
    bool SaveProfiler(const SaveTask *task = nullptr);

    bool isInitialized_ {false};
    std::string outDir_;
    uint32_t hotnessThreshold_ {2};
    std::string realOutPath_;
    PGOProfilerHeader *header_ {nullptr};
    std::unique_ptr<PGOPandaFileInfos> pandaFileInfos_;
    std::unique_ptr<PGORecordDetailInfos> globalRecordInfos_;
    os::memory::Mutex mutex_;
    friend SaveTask;
};

class SaveTask : public Task {
public:
    explicit SaveTask(PGOProfilerEncoder *encoder, int32_t id) : Task(id), encoder_(encoder) {};
    virtual ~SaveTask() override = default;

    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        encoder_->StartSaveTask(this);
        return true;
    }

    TaskType GetTaskType() const override
    {
        return TaskType::PGO_SAVE_TASK;
    }

    NO_COPY_SEMANTIC(SaveTask);
    NO_MOVE_SEMANTIC(SaveTask);
private:
    PGOProfilerEncoder *encoder_;
};
} // namespace panda::ecmascript
#endif  // ECMASCRIPT_PGO_PROFILER_ENCODER_H
