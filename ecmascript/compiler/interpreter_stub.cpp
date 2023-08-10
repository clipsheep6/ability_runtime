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

#include <initializer_list>
#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/access_object_stub_builder.h"
#include "ecmascript/compiler/bc_call_signature.h"
#include "ecmascript/compiler/ic_stub_builder.h"
#include "ecmascript/compiler/interpreter_stub-inl.h"
#include "ecmascript/compiler/llvm_ir_builder.h"
#include "ecmascript/compiler/new_object_stub_builder.h"
#include "ecmascript/compiler/operations_stub_builder.h"
#include "ecmascript/compiler/profiler_stub_builder.h"
#include "ecmascript/compiler/stub_builder-inl.h"
#include "ecmascript/compiler/variable_type.h"
#include "ecmascript/global_env_constants.h"
#include "ecmascript/ic/profile_type_info.h"
#include "ecmascript/interpreter/interpreter_assembly.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_generator_object.h"
#include "ecmascript/message_string.h"
#include "ecmascript/tagged_hash_table.h"
#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::ecmascript::kungfu {
#define DECLARE_ASM_HANDLER_BASE(name, needPrint, V)                                              \
void name##StubBuilder::GenerateCircuit()                                                         \
{                                                                                                 \
    GateRef glue = PtrArgument(static_cast<size_t>(InterpreterHandlerInputs::GLUE));              \
    GateRef sp = PtrArgument(static_cast<size_t>(InterpreterHandlerInputs::SP));                  \
    GateRef pc = PtrArgument(static_cast<size_t>(InterpreterHandlerInputs::PC));                  \
    GateRef constpool = TaggedPointerArgument(                                                    \
        static_cast<size_t>(InterpreterHandlerInputs::CONSTPOOL));                                \
    GateRef profileTypeInfo = TaggedPointerArgument(                                              \
        static_cast<size_t>(InterpreterHandlerInputs::PROFILE_TYPE_INFO));                        \
    GateRef acc = TaggedArgument(static_cast<size_t>(InterpreterHandlerInputs::ACC));             \
    GateRef hotnessCounter = Int32Argument(                                                       \
        static_cast<size_t>(InterpreterHandlerInputs::HOTNESS_COUNTER));                          \
    DebugPrintInstruction<needPrint>();                                                           \
    V()                                                                                           \
    GenerateCircuitImpl(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, callback); \
}

#define DECLARE_ASM_HANDLE_IMPLEMENT(name)                                           \
void name##StubBuilder::GenerateCircuitImpl(GateRef glue, GateRef sp, GateRef pc,    \
                                     GateRef constpool, GateRef profileTypeInfo,     \
                                     GateRef acc, GateRef hotnessCounter,            \
                                     [[maybe_unused]] ProfileOperation callback)

#define REGISTER_PROFILE_CALL_BACK()                                                                              \
    ProfileOperation callback(                                                                                    \
        [this, glue, sp, pc, profileTypeInfo](const std::initializer_list<GateRef> &values, OperationType type) { \
            ProfilerStubBuilder profiler(this);                                                                   \
            profiler.PGOProfiler(glue, pc, GetFunctionFromFrame(GetFrame(sp)), profileTypeInfo, values, type);    \
        });

#define REGISTER_NULL_CALL_BACK() ProfileOperation callback;

#define DECLARE_ASM_HANDLER(name)                                 \
    DECLARE_ASM_HANDLER_BASE(name, true, REGISTER_NULL_CALL_BACK) \
    DECLARE_ASM_HANDLE_IMPLEMENT(name)

#define DECLARE_ASM_HANDLER_NOPRINT(name)                          \
    DECLARE_ASM_HANDLER_BASE(name, false, REGISTER_NULL_CALL_BACK) \
    DECLARE_ASM_HANDLE_IMPLEMENT(name)

#define DECLARE_ASM_HANDLER_PROFILE(name, ...) DECLARE_ASM_HANDLER_BASE(name, true, REGISTER_PROFILE_CALL_BACK)

// TYPE:{OFFSET, ACC_VARACC, JUMP, SSD}
#define DISPATCH_BAK(TYPE, ...) DISPATCH_##TYPE(__VA_ARGS__)

// Dispatch(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, offset)
#define DISPATCH_OFFSET(offset)                                                           \
    DISPATCH_BASE(profileTypeInfo, acc, hotnessCounter, offset)

// Dispatch(glue, sp, pc, constpool, profileTypeInfo, *varAcc, hotnessCounter, offset)
#define DISPATCH_VARACC(offset)                                                           \
    DISPATCH_BASE(profileTypeInfo, *varAcc, hotnessCounter, offset)

// Dispatch(glue, sp, pc, constpool, *varProfileTypeInfo, acc, *varHotnessCounter, offset)
#define DISPATCH_JUMP(offset)                                                             \
    DISPATCH_BASE(*varProfileTypeInfo, acc, *varHotnessCounter, offset)

#define DISPATCH_SSD(offset)                                                              \
    Dispatch(glue, *varSp, *varPc, *varConstpool, *varProfileTypeInfo, *varAcc,           \
             *varHotnessCounter, offset)

#define DISPATCH_BASE(...)                                                                \
    Dispatch(glue, sp, pc, constpool, __VA_ARGS__)

#define INT_PTR(opcode)                                                                   \
    IntPtr(BytecodeInstruction::Size(BytecodeInstruction::Opcode::opcode))

#define DISPATCH_WITH_ACC(opcode) DISPATCH_BAK(VARACC, INT_PTR(opcode))

#define DISPATCH(opcode) DISPATCH_BAK(OFFSET, INT_PTR(opcode))

#define DISPATCH_LAST()                                                                   \
    DispatchLast(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter)           \

#define DISPATCH_LAST_WITH_ACC()                                                          \
    DispatchLast(glue, sp, pc, constpool, profileTypeInfo, *varAcc, hotnessCounter)       \

#define UPDATE_HOTNESS(_sp, callback)                                                                          \
    varHotnessCounter = Int32Add(offset, *varHotnessCounter);                                                  \
    Branch(Int32LessThan(*varHotnessCounter, Int32(0)), &slowPath, &dispatch);                                 \
    Bind(&slowPath);                                                                                           \
    {                                                                                                          \
        GateRef func = GetFunctionFromFrame(GetFrame(_sp));                                                    \
        GateRef iVecOffset = IntPtr(JSThread::GlueData::GetInterruptVectorOffset(env->IsArch32Bit()));         \
        GateRef interruptsFlag = Load(VariableType::INT8(), glue, iVecOffset);                                 \
        varHotnessCounter = Int32(EcmaInterpreter::METHOD_HOTNESS_THRESHOLD);                                  \
        Label callRuntime(env);                                                                                \
        Branch(BoolOr(TaggedIsUndefined(*varProfileTypeInfo), Int8Equal(interruptsFlag,                        \
            Int8(VmThreadControl::VM_NEED_SUSPENSION))), &callRuntime, &dispatch);                             \
        Bind(&callRuntime);                                                                                    \
        {                                                                                                      \
            if (!(callback).IsEmpty()) {                                                                       \
                varProfileTypeInfo = CallRuntime(glue, RTSTUB_ID(UpdateHotnessCounterWithProf), { func });     \
            } else {                                                                                           \
                varProfileTypeInfo = CallRuntime(glue, RTSTUB_ID(UpdateHotnessCounter), { func });             \
            }                                                                                                  \
        }                                                                                                      \
        Jump(&dispatch);                                                                                       \
    }                                                                                                          \
    Bind(&dispatch);

#define CHECK_EXCEPTION(res, offset)                                                      \
    CheckException(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter,         \
                   res, offset)

#define CHECK_EXCEPTION_VARACC(res, offset)                                               \
    CheckException(glue, sp, pc, constpool, profileTypeInfo, *varAcc, hotnessCounter,     \
                   res, offset)

#define CHECK_EXCEPTION_WITH_JUMP(res, jump)                                              \
    CheckExceptionWithJump(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, \
		           res, jump)

#define CHECK_EXCEPTION_WITH_ACC(res, offset)                                             \
    CheckExceptionWithVar(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter,  \
		          res, offset)

#define CHECK_EXCEPTION_WITH_VARACC(res, offset)                                              \
    CheckExceptionWithVar(glue, sp, pc, constpool, profileTypeInfo, *varAcc, hotnessCounter,  \
		          res, offset)

#define CHECK_PENDING_EXCEPTION(res, offset)                                              \
    CheckPendingException(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter,  \
		          res, offset)

template <bool needPrint>
void InterpreterStubBuilder::DebugPrintInstruction()
{
#if ECMASCRIPT_ENABLE_INTERPRETER_LOG
    if (!needPrint) {
        return;
    }
    GateRef glue = PtrArgument(static_cast<size_t>(InterpreterHandlerInputs::GLUE));
    GateRef pc = PtrArgument(static_cast<size_t>(InterpreterHandlerInputs::PC));
    CallNGCRuntime(glue, RTSTUB_ID(DebugPrintInstruction), { glue, pc });
#endif
}

DECLARE_ASM_HANDLER(HandleLdnan)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = DoubleToTaggedDoublePtr(Double(base::NAN_VALUE));
    DISPATCH_WITH_ACC(LDNAN);
}

DECLARE_ASM_HANDLER(HandleLdinfinity)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = DoubleToTaggedDoublePtr(Double(base::POSITIVE_INFINITY));
    DISPATCH_WITH_ACC(LDINFINITY);
}

DECLARE_ASM_HANDLER(HandleLdundefined)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = Undefined();
    DISPATCH_WITH_ACC(LDUNDEFINED);
}

DECLARE_ASM_HANDLER(HandleLdnull)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = Null();
    DISPATCH_WITH_ACC(LDNULL);
}

DECLARE_ASM_HANDLER(HandleLdsymbol)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = CallRuntime(glue, RTSTUB_ID(GetSymbolFunction), {});
    DISPATCH_WITH_ACC(LDSYMBOL);
}

DECLARE_ASM_HANDLER(HandleLdglobal)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = GetGlobalObject(glue);
    DISPATCH_WITH_ACC(LDGLOBAL);
}

DECLARE_ASM_HANDLER(HandleLdtrue)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = TaggedTrue();
    DISPATCH_WITH_ACC(LDTRUE);
}

DECLARE_ASM_HANDLER(HandleLdfalse)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = TaggedFalse();
    DISPATCH_WITH_ACC(LDFALSE);
}

DECLARE_ASM_HANDLER(HandleTypeofImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = FastTypeOf(glue, acc);
    DISPATCH_WITH_ACC(TYPEOF_IMM8);
}

DECLARE_ASM_HANDLER(HandleTypeofImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = FastTypeOf(glue, acc);
    DISPATCH_WITH_ACC(TYPEOF_IMM16);
}

DECLARE_ASM_HANDLER(HandlePoplexenv)
{
    GateRef state = GetFrame(sp);
    GateRef currentLexEnv = GetEnvFromFrame(state);
    GateRef parentLexEnv = GetParentEnv(currentLexEnv);
    SetEnvToFrame(glue, state, parentLexEnv);
    DISPATCH(POPLEXENV);
}

DECLARE_ASM_HANDLER(HandleDeprecatedPoplexenvPrefNone)
{
    GateRef state = GetFrame(sp);
    GateRef currentLexEnv = GetEnvFromFrame(state);
    GateRef parentLexEnv = GetParentEnv(currentLexEnv);
    SetEnvToFrame(glue, state, parentLexEnv);
    DISPATCH(DEPRECATED_POPLEXENV_PREF_NONE);
}

DECLARE_ASM_HANDLER(HandleGetunmappedargs)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(argumentsList, VariableType::JS_ANY(), Hole());
    DEFVARIABLE(argumentsObj, VariableType::JS_ANY(), Hole());
    auto env = GetEnvironment();
    GateRef startIdxAndNumArgs = GetStartIdxAndNumArgs(sp, Int32(0));
    // 32: high 32 bits = startIdx, low 32 bits = numArgs
    GateRef startIdx = TruncInt64ToInt32(Int64LSR(startIdxAndNumArgs, Int64(32)));
    GateRef numArgs = TruncInt64ToInt32(startIdxAndNumArgs);

    Label newArgumentsObj(env);
    Label checkException(env);
    Label dispatch(env);
    Label slowPath(env);
    NewObjectStubBuilder newBuilder(this);
    newBuilder.SetParameters(glue, 0);
    Label afterArgumentsList(env);
    newBuilder.NewArgumentsList(&argumentsList, &afterArgumentsList, sp, startIdx, numArgs);
    Bind(&afterArgumentsList);
    Branch(TaggedIsException(*argumentsList), &slowPath, &newArgumentsObj);
    Bind(&newArgumentsObj);
    Label afterArgumentsObj(env);
    newBuilder.NewArgumentsObj(&argumentsObj, &afterArgumentsObj, *argumentsList, numArgs);
    Bind(&afterArgumentsObj);
    Branch(TaggedIsException(*argumentsObj), &slowPath, &checkException);
    Bind(&checkException);
    Branch(HasPendingException(glue), &slowPath, &dispatch);
    Bind(&dispatch);
    {
        varAcc = *argumentsObj;
        DISPATCH_WITH_ACC(GETUNMAPPEDARGS);
    }

    Bind(&slowPath);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(GetUnmapedArgs), {});
        CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(GETUNMAPPEDARGS));
    }
}

DECLARE_ASM_HANDLER(HandleCopyrestargsImm8)
{
    GateRef restIdx = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CopyRestArgs), { IntToTaggedInt(restIdx) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(COPYRESTARGS_IMM8));
}

DECLARE_ASM_HANDLER(HandleWideCopyrestargsPrefImm16)
{
    GateRef restIdx = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CopyRestArgs), { IntToTaggedInt(restIdx) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(WIDE_COPYRESTARGS_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleCreateobjectwithexcludedkeysImm8V8V8)
{
    GateRef numKeys = ReadInst8_0(pc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef firstArgRegIdx = ZExtInt8ToInt16(ReadInst8_2(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectWithExcludedKeys),
        { Int16ToTaggedInt(numKeys), obj, Int16ToTaggedInt(firstArgRegIdx) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleWideCreateobjectwithexcludedkeysPrefImm16V8V8)
{
    GateRef numKeys = ReadInst16_1(pc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef firstArgRegIdx = ZExtInt8ToInt16(ReadInst8_4(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectWithExcludedKeys),
        { Int16ToTaggedInt(numKeys), obj, Int16ToTaggedInt(firstArgRegIdx) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8));
}

DECLARE_ASM_HANDLER(HandleThrowIfsupernotcorrectcallPrefImm16)
{
    GateRef imm = ReadInst16_1(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(ThrowIfSuperNotCorrectCall),
        { Int16ToTaggedInt(imm), acc }); // acc is thisValue
    CHECK_EXCEPTION(res, INT_PTR(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleThrowIfsupernotcorrectcallPrefImm8)
{
    GateRef imm = ReadInst8_1(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(ThrowIfSuperNotCorrectCall),
        { Int8ToTaggedInt(imm), acc }); // acc is thisValue
    CHECK_EXCEPTION(res, INT_PTR(THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8));
}

DECLARE_ASM_HANDLER(HandleAsyncfunctionresolveV8)
{
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef value = acc;
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncFunctionResolveOrReject),
                              { asyncFuncObj, value, TaggedTrue() });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(ASYNCFUNCTIONRESOLVE_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedAsyncfunctionresolvePrefV8V8V8)
{
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncFunctionResolveOrReject),
                              { asyncFuncObj, value, TaggedTrue() });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleAsyncfunctionrejectV8)
{
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef value = acc;
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncFunctionResolveOrReject),
                              { asyncFuncObj, value, TaggedFalse() });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(ASYNCFUNCTIONREJECT_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedAsyncfunctionrejectPrefV8V8V8)
{
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncFunctionResolveOrReject),
                              { asyncFuncObj, value, TaggedFalse() });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleDefinegettersetterbyvalueV8V8V8V8)
{
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef prop = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef getter = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef setter = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(DefineGetterSetterByValue),
                              { obj, prop, getter, setter, acc }); // acc is flag
    callback.ProfileObjLayoutByStore(obj);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleGetpropiterator)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(GetPropIterator), { *varAcc });
    CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(GETPROPITERATOR));
}

