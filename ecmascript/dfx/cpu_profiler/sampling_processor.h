/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_DFX_CPU_PROFILER_SAMPLING_PROCESSOR_H
#define ECMASCRIPT_DFX_CPU_PROFILER_SAMPLING_PROCESSOR_H

#include <csignal>
#include <cstdint>
#include <pthread.h>

#include "libpandabase/macros.h"

namespace panda::ecmascript {
class SamplesRecord;
class JSThread;
class EcmaVM;
class SamplingProcessor {
public:
    static uint64_t GetMicrosecondsTimeStamp();

    virtual ~SamplingProcessor();

    static void *Run(void *arg);

    NO_COPY_SEMANTIC(SamplingProcessor);
    NO_MOVE_SEMANTIC(SamplingProcessor);
};

struct RunParams {
    SamplesRecord *generatorParam;
    uint32_t intervalParam;
    pthread_t pidParam;

    RunParams(SamplesRecord *generatorParam, uint32_t intervalParam, pthread_t pidParam)
        :generatorParam(generatorParam), intervalParam(intervalParam), pidParam(pidParam) {};
};
} // namespace panda::ecmascript
#endif // ECMASCRIPT_DFX_CPU_PROFILER_SAMPLING_PROCESSOR_H
