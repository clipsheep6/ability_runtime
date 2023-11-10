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

#ifndef ECMASCRIPT_BASE_ERROR_TYPE_H
#define ECMASCRIPT_BASE_ERROR_TYPE_H

#include <cstdint>

namespace panda::ecmascript::base {
enum class ErrorType : uint8_t {
    ERROR = 0,
    EVAL_ERROR,
    RANGE_ERROR,
    REFERENCE_ERROR,
    SYNTAX_ERROR,
    TYPE_ERROR,
    URI_ERROR,
    AGGREGATE_ERROR,
    OOM_ERROR,
    TERMINATION_ERROR,
};
}  // namespace panda::ecmascript::base

#endif  // ECMASCRIPT_BASE_ERROR_TYPE_H