DECLARE_ASM_HANDLER(HandleAsyncfunctionenter)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncFunctionEnter), {});
    CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(ASYNCFUNCTIONENTER));
}

DECLARE_ASM_HANDLER(HandleLdhole)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = Hole();
    DISPATCH_WITH_ACC(LDHOLE);
}

DECLARE_ASM_HANDLER(HandleCreateemptyobject)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateEmptyObject), {});
    callback.ProfileCreateObject(res);
    varAcc = res;
    DISPATCH_WITH_ACC(CREATEEMPTYOBJECT);
}

DECLARE_ASM_HANDLER(HandleCreateemptyarrayImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    NewObjectStubBuilder newBuilder(this);
    varAcc = newBuilder.CreateEmptyArray(glue, callback);
    DISPATCH_WITH_ACC(CREATEEMPTYARRAY_IMM8);
}

DECLARE_ASM_HANDLER(HandleCreateemptyarrayImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    NewObjectStubBuilder newBuilder(this);
    varAcc = newBuilder.CreateEmptyArray(glue, callback);
    DISPATCH_WITH_ACC(CREATEEMPTYARRAY_IMM16);
}

DECLARE_ASM_HANDLER(HandleGetiteratorImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(GetIterator), { *varAcc });
    CHECK_PENDING_EXCEPTION(res, INT_PTR(GETITERATOR_IMM8));
}

DECLARE_ASM_HANDLER(HandleGetiteratorImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(GetIterator), { *varAcc });
    CHECK_PENDING_EXCEPTION(res, INT_PTR(GETITERATOR_IMM16));
}

DECLARE_ASM_HANDLER(HandleGetasynciteratorImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(GetAsyncIterator), { *varAcc });
    CHECK_PENDING_EXCEPTION(res, INT_PTR(GETASYNCITERATOR_IMM8));
}

DECLARE_ASM_HANDLER(HandleThrowPatternnoncoerciblePrefNone)
{
    CallRuntime(glue, RTSTUB_ID(ThrowPatternNonCoercible), {});
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleThrowDeletesuperpropertyPrefNone)
{
    CallRuntime(glue, RTSTUB_ID(ThrowDeleteSuperProperty), {});
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleDebugger)
{
    CallRuntime(glue, RTSTUB_ID(NotifyDebuggerStatement), {});
    DISPATCH(DEBUGGER);
}

DECLARE_ASM_HANDLER(HandleMul2Imm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Mul(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(MUL2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleDiv2Imm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Div(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DIV2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleMod2Imm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Mod(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(MOD2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleEqImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Equal(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(EQ_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleJequndefinedImm16)
{
    DISPATCH(NOP);
}

DECLARE_ASM_HANDLER(HandleJequndefinedImm8)
{
    DISPATCH(NOP);
}

DECLARE_ASM_HANDLER(HandleNoteqImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.NotEqual(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(NOTEQ_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleLessImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Less(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LESS_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleLesseqImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.LessEq(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LESSEQ_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleGreaterImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Greater(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(GREATER_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleGreatereqImm8V8)
{
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.GreaterEq(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(GREATEREQ_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleNop)
{
    DISPATCH(NOP);
}

DECLARE_ASM_HANDLER(HandleLdaV8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef vsrc = ReadInst8_0(pc);
    varAcc = GetVregValue(sp, ZExtInt8ToPtr(vsrc));
    DISPATCH_WITH_ACC(LDA_V8);
}

DECLARE_ASM_HANDLER(HandleStaV8)
{
    GateRef vdst = ReadInst8_0(pc);
    SetVregValue(glue, sp, ZExtInt8ToPtr(vdst), acc);
    DISPATCH(STA_V8);
}

DECLARE_ASM_HANDLER(HandleJmpImm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    GateRef offset = ReadInstSigned8_0(pc);
    Label dispatch(env);
    Label slowPath(env);

    UPDATE_HOTNESS(sp, callback);
    DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
}

DECLARE_ASM_HANDLER(HandleJmpImm16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    GateRef offset = ReadInstSigned16_0(pc);
    Label dispatch(env);
    Label slowPath(env);

    UPDATE_HOTNESS(sp, callback);
    DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
}

DECLARE_ASM_HANDLER(HandleJmpImm32)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    GateRef offset = ReadInstSigned32_0(pc);
    Label dispatch(env);
    Label slowPath(env);
    UPDATE_HOTNESS(sp, callback);
    DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
}

DECLARE_ASM_HANDLER(HandleLdlexvarImm4Imm4)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef level = ZExtInt8ToInt32(ReadInst4_0(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst4_1(pc));
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    GateRef variable = GetPropertiesFromLexicalEnv(*currentEnv, slot);
    varAcc = variable;

    DISPATCH_WITH_ACC(LDLEXVAR_IMM4_IMM4);
}

DECLARE_ASM_HANDLER(HandleLdlexvarImm8Imm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef level = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst8_1(pc));

    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    GateRef variable = GetPropertiesFromLexicalEnv(*currentEnv, slot);
    varAcc = variable;
    DISPATCH_WITH_ACC(LDLEXVAR_IMM8_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideLdlexvarPrefImm16Imm16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef level = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef slot = ZExtInt16ToInt32(ReadInst16_3(pc));

    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    GateRef variable = GetPropertiesFromLexicalEnv(*currentEnv, slot);
    varAcc = variable;
    DISPATCH_WITH_ACC(WIDE_LDLEXVAR_PREF_IMM16_IMM16);
}

DECLARE_ASM_HANDLER(HandleStlexvarImm4Imm4)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt8ToInt32(ReadInst4_0(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst4_1(pc));

    GateRef value = acc;
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(STLEXVAR_IMM4_IMM4);
}

DECLARE_ASM_HANDLER(HandleStlexvarImm8Imm8)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst8_1(pc));

    GateRef value = acc;
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(STLEXVAR_IMM8_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideStlexvarPrefImm16Imm16)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef slot = ZExtInt16ToInt32(ReadInst16_3(pc));

    GateRef value = acc;
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(WIDE_STLEXVAR_PREF_IMM16_IMM16);
}

DECLARE_ASM_HANDLER(HandleDeprecatedStlexvarPrefImm16Imm16V8)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef slot = ZExtInt16ToInt32(ReadInst16_3(pc));

    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_5(pc)));
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM16_IMM16_V8);
}

DECLARE_ASM_HANDLER(HandleDeprecatedStlexvarPrefImm8Imm8V8)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt8ToInt32(ReadInst8_1(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst8_2(pc));

    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM8_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleDeprecatedStlexvarPrefImm4Imm4V8)
{
    auto env = GetEnvironment();
    GateRef level = ZExtInt8ToInt32(ReadInst4_2(pc));
    GateRef slot = ZExtInt8ToInt32(ReadInst4_3(pc));

    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef state = GetFrame(sp);
    DEFVARIABLE(currentEnv, VariableType::JS_ANY(), GetEnvFromFrame(state));
    DEFVARIABLE(i, VariableType::INT32(), Int32(0));

    Label loopHead(env);
    Label loopEnd(env);
    Label afterLoop(env);
    Branch(Int32LessThan(*i, level), &loopHead, &afterLoop);
    LoopBegin(&loopHead);
    currentEnv = GetParentEnv(*currentEnv);
    i = Int32Add(*i, Int32(1));
    Branch(Int32LessThan(*i, level), &loopEnd, &afterLoop);
    Bind(&loopEnd);
    LoopEnd(&loopHead);
    Bind(&afterLoop);
    SetPropertiesToLexicalEnv(glue, *currentEnv, slot, value);
    DISPATCH(DEPRECATED_STLEXVAR_PREF_IMM4_IMM4_V8);
}

DECLARE_ASM_HANDLER(HandleIncImm8)
{
    OperationsStubBuilder builder(this);
    GateRef result = builder.Inc(glue, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(INC_IMM8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedIncPrefV8)
{
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Inc(glue, value);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_INC_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleDecImm8)
{
    OperationsStubBuilder builder(this);
    GateRef result = builder.Dec(glue, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEC_IMM8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedDecPrefV8)
{
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Dec(glue, value);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_DEC_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleExpImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef base = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(Exp), { base, acc });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(EXP_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleIsinImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef prop = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(IsIn), { prop, acc }); // acc is obj
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(ISIN_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleInstanceofImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef target = acc;
    AccessObjectStubBuilder builder(this);
    GateRef result = InstanceOf(glue, obj, target, profileTypeInfo, slotId, callback);
    CHECK_PENDING_EXCEPTION(result, INT_PTR(INSTANCEOF_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleStrictnoteqImm8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));

    OperationsStubBuilder builder(this);
    varAcc = builder.StrictNotEqual(glue, left, acc, callback);
    DISPATCH_WITH_ACC(STRICTNOTEQ_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleStricteqImm8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));

    OperationsStubBuilder builder(this);
    varAcc = builder.StrictEqual(glue, left, acc, callback);
    DISPATCH_WITH_ACC(STRICTEQ_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleResumegenerator)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef obj = *varAcc;

#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef frame = GetFrame(sp);
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(StartCallTimer), { glue, curFunc, False() });
#endif
    Label isAsyncGeneratorObj(env);
    Label notAsyncGeneratorObj(env);
    Label dispatch(env);
    Branch(TaggedIsAsyncGeneratorObject(obj), &isAsyncGeneratorObj, &notAsyncGeneratorObj);
    Bind(&isAsyncGeneratorObj);
    {
        GateRef resumeResultOffset = IntPtr(JSAsyncGeneratorObject::GENERATOR_RESUME_RESULT_OFFSET);
        varAcc = Load(VariableType::JS_ANY(), obj, resumeResultOffset);
        Jump(&dispatch);
    }
    Bind(&notAsyncGeneratorObj);
    {
        GateRef resumeResultOffset = IntPtr(JSGeneratorObject::GENERATOR_RESUME_RESULT_OFFSET);
        varAcc = Load(VariableType::JS_ANY(), obj, resumeResultOffset);
        Jump(&dispatch);
    }
    Bind(&dispatch);
    DISPATCH_WITH_ACC(RESUMEGENERATOR);
}

DECLARE_ASM_HANDLER(HandleDeprecatedResumegeneratorPrefV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef frame = GetFrame(sp);
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(StartCallTimer), { glue, curFunc, False() });
#endif

    Label isAsyncGeneratorObj(env);
    Label notAsyncGeneratorObj(env);
    Label dispatch(env);
    Branch(TaggedIsAsyncGeneratorObject(obj), &isAsyncGeneratorObj, &notAsyncGeneratorObj);
    Bind(&isAsyncGeneratorObj);
    {
        GateRef resumeResultOffset = IntPtr(JSAsyncGeneratorObject::GENERATOR_RESUME_RESULT_OFFSET);
        varAcc = Load(VariableType::JS_ANY(), obj, resumeResultOffset);
        Jump(&dispatch);
    }
    Bind(&notAsyncGeneratorObj);
    {
        GateRef resumeResultOffset = IntPtr(JSGeneratorObject::GENERATOR_RESUME_RESULT_OFFSET);
        varAcc = Load(VariableType::JS_ANY(), obj, resumeResultOffset);
        Jump(&dispatch);
    }
    Bind(&dispatch);
    DISPATCH_WITH_ACC(DEPRECATED_RESUMEGENERATOR_PREF_V8);
}

DECLARE_ASM_HANDLER(HandleGetresumemode)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef obj = *varAcc;

    Label isAsyncGeneratorObj(env);
    Label notAsyncGeneratorObj(env);
    Label dispatch(env);
    Branch(TaggedIsAsyncGeneratorObject(obj), &isAsyncGeneratorObj, &notAsyncGeneratorObj);
    Bind(&isAsyncGeneratorObj);
    {
        varAcc = IntToTaggedPtr(GetResumeModeFromAsyncGeneratorObject(obj));
        Jump(&dispatch);
    }
    Bind(&notAsyncGeneratorObj);
    {
        varAcc = IntToTaggedPtr(GetResumeModeFromGeneratorObject(obj));
        Jump(&dispatch);
    }
    Bind(&dispatch);
    DISPATCH_WITH_ACC(GETRESUMEMODE);
}

DECLARE_ASM_HANDLER(HandleDeprecatedGetresumemodePrefV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));

    Label isAsyncGeneratorObj(env);
    Label notAsyncGeneratorObj(env);
    Label dispatch(env);
    Branch(TaggedIsAsyncGeneratorObject(obj), &isAsyncGeneratorObj, &notAsyncGeneratorObj);
    Bind(&isAsyncGeneratorObj);
    {
        varAcc = IntToTaggedPtr(GetResumeModeFromAsyncGeneratorObject(obj));
        Jump(&dispatch);
    }
    Bind(&notAsyncGeneratorObj);
    {
        varAcc = IntToTaggedPtr(GetResumeModeFromGeneratorObject(obj));
        Jump(&dispatch);
    }
    Bind(&dispatch);
    DISPATCH_WITH_ACC(DEPRECATED_GETRESUMEMODE_PREF_V8);
}

DECLARE_ASM_HANDLER(HandleCreategeneratorobjV8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef genFunc = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CreateGeneratorObj), { genFunc });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(CREATEGENERATOROBJ_V8));
}

DECLARE_ASM_HANDLER(HandleThrowConstassignmentPrefV8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(v0));
    CallRuntime(glue, RTSTUB_ID(ThrowConstAssignment), { value });
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleGettemplateobjectImm8)
{
    GateRef literal = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(GetTemplateObject), { literal });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(GETTEMPLATEOBJECT_IMM8));
}

DECLARE_ASM_HANDLER(HandleGettemplateobjectImm16)
{
    GateRef literal = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(GetTemplateObject), { literal });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(GETTEMPLATEOBJECT_IMM16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedGettemplateobjectPrefV8)
{
    GateRef literal = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef result = CallRuntime(glue, RTSTUB_ID(GetTemplateObject), { literal });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_GETTEMPLATEOBJECT_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleGetnextpropnameV8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef iter = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(GetNextPropName), { iter });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(GETNEXTPROPNAME_V8));
}

DECLARE_ASM_HANDLER(HandleThrowIfnotobjectPrefV8)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(v0));
    Label isEcmaObject(env);
    Label notEcmaObject(env);
    Label isHeapObject(env);
    Branch(TaggedIsHeapObject(value), &isHeapObject, &notEcmaObject);
    Bind(&isHeapObject);
    Branch(TaggedObjectIsEcmaObject(value), &isEcmaObject, &notEcmaObject);
    Bind(&isEcmaObject);
    {
        DISPATCH(THROW_IFNOTOBJECT_PREF_V8);
    }
    Bind(&notEcmaObject);
    CallRuntime(glue, RTSTUB_ID(ThrowIfNotObject), {});
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleCloseiteratorImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef iter = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CloseIterator), { iter });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(CLOSEITERATOR_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleCloseiteratorImm16V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef iter = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CloseIterator), { iter });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(CLOSEITERATOR_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleSupercallspreadImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef array = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(SuperCallSpread), { acc, array });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(SUPERCALLSPREAD_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleDelobjpropV8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef prop = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(DelObjProp), { obj, prop });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DELOBJPROP_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedDelobjpropPrefV8V8)
{
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef prop = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef result = CallRuntime(glue, RTSTUB_ID(DelObjProp), { obj, prop });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_DELOBJPROP_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleNewobjapplyImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(NewObjApply), { func, acc }); // acc is array
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(NEWOBJAPPLY_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleNewobjapplyImm16V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(NewObjApply), { func, acc }); // acc is array
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(NEWOBJAPPLY_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleCreateiterresultobjV8V8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef v1 = ReadInst8_1(pc);
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef flag = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CreateIterResultObj), { value, flag });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(CREATEITERRESULTOBJ_V8_V8));
}

