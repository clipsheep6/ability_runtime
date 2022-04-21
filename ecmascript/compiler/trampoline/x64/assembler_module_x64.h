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

#ifndef ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
#define ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
#include "ecmascript/compiler/assembler/assembler_x64.h"

namespace panda::ecmascript::x64 {
class AssemblerModuleX64 {
public:
    static void Generate_AsmInterpCallRuntime(AssemblerX64 *assembler);
    static void Generate_OptimizedCallRuntime(AssemblerX64 *assembler);
};
}  // namespace panda::ecmascript::x64
#endif  // ECMASCRIPT_COMPILER_ASSEMBLER_MODULE_X64_H
