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

#ifndef ECMASCRIPT_BUILTINS_BUILTINS_FUNCTION_H
#define ECMASCRIPT_BUILTINS_BUILTINS_FUNCTION_H

#include "ecmascript/base/builtins_base.h"
#include "ecmascript/js_tagged_value-inl.h"

#define BUILTIN_FUNCTION_PROTOTYPE_FUNCTIONS(V)                                     \
    /* Function.prototype.apply ( thisArg, argArray ) */                            \
    V("apply",           FunctionPrototypeApply,      2, FunctionPrototypeApply)    \
    /* Function.prototype.bind ( thisArg , ...args) */                              \
    V("bind",            FunctionPrototypeBind,       1, INVALID)                   \
    /* Function.prototype.call (thisArg , ...args) */                               \
    V("call",            FunctionPrototypeCall,       1, INVALID)

namespace panda::ecmascript::builtins {
class BuiltinsFunction : public base::BuiltinsBase {
public:
    // ecma 19.2.1 Function (p1, p2, ... , pn, body)
    static JSTaggedValue FunctionConstructor(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3 The Function prototype object is itself a built-in function object.
    static JSTaggedValue FunctionPrototypeInvokeSelf(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3.1 Function.prototype.apply (thisArg, argArray)
    static JSTaggedValue FunctionPrototypeApply(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3.2 Function.prototype.bind (thisArg , ...args)
    static JSTaggedValue FunctionPrototypeBind(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3.3 Function.prototype.call (thisArg , ...args)
    static JSTaggedValue FunctionPrototypeCall(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3.5 Function.prototype.toString ()
    static JSTaggedValue FunctionPrototypeToString(EcmaRuntimeCallInfo *argv);

    // ecma 19.2.3.6 Function.prototype[@@hasInstance] (V)
    static JSTaggedValue FunctionPrototypeHasInstance(EcmaRuntimeCallInfo *argv);

    static Span<const base::BuiltinFunctionEntry> GetFunctionPrototypeFunctions()
    {
        return Span<const base::BuiltinFunctionEntry>(FUNCTION_PROTOTYPE_FUNCTIONS);
    }

private:
#define BUILTIN_FUNCTION_FUNCTION_ENTRY(name, func, length, id) \
    base::BuiltinFunctionEntry::Create(name, BuiltinsFunction::func, length, kungfu::BuiltinsStubCSigns::id),

    static constexpr std::array FUNCTION_PROTOTYPE_FUNCTIONS = {
        BUILTIN_FUNCTION_PROTOTYPE_FUNCTIONS(BUILTIN_FUNCTION_FUNCTION_ENTRY)
    };
#undef BUILTIN_FUNCTION_FUNCTION_ENTRY
};
}  // namespace panda::ecmascript::builtins
#endif  // ECMASCRIPT_BUILTINS_BUILTINS_FUNCTION_H
