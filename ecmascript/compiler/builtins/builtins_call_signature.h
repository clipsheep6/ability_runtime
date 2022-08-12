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

#ifndef ECMASCRIPT_COMPILER_BUILTINS_CALL_SIGNATURE_H
#define ECMASCRIPT_COMPILER_BUILTINS_CALL_SIGNATURE_H

#include "ecmascript/base/config.h"
#include "ecmascript/compiler/rt_call_signature.h"

namespace panda::ecmascript::kungfu {
#define IGNORE_BC_STUB(...)
// V: Not Enabled, T: Enabled, D: Always Disable SingleStepDebugging
#define BUILTINS_STUB_LIST(V)                       \
    V(CharCodeAt)                                   \
    V(IndexOf)                                      \

class BuiltinsStubCSigns {
public:
    enum ID {
#define DEF_STUB_ID(name) name,
        BUILTINS_STUB_LIST(DEF_STUB_ID)
#undef DEF_STUB_ID
        NUM_OF_BUILTINS_STUBS
    };

    static void Initialize();

    static void GetCSigns(std::vector<const CallSignature*>& callSigns);

    static const CallSignature *Get(size_t index)
    {
        ASSERT(index < NUM_OF_BUILTINS_STUBS);
        return &callSigns_[index];
    }

    static const CallSignature* BuiltinsHandler()
    {
        return &builtinsCSign_;
    }
private:
    static CallSignature callSigns_[NUM_OF_BUILTINS_STUBS];
    static CallSignature builtinsCSign_;
};

enum class BuiltinsArgs : size_t {
    GLUE = 0,
    NATIVECODE,
    FUNC,
    THISVALUE,
    NUMARGS,
    ARGV,
    NUM_OF_INPUTS,
};

#define BUILTINS_STUB_ID(name) kungfu::BuiltinsStubCSigns::name
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BUILTINS_CALL_SIGNATURE_H
