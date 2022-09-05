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

#ifndef ECMASCRIPT_INTERPRETER_INTERPRETER_INL_H
#define ECMASCRIPT_INTERPRETER_INTERPRETER_INL_H

#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/ic/ic_runtime_stub-inl.h"
#include "ecmascript/interpreter/fast_runtime_stub-inl.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/interpreter/interpreter_assembly.h"
#include "ecmascript/interpreter/frame_handler.h"
#include "ecmascript/interpreter/slow_runtime_stub.h"
#include "ecmascript/jspandafile/literal_data_extractor.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_async_generator_object.h"
#include "ecmascript/js_generator_object.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/mem/concurrent_marker.h"
#include "ecmascript/module/js_module_manager.h"
#include "ecmascript/runtime_call_id.h"
#include "ecmascript/template_string.h"
#include "ecmascript/tooling/interface/js_debugger_manager.h"

#include "libpandafile/code_data_accessor.h"
#include "libpandafile/file.h"
#include "libpandafile/method_data_accessor-inl.h"

#if defined(ECMASCRIPT_SUPPORT_CPUPROFILER)
#include "ecmascript/dfx/cpu_profiler/cpu_profiler.h"
#endif

namespace panda::ecmascript {
using CommonStubCSigns = kungfu::CommonStubCSigns;
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvoid-ptr-dereference"
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#if ECMASCRIPT_ENABLE_INTERPRETER_LOG
#define HANDLE_OPCODE(opcode)           \
    HANDLE_##opcode:                    \
    {                                   \
        BytecodeInstruction inst(pc);   \
        LOG_INTERPRETER(DEBUG) << inst; \
    }
#else
#define HANDLE_OPCODE(opcode)       \
    HANDLE_##opcode:
#endif

#define FIRST_HANDLE_OPCODE(opcode) \
    HANDLE_##opcode:

#define LOG_INST() false && LOG_INTERPRETER(DEBUG)

#define DEBUG_HANDLE_OPCODE(opcode) \
    DEBUG_HANDLE_##opcode:

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADVANCE_PC(offset) \
    pc += (offset);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-macro-usage)

#define GOTO_NEXT()  // NOLINT(clang-diagnostic-gnu-label-as-value, cppcoreguidelines-macro-usage)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISPATCH(curOpcode)                                       \
    do {                                                          \
        ADVANCE_PC(BytecodeInstruction::Size(EcmaOpcode::curOpcode))  \
        opcode = READ_INST_OP(); goto *dispatchTable[opcode];     \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISPATCH_OFFSET(offset)                               \
    do {                                                      \
        ADVANCE_PC(offset)                                    \
        opcode = READ_INST_OP(); goto *dispatchTable[opcode]; \
    } while (false)

#define DISPATCH_THROW()                  \
    do {                                  \
        opcode = *(pc + 1);               \
        goto *throwDispatchTable[opcode]; \
    } while (false)

#define DISPATCH_WIDE()                  \
    do {                                 \
        opcode = *(pc + 1);              \
        goto *wideDispatchTable[opcode]; \
    } while (false)

#define DISPATCH_DEPRECATED()                  \
    do {                                       \
        opcode = *(pc + 1);                    \
        goto *deprecatedDispatchTable[opcode]; \
    } while (false)

#define DEBUG_DISPATCH_THROW()                  \
    do {                                        \
        opcode = *(pc + 1);                     \
        goto *throwDebugDispatchTable[opcode];  \
    } while (false)

#define DEBUG_DISPATCH_WIDE()                  \
    do {                                       \
        opcode = *(pc + 1);                    \
        goto *wideDebugDispatchTable[opcode];  \
    } while (false)

#define DEBUG_DISPATCH_DEPRECATED()                  \
    do {                                             \
        opcode = *(pc + 1);                          \
        goto *deprecatedDebugDispatchTable[opcode];  \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_FRAME(CurrentSp) \
    (reinterpret_cast<InterpretedFrame *>(CurrentSp) - 1)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_ENTRY_FRAME(sp) \
    (reinterpret_cast<InterpretedEntryFrame *>(sp) - 1)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
#define GET_BUILTIN_FRAME(sp) \
    (reinterpret_cast<InterpretedBuiltinFrame *>(sp) - 1)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SAVE_PC() (GET_FRAME(sp)->pc = pc)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SAVE_ACC() (GET_FRAME(sp)->acc = acc)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RESTORE_ACC() (acc = GET_FRAME(sp)->acc)  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_VREG(idx) (sp[idx])  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_VREG_VALUE(idx) (JSTaggedValue(sp[idx]))  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SET_VREG(idx, val) (sp[idx] = (val));  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
#define GET_ACC() (acc)                        // NOLINT(cppcoreguidelines-macro-usage)
#define SET_ACC(val) (acc = val)               // NOLINT(cppcoreguidelines-macro-usage)

#define GET_METHOD_FROM_CACHE(index) \
    ConstantPool::GetMethodFromCache(thread, constpool, index)

#define GET_STR_FROM_CACHE(index) \
    ConstantPool::GetStringFromCache(thread, constpool, index)

#define GET_LITERA_FROM_CACHE(index, type) \
    ConstantPool::GetLiteralFromCache<type>(thread, constpool, index)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_GOTO_EXCEPTION_HANDLER()          \
    do {                                              \
        SAVE_PC();                                    \
        goto *dispatchTable[EXCEPTION_OPCODE]; \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_HANDLE_RETURN()                                                     \
    do {                                                                                \
        size_t jumpSize = GetJumpSizeAfterCall(pc);                                     \
        DISPATCH_OFFSET(jumpSize);                                                      \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_SWITCH_TO_DEBUGGER_TABLE()                 \
    if (ecmaVm->GetJsDebuggerManager()->IsDebugMode()) { \
        dispatchTable = debugDispatchTable.data();       \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REAL_GOTO_DISPATCH_OPCODE(opcode)                       \
    do {                                                        \
        ASSERT(static_cast<uint16_t>(opcode) <= 0xff);          \
        goto *instDispatchTable[static_cast<uint8_t>(opcode)];  \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REAL_GOTO_SECOND_DISPATCH_OPCODE(opcode)                                    \
    do {                                                                            \
        ASSERT(static_cast<uint16_t>(opcode) > 0xff);                               \
        goto *instDispatchTable[static_cast<uint16_t>(opcode) >> sizeof(uint8_t)];  \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REAL_GOTO_EXCEPTION_HANDLER()                     \
    do {                                                  \
        SAVE_PC();                                        \
        goto *instDispatchTable[EXCEPTION_OPCODE]; \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_RETURN_IF_ABRUPT(result)      \
    do {                                          \
        if (result.IsException()) {               \
            INTERPRETER_GOTO_EXCEPTION_HANDLER(); \
        }                                         \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOTIFY_DEBUGGER_EVENT()          \
    do {                                 \
        SAVE_ACC();                      \
        SAVE_PC();                       \
        NotifyBytecodePcChanged(thread); \
        RESTORE_ACC();                   \
    } while (false)

/*
 * reasons of set acc with hole:
 * 1. acc will become illegal when new error
 * 2. debugger logic will save acc, so illegal acc will set to frame
 * 3. when debugger trigger gc, will mark an invalid acc and crash
 * 4. acc will set to exception later, so it can set to hole template
 */
#define NOTIFY_DEBUGGER_EXCEPTION_EVENT() \
    do {                                  \
        SET_ACC(JSTaggedValue::Hole());   \
        NOTIFY_DEBUGGER_EVENT();          \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_INITIALIZE()                                  \
    do {                                                              \
        SAVE_PC();                                                    \
        thread->CheckSafepoint();                                     \
        funcTagged = sp[startReg];                                    \
        JSTaggedValue funcValue(funcTagged);                          \
        if (!funcValue.IsCallable()) {                                \
            {                                                         \
                [[maybe_unused]] EcmaHandleScope handleScope(thread); \
                JSHandle<JSObject> error = factory->GetJSError(       \
                    ErrorType::TYPE_ERROR, "is not callable");        \
                thread->SetException(error.GetTaggedValue());         \
            }                                                         \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                     \
        }                                                             \
        funcObject = ECMAObject::Cast(funcValue.GetTaggedObject());   \
        methodHandle.Update(JSTaggedValue(funcObject->GetCallTarget())); \
        newSp = sp - InterpretedFrame::NumOfMembers();                \
    } while (false)

#define CALL_INITIALIZE()                                             \
    do {                                                              \
        SAVE_PC();                                                    \
        SAVE_ACC();                                                   \
        thread->CheckSafepoint();                                     \
        RESTORE_ACC();                                                \
        funcTagged = acc.GetRawData();                                \
        JSTaggedValue funcValue = acc;                                \
        if (!funcValue.IsCallable()) {                                \
            {                                                         \
                [[maybe_unused]] EcmaHandleScope handleScope(thread); \
                JSHandle<JSObject> error = factory->GetJSError(       \
                    ErrorType::TYPE_ERROR, "is not callable");        \
                thread->SetException(error.GetTaggedValue());         \
            }                                                         \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                     \
        }                                                             \
        funcObject = ECMAObject::Cast(funcValue.GetTaggedObject());   \
        methodHandle.Update(JSTaggedValue(funcObject->GetCallTarget())); \
        newSp = sp - InterpretedFrame::NumOfMembers();                \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_UNDEFINED(n)                           \
    do {                                                 \
        for (int i = 0; i < (n); i++) {                  \
            *(--newSp) = JSTaggedValue::VALUE_UNDEFINED; \
        }                                                \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_0()          \
    do {                            \
        /* do nothing when 0 arg */ \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_0() CALL_PUSH_ARGS_0()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_1()   \
    do {                     \
        *(--newSp) = sp[a0]; \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_1() CALL_PUSH_ARGS_1()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_2()   \
    do {                     \
        *(--newSp) = sp[a1]; \
        CALL_PUSH_ARGS_1();  \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_2() CALL_PUSH_ARGS_2()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_3()   \
    do {                     \
        *(--newSp) = sp[a2]; \
        CALL_PUSH_ARGS_2();  \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_3() CALL_PUSH_ARGS_3()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_RANGE()                                               \
    do {                                                                     \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - actualNumArgs))) { \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        for (int i = actualNumArgs; i >= 0; i--) {                           \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];            \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_RANGE()                                               \
    do {                                                                     \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - actualNumArgs))) { \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        for (int i = actualNumArgs; i > 0; i--) {                            \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];             \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_THISRANGE()                                          \
    do {                                                                     \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - actualNumArgs))) { \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        /* 1: skip this */                                                   \
        for (int i = actualNumArgs; i > 0; i--) {                        \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];             \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_THISRANGE()                                          \
    do {                                                                     \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - actualNumArgs))) { \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        /* 1: skip this */                                                   \
        for (int i = actualNumArgs + 1; i > 1; i--) {                        \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];             \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_0_NO_EXTRA() \
    do {                            \
        /* do nothing when 0 arg */ \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_0_NO_EXTRA() CALL_PUSH_ARGS_0_NO_EXTRA()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_1_NO_EXTRA()                             \
    do {                                                        \
        if (declaredNumArgs >= ActualNumArgsOfCall::CALLARG1) { \
            *(--newSp) = sp[a0];                                \
        }                                                       \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_1_NO_EXTRA() CALL_PUSH_ARGS_1_NO_EXTRA()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_2_NO_EXTRA()                              \
    do {                                                         \
        if (declaredNumArgs >= ActualNumArgsOfCall::CALLARGS2) { \
            *(--newSp) = sp[a1];                                 \
        }                                                        \
        DEPRECATED_CALL_PUSH_ARGS_1_NO_EXTRA();                             \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_2_NO_EXTRA() CALL_PUSH_ARGS_2_NO_EXTRA()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_3_NO_EXTRA()                              \
    do {                                                         \
        if (declaredNumArgs >= ActualNumArgsOfCall::CALLARGS3) { \
            *(--newSp) = sp[a2];                                 \
        }                                                        \
        DEPRECATED_CALL_PUSH_ARGS_2_NO_EXTRA();                             \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_3_NO_EXTRA() CALL_PUSH_ARGS_3_NO_EXTRA()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_RANGE_NO_EXTRA()                                      \
    do {                                                                     \
        int num = std::min(actualNumArgs, declaredNumArgs);                  \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - num))) {           \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        for (int i = num - 1; i >= 0; i--) {                                      \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];             \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_RANGE_NO_EXTRA()                                      \
    do {                                                                     \
        int num = std::min(actualNumArgs, declaredNumArgs);                  \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - num))) {           \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                            \
        }                                                                    \
        for (int i = num; i > 0; i--) {                                      \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];             \
        }                                                                    \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS_THISRANGE_NO_EXTRA()                                      \
    do {                                                                         \
        int num = std::min(actualNumArgs, declaredNumArgs);                      \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - num))) {               \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                                \
        }                                                                        \
        /* 1: skip this */                                                       \
        for (int i = num; i > 0; i--) {                                      \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];                 \
        }                                                                        \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS_THISRANGE_NO_EXTRA()                                      \
    do {                                                                         \
        int num = std::min(actualNumArgs, declaredNumArgs);                      \
        if (UNLIKELY(thread->DoStackOverflowCheck(newSp - num))) {               \
            INTERPRETER_GOTO_EXCEPTION_HANDLER();                                \
        }                                                                        \
        /* 1: skip this */                                                       \
        for (int i = num + 1; i > 1; i--) {                                      \
            *(--newSp) = sp[startReg + static_cast<uint32_t>(i)];                 \
        }                                                                        \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_PUSH_ARGS(ARG_TYPE)                                                   \
    do {                                                                           \
        if (methodHandle->IsNativeWithCallField()) {                               \
            /* native, just push all args directly */                              \
            CALL_PUSH_ARGS_##ARG_TYPE();                                           \
            goto setVregsAndFrameNative;                                           \
        }                                                                          \
        int32_t declaredNumArgs =                                                  \
            static_cast<int32_t>(methodHandle->GetNumArgsWithCallField());         \
        if (actualNumArgs == declaredNumArgs) {                                    \
            /* fast path, just push all args directly */                           \
            CALL_PUSH_ARGS_##ARG_TYPE();                                           \
            goto setVregsAndFrameNotNative;                                        \
        }                                                                          \
        /* slow path */                                                            \
        if (!methodHandle->HaveExtraWithCallField()) {                             \
            /* push length = declaredNumArgs, may push undefined */                \
            CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);                  \
            CALL_PUSH_ARGS_##ARG_TYPE##_NO_EXTRA();                                \
        } else {                                                                   \
            /* push actualNumArgs in the end, then all args, may push undefined */ \
            *(--newSp) = JSTaggedValue(actualNumArgs).GetRawData();                \
            CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);                  \
            CALL_PUSH_ARGS_##ARG_TYPE();                                           \
        }                                                                          \
        goto setVregsAndFrameNotNative;                                            \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEPRECATED_CALL_PUSH_ARGS(ARG_TYPE)                                                   \
    do {                                                                           \
        if (methodHandle->IsNativeWithCallField()) {                                     \
            /* native, just push all args directly */                              \
            DEPRECATED_CALL_PUSH_ARGS_##ARG_TYPE();                                           \
            goto deprecatedSetVregsAndFrameNative;                                           \
        }                                                                          \
        int32_t declaredNumArgs =                                                  \
            static_cast<int32_t>(methodHandle->GetNumArgsWithCallField());               \
        if (actualNumArgs == declaredNumArgs) {                                    \
            /* fast path, just push all args directly */                           \
            DEPRECATED_CALL_PUSH_ARGS_##ARG_TYPE();                                           \
            goto deprecatedSetVregsAndFrameNotNative;                                        \
        }                                                                          \
        /* slow path */                                                            \
        if (!methodHandle->HaveExtraWithCallField()) {                                   \
            /* push length = declaredNumArgs, may push undefined */                \
            CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);                  \
            DEPRECATED_CALL_PUSH_ARGS_##ARG_TYPE##_NO_EXTRA();                                \
        } else {                                                                   \
            /* push actualNumArgs in the end, then all args, may push undefined */ \
            *(--newSp) = JSTaggedValue(actualNumArgs).GetRawData();                \
            CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);                  \
            DEPRECATED_CALL_PUSH_ARGS_##ARG_TYPE();                                           \
        }                                                                          \
        goto deprecatedSetVregsAndFrameNotNative;                                            \
    } while (false)

#if ECMASCRIPT_ENABLE_IC
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPDATE_HOTNESS_COUNTER_NON_ACC(offset)   (UpdateHotnessCounter(thread, sp, acc, offset))

#define UPDATE_HOTNESS_COUNTER(offset)                       \
    do {                                                     \
        if (UpdateHotnessCounter(thread, sp, acc, offset)) { \
            RESTORE_ACC();                                   \
        }                                                    \
    } while (false)
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPDATE_HOTNESS_COUNTER(offset) static_cast<void>(0)
#define UPDATE_HOTNESS_COUNTER_NON_ACC(offset) static_cast<void>(0)
#endif

#define READ_INST_OP() READ_INST_8(0)               // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_4_0() (READ_INST_8(1) & 0xf)      // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_4_1() (READ_INST_8(1) >> 4 & 0xf) // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_4_2() (READ_INST_8(2) & 0xf)      // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_4_3() (READ_INST_8(2) >> 4 & 0xf) // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_0() READ_INST_8(1)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_1() READ_INST_8(2)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_2() READ_INST_8(3)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_3() READ_INST_8(4)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_4() READ_INST_8(5)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_5() READ_INST_8(6)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_6() READ_INST_8(7)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_7() READ_INST_8(8)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8_8() READ_INST_8(9)              // NOLINT(hicpp-signed-bitwise, cppcoreguidelines-macro-usage)
#define READ_INST_8(offset) (*(pc + (offset)))
#define MOVE_AND_READ_INST_8(currentInst, offset) \
    currentInst <<= 8;                            \
    currentInst += READ_INST_8(offset);           \

#define READ_INST_16_0() READ_INST_16(2)
#define READ_INST_16_1() READ_INST_16(3)
#define READ_INST_16_2() READ_INST_16(4)
#define READ_INST_16_3() READ_INST_16(5)
#define READ_INST_16_4() READ_INST_16(6)
#define READ_INST_16_5() READ_INST_16(7)
#define READ_INST_16_6() READ_INST_16(8)
#define READ_INST_16(offset)                          \
    ({                                                \
        uint16_t currentInst = READ_INST_8(offset);   \
        MOVE_AND_READ_INST_8(currentInst, offset - 1) \
    })

#define READ_INST_32_0() READ_INST_32(4)
#define READ_INST_32_1() READ_INST_32(5)
#define READ_INST_32_2() READ_INST_32(6)
#define READ_INST_32(offset)                          \
    ({                                                \
        uint32_t currentInst = READ_INST_8(offset);   \
        MOVE_AND_READ_INST_8(currentInst, offset - 1) \
        MOVE_AND_READ_INST_8(currentInst, offset - 2) \
        MOVE_AND_READ_INST_8(currentInst, offset - 3) \
    })

#define READ_INST_64_0()                       \
    ({                                         \
        uint64_t currentInst = READ_INST_8(8); \
        MOVE_AND_READ_INST_8(currentInst, 7)   \
        MOVE_AND_READ_INST_8(currentInst, 6)   \
        MOVE_AND_READ_INST_8(currentInst, 5)   \
        MOVE_AND_READ_INST_8(currentInst, 4)   \
        MOVE_AND_READ_INST_8(currentInst, 3)   \
        MOVE_AND_READ_INST_8(currentInst, 2)   \
        MOVE_AND_READ_INST_8(currentInst, 1)   \
    })

JSTaggedValue EcmaInterpreter::ExecuteNative(EcmaRuntimeCallInfo *info)
{
    JSThread *thread = info->GetThread();
    INTERPRETER_TRACE(thread, ExecuteNative);

    // current is entry frame.
    JSTaggedType *sp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    JSTaggedType *newSp = sp - InterpretedEntryFrame::NumOfMembers();

    InterpretedFrame *state = GET_FRAME(newSp);
    state->base.prev = sp;
    state->base.type = FrameType::INTERPRETER_FRAME;
    state->pc = nullptr;
    state->function = info->GetFunctionValue();
    thread->SetCurrentSPFrame(newSp);
#if ECMASCRIPT_ENABLE_ACTIVE_CPUPROFILER
    CpuProfiler *profiler = thread->GetEcmaVM()->GetProfiler();
    if (profiler != nullptr) {
        profiler->IsNeedAndGetStack(thread);
    }
#endif
    thread->CheckSafepoint();
    ECMAObject *callTarget = reinterpret_cast<ECMAObject*>(info->GetFunctionValue().GetTaggedObject());
    Method *method = callTarget->GetCallTarget();
    LOG_INST() << "Entry: Runtime Call.";
    JSTaggedValue tagged =
        reinterpret_cast<EcmaEntrypoint>(const_cast<void *>(method->GetNativePointer()))(info);
    LOG_INST() << "Exit: Runtime Call.";

    InterpretedEntryFrame *entryState = GET_ENTRY_FRAME(sp);
    JSTaggedType *prevSp = entryState->base.prev;
    thread->SetCurrentSPFrame(prevSp);
#if ECMASCRIPT_ENABLE_ACTIVE_CPUPROFILER
    if (profiler != nullptr) {
        profiler->IsNeedAndGetStack(thread);
    }
#endif
    return tagged;
}

JSTaggedValue EcmaInterpreter::Execute(EcmaRuntimeCallInfo *info)
{
    if (info == nullptr) {
        return JSTaggedValue::Exception();
    }

    JSThread *thread = info->GetThread();
    INTERPRETER_TRACE(thread, Execute);
    if (thread->IsAsmInterpreter()) {
        return InterpreterAssembly::Execute(info);
    }

    JSHandle<JSTaggedValue> func = info->GetFunction();
    ECMAObject *callTarget = reinterpret_cast<ECMAObject*>(func.GetTaggedValue().GetTaggedObject());
    ASSERT(callTarget != nullptr);
    Method *method = callTarget->GetCallTarget();
    if (method->IsNativeWithCallField()) {
        return EcmaInterpreter::ExecuteNative(info);
    }

    // current is entry frame.
    JSTaggedType *sp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());
    int32_t actualNumArgs = static_cast<int32_t>(info->GetArgsNumber());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    JSTaggedType *newSp = sp - InterpretedEntryFrame::NumOfMembers();
    if (UNLIKELY(thread->DoStackOverflowCheck(newSp - actualNumArgs - NUM_MANDATORY_JSFUNC_ARGS))) {
        return JSTaggedValue::Undefined();
    }

    int32_t declaredNumArgs = static_cast<int32_t>(method->GetNumArgsWithCallField());
    // push args
    if (actualNumArgs == declaredNumArgs) {
        // fast path, just push all args directly
        for (int i = actualNumArgs - 1; i >= 0; i--) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = info->GetCallArgValue(i).GetRawData();
        }
    } else {
        // slow path
        if (!method->HaveExtraWithCallField()) {
            // push length = declaredNumArgs, may push undefined
            if (declaredNumArgs > actualNumArgs) {
                CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);
            }
            for (int32_t i = std::min(actualNumArgs, declaredNumArgs) - 1; i >= 0; i--) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                *(--newSp) = info->GetCallArgValue(i).GetRawData();
            }
        } else {
            // push actualNumArgs in the end, then all args, may push undefined
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = JSTaggedValue(actualNumArgs).GetRawData();
            if (declaredNumArgs > actualNumArgs) {
                CALL_PUSH_UNDEFINED(declaredNumArgs - actualNumArgs);
            }
            for (int32_t i = actualNumArgs - 1; i >= 0; i--) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                *(--newSp) = info->GetCallArgValue(i).GetRawData();
            }
        }
    }
    uint64_t callField = method->GetCallField();
    if ((callField & CALL_TYPE_MASK) != 0) {
        // not normal call type, setting func/newTarget/this cannot be skipped
        if (method->HaveThisWithCallField()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = info->GetThisValue().GetRawData();  // push this
        }
        if (method->HaveNewTargetWithCallField()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = info->GetNewTargetValue().GetRawData();  // push new target
        }
        if (method->HaveFuncWithCallField()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = info->GetFunctionValue().GetRawData();  // push func
        }
    }
    int32_t numVregs = static_cast<int32_t>(method->GetNumVregsWithCallField());
    if (UNLIKELY(thread->DoStackOverflowCheck(newSp - numVregs))) {
        return JSTaggedValue::Undefined();
    }
    // push vregs
    CALL_PUSH_UNDEFINED(numVregs);

    const uint8_t *pc = method->GetBytecodeArray();
    InterpretedFrame *state = GET_FRAME(newSp);
    state->pc = pc;
    state->function = info->GetFunctionValue();
    state->acc = JSTaggedValue::Hole();

    state->constpool = method->GetConstantPool();
    JSHandle<JSFunction> thisFunc = JSHandle<JSFunction>::Cast(func);
    state->profileTypeInfo = thisFunc->GetProfileTypeInfo();
    state->base.prev = sp;
    state->base.type = FrameType::INTERPRETER_FRAME;
    state->env = thisFunc->GetLexicalEnv();
    thread->SetCurrentSPFrame(newSp);
#if ECMASCRIPT_ENABLE_ACTIVE_CPUPROFILER
    CpuProfiler *profiler = thread->GetEcmaVM()->GetProfiler();
    if (profiler != nullptr) {
        profiler->IsNeedAndGetStack(thread);
    }
#endif
    thread->CheckSafepoint();
    LOG_INST() << "Entry: Runtime Call " << std::hex << reinterpret_cast<uintptr_t>(newSp) << " "
                            << std::hex << reinterpret_cast<uintptr_t>(pc);

    EcmaInterpreter::RunInternal(thread, pc, newSp);

    // NOLINTNEXTLINE(readability-identifier-naming)
    const JSTaggedValue resAcc = state->acc;
    // pop frame
    InterpretedEntryFrame *entryState = GET_ENTRY_FRAME(sp);
    JSTaggedType *prevSp = entryState->base.prev;
    thread->SetCurrentSPFrame(prevSp);
#if ECMASCRIPT_ENABLE_ACTIVE_CPUPROFILER
    if (profiler != nullptr) {
        profiler->IsNeedAndGetStack(thread);
    }
#endif
    return resAcc;
}

JSTaggedValue EcmaInterpreter::GeneratorReEnterInterpreter(JSThread *thread, JSHandle<GeneratorContext> context)
{
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSFunction> func = JSHandle<JSFunction>::Cast(JSHandle<JSTaggedValue>(thread, context->GetMethod()));
    Method *method = func->GetCallTarget();
    if (method->IsAotWithCallField()) {
        return GeneratorReEnterAot(thread, context);
    }

    if (thread->IsAsmInterpreter()) {
        return InterpreterAssembly::GeneratorReEnterInterpreter(thread, context);
    }

    JSTaggedType *currentSp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());

    // push break frame
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    JSTaggedType *breakSp = currentSp - InterpretedFrame::NumOfMembers();
    if (UNLIKELY(thread->DoStackOverflowCheck(breakSp))) {
        return JSTaggedValue::Exception();
    }

    InterpretedFrame *breakState = GET_FRAME(breakSp);
    breakState->pc = nullptr;
    breakState->function = JSTaggedValue::Hole();
    breakState->base.prev = currentSp;
    breakState->base.type = FrameType::INTERPRETER_FRAME;

    // create new frame and resume sp and pc
    uint32_t nregs = context->GetNRegs();
    size_t newFrameSize = InterpretedFrame::NumOfMembers() + nregs;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic
    JSTaggedType *newSp = breakSp - newFrameSize;
    if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
        return JSTaggedValue::Exception();
    }
    JSHandle<TaggedArray> regsArray(thread, context->GetRegsArray());
    for (size_t i = 0; i < nregs; i++) {
        newSp[i] = regsArray->Get(i).GetRawData();  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    uint32_t pcOffset = context->GetBCOffset();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const uint8_t *resumePc = method->GetBytecodeArray() + pcOffset;

    InterpretedFrame *state = GET_FRAME(newSp);
    state->pc = resumePc;
    state->function = func.GetTaggedValue();
    state->constpool = method->GetConstantPool();
    state->profileTypeInfo = func->GetProfileTypeInfo();
    state->acc = context->GetAcc();
    state->base.prev = breakSp;
    state->base.type = FrameType::INTERPRETER_FRAME;
    JSTaggedValue env = context->GetLexicalEnv();
    state->env = env;
    // execute interpreter
    thread->SetCurrentSPFrame(newSp);

    EcmaInterpreter::RunInternal(thread, resumePc, newSp);

    JSTaggedValue res = state->acc;
    // pop frame
    thread->SetCurrentSPFrame(currentSp);
    return res;
}

JSTaggedValue EcmaInterpreter::GeneratorReEnterAot(JSThread *thread, JSHandle<GeneratorContext> context)
{
    JSHandle<JSFunction> func = JSHandle<JSFunction>::Cast(JSHandle<JSTaggedValue>(thread, context->GetMethod()));
    Method *method = func->GetCallTarget();
    JSTaggedValue genObject = context->GetGeneratorObject();
    std::vector<JSTaggedType> args(method->GetNumArgs() + NUM_MANDATORY_JSFUNC_ARGS + 1,
                                   JSTaggedValue::Undefined().GetRawData());
    args[0] = func.GetTaggedValue().GetRawData();
    args[1] = genObject.GetRawData();
    JSTaggedValue env = func->GetLexicalEnv();
    args[args.size() - 1] = env.GetRawData(); // last arg is env.
    auto entry = thread->GetRTInterface(kungfu::RuntimeStubCSigns::ID_JSFunctionEntry);
    auto res = reinterpret_cast<JSFunctionEntryType>(entry)(thread->GetGlueAddr(),
                                                            reinterpret_cast<uintptr_t>(thread->GetLastLeaveFrame()),
                                                            static_cast<uint32_t>(args.size()) - 1,
                                                            static_cast<uint32_t>(args.size()) - 1,
                                                            args.data(),
                                                            method->GetCodeEntry());
    return JSTaggedValue(res);
}

void EcmaInterpreter::NotifyBytecodePcChanged(JSThread *thread)
{
    FrameHandler frameHandler(thread);
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        if (frameHandler.IsEntryFrame()) {
            continue;
        }
        Method *method = frameHandler.GetMethod();
        // Skip builtins method
        if (method->IsNativeWithCallField()) {
            continue;
        }
        auto bcOffset = frameHandler.GetBytecodeOffset();
        auto *debuggerMgr = thread->GetEcmaVM()->GetJsDebuggerManager();
        debuggerMgr->GetNotificationManager()->BytecodePcChangedEvent(thread, method, bcOffset);
        return;
    }
}