DECLARE_ASM_HANDLER(HandleAsyncfunctionawaituncaughtV8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef value = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(AsyncFunctionAwaitUncaught), { asyncFuncObj, value });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(ASYNCFUNCTIONAWAITUNCAUGHT_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedAsyncfunctionawaituncaughtPrefV8V8)
{
    GateRef asyncFuncObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef result = CallRuntime(glue, RTSTUB_ID(AsyncFunctionAwaitUncaught), { asyncFuncObj, value });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleThrowUndefinedifholePrefV8V8)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef hole = GetVregValue(sp, ZExtInt8ToPtr(v0));
    Label isHole(env);
    Label notHole(env);
    Branch(TaggedIsHole(hole), &isHole, &notHole);
    Bind(&notHole);
    {
        DISPATCH(THROW_UNDEFINEDIFHOLE_PREF_V8_V8);
    }
    Bind(&isHole);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(v1));
    // assert obj.IsString()
    CallRuntime(glue, RTSTUB_ID(ThrowUndefinedIfHole), { obj });
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleThrowUndefinedifholewithnamePrefId16)
{
    auto env = GetEnvironment();

    GateRef hole = acc;
    Label isHole(env);
    Label notHole(env);
    Branch(TaggedIsHole(hole), &isHole, &notHole);
    Bind(&notHole);
    {
        DISPATCH(THROW_UNDEFINEDIFHOLEWITHNAME_PREF_ID16);
    }
    Bind(&isHole);
    GateRef stringId = ReadInst16_1(pc);
    GateRef str = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    // assert obj.IsString()
    CallRuntime(glue, RTSTUB_ID(ThrowUndefinedIfHole), { str });
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleCopydatapropertiesV8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef dst = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef src = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(CopyDataProperties), { dst, src });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(COPYDATAPROPERTIES_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedCopydatapropertiesPrefV8V8)
{
    GateRef dst = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef src = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CopyDataProperties), { dst, src });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_COPYDATAPROPERTIES_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleStarrayspreadV8V8)
{
    GateRef v0 = ReadInst8_0(pc);
    GateRef v1 = ReadInst8_1(pc);
    GateRef dst = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef result = CallRuntime(glue, RTSTUB_ID(StArraySpread), { dst, index, acc }); // acc is res
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(STARRAYSPREAD_V8_V8));
}

DECLARE_ASM_HANDLER(HandleSetobjectwithprotoImm8V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_1(pc);
    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef obj = *varAcc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(SetObjectWithProto), { proto, obj });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(result, &notException);
    Bind(&notException);
    DISPATCH_WITH_ACC(SETOBJECTWITHPROTO_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleSetobjectwithprotoImm16V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_2(pc);
    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef obj = *varAcc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(SetObjectWithProto), { proto, obj });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(result, &notException);
    Bind(&notException);
    DISPATCH_WITH_ACC(SETOBJECTWITHPROTO_IMM16_V8);
}

DECLARE_ASM_HANDLER(HandleDeprecatedSetobjectwithprotoPrefV8V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef result = CallRuntime(glue, RTSTUB_ID(SetObjectWithProto), { proto, obj });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(result, &notException);
    Bind(&notException);
    DISPATCH_WITH_ACC(DEPRECATED_SETOBJECTWITHPROTO_PREF_V8_V8);
}

DECLARE_ASM_HANDLER(HandleStobjbyvalueImm8V8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef value = acc;
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.StoreObjByValue(glue, receiver, propKey, value, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STOBJBYVALUE_IMM8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleStobjbyvalueImm16V8V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef v1 = ReadInst8_3(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef value = acc;
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.StoreObjByValue(glue, receiver, propKey, value, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STOBJBYVALUE_IMM16_V8_V8));
}

DECLARE_ASM_HANDLER(HandleStownbyvalueImm8V8V8)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
    Label isHeapObject(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    Label notClassConstructor(env);
    Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
    Bind(&notClassConstructor);
    Label notClassPrototype(env);
    Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
    Bind(&notClassPrototype);
    {
        // fast path
        GateRef result = SetPropertyByValue(glue, receiver, propKey, acc, true, callback); // acc is value
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYVALUE_IMM8_V8_V8));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StOwnByValue), { receiver, propKey, acc });
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYVALUE_IMM8_V8_V8));
    }
}

DECLARE_ASM_HANDLER(HandleStownbyvalueImm16V8V8)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_2(pc);
    GateRef v1 = ReadInst8_3(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
    Label isHeapObject(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    Label notClassConstructor(env);
    Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
    Bind(&notClassConstructor);
    Label notClassPrototype(env);
    Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
    Bind(&notClassPrototype);
    {
        // fast path
        GateRef result = SetPropertyByValue(glue, receiver, propKey, acc, true, callback); // acc is value
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYVALUE_IMM16_V8_V8));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StOwnByValue), { receiver, propKey, acc });
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYVALUE_IMM16_V8_V8));
    }
}

DECLARE_ASM_HANDLER(HandleStsuperbyvalueImm8V8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
     // acc is value, sp for thisFunc
    GateRef result = CallRuntime(glue, RTSTUB_ID(StSuperByValue), { receiver, propKey, acc });
    CHECK_EXCEPTION(result, INT_PTR(STSUPERBYVALUE_IMM8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleStsuperbyvalueImm16V8V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef v1 = ReadInst8_3(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
     // acc is value, sp for thisFunc
    GateRef result = CallRuntime(glue, RTSTUB_ID(StSuperByValue), { receiver, propKey, acc });
    CHECK_EXCEPTION(result, INT_PTR(STSUPERBYVALUE_IMM16_V8_V8));
}

DECLARE_ASM_HANDLER(HandleStsuperbynameImm8Id16V8)
{
    GateRef stringId = ReadInst16_1(pc);
    GateRef v0 = ReadInst8_3(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(StSuperByValue), { receiver, propKey, acc });
    CHECK_EXCEPTION(result, INT_PTR(STSUPERBYNAME_IMM8_ID16_V8));
}

DECLARE_ASM_HANDLER(HandleStsuperbynameImm16Id16V8)
{
    GateRef stringId = ReadInst16_2(pc);
    GateRef v0 = ReadInst8_4(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(StSuperByValue), { receiver, propKey, acc });
    CHECK_EXCEPTION(result, INT_PTR(STSUPERBYNAME_IMM16_ID16_V8));
}

DECLARE_ASM_HANDLER(HandleStobjbyindexImm8V8Imm16)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = ZExtInt16ToInt32(ReadInst16_2(pc));
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, false, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOBJBYINDEX_IMM8_V8_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StObjByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(STOBJBYINDEX_IMM8_V8_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleStobjbyindexImm16V8Imm16)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = ZExtInt16ToInt32(ReadInst16_3(pc));
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, false, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOBJBYINDEX_IMM16_V8_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StObjByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(STOBJBYINDEX_IMM16_V8_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleWideStobjbyindexPrefV8Imm32)
{
    auto env = GetEnvironment();

    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef index = ReadInst32_2(pc);
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, false, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(WIDE_STOBJBYINDEX_PREF_V8_IMM32));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StObjByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(WIDE_STOBJBYINDEX_PREF_V8_IMM32));
    }
}

DECLARE_ASM_HANDLER(HandleStownbyindexImm16V8Imm16)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = ZExtInt16ToInt32(ReadInst16_3(pc));
    Label isHeapObject(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    Label notClassConstructor(env);
    Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
    Bind(&notClassConstructor);
    Label notClassPrototype(env);
    Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
    Bind(&notClassPrototype);
    {
        // fast path
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, true, callback); // acc is value
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYINDEX_IMM16_V8_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StOwnByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYINDEX_IMM16_V8_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleStownbyindexImm8V8Imm16)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = ZExtInt16ToInt32(ReadInst16_2(pc));
    Label isHeapObject(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    Label notClassConstructor(env);
    Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
    Bind(&notClassConstructor);
    Label notClassPrototype(env);
    Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
    Bind(&notClassPrototype);
    {
        // fast path
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, true, callback); // acc is value
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYINDEX_IMM8_V8_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StOwnByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(STOWNBYINDEX_IMM8_V8_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleWideStownbyindexPrefV8Imm32)
{
    auto env = GetEnvironment();

    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef index = ReadInst32_2(pc);
    Label isHeapObject(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    Label notClassConstructor(env);
    Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
    Bind(&notClassConstructor);
    Label notClassPrototype(env);
    Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
    Bind(&notClassPrototype);
    {
        // fast path
        GateRef result = SetPropertyByIndex(glue, receiver, index, acc, true, callback); // acc is value
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION(result, INT_PTR(WIDE_STOWNBYINDEX_PREF_V8_IMM32));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(StOwnByIndex),
                                     { receiver, IntToTaggedInt(index), acc });
        CHECK_EXCEPTION(result, INT_PTR(WIDE_STOWNBYINDEX_PREF_V8_IMM32));
    }
}

DECLARE_ASM_HANDLER(HandleNegImm8)
{
    OperationsStubBuilder builder(this);
    GateRef result = builder.Neg(glue, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(NEG_IMM8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedNegPrefV8)
{
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Neg(glue, value);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_NEG_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleNotImm8)
{
    OperationsStubBuilder builder(this);
    GateRef result = builder.Not(glue, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(NOT_IMM8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedNotPrefV8)
{
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Not(glue, value);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_NOT_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleAnd2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.And(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(AND2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleOr2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Or(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(OR2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleXor2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Xor(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(XOR2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleAshr2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Ashr(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(ASHR2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleShr2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Shr(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(SHR2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleShl2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Shl(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(SHL2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleStobjbynameImm8Id16V8)
{
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.StoreObjByName(glue, receiver, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STOBJBYNAME_IMM8_ID16_V8));
}

DECLARE_ASM_HANDLER(HandleStobjbynameImm16Id16V8)
{
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_4(pc)));
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.StoreObjByName(glue, receiver, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STOBJBYNAME_IMM16_ID16_V8));
}

DECLARE_ASM_HANDLER(HandleStownbyvaluewithnamesetImm16V8V8)
{
    auto env = GetEnvironment();
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));

    Label isHeapObject(env);
    Label slowPath(env);
    Label notClassConstructor(env);
    Label notClassPrototype(env);
    Label notHole(env);
    Label notException(env);
    Label notException1(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    {
        Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
            Bind(&notClassPrototype);
            {
                GateRef res = SetPropertyByValue(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(res), &slowPath, &notHole);
                Bind(&notHole);
                {
                    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
                    Bind(&notException);
                    CallRuntime(glue, RTSTUB_ID(SetFunctionNameNoPrefix), { acc, propKey });
                    DISPATCH(STOWNBYVALUEWITHNAMESET_IMM16_V8_V8);
                }
            }
        }
    }
    Bind(&slowPath);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(StOwnByValueWithNameSet), { receiver, propKey, acc });
        CHECK_EXCEPTION_WITH_JUMP(res, &notException1);
        Bind(&notException1);
        DISPATCH(STOWNBYVALUEWITHNAMESET_IMM16_V8_V8);
    }
}

DECLARE_ASM_HANDLER(HandleStownbyvaluewithnamesetImm8V8V8)
{
    auto env = GetEnvironment();
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));

    Label isHeapObject(env);
    Label slowPath(env);
    Label notClassConstructor(env);
    Label notClassPrototype(env);
    Label notHole(env);
    Label notException(env);
    Label notException1(env);
    Branch(TaggedIsHeapObject(receiver), &isHeapObject, &slowPath);
    Bind(&isHeapObject);
    {
        Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Branch(IsClassPrototype(receiver), &slowPath, &notClassPrototype);
            Bind(&notClassPrototype);
            {
                GateRef res = SetPropertyByValue(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(res), &slowPath, &notHole);
                Bind(&notHole);
                {
                    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
                    Bind(&notException);
                    CallRuntime(glue, RTSTUB_ID(SetFunctionNameNoPrefix), { acc, propKey });
                    DISPATCH(STOWNBYVALUEWITHNAMESET_IMM8_V8_V8);
                }
            }
        }
    }
    Bind(&slowPath);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(StOwnByValueWithNameSet), { receiver, propKey, acc });
        CHECK_EXCEPTION_WITH_JUMP(res, &notException1);
        Bind(&notException1);
        DISPATCH(STOWNBYVALUEWITHNAMESET_IMM8_V8_V8);
    }
}

DECLARE_ASM_HANDLER(HandleStownbynameImm8Id16V8)
{
    auto env = GetEnvironment();
    GateRef stringId = ReadInst16_1(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label checkResult(env);

    Label isJSObject(env);
    Label slowPath(env);
    Branch(IsJSObject(receiver), &isJSObject, &slowPath);
    Bind(&isJSObject);
    {
        Label notClassConstructor(env);
        Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Label fastPath(env);
            Branch(IsClassPrototype(receiver), &slowPath, &fastPath);
            Bind(&fastPath);
            {
                result = SetPropertyByName(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(*result), &slowPath, &checkResult);
            }
        }
    }
    Bind(&slowPath);
    {
        result = CallRuntime(glue, RTSTUB_ID(StOwnByName), { receiver, propKey, acc });
        Jump(&checkResult);
    }
    Bind(&checkResult);
    {
        CHECK_EXCEPTION(*result, INT_PTR(STOWNBYNAME_IMM8_ID16_V8));
    }
}

DECLARE_ASM_HANDLER(HandleStownbynameImm16Id16V8)
{
    auto env = GetEnvironment();
    GateRef stringId = ReadInst16_2(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_4(pc)));
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    Label checkResult(env);

    Label isJSObject(env);
    Label slowPath(env);
    Branch(IsJSObject(receiver), &isJSObject, &slowPath);
    Bind(&isJSObject);
    {
        Label notClassConstructor(env);
        Branch(IsClassConstructor(receiver), &slowPath, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Label fastPath(env);
            Branch(IsClassPrototype(receiver), &slowPath, &fastPath);
            Bind(&fastPath);
            {
                result = SetPropertyByName(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(*result), &slowPath, &checkResult);
            }
        }
    }
    Bind(&slowPath);
    {
        result = CallRuntime(glue, RTSTUB_ID(StOwnByName), { receiver, propKey, acc });
        Jump(&checkResult);
    }
    Bind(&checkResult);
    {
        CHECK_EXCEPTION(*result, INT_PTR(STOWNBYNAME_IMM16_ID16_V8));
    }
}

DECLARE_ASM_HANDLER(HandleStownbynamewithnamesetImm8Id16V8)
{
    auto env = GetEnvironment();
    GateRef stringId = ReadInst16_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    Label isJSObject(env);
    Label notJSObject(env);
    Label notClassConstructor(env);
    Label notClassPrototype(env);
    Label notHole(env);
    Label notException(env);
    Label notException1(env);
    Branch(IsJSObject(receiver), &isJSObject, &notJSObject);
    Bind(&isJSObject);
    {
        Branch(IsClassConstructor(receiver), &notJSObject, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Branch(IsClassPrototype(receiver), &notJSObject, &notClassPrototype);
            Bind(&notClassPrototype);
            {
                GateRef res = SetPropertyByName(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(res), &notJSObject, &notHole);
                Bind(&notHole);
                {
                    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
                    Bind(&notException);
                    CallRuntime(glue, RTSTUB_ID(SetFunctionNameNoPrefix), { acc, propKey });
                    DISPATCH(STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8);
                }
            }
        }
    }
    Bind(&notJSObject);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(StOwnByNameWithNameSet), { receiver, propKey, acc });
        CHECK_EXCEPTION_WITH_JUMP(res, &notException1);
        Bind(&notException1);
        DISPATCH(STOWNBYNAMEWITHNAMESET_IMM8_ID16_V8);
    }
}
DECLARE_ASM_HANDLER(HandleStownbynamewithnamesetImm16Id16V8)
{
    auto env = GetEnvironment();
    GateRef stringId = ReadInst16_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_4(pc)));
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    Label isJSObject(env);
    Label notJSObject(env);
    Label notClassConstructor(env);
    Label notClassPrototype(env);
    Label notHole(env);
    Label notException(env);
    Label notException1(env);
    Branch(IsJSObject(receiver), &isJSObject, &notJSObject);
    Bind(&isJSObject);
    {
        Branch(IsClassConstructor(receiver), &notJSObject, &notClassConstructor);
        Bind(&notClassConstructor);
        {
            Branch(IsClassPrototype(receiver), &notJSObject, &notClassPrototype);
            Bind(&notClassPrototype);
            {
                GateRef res = SetPropertyByName(glue, receiver, propKey, acc, true, callback);
                Branch(TaggedIsHole(res), &notJSObject, &notHole);
                Bind(&notHole);
                {
                    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
                    Bind(&notException);
                    CallRuntime(glue, RTSTUB_ID(SetFunctionNameNoPrefix), { acc, propKey });
                    DISPATCH(STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8);
                }
            }
        }
    }
    Bind(&notJSObject);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(StOwnByNameWithNameSet), { receiver, propKey, acc });
        CHECK_EXCEPTION_WITH_JUMP(res, &notException1);
        Bind(&notException1);
        DISPATCH(STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8);
    }
}

