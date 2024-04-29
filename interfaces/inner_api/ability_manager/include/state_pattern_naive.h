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

#ifndef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_H

/**
 * Highly not recommanded to inlcude this header in a header file.
 * When using naive state design pattern, please define `STATE_PATTERN_NAIVE_STATE`
 *   in the **cpp** file. And then include this header in the **cpp** file.
 * If an error log is needed, please define `STATE_PATTERN_NAIVE_LOGGER`.
**/
#ifdef STATE_PATTERN_NAIVE_STATE

#define STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(state, returnVal)     \
do {                                                                   \
    STATE_PATTERN_NAIVE_STATE = state;                                 \
    return returnVal;                                                  \
} while (0)                                                            \

#define STATE_PATTERN_NAIVE_ACCEPT(state, returnVal)     \
do {                                                     \
    if (STATE_PATTERN_NAIVE_STATE != state) {            \
        return returnVal;                                \
    }                                                    \
} while (0)                                              \

#define STATE_PATTERN_NAIVE_REJECT(state, returnVal)     \
do {                                                     \
    if (STATE_PATTERN_NAIVE_STATE == state) {            \
        return returnVal;                                \
    }                                                    \
} while (0)                                              \

#ifdef STATE_PATTERN_NAIVE_LOGGER
#define STATE_PATTERN_NAIVE_ACCEPT_LOG(state, returnVal, ...)        \
do {                                                                 \
    if (STATE_PATTERN_NAIVE_STATE != state) {                        \
        STATE_PATTERN_NAIVE_LOGGER(__VA_ARGS__);                     \
        return returnVal;                                            \
    }                                                                \
} while (0)                                                          \

#define STATE_PATTERN_NAIVE_REJECT_LOG(state, returnVal, ...)        \
do {                                                                 \
    if (STATE_PATTERN_NAIVE_STATE == state) {                        \
        STATE_PATTERN_NAIVE_LOGGER(__VA_ARGS__);                     \
        return returnVal;                                            \
    }                                                                \
} while (0)                                                          \

#endif // STATE_PATTERN_NAIVE_LOGGER
#endif // STATE_PATTERN_NAIVE_STATE
#endif // STATE_PATTERN_NAIVE_H