const JSPandaFile *EcmaInterpreter::GetNativeCallPandafile(JSThread *thread)
{
    FrameHandler frameHandler(thread);
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        if (frameHandler.IsEntryFrame()) {
            continue;
        }
        Method *method = frameHandler.GetMethod();
        // Skip builtins method
        if (method->IsNativeWithCallField()) {
            continue;
        }
        const JSPandaFile *jsPandaFile = method->GetJSPandaFile();
        return jsPandaFile;
    }
    UNREACHABLE();
}

// NOLINTNEXTLINE(readability-function-size)
NO_UB_SANITIZE void EcmaInterpreter::RunInternal(JSThread *thread, const uint8_t *pc, JSTaggedType *sp)
{
    INTERPRETER_TRACE(thread, RunInternal);
    uint8_t opcode = READ_INST_OP();
    JSTaggedValue acc = JSTaggedValue::Hole();
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    JSTaggedValue globalObj = globalEnv->GetGlobalObject();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSMutableHandle<Method> methodHandle(thread, JSTaggedValue::Undefined());

    constexpr size_t numOps = 0x100;
    constexpr size_t numThrowOps = 9;
    constexpr size_t numWideOps = 20;
    constexpr size_t numDeprecatedOps = 46;

    static std::array<const void *, numOps> instDispatchTable {
#include "templates/instruction_dispatch.inl"
    };

    static std::array<const void *, numThrowOps> throwDispatchTable {
#include "templates/throw_instruction_dispatch.inl"
    };

    static std::array<const void *, numWideOps> wideDispatchTable {
#include "templates/wide_instruction_dispatch.inl"
    };

    static std::array<const void *, numDeprecatedOps> deprecatedDispatchTable {
#include "templates/deprecated_instruction_dispatch.inl"
    };

    static std::array<const void *, numOps> debugDispatchTable {
#include "templates/debugger_instruction_dispatch.inl"
    };

    static std::array<const void *, numThrowOps> throwDebugDispatchTable {
#include "templates/debugger_throw_instruction_dispatch.inl"
    };

    static std::array<const void *, numWideOps> wideDebugDispatchTable {
#include "templates/debugger_wide_instruction_dispatch.inl"
    };

    static std::array<const void *, numDeprecatedOps> deprecatedDebugDispatchTable {
#include "templates/debugger_deprecated_instruction_dispatch.inl"
    };

    auto *dispatchTable = instDispatchTable.data();
    CHECK_SWITCH_TO_DEBUGGER_TABLE();
    goto *dispatchTable[opcode];

    HANDLE_OPCODE(MOV_V4_V4) {
        uint16_t vdst = READ_INST_4_0();
        uint16_t vsrc = READ_INST_4_1();
        LOG_INST() << "mov v" << vdst << ", v" << vsrc;
        uint64_t value = GET_VREG(vsrc);
        SET_VREG(vdst, value)
        DISPATCH(MOV_V4_V4);
    }
    HANDLE_OPCODE(MOV_V8_V8) {
        uint16_t vdst = READ_INST_8_0();
        uint16_t vsrc = READ_INST_8_1();
        LOG_INST() << "mov v" << vdst << ", v" << vsrc;
        uint64_t value = GET_VREG(vsrc);
        SET_VREG(vdst, value)
        DISPATCH(MOV_V8_V8);
    }
    HANDLE_OPCODE(MOV_V16_V16) {
        uint16_t vdst = READ_INST_16_0();
        uint16_t vsrc = READ_INST_16_2();
        LOG_INST() << "mov v" << vdst << ", v" << vsrc;
        uint64_t value = GET_VREG(vsrc);
        SET_VREG(vdst, value)
        DISPATCH(MOV_V16_V16);
    }
    HANDLE_OPCODE(LDA_STR_ID16) {
        uint16_t stringId = READ_INST_16_0();
        LOG_INST() << "lda.str " << std::hex << stringId;
        auto constpool = GetConstantPool(sp);
        SET_ACC(GET_STR_FROM_CACHE(stringId));
        DISPATCH(LDA_STR_ID16);
    }
    HANDLE_OPCODE(JMP_IMM8) {
        int8_t offset = READ_INST_8_0();
        UPDATE_HOTNESS_COUNTER(offset);
        LOG_INST() << "jmp " << std::hex << static_cast<int32_t>(offset);
        DISPATCH_OFFSET(offset);
    }
    HANDLE_OPCODE(JMP_IMM16) {
        int16_t offset = READ_INST_16_0();
        UPDATE_HOTNESS_COUNTER(offset);
        LOG_INST() << "jmp " << std::hex << static_cast<int32_t>(offset);
        DISPATCH_OFFSET(offset);
    }
    HANDLE_OPCODE(JMP_IMM32) {
        int32_t offset = READ_INST_32_0();
        UPDATE_HOTNESS_COUNTER(offset);
        LOG_INST() << "jmp " << std::hex << offset;
        DISPATCH_OFFSET(offset);
    }
    HANDLE_OPCODE(JEQZ_IMM8) {
        int8_t offset = READ_INST_8_0();
        LOG_INST() << "jeqz ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::False() || (GET_ACC().IsInt() && GET_ACC().GetInt() == 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() == 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JEQZ_IMM8);
        }
    }
    HANDLE_OPCODE(JEQZ_IMM16) {
        int16_t offset = READ_INST_16_0();
        LOG_INST() << "jeqz ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::False() || (GET_ACC().IsInt() && GET_ACC().GetInt() == 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() == 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JEQZ_IMM16);
        }
    }
    HANDLE_OPCODE(JEQZ_IMM32) {
        int32_t offset = READ_INST_32_0();
        LOG_INST() << "jeqz ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::False() || (GET_ACC().IsInt() && GET_ACC().GetInt() == 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() == 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JEQZ_IMM32);
        }
    }
    HANDLE_OPCODE(JNEZ_IMM8) {
        int8_t offset = READ_INST_8_0();
        LOG_INST() << "jnez ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::True() || (GET_ACC().IsInt() && GET_ACC().GetInt() != 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() != 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JNEZ_IMM8);
        }
    }
    HANDLE_OPCODE(JNEZ_IMM16) {
        int16_t offset = READ_INST_16_0();
        LOG_INST() << "jnez ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::True() || (GET_ACC().IsInt() && GET_ACC().GetInt() != 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() != 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JNEZ_IMM16);
        }
    }
    HANDLE_OPCODE(JNEZ_IMM32) {
        int32_t offset = READ_INST_32_0();
        LOG_INST() << "jnez ->\t"
                   << "cond jmpz " << std::hex << static_cast<int32_t>(offset);
        if (GET_ACC() == JSTaggedValue::True() || (GET_ACC().IsInt() && GET_ACC().GetInt() != 0) ||
            (GET_ACC().IsDouble() && GET_ACC().GetDouble() != 0)) {
            UPDATE_HOTNESS_COUNTER(offset);
            DISPATCH_OFFSET(offset);
        } else {
            DISPATCH(JNEZ_IMM32);
        }
    }
    HANDLE_OPCODE(LDA_V8) {
        uint16_t vsrc = READ_INST_8_0();
        LOG_INST() << "lda v" << vsrc;
        uint64_t value = GET_VREG(vsrc);
        SET_ACC(JSTaggedValue(value));
        DISPATCH(LDA_V8);
    }
    HANDLE_OPCODE(STA_V8) {
        uint16_t vdst = READ_INST_8_0();
        LOG_INST() << "sta v" << vdst;
        SET_VREG(vdst, GET_ACC().GetRawData())
        DISPATCH(STA_V8);
    }
    HANDLE_OPCODE(LDAI_IMM32) {
        int32_t imm = READ_INST_32_0();
        LOG_INST() << "ldai " << std::hex << imm;
        SET_ACC(JSTaggedValue(imm));
        DISPATCH(LDAI_IMM32);
    }

    HANDLE_OPCODE(FLDAI_IMM64) {
        auto imm = bit_cast<double>(READ_INST_64_0());
        LOG_INST() << "fldai " << imm;
        SET_ACC(JSTaggedValue(imm));
        DISPATCH(FLDAI_IMM64);
    }
    {
        int32_t actualNumArgs;
        int32_t startReg;
        JSTaggedType funcTagged;
        ECMAObject *funcObject;
        JSTaggedType *newSp;
        bool callThis;

        HANDLE_OPCODE(CALLARG0_IMM8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG0;
            LOG_INST() << "callarg0";
            CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(0);
        }
        HANDLE_OPCODE(DEPRECATED_CALLARG0_PREF_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG0;
            startReg = READ_INST_8_1();
            LOG_INST() << "callarg0 "
                       << "v" << startReg;
            DEPRECATED_CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(0);
        }
        HANDLE_OPCODE(CALLARG1_IMM8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG1;
            uint32_t a0 = READ_INST_8_1();
            LOG_INST() << "callarg1 "
                       << "v" << a0;
            CALL_INITIALIZE();
            callThis = false;
            CALL_PUSH_ARGS(1);
        }
        HANDLE_OPCODE(DEPRECATED_CALLARG1_PREF_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG1;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            LOG_INST() << "callarg1 "
                       << "v" << startReg << ", v" << a0;
            DEPRECATED_CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(1);
        }
        HANDLE_OPCODE(CALLARGS2_IMM8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS2;
            uint32_t a0 = READ_INST_8_1();
            uint32_t a1 = READ_INST_8_2();
            LOG_INST() << "callargs2 "
                       << "v" << a0 << ", v" << a1;
            CALL_INITIALIZE();
            callThis = false;
            CALL_PUSH_ARGS(2);
        }
        HANDLE_OPCODE(DEPRECATED_CALLARGS2_PREF_V8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS2;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            LOG_INST() << "callargs2 "
                       << "v" << startReg << ", v" << a0 << ", v" << a1;
            DEPRECATED_CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(2);
        }
        HANDLE_OPCODE(CALLARGS3_IMM8_V8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS3;
            uint32_t a0 = READ_INST_8_1();
            uint32_t a1 = READ_INST_8_2();
            uint32_t a2 = READ_INST_8_3();
            LOG_INST() << "callargs3 "
                       << "v" << a0 << ", v" << a1 << ", v" << a2;
            CALL_INITIALIZE();
            callThis = false;
            CALL_PUSH_ARGS(3);
        }
        HANDLE_OPCODE(DEPRECATED_CALLARGS3_PREF_V8_V8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS3;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            uint32_t a2 = READ_INST_8_4();
            LOG_INST() << "callargs3 "
                       << "v" << startReg << ", v" << a0 << ", v" << a1 << ", v" << a2;
            DEPRECATED_CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(3);
        }
        HANDLE_OPCODE(CALLTHIS0_IMM8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG0;
            startReg = READ_INST_8_1();
            LOG_INST() << "call.this0, v" << startReg;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(0);
        }
        HANDLE_OPCODE(CALLTHIS1_IMM8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARG1;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            LOG_INST() << "call.this1, v" << startReg << " v" << a0;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(1);
        }
        HANDLE_OPCODE(CALLTHIS2_IMM8_V8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS2;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            LOG_INST() << "call.this2, v" << startReg << " v" << a0 << " v" << a1;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(2);
        }
        HANDLE_OPCODE(CALLTHIS3_IMM8_V8_V8_V8_V8) {
            actualNumArgs = ActualNumArgsOfCall::CALLARGS3;
            startReg = READ_INST_8_1();
            uint32_t a0 = READ_INST_8_2();
            uint32_t a1 = READ_INST_8_3();
            uint32_t a2 = READ_INST_8_4();
            LOG_INST() << "call.this3, v" << startReg << " v" << a0 << " v" << a1 << " v" << a2;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(3);
        }
        HANDLE_OPCODE(CALLTHISRANGE_IMM8_IMM8_V8) {
            actualNumArgs = READ_INST_8_1();
            startReg = READ_INST_8_2();
            LOG_INST() << "call.this.range " << actualNumArgs << ", v" << startReg;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(THISRANGE);
        }
        HANDLE_OPCODE(WIDE_CALLTHISRANGE_PREF_IMM16_V8) {
            actualNumArgs = READ_INST_16_1();
            startReg = READ_INST_8_3();
            LOG_INST() << "call.this.range " << actualNumArgs << ", v" << startReg;
            CALL_INITIALIZE();
            callThis = true;
            CALL_PUSH_ARGS(THISRANGE);
        }
        HANDLE_OPCODE(DEPRECATED_CALLTHISRANGE_PREF_IMM16_V8) {
            actualNumArgs = static_cast<int32_t>(READ_INST_16_1() - 1);  // 1: exclude this
            startReg = READ_INST_8_3();
            LOG_INST() << "call.this.range " << actualNumArgs << ", v" << startReg;
            DEPRECATED_CALL_INITIALIZE();
            callThis = true;
            DEPRECATED_CALL_PUSH_ARGS(THISRANGE);
        }
        HANDLE_OPCODE(CALLRANGE_IMM8_IMM8_V8) {
            actualNumArgs = READ_INST_8_1();
            startReg = READ_INST_8_2();
            LOG_INST() << "calli.range " << actualNumArgs << ", v" << startReg;
            CALL_INITIALIZE();
            callThis = false;
            CALL_PUSH_ARGS(RANGE);
        }
        HANDLE_OPCODE(WIDE_CALLRANGE_PREF_IMM16_V8) {
            actualNumArgs = READ_INST_16_1();
            startReg = READ_INST_8_3();
            LOG_INST() << "calli.range " << actualNumArgs << ", v" << startReg;
            CALL_INITIALIZE();
            callThis = false;
            CALL_PUSH_ARGS(RANGE);
        }
        HANDLE_OPCODE(DEPRECATED_CALLRANGE_PREF_IMM16_V8) {
            actualNumArgs = READ_INST_16_1();
            startReg = READ_INST_8_3();
            LOG_INST() << "calli.range " << actualNumArgs << ", v" << startReg;
            DEPRECATED_CALL_INITIALIZE();
            callThis = false;
            DEPRECATED_CALL_PUSH_ARGS(RANGE);
        }
        setVregsAndFrameNative:
            startReg--;
        deprecatedSetVregsAndFrameNative: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *(--newSp) = (callThis ? sp[startReg + 1] : JSTaggedValue::VALUE_UNDEFINED);  // push this
            *(--newSp) = JSTaggedValue::VALUE_UNDEFINED;  // push new target
            *(--newSp) = ToUintPtr(funcObject);  // push func
            ASSERT(methodHandle->GetNumVregsWithCallField() == 0);  // no need to push vregs
            *(--newSp) = actualNumArgs + NUM_MANDATORY_JSFUNC_ARGS;
            *(--newSp) = ToUintPtr(thread);
            EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp);

            InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
            state->base.prev = sp;
            state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
            state->pc = nullptr;
            state->function = JSTaggedValue(funcTagged);
            thread->SetCurrentSPFrame(newSp);
            LOG_INST() << "Entry: Runtime Call.";
            SAVE_PC();
            JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
            thread->SetCurrentSPFrame(sp);
            if (UNLIKELY(thread->HasPendingException())) {
                INTERPRETER_GOTO_EXCEPTION_HANDLER();
            }
            LOG_INST() << "Exit: Runtime Call.";
            SET_ACC(retValue);
            INTERPRETER_HANDLE_RETURN();
        }
        setVregsAndFrameNotNative:
            startReg--;
        deprecatedSetVregsAndFrameNotNative: {
            if (JSFunction::Cast(funcObject)->IsClassConstructor()) {
                {
                    [[maybe_unused]] EcmaHandleScope handleScope(thread);
                    JSHandle<JSObject> error =
                        factory->GetJSError(ErrorType::TYPE_ERROR, "class constructor cannot called without 'new'");
                    thread->SetException(error.GetTaggedValue());
                }
                INTERPRETER_GOTO_EXCEPTION_HANDLER();
            }
            uint64_t callField = methodHandle->GetCallField();
            if ((callField & CALL_TYPE_MASK) != 0) {
                // not normal call type, setting func/newTarget/this cannot be skipped
                if (methodHandle->HaveThisWithCallField()) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    *(--newSp) = (callThis ? sp[startReg + 1] : JSTaggedValue::VALUE_UNDEFINED);  // push this
                }
                if (methodHandle->HaveNewTargetWithCallField()) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    *(--newSp) = JSTaggedValue::VALUE_UNDEFINED;  // push new target
                }
                if (methodHandle->HaveFuncWithCallField()) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    *(--newSp) = ToUintPtr(funcObject);  // push func
                }
            }
            int32_t numVregs = static_cast<int32_t>(methodHandle->GetNumVregsWithCallField());
            if (UNLIKELY(thread->DoStackOverflowCheck(newSp - numVregs))) {
                INTERPRETER_GOTO_EXCEPTION_HANDLER();
            }
            // push vregs
            CALL_PUSH_UNDEFINED(numVregs);
            SAVE_PC();

            InterpretedFrame *state = GET_FRAME(newSp);
            state->base.prev = sp;
            state->base.type = FrameType::INTERPRETER_FRAME;
            state->pc = pc = methodHandle->GetBytecodeArray();
            sp = newSp;
            state->function = JSTaggedValue(funcTagged);
            state->acc = JSTaggedValue::Hole();
            state->constpool = methodHandle->GetConstantPool();
            state->profileTypeInfo = JSFunction::Cast(funcObject)->GetProfileTypeInfo();
            JSTaggedValue env = JSFunction::Cast(funcObject)->GetLexicalEnv();
            state->env = env;
            thread->SetCurrentSPFrame(newSp);
            LOG_INST() << "Entry: Runtime Call " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                    << std::hex << reinterpret_cast<uintptr_t>(pc);
            DISPATCH_OFFSET(0);
        }
    }
    HANDLE_OPCODE(APPLY_IMM8_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::callspread"
                    << " v" << v0 << " v" << v1;
        JSTaggedValue func = GET_ACC();
        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue array = GET_VREG_VALUE(v1);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CallSpread(thread, func, obj, array);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        DISPATCH(APPLY_IMM8_V8_V8);
    }
    HANDLE_OPCODE(DEPRECATED_CALLSPREAD_PREF_V8_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        uint16_t v2 = READ_INST_8_3();
        LOG_INST() << "intrinsics::callspread"
                    << " v" << v0 << " v" << v1 << " v" << v2;
        JSTaggedValue func = GET_VREG_VALUE(v0);
        JSTaggedValue obj = GET_VREG_VALUE(v1);
        JSTaggedValue array = GET_VREG_VALUE(v2);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CallSpread(thread, func, obj, array);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        DISPATCH(DEPRECATED_CALLSPREAD_PREF_V8_V8_V8);
    }
    HANDLE_OPCODE(RETURN) {
        LOG_INST() << "return";
        InterpretedFrame *state = GET_FRAME(sp);
        LOG_INST() << "Exit: Runtime Call " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                << std::hex << reinterpret_cast<uintptr_t>(state->pc);
        methodHandle.Update(JSFunction::Cast(state->function.GetTaggedObject())->GetMethod());
        [[maybe_unused]] auto fistPC = methodHandle->GetBytecodeArray();
        UPDATE_HOTNESS_COUNTER(-(pc - fistPC));
        JSTaggedType *currentSp = sp;
        sp = state->base.prev;
        ASSERT(sp != nullptr);
        InterpretedFrame *prevState = GET_FRAME(sp);
        pc = prevState->pc;
        // entry frame
        if (FrameHandler::IsEntryFrame(pc)) {
            state->acc = acc;
            return;
        }
        thread->SetCurrentSPFrame(sp);
        if (IsFastNewFrameExit(currentSp)) {
            JSFunction *func = JSFunction::Cast(GetThisFunction(currentSp).GetTaggedObject());
            if (acc.IsECMAObject()) {
                INTERPRETER_HANDLE_RETURN();
            }

            if (func->IsBase()) {
                JSTaggedValue thisObject = GetThisObjectFromFastNewFrame(currentSp);
                SET_ACC(thisObject);
                INTERPRETER_HANDLE_RETURN();
            }

            if (!acc.IsUndefined()) {
                {
                    [[maybe_unused]] EcmaHandleScope handleScope(thread);
                    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
                        "Derived constructor must return object or undefined");
                    thread->SetException(error.GetTaggedValue());
                }
                INTERPRETER_GOTO_EXCEPTION_HANDLER();
            }

            JSTaggedValue thisObject = GetThisObjectFromFastNewFrame(currentSp);
            SET_ACC(thisObject);
            INTERPRETER_HANDLE_RETURN();
        }
        INTERPRETER_HANDLE_RETURN();
    }
    HANDLE_OPCODE(RETURNUNDEFINED) {
        LOG_INST() << "return.undefined";
        InterpretedFrame *state = GET_FRAME(sp);
        LOG_INST() << "Exit: Runtime Call " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                << std::hex << reinterpret_cast<uintptr_t>(state->pc);
        methodHandle.Update(JSFunction::Cast(state->function.GetTaggedObject())->GetMethod());
        [[maybe_unused]] auto fistPC = methodHandle->GetBytecodeArray();
        UPDATE_HOTNESS_COUNTER_NON_ACC(-(pc - fistPC));
        JSTaggedType *currentSp = sp;
        sp = state->base.prev;
        ASSERT(sp != nullptr);
        InterpretedFrame *prevState = GET_FRAME(sp);
        pc = prevState->pc;
        // entry frame
        if (FrameHandler::IsEntryFrame(pc)) {
            state->acc = JSTaggedValue::Undefined();
            return;
        }
        thread->SetCurrentSPFrame(sp);
        if (IsFastNewFrameExit(currentSp)) {
            JSFunction *func = JSFunction::Cast(GetThisFunction(currentSp).GetTaggedObject());
            if (func->IsBase()) {
                JSTaggedValue thisObject = GetThisObjectFromFastNewFrame(currentSp);
                SET_ACC(thisObject);
                INTERPRETER_HANDLE_RETURN();
            }

            if (!acc.IsUndefined()) {
                {
                    [[maybe_unused]] EcmaHandleScope handleScope(thread);
                    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
                        "Derived constructor must return object or undefined");
                    thread->SetException(error.GetTaggedValue());
                }
                INTERPRETER_GOTO_EXCEPTION_HANDLER();
            }

            JSTaggedValue thisObject = GetThisObjectFromFastNewFrame(currentSp);
            SET_ACC(thisObject);
            INTERPRETER_HANDLE_RETURN();
        } else {
            SET_ACC(JSTaggedValue::Undefined());
        }
        INTERPRETER_HANDLE_RETURN();
    }
    HANDLE_OPCODE(LDNAN) {
        LOG_INST() << "intrinsics::ldnan";
        SET_ACC(JSTaggedValue(base::NAN_VALUE));
        DISPATCH(LDNAN);
    }
    HANDLE_OPCODE(LDINFINITY) {
        LOG_INST() << "intrinsics::ldinfinity";
        SET_ACC(JSTaggedValue(base::POSITIVE_INFINITY));
        DISPATCH(LDINFINITY);
    }
    HANDLE_OPCODE(LDUNDEFINED) {
        LOG_INST() << "intrinsics::ldundefined";
        SET_ACC(JSTaggedValue::Undefined());
        DISPATCH(LDUNDEFINED);
    }
    HANDLE_OPCODE(LDNULL) {
        LOG_INST() << "intrinsics::ldnull";
        SET_ACC(JSTaggedValue::Null());
        DISPATCH(LDNULL);
    }
    HANDLE_OPCODE(LDSYMBOL) {
        LOG_INST() << "intrinsics::ldsymbol";
        SET_ACC(globalEnv->GetSymbolFunction().GetTaggedValue());
        DISPATCH(LDSYMBOL);
    }
    HANDLE_OPCODE(LDGLOBAL) {
        LOG_INST() << "intrinsics::ldglobal";
        SET_ACC(globalObj);
        DISPATCH(LDGLOBAL);
    }
    HANDLE_OPCODE(LDTRUE) {
        LOG_INST() << "intrinsics::ldtrue";
        SET_ACC(JSTaggedValue::True());
        DISPATCH(LDTRUE);
    }
    HANDLE_OPCODE(LDFALSE) {
        LOG_INST() << "intrinsics::ldfalse";
        SET_ACC(JSTaggedValue::False());
        DISPATCH(LDFALSE);
    }
    HANDLE_OPCODE(GETUNMAPPEDARGS) {
        LOG_INST() << "intrinsics::getunmappedargs";

        uint32_t startIdx = 0;
        uint32_t actualNumArgs = GetNumArgs(sp, 0, startIdx);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetUnmapedArgs(thread, sp, actualNumArgs, startIdx);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETUNMAPPEDARGS);
    }
    HANDLE_OPCODE(ASYNCFUNCTIONENTER) {
        LOG_INST() << "intrinsics::asyncfunctionenter";
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionEnter(thread);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ASYNCFUNCTIONENTER);
    }
    HANDLE_OPCODE(TYPEOF_IMM8) {
        LOG_INST() << "intrinsics::typeof";
        JSTaggedValue res = FastRuntimeStub::FastTypeOf(thread, GET_ACC());
        SET_ACC(res);
        DISPATCH(TYPEOF_IMM8);
    }
    HANDLE_OPCODE(TYPEOF_IMM16) {
        LOG_INST() << "intrinsics::typeof";
        JSTaggedValue res = FastRuntimeStub::FastTypeOf(thread, GET_ACC());
        SET_ACC(res);
        DISPATCH(TYPEOF_IMM16);
    }
    HANDLE_OPCODE(GETPROPITERATOR) {
        LOG_INST() << "intrinsics::getpropiterator";
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetPropIterator(thread, GET_ACC());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETPROPITERATOR);
    }
    // TODO
    // HANDLE_OPCODE(ITERNEXT_IMM8_V8) {
    //     uint16_t v0 = READ_INST_8_1();
    //     LOG_INST() << "intrinsics::iternext"
    //                << " v" << v0;
    //     SAVE_PC();
    //     JSTaggedValue iter = GET_VREG_VALUE(v0);
    //     JSTaggedValue res = SlowRuntimeStub::IterNext(thread, iter);
    //     INTERPRETER_RETURN_IF_ABRUPT(res);
    //     SET_ACC(res);
    //     DISPATCH(ITERNEXT_IMM8_V8);
    // }
    HANDLE_OPCODE(CLOSEITERATOR_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::closeiterator"
                   << " v" << v0;
        SAVE_PC();
        JSTaggedValue iter = GET_VREG_VALUE(v0);
        JSTaggedValue res = SlowRuntimeStub::CloseIterator(thread, iter);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CLOSEITERATOR_IMM8_V8);
    }
    HANDLE_OPCODE(ADD2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::add2"
                   << " v" << v0;
        int32_t a0;
        int32_t a1;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // number, fast path
        if (left.IsInt() && right.IsInt()) {
            a0 = left.GetInt();
            a1 = right.GetInt();
            if ((a0 > 0 && a1 > INT32_MAX - a0) || (a0 < 0 && a1 < INT32_MIN - a0)) {
                auto ret = static_cast<double>(a0) + static_cast<double>(a1);
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 + a1));
            }
        } else if (left.IsNumber() && right.IsNumber()) {
            double a0Double = left.IsInt() ? left.GetInt() : left.GetDouble();
            double a1Double = right.IsInt() ? right.GetInt() : right.GetDouble();
            double ret = a0Double + a1Double;
            SET_ACC(JSTaggedValue(ret));
        } else {
            // one or both are not number, slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Add2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(ADD2_IMM8_V8);
    }
    HANDLE_OPCODE(SUB2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::sub2"
                   << " v" << v0;
        int32_t a0;
        int32_t a1;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        if (left.IsInt() && right.IsInt()) {
            a0 = left.GetInt();
            a1 = -right.GetInt();
            if ((a0 > 0 && a1 > INT32_MAX - a0) || (a0 < 0 && a1 < INT32_MIN - a0)) {
                auto ret = static_cast<double>(a0) + static_cast<double>(a1);
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 + a1));
            }
        } else if (left.IsNumber() && right.IsNumber()) {
            double a0Double = left.IsInt() ? left.GetInt() : left.GetDouble();
            double a1Double = right.IsInt() ? right.GetInt() : right.GetDouble();
            double ret = a0Double - a1Double;
            SET_ACC(JSTaggedValue(ret));
        } else {
            // one or both are not number, slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Sub2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(SUB2_IMM8_V8);
    }
    HANDLE_OPCODE(MUL2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::mul2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = acc;
        JSTaggedValue value = FastRuntimeStub::FastMul(left, right);
        if (!value.IsHole()) {
            SET_ACC(value);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Mul2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(MUL2_IMM8_V8);
    }
    HANDLE_OPCODE(DIV2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::div2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = acc;
        // fast path
        JSTaggedValue res = FastRuntimeStub::FastDiv(left, right);
        if (!res.IsHole()) {
            SET_ACC(res);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue slowRes = SlowRuntimeStub::Div2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(slowRes);
            SET_ACC(slowRes);
        }
        DISPATCH(DIV2_IMM8_V8);
    }
    HANDLE_OPCODE(MOD2_IMM8_V8) {
        uint16_t vs = READ_INST_8_1();
        LOG_INST() << "intrinsics::mod2"
                   << " v" << vs;
        JSTaggedValue left = GET_VREG_VALUE(vs);
        JSTaggedValue right = GET_ACC();
        JSTaggedValue res = FastRuntimeStub::FastMod(left, right);
        if (!res.IsHole()) {
            SET_ACC(res);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue slowRes = SlowRuntimeStub::Mod2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(slowRes);
            SET_ACC(slowRes);
        }
        DISPATCH(MOD2_IMM8_V8);
    }
    HANDLE_OPCODE(EQ_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::eq"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = acc;
        JSTaggedValue res = FastRuntimeStub::FastEqual(left, right);
        if (!res.IsHole()) {
            SET_ACC(res);
        } else {
            // slow path
            SAVE_PC();
            res = SlowRuntimeStub::Eq(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }

        DISPATCH(EQ_IMM8_V8);
    }
    HANDLE_OPCODE(NOTEQ_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::noteq"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = acc;

        JSTaggedValue res = FastRuntimeStub::FastEqual(left, right);
        if (!res.IsHole()) {
            res = res.IsTrue() ? JSTaggedValue::False() : JSTaggedValue::True();
            SET_ACC(res);
        } else {
            // slow path
            SAVE_PC();
            res = SlowRuntimeStub::NotEq(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(NOTEQ_IMM8_V8);
    }
    HANDLE_OPCODE(LESS_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::less"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        if (left.IsNumber() && right.IsNumber()) {
            // fast path
            double valueA = left.IsInt() ? static_cast<double>(left.GetInt()) : left.GetDouble();
            double valueB = right.IsInt() ? static_cast<double>(right.GetInt()) : right.GetDouble();
            bool ret = JSTaggedValue::StrictNumberCompare(valueA, valueB) == ComparisonResult::LESS;
            SET_ACC(ret ? JSTaggedValue::True() : JSTaggedValue::False());
        } else if (left.IsBigInt() && right.IsBigInt()) {
            bool result = BigInt::LessThan(left, right);
            SET_ACC(JSTaggedValue(result));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Less(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(LESS_IMM8_V8);
    }
    HANDLE_OPCODE(LESSEQ_IMM8_V8) {
        uint16_t vs = READ_INST_8_1();
        LOG_INST() << "intrinsics::lesseq "
                   << " v" << vs;
        JSTaggedValue left = GET_VREG_VALUE(vs);
        JSTaggedValue right = GET_ACC();
        if (left.IsNumber() && right.IsNumber()) {
            // fast path
            double valueA = left.IsInt() ? static_cast<double>(left.GetInt()) : left.GetDouble();
            double valueB = right.IsInt() ? static_cast<double>(right.GetInt()) : right.GetDouble();
            bool ret = JSTaggedValue::StrictNumberCompare(valueA, valueB) <= ComparisonResult::EQUAL;
            SET_ACC(ret ? JSTaggedValue::True() : JSTaggedValue::False());
        } else if (left.IsBigInt() && right.IsBigInt()) {
            bool result = BigInt::LessThan(left, right) || BigInt::Equal(left, right);
            SET_ACC(JSTaggedValue(result));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::LessEq(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(LESSEQ_IMM8_V8);
    }
    HANDLE_OPCODE(GREATER_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::greater"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = acc;
        if (left.IsNumber() && right.IsNumber()) {
            // fast path
            double valueA = left.IsInt() ? static_cast<double>(left.GetInt()) : left.GetDouble();
            double valueB = right.IsInt() ? static_cast<double>(right.GetInt()) : right.GetDouble();
            bool ret = JSTaggedValue::StrictNumberCompare(valueA, valueB) == ComparisonResult::GREAT;
            SET_ACC(ret ? JSTaggedValue::True() : JSTaggedValue::False());
        } else if (left.IsBigInt() && right.IsBigInt()) {
            bool result = BigInt::LessThan(right, left);
            SET_ACC(JSTaggedValue(result));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Greater(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(GREATER_IMM8_V8);
    }
    HANDLE_OPCODE(GREATEREQ_IMM8_V8) {
        uint16_t vs = READ_INST_8_1();
        LOG_INST() << "intrinsics::greateq "
                   << " v" << vs;
        JSTaggedValue left = GET_VREG_VALUE(vs);
        JSTaggedValue right = GET_ACC();
        if (left.IsNumber() && right.IsNumber()) {
            // fast path
            double valueA = left.IsInt() ? static_cast<double>(left.GetInt()) : left.GetDouble();
            double valueB = right.IsInt() ? static_cast<double>(right.GetInt()) : right.GetDouble();
            ComparisonResult comparison = JSTaggedValue::StrictNumberCompare(valueA, valueB);
            bool ret = (comparison == ComparisonResult::GREAT) || (comparison == ComparisonResult::EQUAL);
            SET_ACC(ret ? JSTaggedValue::True() : JSTaggedValue::False());
        } else if (left.IsBigInt() && right.IsBigInt()) {
            bool result = BigInt::LessThan(right, left) || BigInt::Equal(right, left);
            SET_ACC(JSTaggedValue(result));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::GreaterEq(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(GREATEREQ_IMM8_V8);
    }
    HANDLE_OPCODE(SHL2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::shl2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            using unsigned_type = std::make_unsigned_t<int32_t>;
            auto ret =
                static_cast<int32_t>(static_cast<unsigned_type>(opNumber0) << shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            using unsigned_type = std::make_unsigned_t<int32_t>;
            auto ret =
                static_cast<int32_t>(static_cast<unsigned_type>(opNumber0) << shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Shl2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(SHL2_IMM8_V8);
    }
    HANDLE_OPCODE(SHR2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::shr2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            using unsigned_type = std::make_unsigned_t<uint32_t>;
            auto ret =
                static_cast<uint32_t>(static_cast<unsigned_type>(opNumber0) >> shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            using unsigned_type = std::make_unsigned_t<uint32_t>;
            auto ret =
                static_cast<uint32_t>(static_cast<unsigned_type>(opNumber0) >> shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Shr2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(SHR2_IMM8_V8);
    }
    HANDLE_OPCODE(ASHR2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::ashr2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            auto ret = static_cast<int32_t>(opNumber0 >> shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            uint32_t shift =
                static_cast<uint32_t>(opNumber1) & 0x1f; // NOLINT(hicpp-signed-bitwise, readability-magic-numbers)
            auto ret = static_cast<int32_t>(opNumber0 >> shift); // NOLINT(hicpp-signed-bitwise)
            SET_ACC(JSTaggedValue(ret));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Ashr2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(ASHR2_IMM8_V8);
    }
    HANDLE_OPCODE(AND2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::and2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) & static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) & static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::And2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(AND2_IMM8_V8);
    }
    HANDLE_OPCODE(OR2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::or2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) | static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) | static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Or2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(OR2_IMM8_V8);
    }
    HANDLE_OPCODE(XOR2_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::xor2"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        // both number, fast path
        if (left.IsInt() && right.IsInt()) {
            int32_t opNumber0 = left.GetInt();
            int32_t opNumber1 = right.GetInt();
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) ^ static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else if (left.IsNumber() && right.IsNumber()) {
            int32_t opNumber0 =
                left.IsInt() ? left.GetInt() : base::NumberHelper::DoubleToInt(left.GetDouble(), base::INT32_BITS);
            int32_t opNumber1 =
                right.IsInt() ? right.GetInt() : base::NumberHelper::DoubleToInt(right.GetDouble(), base::INT32_BITS);
            // NOLINT(hicpp-signed-bitwise)
            auto ret = static_cast<uint32_t>(opNumber0) ^ static_cast<uint32_t>(opNumber1);
            SET_ACC(JSTaggedValue(static_cast<int32_t>(ret)));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Xor2(thread, left, right);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(XOR2_IMM8_V8);
    }
    HANDLE_OPCODE(EXP_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::exp"
                   << " v" << v0;
        JSTaggedValue base = GET_VREG_VALUE(v0);
        JSTaggedValue exponent = GET_ACC();
        if (base.IsNumber() && exponent.IsNumber()) {
            // fast path
            double doubleBase = base.IsInt() ? base.GetInt() : base.GetDouble();
            double doubleExponent = exponent.IsInt() ? exponent.GetInt() : exponent.GetDouble();
            if (std::abs(doubleBase) == 1 && std::isinf(doubleExponent)) {
                SET_ACC(JSTaggedValue(base::NAN_VALUE));
            }
            bool baseZero = doubleBase == 0 &&
                            (bit_cast<uint64_t>(doubleBase) & base::DOUBLE_SIGN_MASK) == base::DOUBLE_SIGN_MASK;
            bool isFinite = std::isfinite(doubleExponent);
            bool truncEqual = base::NumberHelper::TruncateDouble(doubleExponent) == doubleExponent;
            bool halfTruncEqual = (base::NumberHelper::TruncateDouble(doubleExponent / 2) + base::HALF) ==
                                  (doubleExponent / 2);
            if (baseZero && isFinite && truncEqual && halfTruncEqual) {
                if (doubleExponent > 0) {
                    SET_ACC(JSTaggedValue(-0.0));
                }
                if (doubleExponent < 0) {
                    SET_ACC(JSTaggedValue(-base::POSITIVE_INFINITY));
                }
            }
            SET_ACC(JSTaggedValue(std::pow(doubleBase, doubleExponent)));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Exp(thread, base, exponent);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(EXP_IMM8_V8);
    }
    HANDLE_OPCODE(ISIN_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::isin"
                   << " v" << v0;
        JSTaggedValue prop = GET_VREG_VALUE(v0);
        JSTaggedValue obj = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::IsIn(thread, prop, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ISIN_IMM8_V8);
    }
    HANDLE_OPCODE(INSTANCEOF_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::instanceof"
                   << " v" << v0;
        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue target = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::Instanceof(thread, obj, target);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(INSTANCEOF_IMM8_V8);
    }
    HANDLE_OPCODE(STRICTNOTEQ_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::strictnoteq"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        bool res = FastRuntimeStub::FastStrictEqual(left, right);
        SET_ACC(JSTaggedValue(!res));
        DISPATCH(STRICTNOTEQ_IMM8_V8);
    }
    HANDLE_OPCODE(STRICTEQ_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::stricteq"
                   << " v" << v0;
        JSTaggedValue left = GET_VREG_VALUE(v0);
        JSTaggedValue right = GET_ACC();
        bool res = FastRuntimeStub::FastStrictEqual(left, right);
        SET_ACC(JSTaggedValue(res));
        DISPATCH(STRICTEQ_IMM8_V8);
    }
    HANDLE_OPCODE(CREATEITERRESULTOBJ_V8_V8) {
        uint16_t v0 = READ_INST_8_0();
        uint16_t v1 = READ_INST_8_1();
        LOG_INST() << "intrinsics::createiterresultobj"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue value = GET_VREG_VALUE(v0);
        JSTaggedValue flag = GET_VREG_VALUE(v1);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateIterResultObj(thread, value, flag);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEITERRESULTOBJ_V8_V8);
    }
    HANDLE_OPCODE(NEWOBJAPPLY_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsic::newobjspeard"
                   << " v" << v0;
        JSTaggedValue func = GET_VREG_VALUE(v0);
        JSTaggedValue array = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewObjApply(thread, func, array);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(NEWOBJAPPLY_IMM8_V8);
    }
    HANDLE_OPCODE(NEWOBJAPPLY_IMM16_V8) {
        uint16_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsic::newobjspeard"
                   << " v" << v0;
        JSTaggedValue func = GET_VREG_VALUE(v0);
        JSTaggedValue array = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewObjApply(thread, func, array);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(NEWOBJAPPLY_IMM16_V8);
    }
    HANDLE_OPCODE(STOWNBYNAME_IMM8_ID16_V8) {
        uint16_t stringId = READ_INST_16_1();
        uint32_t v0 = READ_INST_8_3();
        LOG_INST() << "intrinsics::stownbyname "
                   << "v" << v0 << " stringId:" << stringId;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsJSObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            auto constpool = GetConstantPool(sp);
            JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
            RESTORE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            SAVE_ACC();
            receiver = GET_VREG_VALUE(v0);
            JSTaggedValue res = FastRuntimeStub::SetPropertyByName<true>(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOWNBYNAME_IMM8_ID16_V8);
            }
            RESTORE_ACC();
        }

        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        auto propKey = GET_STR_FROM_CACHE(stringId);  // Maybe moved by GC
        RESTORE_ACC();
        auto value = GET_ACC();                                  // Maybe moved by GC
        receiver = GET_VREG_VALUE(v0);                           // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StOwnByName(thread, receiver, propKey, value);
        RESTORE_ACC();
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(STOWNBYNAME_IMM8_ID16_V8);
    }
    HANDLE_OPCODE(STOWNBYNAME_IMM16_ID16_V8) {
        uint16_t stringId = READ_INST_16_2();
        uint32_t v0 = READ_INST_8_4();
        LOG_INST() << "intrinsics::stownbyname "
                   << "v" << v0 << " stringId:" << stringId;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsJSObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            auto constpool = GetConstantPool(sp);
            JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
            RESTORE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            SAVE_ACC();
            receiver = GET_VREG_VALUE(v0);
            JSTaggedValue res = FastRuntimeStub::SetPropertyByName<true>(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOWNBYNAME_IMM16_ID16_V8);
            }
            RESTORE_ACC();
        }

        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        auto propKey = GET_STR_FROM_CACHE(stringId);  // Maybe moved by GC
        RESTORE_ACC();
        auto value = GET_ACC();                                  // Maybe moved by GC
        receiver = GET_VREG_VALUE(v0);                           // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StOwnByName(thread, receiver, propKey, value);
        RESTORE_ACC();
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(STOWNBYNAME_IMM16_ID16_V8);
    }
    HANDLE_OPCODE(CREATEEMPTYARRAY_IMM8) {
        LOG_INST() << "intrinsics::createemptyarray";
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateEmptyArray(thread, factory, globalEnv);
        SET_ACC(res);
        DISPATCH(CREATEEMPTYARRAY_IMM8);
    }
    HANDLE_OPCODE(CREATEEMPTYARRAY_IMM16) {
        LOG_INST() << "intrinsics::createemptyarray";
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateEmptyArray(thread, factory, globalEnv);
        SET_ACC(res);
        DISPATCH(CREATEEMPTYARRAY_IMM16);
    }
    HANDLE_OPCODE(CREATEEMPTYOBJECT) {
        LOG_INST() << "intrinsics::createemptyobject";
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateEmptyObject(thread, factory, globalEnv);
        SET_ACC(res);
        DISPATCH(CREATEEMPTYOBJECT);
    }
    HANDLE_OPCODE(CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8) {
        uint16_t stringId = READ_INST_16_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue pattern = GET_STR_FROM_CACHE(stringId);
        uint8_t flags = READ_INST_8_3();
        LOG_INST() << "intrinsics::createregexpwithliteral "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(pattern.GetTaggedObject()))
                   << ", flags:" << flags;
        JSTaggedValue res = SlowRuntimeStub::CreateRegExpWithLiteral(thread, pattern, flags);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8);
    }
    HANDLE_OPCODE(CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8) {
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue pattern = GET_STR_FROM_CACHE(stringId);
        uint8_t flags = READ_INST_8_4();
        LOG_INST() << "intrinsics::createregexpwithliteral "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(pattern.GetTaggedObject()))
                   << ", flags:" << flags;
        JSTaggedValue res = SlowRuntimeStub::CreateRegExpWithLiteral(thread, pattern, flags);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8);
    }
    HANDLE_OPCODE(GETNEXTPROPNAME_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsic::getnextpropname"
                   << " v" << v0;
        JSTaggedValue iter = GET_VREG_VALUE(v0);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetNextPropName(thread, iter);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETNEXTPROPNAME_V8);
    }
    HANDLE_OPCODE(STOWNBYVALUE_IMM8_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::stownbyvalue"
                   << " v" << v0 << " v" << v1;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByValue<true>(thread, receiver, propKey, value);

            // SetPropertyByValue maybe gc need update the value
            RESTORE_ACC();
            propKey = GET_VREG_VALUE(v1);
            value = GET_ACC();
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOWNBYVALUE_IMM8_V8_V8);
            }
        }

        // slow path
        SAVE_ACC();
        receiver = GET_VREG_VALUE(v0);      // Maybe moved by GC
        auto propKey = GET_VREG_VALUE(v1);  // Maybe moved by GC
        auto value = GET_ACC();             // Maybe moved by GC
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StOwnByValue(thread, receiver, propKey, value);
        RESTORE_ACC();
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(STOWNBYVALUE_IMM8_V8_V8);
    }
    HANDLE_OPCODE(LDHOLE) {
        LOG_INST() << "intrinsic::ldhole";
        SET_ACC(JSTaggedValue::Hole());
        DISPATCH(LDHOLE);
    }
    HANDLE_OPCODE(DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8) {
        uint16_t v0 = READ_INST_8_0();
        uint16_t v1 = READ_INST_8_1();
        uint16_t v2 = READ_INST_8_2();
        uint16_t v3 = READ_INST_8_3();
        LOG_INST() << "intrinsics::definegettersetterbyvalue"
                   << " v" << v0 << " v" << v1 << " v" << v2 << " v" << v3;

        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue prop = GET_VREG_VALUE(v1);
        JSTaggedValue getter = GET_VREG_VALUE(v2);
        JSTaggedValue setter = GET_VREG_VALUE(v3);
        JSTaggedValue flag = GET_ACC();
        SAVE_PC();
        JSTaggedValue res =
            SlowRuntimeStub::DefineGetterSetterByValue(thread, obj, prop, getter, setter, flag.ToBoolean());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8);
    }
    HANDLE_OPCODE(STOBJBYVALUE_IMM8_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();

        LOG_INST() << "intrinsics::stobjbyvalue"
                   << " v" << v0 << " v" << v1;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            JSTaggedValue res = JSTaggedValue::Hole();
            SAVE_ACC();

            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryStoreICByValue(thread, receiver, propKey, firstValue, secondValue, value);
            }
            // IC miss and not enter the megamorphic state, store as polymorphic
            if (res.IsHole() && !firstValue.IsHole()) {
                res = ICRuntimeStub::StoreICByValue(thread,
                                                    profileTypeArray,
                                                    receiver, propKey, value, slotId);
            }

            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYVALUE_IMM8_V8_V8);
            }
        }