DECLARE_ASM_HANDLER(HandleLdfunction)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    varAcc = GetFunctionFromFrame(GetFrame(sp));
    DISPATCH_WITH_ACC(LDFUNCTION);
}

DECLARE_ASM_HANDLER(HandleMovV4V4)
{
    GateRef vdst = ZExtInt8ToPtr(ReadInst4_0(pc));
    GateRef vsrc = ZExtInt8ToPtr(ReadInst4_1(pc));
    GateRef value = GetVregValue(sp, vsrc);
    SetVregValue(glue, sp, vdst, value);
    DISPATCH(MOV_V4_V4);
}

DECLARE_ASM_HANDLER(HandleMovV8V8)
{
    GateRef vdst = ZExtInt8ToPtr(ReadInst8_0(pc));
    GateRef vsrc = ZExtInt8ToPtr(ReadInst8_1(pc));
    GateRef value = GetVregValue(sp, vsrc);
    SetVregValue(glue, sp, vdst, value);
    DISPATCH(MOV_V8_V8);
}

DECLARE_ASM_HANDLER(HandleMovV16V16)
{
    GateRef vdst = ZExtInt16ToPtr(ReadInst16_0(pc));
    GateRef vsrc = ZExtInt16ToPtr(ReadInst16_2(pc));
    GateRef value = GetVregValue(sp, vsrc);
    SetVregValue(glue, sp, vdst, value);
    DISPATCH(MOV_V16_V16);
}

DECLARE_ASM_HANDLER(HandleLdaStrId16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst16_0(pc);
    varAcc = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    DISPATCH_WITH_ACC(LDA_STR_ID16);
}

DECLARE_ASM_HANDLER(HandleLdaiImm32)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef imm = ReadInst32_0(pc);
    varAcc = IntToTaggedPtr(imm);
    DISPATCH_WITH_ACC(LDAI_IMM32);
}

DECLARE_ASM_HANDLER(HandleFldaiImm64)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef imm = CastInt64ToFloat64(ReadInst64_0(pc));
    varAcc = DoubleToTaggedDoublePtr(imm);
    DISPATCH_WITH_ACC(FLDAI_IMM64);
}

DECLARE_ASM_HANDLER(HandleJeqzImm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualFalse(env);
    Label accNotEqualFalse(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsFalse(acc), &accEqualFalse, &accNotEqualFalse);
    Bind(&accNotEqualFalse);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accEqualFalse, &accNotInt);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &accEqualFalse, &last);
            }
        }
    }
    Bind(&accEqualFalse);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned8_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JEQZ_IMM8));
}

DECLARE_ASM_HANDLER(HandleJeqzImm16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualFalse(env);
    Label accNotEqualFalse(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsFalse(acc), &accEqualFalse, &accNotEqualFalse);
    Bind(&accNotEqualFalse);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accEqualFalse, &accNotInt);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &accEqualFalse, &last);
            }
        }
    }
    Bind(&accEqualFalse);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned16_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JEQZ_IMM16));
}

DECLARE_ASM_HANDLER(HandleJeqzImm32)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualFalse(env);
    Label accNotEqualFalse(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsFalse(acc), &accEqualFalse, &accNotEqualFalse);
    Bind(&accNotEqualFalse);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accEqualFalse, &accNotInt);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &accEqualFalse, &last);
            }
        }
    }
    Bind(&accEqualFalse);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned32_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JEQZ_IMM32));
}

DECLARE_ASM_HANDLER(HandleJnezImm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualTrue(env);
    Label accNotEqualTrue(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsTrue(acc), &accEqualTrue, &accNotEqualTrue);
    Bind(&accNotEqualTrue);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accNotInt, &accEqualTrue);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &last, &accEqualTrue);
            }
        }
    }
    Bind(&accEqualTrue);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned8_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JNEZ_IMM8));
}

DECLARE_ASM_HANDLER(HandleJnezImm16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualTrue(env);
    Label accNotEqualTrue(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsTrue(acc), &accEqualTrue, &accNotEqualTrue);
    Bind(&accNotEqualTrue);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accNotInt, &accEqualTrue);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &last, &accEqualTrue);
            }
        }
    }
    Bind(&accEqualTrue);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned16_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JNEZ_IMM16));
}

DECLARE_ASM_HANDLER(HandleJnezImm32)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_ANY(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label accEqualTrue(env);
    Label accNotEqualTrue(env);
    Label accIsInt(env);
    Label accNotInt(env);
    Label accIsDouble(env);
    Label last(env);
    Branch(TaggedIsTrue(acc), &accEqualTrue, &accNotEqualTrue);
    Bind(&accNotEqualTrue);
    {
        Branch(TaggedIsInt(acc), &accIsInt, &accNotInt);
        Bind(&accIsInt);
        {
            Branch(Int32Equal(TaggedGetInt(acc), Int32(0)), &accNotInt, &accEqualTrue);
        }
        Bind(&accNotInt);
        {
            Branch(TaggedIsDouble(acc), &accIsDouble, &last);
            Bind(&accIsDouble);
            {
                Branch(DoubleEqual(GetDoubleOfTDouble(acc), Double(0)), &last, &accEqualTrue);
            }
        }
    }
    Bind(&accEqualTrue);
    {
        Label dispatch(env);
        Label slowPath(env);
        GateRef offset = ReadInstSigned32_0(pc);
        UPDATE_HOTNESS(sp, callback);
        DISPATCH_BAK(JUMP, SExtInt32ToPtr(offset));
    }
    Bind(&last);
    DISPATCH_BAK(JUMP, INT_PTR(JNEZ_IMM32));
}

DECLARE_ASM_HANDLER(HandleReturn)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label pcEqualNullptr(env);
    Label pcNotEqualNullptr(env);
    Label updateHotness(env);
    Label tryContinue(env);
    Label dispatch(env);
    Label slowPath(env);

    GateRef frame = GetFrame(*varSp);
    Branch(TaggedIsUndefined(*varProfileTypeInfo), &updateHotness, &tryContinue);
    Bind(&updateHotness);
    {
        GateRef function = GetFunctionFromFrame(frame);
        GateRef method = Load(VariableType::JS_ANY(), function,
            IntPtr(JSFunctionBase::METHOD_OFFSET));
        GateRef fistPC = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef offset = Int32Not(TruncPtrToInt32(PtrSub(*varPc, fistPC)));
        UPDATE_HOTNESS(*varSp, callback);
        SetHotnessCounter(glue, method, *varHotnessCounter);
        Jump(&tryContinue);
    }

    Bind(&tryContinue);
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(EndCallTimer), { glue, curFunc });
#endif
    GateRef currentSp = *varSp;
    varSp = Load(VariableType::NATIVE_POINTER(), frame,
                 IntPtr(AsmInterpretedFrame::GetBaseOffset(env->IsArch32Bit())));
    GateRef prevState = GetFrame(*varSp);
    varPc = GetPcFromFrame(prevState);
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcEqualNullptr, &pcNotEqualNullptr);
    Bind(&pcEqualNullptr);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { *varAcc, *varSp, currentSp });
        Return();
    }
    Bind(&pcNotEqualNullptr);
    {
        GateRef function = GetFunctionFromFrame(prevState);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        GateRef jumpSize = GetCallSizeFromFrame(prevState);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndDispatch),
                       { glue, currentSp, *varPc, *varConstpool, *varProfileTypeInfo,
                         *varAcc, *varHotnessCounter, jumpSize });
        Return();
    }
}

DECLARE_ASM_HANDLER(HandleReturnundefined)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label pcEqualNullptr(env);
    Label pcNotEqualNullptr(env);
    Label updateHotness(env);
    Label tryContinue(env);
    Label dispatch(env);
    Label slowPath(env);

    GateRef frame = GetFrame(*varSp);
    Branch(TaggedIsUndefined(*varProfileTypeInfo), &updateHotness, &tryContinue);
    Bind(&updateHotness);
    {
        GateRef function = GetFunctionFromFrame(frame);
        GateRef method = Load(VariableType::JS_ANY(), function,
            IntPtr(JSFunctionBase::METHOD_OFFSET));
        GateRef fistPC = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef offset = Int32Not(TruncPtrToInt32(PtrSub(*varPc, fistPC)));
        UPDATE_HOTNESS(*varSp, callback);
        SetHotnessCounter(glue, method, *varHotnessCounter);
        Jump(&tryContinue);
    }

    Bind(&tryContinue);
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(EndCallTimer), { glue, curFunc });
#endif
    GateRef currentSp = *varSp;
    varSp = Load(VariableType::NATIVE_POINTER(), frame,
        IntPtr(AsmInterpretedFrame::GetBaseOffset(env->IsArch32Bit())));
    GateRef prevState = GetFrame(*varSp);
    varPc = GetPcFromFrame(prevState);
    varAcc = Undefined();
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcEqualNullptr, &pcNotEqualNullptr);
    Bind(&pcEqualNullptr);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { *varAcc, *varSp, currentSp });
        Return();
    }
    Bind(&pcNotEqualNullptr);
    {
        GateRef function = GetFunctionFromFrame(prevState);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        GateRef jumpSize = GetCallSizeFromFrame(prevState);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndDispatch),
                       { glue, currentSp, *varPc, *varConstpool, *varProfileTypeInfo,
                         *varAcc, *varHotnessCounter, jumpSize });
        Return();
    }
}

DECLARE_ASM_HANDLER(HandleSuspendgeneratorV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label pcEqualNullptr(env);
    Label pcNotEqualNullptr(env);
    Label updateHotness(env);
    Label tryContinue(env);
    Label dispatch(env);
    Label slowPath(env);

    GateRef genObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef value = *varAcc;
    GateRef frame = GetFrame(*varSp);
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuspendGenerator), { genObj, value });
    Label isException(env);
    Label notException(env);
    Branch(TaggedIsException(res), &isException, &notException);
    Bind(&isException);
    {
        DispatchLast(glue, *varSp, *varPc, *varConstpool, *varProfileTypeInfo, *varAcc, *varHotnessCounter);
    }
    Bind(&notException);
    varAcc = res;
    Branch(TaggedIsUndefined(*varProfileTypeInfo), &updateHotness, &tryContinue);
    Bind(&updateHotness);
    {
        GateRef function = GetFunctionFromFrame(frame);
        GateRef method = Load(VariableType::JS_ANY(), function,
            IntPtr(JSFunctionBase::METHOD_OFFSET));
        GateRef fistPC = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef offset = Int32Not(TruncPtrToInt32(PtrSub(*varPc, fistPC)));
        UPDATE_HOTNESS(*varSp, callback);
        SetHotnessCounter(glue, method, *varHotnessCounter);
        Jump(&tryContinue);
    }

    Bind(&tryContinue);
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(EndCallTimer), { glue, curFunc });
#endif
    GateRef currentSp = *varSp;
    varSp = Load(VariableType::NATIVE_POINTER(), frame,
        IntPtr(AsmInterpretedFrame::GetBaseOffset(env->IsArch32Bit())));
    GateRef prevState = GetFrame(*varSp);
    varPc = GetPcFromFrame(prevState);
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcEqualNullptr, &pcNotEqualNullptr);
    Bind(&pcEqualNullptr);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { *varAcc, *varSp, currentSp });
        Return();
    }
    Bind(&pcNotEqualNullptr);
    {
        GateRef function = GetFunctionFromFrame(prevState);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        GateRef jumpSize = GetCallSizeFromFrame(prevState);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndDispatch),
                    { glue, currentSp, *varPc, *varConstpool, *varProfileTypeInfo,
                      *varAcc, *varHotnessCounter, jumpSize });
        Return();
    }
}

DECLARE_ASM_HANDLER(HandleDeprecatedSuspendgeneratorPrefV8V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label pcEqualNullptr(env);
    Label pcNotEqualNullptr(env);
    Label updateHotness(env);
    Label tryContinue(env);
    Label dispatch(env);
    Label slowPath(env);

    GateRef genObj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef frame = GetFrame(*varSp);
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuspendGenerator), { genObj, value });
    Label isException(env);
    Label notException(env);
    Branch(TaggedIsException(res), &isException, &notException);
    Bind(&isException);
    {
        DispatchLast(glue, *varSp, *varPc, *varConstpool, *varProfileTypeInfo, *varAcc, *varHotnessCounter);
    }
    Bind(&notException);
    varAcc = res;
    Branch(TaggedIsUndefined(*varProfileTypeInfo), &updateHotness, &tryContinue);
    Bind(&updateHotness);
    {
        GateRef function = GetFunctionFromFrame(frame);
        GateRef method = Load(VariableType::JS_ANY(), function,
            IntPtr(JSFunctionBase::METHOD_OFFSET));
        GateRef fistPC = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef offset = Int32Not(TruncPtrToInt32(PtrSub(*varPc, fistPC)));
        UPDATE_HOTNESS(*varSp, callback);
        SetHotnessCounter(glue, method, *varHotnessCounter);
        Jump(&tryContinue);
    }

    Bind(&tryContinue);
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(EndCallTimer), { glue, curFunc });
#endif
    GateRef currentSp = *varSp;
    varSp = Load(VariableType::NATIVE_POINTER(), frame,
        IntPtr(AsmInterpretedFrame::GetBaseOffset(env->IsArch32Bit())));
    GateRef prevState = GetFrame(*varSp);
    varPc = GetPcFromFrame(prevState);
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcEqualNullptr, &pcNotEqualNullptr);
    Bind(&pcEqualNullptr);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { *varAcc, *varSp, currentSp });
        Return();
    }
    Bind(&pcNotEqualNullptr);
    {
        GateRef function = GetFunctionFromFrame(prevState);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        GateRef jumpSize = GetCallSizeFromFrame(prevState);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndDispatch),
                    { glue, currentSp, *varPc, *varConstpool, *varProfileTypeInfo,
                      *varAcc, *varHotnessCounter, jumpSize });
        Return();
    }
}

