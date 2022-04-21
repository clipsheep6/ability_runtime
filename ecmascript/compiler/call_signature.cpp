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

#include "ecmascript/compiler/call_signature.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "llvm-c/Core.h"
#include "llvm/Support/Host.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace panda::ecmascript::kungfu {
DEF_CALL_SIGNATURE(Add)
{
    // 3 : 3 input parameters
    CallSignature Add("Add", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY()); // number or hole
    *callSign = Add;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(Sub)
{
    // 3 : 3 input parameters
    CallSignature Sub("Sub", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY()); // number or hole
    *callSign = Sub;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(Mul)
{
    // 3 : 3 input parameters
    CallSignature Mul("Mul", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY()); // number or hole
    *callSign = Mul;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

#ifndef NDEBUG
DEF_CALL_SIGNATURE(MulGCTest)
{
    // 3 : 3 input parameters
    CallSignature MulGC("MulGCTest", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = MulGC;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::INT64(),
        VariableType::INT64(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}
#else
DEF_CALL_SIGNATURE(MulGCTest) {}
#endif

DEF_CALL_SIGNATURE(Div)
{
    // 3 : 3 input parameters
    CallSignature Div("Div", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY()); // float or hole
    *callSign = Div;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(Mod)
{
    // 3 : 3 input parameters
    CallSignature Mod("Mod", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY()); // int,float or hole
    *callSign = Mod;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TypeOf)
{
    // 2 input parameters
    CallSignature TypeOf("TypeOf", 0, 2, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_POINTER());
    *callSign = TypeOf;
    // 2 input parameters
    std::array<VariableType, 2> params = {
        VariableType::NATIVE_POINTER(), // glue
        VariableType::JS_ANY(), // ACC
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(Equal)
{
    // 3 input parameters, return may be true/false/hole
    CallSignature Equal("Equal", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = Equal;
    // 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByName)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByName("SetPropertyByName", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64());
    *callSign = setPropertyByName;

    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByNameWithOwn)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByNameWithOwn("SetPropertyByNameWithOwn", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64());
    *callSign = setPropertyByNameWithOwn;

    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByValue)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByName("SetPropertyByValue", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64());
    *callSign = setPropertyByName;

    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByValueWithOwn)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByValueWithOwn("SetPropertyByValueWithOwn", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64());
    *callSign = setPropertyByValueWithOwn;

    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(GetPropertyByName)
{
        // 3 : 3 input parameters
    CallSignature getPropertyByName("GetPropertyByName", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = getPropertyByName;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(), // glue
        VariableType::JS_ANY(),         // receiver
        VariableType::JS_POINTER(), // key
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(GetPropertyByIndex)
{
    // 3 : 3 input parameters
    CallSignature getPropertyByIndex("GetPropertyByIndex", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::JS_ANY());
    *callSign = getPropertyByIndex;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(), // glue
        VariableType::JS_ANY(), // receiver
        VariableType::INT32(), // index
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByIndex)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByIndex("SetPropertyByIndex", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64()); // hole or undefined
    *callSign = setPropertyByIndex;
    // 4 : 4 input parameters
    std::array<VariableType, 4> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::INT32(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetPropertyByIndexWithOwn)
{
    // 4 : 4 input parameters
    CallSignature setPropertyByIndexWithOwn("SetPropertyByIndexWithOwn", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::INT64()); // hole or undefined
    *callSign = setPropertyByIndexWithOwn;
    // 4 : 4 input parameters
    std::array<VariableType, 4> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::INT32(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(GetPropertyByValue)
{
    // 3 : 3 input parameters
    CallSignature getPropertyByValue("GetPropertyByValue", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
                                      VariableType::JS_ANY());
    *callSign = getPropertyByValue;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TryLoadICByName)
{
    // 4 : 4 input parameters
    CallSignature tryLoadICByName("TryLoadICByName", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = tryLoadICByName;
    // 4 : 4 input parameters
    std::array<VariableType, 4> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TryLoadICByValue)
{
    // 5 : 5 input parameters
    CallSignature tryLoadICByValue("TryLoadICByValue", 0, 5,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = tryLoadICByValue;
    // 5 : 5 input parameters
    std::array<VariableType, 5> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TryStoreICByName)
{
    // 5 : 5 input parameters
    CallSignature tryStoreICByName("TryStoreICByName", 0, 5,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::INT64()); // undefined or hole
    *callSign = tryStoreICByName;
    // 5 : 5 input parameters
    std::array<VariableType, 5> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TryStoreICByValue)
{
    // 6 : 6 input parameters
    CallSignature tryStoreICByValue("TryStoreICByValue", 0, 6,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::INT64()); // undefined or hole
    *callSign = tryStoreICByValue;
    // 6 : 6 input parameters
    std::array<VariableType, 6> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(SetValueWithBarrier)
{
    // 4 : 4 input parameters
    CallSignature setValueWithBarrier("SetValueWithBarrier", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        VariableType::VOID());
    *callSign = setValueWithBarrier;

    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(TestAbsoluteAddressRelocation)
{
    // 2 : 2 input parameters
    CallSignature TestAbsoluteAddressRelocation("TestAbsoluteAddressRelocation", 0, 2,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::INT64()); // undefined or hole
    *callSign = TestAbsoluteAddressRelocation;
    // 2 : 2 input parameters
    std::array<VariableType, 2> params = {
        VariableType::INT64(),
        VariableType::INT64(),
    };
    callSign->SetParameters(params.data());
}

DEF_CALL_SIGNATURE(GetTaggedArrayPtrTest)
{
    // 2 : 2 input parameters
    CallSignature getTaggedArrayPtr("GetTaggedArrayPtrTest", 0, 2, ArgumentsOrder::DEFAULT_ORDER,
                                     VariableType::JS_POINTER());
    *callSign = getTaggedArrayPtr;
    // 2 : 2 input parameters
    std::array<VariableType, 2> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(BytecodeHandler)
{
    // 7 : 7 input parameters
    CallSignature bytecodeHandler("bytecodeHandler", 0, 7,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = bytecodeHandler;
    std::array<VariableType, 7> params = { // 7 : 7 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::BYTECODE_HANDLER);
}

DEF_CALL_SIGNATURE(SingleStepDebugging)
{
    // 7 : 7 input parameters
    CallSignature singleStepDebugging("singleStepDebugging", 0, 7,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = singleStepDebugging;
    std::array<VariableType, 7> params = { // 7 : 7 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::GHCCallConv);
}

DEF_CALL_SIGNATURE(AsmInterpreterEntry)
{
    // 7 : 7 input parameters
    CallSignature asmInterpreterEntry("AsmInterpreterEntry", 0, 7,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = asmInterpreterEntry;
    std::array<VariableType, 7> params = { // 7 : 7 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(HandleOverflow)
{
    // 7 : 7 input parameters
    CallSignature handleOverflow("HandleOverflow", 0, 7,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = handleOverflow;
    std::array<VariableType, 7> params = { // 7 : 7 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
    };
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::GHCCallConv);
}

DEF_CALL_SIGNATURE(AsmIntCallRuntime)
{
    /* 3 : 3 input parameters */
    CallSignature asmIntCallRuntime("AsmIntCallRuntime", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = asmIntCallRuntime;
    std::array<VariableType, 3> params = { /* 3 : 3 input parameters */
        VariableType::NATIVE_POINTER(),
        VariableType::INT64(),
        VariableType::INT64(),
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::ASM_STUB);
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(OptimizedCallRuntime)
{
    /* 3 : 3 input parameters */
    CallSignature runtimeCallTrampoline("OptimizedCallRuntime", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = runtimeCallTrampoline;
    std::array<VariableType, 3> params = { /* 3 : 3 input parameters */
        VariableType::NATIVE_POINTER(),
        VariableType::INT64(),
        VariableType::INT64(),
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(OptimizedCallRuntimeWithArgv)
{
    /* 4 : 4 input parameters */
    CallSignature runtimeCallTrampoline("OptimizedCallRuntimeWithArgv", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = runtimeCallTrampoline;
    std::array<VariableType, 4> params = { /* 4 : 4 input parameters */
        VariableType::NATIVE_POINTER(), // glue
        VariableType::INT64(),   // runtimeId
        VariableType::INT32(),   // argc
        VariableType::NATIVE_POINTER(), // argv
    };
    callSign->SetVariadicArgs(false);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(OptimizedCallOptimized)
{
    /* 4 : 4 input parameters */
    CallSignature runtimeCallTrampoline("OptimizedCallOptimized", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = runtimeCallTrampoline;
    std::array<VariableType, 4> params = { /* 4 : 4 input parameters */
        VariableType::NATIVE_POINTER(),
        VariableType::INT32(),
        VariableType::INT32(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(JSCall)
{
    // 5 : 5 input parameters
    CallSignature jSCall("JSCall", 0, 5,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = jSCall;
    std::array<VariableType, 5> params = { // 5 : 5 input parameters
        VariableType::NATIVE_POINTER(),     // glue
        VariableType::INT32(),       // actual argC
        VariableType::JS_ANY(),      // call target
        VariableType::JS_ANY(),      // new target
        VariableType::JS_ANY(),      // thisobj
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetCallConv(CallSignature::CallConv::WebKitJSCallConv);
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(JSCallWithArgV)
{
    // 4 : 4 input parameters
    CallSignature jSCallWithArgV("JSCallWithArgV", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = jSCallWithArgV;
    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),     // glue
        VariableType::INT32(),       // actual argC
        VariableType::JS_ANY(),      // call target
        VariableType::NATIVE_POINTER(),    // argv
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetTailCall(true);
}

DEF_CALL_SIGNATURE(ResumeRspAndDispatch)
{
    // 8 : 8 input parameters
    CallSignature resumeRspAndDispatch("ResumeRspAndDispatch", 0, 8,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = resumeRspAndDispatch;
    std::array<VariableType, 8> params = { // 8 : 8 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetCallConv(CallSignature::CallConv::GHCCallConv);
}

DEF_CALL_SIGNATURE(ResumeRspAndReturn)
{
    // 2 : 2 input parameters
    CallSignature resumeRspAndReturn("ResumeRspAndReturn", 0, 2,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = resumeRspAndReturn;
    std::array<VariableType, 2> params = { // 2 : 2 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
    callSign->SetCallConv(CallSignature::CallConv::GHCCallConv);
}

#define PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(name)                      \
    /* 2 : 2 input parameters */                                         \
    CallSignature signature(#name, 0, 2,                                 \
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());            \
    *callSign = signature;                                               \
    std::array<VariableType, 2> params = { /* 2 : 2 input parameters */  \
        VariableType::NATIVE_POINTER(),                                  \
        VariableType::JS_ANY(),                                          \
    };                                                                   \
    callSign->SetVariadicArgs(true);                                     \
    callSign->SetParameters(params.data());                              \
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);    \
    callSign->SetCallConv(CallSignature::CallConv::GHCCallConv);

#define PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(name)             \
    /* 2 : 2 input parameters */                                       \
    CallSignature signature(#name, 0, 2,                               \
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());        \
    *callSign = signature;                                             \
    std::array<VariableType, 2> params = { /* 2 : 2 input parameters */\
        VariableType::NATIVE_POINTER(),                                \
        VariableType::JS_ANY(),                                        \
    };                                                                 \
    callSign->SetVariadicArgs(true);                                   \
    callSign->SetParameters(params.data());                            \
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);  \
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);

DEF_CALL_SIGNATURE(PushCallArgs0AndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallArgs0AndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallArgs0AndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs0AndDispatch)
}

DEF_CALL_SIGNATURE(PushCallArgs0AndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs0AndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(PushCallArgs1AndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallArgs1AndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallArgs1AndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs1AndDispatch)
}

DEF_CALL_SIGNATURE(PushCallArgs1AndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs1AndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(PushCallArgs2AndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallArgs2AndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallArgs2AndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs2AndDispatch)
}

DEF_CALL_SIGNATURE(PushCallArgs2AndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs2AndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(PushCallArgs3AndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallArgs3AndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallArgs3AndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs3AndDispatch)
}

DEF_CALL_SIGNATURE(PushCallArgs3AndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallArgs3AndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(PushCallIRangeAndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallIRangeAndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallIRangeAndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallIRangeAndDispatch)
}

DEF_CALL_SIGNATURE(PushCallIRangeAndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallIRangeAndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(PushCallIThisRangeAndDispatchNative)
{
    PUSH_CALL_ARGS_AND_DISPATCH_NATIVE_SIGNATURE(PushCallIThisRangeAndDispatchNative)
}

DEF_CALL_SIGNATURE(PushCallIThisRangeAndDispatch)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallIThisRangeAndDispatch)
}

DEF_CALL_SIGNATURE(PushCallIThisRangeAndDispatchSlowPath)
{
    PUSH_CALL_ARGS_AND_DISPATCH_SIGNATURE(PushCallIThisRangeAndDispatchSlowPath)
}

DEF_CALL_SIGNATURE(DebugPrint)
{
    // 1 : 1 input parameters
    CallSignature debugPrint("DebugPrint", 0, 1,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = debugPrint;
    // 1 : 1 input parameters
    std::array<VariableType, 1> params = {
        VariableType::INT32(),
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(FatalPrint)
{
    // 1 : 1 input parameters
    CallSignature fatalPrint("FatalPrint", 0, 1,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = fatalPrint;
    // 1 : 1 input parameters
    std::array<VariableType, 1> params = {
        VariableType::INT32(),
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(InsertOldToNewRememberedSet)
{
    // 3 : 3 input parameters
    CallSignature index("InsertOldToNewRememberedSet", 0, 3, ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = index;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(FloatMod)
{
    // 2 : 2 input parameters
    CallSignature index("FloatMod", 0, 2, ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = index;
    // 2 : 2 input parameters
    std::array<VariableType, 2> params = {
        VariableType::FLOAT64(),
        VariableType::FLOAT64(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(FindElementWithCache)
{
    // 4 : 4 input parameters
    CallSignature index("FindElementWithCache", 0, 4, ArgumentsOrder::DEFAULT_ORDER, VariableType::INT32());
    *callSign = index;
    // 4 : 4 input parameters
    std::array<VariableType, 4> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::INT32(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(DoubleToInt)
{
    // 1 : 1 input parameters
    CallSignature index("DoubleToInt", 0, 1, ArgumentsOrder::DEFAULT_ORDER, VariableType::INT32());
    *callSign = index;
    // 1 : 1 input parameters
    std::array<VariableType, 1> params = {
        VariableType::FLOAT64(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(MarkingBarrier)
{
    // 5 : 5 input parameters
    CallSignature index("MarkingBarrier", 0, 5, ArgumentsOrder::DEFAULT_ORDER, VariableType::VOID());
    *callSign = index;
    // 5 : 5 input parameters
    std::array<VariableType, 5> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallArg0Dyn)
{
    // 2 : 2 input parameters
    CallSignature callArg0Dyn("callArg0Dyn", 0, 2,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callArg0Dyn;
    std::array<VariableType, 2> params = { // 2 : 2 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallArg1Dyn)
{
    // 3 : 3 input parameters
    CallSignature callArg1Dyn("callArg1Dyn", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callArg1Dyn;
    std::array<VariableType, 3> params = { // 3 : 3 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallArgs2Dyn)
{
    // 4 : 4 input parameters
    CallSignature callArgs2Dyn("callArgs2Dyn", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callArgs2Dyn;
    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallArgs3Dyn)
{
    // 5 : 5 input parameters
    CallSignature callArgs3Dyn("callArgs3Dyn", 0, 5,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callArgs3Dyn;
    std::array<VariableType, 5> params = { // 5 : 5 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallIThisRangeDyn)
{
    // 3 : 3 input parameters
    CallSignature callIThisRangeDyn("callIThisRangeDyn", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callIThisRangeDyn;
    std::array<VariableType, 3> params = { // 3 : 3 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY(),
        VariableType::JS_ANY()
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(CallIRangeDyn)
{
    // 2 : 2 input parameters
    CallSignature callIRangeDyn("callIRangeDyn", 0, 2,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_ANY());
    *callSign = callIRangeDyn;
    std::array<VariableType, 2> params = { // 2 : 2 input parameters
        VariableType::NATIVE_POINTER(),
        VariableType::JS_ANY()
    };
    callSign->SetVariadicArgs(true);
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(JsProxyCallInternal)
{
    // 4 : 4 input parameters
    CallSignature proxyCallInternal("JsProxyCallInternal", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, VariableType::JS_POINTER());
    *callSign = proxyCallInternal;
    std::array<VariableType, 4> params = { // 4 : 4 input parameters
        VariableType::NATIVE_POINTER(),    // glue
        VariableType::INT32(),      // actual argC
        VariableType::JS_POINTER(), // callTarget
        VariableType::NATIVE_POINTER(),    // argv
    };
    callSign->SetVariadicArgs(false);
    callSign->SetParameters(params.data());
    callSign->SetTailCall(true);
    callSign->SetTargetKind(CallSignature::TargetKind::COMMON_STUB);
    callSign->SetCallConv(CallSignature::CallConv::CCallConv);
}

DEF_CALL_SIGNATURE(CreateArrayFromList)
{
    // 3 : 3 input parameters
    CallSignature createArrayFromList("CreateArrayFromList", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
                                     VariableType::JS_POINTER());
    *callSign = createArrayFromList;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::INT32(),
        VariableType::NATIVE_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}

DEF_CALL_SIGNATURE(JSObjectGetMethod)
{
    // 3 : 3 input parameters
    CallSignature jsObjectGetMethod("JSObjectGetMethod", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
                                     VariableType::JS_POINTER());
    *callSign = jsObjectGetMethod;
    // 3 : 3 input parameters
    std::array<VariableType, 3> params = {
        VariableType::NATIVE_POINTER(),
        VariableType::JS_POINTER(),
        VariableType::JS_POINTER(),
    };
    callSign->SetParameters(params.data());
    callSign->SetTailCall(false);
    callSign->SetTargetKind(CallSignature::TargetKind::RUNTIME_STUB);
}
}  // namespace panda::ecmascript::kungfu