#endif
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByValue(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYVALUE_IMM8_V8_V8);
            }
            RESTORE_ACC();
        }
        {
            // slow path
            SAVE_ACC();
            SAVE_PC();
            receiver = GET_VREG_VALUE(v0);  // Maybe moved by GC
            JSTaggedValue propKey = GET_VREG_VALUE(v1);   // Maybe moved by GC
            JSTaggedValue value = GET_ACC();              // Maybe moved by GC
            JSTaggedValue res = SlowRuntimeStub::StObjByValue(thread, receiver, propKey, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
        }
        DISPATCH(STOBJBYVALUE_IMM8_V8_V8);
    }
    HANDLE_OPCODE(STOBJBYVALUE_IMM16_V8_V8) {
        uint32_t v0 = READ_INST_8_2();
        uint32_t v1 = READ_INST_8_3();

        LOG_INST() << "intrinsics::stobjbyvalue"
                   << " v" << v0 << " v" << v1;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            JSTaggedValue res = JSTaggedValue::Hole();
            SAVE_ACC();

            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryStoreICByValue(thread, receiver, propKey, firstValue, secondValue, value);
            }
            // IC miss and not enter the megamorphic state, store as polymorphic
            if (res.IsHole() && !firstValue.IsHole()) {
                res = ICRuntimeStub::StoreICByValue(thread,
                                                    profileTypeArray,
                                                    receiver, propKey, value, slotId);
            }

            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYVALUE_IMM16_V8_V8);
            }
        }
#endif
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByValue(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYVALUE_IMM16_V8_V8);
            }
            RESTORE_ACC();
        }
        {
            // slow path
            SAVE_ACC();
            SAVE_PC();
            receiver = GET_VREG_VALUE(v0);  // Maybe moved by GC
            JSTaggedValue propKey = GET_VREG_VALUE(v1);   // Maybe moved by GC
            JSTaggedValue value = GET_ACC();              // Maybe moved by GC
            JSTaggedValue res = SlowRuntimeStub::StObjByValue(thread, receiver, propKey, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
        }
        DISPATCH(STOBJBYVALUE_IMM16_V8_V8);
    }
    HANDLE_OPCODE(STSUPERBYVALUE_IMM8_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();

        LOG_INST() << "intrinsics::stsuperbyvalue"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_VREG_VALUE(v1);
        JSTaggedValue value = GET_ACC();

        // slow path
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::StSuperByValue(thread, receiver, propKey, value, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STSUPERBYVALUE_IMM8_V8_V8);
    }
    HANDLE_OPCODE(TRYLDGLOBALBYNAME_IMM8_ID16) {
        uint16_t stringId = READ_INST_16_1();
        auto constpool = GetConstantPool(sp);
        auto prop = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::tryldglobalbyname "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(prop.GetTaggedObject()));

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            JSTaggedValue res = ICRuntimeStub::LoadGlobalICByName(thread,
                                                                  ProfileTypeInfo::Cast(
                                                                  profileTypeInfo.GetTaggedObject()),
                                                                  globalObj, prop, slotId);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
            DISPATCH(TRYLDGLOBALBYNAME_IMM8_ID16);
        }
#endif

        // order: 1. global record 2. global object
        JSTaggedValue result = SlowRuntimeStub::LdGlobalRecord(thread, prop);
        if (!result.IsUndefined()) {
            SET_ACC(PropertyBox::Cast(result.GetTaggedObject())->GetValue());
        } else {
            JSTaggedValue globalResult = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, prop);
            if (!globalResult.IsHole()) {
                SET_ACC(globalResult);
            } else {
                // slow path
                SAVE_PC();
                JSTaggedValue res = SlowRuntimeStub::TryLdGlobalByNameFromGlobalProto(thread, globalObj, prop);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
            }
        }

        DISPATCH(TRYLDGLOBALBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(TRYLDGLOBALBYNAME_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        auto constpool = GetConstantPool(sp);
        auto prop = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::tryldglobalbyname "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(prop.GetTaggedObject()));

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            JSTaggedValue res = ICRuntimeStub::LoadGlobalICByName(thread,
                                                                  ProfileTypeInfo::Cast(
                                                                  profileTypeInfo.GetTaggedObject()),
                                                                  globalObj, prop, slotId);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
            DISPATCH(TRYLDGLOBALBYNAME_IMM16_ID16);
        }
#endif

        // order: 1. global record 2. global object
        JSTaggedValue result = SlowRuntimeStub::LdGlobalRecord(thread, prop);
        if (!result.IsUndefined()) {
            SET_ACC(PropertyBox::Cast(result.GetTaggedObject())->GetValue());
        } else {
            JSTaggedValue globalResult = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, prop);
            if (!globalResult.IsHole()) {
                SET_ACC(globalResult);
            } else {
                // slow path
                SAVE_PC();
                JSTaggedValue res = SlowRuntimeStub::TryLdGlobalByNameFromGlobalProto(thread, globalObj, prop);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
            }
        }

        DISPATCH(TRYLDGLOBALBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(TRYSTGLOBALBYNAME_IMM8_ID16) {
        uint16_t stringId = READ_INST_16_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::trystglobalbyname"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            JSTaggedValue value = GET_ACC();
            SAVE_ACC();
            JSTaggedValue res = ICRuntimeStub::StoreGlobalICByName(thread,
                                                                   ProfileTypeInfo::Cast(
                                                                       profileTypeInfo.GetTaggedObject()),
                                                                   globalObj, propKey, value, slotId);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
            DISPATCH(TRYSTGLOBALBYNAME_IMM8_ID16);
        }
#endif

        auto recordResult = SlowRuntimeStub::LdGlobalRecord(thread, propKey);
        SAVE_PC();
        // 1. find from global record
        if (!recordResult.IsUndefined()) {
            JSTaggedValue value = GET_ACC();
            SAVE_ACC();
            JSTaggedValue res = SlowRuntimeStub::TryUpdateGlobalRecord(thread, propKey, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
        } else {
            // 2. find from global object
            auto globalResult = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, propKey);
            if (globalResult.IsHole()) {
                auto result = SlowRuntimeStub::ThrowReferenceError(thread, propKey, " is not defined");
                INTERPRETER_RETURN_IF_ABRUPT(result);
            }
            JSTaggedValue value = GET_ACC();
            SAVE_ACC();
            JSTaggedValue res = SlowRuntimeStub::StGlobalVar(thread, propKey, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
        }
        DISPATCH(TRYSTGLOBALBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(STOWNBYVALUEWITHNAMESET_IMM8_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::stownbyvaluewithnameset"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            JSTaggedValue propKey = GET_VREG_VALUE(v1);
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByValue<true>(thread, receiver, propKey, value);

            // SetPropertyByValue maybe gc need update the value
            RESTORE_ACC();
            propKey = GET_VREG_VALUE(v1);
            value = GET_ACC();
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                JSFunction::SetFunctionNameNoPrefix(thread, JSFunction::Cast(value.GetTaggedObject()), propKey);
                RESTORE_ACC();
                DISPATCH(STOWNBYVALUEWITHNAMESET_IMM8_V8_V8);
            }
        }

        // slow path
        SAVE_ACC();
        SAVE_PC();
        receiver = GET_VREG_VALUE(v0);      // Maybe moved by GC
        auto propKey = GET_VREG_VALUE(v1);  // Maybe moved by GC
        auto value = GET_ACC();             // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StOwnByValueWithNameSet(thread, receiver, propKey, value);
        RESTORE_ACC();
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(STOWNBYVALUEWITHNAMESET_IMM8_V8_V8);
    }
    HANDLE_OPCODE(STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8) {
        uint16_t stringId = READ_INST_16_1();
        uint32_t v0 = READ_INST_8_3();
        LOG_INST() << "intrinsics::stownbynamewithnameset "
                   << "v" << v0 << " stringId:" << stringId;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsJSObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            auto constpool = GetConstantPool(sp);
            JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
            RESTORE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            SAVE_ACC();
            JSTaggedValue res = FastRuntimeStub::SetPropertyByName<true>(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                JSFunction::SetFunctionNameNoPrefix(thread, JSFunction::Cast(value.GetTaggedObject()), propKey);
                RESTORE_ACC();
                DISPATCH(STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8);
            }
            RESTORE_ACC();
        }

        SAVE_ACC();
        SAVE_PC();
        receiver = GET_VREG_VALUE(v0);                           // Maybe moved by GC
        auto constpool = GetConstantPool(sp);
        auto propKey = GET_STR_FROM_CACHE(stringId);  // Maybe moved by GC
        RESTORE_ACC();
        auto value = GET_ACC();                                  // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StOwnByNameWithNameSet(thread, receiver, propKey, value);
        RESTORE_ACC();
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8);
    }
    HANDLE_OPCODE(LDGLOBALVAR_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        LOG_INST() << "intrinsics::ldglobalvar stringId:" << stringId;
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            JSTaggedValue res = ICRuntimeStub::LoadGlobalICByName(thread,
                                                                  ProfileTypeInfo::Cast(
                                                                      profileTypeInfo.GetTaggedObject()),
                                                                  globalObj, propKey, slotId);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
            DISPATCH(LDGLOBALVAR_IMM16_ID16);
        }