DECLARE_ASM_HANDLER(HandleTryldglobalbynameImm8Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.TryLoadGlobalByName(glue, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(TRYLDGLOBALBYNAME_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleTryldglobalbynameImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.TryLoadGlobalByName(glue, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(TRYLDGLOBALBYNAME_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleTrystglobalbynameImm8Id16)
{
    GateRef slotId = ZExtInt16ToInt32(ReadInst8_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.TryStoreGlobalByName(glue, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(TRYSTGLOBALBYNAME_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleTrystglobalbynameImm16Id16)
{
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.TryStoreGlobalByName(glue, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(TRYSTGLOBALBYNAME_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleLdglobalvarImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.LoadGlobalVar(glue, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDGLOBALVAR_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleStglobalvarImm16Id16)
{
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.StoreGlobalVar(glue, 0, info, acc, profileTypeInfo, slotId);
    CHECK_EXCEPTION(result, INT_PTR(STGLOBALVAR_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleCreateregexpwithliteralImm8Id16Imm8)
{
    GateRef stringId = ReadInst16_1(pc);
    GateRef pattern = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef flags = ReadInst8_3(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateRegExpWithLiteral),
                              { pattern, Int8ToTaggedInt(flags) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEREGEXPWITHLITERAL_IMM8_ID16_IMM8));
}

DECLARE_ASM_HANDLER(HandleCreateregexpwithliteralImm16Id16Imm8)
{
    GateRef stringId = ReadInst16_2(pc);
    GateRef pattern = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef flags = ReadInst8_4(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateRegExpWithLiteral),
                              { pattern, Int8ToTaggedInt(flags) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEREGEXPWITHLITERAL_IMM16_ID16_IMM8));
}

DECLARE_ASM_HANDLER(HandleIstrue)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    varAcc = FastToBoolean(*varAcc);
    DISPATCH_WITH_ACC(ISTRUE);
}

DECLARE_ASM_HANDLER(HandleIsfalse)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    Label isTrue(env);
    Label isFalse(env);
    Label dispatch(env);
    auto result = FastToBoolean(*varAcc);
    Branch(TaggedIsTrue(result), &isTrue, &isFalse);
    Bind(&isTrue);
    {
        varAcc = TaggedFalse();
        Jump(&dispatch);
    }
    Bind(&isFalse);
    {
        varAcc = TaggedTrue();
        Jump(&dispatch);
    }
    Bind(&dispatch);
    DISPATCH_WITH_ACC(ISFALSE);
}

DECLARE_ASM_HANDLER(HandleTonumberImm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef value = *varAcc;
    Label valueIsNumber(env);
    Label valueNotNumber(env);
    Branch(TaggedIsNumber(value), &valueIsNumber, &valueNotNumber);
    Bind(&valueIsNumber);
    {
        varAcc = value;
        DISPATCH_WITH_ACC(TONUMBER_IMM8);
    }
    Bind(&valueNotNumber);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(ToNumber), { value });
        CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(TONUMBER_IMM8));
    }
}

DECLARE_ASM_HANDLER(HandleDeprecatedTonumberPrefV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    Label valueIsNumber(env);
    Label valueNotNumber(env);
    Branch(TaggedIsNumber(value), &valueIsNumber, &valueNotNumber);
    Bind(&valueIsNumber);
    {
        varAcc = value;
        DISPATCH_WITH_ACC(DEPRECATED_TONUMBER_PREF_V8);
    }
    Bind(&valueNotNumber);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(ToNumber), { value });
        CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(DEPRECATED_TONUMBER_PREF_V8));
    }
}

DECLARE_ASM_HANDLER(HandleAdd2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Add(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(ADD2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleSub2Imm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef left = GetVregValue(sp, ZExtInt8ToPtr(v0));
    OperationsStubBuilder builder(this);
    GateRef result = builder.Sub(glue, left, acc, callback);
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(SUB2_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleLdbigintId16)
{
    GateRef stringId = ReadInst16_0(pc);
    GateRef numberBigInt = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef res = CallRuntime(glue, RTSTUB_ID(LdBigInt), { numberBigInt });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(LDBIGINT_ID16));
}

DECLARE_ASM_HANDLER(HandleTonumericImm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef value = *varAcc;
    Label valueIsNumeric(env);
    Label valueNotNumeric(env);
    Branch(TaggedIsNumeric(value), &valueIsNumeric, &valueNotNumeric);
    Bind(&valueIsNumeric);
    {
        varAcc = value;
        DISPATCH_WITH_ACC(TONUMERIC_IMM8);
    }
    Bind(&valueNotNumeric);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(ToNumeric), { value });
        CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(TONUMERIC_IMM8));
    }
}

DECLARE_ASM_HANDLER(HandleDeprecatedTonumericPrefV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    Label valueIsNumeric(env);
    Label valueNotNumeric(env);
    Branch(TaggedIsNumeric(value), &valueIsNumeric, &valueNotNumeric);
    Bind(&valueIsNumeric);
    {
        varAcc = value;
        DISPATCH_WITH_ACC(DEPRECATED_TONUMERIC_PREF_V8);
    }
    Bind(&valueNotNumeric);
    {
        GateRef res = CallRuntime(glue, RTSTUB_ID(ToNumeric), { value });
        CHECK_EXCEPTION_WITH_VARACC(res, INT_PTR(DEPRECATED_TONUMERIC_PREF_V8));
    }
}

DECLARE_ASM_HANDLER(HandleDynamicimport)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef specifier = *varAcc;
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(DynamicImport), { specifier, currentFunc });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DYNAMICIMPORT));
}

DECLARE_ASM_HANDLER(HandleDeprecatedDynamicimportPrefV8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef specifier = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(DynamicImport), { specifier, currentFunc });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_DYNAMICIMPORT_PREF_V8));
}

DECLARE_ASM_HANDLER(HandleCreateasyncgeneratorobjV8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_0(pc);
    GateRef genFunc = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef result = CallRuntime(glue, RTSTUB_ID(CreateAsyncGeneratorObj), { genFunc });
    Label isException(env);
    Label notException(env);
    Branch(TaggedIsException(result), &isException, &notException);
    Bind(&isException);
    {
        DISPATCH_LAST();
    }
    Bind(&notException);
    varAcc = result;
    DISPATCH_WITH_ACC(CREATEASYNCGENERATOROBJ_V8);
}

DECLARE_ASM_HANDLER(HandleAsyncgeneratorresolveV8V8V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    Label pcEqualNullptr(env);
    Label pcNotEqualNullptr(env);
    Label updateHotness(env);
    Label tryContinue(env);
    Label dispatch(env);
    Label slowPath(env);

    GateRef asyncGenerator = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef flag = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef frame = GetFrame(*varSp);
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncGeneratorResolve),
                              { asyncGenerator, value, flag });
    Label isException(env);
    Label notException(env);
    Branch(TaggedIsException(res), &isException, &notException);
    Bind(&isException);
    {
        DISPATCH_LAST();
    }
    Bind(&notException);
    varAcc = res;
    Branch(TaggedIsUndefined(*varProfileTypeInfo), &updateHotness, &tryContinue);
    Bind(&updateHotness);
    {
        GateRef function = GetFunctionFromFrame(frame);
        GateRef method = Load(VariableType::JS_ANY(), function,
            IntPtr(JSFunctionBase::METHOD_OFFSET));
        GateRef fistPC = Load(VariableType::NATIVE_POINTER(), method,
            IntPtr(Method::NATIVE_POINTER_OR_BYTECODE_ARRAY_OFFSET));
        GateRef offset = Int32Not(TruncPtrToInt32(PtrSub(*varPc, fistPC)));
        UPDATE_HOTNESS(*varSp, callback);
        SetHotnessCounter(glue, method, *varHotnessCounter);
        Jump(&tryContinue);
    }

    Bind(&tryContinue);
#if ECMASCRIPT_ENABLE_FUNCTION_CALL_TIMER
    GateRef curFunc = GetFunctionFromFrame(frame);
    CallNGCRuntime(glue, RTSTUB_ID(EndCallTimer), { glue, curFunc });
#endif
    GateRef currentSp = *varSp;
    varSp = Load(VariableType::NATIVE_POINTER(), frame,
        IntPtr(AsmInterpretedFrame::GetBaseOffset(env->IsArch32Bit())));
    GateRef prevState = GetFrame(*varSp);
    varPc = GetPcFromFrame(prevState);
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcEqualNullptr, &pcNotEqualNullptr);
    Bind(&pcEqualNullptr);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { *varAcc, *varSp, currentSp });
        Return();
    }
    Bind(&pcNotEqualNullptr);
    {
        GateRef function = GetFunctionFromFrame(prevState);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        GateRef jumpSize = GetCallSizeFromFrame(prevState);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndDispatch),
                    { glue, currentSp, *varPc, *varConstpool, *varProfileTypeInfo,
                      *varAcc, *varHotnessCounter, jumpSize });
        Return();
    }
}

DECLARE_ASM_HANDLER(HandleAsyncgeneratorrejectV8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef asyncGenerator = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_0(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncGeneratorReject),
                              { asyncGenerator, acc });
    CHECK_EXCEPTION_VARACC(res, INT_PTR(ASYNCGENERATORREJECT_V8));
}

DECLARE_ASM_HANDLER(HandleSetgeneratorstateImm8)
{
    GateRef index = ReadInst8_0(pc);
    GateRef value = acc;
    CallRuntime(glue, RTSTUB_ID(SetGeneratorState), { value, IntToTaggedInt(index) });
    DISPATCH(SETGENERATORSTATE_IMM8);
}

DECLARE_ASM_HANDLER(HandleDeprecatedAsyncgeneratorrejectPrefV8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef asyncGenerator = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef value = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(AsyncGeneratorReject),
                              { asyncGenerator, value });
    CHECK_EXCEPTION_VARACC(res, INT_PTR(DEPRECATED_ASYNCGENERATORREJECT_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleSupercallthisrangeImm8Imm8V8)
{
    GateRef range = ReadInst8_1(pc);
    GateRef v0 = ZExtInt8ToInt16(ReadInst8_2(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuperCall),
        { currentFunc, Int16ToTaggedInt(v0), Int8ToTaggedInt(range) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(SUPERCALLTHISRANGE_IMM8_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleSupercallarrowrangeImm8Imm8V8)
{
    GateRef range = ReadInst8_1(pc);
    GateRef v0 = ZExtInt8ToInt16(ReadInst8_2(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuperCall),
        { acc, Int16ToTaggedInt(v0), Int8ToTaggedInt(range) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(SUPERCALLARROWRANGE_IMM8_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleWideSupercallthisrangePrefImm16V8)
{
    GateRef range = ReadInst16_1(pc);
    GateRef v0 = ZExtInt8ToInt16(ReadInst8_3(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuperCall),
        { currentFunc, Int16ToTaggedInt(v0), Int16ToTaggedInt(range) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleWideSupercallarrowrangePrefImm16V8)
{
    GateRef range = ReadInst16_1(pc);
    GateRef v0 = ZExtInt8ToInt16(ReadInst8_3(pc));
    GateRef res = CallRuntime(glue, RTSTUB_ID(SuperCall),
        { acc, Int16ToTaggedInt(v0), Int16ToTaggedInt(range) });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleLdsuperbyvalueImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), {  receiver, propKey }); // sp for thisFunc
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDSUPERBYVALUE_IMM8_V8));
}
DECLARE_ASM_HANDLER(HandleLdsuperbyvalueImm16V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = acc;
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), {  receiver, propKey }); // sp for thisFunc
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDSUPERBYVALUE_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdsuperbyvaluePrefV8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), {  receiver, propKey }); // sp for thisFunc
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_LDSUPERBYVALUE_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedGetiteratornextPrefV8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef method = GetVregValue(sp, ZExtInt8ToPtr(v1));
    GateRef result = CallRuntime(glue, RTSTUB_ID(GetIteratorNext), { obj, method });
    CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_GETITERATORNEXT_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleLdobjbyvalueImm8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = acc;
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.LoadObjByValue(glue, receiver, propKey, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDOBJBYVALUE_IMM8_V8));
}

DECLARE_ASM_HANDLER(HandleLdobjbyvalueImm16V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = acc;
    GateRef slotId = ZExtInt8ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.LoadObjByValue(glue, receiver, propKey, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDOBJBYVALUE_IMM16_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdobjbyvaluePrefV8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef v0 = ReadInst8_1(pc);
    GateRef v1 = ReadInst8_2(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v1));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.DeprecatedLoadObjByValue(glue, receiver, propKey);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(DEPRECATED_LDOBJBYVALUE_PREF_V8_V8));
}

DECLARE_ASM_HANDLER(HandleLdsuperbynameImm8Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst16_1(pc);
    GateRef receiver = acc;
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), { receiver, propKey });
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDSUPERBYNAME_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleLdsuperbynameImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst16_2(pc);
    GateRef receiver = acc;
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), { receiver, propKey });
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDSUPERBYNAME_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdsuperbynamePrefId32V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef v0 = ReadInst8_5(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef propKey = GetStringFromConstPool(glue, constpool, stringId);
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdSuperByValue), { receiver, propKey });
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(DEPRECATED_LDSUPERBYNAME_PREF_ID32_V8));
}

DECLARE_ASM_HANDLER(HandleLdobjbyindexImm8Imm16)
{
    auto env = GetEnvironment();

    GateRef receiver = acc;
    GateRef index = ZExtInt16ToInt32(ReadInst16_1(pc));
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = GetPropertyByIndex(glue, receiver, index, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDOBJBYINDEX_IMM8_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(LdObjByIndex),
                                     { receiver, IntToTaggedInt(index), TaggedFalse(), Undefined() });
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDOBJBYINDEX_IMM8_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleLdobjbyindexImm16Imm16)
{
    auto env = GetEnvironment();

    GateRef receiver = acc;
    GateRef index = ZExtInt16ToInt32(ReadInst16_2(pc));
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = GetPropertyByIndex(glue, receiver, index, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDOBJBYINDEX_IMM16_IMM16));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(LdObjByIndex),
                                     { receiver, IntToTaggedInt(index), TaggedFalse(), Undefined() });
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(LDOBJBYINDEX_IMM16_IMM16));
    }
}

DECLARE_ASM_HANDLER(HandleWideLdobjbyindexPrefImm32)
{
    auto env = GetEnvironment();

    GateRef receiver = acc;
    GateRef index = ReadInst32_1(pc);
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = GetPropertyByIndex(glue, receiver, index, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(WIDE_LDOBJBYINDEX_PREF_IMM32));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(LdObjByIndex),
                                     { receiver, IntToTaggedInt(index), TaggedFalse(), Undefined() });
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(WIDE_LDOBJBYINDEX_PREF_IMM32));
    }
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdobjbyindexPrefV8Imm32)
{
    auto env = GetEnvironment();

    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef index = ReadInst32_2(pc);
    Label fastPath(env);
    Label slowPath(env);
    Branch(TaggedIsHeapObject(receiver), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        GateRef result = GetPropertyByIndex(glue, receiver, index, callback);
        Label notHole(env);
        Branch(TaggedIsHole(result), &slowPath, &notHole);
        Bind(&notHole);
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32));
    }
    Bind(&slowPath);
    {
        GateRef result = CallRuntime(glue, RTSTUB_ID(LdObjByIndex),
                                     { receiver, IntToTaggedInt(index), TaggedFalse(), Undefined() });
        CHECK_EXCEPTION_WITH_ACC(result, INT_PTR(DEPRECATED_LDOBJBYINDEX_PREF_V8_IMM32));
    }
}

DECLARE_ASM_HANDLER(HandleStconsttoglobalrecordImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst16_2(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(StGlobalRecord),
                                 { propKey, *varAcc, TaggedTrue() });
    CHECK_EXCEPTION_VARACC(result, INT_PTR(STCONSTTOGLOBALRECORD_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleSttoglobalrecordImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst16_2(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, ZExtInt16ToInt32(stringId));
    GateRef result = CallRuntime(glue, RTSTUB_ID(StGlobalRecord),
                                 { propKey, *varAcc, TaggedFalse() });
    CHECK_EXCEPTION_VARACC(result, INT_PTR(STTOGLOBALRECORD_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedStconsttoglobalrecordPrefId32)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, stringId);
    GateRef result = CallRuntime(glue, RTSTUB_ID(StGlobalRecord),
                                 { propKey, *varAcc, TaggedTrue() });
    CHECK_EXCEPTION_VARACC(result, INT_PTR(DEPRECATED_STCONSTTOGLOBALRECORD_PREF_ID32));
}

DECLARE_ASM_HANDLER(HandleDeprecatedStlettoglobalrecordPrefId32)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, stringId);
    GateRef result = CallRuntime(glue, RTSTUB_ID(StGlobalRecord),
                                 { propKey, *varAcc, TaggedFalse() });
    CHECK_EXCEPTION_VARACC(result, INT_PTR(DEPRECATED_STLETTOGLOBALRECORD_PREF_ID32));
}

DECLARE_ASM_HANDLER(HandleDeprecatedStclasstoglobalrecordPrefId32)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, stringId);
    GateRef result = CallRuntime(glue, RTSTUB_ID(StGlobalRecord),
                                 { propKey, *varAcc, TaggedFalse() });
    CHECK_EXCEPTION_VARACC(result, INT_PTR(DEPRECATED_STCLASSTOGLOBALRECORD_PREF_ID32));
}

