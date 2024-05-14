/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_CJ_ENVIRONMENT_CJ_INTERFACE_H
#define OHOS_ABILITY_CJ_ENVIRONMENT_CJ_INTERFACE_H

#include <stdint.h>

using CJThreadHandle = void*;

namespace OHOS {

enum RTLogLevel {
    RTLOG_VERBOSE,
    RTLOG_DEBUGY,
    RTLOG_INFO,
    RTLOG_WARNING,
    RTLOG_ERROR,
    RTLOG_FATAL_WITHOUT_ABORT,
    RTLOG_FATAL,
    RTLOG_OFF
};

enum RTErrorCode { E_OK = 0, E_ARGS = -1, E_TIMEOUT = -2, E_STATE = -3, E_FAILED = -4 };

struct HeapParam {
    size_t regionSize;
    size_t heapSize;
    double exemptionThreshold;
    double heapUtilization;
    double heapGrowth;
    double allocationRate;
    size_t allocationWaitTime;
};

struct GCParam {
    size_t gcThreshold;
    double garbageThreshold;
    uint64_t gcInterval;
    uint64_t backupGCInterval;
    int32_t gcThreads;
};

struct LogParam {
    enum RTLogLevel logLevel;
};

struct ConcurrencyParam {
    size_t thStackSize;
    size_t coStackSize;
    uint32_t processorNum;
};

struct RuntimeParam {
    struct HeapParam heapParam;
    struct GCParam gcParam;
    struct LogParam logParam;
    struct ConcurrencyParam coParam;
};

#if defined(__OHOS__) && (__OHOS__ == 1)
using LogHandle = void (*)(const char*);
#endif
}
#endif // OHOS_ABILITY_CJ_ENVIRONMENT_CJ_INTERFACE_H