#endif

        JSTaggedValue result = FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, propKey);
        if (!result.IsHole()) {
            SET_ACC(result);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::LdGlobalVarFromGlobalProto(thread, globalObj, propKey);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(LDGLOBALVAR_IMM16_ID16);
    }
    HANDLE_OPCODE(STOBJBYNAME_IMM8_ID16_V8) {
        uint32_t v0 = READ_INST_8_3();
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();
            SAVE_ACC();

            JSTaggedValue receiver = GET_VREG_VALUE(v0);
            JSTaggedValue value = GET_ACC();
            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryStoreICByName(thread, receiver, firstValue, secondValue, value);
            }
            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM8_ID16_V8);
            } else if (!firstValue.IsHole()) { // IC miss and not enter the megamorphic state, store as polymorphic
                uint16_t stringId = READ_INST_16_1();
                SAVE_ACC();
                auto constpool = GetConstantPool(sp);
                JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
                RESTORE_ACC();
                value = GET_ACC();
                receiver = GET_VREG_VALUE(v0);
                res = ICRuntimeStub::StoreICByName(thread,
                                                   profileTypeArray,
                                                   receiver, propKey, value, slotId);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM8_ID16_V8);
            }
        }
#endif
        uint16_t stringId = READ_INST_16_1();
        LOG_INST() << "intrinsics::stobjbyname "
                   << "v" << v0 << " stringId:" << stringId;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            auto constpool = GetConstantPool(sp);
            JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
            RESTORE_ACC();
            JSTaggedValue value = GET_ACC();
            receiver = GET_VREG_VALUE(v0);
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByName(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM8_ID16_V8);
            }
            RESTORE_ACC();
        }
        // slow path
        SAVE_ACC();
        SAVE_PC();
        auto constpool = GetConstantPool(sp);                    // Maybe moved by GC
        auto propKey = GET_STR_FROM_CACHE(stringId);  // Maybe moved by GC
        RESTORE_ACC();
        JSTaggedValue value = GET_ACC();                                  // Maybe moved by GC
        receiver = GET_VREG_VALUE(v0);
        JSTaggedValue res = SlowRuntimeStub::StObjByName(thread, receiver, propKey, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOBJBYNAME_IMM8_ID16_V8);
    }
    HANDLE_OPCODE(STOBJBYNAME_IMM16_ID16_V8) {
        uint16_t stringId = READ_INST_16_2();
        uint32_t v0 = READ_INST_8_4();
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();
            SAVE_ACC();

            JSTaggedValue receiver = GET_VREG_VALUE(v0);
            JSTaggedValue value = GET_ACC();
            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryStoreICByName(thread, receiver, firstValue, secondValue, value);
            }
            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM16_ID16_V8);
            } else if (!firstValue.IsHole()) { // IC miss and not enter the megamorphic state, store as polymorphic
                SAVE_ACC();
                auto constpool = GetConstantPool(sp);
                JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
                RESTORE_ACC();
                value = GET_ACC();
                receiver = GET_VREG_VALUE(v0);
                res = ICRuntimeStub::StoreICByName(thread,
                                                   profileTypeArray,
                                                   receiver, propKey, value, slotId);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM16_ID16_V8);
            }
        }
#endif
        LOG_INST() << "intrinsics::stobjbyname "
                   << "v" << v0 << " stringId:" << stringId;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            auto constpool = GetConstantPool(sp);
            JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
            RESTORE_ACC();
            JSTaggedValue value = GET_ACC();
            receiver = GET_VREG_VALUE(v0);
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByName(thread, receiver, propKey, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYNAME_IMM16_ID16_V8);
            }
            RESTORE_ACC();
        }
        // slow path
        SAVE_ACC();
        SAVE_PC();
        auto constpool = GetConstantPool(sp);                    // Maybe moved by GC
        auto propKey = GET_STR_FROM_CACHE(stringId);  // Maybe moved by GC
        RESTORE_ACC();
        JSTaggedValue value = GET_ACC();                                  // Maybe moved by GC
        receiver = GET_VREG_VALUE(v0);
        JSTaggedValue res = SlowRuntimeStub::StObjByName(thread, receiver, propKey, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOBJBYNAME_IMM16_ID16_V8);
    }
    HANDLE_OPCODE(STSUPERBYNAME_IMM8_ID16_V8) {
        uint16_t stringId = READ_INST_16_1();
        uint32_t v0 = READ_INST_8_3();

        JSTaggedValue obj = GET_VREG_VALUE(v0);
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        JSTaggedValue value = GET_ACC();

        LOG_INST() << "intrinsics::stsuperbyname"
                   << "v" << v0 << " stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << obj.GetRawData()
                   << ", value:" << value.GetRawData();

        // slow path
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::StSuperByValue(thread, obj, propKey, value, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STSUPERBYNAME_IMM8_ID16_V8);
    }
    HANDLE_OPCODE(STGLOBALVAR_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue prop = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        JSTaggedValue value = GET_ACC();

        LOG_INST() << "intrinsics::stglobalvar "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(prop.GetTaggedObject()))
                   << ", value:" << value.GetRawData();
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            SAVE_ACC();
            JSTaggedValue res = ICRuntimeStub::StoreGlobalICByName(thread,
                                                                   ProfileTypeInfo::Cast(
                                                                   profileTypeInfo.GetTaggedObject()),
                                                                   globalObj, prop, value, slotId);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            RESTORE_ACC();
            DISPATCH(STGLOBALVAR_IMM16_ID16);
        }