DECLARE_ASM_HANDLER(HandleGetmodulenamespaceImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst8_0(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(GetModuleNamespaceByIndex), { IntToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(GETMODULENAMESPACE_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideGetmodulenamespacePrefImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst16_1(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(GetModuleNamespaceByIndex), { Int16ToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(WIDE_GETMODULENAMESPACE_PREF_IMM16);
}

DECLARE_ASM_HANDLER(HandleDeprecatedGetmodulenamespacePrefId32)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef prop = GetStringFromConstPool(glue, constpool, stringId);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(GetModuleNamespace), { prop });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(DEPRECATED_GETMODULENAMESPACE_PREF_ID32);
}

DECLARE_ASM_HANDLER(HandleLdlocalmodulevarImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst8_0(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(LdLocalModuleVarByIndex), { Int8ToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(LDLOCALMODULEVAR_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideLdlocalmodulevarPrefImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst16_1(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(LdLocalModuleVarByIndex), { Int16ToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(WIDE_LDLOCALMODULEVAR_PREF_IMM16);
}

DECLARE_ASM_HANDLER(HandleLdexternalmodulevarImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst8_0(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(LdExternalModuleVarByIndex), { Int8ToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(LDEXTERNALMODULEVAR_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideLdexternalmodulevarPrefImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst16_1(pc);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(LdExternalModuleVarByIndex), { Int16ToTaggedInt(index) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(WIDE_LDEXTERNALMODULEVAR_PREF_IMM16);
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdmodulevarPrefId32Imm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef stringId = ReadInst32_1(pc);
    GateRef flag = ZExtInt8ToInt32(ReadInst8_5(pc));
    GateRef key = GetStringFromConstPool(glue, constpool, stringId);
    GateRef moduleRef = CallRuntime(glue, RTSTUB_ID(LdModuleVar), { key, IntToTaggedInt(flag) });
    varAcc = moduleRef;
    DISPATCH_WITH_ACC(DEPRECATED_LDMODULEVAR_PREF_ID32_IMM8);
}

DECLARE_ASM_HANDLER(HandleStmodulevarImm8)
{
    GateRef index = ReadInst8_0(pc);
    GateRef value = acc;

    CallRuntime(glue, RTSTUB_ID(StModuleVarByIndex), { IntToTaggedInt(index), value });
    DISPATCH(STMODULEVAR_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideStmodulevarPrefImm16)
{
    GateRef index = ReadInst16_1(pc);
    GateRef value = acc;

    CallRuntime(glue, RTSTUB_ID(StModuleVarByIndex), { Int16ToTaggedInt(index), value });
    DISPATCH(WIDE_STMODULEVAR_PREF_IMM16);
}

DECLARE_ASM_HANDLER(HandleDeprecatedStmodulevarPrefId32)
{
    GateRef stringId = ReadInst32_1(pc);
    GateRef prop = GetStringFromConstPool(glue, constpool, stringId);
    GateRef value = acc;

    CallRuntime(glue, RTSTUB_ID(StModuleVar), { prop, value });
    DISPATCH(DEPRECATED_STMODULEVAR_PREF_ID32);
}

DECLARE_ASM_HANDLER(HandleNewlexenvImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    auto env = GetEnvironment();
    GateRef numVars = ReadInst8_0(pc);
    GateRef state = GetFrame(sp);
    auto parent = GetEnvFromFrame(state);
    NewObjectStubBuilder newBuilder(this);
    newBuilder.SetParameters(glue, 0);
    Label afterNew(env);
    newBuilder.NewLexicalEnv(&result, &afterNew, ZExtInt16ToInt32(numVars), parent);
    Bind(&afterNew);
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    varAcc = *result;
    SetEnvToFrame(glue, GetFrame(sp), *result);
    DISPATCH_WITH_ACC(NEWLEXENV_IMM8);
}

DECLARE_ASM_HANDLER(HandleWideNewlexenvPrefImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(result, VariableType::JS_ANY(), Hole());
    auto env = GetEnvironment();
    GateRef numVars = ReadInst16_1(pc);
    GateRef state = GetFrame(sp);
    auto parent = GetEnvFromFrame(state);
    NewObjectStubBuilder newBuilder(this);
    newBuilder.SetParameters(glue, 0);
    Label afterNew(env);
    newBuilder.NewLexicalEnv(&result, &afterNew, ZExtInt16ToInt32(numVars), parent);
    Bind(&afterNew);
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    varAcc = *result;
    SetEnvToFrame(glue, GetFrame(sp), *result);
    DISPATCH_WITH_ACC(WIDE_NEWLEXENV_PREF_IMM16);
}

DECLARE_ASM_HANDLER(HandleNewlexenvwithnameImm8Id16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef numVars = ZExtInt8ToInt16(ReadInst8_0(pc));
    GateRef scopeId = ReadInst16_1(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(NewLexicalEnvWithName),
                              { Int16ToTaggedInt(numVars), Int16ToTaggedInt(scopeId) });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
    Bind(&notException);
    varAcc = res;
    GateRef state = GetFrame(sp);
    SetEnvToFrame(glue, state, res);
    DISPATCH_WITH_ACC(NEWLEXENVWITHNAME_IMM8_ID16);
}

DECLARE_ASM_HANDLER(HandleWideNewlexenvwithnamePrefImm16Id16)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef numVars = ReadInst16_1(pc);
    GateRef scopeId = ReadInst16_3(pc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(NewLexicalEnvWithName),
                              { Int16ToTaggedInt(numVars), Int16ToTaggedInt(scopeId) });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(res, &notException);
    Bind(&notException);
    varAcc = res;
    GateRef state = GetFrame(sp);
    SetEnvToFrame(glue, state, res);
    DISPATCH_WITH_ACC(WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16);
}

DECLARE_ASM_HANDLER(HandleDefineclasswithbufferImm8Id16Id16Imm16V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef methodId = ReadInst16_1(pc);
    GateRef literalId = ReadInst16_3(pc);
    GateRef length = ReadInst16_5(pc);
    GateRef v0 = ReadInst8_7(pc);

    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef lexicalEnv = GetEnvFromFrame(GetFrame(sp));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateClassWithBuffer),
                              { proto, lexicalEnv, constpool,
                                Int16ToTaggedInt(methodId),
                                Int16ToTaggedInt(literalId), module });

    Label isException(env);
    Label isNotException(env);
    Branch(TaggedIsException(res), &isException, &isNotException);
    Bind(&isException);
    {
        DISPATCH_LAST_WITH_ACC();
    }
    Bind(&isNotException);
    SetLexicalEnvToFunction(glue, res, lexicalEnv);
    SetModuleToFunction(glue, res, module);
    CallRuntime(glue, RTSTUB_ID(SetClassConstructorLength), { res, Int16ToTaggedInt(length) });
    callback.ProfileDefineClass(res);
    varAcc = res;
    DISPATCH_WITH_ACC(DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8);
}

DECLARE_ASM_HANDLER(HandleDefineclasswithbufferImm16Id16Id16Imm16V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef methodId = ReadInst16_2(pc);
    GateRef literalId = ReadInst16_4(pc);
    GateRef length = ReadInst16_6(pc);
    GateRef v0 = ReadInst8_8(pc);

    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef lexicalEnv = GetEnvFromFrame(GetFrame(sp));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateClassWithBuffer),
                              { proto, lexicalEnv, constpool,
                                Int16ToTaggedInt(methodId),
                                Int16ToTaggedInt(literalId), module });

    Label isException(env);
    Label isNotException(env);
    Branch(TaggedIsException(res), &isException, &isNotException);
    Bind(&isException);
    {
        DISPATCH_LAST_WITH_ACC();
    }
    Bind(&isNotException);
    SetLexicalEnvToFunction(glue, res, lexicalEnv);
    SetModuleToFunction(glue, res, module);
    CallRuntime(glue, RTSTUB_ID(SetClassConstructorLength), { res, Int16ToTaggedInt(length) });
    callback.ProfileDefineClass(res);
    varAcc = res;
    DISPATCH_WITH_ACC(DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8);
}

DECLARE_ASM_HANDLER(HandleDeprecatedDefineclasswithbufferPrefId16Imm16Imm16V8V8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef methodId = ReadInst16_1(pc);
    GateRef literalId = ReadInst16_3(pc);
    GateRef length = ReadInst16_5(pc);
    GateRef v0 = ReadInst8_7(pc);
    GateRef v1 = ReadInst8_8(pc);

    GateRef lexicalEnv = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef proto = GetVregValue(sp, ZExtInt8ToPtr(v1));

    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateClassWithBuffer),
                              { proto, lexicalEnv, constpool,
                                Int16ToTaggedInt(methodId),
                                Int16ToTaggedInt(literalId), module });

    Label isException(env);
    Label isNotException(env);
    Branch(TaggedIsException(res), &isException, &isNotException);
    Bind(&isException);
    {
        DISPATCH_LAST_WITH_ACC();
    }
    Bind(&isNotException);
    SetLexicalEnvToFunction(glue, res, lexicalEnv);
    SetModuleToFunction(glue, res, module);
    CallRuntime(glue, RTSTUB_ID(SetClassConstructorLength), { res, Int16ToTaggedInt(length) });
    varAcc = res;
    DISPATCH_WITH_ACC(DEPRECATED_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8);
}

DECLARE_ASM_HANDLER(HandleLdobjbynameImm8Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef receiver = acc;
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.LoadObjByName(glue, receiver, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDOBJBYNAME_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleLdobjbynameImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    GateRef receiver = acc;
    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.LoadObjByName(glue, receiver, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDOBJBYNAME_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdobjbynamePrefId32V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef receiver = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_5(pc)));
    GateRef stringId = ReadInst32_1(pc);
    GateRef propKey = GetStringFromConstPool(glue, constpool, stringId);
    AccessObjectStubBuilder builder(this);
    GateRef result = builder.DeprecatedLoadObjByName(glue, receiver, propKey);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(DEPRECATED_LDOBJBYNAME_PREF_ID32_V8));
}

DECLARE_ASM_HANDLER(HandleCallarg0Imm8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG0);
    GateRef func = acc;
    GateRef jumpSize = INT_PTR(CALLARG0_IMM8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_ARG0, {}, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallarg0PrefV8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG0);
    GateRef funcReg = ReadInst8_1(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(funcReg));
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLARG0_PREF_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_ARG0, {}, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallarg1Imm8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG1);
    GateRef a0 = ReadInst8_1(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef jumpSize = INT_PTR(CALLARG1_IMM8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_ARG1, { a0Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallarg1PrefV8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG1);
    GateRef funcReg = ReadInst8_1(pc);
    GateRef a0 = ReadInst8_2(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(funcReg));
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLARG1_PREF_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_ARG1, { a0Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallargs2Imm8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS2);
    GateRef a0 = ReadInst8_1(pc);
    GateRef a1 = ReadInst8_2(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef jumpSize = INT_PTR(CALLARGS2_IMM8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_ARG2, { a0Value, a1Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallargs2PrefV8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS2);
    GateRef funcReg = ReadInst8_1(pc);
    GateRef a0 = ReadInst8_2(pc);
    GateRef a1 = ReadInst8_3(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(funcReg));
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLARGS2_PREF_V8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_ARG2, { a0Value, a1Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallargs3Imm8V8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS3);
    GateRef a0 = ReadInst8_1(pc);
    GateRef a1 = ReadInst8_2(pc);
    GateRef a2 = ReadInst8_3(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef a2Value = GetVregValue(sp, ZExtInt8ToPtr(a2));
    GateRef jumpSize = INT_PTR(CALLARGS3_IMM8_V8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_ARG3, { a0Value, a1Value, a2Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallargs3PrefV8V8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS3);
    GateRef funcReg = ReadInst8_1(pc);
    GateRef a0 = ReadInst8_2(pc);
    GateRef a1 = ReadInst8_3(pc);
    GateRef a2 = ReadInst8_4(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(funcReg));
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef a2Value = GetVregValue(sp, ZExtInt8ToPtr(a2));
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLARGS3_PREF_V8_V8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_ARG3, { a0Value, a1Value, a2Value }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallrangeImm8Imm8V8)
{
    GateRef actualNumArgs = ZExtInt8ToInt32(ReadInst8_1(pc));
    GateRef func = acc;
    GateRef argv = PtrAdd(sp, PtrMul(ZExtInt8ToPtr(ReadInst8_2(pc)), IntPtr(8))); // 8: byteSize
    GateRef jumpSize = INT_PTR(CALLRANGE_IMM8_IMM8_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_WITH_ARGV, { numArgs, argv }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleWideCallrangePrefImm16V8)
{
    GateRef actualNumArgs = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef func = acc;
    GateRef argv = PtrAdd(sp, PtrMul(ZExtInt8ToPtr(ReadInst8_2(pc)), IntPtr(8))); // 8: byteSize
    GateRef jumpSize = INT_PTR(WIDE_CALLRANGE_PREF_IMM16_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_WITH_ARGV, { numArgs, argv }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallrangePrefImm16V8)
{
    GateRef actualNumArgs = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef funcReg = ReadInst8_3(pc);
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(funcReg));
    GateRef argv = PtrAdd(sp, PtrMul(
        PtrAdd(ZExtInt8ToPtr(funcReg), IntPtr(1)), IntPtr(8))); // 1: skip function
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLRANGE_PREF_IMM16_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_WITH_ARGV, { numArgs, argv }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallthisrangeImm8Imm8V8)
{
    GateRef actualNumArgs = ZExtInt8ToInt32(ReadInst8_1(pc));
    GateRef thisReg = ZExtInt8ToPtr(ReadInst8_2(pc));
    GateRef func = acc;
    GateRef thisValue = GetVregValue(sp, thisReg);
    GateRef argv = PtrAdd(sp, PtrMul(
        PtrAdd(thisReg, IntPtr(1)), IntPtr(8))); // 1: skip this
    GateRef jumpSize = INT_PTR(CALLTHISRANGE_IMM8_IMM8_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_WITH_ARGV, { numArgs, argv, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleWideCallthisrangePrefImm16V8)
{
    GateRef actualNumArgs = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef thisReg = ZExtInt8ToPtr(ReadInst8_3(pc));
    GateRef func = acc;
    GateRef thisValue = GetVregValue(sp, thisReg);
    GateRef argv = PtrAdd(sp, PtrMul(
        PtrAdd(thisReg, IntPtr(1)), IntPtr(8))); // 1: skip this
    GateRef jumpSize = INT_PTR(WIDE_CALLTHISRANGE_PREF_IMM16_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_WITH_ARGV, { numArgs, argv, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallthisrangePrefImm16V8)
{
    GateRef actualNumArgs = Int32Sub(ZExtInt16ToInt32(ReadInst16_1(pc)), Int32(1));  // 1: exclude this
    GateRef funcReg = ReadInst8_3(pc);
    funcReg = ZExtInt8ToPtr(funcReg);
    GateRef func = GetVregValue(sp, funcReg);
    GateRef thisValue = GetVregValue(sp, PtrAdd(funcReg, IntPtr(1)));
    GateRef argv = PtrAdd(sp, PtrMul(
        PtrAdd(funcReg, IntPtr(2)), IntPtr(8))); // 2: skip function&this
    GateRef jumpSize = INT_PTR(DEPRECATED_CALLTHISRANGE_PREF_IMM16_V8);
    GateRef numArgs = ZExtInt32ToPtr(actualNumArgs);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV, { numArgs, argv, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallthis0Imm8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG0);
    GateRef thisValue = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef func = acc;
    GateRef jumpSize = INT_PTR(CALLTHIS0_IMM8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_ARG0, { thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallthis1Imm8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARG1);
    GateRef thisValue = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef a0 = ReadInst8_2(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef jumpSize = INT_PTR(CALLTHIS1_IMM8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_ARG1, { a0Value, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallthis2Imm8V8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS2);
    GateRef thisValue = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef a0 = ReadInst8_2(pc);
    GateRef a1 = ReadInst8_3(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef jumpSize = INT_PTR(CALLTHIS2_IMM8_V8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_ARG2, { a0Value, a1Value, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCallthis3Imm8V8V8V8V8)
{
    GateRef actualNumArgs = Int32(InterpreterAssembly::ActualNumArgsOfCall::CALLARGS3);
    GateRef thisValue = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef a0 = ReadInst8_2(pc);
    GateRef a1 = ReadInst8_3(pc);
    GateRef a2 = ReadInst8_4(pc);
    GateRef func = acc;
    GateRef a0Value = GetVregValue(sp, ZExtInt8ToPtr(a0));
    GateRef a1Value = GetVregValue(sp, ZExtInt8ToPtr(a1));
    GateRef a2Value = GetVregValue(sp, ZExtInt8ToPtr(a2));
    GateRef jumpSize = INT_PTR(CALLTHIS3_IMM8_V8_V8_V8_V8);
    GateRef res = JSCallDispatch(glue, func, actualNumArgs, jumpSize, hotnessCounter,
                                 JSCallMode::CALL_THIS_ARG3, { a0Value, a1Value, a2Value, thisValue }, callback);
    CHECK_PENDING_EXCEPTION(res, jumpSize);
}

DECLARE_ASM_HANDLER(HandleCreatearraywithbufferImm8Id16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));

    NewObjectStubBuilder newBuilder(this);
    GateRef res = newBuilder.CreateArrayWithBuffer(glue, imm, currentFunc, callback);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEARRAYWITHBUFFER_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleCreatearraywithbufferImm16Id16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_2(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));

    NewObjectStubBuilder newBuilder(this);
    GateRef res = newBuilder.CreateArrayWithBuffer(glue, imm, currentFunc, callback);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEARRAYWITHBUFFER_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedCreatearraywithbufferPrefImm16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));

    NewObjectStubBuilder newBuilder(this);
    GateRef res = newBuilder.CreateArrayWithBuffer(glue, imm, currentFunc, callback);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_CREATEARRAYWITHBUFFER_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleCreateobjectwithbufferImm8Id16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef result = GetObjectLiteralFromConstPool(glue, constpool, imm, module);
    GateRef currentEnv = GetEnvFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectHavingMethod), { result, currentEnv });
    callback.ProfileCreateObject(res);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEOBJECTWITHBUFFER_IMM8_ID16));
}

DECLARE_ASM_HANDLER(HandleCreateobjectwithbufferImm16Id16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_2(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef result = GetObjectLiteralFromConstPool(glue, constpool, imm, module);
    GateRef currentEnv = GetEnvFromFrame(GetFrame(sp));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectHavingMethod), { result, currentEnv });
    callback.ProfileCreateObject(res);
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(CREATEOBJECTWITHBUFFER_IMM16_ID16));
}

DECLARE_ASM_HANDLER(HandleDeprecatedCreateobjectwithbufferPrefImm16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef result = GetObjectLiteralFromConstPool(glue, constpool, imm, module);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectWithBuffer), { result });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_CREATEOBJECTWITHBUFFER_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleNewobjrangeImm8Imm8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(res, VariableType::JS_ANY(), Undefined());
    DEFVARIABLE(thisObj, VariableType::JS_ANY(), Undefined());
    auto env = GetEnvironment();
    GateRef numArgs = ZExtInt8ToInt16(ReadInst8_1(pc));
    GateRef firstArgRegIdx = ZExtInt8ToInt16(ReadInst8_2(pc));
    GateRef firstArgOffset = Int16(1);
    GateRef ctor = GetVregValue(sp, ZExtInt16ToPtr(firstArgRegIdx));
    GateRef actualNumArgs = ZExtInt16ToInt32(Int16Sub(numArgs, firstArgOffset));

    Label ctorIsHeapObject(env);
    Label ctorIsJSFunction(env);
    Label fastPath(env);
    Label slowPath(env);
    Label checkResult(env);
    Label threadCheck(env);
    Label dispatch(env);
    Label ctorIsBase(env);
    Label ctorNotBase(env);
    Label isException(env);

    Branch(TaggedIsHeapObject(ctor), &ctorIsHeapObject, &slowPath);
    Bind(&ctorIsHeapObject);
    Branch(IsJSFunction(ctor), &ctorIsJSFunction, &slowPath);
    Bind(&ctorIsJSFunction);
    Branch(IsConstructor(ctor), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        Branch(IsBase(ctor), &ctorIsBase, &ctorNotBase);
        Bind(&ctorIsBase);
        {
            NewObjectStubBuilder newBuilder(this);
            thisObj = newBuilder.FastNewThisObject(glue, ctor);
            Branch(HasPendingException(glue), &isException, &ctorNotBase);
        }
        Bind(&ctorNotBase);
        GateRef argv = PtrAdd(sp, PtrMul(
            PtrAdd(firstArgRegIdx, firstArgOffset), IntPtr(8))); // 8: skip function
        GateRef jumpSize = IntPtr(-BytecodeInstruction::Size(BytecodeInstruction::Format::IMM8_IMM8_V8));
        res = JSCallDispatch(glue, ctor, actualNumArgs, jumpSize, hotnessCounter,
                             JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV,
                             { ZExtInt32ToPtr(actualNumArgs), argv, *thisObj }, callback);
        Jump(&threadCheck);
    }
    Bind(&slowPath);
    GateRef firstArgIdx = Int16Add(firstArgRegIdx, firstArgOffset);
    GateRef length = Int16Sub(numArgs, firstArgOffset);
    res = CallRuntime(glue, RTSTUB_ID(NewObjRange),
        { ctor, ctor, Int16ToTaggedInt(firstArgIdx), Int16ToTaggedInt(length) });
    Jump(&checkResult);
    Bind(&checkResult);
    {
        Branch(TaggedIsException(*res), &isException, &dispatch);
    }
    Bind(&threadCheck);
    {
        Branch(HasPendingException(glue), &isException, &dispatch);
    }
    Bind(&isException);
    {
        DISPATCH_LAST();
    }
    Bind(&dispatch);
    varAcc = *res;
    DISPATCH_WITH_ACC(NEWOBJRANGE_IMM8_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleNewobjrangeImm16Imm8V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(res, VariableType::JS_ANY(), Undefined());
    DEFVARIABLE(thisObj, VariableType::JS_ANY(), Undefined());
    auto env = GetEnvironment();
    GateRef numArgs = ZExtInt8ToInt16(ReadInst8_2(pc));
    GateRef firstArgRegIdx = ZExtInt8ToInt16(ReadInst8_3(pc));
    GateRef firstArgOffset = Int16(1);
    GateRef ctor = GetVregValue(sp, ZExtInt16ToPtr(firstArgRegIdx));
    GateRef actualNumArgs = ZExtInt16ToInt32(Int16Sub(numArgs, firstArgOffset));

    Label ctorIsHeapObject(env);
    Label ctorIsJSFunction(env);
    Label fastPath(env);
    Label slowPath(env);
    Label checkResult(env);
    Label threadCheck(env);
    Label dispatch(env);
    Label ctorIsBase(env);
    Label ctorNotBase(env);
    Label isException(env);

    Branch(TaggedIsHeapObject(ctor), &ctorIsHeapObject, &slowPath);
    Bind(&ctorIsHeapObject);
    Branch(IsJSFunction(ctor), &ctorIsJSFunction, &slowPath);
    Bind(&ctorIsJSFunction);
    Branch(IsConstructor(ctor), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        Branch(IsBase(ctor), &ctorIsBase, &ctorNotBase);
        Bind(&ctorIsBase);
        {
            NewObjectStubBuilder newBuilder(this);
            thisObj = newBuilder.FastNewThisObject(glue, ctor);
            Branch(HasPendingException(glue), &isException, &ctorNotBase);
        }
        Bind(&ctorNotBase);
        GateRef argv = PtrAdd(sp, PtrMul(
            PtrAdd(firstArgRegIdx, firstArgOffset), IntPtr(8))); // 8: skip function
        GateRef jumpSize =
            IntPtr(-static_cast<int64_t>(BytecodeInstruction::Size(BytecodeInstruction::Format::IMM16_IMM8_V8)));
        res = JSCallDispatch(glue, ctor, actualNumArgs, jumpSize, hotnessCounter,
                             JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV,
                             { ZExtInt32ToPtr(actualNumArgs), argv, *thisObj }, callback);
        Jump(&threadCheck);
    }
    Bind(&slowPath);
    GateRef firstArgIdx = Int16Add(firstArgRegIdx, firstArgOffset);
    GateRef length = Int16Sub(numArgs, firstArgOffset);
    res = CallRuntime(glue, RTSTUB_ID(NewObjRange),
        { ctor, ctor, Int16ToTaggedInt(firstArgIdx), Int16ToTaggedInt(length) });
    Jump(&checkResult);
    Bind(&checkResult);
    {
        Branch(TaggedIsException(*res), &isException, &dispatch);
    }
    Bind(&threadCheck);
    {
        Branch(HasPendingException(glue), &isException, &dispatch);
    }
    Bind(&isException);
    {
        DISPATCH_LAST();
    }
    Bind(&dispatch);
    varAcc = *res;
    DISPATCH_WITH_ACC(NEWOBJRANGE_IMM16_IMM8_V8);
}

DECLARE_ASM_HANDLER(HandleWideNewobjrangePrefImm16V8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(res, VariableType::JS_ANY(), Undefined());
    DEFVARIABLE(thisObj, VariableType::JS_ANY(), Undefined());
    auto env = GetEnvironment();
    GateRef numArgs = ReadInst16_1(pc);
    GateRef firstArgRegIdx = ZExtInt8ToInt16(ReadInst8_3(pc));
    GateRef firstArgOffset = Int16(1);
    GateRef ctor = GetVregValue(sp, ZExtInt16ToPtr(firstArgRegIdx));
    GateRef actualNumArgs = ZExtInt16ToInt32(Int16Sub(numArgs, firstArgOffset));

    Label ctorIsHeapObject(env);
    Label ctorIsJSFunction(env);
    Label fastPath(env);
    Label slowPath(env);
    Label checkResult(env);
    Label threadCheck(env);
    Label dispatch(env);
    Label ctorIsBase(env);
    Label ctorNotBase(env);
    Label isException(env);

    Branch(TaggedIsHeapObject(ctor), &ctorIsHeapObject, &slowPath);
    Bind(&ctorIsHeapObject);
    Branch(IsJSFunction(ctor), &ctorIsJSFunction, &slowPath);
    Bind(&ctorIsJSFunction);
    Branch(IsConstructor(ctor), &fastPath, &slowPath);
    Bind(&fastPath);
    {
        Branch(IsBase(ctor), &ctorIsBase, &ctorNotBase);
        Bind(&ctorIsBase);
        {
            NewObjectStubBuilder newBuilder(this);
            thisObj = newBuilder.FastNewThisObject(glue, ctor);
            Branch(HasPendingException(glue), &isException, &ctorNotBase);
        }
        Bind(&ctorNotBase);
        GateRef argv = PtrAdd(sp, PtrMul(
            PtrAdd(firstArgRegIdx, firstArgOffset), IntPtr(8))); // 8: skip function
        GateRef jumpSize = IntPtr(-BytecodeInstruction::Size(BytecodeInstruction::Format::PREF_IMM16_V8));
        res = JSCallDispatch(glue, ctor, actualNumArgs, jumpSize, hotnessCounter,
                             JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV,
                             { ZExtInt32ToPtr(actualNumArgs), argv, *thisObj }, callback);
        Jump(&threadCheck);
    }
    Bind(&slowPath);
    GateRef firstArgIdx = Int16Add(firstArgRegIdx, firstArgOffset);
    GateRef length = Int16Sub(numArgs, firstArgOffset);
    res = CallRuntime(glue, RTSTUB_ID(NewObjRange),
        { ctor, ctor, Int16ToTaggedInt(firstArgIdx), Int16ToTaggedInt(length) });
    Jump(&checkResult);
    Bind(&checkResult);
    {
        Branch(TaggedIsException(*res), &isException, &dispatch);
    }
    Bind(&threadCheck);
    {
        Branch(HasPendingException(glue), &isException, &dispatch);
    }
    Bind(&isException);
    {
        DISPATCH_LAST();
    }
    Bind(&dispatch);
    varAcc = *res;
    DISPATCH_WITH_ACC(WIDE_NEWOBJRANGE_PREF_IMM16_V8);
}

DECLARE_ASM_HANDLER(HandleDefinefuncImm8Id16Imm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef methodId = ReadInst16_1(pc);
    GateRef length = ReadInst8_3(pc);
    DEFVARIABLE(result, VariableType::JS_POINTER(),
        GetMethodFromConstPool(glue, constpool, ZExtInt16ToInt32(methodId)));
    result = CallRuntime(glue, RTSTUB_ID(DefineFunc), { *result });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    {
        GateRef hclass = LoadHClass(*result);
        SetPropertyInlinedProps(glue, *result, hclass, Int16ToTaggedInt(length),
            Int32(JSFunction::LENGTH_INLINE_PROPERTY_INDEX), VariableType::INT64());
        auto frame = GetFrame(sp);
        GateRef envHandle = GetEnvFromFrame(frame);
        SetLexicalEnvToFunction(glue, *result, envHandle);
        GateRef currentFunc = GetFunctionFromFrame(frame);
        SetModuleToFunction(glue, *result, GetModuleFromFunction(currentFunc));
        SetHomeObjectToFunction(glue, *result, GetHomeObjectFromFunction(currentFunc));
        varAcc = *result;
        DISPATCH_WITH_ACC(DEFINEFUNC_IMM8_ID16_IMM8);
    }
}

DECLARE_ASM_HANDLER(HandleDefinefuncImm16Id16Imm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef methodId = ReadInst16_2(pc);
    GateRef length = ReadInst8_4(pc);
    DEFVARIABLE(result, VariableType::JS_POINTER(),
        GetMethodFromConstPool(glue, constpool, ZExtInt16ToInt32(methodId)));
    result = CallRuntime(glue, RTSTUB_ID(DefineFunc), { *result });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    {
        GateRef hclass = LoadHClass(*result);
        SetPropertyInlinedProps(glue, *result, hclass, Int8ToTaggedInt(length),
            Int32(JSFunction::LENGTH_INLINE_PROPERTY_INDEX), VariableType::INT64());
        auto frame = GetFrame(sp);
        GateRef envHandle = GetEnvFromFrame(frame);
        SetLexicalEnvToFunction(glue, *result, envHandle);
        GateRef currentFunc = GetFunctionFromFrame(frame);
        SetModuleToFunction(glue, *result, GetModuleFromFunction(currentFunc));
        SetHomeObjectToFunction(glue, *result, GetHomeObjectFromFunction(currentFunc));
        varAcc = *result;
        DISPATCH_WITH_ACC(DEFINEFUNC_IMM16_ID16_IMM8);
    }
}

DECLARE_ASM_HANDLER(HandleDefinemethodImm8Id16Imm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef methodId = ReadInst16_1(pc);
    GateRef length = ReadInst8_3(pc);
    DEFVARIABLE(result, VariableType::JS_POINTER(),
        GetMethodFromConstPool(glue, constpool, ZExtInt16ToInt32(methodId)));
    result = CallRuntime(glue, RTSTUB_ID(DefineMethod), { *result, acc });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    {
        GateRef hclass = LoadHClass(*result);
        SetPropertyInlinedProps(glue, *result, hclass, Int8ToTaggedInt(length),
            Int32(JSFunction::LENGTH_INLINE_PROPERTY_INDEX), VariableType::INT64());
        GateRef lexEnv = GetEnvFromFrame(GetFrame(sp));
        SetLexicalEnvToFunction(glue, *result, lexEnv);
        GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
        SetModuleToFunction(glue, *result, GetModuleFromFunction(currentFunc));
        varAcc = *result;
        DISPATCH_WITH_ACC(DEFINEMETHOD_IMM8_ID16_IMM8);
    }
}

DECLARE_ASM_HANDLER(HandleDefinemethodImm16Id16Imm8)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef methodId = ReadInst16_2(pc);
    GateRef length = ReadInst8_4(pc);
    DEFVARIABLE(result, VariableType::JS_POINTER(),
        GetMethodFromConstPool(glue, constpool, ZExtInt16ToInt32(methodId)));
    result = CallRuntime(glue, RTSTUB_ID(DefineMethod), { *result, acc });
    Label notException(env);
    CHECK_EXCEPTION_WITH_JUMP(*result, &notException);
    Bind(&notException);
    {
        GateRef hclass = LoadHClass(*result);
        SetPropertyInlinedProps(glue, *result, hclass, Int8ToTaggedInt(length),
            Int32(JSFunction::LENGTH_INLINE_PROPERTY_INDEX), VariableType::INT64());
        GateRef lexEnv = GetEnvFromFrame(GetFrame(sp));
        SetLexicalEnvToFunction(glue, *result, lexEnv);
        GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
        SetModuleToFunction(glue, *result, GetModuleFromFunction(currentFunc));
        varAcc = *result;
        DISPATCH_WITH_ACC(DEFINEMETHOD_IMM16_ID16_IMM8);
    }
}

DECLARE_ASM_HANDLER(HandleApplyImm8V8V8)
{
    GateRef func = acc;
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef array = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CallSpread), { func, obj, array });
    CHECK_PENDING_EXCEPTION(res, INT_PTR(APPLY_IMM8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleDeprecatedCallspreadPrefV8V8V8)
{
    GateRef func = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_1(pc)));
    GateRef obj = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_2(pc)));
    GateRef array = GetVregValue(sp, ZExtInt8ToPtr(ReadInst8_3(pc)));
    GateRef res = CallRuntime(glue, RTSTUB_ID(CallSpread), { func, obj, array });
    CHECK_PENDING_EXCEPTION(res, INT_PTR(DEPRECATED_CALLSPREAD_PREF_V8_V8_V8));
}

DECLARE_ASM_HANDLER(HandleThrowNotexistsPrefNone)
{
    CallRuntime(glue, RTSTUB_ID(ThrowThrowNotExists), {});
    DISPATCH_LAST();
}
DECLARE_ASM_HANDLER(HandleThrowPrefNone)
{
    CallRuntime(glue, RTSTUB_ID(Throw), { acc });
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleJnstricteqV8Imm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstricteqV8Imm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqV8Imm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqV8Imm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJneV8Imm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJneV8Imm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJeqV8Imm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJeqV8Imm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstrictequndefinedImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstrictequndefinedImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstrictequndefinedImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstrictequndefinedImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJneundefinedImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJneundefinedImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstricteqnullImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstricteqnullImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqnullImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqnullImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnenullImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnenullImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJeqnullImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJeqnullImm8)
{
    DISPATCH(NOP);
}

DECLARE_ASM_HANDLER(HandleJnstricteqzImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJnstricteqzImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqzImm16)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleJstricteqzImm8)
{
    DISPATCH(NOP);
}
DECLARE_ASM_HANDLER(HandleStthisbyvalueImm16V8)
{
    GateRef v0 = ReadInst8_2(pc);
    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef value = acc;
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.StoreObjByValue(glue, receiver, propKey, value, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STTHISBYVALUE_IMM16_V8));
}
DECLARE_ASM_HANDLER(HandleStthisbyvalueImm8V8)
{
    GateRef v0 = ReadInst8_1(pc);
    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef propKey = GetVregValue(sp, ZExtInt8ToPtr(v0));
    GateRef value = acc;
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.StoreObjByValue(glue, receiver, propKey, value, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STTHISBYVALUE_IMM8_V8));
}
DECLARE_ASM_HANDLER(HandleLdthisbyvalueImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef propKey = acc;
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.LoadObjByValue(glue, receiver, propKey, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDTHISBYVALUE_IMM16));
}
DECLARE_ASM_HANDLER(HandleLdthisbyvalueImm8)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef propKey = acc;
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    GateRef result = builder.LoadObjByValue(glue, receiver, propKey, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDTHISBYVALUE_IMM8));
}
DECLARE_ASM_HANDLER(HandleStthisbynameImm16Id16)
{
    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.StoreObjByName(glue, receiver, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STTHISBYNAME_IMM16_ID16));
}
DECLARE_ASM_HANDLER(HandleStthisbynameImm8Id16)
{
    GateRef receiver = GetThisFromFrame(GetFrame(sp));
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.StoreObjByName(glue, receiver, 0, info, acc, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION(result, INT_PTR(STTHISBYNAME_IMM8_ID16));
}
DECLARE_ASM_HANDLER(HandleLdthisbynameImm16Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef slotId = ZExtInt16ToInt32(ReadInst16_0(pc));
    GateRef receiver = GetThisFromFrame(GetFrame(sp));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_2, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.LoadObjByName(glue, receiver, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDTHISBYNAME_IMM16_ID16));
}
DECLARE_ASM_HANDLER(HandleLdthisbynameImm8Id16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef slotId = ZExtInt8ToInt32(ReadInst8_0(pc));
    GateRef receiver = GetThisFromFrame(GetFrame(sp));

    AccessObjectStubBuilder builder(this);
    StringIdInfo info = { constpool, pc, StringIdInfo::Offset::BYTE_1, StringIdInfo::Length::BITS_16 };
    GateRef result = builder.LoadObjByName(glue, receiver, 0, info, profileTypeInfo, slotId, callback);
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(LDTHISBYNAME_IMM8_ID16));
}
DECLARE_ASM_HANDLER(HandleLdthis)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = GetThisFromFrame(GetFrame(sp));
    DISPATCH_WITH_ACC(LDTHIS);
}
DECLARE_ASM_HANDLER(HandleLdnewtarget)
{
    DISPATCH(NOP);
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdlexenvPrefNone)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef state = GetFrame(sp);
    varAcc = GetEnvFromFrame(state);
    DISPATCH_WITH_ACC(DEPRECATED_LDLEXENV_PREF_NONE);
}

DECLARE_ASM_HANDLER(HandleDeprecatedLdhomeobjectPrefNone)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    GateRef state = GetFunctionFromFrame(GetFrame(sp));
    varAcc = GetHomeObjectFromJSFunction(state);
    DISPATCH_WITH_ACC(DEPRECATED_LDHOMEOBJECT_PREF_NONE);
}

DECLARE_ASM_HANDLER(HandleDeprecatedCreateobjecthavingmethodPrefImm16)
{
    GateRef imm = ZExtInt16ToInt32(ReadInst16_1(pc));
    GateRef currentFunc = GetFunctionFromFrame(GetFrame(sp));
    GateRef module = GetModuleFromFunction(currentFunc);
    GateRef result = GetObjectLiteralFromConstPool(glue, constpool, imm, module);
    GateRef res = CallRuntime(glue, RTSTUB_ID(CreateObjectHavingMethod), { result, acc });
    CHECK_EXCEPTION_WITH_ACC(res, INT_PTR(DEPRECATED_CREATEOBJECTHAVINGMETHOD_PREF_IMM16));
}

#define DECLARE_UNUSED_ASM_HANDLE(name)                           \
    DECLARE_ASM_HANDLER(name)                                     \
    {                                                             \
        FatalPrint(glue, { Int32(GET_MESSAGE_STRING_ID(name)) }); \
        DISPATCH_BAK(OFFSET, IntPtr(0));                          \
    }