#endif
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalVar(thread, prop, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STGLOBALVAR_IMM16_ID16);
    }
    HANDLE_OPCODE(CREATEGENERATOROBJ_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::creategeneratorobj"
                   << " v" << v0;
        SAVE_PC();
        JSTaggedValue genFunc = GET_VREG_VALUE(v0);
        JSTaggedValue res = SlowRuntimeStub::CreateGeneratorObj(thread, genFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEGENERATOROBJ_V8);
    }
    HANDLE_OPCODE(STARRAYSPREAD_V8_V8) {
        uint16_t v0 = READ_INST_8_0();
        uint16_t v1 = READ_INST_8_1();
        LOG_INST() << "ecmascript::intrinsics::starrayspread"
                   << " v" << v0 << " v" << v1 << "acc";
        JSTaggedValue dst = GET_VREG_VALUE(v0);
        JSTaggedValue index = GET_VREG_VALUE(v1);
        JSTaggedValue src = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StArraySpread(thread, dst, index, src);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(STARRAYSPREAD_V8_V8);
    }
    HANDLE_OPCODE(LDFUNCTION) {
        LOG_INST() << "intrinsic::ldfunction";
        SET_ACC(GetThisFunction(sp));
        DISPATCH(LDFUNCTION);
    }
    HANDLE_OPCODE(LDBIGINT_ID16) {
        uint16_t stringId = READ_INST_16_0();
        LOG_INST() << "intrinsic::ldbigint";
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue numberBigInt = GET_STR_FROM_CACHE(stringId);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdBigInt(thread, numberBigInt);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDBIGINT_ID16);
    }
    HANDLE_OPCODE(CREATEASYNCGENERATOROBJ_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::createasyncgeneratorobj"
                   << " v" << v0;
        SAVE_PC();
        JSTaggedValue genFunc = GET_VREG_VALUE(v0);
        JSTaggedValue res = SlowRuntimeStub::CreateAsyncGeneratorObj(thread, genFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEASYNCGENERATOROBJ_V8);
    }
    HANDLE_OPCODE(ASYNCGENERATORRESOLVE_V8_V8_V8) {
        uint16_t v0 = READ_INST_8_0();
        uint16_t v1 = READ_INST_8_1();
        uint16_t v2 = READ_INST_8_2();
        LOG_INST() << "intrinsics::asyncgeneratorresolve"
                   << " v" << v0 << " v" << v1 << " v" << v2;
        JSTaggedValue asyncGenerator = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_VREG_VALUE(v1);
        JSTaggedValue flag = GET_VREG_VALUE(v2);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncGeneratorResolve(thread, asyncGenerator, value, flag);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ASYNCGENERATORRESOLVE_V8_V8_V8);
    }
    HANDLE_OPCODE(SUPERCALLTHISRANGE_IMM8_IMM8_V8) {
        uint16_t range = READ_INST_8_1();
        uint16_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::supercall"
                   << " range: " << range << " v" << v0;

        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue newTarget = GetNewTarget(sp);

        SAVE_PC();
        JSTaggedValue superCtor = SlowRuntimeStub::GetSuperConstructor(thread, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(superCtor);

        if (superCtor.IsJSFunction() && superCtor.IsConstructor() && !newTarget.IsUndefined()) {
            JSFunction *superCtorFunc = JSFunction::Cast(superCtor.GetTaggedObject());
            methodHandle.Update(superCtorFunc->GetMethod());
            if (superCtorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize =
                    InterpretedFrame::NumOfMembers() + range + NUM_MANDATORY_JSFUNC_ARGS + 2; // 2:thread & numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = range + NUM_MANDATORY_JSFUNC_ARGS;
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = superCtor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = newTarget.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 0; i < range; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = superCtor;
                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall ";
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);

                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime SuperCall ";
                SET_ACC(retValue);
                DISPATCH(SUPERCALLTHISRANGE_IMM8_IMM8_V8);
            }

            if (IsFastNewFrameEnter(superCtorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = superCtorFunc->IsBase() ?
                    methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                    methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 1;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (superCtorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, superCtor, newTarget, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(superCtorFunc->IsDerivedConstructor());
                    newSp[index++] = newTarget.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = superCtor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = superCtorFunc->GetProfileTypeInfo();
                    state->env = superCtorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 0; i < range && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall " << std::hex << reinterpret_cast<uintptr_t>(sp)
                                        << " " << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SuperCall(thread, thisFunc, newTarget, v0, range);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(SUPERCALLTHISRANGE_IMM8_IMM8_V8);
    }
    HANDLE_OPCODE(WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8) {
        uint16_t range = READ_INST_16_1();
        uint16_t v0 = READ_INST_8_3();
        LOG_INST() << "intrinsics::supercall"
                   << " range: " << range << " v" << v0;

        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue newTarget = GetNewTarget(sp);

        SAVE_PC();
        JSTaggedValue superCtor = SlowRuntimeStub::GetSuperConstructor(thread, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(superCtor);

        if (superCtor.IsJSFunction() && superCtor.IsConstructor() && !newTarget.IsUndefined()) {
            JSFunction *superCtorFunc = JSFunction::Cast(superCtor.GetTaggedObject());
            methodHandle.Update(superCtorFunc->GetMethod());
            if (superCtorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize =
                    InterpretedFrame::NumOfMembers() + range + NUM_MANDATORY_JSFUNC_ARGS + 2; // 2:thread & numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = range + NUM_MANDATORY_JSFUNC_ARGS;
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = superCtor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = newTarget.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 0; i < range; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = superCtor;
                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall ";
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);

                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime SuperCall ";
                SET_ACC(retValue);
                DISPATCH(WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8);
            }

            if (IsFastNewFrameEnter(superCtorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = superCtorFunc->IsBase() ?
                    methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                    methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 1;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (superCtorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, superCtor, newTarget, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(superCtorFunc->IsDerivedConstructor());
                    newSp[index++] = newTarget.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = superCtor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = superCtorFunc->GetProfileTypeInfo();
                    state->env = superCtorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 0; i < range && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall " << std::hex << reinterpret_cast<uintptr_t>(sp)
                                        << " " << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SuperCall(thread, thisFunc, newTarget, v0, range);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8);
    }
    HANDLE_OPCODE(SUPERCALLARROWRANGE_IMM8_IMM8_V8) {
        uint16_t range = READ_INST_8_1();
        uint16_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::supercall"
                   << " range: " << range << " v" << v0;

        JSTaggedValue thisFunc = GET_ACC();
        JSTaggedValue newTarget = GetNewTarget(sp);

        SAVE_PC();
        JSTaggedValue superCtor = SlowRuntimeStub::GetSuperConstructor(thread, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(superCtor);

        if (superCtor.IsJSFunction() && superCtor.IsConstructor() && !newTarget.IsUndefined()) {
            JSFunction *superCtorFunc = JSFunction::Cast(superCtor.GetTaggedObject());
            methodHandle.Update(superCtorFunc->GetMethod());
            if (superCtorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize =
                    InterpretedFrame::NumOfMembers() + range + NUM_MANDATORY_JSFUNC_ARGS + 2; // 2:thread & numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = range + NUM_MANDATORY_JSFUNC_ARGS;
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = superCtor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = newTarget.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 0; i < range; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = superCtor;
                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall ";
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);

                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime SuperCall ";
                SET_ACC(retValue);
                DISPATCH(SUPERCALLARROWRANGE_IMM8_IMM8_V8);
            }

            if (IsFastNewFrameEnter(superCtorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = superCtorFunc->IsBase() ?
                    methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                    methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 1;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (superCtorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, superCtor, newTarget, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(superCtorFunc->IsDerivedConstructor());
                    newSp[index++] = newTarget.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = superCtor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = superCtorFunc->GetProfileTypeInfo();
                    state->env = superCtorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 0; i < range && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall " << std::hex << reinterpret_cast<uintptr_t>(sp)
                                        << " " << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SuperCall(thread, thisFunc, newTarget, v0, range);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(SUPERCALLARROWRANGE_IMM8_IMM8_V8);
    }
    HANDLE_OPCODE(WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8) {
        uint16_t range = READ_INST_16_1();
        uint16_t v0 = READ_INST_8_3();
        LOG_INST() << "intrinsics::supercall"
                   << " range: " << range << " v" << v0;

        JSTaggedValue thisFunc = GET_ACC();
        JSTaggedValue newTarget = GetNewTarget(sp);

        SAVE_PC();
        JSTaggedValue superCtor = SlowRuntimeStub::GetSuperConstructor(thread, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(superCtor);

        if (superCtor.IsJSFunction() && superCtor.IsConstructor() && !newTarget.IsUndefined()) {
            JSFunction *superCtorFunc = JSFunction::Cast(superCtor.GetTaggedObject());
            methodHandle.Update(superCtorFunc->GetMethod());
            if (superCtorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize =
                    InterpretedFrame::NumOfMembers() + range + NUM_MANDATORY_JSFUNC_ARGS + 2; // 2:thread & numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = range + NUM_MANDATORY_JSFUNC_ARGS;
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = superCtor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = newTarget.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 0; i < range; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = superCtor;
                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall ";
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);

                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime SuperCall ";
                SET_ACC(retValue);
                DISPATCH(WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8);
            }

            if (IsFastNewFrameEnter(superCtorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = superCtorFunc->IsBase() ?
                    methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                    methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 1;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (superCtorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, superCtor, newTarget, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(superCtorFunc->IsDerivedConstructor());
                    newSp[index++] = newTarget.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = superCtor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = superCtorFunc->GetProfileTypeInfo();
                    state->env = superCtorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 0; i < range && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(v0 + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime SuperCall " << std::hex << reinterpret_cast<uintptr_t>(sp)
                                        << " " << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SuperCall(thread, thisFunc, newTarget, v0, range);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8);
    }
    HANDLE_OPCODE(SUPERCALLSPREAD_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsic::supercallspread"
                   << " array: v" << v0;

        JSTaggedValue thisFunc = GET_ACC();
        JSTaggedValue newTarget = GetNewTarget(sp);
        JSTaggedValue array = GET_VREG_VALUE(v0);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SuperCallSpread(thread, thisFunc, newTarget, array);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(SUPERCALLSPREAD_IMM8_V8);
    }
    HANDLE_OPCODE(DEPRECATED_CREATEOBJECTHAVINGMETHOD_PREF_IMM16) {
        uint16_t imm = READ_INST_16_1();
        LOG_INST() << "intrinsics::createobjecthavingmethod"
                   << " imm:" << imm;
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSObject *result = JSObject::Cast(GET_METHOD_FROM_CACHE(imm).GetTaggedObject());
        RESTORE_ACC();
        JSTaggedValue env = GET_ACC();

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectHavingMethod(thread, factory, result, env);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_CREATEOBJECTHAVINGMETHOD_PREF_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_LDHOMEOBJECT_PREF_NONE) {
        LOG_INST() << "intrinsics::ldhomeobject";

        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue homeObject = JSFunction::Cast(thisFunc.GetTaggedObject())->GetHomeObject();

        SET_ACC(homeObject);
        DISPATCH(DEPRECATED_LDHOMEOBJECT_PREF_NONE);
    }
    HANDLE_OPCODE(DEBUGGER) {
        LOG_INST() << "intrinsics::debugger";
        DISPATCH(DEBUGGER);
    }
    HANDLE_OPCODE(ISTRUE) {
        LOG_INST() << "intrinsics::istrue";
        if (GET_ACC().ToBoolean()) {
            SET_ACC(JSTaggedValue::True());
        } else {
            SET_ACC(JSTaggedValue::False());
        }
        DISPATCH(ISTRUE);
    }
    HANDLE_OPCODE(ISFALSE) {
        LOG_INST() << "intrinsics::isfalse";
        if (!GET_ACC().ToBoolean()) {
            SET_ACC(JSTaggedValue::True());
        } else {
            SET_ACC(JSTaggedValue::False());
        }
        DISPATCH(ISFALSE);
    }
    HANDLE_OPCODE(EXCEPTION) {
        FrameHandler frameHandler(thread);
        uint32_t pcOffset = panda_file::INVALID_OFFSET;
        for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
            if (frameHandler.IsEntryFrame()) {
                return;
            }
            auto method = frameHandler.GetMethod();
            pcOffset = FindCatchBlock(method, frameHandler.GetBytecodeOffset());
            if (pcOffset != panda_file::INVALID_OFFSET) {
                sp = frameHandler.GetSp();
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                pc = method->GetBytecodeArray() + pcOffset;
                break;
            }
        }
        if (pcOffset == panda_file::INVALID_OFFSET) {
            return;
        }

        auto exception = thread->GetException();
        SET_ACC(exception);
        thread->ClearException();
        thread->SetCurrentSPFrame(sp);
        DISPATCH_OFFSET(0);
    }
    HANDLE_OPCODE(OVERFLOW) {
        LOG_INTERPRETER(FATAL) << "opcode overflow";
    }
    HANDLE_OPCODE(NOP) {
        LOG_INST() << "intrinsics::nop";
        DISPATCH(NOP);
    }
    FIRST_HANDLE_OPCODE(THROW) {
        DISPATCH_THROW();
    }
    FIRST_HANDLE_OPCODE(WIDE) {
        DISPATCH_WIDE();
    }
    FIRST_HANDLE_OPCODE(DEPRECATED) {
        DISPATCH_DEPRECATED();
    }
    HANDLE_OPCODE(THROW_PREF_NONE) {
        LOG_INST() << "intrinsics::throw";
        SAVE_PC();
        SlowRuntimeStub::Throw(thread, GET_ACC());
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_CONSTASSIGNMENT_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "throwconstassignment"
                   << " v" << v0;
        SAVE_PC();
        SlowRuntimeStub::ThrowConstAssignment(thread, GET_VREG_VALUE(v0));
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_NOTEXISTS_PREF_NONE) {
        LOG_INST() << "throwthrownotexists";

        SAVE_PC();
        SlowRuntimeStub::ThrowThrowNotExists(thread);
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_PATTERNNONCOERCIBLE_PREF_NONE) {
        LOG_INST() << "throwpatternnoncoercible";

        SAVE_PC();
        SlowRuntimeStub::ThrowPatternNonCoercible(thread);
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_IFNOTOBJECT_PREF_V8) {
        LOG_INST() << "throwifnotobject";
        uint16_t v0 = READ_INST_8_1();

        JSTaggedValue value = GET_VREG_VALUE(v0);
        // fast path
        if (value.IsECMAObject()) {
            DISPATCH(THROW_IFNOTOBJECT_PREF_V8);
        }

        // slow path
        SAVE_PC();
        SlowRuntimeStub::ThrowIfNotObject(thread);
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_UNDEFINEDIFHOLE_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsic::throwundefinedifhole"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue hole = GET_VREG_VALUE(v0);
        if (!hole.IsHole()) {
            DISPATCH(THROW_UNDEFINEDIFHOLE_PREF_V8_V8);
        }
        JSTaggedValue obj = GET_VREG_VALUE(v1);
        ASSERT(obj.IsString());
        SAVE_PC();
        SlowRuntimeStub::ThrowUndefinedIfHole(thread, obj);
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_DELETESUPERPROPERTY_PREF_NONE) {
        LOG_INST() << "throwdeletesuperproperty";

        SAVE_PC();
        SlowRuntimeStub::ThrowDeleteSuperProperty(thread);
        INTERPRETER_GOTO_EXCEPTION_HANDLER();
    }
    HANDLE_OPCODE(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8) {
        uint8_t imm = READ_INST_8_1();
        JSTaggedValue thisValue = GET_ACC();
        LOG_INST() << "intrinsic::throwifsupernotcorrectcall"
                   << " imm:" << imm;
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::ThrowIfSuperNotCorrectCall(thread, imm, thisValue);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8);
    }
    HANDLE_OPCODE(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16) {
        uint16_t imm = READ_INST_16_1();
        JSTaggedValue thisValue = GET_ACC();
        LOG_INST() << "intrinsic::throwifsupernotcorrectcall"
                   << " imm:" << imm;
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::ThrowIfSuperNotCorrectCall(thread, imm, thisValue);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        DISPATCH(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16);
    }
    HANDLE_OPCODE(CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8) {
        uint8_t numKeys = READ_INST_8_0();
        uint16_t v0 = READ_INST_8_1();
        uint16_t firstArgRegIdx = READ_INST_8_2();
        LOG_INST() << "intrinsics::createobjectwithexcludedkeys " << numKeys << " v" << firstArgRegIdx;

        JSTaggedValue obj = GET_VREG_VALUE(v0);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectWithExcludedKeys(thread, numKeys, obj, firstArgRegIdx);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8);
    }
    HANDLE_OPCODE(WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8) {
        uint16_t numKeys = READ_INST_16_1();
        uint16_t v0 = READ_INST_8_3();
        uint16_t firstArgRegIdx = READ_INST_8_4();
        LOG_INST() << "intrinsics::createobjectwithexcludedkeys " << numKeys << " v" << firstArgRegIdx;

        JSTaggedValue obj = GET_VREG_VALUE(v0);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectWithExcludedKeys(thread, numKeys, obj, firstArgRegIdx);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8);
    }
    HANDLE_OPCODE(NEWOBJRANGE_IMM8_IMM8_V8) {
        uint16_t numArgs = READ_INST_8_1();
        uint16_t firstArgRegIdx = READ_INST_8_2();
        LOG_INST() << "intrinsics::newobjRange " << numArgs << " v" << firstArgRegIdx;
        JSTaggedValue ctor = GET_VREG_VALUE(firstArgRegIdx);

        if (ctor.IsJSFunction() && ctor.IsConstructor()) {
            JSFunction *ctorFunc = JSFunction::Cast(ctor.GetTaggedObject());
            methodHandle.Update(ctorFunc->GetMethod());
            if (ctorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize = InterpretedFrame::NumOfMembers() + numArgs + 4;  // 2: numArgs & thread
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo*>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = numArgs + 2; // 2: for newtarget / this
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = ctor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = ctor.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 1; i < numArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = ctor;
                thread->SetCurrentSPFrame(newSp);

                LOG_INST() << "Entry: Runtime New.";
                SAVE_PC();
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);
                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime New.";
                SET_ACC(retValue);
                DISPATCH(NEWOBJRANGE_IMM8_IMM8_V8);
            }

            if (IsFastNewFrameEnter(ctorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = ctorFunc->IsBase() ?
                                           methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                                           methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 2;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (ctorFunc->IsBase()) {
                    JSTaggedValue newTarget = GET_ACC();
                    thisObj = FastRuntimeStub::NewThisObject(thread, ctor, newTarget, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(ctorFunc->IsDerivedConstructor());
                    JSTaggedValue newTarget = GET_ACC();
                    newSp[index++] = newTarget.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = ctor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = ctorFunc->GetProfileTypeInfo();
                    state->env = ctorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 1; i < numArgs && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime New " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                        << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        // bound function, proxy, other call types, enter slow path
        constexpr uint16_t firstArgOffset = 1;
        // Exclude func and newTarget
        uint16_t firstArgIdx = firstArgRegIdx + firstArgOffset;
        uint16_t length = numArgs - firstArgOffset;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewObjRange(thread, ctor, ctor, firstArgIdx, length);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(NEWOBJRANGE_IMM8_IMM8_V8);
    }
    HANDLE_OPCODE(NEWOBJRANGE_IMM16_IMM8_V8) {
        uint16_t numArgs = READ_INST_8_2();
        uint16_t firstArgRegIdx = READ_INST_8_3();
        LOG_INST() << "intrinsics::newobjRange " << numArgs << " v" << firstArgRegIdx;
        JSTaggedValue ctor = GET_VREG_VALUE(firstArgRegIdx);

        if (ctor.IsJSFunction() && ctor.IsConstructor()) {
            JSFunction *ctorFunc = JSFunction::Cast(ctor.GetTaggedObject());
            methodHandle.Update(ctorFunc->GetMethod());
            if (ctorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize = InterpretedFrame::NumOfMembers() + numArgs + 4;  // 4: newtarget/this & numArgs & thread
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo*>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = numArgs + 2; // +1 for newtarget/this
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = GET_VREG(firstArgRegIdx);
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = GET_VREG(firstArgRegIdx + 1);
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 1; i < numArgs; ++i) {  // 2: func and newTarget
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = ctor;
                thread->SetCurrentSPFrame(newSp);

                LOG_INST() << "Entry: Runtime New.";
                SAVE_PC();
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);
                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime New.";
                SET_ACC(retValue);
                DISPATCH(NEWOBJRANGE_IMM16_IMM8_V8);
            }

            if (IsFastNewFrameEnter(ctorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = ctorFunc->IsBase() ?
                                           methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                                           methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 2;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (ctorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, ctor, ctor, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(ctorFunc->IsDerivedConstructor());
                    newSp[index++] = ctor.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = ctor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = ctorFunc->GetProfileTypeInfo();
                    state->env = ctorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 1; i < numArgs && index < numVregs + numDeclaredArgs; ++i) {  // 2: func and newTarget
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime New " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                        << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        // bound function, proxy, other call types, enter slow path
        constexpr uint16_t firstArgOffset = 1;
        // Exclude func and newTarget
        uint16_t firstArgIdx = firstArgRegIdx + firstArgOffset;
        uint16_t length = numArgs - firstArgOffset;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewObjRange(thread, ctor, ctor, firstArgIdx, length);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(NEWOBJRANGE_IMM16_IMM8_V8);
    }
    HANDLE_OPCODE(WIDE_NEWOBJRANGE_PREF_IMM16_V8) {
        uint16_t numArgs = READ_INST_16_1();
        uint16_t firstArgRegIdx = READ_INST_8_3();
        LOG_INST() << "intrinsics::newobjRange " << numArgs << " v" << firstArgRegIdx;
        JSTaggedValue ctor = GET_VREG_VALUE(firstArgRegIdx);

        if (ctor.IsJSFunction() && ctor.IsConstructor()) {
            JSFunction *ctorFunc = JSFunction::Cast(ctor.GetTaggedObject());
            methodHandle.Update(ctorFunc->GetMethod());
            if (ctorFunc->IsBuiltinConstructor()) {
                ASSERT(methodHandle->GetNumVregsWithCallField() == 0);
                size_t frameSize = InterpretedFrame::NumOfMembers() + numArgs + 4;  // 3: this & numArgs & thread
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                // copy args
                uint32_t index = 0;
                // numArgs
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo*>(newSp);
                newSp[index++] = ToUintPtr(thread);
                newSp[index++] = numArgs + 2; // +1 for this
                // func
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = ctor.GetRawData();
                // newTarget
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = ctor.GetRawData();
                // this
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                for (size_t i = 1; i < numArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                InterpretedBuiltinFrame *state = GET_BUILTIN_FRAME(newSp);
                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_BUILTIN_FRAME;
                state->pc = nullptr;
                state->function = ctor;
                thread->SetCurrentSPFrame(newSp);

                LOG_INST() << "Entry: Runtime New.";
                SAVE_PC();
                JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
                    const_cast<void *>(methodHandle->GetNativePointer()))(ecmaRuntimeCallInfo);
                thread->SetCurrentSPFrame(sp);
                if (UNLIKELY(thread->HasPendingException())) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }
                LOG_INST() << "Exit: Runtime New.";
                SET_ACC(retValue);
                DISPATCH(WIDE_NEWOBJRANGE_PREF_IMM16_V8);
            }

            if (IsFastNewFrameEnter(ctorFunc, methodHandle)) {
                SAVE_PC();
                uint32_t numVregs = methodHandle->GetNumVregsWithCallField();
                uint32_t numDeclaredArgs = ctorFunc->IsBase() ?
                                           methodHandle->GetNumArgsWithCallField() + 1 :  // +1 for this
                                           methodHandle->GetNumArgsWithCallField() + 2;   // +2 for newTarget and this
                // +1 for hidden this, explicit this may be overwritten after bc optimizer
                size_t frameSize = InterpretedFrame::NumOfMembers() + numVregs + numDeclaredArgs + 2;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                JSTaggedType *newSp = sp - frameSize;
                InterpretedFrame *state = GET_FRAME(newSp);

                if (UNLIKELY(thread->DoStackOverflowCheck(newSp))) {
                    INTERPRETER_GOTO_EXCEPTION_HANDLER();
                }

                uint32_t index = 0;
                // initialize vregs value
                for (size_t i = 0; i < numVregs; ++i) {
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // this
                JSTaggedValue thisObj;
                if (ctorFunc->IsBase()) {
                    thisObj = FastRuntimeStub::NewThisObject(thread, ctor, ctor, state);
                    INTERPRETER_RETURN_IF_ABRUPT(thisObj);
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();
                } else {
                    ASSERT(ctorFunc->IsDerivedConstructor());
                    newSp[index++] = ctor.GetRawData();
                    thisObj = JSTaggedValue::Undefined();
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = thisObj.GetRawData();

                    state->function = ctor;
                    state->constpool = methodHandle->GetConstantPool();
                    state->profileTypeInfo = ctorFunc->GetProfileTypeInfo();
                    state->env = ctorFunc->GetLexicalEnv();
                }

                // the second condition ensure not push extra args
                for (size_t i = 1; i < numArgs && index < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = GET_VREG(firstArgRegIdx + i);
                }

                // set undefined to the extra prats of declare
                for (size_t i = index; i < numVregs + numDeclaredArgs; ++i) {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    newSp[index++] = JSTaggedValue::VALUE_UNDEFINED;
                }

                // hidden this object
                newSp[index] = thisObj.GetRawData();

                state->base.prev = sp;
                state->base.type = FrameType::INTERPRETER_FAST_NEW_FRAME;
                state->pc = pc = methodHandle->GetBytecodeArray();
                sp = newSp;
                state->acc = JSTaggedValue::Hole();

                thread->SetCurrentSPFrame(newSp);
                LOG_INST() << "Entry: Runtime New " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                        << std::hex << reinterpret_cast<uintptr_t>(pc);
                DISPATCH_OFFSET(0);
            }
        }

        // bound function, proxy, other call types, enter slow path
        constexpr uint16_t firstArgOffset = 1;
        // Exclude func and newTarget
        uint16_t firstArgIdx = firstArgRegIdx + firstArgOffset;
        uint16_t length = numArgs - firstArgOffset;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewObjRange(thread, ctor, ctor, firstArgIdx, length);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_NEWOBJRANGE_PREF_IMM16_V8);
    }
    HANDLE_OPCODE(NEWLEXENV_IMM8) {
        uint8_t numVars = READ_INST_8_0();
        LOG_INST() << "intrinsics::newlexenv"
                   << " imm " << numVars;

        JSTaggedValue res = FastRuntimeStub::NewLexicalEnv(thread, factory, numVars);
        if (res.IsHole()) {
            SAVE_PC();
            res = SlowRuntimeStub::NewLexicalEnv(thread, numVars);
            INTERPRETER_RETURN_IF_ABRUPT(res);
        }
        SET_ACC(res);
        GET_FRAME(sp)->env = res;
        DISPATCH(NEWLEXENV_IMM8);
    }
    HANDLE_OPCODE(WIDE_NEWLEXENV_PREF_IMM16) {
        uint16_t numVars = READ_INST_16_1();
        LOG_INST() << "intrinsics::newlexenv"
                   << " imm " << numVars;

        JSTaggedValue res = FastRuntimeStub::NewLexicalEnv(thread, factory, numVars);
        if (res.IsHole()) {
            SAVE_PC();
            res = SlowRuntimeStub::NewLexicalEnv(thread, numVars);
            INTERPRETER_RETURN_IF_ABRUPT(res);
        }
        SET_ACC(res);
        GET_FRAME(sp)->env = res;
        DISPATCH(WIDE_NEWLEXENV_PREF_IMM16);
    }
    HANDLE_OPCODE(NEWLEXENVWITHNAME_IMM8_ID16) {
        uint16_t numVars = READ_INST_8_0();
        uint16_t scopeId = READ_INST_16_1();
        LOG_INST() << "intrinsics::newlexenvwithname"
                   << " numVars " << numVars << " scopeId " << scopeId;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewLexicalEnvWithName(thread, numVars, scopeId);
        INTERPRETER_RETURN_IF_ABRUPT(res);

        SET_ACC(res);
        GET_FRAME(sp)->env = res;
        DISPATCH(NEWLEXENVWITHNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16) {
        uint16_t numVars = READ_INST_16_1();
        uint16_t scopeId = READ_INST_16_3();
        LOG_INST() << "intrinsics::newlexenvwithname"
                   << " numVars " << numVars << " scopeId " << scopeId;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::NewLexicalEnvWithName(thread, numVars, scopeId);
        INTERPRETER_RETURN_IF_ABRUPT(res);

        SET_ACC(res);
        GET_FRAME(sp)->env = res;
        DISPATCH(WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_LDLEXENV_PREF_NONE) {
        LOG_INST() << "intrinsics::ldlexenv ";
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        SET_ACC(currentLexenv);
        DISPATCH(DEPRECATED_LDLEXENV_PREF_NONE);
    }
    HANDLE_OPCODE(POPLEXENV) {
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        JSTaggedValue parentLexenv = LexicalEnv::Cast(currentLexenv.GetTaggedObject())->GetParentEnv();
        GET_FRAME(sp)->env = parentLexenv;
        DISPATCH(POPLEXENV);
    }
    HANDLE_OPCODE(DEPRECATED_POPLEXENV_PREF_NONE) {
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        JSTaggedValue parentLexenv = LexicalEnv::Cast(currentLexenv.GetTaggedObject())->GetParentEnv();
        GET_FRAME(sp)->env = parentLexenv;
        DISPATCH(DEPRECATED_POPLEXENV_PREF_NONE);
    }
    HANDLE_OPCODE(GETITERATOR_IMM8) {
        LOG_INST() << "intrinsics::getiterator";
        JSTaggedValue obj = GET_ACC();
        // slow path
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetIterator(thread, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETITERATOR_IMM8);
    }
    HANDLE_OPCODE(GETITERATOR_IMM16) {
        LOG_INST() << "intrinsics::getiterator";
        JSTaggedValue obj = GET_ACC();
        // slow path
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetIterator(thread, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETITERATOR_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_GETITERATORNEXT_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsic::getiteratornext"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue method = GET_VREG_VALUE(v1);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetIteratorNext(thread, obj, method);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_GETITERATORNEXT_PREF_V8_V8);
    }
    HANDLE_OPCODE(CREATEARRAYWITHBUFFER_IMM8_ID16) {
        uint16_t imm = READ_INST_16_1();
        LOG_INST() << "intrinsics::createarraywithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSArray *result = JSArray::Cast(GET_LITERA_FROM_CACHE(imm, ConstPoolType::ARRAY_LITERAL).GetTaggedObject());
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateArrayWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEARRAYWITHBUFFER_IMM8_ID16);
    }
    HANDLE_OPCODE(CREATEARRAYWITHBUFFER_IMM16_ID16) {
        uint16_t imm = READ_INST_16_2();
        LOG_INST() << "intrinsics::createarraywithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSArray *result = JSArray::Cast(GET_LITERA_FROM_CACHE(imm, ConstPoolType::ARRAY_LITERAL).GetTaggedObject());
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateArrayWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEARRAYWITHBUFFER_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_CREATEARRAYWITHBUFFER_PREF_IMM16) {
        uint16_t imm = READ_INST_16_1();
        LOG_INST() << "intrinsics::createarraywithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSArray *result = JSArray::Cast(GET_METHOD_FROM_CACHE(imm).GetTaggedObject());
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateArrayWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_CREATEARRAYWITHBUFFER_PREF_IMM16);
    }
    HANDLE_OPCODE(CREATEOBJECTWITHBUFFER_IMM8_ID16) {
        uint16_t imm = READ_INST_16_1();
        LOG_INST() << "intrinsics::createobjectwithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSObject *result = JSObject::Cast(GET_LITERA_FROM_CACHE(imm, ConstPoolType::OBJECT_LITERAL).GetTaggedObject());

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEOBJECTWITHBUFFER_IMM8_ID16);
    }
    HANDLE_OPCODE(CREATEOBJECTWITHBUFFER_IMM16_ID16) {
        uint16_t imm = READ_INST_16_2();
        LOG_INST() << "intrinsics::createobjectwithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSObject *result = JSObject::Cast(GET_LITERA_FROM_CACHE(imm, ConstPoolType::OBJECT_LITERAL).GetTaggedObject());

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(CREATEOBJECTWITHBUFFER_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_CREATEOBJECTWITHBUFFER_PREF_IMM16) {
        uint16_t imm = READ_INST_16_1();
        LOG_INST() << "intrinsics::createobjectwithbuffer"
                   << " imm:" << imm;
        auto constpool = GetConstantPool(sp);
        JSObject *result = JSObject::Cast(GET_METHOD_FROM_CACHE(imm).GetTaggedObject());

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CreateObjectWithBuffer(thread, factory, result);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_CREATEOBJECTWITHBUFFER_PREF_IMM16);
    }
    HANDLE_OPCODE(TONUMBER_IMM8) {
        LOG_INST() << "intrinsics::tonumber";
        JSTaggedValue value = GET_ACC();
        if (value.IsNumber()) {
            // fast path
            SET_ACC(value);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::ToNumber(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(TONUMBER_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_TONUMBER_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::tonumber"
                   << " v" << v0;
        JSTaggedValue value = GET_VREG_VALUE(v0);
        if (value.IsNumber()) {
            // fast path
            SET_ACC(value);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::ToNumber(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_TONUMBER_PREF_V8);
    }
    HANDLE_OPCODE(TONUMERIC_IMM8) {
        LOG_INST() << "intrinsics::tonumeric";
        JSTaggedValue value = GET_ACC();
        if (value.IsNumber() || value.IsBigInt()) {
            // fast path
            SET_ACC(value);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::ToNumeric(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(TONUMERIC_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_TONUMERIC_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::tonumeric"
                   << " v" << v0;
        JSTaggedValue value = GET_VREG_VALUE(v0);
        if (value.IsNumber() || value.IsBigInt()) {
            // fast path
            SET_ACC(value);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::ToNumeric(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_TONUMERIC_PREF_V8);
    }
    HANDLE_OPCODE(NEG_IMM8) {
        LOG_INST() << "intrinsics::neg";
        JSTaggedValue value = GET_ACC();
        // fast path
        if (value.IsInt()) {
            if (value.GetInt() == 0) {
                SET_ACC(JSTaggedValue(-0.0));
            } else {
                SET_ACC(JSTaggedValue(-value.GetInt()));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(-value.GetDouble()));
        } else {  // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Neg(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(NEG_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_NEG_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::neg"
                   << " v" << v0;
        JSTaggedValue value = GET_VREG_VALUE(v0);
        // fast path
        if (value.IsInt()) {
            if (value.GetInt() == 0) {
                SET_ACC(JSTaggedValue(-0.0));
            } else {
                SET_ACC(JSTaggedValue(-value.GetInt()));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(-value.GetDouble()));
        } else {  // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Neg(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_NEG_PREF_V8);
    }
    HANDLE_OPCODE(NOT_IMM8) {
        LOG_INST() << "intrinsics::not";
        JSTaggedValue value = GET_ACC();
        int32_t number;
        // number, fast path
        if (value.IsInt()) {
            number = static_cast<int32_t>(value.GetInt());
            SET_ACC(JSTaggedValue(~number));  // NOLINT(hicpp-signed-bitwise);
        } else if (value.IsDouble()) {
            number = base::NumberHelper::DoubleToInt(value.GetDouble(), base::INT32_BITS);
            SET_ACC(JSTaggedValue(~number));  // NOLINT(hicpp-signed-bitwise);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Not(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(NOT_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_NOT_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::not"
                   << " v" << v0;
        JSTaggedValue value = GET_VREG_VALUE(v0);
        int32_t number;
        // number, fast path
        if (value.IsInt()) {
            number = static_cast<int32_t>(value.GetInt());
            SET_ACC(JSTaggedValue(~number));  // NOLINT(hicpp-signed-bitwise);
        } else if (value.IsDouble()) {
            number = base::NumberHelper::DoubleToInt(value.GetDouble(), base::INT32_BITS);
            SET_ACC(JSTaggedValue(~number));  // NOLINT(hicpp-signed-bitwise);
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Not(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_NOT_PREF_V8);
    }
    HANDLE_OPCODE(INC_IMM8) {
        LOG_INST() << "intrinsics::inc";

        JSTaggedValue value = GET_ACC();
        // number fast path
        if (value.IsInt()) {
            int32_t a0 = value.GetInt();
            if (UNLIKELY(a0 == INT32_MAX)) {
                auto ret = static_cast<double>(a0) + 1.0;
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 + 1));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(value.GetDouble() + 1.0));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Inc(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(INC_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_INC_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();

        LOG_INST() << "intrinsics::inc"
                   << " v" << v0;

        JSTaggedValue value = GET_VREG_VALUE(v0);
        // number fast path
        if (value.IsInt()) {
            int32_t a0 = value.GetInt();
            if (UNLIKELY(a0 == INT32_MAX)) {
                auto ret = static_cast<double>(a0) + 1.0;
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 + 1));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(value.GetDouble() + 1.0));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Inc(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_INC_PREF_V8);
    }
    HANDLE_OPCODE(DEC_IMM8) {
        LOG_INST() << "intrinsics::dec";

        JSTaggedValue value = GET_ACC();
        // number, fast path
        if (value.IsInt()) {
            int32_t a0 = value.GetInt();
            if (UNLIKELY(a0 == INT32_MIN)) {
                auto ret = static_cast<double>(a0) - 1.0;
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 - 1));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(value.GetDouble() - 1.0));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Dec(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEC_IMM8);
    }
    HANDLE_OPCODE(DEPRECATED_DEC_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::dec"
                   << " v" << v0;

        JSTaggedValue value = GET_VREG_VALUE(v0);
        // number, fast path
        if (value.IsInt()) {
            int32_t a0 = value.GetInt();
            if (UNLIKELY(a0 == INT32_MIN)) {
                auto ret = static_cast<double>(a0) - 1.0;
                SET_ACC(JSTaggedValue(ret));
            } else {
                SET_ACC(JSTaggedValue(a0 - 1));
            }
        } else if (value.IsDouble()) {
            SET_ACC(JSTaggedValue(value.GetDouble() - 1.0));
        } else {
            // slow path
            SAVE_PC();
            JSTaggedValue res = SlowRuntimeStub::Dec(thread, value);
            INTERPRETER_RETURN_IF_ABRUPT(res);
            SET_ACC(res);
        }
        DISPATCH(DEPRECATED_DEC_PREF_V8);
    }
    HANDLE_OPCODE(DEFINEFUNC_IMM8_ID16_IMM8) {
        uint16_t methodId = READ_INST_16_1();
        uint16_t length = READ_INST_8_3();
        LOG_INST() << "intrinsics::definefunc length: " << length;
        auto constpool = GetConstantPool(sp);
        Method *method = Method::Cast(GET_METHOD_FROM_CACHE(methodId).GetTaggedObject());
        ASSERT(method != nullptr);

        auto res = SlowRuntimeStub::DefineFunc(thread, method);
        JSFunction *jsFunc = JSFunction::Cast(res.GetTaggedObject());

        jsFunc->SetPropertyInlinedProps(thread, JSFunction::LENGTH_INLINE_PROPERTY_INDEX, JSTaggedValue(length));
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue envHandle = state->env;
        jsFunc->SetLexicalEnv(thread, envHandle);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        jsFunc->SetModule(thread, currentFunc->GetModule());
        jsFunc->SetHomeObject(thread, currentFunc->GetHomeObject());
        SET_ACC(JSTaggedValue(jsFunc));

        DISPATCH(DEFINEFUNC_IMM8_ID16_IMM8);
    }
    HANDLE_OPCODE(DEFINEFUNC_IMM16_ID16_IMM8) {
        uint16_t methodId = READ_INST_16_2();
        uint16_t length = READ_INST_8_4();
        LOG_INST() << "intrinsics::definefunc length: " << length;

        auto constpool = GetConstantPool(sp);
        Method *method = Method::Cast(GET_METHOD_FROM_CACHE(methodId).GetTaggedObject());
        ASSERT(method != nullptr);

        auto res = SlowRuntimeStub::DefineFunc(thread, method);
        JSFunction *jsFunc = JSFunction::Cast(res.GetTaggedObject());

        jsFunc->SetPropertyInlinedProps(thread, JSFunction::LENGTH_INLINE_PROPERTY_INDEX, JSTaggedValue(length));
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue envHandle = state->env;
        jsFunc->SetLexicalEnv(thread, envHandle);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        jsFunc->SetModule(thread, currentFunc->GetModule());
        jsFunc->SetHomeObject(thread, currentFunc->GetHomeObject());
        SET_ACC(JSTaggedValue(jsFunc));

        DISPATCH(DEFINEFUNC_IMM16_ID16_IMM8);
    }
    HANDLE_OPCODE(DEFINEMETHOD_IMM8_ID16_IMM8) {
        uint16_t methodId = READ_INST_16_1();
        uint16_t length = READ_INST_8_3();
        LOG_INST() << "intrinsics::definemethod length: " << length;
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        Method *method = Method::Cast(GET_METHOD_FROM_CACHE(methodId).GetTaggedObject());
        ASSERT(method != nullptr);
        RESTORE_ACC();

        SAVE_PC();
        JSTaggedValue homeObject = GET_ACC();
        auto res = SlowRuntimeStub::DefineMethod(thread, method, homeObject);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        JSFunction *result = JSFunction::Cast(res.GetTaggedObject());

        result->SetPropertyInlinedProps(thread, JSFunction::LENGTH_INLINE_PROPERTY_INDEX, JSTaggedValue(length));
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue taggedCurEnv = state->env;
        result->SetLexicalEnv(thread, taggedCurEnv);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        result->SetModule(thread, currentFunc->GetModule());
        SET_ACC(JSTaggedValue(result));

        DISPATCH(DEFINEMETHOD_IMM8_ID16_IMM8);
    }
    HANDLE_OPCODE(DEFINEMETHOD_IMM16_ID16_IMM8) {
        uint16_t methodId = READ_INST_16_2();
        uint16_t length = READ_INST_8_4();
        LOG_INST() << "intrinsics::definemethod length: " << length;
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        Method *method = Method::Cast(GET_METHOD_FROM_CACHE(methodId).GetTaggedObject());
        ASSERT(method != nullptr);
        RESTORE_ACC();

        SAVE_PC();
        JSTaggedValue homeObject = GET_ACC();
        auto res = SlowRuntimeStub::DefineMethod(thread, method, homeObject);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        JSFunction *result = JSFunction::Cast(res.GetTaggedObject());

        result->SetPropertyInlinedProps(thread, JSFunction::LENGTH_INLINE_PROPERTY_INDEX, JSTaggedValue(length));
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue taggedCurEnv = state->env;
        result->SetLexicalEnv(thread, taggedCurEnv);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        result->SetModule(thread, currentFunc->GetModule());
        SET_ACC(JSTaggedValue(result));

        DISPATCH(DEFINEMETHOD_IMM16_ID16_IMM8);
    }
    HANDLE_OPCODE(DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8) {
        uint16_t methodId = READ_INST_16_1();
        uint16_t literaId = READ_INST_16_3();
        uint16_t length = READ_INST_16_5();
        uint16_t v0 = READ_INST_8_7();
        LOG_INST() << "intrinsics::defineclasswithbuffer"
                   << " method id:" << methodId << " lexenv: v" << v0;

        JSTaggedValue proto = GET_VREG_VALUE(v0);

        SAVE_PC();
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue res =
            SlowRuntimeStub::CreateClassWithBuffer(thread, proto, state->env, GetConstantPool(sp), methodId, literaId);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        ASSERT(res.IsClassConstructor());
        JSFunction *cls = JSFunction::Cast(res.GetTaggedObject());

        cls->SetLexicalEnv(thread, state->env);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        cls->SetModule(thread, currentFunc->GetModule());

        SlowRuntimeStub::SetClassConstructorLength(thread, res, JSTaggedValue(length));

        SET_ACC(res);
        DISPATCH(DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8);
    }
    HANDLE_OPCODE(DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8) {
        uint16_t methodId = READ_INST_16_2();
        uint16_t literaId = READ_INST_16_4();
        uint16_t length = READ_INST_16_6();
        uint16_t v0 = READ_INST_8_8();
        LOG_INST() << "intrinsics::defineclasswithbuffer"
                   << " method id:" << methodId << " lexenv: v" << v0;

        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue proto = GET_VREG_VALUE(v0);

        SAVE_PC();
        JSTaggedValue res =
            SlowRuntimeStub::CreateClassWithBuffer(thread, proto, state->env, GetConstantPool(sp), methodId, literaId);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        ASSERT(res.IsClassConstructor());
        JSFunction *cls = JSFunction::Cast(res.GetTaggedObject());

        cls->SetLexicalEnv(thread, state->env);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        cls->SetModule(thread, currentFunc->GetModule());

        SlowRuntimeStub::SetClassConstructorLength(thread, res, JSTaggedValue(length));

        SET_ACC(res);
        DISPATCH(DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8);
    }
    HANDLE_OPCODE(DEPRECATED_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8) {
        uint16_t methodId = READ_INST_16_1();
        uint16_t length = READ_INST_16_5();
        uint16_t v0 = READ_INST_8_7();
        uint16_t v1 = READ_INST_8_8();
        LOG_INST() << "intrinsics::defineclasswithbuffer"
                   << " method id:" << methodId << " lexenv: v" << v0 << " parent: v" << v1;

        JSTaggedValue lexenv = GET_VREG_VALUE(v0);
        JSTaggedValue proto = GET_VREG_VALUE(v1);

        SAVE_PC();
        JSTaggedValue res =
            SlowRuntimeStub::CreateClassWithBuffer(thread, proto, lexenv, GetConstantPool(sp), methodId, methodId + 1);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        ASSERT(res.IsClassConstructor());
        JSFunction *cls = JSFunction::Cast(res.GetTaggedObject());

        lexenv = GET_VREG_VALUE(v0);  // slow runtime may gc
        cls->SetLexicalEnv(thread, lexenv);

        JSFunction *currentFunc = JSFunction::Cast((GET_FRAME(sp)->function).GetTaggedObject());
        cls->SetModule(thread, currentFunc->GetModule());

        SlowRuntimeStub::SetClassConstructorLength(thread, res, JSTaggedValue(length));

        SET_ACC(res);
        DISPATCH(DEPRECATED_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8);
    }
    HANDLE_OPCODE(RESUMEGENERATOR) {
        LOG_INST() << "intrinsics::resumegenerator";
        JSTaggedValue objVal = GET_ACC();

        if (objVal.IsAsyncGeneratorObject()) {
            JSAsyncGeneratorObject *obj = JSAsyncGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(obj->GetResumeResult());
        } else {
            JSGeneratorObject *obj = JSGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(obj->GetResumeResult());
        }
        DISPATCH(RESUMEGENERATOR);
    }
    HANDLE_OPCODE(DEPRECATED_RESUMEGENERATOR_PREF_V8) {
        LOG_INST() << "intrinsics::resumegenerator";
        uint16_t vs = READ_INST_8_1();
        JSTaggedValue objVal = GET_VREG_VALUE(vs);

        if (objVal.IsAsyncGeneratorObject()) {
            JSAsyncGeneratorObject *obj = JSAsyncGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(obj->GetResumeResult());
        } else {
            JSGeneratorObject *obj = JSGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(obj->GetResumeResult());
        }
        DISPATCH(DEPRECATED_RESUMEGENERATOR_PREF_V8);
    }
    HANDLE_OPCODE(GETRESUMEMODE) {
        LOG_INST() << "intrinsics::getresumemode";
        JSTaggedValue objVal = GET_ACC();

        if (objVal.IsAsyncGeneratorObject()) {
            JSAsyncGeneratorObject *obj = JSAsyncGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(JSTaggedValue(static_cast<int>(obj->GetResumeMode())));
        } else {
            JSGeneratorObject *obj = JSGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(JSTaggedValue(static_cast<int>(obj->GetResumeMode())));
        }
        DISPATCH(GETRESUMEMODE);
    }
    HANDLE_OPCODE(DEPRECATED_GETRESUMEMODE_PREF_V8) {
        LOG_INST() << "intrinsics::getresumemode";
        uint16_t vs = READ_INST_8_1();
        JSTaggedValue objVal = GET_VREG_VALUE(vs);

        if (objVal.IsAsyncGeneratorObject()) {
            JSAsyncGeneratorObject *obj = JSAsyncGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(JSTaggedValue(static_cast<int>(obj->GetResumeMode())));
        } else {
            JSGeneratorObject *obj = JSGeneratorObject::Cast(objVal.GetTaggedObject());
            SET_ACC(JSTaggedValue(static_cast<int>(obj->GetResumeMode())));
        }
        DISPATCH(DEPRECATED_GETRESUMEMODE_PREF_V8);
    }
    HANDLE_OPCODE(GETTEMPLATEOBJECT_IMM8) {
        LOG_INST() << "intrinsic::gettemplateobject";

        JSTaggedValue literal = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetTemplateObject(thread, literal);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETTEMPLATEOBJECT_IMM8);
    }
    HANDLE_OPCODE(GETTEMPLATEOBJECT_IMM16) {
        LOG_INST() << "intrinsic::gettemplateobject";

        JSTaggedValue literal = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetTemplateObject(thread, literal);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(GETTEMPLATEOBJECT_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_GETTEMPLATEOBJECT_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsic::gettemplateobject"
                   << " v" << v0;

        JSTaggedValue literal = GET_VREG_VALUE(v0);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::GetTemplateObject(thread, literal);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_GETTEMPLATEOBJECT_PREF_V8);
    }
    HANDLE_OPCODE(DELOBJPROP_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::delobjprop"
                   << " v0" << v0;

        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue prop = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::DelObjProp(thread, obj, prop);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        DISPATCH(DELOBJPROP_V8);
    }
    HANDLE_OPCODE(DEPRECATED_DELOBJPROP_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::delobjprop"
                   << " v0" << v0 << " v1" << v1;

        JSTaggedValue obj = GET_VREG_VALUE(v0);
        JSTaggedValue prop = GET_VREG_VALUE(v1);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::DelObjProp(thread, obj, prop);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        DISPATCH(DEPRECATED_DELOBJPROP_PREF_V8_V8);
    }
    HANDLE_OPCODE(SUSPENDGENERATOR_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::suspendgenerator"
                   << " v" << v0;
        JSTaggedValue genObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_ACC();
        // suspend will record bytecode offset
        SAVE_PC();
        SAVE_ACC();
        JSTaggedValue res = SlowRuntimeStub::SuspendGenerator(thread, genObj, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        InterpretedFrame *state = GET_FRAME(sp);
        methodHandle.Update(JSFunction::Cast(state->function.GetTaggedObject())->GetMethod());
        [[maybe_unused]] auto fistPC = methodHandle->GetBytecodeArray();
        UPDATE_HOTNESS_COUNTER(-(pc - fistPC));
        LOG_INST() << "Exit: SuspendGenerator " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                << std::hex << reinterpret_cast<uintptr_t>(state->pc);
        sp = state->base.prev;
        ASSERT(sp != nullptr);
        InterpretedFrame *prevState = GET_FRAME(sp);
        pc = prevState->pc;
        // entry frame
        if (FrameHandler::IsEntryFrame(pc)) {
            state->acc = acc;
            return;
        }

        thread->SetCurrentSPFrame(sp);

        INTERPRETER_HANDLE_RETURN();
    }
    HANDLE_OPCODE(DEPRECATED_SUSPENDGENERATOR_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::suspendgenerator"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue genObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_VREG_VALUE(v1);
        // suspend will record bytecode offset
        SAVE_PC();
        SAVE_ACC();
        JSTaggedValue res = SlowRuntimeStub::SuspendGenerator(thread, genObj, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);

        InterpretedFrame *state = GET_FRAME(sp);
        Method *method = JSFunction::Cast(state->function.GetTaggedObject())->GetCallTarget();
        [[maybe_unused]] auto fistPC = method->GetBytecodeArray();
        UPDATE_HOTNESS_COUNTER(-(pc - fistPC));
        LOG_INST() << "Exit: SuspendGenerator " << std::hex << reinterpret_cast<uintptr_t>(sp) << " "
                                << std::hex << reinterpret_cast<uintptr_t>(state->pc);
        sp = state->base.prev;
        ASSERT(sp != nullptr);
        InterpretedFrame *prevState = GET_FRAME(sp);
        pc = prevState->pc;
        // entry frame
        if (FrameHandler::IsEntryFrame(pc)) {
            state->acc = acc;
            return;
        }

        thread->SetCurrentSPFrame(sp);

        INTERPRETER_HANDLE_RETURN();
    }
    HANDLE_OPCODE(ASYNCFUNCTIONAWAITUNCAUGHT_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::asyncfunctionawaituncaught"
                   << " v" << v0;
        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionAwaitUncaught(thread, asyncFuncObj, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ASYNCFUNCTIONAWAITUNCAUGHT_V8);
    }
    HANDLE_OPCODE(DEPRECATED_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::asyncfunctionawaituncaught"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_VREG_VALUE(v1);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionAwaitUncaught(thread, asyncFuncObj, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8);
    }
    HANDLE_OPCODE(COPYDATAPROPERTIES_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsic::copydataproperties"
                   << " v" << v0;
        JSTaggedValue dst = GET_VREG_VALUE(v0);
        JSTaggedValue src = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CopyDataProperties(thread, dst, src);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(COPYDATAPROPERTIES_V8);
    }
    HANDLE_OPCODE(DEPRECATED_COPYDATAPROPERTIES_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsic::copydataproperties"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue dst = GET_VREG_VALUE(v0);
        JSTaggedValue src = GET_VREG_VALUE(v1);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CopyDataProperties(thread, dst, src);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_COPYDATAPROPERTIES_PREF_V8_V8);
    }
    HANDLE_OPCODE(SETOBJECTWITHPROTO_IMM8_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::setobjectwithproto"
                   << " v" << v0;
        JSTaggedValue proto = GET_VREG_VALUE(v0);
        JSTaggedValue obj = GET_ACC();
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SetObjectWithProto(thread, proto, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(SETOBJECTWITHPROTO_IMM8_V8);
    }
    HANDLE_OPCODE(SETOBJECTWITHPROTO_IMM16_V8) {
        uint16_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::setobjectwithproto"
                   << " v" << v0;
        JSTaggedValue proto = GET_VREG_VALUE(v0);
        JSTaggedValue obj = GET_ACC();
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SetObjectWithProto(thread, proto, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(SETOBJECTWITHPROTO_IMM16_V8);
    }
    HANDLE_OPCODE(DEPRECATED_SETOBJECTWITHPROTO_PREF_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        uint16_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::setobjectwithproto"
                   << " v" << v0 << " v" << v1;
        JSTaggedValue proto = GET_VREG_VALUE(v0);
        JSTaggedValue obj = GET_VREG_VALUE(v1);
        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::SetObjectWithProto(thread, proto, obj);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(DEPRECATED_SETOBJECTWITHPROTO_PREF_V8_V8);
    }
    HANDLE_OPCODE(LDOBJBYVALUE_IMM8_V8) {
        uint32_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::Ldobjbyvalue"
                   << " v" << v0;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_ACC();

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();

            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryLoadICByValue(thread, receiver, propKey, firstValue, secondValue);
            }
            // IC miss and not enter the megamorphic state, store as polymorphic
            if (res.IsHole() && !firstValue.IsHole()) {
                res = ICRuntimeStub::LoadICByValue(thread,
                                                   profileTypeArray,
                                                   receiver, propKey, slotId);
            }

            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYVALUE_IMM8_V8);
            }
        }
#endif
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByValue(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYVALUE_IMM8_V8);
            }
        }
        // slow path
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByValue(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYVALUE_IMM8_V8);
    }
    HANDLE_OPCODE(LDOBJBYVALUE_IMM16_V8) {
        uint32_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::Ldobjbyvalue"
                   << " v" << v0;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_ACC();

#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();

            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryLoadICByValue(thread, receiver, propKey, firstValue, secondValue);
            }
            // IC miss and not enter the megamorphic state, store as polymorphic
            if (res.IsHole() && !firstValue.IsHole()) {
                res = ICRuntimeStub::LoadICByValue(thread,
                                                   profileTypeArray,
                                                   receiver, propKey, slotId);
            }

            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYVALUE_IMM16_V8);
            }
        }
#endif
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByValue(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYVALUE_IMM16_V8);
            }
        }
        // slow path
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByValue(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYVALUE_IMM16_V8);
    }
    HANDLE_OPCODE(DEPRECATED_LDOBJBYVALUE_PREF_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::Ldobjbyvalue"
                   << " v" << v0 << " v" << v1;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_VREG_VALUE(v1);

        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByValue(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(DEPRECATED_LDOBJBYVALUE_PREF_V8_V8);
            }
        }
        // slow path
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByValue(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_LDOBJBYVALUE_PREF_V8_V8);
    }
    HANDLE_OPCODE(LDSUPERBYVALUE_IMM8_V8) {
        uint32_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::Ldsuperbyvalue"
                   << " v" << v0;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_ACC();

        // slow path
        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, receiver, propKey, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDSUPERBYVALUE_IMM8_V8);
    }
    HANDLE_OPCODE(LDSUPERBYVALUE_IMM16_V8) {
        uint32_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::Ldsuperbyvalue"
                   << " v" << v0;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_ACC();

        // slow path
        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, receiver, propKey, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDSUPERBYVALUE_IMM8_V8);
    }
    HANDLE_OPCODE(DEPRECATED_LDSUPERBYVALUE_PREF_V8_V8) {
        uint32_t v0 = READ_INST_8_1();
        uint32_t v1 = READ_INST_8_2();
        LOG_INST() << "intrinsics::Ldsuperbyvalue"
                   << " v" << v0 << " v" << v1;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        JSTaggedValue propKey = GET_VREG_VALUE(v1);

        // slow path
        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, receiver, propKey, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_LDSUPERBYVALUE_PREF_V8_V8);
    }
    HANDLE_OPCODE(LDOBJBYINDEX_IMM8_IMM16) {
        uint32_t idx = READ_INST_16_1();
        LOG_INST() << "intrinsics::ldobjbyindex"
                   << " imm" << idx;

        JSTaggedValue receiver = GET_ACC();
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByIndex(thread, receiver, idx);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYINDEX_IMM8_IMM16);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByIndex(thread, receiver, idx, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYINDEX_IMM8_IMM16);
    }
    HANDLE_OPCODE(LDOBJBYINDEX_IMM16_IMM16) {
        uint32_t idx = READ_INST_16_2();
        LOG_INST() << "intrinsics::ldobjbyindex"
                   << " imm" << idx;

        JSTaggedValue receiver = GET_ACC();
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByIndex(thread, receiver, idx);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYINDEX_IMM16_IMM16);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByIndex(thread, receiver, idx, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYINDEX_IMM16_IMM16);
    }
    HANDLE_OPCODE(WIDE_LDOBJBYINDEX_PREF_IMM32) {
        uint32_t idx = READ_INST_32_1();
        LOG_INST() << "intrinsics::ldobjbyindex"
                   << " imm" << idx;

        JSTaggedValue receiver = GET_ACC();
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByIndex(thread, receiver, idx);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(WIDE_LDOBJBYINDEX_PREF_IMM32);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByIndex(thread, receiver, idx, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_LDOBJBYINDEX_PREF_IMM32);
    }
    HANDLE_OPCODE(DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32) {
        uint16_t v0 = READ_INST_8_1();
        uint32_t idx = READ_INST_32_2();
        LOG_INST() << "intrinsics::ldobjbyindex"
                   << " v" << v0 << " imm" << idx;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        // fast path
        if (LIKELY(receiver.IsHeapObject())) {
            JSTaggedValue res = FastRuntimeStub::GetPropertyByIndex(thread, receiver, idx);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByIndex(thread, receiver, idx, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32);
    }
    HANDLE_OPCODE(STOBJBYINDEX_IMM8_V8_IMM16) {
        uint8_t v0 = READ_INST_8_1();
        uint16_t index = READ_INST_16_2();
        LOG_INST() << "intrinsics::stobjbyindex"
                   << " v" << v0 << " imm" << index;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByIndex(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYINDEX_IMM8_V8_IMM16);
            }
            RESTORE_ACC();
        }
        // slow path
        SAVE_ACC();
        SAVE_PC();
        receiver = GET_VREG_VALUE(v0);    // Maybe moved by GC
        JSTaggedValue value = GET_ACC();  // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StObjByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOBJBYINDEX_IMM8_V8_IMM16);
    }
    HANDLE_OPCODE(STOBJBYINDEX_IMM16_V8_IMM16) {
        uint8_t v0 = READ_INST_8_2();
        uint16_t index = READ_INST_16_3();
        LOG_INST() << "intrinsics::stobjbyindex"
                   << " v" << v0 << " imm" << index;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByIndex(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOBJBYINDEX_IMM16_V8_IMM16);
            }
            RESTORE_ACC();
        }
        // slow path
        SAVE_ACC();
        SAVE_PC();
        receiver = GET_VREG_VALUE(v0);    // Maybe moved by GC
        JSTaggedValue value = GET_ACC();  // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StObjByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOBJBYINDEX_IMM16_V8_IMM16);
    }
    HANDLE_OPCODE(WIDE_STOBJBYINDEX_PREF_V8_IMM32) {
        uint8_t v0 = READ_INST_8_1();
        uint32_t index = READ_INST_32_2();
        LOG_INST() << "intrinsics::stobjbyindex"
                   << " v" << v0 << " imm" << index;

        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        if (receiver.IsHeapObject()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res = FastRuntimeStub::SetPropertyByIndex(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(WIDE_STOBJBYINDEX_PREF_V8_IMM32);
            }
            RESTORE_ACC();
        }
        // slow path
        SAVE_ACC();
        SAVE_PC();
        receiver = GET_VREG_VALUE(v0);    // Maybe moved by GC
        JSTaggedValue value = GET_ACC();  // Maybe moved by GC
        JSTaggedValue res = SlowRuntimeStub::StObjByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(WIDE_STOBJBYINDEX_PREF_V8_IMM32);
    }
    HANDLE_OPCODE(STOWNBYINDEX_IMM8_V8_IMM16) {
        uint8_t v0 = READ_INST_8_1();
        uint16_t index = READ_INST_16_2();
        LOG_INST() << "intrinsics::stownbyindex"
                   << " v" << v0 << " imm" << index;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        // fast path
        if (receiver.IsHeapObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res =
                FastRuntimeStub::SetPropertyByIndex<true>(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOWNBYINDEX_IMM8_V8_IMM16);
            }
            RESTORE_ACC();
        }
        SAVE_ACC();
        receiver = GET_VREG_VALUE(v0);  // Maybe moved by GC
        auto value = GET_ACC();         // Maybe moved by GC
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StOwnByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOWNBYINDEX_IMM8_V8_IMM16);
    }
    HANDLE_OPCODE(STOWNBYINDEX_IMM16_V8_IMM16) {
        uint8_t v0 = READ_INST_8_2();
        uint16_t index = READ_INST_16_3();
        LOG_INST() << "intrinsics::stownbyindex"
                   << " v" << v0 << " imm" << index;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        // fast path
        if (receiver.IsHeapObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res =
                FastRuntimeStub::SetPropertyByIndex<true>(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(STOWNBYINDEX_IMM16_V8_IMM16);
            }
            RESTORE_ACC();
        }
        SAVE_ACC();
        receiver = GET_VREG_VALUE(v0);  // Maybe moved by GC
        auto value = GET_ACC();         // Maybe moved by GC
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StOwnByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STOWNBYINDEX_IMM16_V8_IMM16);
    }
    HANDLE_OPCODE(WIDE_STOWNBYINDEX_PREF_V8_IMM32) {
        uint8_t v0 = READ_INST_8_1();
        uint32_t index = READ_INST_32_2();
        LOG_INST() << "intrinsics::stownbyindex"
                   << " v" << v0 << " imm" << index;
        JSTaggedValue receiver = GET_VREG_VALUE(v0);
        // fast path
        if (receiver.IsHeapObject() && !receiver.IsClassConstructor() && !receiver.IsClassPrototype()) {
            SAVE_ACC();
            JSTaggedValue value = GET_ACC();
            // fast path
            JSTaggedValue res =
                FastRuntimeStub::SetPropertyByIndex<true>(thread, receiver, index, value);
            if (!res.IsHole()) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                RESTORE_ACC();
                DISPATCH(WIDE_STOWNBYINDEX_PREF_V8_IMM32);
            }
            RESTORE_ACC();
        }
        SAVE_ACC();
        receiver = GET_VREG_VALUE(v0);  // Maybe moved by GC
        auto value = GET_ACC();         // Maybe moved by GC
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StOwnByIndex(thread, receiver, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(WIDE_STOWNBYINDEX_PREF_V8_IMM32);
    }
    HANDLE_OPCODE(COPYRESTARGS_IMM8) {
        uint8_t restIdx = READ_INST_8_0();
        LOG_INST() << "intrinsics::copyrestargs"
                   << " index: " << restIdx;

        uint32_t startIdx = 0;
        uint32_t restNumArgs = GetNumArgs(sp, restIdx, startIdx);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CopyRestArgs(thread, sp, restNumArgs, startIdx);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(COPYRESTARGS_IMM8);
    }
    HANDLE_OPCODE(WIDE_COPYRESTARGS_PREF_IMM16) {
        uint16_t restIdx = READ_INST_16_1();
        LOG_INST() << "intrinsics::copyrestargs"
                   << " index: " << restIdx;

        uint32_t startIdx = 0;
        uint32_t restNumArgs = GetNumArgs(sp, restIdx, startIdx);

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::CopyRestArgs(thread, sp, restNumArgs, startIdx);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_COPYRESTARGS_PREF_IMM16);
    }
    HANDLE_OPCODE(LDLEXVAR_IMM4_IMM4) {
        uint16_t level = READ_INST_4_0();
        uint16_t slot = READ_INST_4_1();

        LOG_INST() << "intrinsics::ldlexvar"
                   << " level:" << level << " slot:" << slot;
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        JSTaggedValue env(currentLexenv);
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        SET_ACC(LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot));
        DISPATCH(LDLEXVAR_IMM4_IMM4);
    }
    HANDLE_OPCODE(LDLEXVAR_IMM8_IMM8) {
        uint16_t level = READ_INST_8_0();
        uint16_t slot = READ_INST_8_1();

        LOG_INST() << "intrinsics::ldlexvar"
                   << " level:" << level << " slot:" << slot;
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        JSTaggedValue env(currentLexenv);
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        SET_ACC(LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot));
        DISPATCH(LDLEXVAR_IMM8_IMM8);
    }
    HANDLE_OPCODE(WIDE_LDLEXVAR_PREF_IMM16_IMM16) {
        uint16_t level = READ_INST_16_1();
        uint16_t slot = READ_INST_16_3();

        LOG_INST() << "intrinsics::ldlexvar"
                   << " level:" << level << " slot:" << slot;
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue currentLexenv = state->env;
        JSTaggedValue env(currentLexenv);
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        SET_ACC(LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot));
        DISPATCH(WIDE_LDLEXVAR_PREF_IMM16_IMM16);
    }
    HANDLE_OPCODE(STLEXVAR_IMM4_IMM4) {
        uint16_t level = READ_INST_4_0();
        uint16_t slot = READ_INST_4_1();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot;

        JSTaggedValue value = GET_ACC();
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(STLEXVAR_IMM4_IMM4);
    }
    HANDLE_OPCODE(STLEXVAR_IMM8_IMM8) {
        uint16_t level = READ_INST_8_0();
        uint16_t slot = READ_INST_8_1();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot;

        JSTaggedValue value = GET_ACC();
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(STLEXVAR_IMM8_IMM8);
    }
    HANDLE_OPCODE(WIDE_STLEXVAR_PREF_IMM16_IMM16) {
        uint16_t level = READ_INST_16_1();
        uint16_t slot = READ_INST_16_3();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot;

        JSTaggedValue value = GET_ACC();
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(WIDE_STLEXVAR_PREF_IMM16_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_STLEXVAR_PREF_IMM4_IMM4_V8) {
        uint16_t level = READ_INST_4_2();
        uint16_t slot = READ_INST_4_3();
        uint16_t v0 = READ_INST_8_2();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot << " v" << v0;

        JSTaggedValue value = GET_VREG_VALUE(v0);
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM4_IMM4_V8);
    }
    HANDLE_OPCODE(DEPRECATED_STLEXVAR_PREF_IMM8_IMM8_V8) {
        uint16_t level = READ_INST_8_1();
        uint16_t slot = READ_INST_8_2();
        uint16_t v0 = READ_INST_8_3();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot << " v" << v0;

        JSTaggedValue value = GET_VREG_VALUE(v0);
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM8_IMM8_V8);
    }
    HANDLE_OPCODE(DEPRECATED_STLEXVAR_PREF_IMM16_IMM16_V8) {
        uint16_t level = READ_INST_16_1();
        uint16_t slot = READ_INST_16_3();
        uint16_t v0 = READ_INST_8_5();
        LOG_INST() << "intrinsics::stlexvar"
                   << " level:" << level << " slot:" << slot << " v" << v0;

        JSTaggedValue value = GET_VREG_VALUE(v0);
        InterpretedFrame *state = GET_FRAME(sp);
        JSTaggedValue env = state->env;
        for (uint32_t i = 0; i < level; i++) {
            JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
            ASSERT(!taggedParentEnv.IsUndefined());
            env = taggedParentEnv;
        }
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, value);

        DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM16_IMM16_V8);
    }
    HANDLE_OPCODE(GETMODULENAMESPACE_IMM8) {
        uint16_t stringId = READ_INST_8_0();
        auto constpool = GetConstantPool(sp);
        auto localName = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::getmodulenamespace "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(localName.GetTaggedObject()));

        JSTaggedValue moduleNamespace = SlowRuntimeStub::GetModuleNamespace(thread, localName);
        INTERPRETER_RETURN_IF_ABRUPT(moduleNamespace);
        SET_ACC(moduleNamespace);
        DISPATCH(GETMODULENAMESPACE_IMM8);
    }
    HANDLE_OPCODE(WIDE_GETMODULENAMESPACE_PREF_IMM16) {
        uint16_t stringId = READ_INST_16_1();
        auto constpool = GetConstantPool(sp);
        auto localName = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::getmodulenamespace "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(localName.GetTaggedObject()));

        JSTaggedValue moduleNamespace = SlowRuntimeStub::GetModuleNamespace(thread, localName);
        INTERPRETER_RETURN_IF_ABRUPT(moduleNamespace);
        SET_ACC(moduleNamespace);
        DISPATCH(WIDE_GETMODULENAMESPACE_PREF_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_GETMODULENAMESPACE_PREF_ID32) {
        uint16_t stringId = READ_INST_32_1();
        auto constpool = GetConstantPool(sp);
        auto localName = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::getmodulenamespace "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(localName.GetTaggedObject()));

        JSTaggedValue moduleNamespace = SlowRuntimeStub::GetModuleNamespace(thread, localName);
        INTERPRETER_RETURN_IF_ABRUPT(moduleNamespace);
        SET_ACC(moduleNamespace);
        DISPATCH(DEPRECATED_GETMODULENAMESPACE_PREF_ID32);
    }
    HANDLE_OPCODE(STMODULEVAR_IMM8) {
        uint16_t stringId = READ_INST_8_0();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        auto key = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();

        LOG_INST() << "intrinsics::stmodulevar "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();

        SlowRuntimeStub::StModuleVar(thread, key, value);
        RESTORE_ACC();
        DISPATCH(STMODULEVAR_IMM8);
    }
    HANDLE_OPCODE(WIDE_STMODULEVAR_PREF_IMM16) {
        uint16_t stringId = READ_INST_16_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        auto key = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();

        LOG_INST() << "intrinsics::stmodulevar "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();

        SlowRuntimeStub::StModuleVar(thread, key, value);
        RESTORE_ACC();
        DISPATCH(WIDE_STMODULEVAR_PREF_IMM16);
    }
    HANDLE_OPCODE(DEPRECATED_STMODULEVAR_PREF_ID32) {
        uint16_t stringId = READ_INST_32_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        auto key = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();

        LOG_INST() << "intrinsics::stmodulevar "
                   << "stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();

        SlowRuntimeStub::StModuleVar(thread, key, value);
        RESTORE_ACC();
        DISPATCH(DEPRECATED_STMODULEVAR_PREF_ID32);
    }
    HANDLE_OPCODE(LDLOCALMODULEVAR_IMM8) {
        uint16_t stringId = READ_INST_8_0();

        auto constpool = GetConstantPool(sp);
        JSTaggedValue key = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldmodulevar "
                   << "string_id:" << stringId << ", "
                   << "key: " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue moduleVar = SlowRuntimeStub::LdModuleVar(thread, key, true);
        INTERPRETER_RETURN_IF_ABRUPT(moduleVar);
        SET_ACC(moduleVar);
        DISPATCH(LDLOCALMODULEVAR_IMM8);
    }
    HANDLE_OPCODE(WIDE_LDLOCALMODULEVAR_PREF_IMM16) {
        uint16_t stringId = READ_INST_16_1();

        auto constpool = GetConstantPool(sp);
        JSTaggedValue key = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldmodulevar "
                   << "string_id:" << stringId << ", "
                   << "key: " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue moduleVar = SlowRuntimeStub::LdModuleVar(thread, key, true);
        INTERPRETER_RETURN_IF_ABRUPT(moduleVar);
        SET_ACC(moduleVar);
        DISPATCH(WIDE_LDLOCALMODULEVAR_PREF_IMM16);
    }
    HANDLE_OPCODE(LDEXTERNALMODULEVAR_IMM8) {
        uint16_t stringId = READ_INST_8_0();

        auto constpool = GetConstantPool(sp);
        JSTaggedValue key = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldmodulevar "
                   << "string_id:" << stringId << ", "
                   << "key: " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue moduleVar = SlowRuntimeStub::LdModuleVar(thread, key, false);
        INTERPRETER_RETURN_IF_ABRUPT(moduleVar);
        SET_ACC(moduleVar);
        DISPATCH(LDEXTERNALMODULEVAR_IMM8);
    }
    HANDLE_OPCODE(WIDE_LDEXTERNALMODULEVAR_PREF_IMM16) {
        uint16_t stringId = READ_INST_16_1();

        auto constpool = GetConstantPool(sp);
        JSTaggedValue key = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldmodulevar "
                   << "string_id:" << stringId << ", "
                   << "key: " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue moduleVar = SlowRuntimeStub::LdModuleVar(thread, key, false);
        INTERPRETER_RETURN_IF_ABRUPT(moduleVar);
        SET_ACC(moduleVar);
        DISPATCH(WIDE_LDEXTERNALMODULEVAR_PREF_IMM16);
    }
    HANDLE_OPCODE(STCONSTTOGLOBALRECORD_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::stconsttoglobalrecord"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalRecord(thread, propKey, value, true);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STCONSTTOGLOBALRECORD_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_STCONSTTOGLOBALRECORD_PREF_ID32) {
        uint16_t stringId = READ_INST_32_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::stconsttoglobalrecord"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalRecord(thread, propKey, value, true);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(DEPRECATED_STCONSTTOGLOBALRECORD_PREF_ID32);
    }
    HANDLE_OPCODE(STTOGLOBALRECORD_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::stlettoglobalrecord"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalRecord(thread, propKey, value, false);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(STTOGLOBALRECORD_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_STLETTOGLOBALRECORD_PREF_ID32) {
        uint16_t stringId = READ_INST_32_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::stlettoglobalrecord"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalRecord(thread, propKey, value, false);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(DEPRECATED_STLETTOGLOBALRECORD_PREF_ID32);
    }
    HANDLE_OPCODE(DEPRECATED_STCLASSTOGLOBALRECORD_PREF_ID32) {
        uint16_t stringId = READ_INST_32_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        LOG_INST() << "intrinsics::stclasstoglobalrecord"
                   << " stringId:" << stringId << ", " << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject()));

        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StGlobalRecord(thread, propKey, value, false);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(DEPRECATED_STCLASSTOGLOBALRECORD_PREF_ID32);
    }
    HANDLE_OPCODE(DEPRECATED_LDMODULEVAR_PREF_ID32_IMM8) {
        uint16_t stringId = READ_INST_16_1();
        uint8_t innerFlag = READ_INST_8_5();

        auto constpool = GetConstantPool(sp);
        JSTaggedValue key = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldmodulevar "
                   << "string_id:" << stringId << ", "
                   << "key: " << ConvertToString(EcmaString::Cast(key.GetTaggedObject()));

        JSTaggedValue moduleVar = SlowRuntimeStub::LdModuleVar(thread, key, innerFlag != 0);
        INTERPRETER_RETURN_IF_ABRUPT(moduleVar);
        SET_ACC(moduleVar);
        DISPATCH(DEPRECATED_LDMODULEVAR_PREF_ID32_IMM8);
    }
    HANDLE_OPCODE(LDOBJBYNAME_IMM8_ID16) {
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_8_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();

            JSTaggedValue receiver = GET_ACC();
            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryLoadICByName(thread, receiver, firstValue, secondValue);
            }
            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM8_ID16);
            } else if (!firstValue.IsHole()) { // IC miss and not enter the megamorphic state, store as polymorphic
                uint16_t stringId = READ_INST_16_1();
                SAVE_ACC();
                auto constpool = GetConstantPool(sp);
                JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
                RESTORE_ACC();
                receiver = GET_ACC();
                res = ICRuntimeStub::LoadICByName(thread,
                                                  profileTypeArray,
                                                  receiver, propKey, slotId);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM8_ID16);
            }
        }
#endif
        uint16_t stringId = READ_INST_16_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        JSTaggedValue receiver = GET_ACC();
        LOG_INST() << "intrinsics::ldobjbyname stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << receiver.GetRawData();

        if (LIKELY(receiver.IsHeapObject())) {
            // fast path
            JSTaggedValue res = FastRuntimeStub::GetPropertyByName(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM8_ID16);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByName(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(LDOBJBYNAME_IMM16_ID16) {
#if ECMASCRIPT_ENABLE_IC
        auto profileTypeInfo = GetRuntimeProfileTypeInfo(sp);
        if (!profileTypeInfo.IsUndefined()) {
            uint16_t slotId = READ_INST_16_0();
            auto profileTypeArray = ProfileTypeInfo::Cast(profileTypeInfo.GetTaggedObject());
            JSTaggedValue firstValue = profileTypeArray->Get(slotId);
            JSTaggedValue res = JSTaggedValue::Hole();

            JSTaggedValue receiver = GET_ACC();
            if (LIKELY(firstValue.IsHeapObject())) {
                JSTaggedValue secondValue = profileTypeArray->Get(slotId + 1);
                res = ICRuntimeStub::TryLoadICByName(thread, receiver, firstValue, secondValue);
            }
            if (LIKELY(!res.IsHole())) {
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM16_ID16);
            } else if (!firstValue.IsHole()) { // IC miss and not enter the megamorphic state, store as polymorphic
                uint16_t stringId = READ_INST_16_2();
                SAVE_ACC();
                auto constpool = GetConstantPool(sp);
                JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
                RESTORE_ACC();
                receiver = GET_ACC();
                res = ICRuntimeStub::LoadICByName(thread,
                                                  profileTypeArray,
                                                  receiver, propKey, slotId);
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM16_ID16);
            }
        }
#endif
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        JSTaggedValue receiver = GET_ACC();
        LOG_INST() << "intrinsics::ldobjbyname stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << receiver.GetRawData();

        if (LIKELY(receiver.IsHeapObject())) {
            // fast path
            JSTaggedValue res = FastRuntimeStub::GetPropertyByName(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(LDOBJBYNAME_IMM16_ID16);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByName(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDOBJBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_LDOBJBYNAME_PREF_ID32_V8) {
        uint32_t v0 = READ_INST_8_5();
        JSTaggedValue receiver = GET_VREG_VALUE(v0);

        uint16_t stringId = READ_INST_32_1();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        LOG_INST() << "intrinsics::ldobjbyname "
                   << "v" << v0 << " stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << receiver.GetRawData();

        if (LIKELY(receiver.IsHeapObject())) {
            // fast path
            JSTaggedValue res = FastRuntimeStub::GetPropertyByName(thread, receiver, propKey);
            if (!res.IsHole()) {
                ASSERT(!res.IsAccessor());
                INTERPRETER_RETURN_IF_ABRUPT(res);
                SET_ACC(res);
                DISPATCH(DEPRECATED_LDOBJBYNAME_PREF_ID32_V8);
            }
        }
        // not meet fast condition or fast path return hole, walk slow path
        // slow stub not need receiver
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdObjByName(thread, receiver, propKey, false, JSTaggedValue::Undefined());
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_LDOBJBYNAME_PREF_ID32_V8);
    }
    HANDLE_OPCODE(LDSUPERBYNAME_IMM8_ID16) {
        uint16_t stringId = READ_INST_16_1();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();

        JSTaggedValue obj = GET_ACC();
        LOG_INST() << "intrinsics::ldsuperbyname stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << obj.GetRawData();

        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, obj, propKey, thisFunc);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDSUPERBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(LDSUPERBYNAME_IMM16_ID16) {
        uint16_t stringId = READ_INST_16_2();
        SAVE_ACC();
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);
        RESTORE_ACC();
        JSTaggedValue obj = GET_ACC();

        LOG_INST() << "intrinsics::ldsuperbyname stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << obj.GetRawData();

        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, obj, propKey, thisFunc);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(LDSUPERBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(DEPRECATED_LDSUPERBYNAME_PREF_ID32_V8) {
        uint32_t stringId = READ_INST_32_1();
        uint32_t v0 = READ_INST_8_5();
        JSTaggedValue obj = GET_VREG_VALUE(v0);
        auto constpool = GetConstantPool(sp);
        JSTaggedValue propKey = GET_STR_FROM_CACHE(stringId);

        LOG_INST() << "intrinsics::ldsuperbyname"
                   << "v" << v0 << " stringId:" << stringId << ", "
                   << ConvertToString(EcmaString::Cast(propKey.GetTaggedObject())) << ", obj:" << obj.GetRawData();

        SAVE_PC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        JSTaggedValue res = SlowRuntimeStub::LdSuperByValue(thread, obj, propKey, thisFunc);

        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_LDSUPERBYNAME_PREF_ID32_V8);
    }
    HANDLE_OPCODE(ASYNCFUNCTIONRESOLVE_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::asyncfunctionresolve"
                   << " v" << v0;

        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionResolveOrReject(thread, asyncFuncObj, value, true);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ASYNCFUNCTIONRESOLVE_V8);
    }
    HANDLE_OPCODE(DEPRECATED_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        [[maybe_unused]] uint16_t v1 = READ_INST_8_2();
        uint16_t v2 = READ_INST_8_3();
        LOG_INST() << "intrinsics::asyncfunctionresolve"
                   << " v" << v0 << " v" << v1 << " v" << v2;

        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_VREG_VALUE(v2);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionResolveOrReject(thread, asyncFuncObj, value, true);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8);
    }
    HANDLE_OPCODE(ASYNCFUNCTIONREJECT_V8) {
        uint16_t v0 = READ_INST_8_0();
        LOG_INST() << "intrinsics::asyncfunctionreject"
                   << " v" << v0;

        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionResolveOrReject(thread, asyncFuncObj, value, false);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(ASYNCFUNCTIONREJECT_V8);
    }
    HANDLE_OPCODE(DEPRECATED_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8) {
        uint16_t v0 = READ_INST_8_1();
        [[maybe_unused]] uint16_t v1 = READ_INST_8_2();
        uint16_t v2 = READ_INST_8_3();
        LOG_INST() << "intrinsics::asyncfunctionreject"
                   << " v" << v0 << " v" << v1 << " v" << v2;

        JSTaggedValue asyncFuncObj = GET_VREG_VALUE(v0);
        JSTaggedValue value = GET_VREG_VALUE(v2);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::AsyncFunctionResolveOrReject(thread, asyncFuncObj, value, false);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8);
    }
    // TODO: Implement code of 16bits ic slot bytecode
    HANDLE_OPCODE(CLOSEITERATOR_IMM16_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(CLOSEITERATOR_IMM16_V8);
    }
    HANDLE_OPCODE(STOWNBYVALUE_IMM16_V8_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STOWNBYVALUE_IMM16_V8_V8);
    }
    HANDLE_OPCODE(STSUPERBYVALUE_IMM16_V8_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STSUPERBYVALUE_IMM16_V8_V8);
    }
    HANDLE_OPCODE(TRYSTGLOBALBYNAME_IMM16_ID16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(TRYSTGLOBALBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(STSUPERBYNAME_IMM16_ID16_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STSUPERBYNAME_IMM16_ID16_V8);
    }
    HANDLE_OPCODE(STOWNBYVALUEWITHNAMESET_IMM16_V8_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STOWNBYVALUEWITHNAMESET_IMM16_V8_V8);
    }
    HANDLE_OPCODE(STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8);
    }
    HANDLE_OPCODE(JSTRICTEQZ_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQZ_IMM8);
    }
    HANDLE_OPCODE(JSTRICTEQZ_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQZ_IMM16);
    }
    HANDLE_OPCODE(JNSTRICTEQZ_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQZ_IMM8);
    }
    HANDLE_OPCODE(JNSTRICTEQZ_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQZ_IMM16);
    }
    HANDLE_OPCODE(JEQNULL_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQNULL_IMM8);
    }
    HANDLE_OPCODE(JEQNULL_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQNULL_IMM16);
    }
    HANDLE_OPCODE(JNENULL_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNENULL_IMM8);
    }
    HANDLE_OPCODE(JNENULL_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNENULL_IMM16);
    }
    HANDLE_OPCODE(JSTRICTEQNULL_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQNULL_IMM8);
    }
    HANDLE_OPCODE(JSTRICTEQNULL_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQNULL_IMM16);
    }
    HANDLE_OPCODE(JNSTRICTEQNULL_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQNULL_IMM8);
    }
    HANDLE_OPCODE(JNSTRICTEQNULL_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQNULL_IMM16);
    }
    HANDLE_OPCODE(JEQUNDEFINED_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQUNDEFINED_IMM8);
    }
    HANDLE_OPCODE(JEQUNDEFINED_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQUNDEFINED_IMM16);
    }
    HANDLE_OPCODE(JNEUNDEFINED_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNEUNDEFINED_IMM8);
    }
    HANDLE_OPCODE(JNEUNDEFINED_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNEUNDEFINED_IMM16);
    }
    HANDLE_OPCODE(JSTRICTEQUNDEFINED_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQUNDEFINED_IMM8);
    }
    HANDLE_OPCODE(JSTRICTEQUNDEFINED_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQUNDEFINED_IMM16);
    }
    HANDLE_OPCODE(JNSTRICTEQUNDEFINED_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQUNDEFINED_IMM8);
    }
    HANDLE_OPCODE(JNSTRICTEQUNDEFINED_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQUNDEFINED_IMM16);
    }
    HANDLE_OPCODE(JEQ_V8_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQ_V8_IMM8);
    }
    HANDLE_OPCODE(JEQ_V8_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JEQ_V8_IMM16);
    }
    HANDLE_OPCODE(JNE_V8_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNE_V8_IMM8);
    }
    HANDLE_OPCODE(JNE_V8_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNE_V8_IMM16);
    }
    HANDLE_OPCODE(JSTRICTEQ_V8_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQ_V8_IMM8);
    }
    HANDLE_OPCODE(JSTRICTEQ_V8_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JSTRICTEQ_V8_IMM16);
    }
    HANDLE_OPCODE(JNSTRICTEQ_V8_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQ_V8_IMM8);
    }
    HANDLE_OPCODE(JNSTRICTEQ_V8_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(JNSTRICTEQ_V8_IMM16);
    }
    HANDLE_OPCODE(LDNEWTARGET) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDNEWTARGET);
    }
    HANDLE_OPCODE(LDTHIS) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDTHIS);
    }
    HANDLE_OPCODE(LDTHISBYVALUE_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDTHISBYVALUE_IMM8);
    }
    HANDLE_OPCODE(LDTHISBYVALUE_IMM16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDTHISBYVALUE_IMM16);
    }
    HANDLE_OPCODE(STTHISBYVALUE_IMM8_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STTHISBYVALUE_IMM8_V8);
    }
    HANDLE_OPCODE(STTHISBYVALUE_IMM16_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STTHISBYVALUE_IMM16_V8);
    }
    HANDLE_OPCODE(LDTHISBYNAME_IMM8_ID16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDTHISBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(LDTHISBYNAME_IMM16_ID16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDTHISBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(STTHISBYNAME_IMM8_ID16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STTHISBYNAME_IMM8_ID16);
    }
    HANDLE_OPCODE(STTHISBYNAME_IMM16_ID16) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STTHISBYNAME_IMM16_ID16);
    }
    HANDLE_OPCODE(LDPATCHVAR_IMM8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(LDPATCHVAR_IMM8);
    }
    HANDLE_OPCODE(STPATCHVAR_IMM8_V8) {
        LOG_FULL(FATAL) << "not implement";
        DISPATCH(STPATCHVAR_IMM8_V8);
    }
    HANDLE_OPCODE(WIDE_LDPATCHVAR_PREF_IMM16) {
        uint16_t index = READ_INST_16_1();
        LOG_INST() << "intrinsics::ldpatchvar" << " imm: " << index;

        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::LdPatchVar(thread, index);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(WIDE_LDPATCHVAR_PREF_IMM16);
    }
    HANDLE_OPCODE(WIDE_STPATCHVAR_PREF_IMM16) {
        uint16_t index = READ_INST_16_1();
        LOG_INST() << "intrinsics::stpatchvar" << " imm: " << index;
        JSTaggedValue value = GET_ACC();

        SAVE_ACC();
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::StPatchVar(thread, index, value);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        RESTORE_ACC();
        DISPATCH(WIDE_STPATCHVAR_PREF_IMM16);
    }
    HANDLE_OPCODE(DYNAMICIMPORT) {
        LOG_INST() << "intrinsics::dynamicimport";
        JSTaggedValue specifier = GET_ACC();
        JSTaggedValue thisFunc = GetThisFunction(sp);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::DynamicImport(thread, specifier, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DYNAMICIMPORT);
    }
    HANDLE_OPCODE(DEPRECATED_DYNAMICIMPORT_PREF_V8) {
        uint16_t v0 = READ_INST_8_1();
        LOG_INST() << "intrinsics::dynamicimport";
        JSTaggedValue specifier = GET_VREG_VALUE(v0);
        JSTaggedValue thisFunc = GetThisFunction(sp);
        SAVE_PC();
        JSTaggedValue res = SlowRuntimeStub::DynamicImport(thread, specifier, thisFunc);
        INTERPRETER_RETURN_IF_ABRUPT(res);
        SET_ACC(res);
        DISPATCH(DEPRECATED_DYNAMICIMPORT_PREF_V8);
    }