ASM_UNUSED_BC_STUB_LIST(DECLARE_UNUSED_ASM_HANDLE)
#undef DECLARE_UNUSED_ASM_HANDLE

DECLARE_ASM_HANDLER_NOPRINT(HandleThrow)
{
    GateRef opcode = ZExtInt8ToPtr(ReadInst8_0(pc));
    auto index = IntPtr(kungfu::BytecodeStubCSigns::ID_Throw_Start);
    auto jumpIndex = PtrAdd(opcode, index);
    DispatchWithId(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, jumpIndex);
}

DECLARE_ASM_HANDLER_NOPRINT(HandleWide)
{
    GateRef opcode = ZExtInt8ToPtr(ReadInst8_0(pc));
    auto index = IntPtr(kungfu::BytecodeStubCSigns::ID_Wide_Start);
    auto jumpIndex = PtrAdd(opcode, index);
    DispatchWithId(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, jumpIndex);
}

DECLARE_ASM_HANDLER_NOPRINT(HandleDeprecated)
{
    GateRef opcode = ZExtInt8ToPtr(ReadInst8_0(pc));
    auto index = IntPtr(kungfu::BytecodeStubCSigns::ID_Deprecated_Start);
    auto jumpIndex = PtrAdd(opcode, index);
    DispatchWithId(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, jumpIndex);
}

DECLARE_ASM_HANDLER_NOPRINT(HandleCallRuntime)
{
    GateRef opcode = ZExtInt8ToPtr(ReadInst8_0(pc));
    auto index = IntPtr(kungfu::BytecodeStubCSigns::ID_CallRuntime_Start);
    auto jumpIndex = PtrAdd(opcode, index);
    DispatchWithId(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter, jumpIndex);
}

// interpreter helper handler
DECLARE_ASM_HANDLER_NOPRINT(ExceptionHandler)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    Label pcIsInvalid(env);
    Label pcNotInvalid(env);
    GateRef exceptionOffset = IntPtr(JSThread::GlueData::GetExceptionOffset(env->IsArch32Bit()));
    GateRef exception = Load(VariableType::JS_ANY(), glue, exceptionOffset);
    varPc = TaggedCastToIntPtr(CallRuntime(glue, RTSTUB_ID(UpFrame), {}));
    varSp = GetCurrentFrame(glue);
    Branch(IntPtrEqual(*varPc, IntPtr(0)), &pcIsInvalid, &pcNotInvalid);
    Bind(&pcIsInvalid);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeUncaughtFrameAndReturn), { glue, *varSp, *varAcc });
        Return();
    }
    Bind(&pcNotInvalid);
    {
        varAcc = exception;
        // clear exception
        Store(VariableType::INT64(), glue, glue, exceptionOffset, Hole());
        GateRef function = GetFunctionFromFrame(GetFrame(*varSp));
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varConstpool = GetConstpoolFromMethod(method);
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
        CallNGCRuntime(glue, RTSTUB_ID(ResumeCaughtFrameAndDispatch), {
            glue, *varSp, *varPc, *varConstpool,
            *varProfileTypeInfo, *varAcc, *varHotnessCounter});
        Return();
    }
}

DECLARE_ASM_HANDLER(SingleStepDebugging)
{
    auto env = GetEnvironment();
    DEFVARIABLE(varPc, VariableType::NATIVE_POINTER(), pc);
    DEFVARIABLE(varSp, VariableType::NATIVE_POINTER(), sp);
    DEFVARIABLE(varConstpool, VariableType::JS_POINTER(), constpool);
    DEFVARIABLE(varProfileTypeInfo, VariableType::JS_POINTER(), profileTypeInfo);
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    DEFVARIABLE(varHotnessCounter, VariableType::INT32(), hotnessCounter);

    GateRef frame = GetFrame(*varSp);
    SetPcToFrame(glue, frame, *varPc);
    GateRef currentSp = *varSp;
    varSp = TaggedCastToIntPtr(CallRuntime(glue,
                                           RTSTUB_ID(JumpToCInterpreter),
                                           { constpool, profileTypeInfo, acc,
                                             IntToTaggedInt(hotnessCounter)}));
    GateRef frameAfter = GetFrame(*varSp);
    varPc = GetPcFromFrame(frameAfter);
    Label shouldReturn(env);
    Label shouldContinue(env);

    Branch(IntPtrEqual(*varPc, IntPtr(0)), &shouldReturn, &shouldContinue);
    Bind(&shouldReturn);
    {
        CallNGCRuntime(glue, RTSTUB_ID(ResumeRspAndReturn), { Undefined(), *varSp, currentSp });
        Return();
    }
    Bind(&shouldContinue);
    {
        varAcc = GetAccFromFrame(frameAfter);
        GateRef function = GetFunctionFromFrame(frameAfter);
        GateRef method = Load(VariableType::JS_ANY(), function, IntPtr(JSFunctionBase::METHOD_OFFSET));
        varProfileTypeInfo = GetProfileTypeInfoFromMethod(method);
        varConstpool = GetConstpoolFromMethod(method);
        varHotnessCounter = GetHotnessCounterFromMethod(method);
    }
    Label isException(env);
    Label notException(env);
    Branch(TaggedIsException(*varAcc), &isException, &notException);
    Bind(&isException);
    DispatchLast(glue, *varSp, *varPc, *varConstpool, *varProfileTypeInfo, *varAcc,
                 *varHotnessCounter);
    Bind(&notException);
    DISPATCH_BAK(SSD, IntPtr(0));
}

DECLARE_ASM_HANDLER(BCDebuggerEntry)
{
    GateRef frame = GetFrame(sp);
    SetPcToFrame(glue, frame, pc);
    // NOTIFY_DEBUGGER_EVENT()
    CallRuntime(glue, RTSTUB_ID(NotifyBytecodePcChanged), {});
    // goto normal handle stub
    DispatchDebugger(glue, sp, pc, constpool, profileTypeInfo, acc, hotnessCounter);
}

DECLARE_ASM_HANDLER(BCDebuggerExceptionEntry)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);
    varAcc = Hole();
    GateRef frame = GetFrame(sp);
    SetPcToFrame(glue, frame, pc);
    // NOTIFY_DEBUGGER_EVENT()
    CallRuntime(glue, RTSTUB_ID(NotifyBytecodePcChanged), {});
    // goto last handle stub
    DispatchDebuggerLast(glue, sp, pc, constpool, profileTypeInfo, *varAcc, hotnessCounter);
}

DECLARE_ASM_HANDLER(NewObjectRangeThrowException)
{
    CallRuntime(glue, RTSTUB_ID(ThrowDerivedMustReturnException), {});
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(ThrowStackOverflowException)
{
    CallRuntime(glue, RTSTUB_ID(ThrowStackOverflowException), {});
    DISPATCH_LAST();
}

DECLARE_ASM_HANDLER(HandleWideLdpatchvarPrefImm16)
{
    DEFVARIABLE(varAcc, VariableType::JS_ANY(), acc);

    GateRef index = ReadInst16_1(pc);
    GateRef result = CallRuntime(glue, RTSTUB_ID(LdPatchVar), { Int16ToTaggedInt(index) });
    CHECK_EXCEPTION_WITH_VARACC(result, INT_PTR(WIDE_LDPATCHVAR_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleWideStpatchvarPrefImm16)
{
    GateRef index = ReadInst16_1(pc);
    GateRef result = CallRuntime(glue, RTSTUB_ID(StPatchVar), { Int16ToTaggedInt(index), acc });
    CHECK_EXCEPTION(result, INT_PTR(WIDE_STPATCHVAR_PREF_IMM16));
}

DECLARE_ASM_HANDLER(HandleCallRuntimeNotifyConcurrentResultPrefNone)
{
    GateRef funcObj = GetFunctionFromFrame(GetFrame(sp));
    CallRuntime(glue, RTSTUB_ID(NotifyConcurrentResult), {acc, funcObj});
    DISPATCH(CALLRUNTIME_NOTIFYCONCURRENTRESULT_PREF_NONE);
}

ASM_INTERPRETER_BC_TYPE_PROFILER_STUB_LIST(DECLARE_ASM_HANDLER_PROFILE)
ASM_INTERPRETER_BC_LAYOUT_PROFILER_STUB_LIST(DECLARE_ASM_HANDLER_PROFILE)
ASM_INTERPRETER_BC_FUNC_HOT_PROFILER_STUB_LIST(DECLARE_ASM_HANDLER_PROFILE)
ASM_INTERPRETER_BC_FUNC_COUNT_PROFILER_STUB_LIST(DECLARE_ASM_HANDLER_PROFILE)

#undef DECLARE_ASM_HANDLER
#undef DISPATCH
#undef DISPATCH_WITH_ACC
#undef DISPATCH_LAST
#undef DISPATCH_LAST_WITH_ACC
#undef USE_PARAMS
}  // namespace panda::ecmascript::kungfu