#include "templates/debugger_instruction_handler.inl"
}

void EcmaInterpreter::InitStackFrame(JSThread *thread)
{
    if (thread->IsAsmInterpreter()) {
        return InterpreterAssembly::InitStackFrame(thread);
    }
    JSTaggedType *prevSp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());
    InterpretedFrame *state = GET_FRAME(prevSp);
    state->pc = nullptr;
    state->function = JSTaggedValue::Hole();
    state->acc = JSTaggedValue::Hole();
    state->constpool = JSTaggedValue::Hole();
    state->profileTypeInfo = JSTaggedValue::Undefined();
    state->base.type = FrameType::INTERPRETER_FRAME;
    state->base.prev = nullptr;
}

uint32_t EcmaInterpreter::FindCatchBlock(Method *caller, uint32_t pc)
{
    auto *pandaFile = caller->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pandaFile, caller->GetMethodId());
    panda_file::CodeDataAccessor cda(*pandaFile, mda.GetCodeId().value());

    uint32_t pcOffset = panda_file::INVALID_OFFSET;
    cda.EnumerateTryBlocks([&pcOffset, pc](panda_file::CodeDataAccessor::TryBlock &try_block) {
        if ((try_block.GetStartPc() <= pc) && ((try_block.GetStartPc() + try_block.GetLength()) > pc)) {
            try_block.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
                pcOffset = catch_block.GetHandlerPc();
                return false;
            });
        }
        return pcOffset == panda_file::INVALID_OFFSET;
    });
    return pcOffset;
}

JSTaggedValue EcmaInterpreter::GetThisFunction(JSTaggedType *sp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    return state->function;
}

JSTaggedValue EcmaInterpreter::GetNewTarget(JSTaggedType *sp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    Method *method = JSFunction::Cast(state->function.GetTaggedObject())->GetCallTarget();
    ASSERT(method->HaveNewTargetWithCallField());
    uint32_t numVregs = method->GetNumVregsWithCallField();
    bool haveFunc = method->HaveFuncWithCallField();
    return JSTaggedValue(sp[numVregs + haveFunc]);
}

uint32_t EcmaInterpreter::GetNumArgs(JSTaggedType *sp, uint32_t restIdx, uint32_t &startIdx)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    Method *method = JSFunction::Cast(state->function.GetTaggedObject())->GetCallTarget();
    ASSERT(method->HaveExtraWithCallField());

    uint32_t numVregs = method->GetNumVregsWithCallField();
    bool haveFunc = method->HaveFuncWithCallField();
    bool haveNewTarget = method->HaveNewTargetWithCallField();
    bool haveThis = method->HaveThisWithCallField();
    uint32_t copyArgs = haveFunc + haveNewTarget + haveThis;
    uint32_t numArgs = method->GetNumArgsWithCallField();

    JSTaggedType *lastFrame = state->base.prev;
    // The prev frame of InterpretedFrame may entry frame or interpreter frame.
    if (FrameHandler::GetFrameType(state->base.prev) == FrameType::INTERPRETER_ENTRY_FRAME) {
        lastFrame = lastFrame - InterpretedEntryFrame::NumOfMembers();
    } else {
        lastFrame = lastFrame - InterpretedFrame::NumOfMembers();
    }

    if (static_cast<uint32_t>(lastFrame - sp) > numVregs + copyArgs + numArgs) {
        // In this case, actualNumArgs is in the end
        // If not, then actualNumArgs == declaredNumArgs, therefore do nothing
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        numArgs = static_cast<uint32_t>(JSTaggedValue(*(lastFrame - 1)).GetInt());
    }
    startIdx = numVregs + copyArgs + restIdx;
    return ((numArgs > restIdx) ? (numArgs - restIdx) : 0);
}

size_t EcmaInterpreter::GetJumpSizeAfterCall(const uint8_t *prevPc)
{
    auto op = BytecodeInstruction(prevPc).GetOpcode();
    size_t jumpSize = BytecodeInstruction::Size(op);
    return jumpSize;
}

JSTaggedValue EcmaInterpreter::GetRuntimeProfileTypeInfo(JSTaggedType *sp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    return state->profileTypeInfo;
}

JSTaggedValue EcmaInterpreter::GetConstantPool(JSTaggedType *sp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    return state->constpool;
}

bool EcmaInterpreter::UpdateHotnessCounter(JSThread* thread, JSTaggedType *sp, JSTaggedValue acc, int32_t offset)
{
    InterpretedFrame *state = GET_FRAME(sp);
    auto method = JSFunction::Cast(state->function.GetTaggedObject())->GetCallTarget();
    auto hotnessCounter = method->GetHotnessCounter();

    hotnessCounter += offset;
    if (UNLIKELY(hotnessCounter <= 0)) {
        bool needRestoreAcc = false;
        SAVE_ACC();
        needRestoreAcc = thread->CheckSafepoint();
        RESTORE_ACC();
        method = JSFunction::Cast(state->function.GetTaggedObject())->GetCallTarget();
        if (state->profileTypeInfo == JSTaggedValue::Undefined()) {
            state->acc = acc;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            auto thisFunc = JSFunction::Cast(state->function.GetTaggedObject());
            method = thisFunc->GetCallTarget(); // for CheckSafepoint, method need retrieve.
            method->SetHotnessCounter(EcmaInterpreter::METHOD_HOTNESS_THRESHOLD);
            auto res = SlowRuntimeStub::NotifyInlineCache(thread, thisFunc, method);
            state->profileTypeInfo = res;
            return true;
        } else {
            method->SetHotnessCounter(EcmaInterpreter::METHOD_HOTNESS_THRESHOLD);
            return needRestoreAcc;
        }
    }
    method->SetHotnessCounter(hotnessCounter);
    return false;
}

// only use for fast new, not universal API
JSTaggedValue EcmaInterpreter::GetThisObjectFromFastNewFrame(JSTaggedType *sp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    InterpretedFrame *state = reinterpret_cast<InterpretedFrame *>(sp) - 1;
    Method *method = ECMAObject::Cast(state->function.GetTaggedObject())->GetCallTarget();
    ASSERT(method->OnlyHaveThisWithCallField() || method->OnlyHaveNewTagetAndThisWithCallField());
    uint32_t numVregs = method->GetNumVregsWithCallField();
    uint32_t numDeclaredArgs;
    if (method->OnlyHaveThisWithCallField()) {
        numDeclaredArgs = method->GetNumArgsWithCallField() + 1;  // 1: explicit this object
    } else {
        numDeclaredArgs = method->GetNumArgsWithCallField() + 2;  // 2: newTarget and explicit this object
    }
    uint32_t hiddenThisObjectIndex = numVregs + numDeclaredArgs;   // hidden this object in the end of fast new frame
    return JSTaggedValue(sp[hiddenThisObjectIndex]);
}

bool EcmaInterpreter::IsFastNewFrameEnter(JSFunction *ctor, JSHandle<Method> method)
{
    if (method->IsNativeWithCallField()) {
        return false;
    }

    if (ctor->IsBase()) {
        return method->OnlyHaveThisWithCallField();
    }

    if (ctor->IsDerivedConstructor()) {
        return method->OnlyHaveNewTagetAndThisWithCallField();
    }

    return false;
}

bool EcmaInterpreter::IsFastNewFrameExit(JSTaggedType *sp)
{
    return GET_FRAME(sp)->base.type == FrameType::INTERPRETER_FAST_NEW_FRAME;
}

int16_t EcmaInterpreter::GetHotnessCounter(uint32_t codeSize)
{
    auto result = codeSize * METHOD_HOTNESS_THRESHOLD_FACTOR;
    return (result > METHOD_HOTNESS_THRESHOLD) ?
        METHOD_HOTNESS_THRESHOLD : static_cast<int16_t>(result);
}

std::string GetEcmaOpcodeStr(EcmaOpcode opcode)
{
    const std::map<EcmaOpcode, const char *> strMap = {
        {EcmaOpcode::LDNAN, "LDNAN"},
        {EcmaOpcode::LDINFINITY, "LDINFINITY"},
        {EcmaOpcode::LDUNDEFINED, "LDUNDEFINED"},
        {EcmaOpcode::LDNULL, "LDNULL"},
        {EcmaOpcode::LDSYMBOL, "LDSYMBOL"},
        {EcmaOpcode::LDGLOBAL, "LDGLOBAL"},
        {EcmaOpcode::LDTRUE, "LDTRUE"},
        {EcmaOpcode::LDFALSE, "LDFALSE"},
        {EcmaOpcode::LDHOLE, "LDHOLE"},
        {EcmaOpcode::LDNEWTARGET, "LDNEWTARGET"},
        {EcmaOpcode::POPLEXENV, "POPLEXENV"},
        {EcmaOpcode::GETUNMAPPEDARGS, "GETUNMAPPEDARGS"},
        {EcmaOpcode::ASYNCFUNCTIONENTER, "ASYNCFUNCTIONENTER"},
        {EcmaOpcode::LDTHIS, "LDTHIS"},
        {EcmaOpcode::LDFUNCTION, "LDFUNCTION"},
        {EcmaOpcode::DEBUGGER, "DEBUGGER"},
        {EcmaOpcode::GETPROPITERATOR, "GETPROPITERATOR"},
        {EcmaOpcode::GETITERATOR_IMM8, "GETITERATOR_IMM8"},
        {EcmaOpcode::GETITERATOR_IMM16, "GETITERATOR_IMM16"},
        {EcmaOpcode::CLOSEITERATOR_IMM8_V8, "CLOSEITERATOR_IMM8_V8"},
        {EcmaOpcode::CLOSEITERATOR_IMM16_V8, "CLOSEITERATOR_IMM16_V8"},
        {EcmaOpcode::CREATEASYNCGENERATOROBJ_V8, "CREATEASYNCGENERATOROBJ_V8"},
        {EcmaOpcode::CREATEEMPTYOBJECT, "CREATEEMPTYOBJECT"},
        {EcmaOpcode::CREATEEMPTYARRAY_IMM8, "CREATEEMPTYARRAY_IMM8"},
        {EcmaOpcode::CREATEEMPTYARRAY_IMM16, "CREATEEMPTYARRAY_IMM16"},
        {EcmaOpcode::CREATEGENERATOROBJ_V8, "CREATEGENERATOROBJ_V8"},
        {EcmaOpcode::CREATEITERRESULTOBJ_V8_V8, "CREATEITERRESULTOBJ_V8_V8"},
        {EcmaOpcode::CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8, "CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8"},
        {EcmaOpcode::ASYNCGENERATORRESOLVE_V8_V8_V8, "ASYNCGENERATORRESOLVE_V8_V8_V8"},
        {EcmaOpcode::CREATEARRAYWITHBUFFER_IMM8_ID16, "CREATEARRAYWITHBUFFER_IMM8_ID16"},
        {EcmaOpcode::CREATEARRAYWITHBUFFER_IMM16_ID16, "CREATEARRAYWITHBUFFER_IMM16_ID16"},
        {EcmaOpcode::CALLTHIS0_IMM8_V8, "CALLTHIS0_IMM8_V8"},
        {EcmaOpcode::CALLTHIS1_IMM8_V8_V8, "CALLTHIS1_IMM8_V8_V8"},
        {EcmaOpcode::CREATEOBJECTWITHBUFFER_IMM8_ID16, "CREATEOBJECTWITHBUFFER_IMM8_ID16"},
        {EcmaOpcode::CREATEOBJECTWITHBUFFER_IMM16_ID16, "CREATEOBJECTWITHBUFFER_IMM16_ID16"},
        {EcmaOpcode::CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8, "CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8"},
        {EcmaOpcode::CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8, "CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8"},
        {EcmaOpcode::NEWOBJAPPLY_IMM8_V8, "NEWOBJAPPLY_IMM8_V8"},
        {EcmaOpcode::NEWOBJAPPLY_IMM16_V8, "NEWOBJAPPLY_IMM16_V8"},
        {EcmaOpcode::NEWOBJRANGE_IMM8_IMM8_V8, "NEWOBJRANGE_IMM8_IMM8_V8"},
        {EcmaOpcode::NEWOBJRANGE_IMM16_IMM8_V8, "NEWOBJRANGE_IMM16_IMM8_V8"},
        {EcmaOpcode::NEWLEXENV_IMM8, "NEWLEXENV_IMM8"},
        {EcmaOpcode::NEWLEXENVWITHNAME_IMM8_ID16, "NEWLEXENVWITHNAME_IMM8_ID16"},
        {EcmaOpcode::ADD2_IMM8_V8, "ADD2_IMM8_V8"},
        {EcmaOpcode::SUB2_IMM8_V8, "SUB2_IMM8_V8"},
        {EcmaOpcode::MUL2_IMM8_V8, "MUL2_IMM8_V8"},
        {EcmaOpcode::DIV2_IMM8_V8, "DIV2_IMM8_V8"},
        {EcmaOpcode::MOD2_IMM8_V8, "MOD2_IMM8_V8"},
        {EcmaOpcode::EQ_IMM8_V8, "EQ_IMM8_V8"},
        {EcmaOpcode::NOTEQ_IMM8_V8, "NOTEQ_IMM8_V8"},
        {EcmaOpcode::LESS_IMM8_V8, "LESS_IMM8_V8"},
        {EcmaOpcode::LESSEQ_IMM8_V8, "LESSEQ_IMM8_V8"},
        {EcmaOpcode::GREATER_IMM8_V8, "GREATER_IMM8_V8"},
        {EcmaOpcode::GREATEREQ_IMM8_V8, "GREATEREQ_IMM8_V8"},
        {EcmaOpcode::SHL2_IMM8_V8, "SHL2_IMM8_V8"},
        {EcmaOpcode::SHR2_IMM8_V8, "SHR2_IMM8_V8"},
        {EcmaOpcode::ASHR2_IMM8_V8, "ASHR2_IMM8_V8"},
        {EcmaOpcode::AND2_IMM8_V8, "AND2_IMM8_V8"},
        {EcmaOpcode::OR2_IMM8_V8, "OR2_IMM8_V8"},
        {EcmaOpcode::XOR2_IMM8_V8, "XOR2_IMM8_V8"},
        {EcmaOpcode::EXP_IMM8_V8, "EXP_IMM8_V8"},
        {EcmaOpcode::TYPEOF_IMM8, "TYPEOF_IMM8"},
        {EcmaOpcode::TYPEOF_IMM16, "TYPEOF_IMM16"},
        {EcmaOpcode::TONUMBER_IMM8, "TONUMBER_IMM8"},
        {EcmaOpcode::TONUMERIC_IMM8, "TONUMERIC_IMM8"},
        {EcmaOpcode::NEG_IMM8, "NEG_IMM8"},
        {EcmaOpcode::NOT_IMM8, "NOT_IMM8"},
        {EcmaOpcode::INC_IMM8, "INC_IMM8"},
        {EcmaOpcode::DEC_IMM8, "DEC_IMM8"},
        {EcmaOpcode::ISIN_IMM8_V8, "ISIN_IMM8_V8"},
        {EcmaOpcode::INSTANCEOF_IMM8_V8, "INSTANCEOF_IMM8_V8"},
        {EcmaOpcode::STRICTNOTEQ_IMM8_V8, "STRICTNOTEQ_IMM8_V8"},
        {EcmaOpcode::STRICTEQ_IMM8_V8, "STRICTEQ_IMM8_V8"},
        {EcmaOpcode::ISTRUE, "ISTRUE"},
        {EcmaOpcode::ISFALSE, "ISFALSE"},
        {EcmaOpcode::CALLTHIS2_IMM8_V8_V8_V8, "CALLTHIS2_IMM8_V8_V8_V8"},
        {EcmaOpcode::CALLTHIS3_IMM8_V8_V8_V8_V8, "CALLTHIS3_IMM8_V8_V8_V8_V8"},
        {EcmaOpcode::CALLTHISRANGE_IMM8_IMM8_V8, "CALLTHISRANGE_IMM8_IMM8_V8"},
        {EcmaOpcode::SUPERCALLTHISRANGE_IMM8_IMM8_V8, "SUPERCALLTHISRANGE_IMM8_IMM8_V8"},
        {EcmaOpcode::SUPERCALLARROWRANGE_IMM8_IMM8_V8, "SUPERCALLARROWRANGE_IMM8_IMM8_V8"},
        {EcmaOpcode::DEFINEFUNC_IMM8_ID16_IMM8, "DEFINEFUNC_IMM8_ID16_IMM8"},
        {EcmaOpcode::DEFINEFUNC_IMM16_ID16_IMM8, "DEFINEFUNC_IMM16_ID16_IMM8"},
        {EcmaOpcode::DEFINEMETHOD_IMM8_ID16_IMM8, "DEFINEMETHOD_IMM8_ID16_IMM8"},
        {EcmaOpcode::CALLARG0_IMM8, "CALLARG0_IMM8"},
        {EcmaOpcode::SUPERCALLSPREAD_IMM8_V8, "SUPERCALLSPREAD_IMM8_V8"},
        {EcmaOpcode::APPLY_IMM8_V8_V8, "APPLY_IMM8_V8_V8"},
        {EcmaOpcode::CALLARGS2_IMM8_V8_V8, "CALLARGS2_IMM8_V8_V8"},
        {EcmaOpcode::CALLARGS3_IMM8_V8_V8_V8, "CALLARGS3_IMM8_V8_V8_V8"},
        {EcmaOpcode::CALLRANGE_IMM8_IMM8_V8, "CALLRANGE_IMM8_IMM8_V8"},
        {EcmaOpcode::DEFINEMETHOD_IMM16_ID16_IMM8, "DEFINEMETHOD_IMM16_ID16_IMM8"},
        {EcmaOpcode::LDEXTERNALMODULEVAR_IMM8, "LDEXTERNALMODULEVAR_IMM8"},
        {EcmaOpcode::DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8, "DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8"},
        {EcmaOpcode::LDTHISBYNAME_IMM8_ID16, "LDTHISBYNAME_IMM8_ID16"},
        {EcmaOpcode::LDTHISBYNAME_IMM16_ID16, "LDTHISBYNAME_IMM16_ID16"},
        {EcmaOpcode::STTHISBYNAME_IMM8_ID16, "STTHISBYNAME_IMM8_ID16"},
        {EcmaOpcode::STTHISBYNAME_IMM16_ID16, "STTHISBYNAME_IMM16_ID16"},
        {EcmaOpcode::LDTHISBYVALUE_IMM8, "LDTHISBYVALUE_IMM8"},
        {EcmaOpcode::LDTHISBYVALUE_IMM16, "LDTHISBYVALUE_IMM16"},
        {EcmaOpcode::STTHISBYVALUE_IMM8_V8, "STTHISBYVALUE_IMM8_V8"},
        {EcmaOpcode::STTHISBYVALUE_IMM16_V8, "STTHISBYVALUE_IMM16_V8"},
        {EcmaOpcode::LDPATCHVAR_IMM8, "LDPATCHVAR_IMM8"},
        {EcmaOpcode::STPATCHVAR_IMM8_V8, "STPATCHVAR_IMM8_V8"},
        {EcmaOpcode::DYNAMICIMPORT, "DYNAMICIMPORT"},
        {EcmaOpcode::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8, "DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8"},
        {EcmaOpcode::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8, "DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8"},
        {EcmaOpcode::RESUMEGENERATOR, "RESUMEGENERATOR"},
        {EcmaOpcode::GETRESUMEMODE, "GETRESUMEMODE"},
        {EcmaOpcode::GETTEMPLATEOBJECT_IMM8, "GETTEMPLATEOBJECT_IMM8"},
        {EcmaOpcode::GETTEMPLATEOBJECT_IMM16, "GETTEMPLATEOBJECT_IMM16"},
        {EcmaOpcode::GETNEXTPROPNAME_V8, "GETNEXTPROPNAME_V8"},
        {EcmaOpcode::JSTRICTEQZ_IMM8, "JSTRICTEQZ_IMM8"},
        {EcmaOpcode::JSTRICTEQZ_IMM16, "JSTRICTEQZ_IMM16"},
        {EcmaOpcode::SETOBJECTWITHPROTO_IMM8_V8, "SETOBJECTWITHPROTO_IMM8_V8"},
        {EcmaOpcode::DELOBJPROP_V8, "DELOBJPROP_V8"},
        {EcmaOpcode::SUSPENDGENERATOR_V8, "SUSPENDGENERATOR_V8"},
        {EcmaOpcode::ASYNCFUNCTIONAWAITUNCAUGHT_V8, "ASYNCFUNCTIONAWAITUNCAUGHT_V8"},
        {EcmaOpcode::COPYDATAPROPERTIES_V8, "COPYDATAPROPERTIES_V8"},
        {EcmaOpcode::STARRAYSPREAD_V8_V8, "STARRAYSPREAD_V8_V8"},
        {EcmaOpcode::SETOBJECTWITHPROTO_IMM16_V8, "SETOBJECTWITHPROTO_IMM16_V8"},
        {EcmaOpcode::LDOBJBYVALUE_IMM8_V8, "LDOBJBYVALUE_IMM8_V8"},
        {EcmaOpcode::LDOBJBYVALUE_IMM16_V8, "LDOBJBYVALUE_IMM16_V8"},
        {EcmaOpcode::STOBJBYVALUE_IMM8_V8_V8, "STOBJBYVALUE_IMM8_V8_V8"},
        {EcmaOpcode::STOBJBYVALUE_IMM16_V8_V8, "STOBJBYVALUE_IMM16_V8_V8"},
        {EcmaOpcode::STOWNBYVALUE_IMM8_V8_V8, "STOWNBYVALUE_IMM8_V8_V8"},
        {EcmaOpcode::STOWNBYVALUE_IMM16_V8_V8, "STOWNBYVALUE_IMM16_V8_V8"},
        {EcmaOpcode::LDSUPERBYVALUE_IMM8_V8, "LDSUPERBYVALUE_IMM8_V8"},
        {EcmaOpcode::LDSUPERBYVALUE_IMM16_V8, "LDSUPERBYVALUE_IMM16_V8"},
        {EcmaOpcode::STSUPERBYVALUE_IMM8_V8_V8, "STSUPERBYVALUE_IMM8_V8_V8"},
        {EcmaOpcode::STSUPERBYVALUE_IMM16_V8_V8, "STSUPERBYVALUE_IMM16_V8_V8"},
        {EcmaOpcode::LDOBJBYINDEX_IMM8_IMM16, "LDOBJBYINDEX_IMM8_IMM16"},
        {EcmaOpcode::LDOBJBYINDEX_IMM16_IMM16, "LDOBJBYINDEX_IMM16_IMM16"},
        {EcmaOpcode::STOBJBYINDEX_IMM8_V8_IMM16, "STOBJBYINDEX_IMM8_V8_IMM16"},
        {EcmaOpcode::STOBJBYINDEX_IMM16_V8_IMM16, "STOBJBYINDEX_IMM16_V8_IMM16"},
        {EcmaOpcode::STOWNBYINDEX_IMM8_V8_IMM16, "STOWNBYINDEX_IMM8_V8_IMM16"},
        {EcmaOpcode::STOWNBYINDEX_IMM16_V8_IMM16, "STOWNBYINDEX_IMM16_V8_IMM16"},
        {EcmaOpcode::ASYNCFUNCTIONRESOLVE_V8, "ASYNCFUNCTIONRESOLVE_V8"},
        {EcmaOpcode::ASYNCFUNCTIONREJECT_V8, "ASYNCFUNCTIONREJECT_V8"},
        {EcmaOpcode::COPYRESTARGS_IMM8, "COPYRESTARGS_IMM8"},
        {EcmaOpcode::LDLEXVAR_IMM4_IMM4, "LDLEXVAR_IMM4_IMM4"},
        {EcmaOpcode::STLEXVAR_IMM4_IMM4, "STLEXVAR_IMM4_IMM4"},
        {EcmaOpcode::GETMODULENAMESPACE_IMM8, "GETMODULENAMESPACE_IMM8"},
        {EcmaOpcode::STMODULEVAR_IMM8, "STMODULEVAR_IMM8"},
        {EcmaOpcode::TRYLDGLOBALBYNAME_IMM8_ID16, "TRYLDGLOBALBYNAME_IMM8_ID16"},
        {EcmaOpcode::TRYLDGLOBALBYNAME_IMM16_ID16, "TRYLDGLOBALBYNAME_IMM16_ID16"},
        {EcmaOpcode::TRYSTGLOBALBYNAME_IMM8_ID16, "TRYSTGLOBALBYNAME_IMM8_ID16"},
        {EcmaOpcode::TRYSTGLOBALBYNAME_IMM16_ID16, "TRYSTGLOBALBYNAME_IMM16_ID16"},
        {EcmaOpcode::LDGLOBALVAR_IMM16_ID16, "LDGLOBALVAR_IMM16_ID16"},
        {EcmaOpcode::STGLOBALVAR_IMM16_ID16, "STGLOBALVAR_IMM16_ID16"},
        {EcmaOpcode::LDOBJBYNAME_IMM8_ID16, "LDOBJBYNAME_IMM8_ID16"},
        {EcmaOpcode::LDOBJBYNAME_IMM16_ID16, "LDOBJBYNAME_IMM16_ID16"},
        {EcmaOpcode::STOBJBYNAME_IMM8_ID16_V8, "STOBJBYNAME_IMM8_ID16_V8"},
        {EcmaOpcode::STOBJBYNAME_IMM16_ID16_V8, "STOBJBYNAME_IMM16_ID16_V8"},
        {EcmaOpcode::STOWNBYNAME_IMM8_ID16_V8, "STOWNBYNAME_IMM8_ID16_V8"},
        {EcmaOpcode::STOWNBYNAME_IMM16_ID16_V8, "STOWNBYNAME_IMM16_ID16_V8"},
        {EcmaOpcode::LDSUPERBYNAME_IMM8_ID16, "LDSUPERBYNAME_IMM8_ID16"},
        {EcmaOpcode::LDSUPERBYNAME_IMM16_ID16, "LDSUPERBYNAME_IMM16_ID16"},
        {EcmaOpcode::STSUPERBYNAME_IMM8_ID16_V8, "STSUPERBYNAME_IMM8_ID16_V8"},
        {EcmaOpcode::STSUPERBYNAME_IMM16_ID16_V8, "STSUPERBYNAME_IMM16_ID16_V8"},
        {EcmaOpcode::LDLOCALMODULEVAR_IMM8, "LDLOCALMODULEVAR_IMM8"},
        {EcmaOpcode::STCONSTTOGLOBALRECORD_IMM16_ID16, "STCONSTTOGLOBALRECORD_IMM16_ID16"},
        {EcmaOpcode::STTOGLOBALRECORD_IMM16_ID16, "STTOGLOBALRECORD_IMM16_ID16"},
        {EcmaOpcode::JNSTRICTEQZ_IMM8, "JNSTRICTEQZ_IMM8"},
        {EcmaOpcode::JNSTRICTEQZ_IMM16, "JNSTRICTEQZ_IMM16"},
        {EcmaOpcode::JEQNULL_IMM8, "JEQNULL_IMM8"},
        {EcmaOpcode::JEQNULL_IMM16, "JEQNULL_IMM16"},
        {EcmaOpcode::STOWNBYVALUEWITHNAMESET_IMM8_V8_V8, "STOWNBYVALUEWITHNAMESET_IMM8_V8_V8"},
        {EcmaOpcode::STOWNBYVALUEWITHNAMESET_IMM16_V8_V8, "STOWNBYVALUEWITHNAMESET_IMM16_V8_V8"},
        {EcmaOpcode::STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8, "STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8"},
        {EcmaOpcode::STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8, "STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8"},
        {EcmaOpcode::LDBIGINT_ID16, "LDBIGINT_ID16"},
        {EcmaOpcode::LDA_STR_ID16, "LDA_STR_ID16"},
        {EcmaOpcode::JMP_IMM8, "JMP_IMM8"},
        {EcmaOpcode::JMP_IMM16, "JMP_IMM16"},
        {EcmaOpcode::JMP_IMM32, "JMP_IMM32"},
        {EcmaOpcode::JEQZ_IMM8, "JEQZ_IMM8"},
        {EcmaOpcode::JEQZ_IMM16, "JEQZ_IMM16"},
        {EcmaOpcode::JNEZ_IMM8, "JNEZ_IMM8"},
        {EcmaOpcode::JNEZ_IMM16, "JNEZ_IMM16"},
        {EcmaOpcode::JNENULL_IMM8, "JNENULL_IMM8"},
        {EcmaOpcode::JNENULL_IMM16, "JNENULL_IMM16"},
        {EcmaOpcode::LDA_V8, "LDA_V8"},
        {EcmaOpcode::STA_V8, "STA_V8"},
        {EcmaOpcode::LDAI_IMM32, "LDAI_IMM32"},
        {EcmaOpcode::FLDAI_IMM64, "FLDAI_IMM64"},
        {EcmaOpcode::RETURN, "RETURN"},
        {EcmaOpcode::RETURNUNDEFINED, "RETURNUNDEFINED"},
        {EcmaOpcode::LDLEXVAR_IMM8_IMM8, "LDLEXVAR_IMM8_IMM8"},
        {EcmaOpcode::JSTRICTEQNULL_IMM8, "JSTRICTEQNULL_IMM8"},
        {EcmaOpcode::STLEXVAR_IMM8_IMM8, "STLEXVAR_IMM8_IMM8"},
        {EcmaOpcode::JSTRICTEQNULL_IMM16, "JSTRICTEQNULL_IMM16"},
        {EcmaOpcode::CALLARG1_IMM8_V8, "CALLARG1_IMM8_V8"},
        {EcmaOpcode::JNSTRICTEQNULL_IMM8, "JNSTRICTEQNULL_IMM8"},
        {EcmaOpcode::JNSTRICTEQNULL_IMM16, "JNSTRICTEQNULL_IMM16"},
        {EcmaOpcode::JEQUNDEFINED_IMM8, "JEQUNDEFINED_IMM8"},
        {EcmaOpcode::JEQUNDEFINED_IMM16, "JEQUNDEFINED_IMM16"},
        {EcmaOpcode::JNEUNDEFINED_IMM8, "JNEUNDEFINED_IMM8"},
        {EcmaOpcode::JNEUNDEFINED_IMM16, "JNEUNDEFINED_IMM16"},
        {EcmaOpcode::JSTRICTEQUNDEFINED_IMM8, "JSTRICTEQUNDEFINED_IMM8"},
        {EcmaOpcode::JSTRICTEQUNDEFINED_IMM16, "JSTRICTEQUNDEFINED_IMM16"},
        {EcmaOpcode::JNSTRICTEQUNDEFINED_IMM8, "JNSTRICTEQUNDEFINED_IMM8"},
        {EcmaOpcode::JNSTRICTEQUNDEFINED_IMM16, "JNSTRICTEQUNDEFINED_IMM16"},
        {EcmaOpcode::JEQ_V8_IMM8, "JEQ_V8_IMM8"},
        {EcmaOpcode::JEQ_V8_IMM16, "JEQ_V8_IMM16"},
        {EcmaOpcode::JNE_V8_IMM8, "JNE_V8_IMM8"},
        {EcmaOpcode::JNE_V8_IMM16, "JNE_V8_IMM16"},
        {EcmaOpcode::JSTRICTEQ_V8_IMM8, "JSTRICTEQ_V8_IMM8"},
        {EcmaOpcode::JSTRICTEQ_V8_IMM16, "JSTRICTEQ_V8_IMM16"},
        {EcmaOpcode::JNSTRICTEQ_V8_IMM8, "JNSTRICTEQ_V8_IMM8"},
        {EcmaOpcode::JNSTRICTEQ_V8_IMM16, "JNSTRICTEQ_V8_IMM16"},
        {EcmaOpcode::MOV_V4_V4, "MOV_V4_V4"},
        {EcmaOpcode::MOV_V8_V8, "MOV_V8_V8"},
        {EcmaOpcode::MOV_V16_V16, "MOV_V16_V16"},
        {EcmaOpcode::NOP, "NOP"},
        {static_cast<EcmaOpcode>(EXCEPTION_OPCODE), "EXCEPTION_OPCODE"},
        {EcmaOpcode::DEPRECATED_LDLEXENV_PREF_NONE, "DEPRECATED_LDLEXENV"},
        {EcmaOpcode::WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8, "WIDE_CREATEOBJECTWITHEXCLUDEDKEYS"},
        {EcmaOpcode::THROW_PREF_NONE, "THROW_PREF"},
        {EcmaOpcode::DEPRECATED_POPLEXENV_PREF_NONE, "DEPRECATED_POPLEXENV"},
        {EcmaOpcode::WIDE_NEWOBJRANGE_PREF_IMM16_V8, "WIDE_NEWOBJRANGE"},
        {EcmaOpcode::THROW_NOTEXISTS_PREF_NONE, "THROW_NOTEXISTS"},
        {EcmaOpcode::DEPRECATED_GETITERATORNEXT_PREF_V8_V8, "DEPRECATED_GETITERATORNEXT"},
        {EcmaOpcode::WIDE_NEWLEXENV_PREF_IMM16, "WIDE_NEWLEXENV"},
        {EcmaOpcode::THROW_PATTERNNONCOERCIBLE_PREF_NONE, "THROW_PATTERNNONCOERCIBLE"},
        {EcmaOpcode::DEPRECATED_CREATEARRAYWITHBUFFER_PREF_IMM16, "DEPRECATED_CREATEARRAYWITHBUFFER"},
        {EcmaOpcode::WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16, "WIDE_NEWLEXENVWITHNAME"},
        {EcmaOpcode::THROW_DELETESUPERPROPERTY_PREF_NONE, "THROW_DELETESUPERPROPERTY"},
        {EcmaOpcode::DEPRECATED_CREATEOBJECTWITHBUFFER_PREF_IMM16, "DEPRECATED_CREATEOBJECTWITHBUFFER"},
        {EcmaOpcode::WIDE_CALLRANGE_PREF_IMM16_V8, "WIDE_CALLRANGE"},
        {EcmaOpcode::THROW_CONSTASSIGNMENT_PREF_V8, "THROW_CONSTASSIGNMENT"},
        {EcmaOpcode::DEPRECATED_TONUMBER_PREF_V8, "DEPRECATED_TONUMBER"},
        {EcmaOpcode::WIDE_CALLTHISRANGE_PREF_IMM16_V8, "WIDE_CALLTHISRANGE"},
        {EcmaOpcode::THROW_IFNOTOBJECT_PREF_V8, "THROW_IFNOTOBJECT"},
        {EcmaOpcode::DEPRECATED_TONUMERIC_PREF_V8, "DEPRECATED_TONUMERIC"},
        {EcmaOpcode::WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8, "WIDE_SUPERCALLTHISRANGE"},
        {EcmaOpcode::THROW_UNDEFINEDIFHOLE_PREF_V8_V8, "THROW_UNDEFINEDIFHOLE"},
        {EcmaOpcode::DEPRECATED_NEG_PREF_V8, "DEPRECATED_NEG"},
        {EcmaOpcode::WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8, "WIDE_SUPERCALLARROWRANGE"},
        {EcmaOpcode::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8, "THROW_IFSUPERNOTCORRECTCALL"},
        {EcmaOpcode::DEPRECATED_NOT_PREF_V8, "DEPRECATED_NOT"},
        {EcmaOpcode::WIDE_LDOBJBYINDEX_PREF_IMM32, "WIDE_LDOBJBYINDEX"},
        {EcmaOpcode::THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16, "THROW_IFSUPERNOTCORRECTCALL"},
        {EcmaOpcode::DEPRECATED_INC_PREF_V8, "DEPRECATED_INC"},
        {EcmaOpcode::WIDE_STOBJBYINDEX_PREF_V8_IMM32, "WIDE_STOBJBYINDEX"},
        {EcmaOpcode::DEPRECATED_DEC_PREF_V8, "DEPRECATED_DEC"},
        {EcmaOpcode::WIDE_STOWNBYINDEX_PREF_V8_IMM32, "WIDE_STOWNBYINDEX"},
        {EcmaOpcode::DEPRECATED_CALLARG0_PREF_V8, "DEPRECATED_CALLARG0"},
        {EcmaOpcode::WIDE_COPYRESTARGS_PREF_IMM16, "WIDE_COPYRESTARGS"},
        {EcmaOpcode::DEPRECATED_CALLARG1_PREF_V8_V8, "DEPRECATED_CALLARG1"},
        {EcmaOpcode::WIDE_LDLEXVAR_PREF_IMM16_IMM16, "WIDE_LDLEXVAR"},
        {EcmaOpcode::DEPRECATED_CALLARGS2_PREF_V8_V8_V8, "DEPRECATED_CALLARGS2"},
        {EcmaOpcode::WIDE_STLEXVAR_PREF_IMM16_IMM16, "WIDE_STLEXVAR"},
        {EcmaOpcode::DEPRECATED_CALLARGS3_PREF_V8_V8_V8_V8, "DEPRECATED_CALLARGS3"},
        {EcmaOpcode::WIDE_GETMODULENAMESPACE_PREF_IMM16, "WIDE_GETMODULENAMESPACE"},
        {EcmaOpcode::DEPRECATED_CALLSPREAD_PREF_V8_V8_V8, "DEPRECATED_CALLSPREAD"},
        {EcmaOpcode::WIDE_STMODULEVAR_PREF_IMM16, "WIDE_STMODULEVAR"},
        {EcmaOpcode::WIDE_LDLOCALMODULEVAR_PREF_IMM16, "WIDE_LDLOCALMODULEVAR"},
        {EcmaOpcode::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16, "WIDE_LDEXTERNALMODULEVAR"},
        {EcmaOpcode::WIDE_LDPATCHVAR_PREF_IMM16, "LDPATCHVAR"},
        {EcmaOpcode::WIDE_STPATCHVAR_PREF_IMM16, "STPATCHVAR"},
        {EcmaOpcode::DEPRECATED_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8, "DEPRECATED_DEFINECLASSWITHBUFFER"},
        {EcmaOpcode::DEPRECATED_RESUMEGENERATOR_PREF_V8, "DEPRECATED_RESUMEGENERATOR"},
        {EcmaOpcode::DEPRECATED_GETRESUMEMODE_PREF_V8, "DEPRECATED_GETRESUMEMODE"},
        {EcmaOpcode::DEPRECATED_GETTEMPLATEOBJECT_PREF_V8, "DEPRECATED_GETTEMPLATEOBJECT"},
        {EcmaOpcode::DEPRECATED_DELOBJPROP_PREF_V8_V8, "DEPRECATED_DELOBJPROP"},
        {EcmaOpcode::DEPRECATED_SUSPENDGENERATOR_PREF_V8_V8, "DEPRECATED_SUSPENDGENERATOR"},
        {EcmaOpcode::DEPRECATED_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8, "DEPRECATED_ASYNCFUNCTIONAWAITUNCAUGHT"},
        {EcmaOpcode::DEPRECATED_COPYDATAPROPERTIES_PREF_V8_V8, "DEPRECATED_COPYDATAPROPERTIES"},
        {EcmaOpcode::DEPRECATED_SETOBJECTWITHPROTO_PREF_V8_V8, "DEPRECATED_SETOBJECTWITHPROTO"},
        {EcmaOpcode::DEPRECATED_LDOBJBYVALUE_PREF_V8_V8, "DEPRECATED_LDOBJBYVALUE"},
        {EcmaOpcode::DEPRECATED_LDSUPERBYVALUE_PREF_V8_V8, "DEPRECATED_LDSUPERBYVALUE"},
        {EcmaOpcode::DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32, "DEPRECATED_LDOBJBYINDEX"},
        {EcmaOpcode::DEPRECATED_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8, "DEPRECATED_ASYNCFUNCTIONRESOLVE"},
        {EcmaOpcode::DEPRECATED_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8, "DEPRECATED_ASYNCFUNCTIONREJECT"},
        {EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM4_IMM4_V8, "DEPRECATED_STLEXVAR"},
        {EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM8_IMM8_V8, "DEPRECATED_STLEXVAR"},
        {EcmaOpcode::DEPRECATED_STLEXVAR_PREF_IMM16_IMM16_V8, "DEPRECATED_STLEXVAR"},
        {EcmaOpcode::DEPRECATED_GETMODULENAMESPACE_PREF_ID32, "DEPRECATED_GETMODULENAMESPACE"},
        {EcmaOpcode::DEPRECATED_STMODULEVAR_PREF_ID32, "DEPRECATED_STMODULEVAR"},
        {EcmaOpcode::DEPRECATED_LDOBJBYNAME_PREF_ID32_V8, "DEPRECATED_LDOBJBYNAME"},
        {EcmaOpcode::DEPRECATED_LDSUPERBYNAME_PREF_ID32_V8, "DEPRECATED_LDSUPERBYNAME"},
        {EcmaOpcode::DEPRECATED_LDMODULEVAR_PREF_ID32_IMM8, "DEPRECATED_LDMODULEVAR"},
        {EcmaOpcode::DEPRECATED_STCONSTTOGLOBALRECORD_PREF_ID32, "DEPRECATED_STCONSTTOGLOBALRECORD"},
        {EcmaOpcode::DEPRECATED_STLETTOGLOBALRECORD_PREF_ID32, "DEPRECATED_STLETTOGLOBALRECORD"},
        {EcmaOpcode::DEPRECATED_STCLASSTOGLOBALRECORD_PREF_ID32, "DEPRECATED_STCLASSTOGLOBALRECORD"},
    };
    if (strMap.count(opcode) > 0) {
        return strMap.at(opcode);
    }
    return "bytecode-" + std::to_string(static_cast<uint16_t>(opcode));
}
#undef LOG_INST
#undef HANDLE_OPCODE
#undef ADVANCE_PC
#undef GOTO_NEXT
#undef DISPATCH
#undef DISPATCH_OFFSET
#undef GET_FRAME
#undef GET_ENTRY_FRAME
#undef SAVE_PC
#undef SAVE_ACC
#undef RESTORE_ACC
#undef INTERPRETER_GOTO_EXCEPTION_HANDLER
#undef CHECK_SWITCH_TO_DEBUGGER_TABLE
#undef REAL_GOTO_DISPATCH_OPCODE
#undef REAL_GOTO_EXCEPTION_HANDLER
#undef INTERPRETER_RETURN_IF_ABRUPT
#undef NOTIFY_DEBUGGER_EVENT
#undef DEPRECATED_CALL_INITIALIZE
#undef CALL_PUSH_UNDEFINED
#undef DEPRECATED_CALL_PUSH_ARGS_0
#undef DEPRECATED_CALL_PUSH_ARGS_1
#undef DEPRECATED_CALL_PUSH_ARGS_2
#undef DEPRECATED_CALL_PUSH_ARGS_3
#undef DEPRECATED_CALL_PUSH_ARGS_RANGE
#undef DEPRECATED_CALL_PUSH_ARGS_THISRANGE
#undef DEPRECATED_CALL_PUSH_ARGS_0_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS_1_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS_2_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS_3_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS_RANGE_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS_THISRANGE_NO_EXTRA
#undef DEPRECATED_CALL_PUSH_ARGS
#undef UPDATE_HOTNESS_COUNTER_NON_ACC
#undef UPDATE_HOTNESS_COUNTER
#undef GET_VREG
#undef GET_VREG_VALUE
#undef SET_VREG
#undef GET_ACC
#undef SET_ACC
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_INTERPRETER_INTERPRETER_INL_H
