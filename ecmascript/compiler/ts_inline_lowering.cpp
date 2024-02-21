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
#include "ecmascript/compiler/bytecode_circuit_builder.h"
#include "ecmascript/compiler/bytecodes.h"
#include "ecmascript/compiler/compiler_log.h"
#include "ecmascript/compiler/pass.h"
#include "ecmascript/compiler/ts_inline_lowering.h"
#include "ecmascript/compiler/type_info_accessors.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "ecmascript/ts_types/ts_type.h"
#include "libpandabase/utils/utf.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "ecmascript/ts_types/ts_type_accessor.h"

namespace panda::ecmascript::kungfu {
void TSInlineLowering::RunTSInlineLowering()
{
    circuit_->AdvanceTime();
    ChunkQueue<InlineTypeInfoAccessor> workList(chunk_);
    UpdateWorkList(workList);

    while (!workList.empty()) {
        InlineTypeInfoAccessor &info = workList.front();
        workList.pop();
        TryInline(info, workList);
    }
    CollectInlineInfo();
}

void TSInlineLowering::CollectInlineInfo()
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (const auto &gate : gateList) {
        auto op = acc_.GetOpCode(gate);
        if (op == OpCode::FRAME_ARGS) {
            GetInlinedMethodId(gate);
        }
    }
}

void TSInlineLowering::GetInlinedMethodId(GateRef gate)
{
    ASSERT(acc_.GetOpCode(gate) == OpCode::FRAME_ARGS);
    GateRef func = acc_.GetValueIn(gate, static_cast<size_t>(FrameArgIdx::FUNC));
    uint32_t methodOffset = 0;
    auto funcType = acc_.GetGateType(func);
    if (tsManager_->IsFunctionTypeKind(funcType)) {
        GlobalTSTypeRef gt = funcType.GetGTRef();
        methodOffset = tsManager_->GetFuncMethodOffset(gt);
    }
    if (methodOffset == 0) {
        auto pgoType = acc_.TryGetPGOType(gate);
        if (pgoType.IsValidCallMethodId()) {
            methodOffset = pgoType.GetCallMethodId();
        }
    }
    acc_.UpdateMethodOffset(gate, methodOffset);
}

void TSInlineLowering::CandidateInlineCall(GateRef gate, ChunkQueue<InlineTypeInfoAccessor> &workList)
{
    EcmaOpcode ecmaOpcode = acc_.GetByteCodeOpcode(gate);
    switch (ecmaOpcode) {
        case EcmaOpcode::LDOBJBYNAME_IMM8_ID16:
        case EcmaOpcode::LDOBJBYNAME_IMM16_ID16:
        case EcmaOpcode::LDTHISBYNAME_IMM8_ID16:
        case EcmaOpcode::LDTHISBYNAME_IMM16_ID16:
            CandidateAccessor(gate, workList, CallKind::CALL_GETTER);
            break;
        case EcmaOpcode::STOBJBYNAME_IMM8_ID16_V8:
        case EcmaOpcode::STOBJBYNAME_IMM16_ID16_V8:
        case EcmaOpcode::STTHISBYNAME_IMM8_ID16:
        case EcmaOpcode::STTHISBYNAME_IMM16_ID16:
            CandidateAccessor(gate, workList, CallKind::CALL_SETTER);
            break;
        case EcmaOpcode::CALLTHIS0_IMM8_V8:
        case EcmaOpcode::CALLTHIS1_IMM8_V8_V8:
        case EcmaOpcode::CALLTHIS2_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLTHIS3_IMM8_V8_V8_V8_V8:
        case EcmaOpcode::CALLTHISRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLTHISRANGE_PREF_IMM16_V8:
            CandidateNormalCall(gate, workList, CallKind::CALL_THIS);
            break;
        case EcmaOpcode::CALLRUNTIME_CALLINIT_PREF_IMM8_V8:
            CandidateNormalCall(gate, workList, CallKind::CALL_INIT);
            break;
        case EcmaOpcode::CALLARG0_IMM8:
        case EcmaOpcode::CALLARG1_IMM8_V8:
        case EcmaOpcode::CALLARGS2_IMM8_V8_V8:
        case EcmaOpcode::CALLARGS3_IMM8_V8_V8_V8:
        case EcmaOpcode::CALLRANGE_IMM8_IMM8_V8:
        case EcmaOpcode::WIDE_CALLRANGE_PREF_IMM16_V8:
            CandidateNormalCall(gate, workList, CallKind::CALL);
            break;
        default:
            break;
    }
}

void TSInlineLowering::TryInline(InlineTypeInfoAccessor &info, ChunkQueue<InlineTypeInfoAccessor> &workList)
{
    GateRef gate = info.GetCallGate();
    // inline doesn't support try-catch
    bool inTryCatch = FilterCallInTryCatch(gate);
    if (inTryCatch) {
        return;
    }

    MethodLiteral* inlinedMethod = nullptr;
    uint32_t methodOffset = info.GetCallMethodId();
    if (methodOffset == 0 || ctx_->IsSkippedMethod(methodOffset)) {
        return;
    }
    if (IsRecursiveFunc(info, methodOffset)) {
        return;
    }
    inlinedMethod = ctx_->GetJSPandaFile()->FindMethodLiteral(methodOffset);
    if (!CheckParameter(gate, info, inlinedMethod)) {
        return;
    }
    auto &bytecodeInfo = ctx_->GetBytecodeInfo();
    auto &methodInfo = bytecodeInfo.GetMethodList().at(methodOffset);
    auto &methodPcInfos = bytecodeInfo.GetMethodPcInfos();
    auto &methodPcInfo = methodPcInfos[methodInfo.GetMethodPcInfoIndex()];
    if (ctx_->FilterMethod(inlinedMethod, methodPcInfo)) {
        return;
    }
    GateRef frameState = GetFrameState(info);
    GateRef frameArgs = acc_.GetValueIn(frameState);
    size_t inlineCallCounts = GetOrInitialInlineCounts(frameArgs);
    if (IsSmallMethod(methodPcInfo.pcOffsets.size()) && !IsInlineCountsOverflow(inlineCallCounts)) {
        inlineSuccess_ = FilterInlinedMethod(inlinedMethod, methodPcInfo.pcOffsets);
        if (inlineSuccess_) {
            SetInitCallTargetAndConstPoolId(info);
            CircuitRootScope scope(circuit_);
            if (!noCheck_ && !info.IsCallInit()) {
                InlineCheck(info);
            }
            InlineCall(methodInfo, methodPcInfo, inlinedMethod, info);
            UpdateInlineCounts(frameArgs, inlineCallCounts);
            if (info.IsNormalCall()) {
                UpdateWorkList(workList);
            } else {
                lastCallId_ = circuit_->GetGateCount() - 1;
            }
        }
    }

    if ((inlinedMethod != nullptr) && IsLogEnabled() && inlineSuccess_) {
        inlineSuccess_ = false;
        auto jsPandaFile = ctx_->GetJSPandaFile();
        const std::string methodName(
            MethodLiteral::GetMethodName(jsPandaFile, inlinedMethod->GetMethodId()));
        std::string fileName = jsPandaFile->GetFileName();
        const std::string recordName(MethodLiteral::GetRecordName(jsPandaFile, inlinedMethod->GetMethodId()));
        std::string fullName = methodName + "@" + recordName + "@" + fileName;
        LOG_COMPILER(INFO) << "";
        LOG_COMPILER(INFO) << "\033[34m"
                           << "===================="
                           << " After inlining "
                           << "[" << fullName << "]"
                           << " Caller method "
                           << "[" << methodName_ << "]"
                           << "===================="
                           << "\033[0m";
        circuit_->PrintAllGatesWithBytecode();
        LOG_COMPILER(INFO) << "\033[34m" << "========================= End ==========================" << "\033[0m";
    }
}

bool TSInlineLowering::FilterInlinedMethod(MethodLiteral* method, std::vector<const uint8_t*> pcOffsets)
{
    const JSPandaFile *jsPandaFile = ctx_->GetJSPandaFile();
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, method->GetMethodId());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());
    if (cda.GetTriesSize() != 0) {
        return false;
    }
    for (size_t i = 0; i < pcOffsets.size(); i++) {
        auto pc = pcOffsets[i];
        auto ecmaOpcode = ctx_->GetByteCodes()->GetOpcode(pc);
        switch (ecmaOpcode) {
            case EcmaOpcode::GETUNMAPPEDARGS:
            case EcmaOpcode::SUSPENDGENERATOR_V8:
            case EcmaOpcode::RESUMEGENERATOR:
            case EcmaOpcode::COPYRESTARGS_IMM8:
            case EcmaOpcode::WIDE_COPYRESTARGS_PREF_IMM16:
            case EcmaOpcode::CREATEASYNCGENERATOROBJ_V8:
                return false;
            default:
                break;
        }
    }
    return true;
}

void TSInlineLowering::InlineCall(MethodInfo &methodInfo, MethodPcInfo &methodPCInfo, MethodLiteral* method,
                                  InlineTypeInfoAccessor &info)
{
    const JSPandaFile *jsPandaFile = ctx_->GetJSPandaFile();
    TSManager *tsManager = ctx_->GetTSManager();
    CompilerLog *log = ctx_->GetCompilerLog();
    CString recordName = MethodLiteral::GetRecordName(jsPandaFile, method->GetMethodId());
    bool hasTyps = jsPandaFile->HasTSTypes(recordName);
    const std::string methodName(MethodLiteral::GetMethodName(jsPandaFile, method->GetMethodId()));
    std::string fileName = jsPandaFile->GetFileName();
    std::string fullName = methodName + "@" + std::string(recordName) + "@" + fileName;

    circuit_->InitRoot();
    BytecodeCircuitBuilder builder(jsPandaFile, method, methodPCInfo,
                                   tsManager, circuit_,
                                   ctx_->GetByteCodes(), true, IsLogEnabled(),
                                   enableTypeLowering_, fullName, recordName, ctx_->GetPfDecoder(), true,
                                   passOptions_->EnableOptTrackField());
    {
        if (enableTypeLowering_) {
            BuildFrameStateChain(info, builder);
        }
        TimeScope timeScope("BytecodeToCircuit", methodName, method->GetMethodId().GetOffset(), log);
        builder.BytecodeToCircuit();
    }

    ReplaceInput(info, glue_, method);

    PassData data(&builder, circuit_, ctx_, log, fullName,
                  &methodInfo, hasTyps, recordName,
                  method, method->GetMethodId().GetOffset(), nativeAreaAllocator_, ctx_->GetPfDecoder(), passOptions_);
    PassRunner<PassData> pipeline(&data);
    pipeline.RunPass<RedundantPhiEliminationPass>();
    if (builder.EnableLoopOptimization()) {
        pipeline.RunPass<LoopOptimizationPass>();
        pipeline.RunPass<RedundantPhiEliminationPass>();
    }
    pipeline.RunPass<TypeInferPass>();
    pipeline.RunPass<PGOTypeInferPass>();
}

bool TSInlineLowering::CheckParameter(GateRef gate, InlineTypeInfoAccessor &info, MethodLiteral* method)
{
    if (method == nullptr) {
        return false;
    }
    if (info.IsCallAccessor()) {
        return true;
    }
    size_t numIns = acc_.GetNumValueIn(gate);
    size_t fixedInputsNum = info.IsCallThis() ? 2 : 1; // 2: calltarget and this

    uint32_t declaredNumArgs = method->GetNumArgsWithCallField();
    return declaredNumArgs == (numIns - fixedInputsNum);
}

void TSInlineLowering::ReplaceCallInput(InlineTypeInfoAccessor &info, GateRef glue, MethodLiteral *method)
{
    GateRef gate = info.GetCallGate();
    bool isCallThis = info.IsCallThis();
    std::vector<GateRef> vec;
    size_t numIns = acc_.GetNumValueIn(gate);
    // 1: last one elem is function
    GateRef callTarget = acc_.GetValueIn(gate, numIns - 1);
    GateRef thisObj = Circuit::NullGate();
    size_t fixedInputsNum = 0;
    if (isCallThis) {
        fixedInputsNum = 2; // 2: call target and this
        thisObj = acc_.GetValueIn(gate, 0);
    } else {
        fixedInputsNum = 1; // 1: call target
        thisObj = builder_.Undefined();
    }
    // -1: callTarget
    size_t actualArgc = numIns + NUM_MANDATORY_JSFUNC_ARGS - fixedInputsNum;
    vec.emplace_back(glue); // glue
    if (!method->IsFastCall()) {
        vec.emplace_back(builder_.Int64(actualArgc)); // argc
    }
    vec.emplace_back(callTarget);
    if (!method->IsFastCall()) {
        vec.emplace_back(builder_.Undefined()); // newTarget
    }
    vec.emplace_back(thisObj);
    // -1: call Target
    for (size_t i = fixedInputsNum - 1; i < numIns - 1; i++) {
        vec.emplace_back(acc_.GetValueIn(gate, i));
    }
    LowerToInlineCall(info, vec, method);
}

void TSInlineLowering::ReplaceAccessorInput(InlineTypeInfoAccessor &info, GateRef glue, MethodLiteral *method)
{
    GateRef gate = info.GetCallGate();
    std::vector<GateRef> vec;
    GateRef thisObj = GetAccessorReceiver(gate);
    GateRef callTarget = Circuit::NullGate();
    callTarget = BuildAccessor(info);
    size_t actualArgc = 0;
    if (info.IsCallGetter()) {
        actualArgc = NUM_MANDATORY_JSFUNC_ARGS;
    } else if (info.IsCallSetter()) {
        actualArgc = NUM_MANDATORY_JSFUNC_ARGS + 1;
    } else {
        UNREACHABLE();
    }

    vec.emplace_back(glue); // glue
    if (!method->IsFastCall()) {
        vec.emplace_back(builder_.Int64(actualArgc)); // argc
    }
    vec.emplace_back(callTarget);
    if (!method->IsFastCall()) {
        vec.emplace_back(builder_.Undefined()); // newTarget
    }
    vec.emplace_back(thisObj);

    if (info.IsCallSetter()) {
        vec.emplace_back(GetCallSetterValue(gate));
    }
    LowerToInlineCall(info, vec, method);
}

GateRef TSInlineLowering::BuildAccessor(InlineTypeInfoAccessor &info)
{
    GateRef gate = info.GetCallGate();
    Environment env(gate, circuit_, &builder_);
    GateRef receiver = GetAccessorReceiver(gate);
    GateRef accessor = Circuit::NullGate();
    uint32_t plrData = info.GetPlr().GetData();

    const PGORWOpType *pgoTypes = acc_.TryGetPGOType(gate).GetPGORWOpType();
    ASSERT(pgoTypes->GetCount() == 1);
    auto pgoType = pgoTypes->GetObjectInfo(0);
    ProfileTyper holderType = std::make_pair(pgoType.GetHoldRootType(), pgoType.GetHoldType());
    PGOTypeManager *ptManager = thread_->GetCurrentEcmaContext()->GetPTManager();
    int holderHCIndex = static_cast<int>(ptManager->GetHClassIndexByProfileType(holderType));
    ASSERT(ptManager->QueryHClass(holderType.first, holderType.second).IsJSHClass());
    ArgumentAccessor argAcc(circuit_);
    GateRef jsFunc = argAcc.GetFrameArgsIn(gate, FrameArgIdx::FUNC);

    auto currentLabel = env.GetCurrentLabel();
    auto state = currentLabel->GetControl();
    auto depend = currentLabel->GetDepend();
    GateRef holder = circuit_->NewGate(circuit_->LookUpHolder(), MachineType::I64,
                                       { state, depend, receiver, builder_.Int32(holderHCIndex), jsFunc},
                                       GateType::AnyType());

    GateRef frameState = acc_.GetFrameState(gate);
    // Check whether constpool ids of caller functions and callee functions are consistent. If they are consistent,
    // the caller function is used in get constpool. It avoids a depend dependence on the accessor function, and thus a
    // load accessor.
    // Example:
    // function A inline Accessor B, if A.constpoolId == B.constpoolId, Use A.GetConstpool to replace B.GetConstpool
    if (initConstantPoolId_ == GetAccessorConstpoolId(info)) {
        frameState = initCallTarget_;
    }
    if (info.IsCallGetter()) {
        accessor = circuit_->NewGate(circuit_->LoadGetter(), MachineType::I64,
                                     { holder, holder, builder_.Int32(plrData), frameState },
                                     GateType::AnyType());
    } else {
        accessor = circuit_->NewGate(circuit_->LoadSetter(), MachineType::I64,
                                     { holder, holder, builder_.Int32(plrData), frameState },
                                     GateType::AnyType());
    }
    acc_.ReplaceDependIn(gate, holder);
    acc_.ReplaceStateIn(gate, holder);
    return accessor;
}

void TSInlineLowering::ReplaceInput(InlineTypeInfoAccessor &info, GateRef glue, MethodLiteral *method)
{
    if (info.IsNormalCall()) {
        ReplaceCallInput(info, glue, method);
    } else {
        ASSERT(info.IsCallAccessor());
        ReplaceAccessorInput(info, glue, method);
    }
}

GateRef TSInlineLowering::MergeAllReturn(const std::vector<GateRef> &returnVector, GateRef &state, GateRef &depend)
{
    size_t numOfIns = returnVector.size();
    auto stateList = std::vector<GateRef>(numOfIns, Circuit::NullGate());
    auto dependList = std::vector<GateRef>(numOfIns + 1, Circuit::NullGate());
    auto vaueList = std::vector<GateRef>(numOfIns + 1, Circuit::NullGate());

    for (size_t i = 0; i < returnVector.size(); i++) {
        GateRef returnGate = returnVector.at(i);
        ASSERT(acc_.GetOpCode(acc_.GetState(returnGate)) != OpCode::IF_EXCEPTION);
        stateList[i] = acc_.GetState(returnGate);
        dependList[i + 1] = acc_.GetDep(returnGate);
        vaueList[i + 1] = acc_.GetValueIn(returnGate, 0);
        acc_.DeleteGate(returnGate);
    }

    state = circuit_->NewGate(circuit_->Merge(numOfIns), stateList);
    dependList[0] = state;
    vaueList[0] = state;
    depend = circuit_->NewGate(circuit_->DependSelector(numOfIns), dependList);
    return circuit_->NewGate(circuit_->ValueSelector(numOfIns), MachineType::I64, numOfIns + 1,
                             vaueList.data(), GateType::AnyType());
}

void TSInlineLowering::ReplaceEntryGate(GateRef callGate, GateRef callerFunc, GateRef inlineFunc, GateRef glue)
{
    auto stateEntry = acc_.GetStateRoot();
    auto dependEntry = acc_.GetDependRoot();

    GateRef callState = acc_.GetState(callGate);
    GateRef callDepend = acc_.GetDep(callGate);
    auto stateUse = acc_.Uses(stateEntry);

    // support inline trace
    GateRef newDep = Circuit::NullGate();
    if (traceInline_) {
        std::vector<GateRef> args{callerFunc, inlineFunc};
        newDep = TraceInlineFunction(glue, callDepend, args, callGate);
    } else {
        newDep = callDepend;
    }

    for (auto stateUseIt = stateUse.begin(); stateUseIt != stateUse.end();) {
        stateUseIt = acc_.ReplaceIn(stateUseIt, callState);
    }

    auto dependUse = acc_.Uses(dependEntry);
    for (auto dependUseIt = dependUse.begin(); dependUseIt != dependUse.end();) {
        dependUseIt = acc_.ReplaceIn(dependUseIt, newDep);
    }
}

GateRef TSInlineLowering::TraceInlineFunction(GateRef glue, GateRef depend, std::vector<GateRef> &args,
                                              GateRef callGate)
{
    size_t index = RTSTUB_ID(AotInlineTrace);
    GateRef result = builder_.NoLabelCallRuntime(glue, depend, index, args, callGate);
    return result;
}

void TSInlineLowering::ReplaceReturnGate(GateRef callGate)
{
    std::vector<GateRef> returnVector;
    acc_.GetReturnOuts(returnVector);

    GateRef value = Circuit::NullGate();
    GateRef state = Circuit::NullGate();
    GateRef depend = Circuit::NullGate();

    if (returnVector.size() == 1) {
        GateRef returnGate = returnVector.at(0);
        GateRef returnState = acc_.GetState(returnGate);
        depend = acc_.GetDep(returnGate);
        state = returnState;
        value = acc_.GetValueIn(returnGate, 0);
        acc_.DeleteGate(returnGate);
    } else {
        value = MergeAllReturn(returnVector, state, depend);
    }
    SupplementType(callGate, value);
    ReplaceHirAndDeleteState(callGate, state, depend, value);
}

void TSInlineLowering::ReplaceHirAndDeleteState(GateRef gate, GateRef state, GateRef depend, GateRef value)
{
    auto uses = acc_.Uses(gate);
    for (auto useIt = uses.begin(); useIt != uses.end();) {
        if (acc_.IsStateIn(useIt)) {
            useIt = acc_.ReplaceIn(useIt, state);
        } else if (acc_.IsDependIn(useIt)) {
            useIt = acc_.ReplaceIn(useIt, depend);
        } else if (acc_.IsValueIn(useIt)) {
            useIt = acc_.ReplaceIn(useIt, value);
        } else {
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
        }
    }
    acc_.DeleteGate(gate);
}

void TSInlineLowering::LowerToInlineCall(
    InlineTypeInfoAccessor &info, const std::vector<GateRef> &args, MethodLiteral *method)
{
    GateRef callGate = info.GetCallGate();
    // replace in value/args
    ArgumentAccessor argAcc(circuit_);
    ASSERT(argAcc.ArgsCount() == args.size());
    for (size_t i = 0; i < argAcc.ArgsCount(); i++) {
        GateRef arg = argAcc.ArgsAt(i);
        acc_.UpdateAllUses(arg, args.at(i));
        if (acc_.GetGateType(args.at(i)).IsAnyType()) {
            acc_.SetGateType(args.at(i), acc_.GetGateType(arg));
        }
        acc_.DeleteGate(arg);
    }
    // replace in depend and state
    GateRef glue = args.at(static_cast<size_t>(CommonArgIdx::GLUE));
    GateRef inlineFunc;
    if (method->IsFastCall()) {
        inlineFunc = args.at(static_cast<size_t>(FastCallArgIdx::FUNC));
    } else {
        inlineFunc = args.at(static_cast<size_t>(CommonArgIdx::FUNC));
    }
    GateRef frameArgs = GetFrameArgs(info);
    GateRef callerFunc = acc_.GetValueIn(frameArgs, 0);
    ReplaceEntryGate(callGate, callerFunc, inlineFunc, glue);
    // replace use gate
    ReplaceReturnGate(callGate);
    // remove Useless root gates
    RemoveRoot();
}

void TSInlineLowering::InlineFuncCheck(const InlineTypeInfoAccessor &info)
{
    GateRef gate = info.GetCallGate();
    GateRef callState = acc_.GetState(gate);
    GateRef callDepend = acc_.GetDep(gate);
    ASSERT(acc_.HasFrameState(gate));
    GateRef frameState = acc_.GetFrameState(gate);
    size_t funcIndex = acc_.GetNumValueIn(gate) - 1;
    GateRef inlineFunc =  acc_.GetValueIn(gate, funcIndex);
    // Do not load from inlineFunc beacause type in inlineFunc could be modified by others
    uint32_t methodOffset = info.GetCallMethodId();
    GateRef ret = circuit_->NewGate(circuit_->JSInlineTargetTypeCheck(info.GetType()),
        MachineType::I1, {callState, callDepend, inlineFunc, builder_.IntPtr(methodOffset), frameState},
        GateType::NJSValue());
    acc_.ReplaceStateIn(gate, ret);
    acc_.ReplaceDependIn(gate, ret);
}

void TSInlineLowering::InlineAccessorCheck(const InlineTypeInfoAccessor &info)
{
    ASSERT(info.IsCallAccessor());
    GateRef gate = info.GetCallGate();
    GateRef receiver = GetAccessorReceiver(gate);
    Environment env(gate, circuit_, &builder_);

    const PGORWOpType *pgoTypes = acc_.TryGetPGOType(gate).GetPGORWOpType();
    ASSERT(pgoTypes->GetCount() == 1);
    auto pgoType = pgoTypes->GetObjectInfo(0);
    ProfileTyper receiverType = std::make_pair(pgoType.GetReceiverRootType(), pgoType.GetReceiverType());
    PGOTypeManager *ptManager = thread_->GetCurrentEcmaContext()->GetPTManager();
    int receiverHCIndex = static_cast<int>(ptManager->GetHClassIndexByProfileType(receiverType));
    ASSERT(ptManager->QueryHClass(receiverType.first, receiverType.second).IsJSHClass());

    bool noNeedCheckHeapObject = acc_.IsHeapObjectFromElementsKind(receiver);
    builder_.ObjectTypeCheck(acc_.GetGateType(gate), noNeedCheckHeapObject, receiver, builder_.Int32(receiverHCIndex),
                             acc_.GetFrameState(gate));
    auto currentLabel = env.GetCurrentLabel();
    auto callState = currentLabel->GetControl();
    auto callDepend = currentLabel->GetDepend();
    auto frameState = acc_.GetFrameState(gate);
    ArgumentAccessor argAcc(circuit_);
    GateRef jsFunc = argAcc.GetFrameArgsIn(gate, FrameArgIdx::FUNC);
    GateRef ret = circuit_->NewGate(circuit_->PrototypeCheck(receiverHCIndex), MachineType::I1,
        {callState, callDepend, jsFunc, frameState}, GateType::NJSValue());
    acc_.ReplaceStateIn(gate, ret);
    acc_.ReplaceDependIn(gate, ret);
}

void TSInlineLowering::InlineCheck(InlineTypeInfoAccessor &info)
{
    if (info.IsNormalCall()) {
        InlineFuncCheck(info);
    } else {
        InlineAccessorCheck(info);
    }
}

void TSInlineLowering::RemoveRoot()
{
    GateRef circuitRoot = acc_.GetCircuitRoot();
    auto uses = acc_.Uses(circuitRoot);
    for (auto it = uses.begin(); it != uses.end();) {
        it = acc_.DeleteGate(it);
    }

    acc_.DeleteGate(circuitRoot);
}

void TSInlineLowering::BuildFrameStateChain(InlineTypeInfoAccessor &info, BytecodeCircuitBuilder &builder)
{
    GateRef preFrameState = GetFrameState(info);
    ASSERT(acc_.GetOpCode(preFrameState) == OpCode::FRAME_STATE);
    builder.SetPreFrameState(preFrameState);
    GateRef frameArgs = acc_.GetFrameArgs(preFrameState);
    builder.SetPreFrameArgs(frameArgs);
}

bool TSInlineLowering::FilterCallInTryCatch(GateRef gate)
{
    auto uses = acc_.Uses(gate);
    for (auto it = uses.begin(); it != uses.end(); ++it) {
        if (acc_.GetOpCode(*it) == OpCode::IF_SUCCESS || acc_.GetOpCode(*it) == OpCode::IF_EXCEPTION) {
            return true;
        }
    }
    return false;
}

void TSInlineLowering::SupplementType(GateRef callGate, GateRef targetGate)
{
    GateType callGateType = acc_.GetGateType(callGate);
    GateType targetGateType = acc_.GetGateType(targetGate);
    if (!callGateType.IsAnyType() && targetGateType.IsAnyType()) {
        acc_.SetGateType(targetGate, callGateType);
    }
}

void TSInlineLowering::UpdateWorkList(ChunkQueue<InlineTypeInfoAccessor> &workList)
{
    std::vector<GateRef> gateList;
    circuit_->GetAllGates(gateList);
    for (const auto &gate : gateList) {
        if (acc_.GetId(gate) <= lastCallId_) {
            continue;
        }
        auto op = acc_.GetOpCode(gate);
        if (op == OpCode::JS_BYTECODE) {
            CandidateInlineCall(gate, workList);
        }
    }
}

size_t TSInlineLowering::GetOrInitialInlineCounts(GateRef frameArgs)
{
    auto it = inlinedCallMap_.find(frameArgs);
    if (it == inlinedCallMap_.end()) {
        inlinedCallMap_[frameArgs] = 0;
    }
    return inlinedCallMap_[frameArgs];
}

bool TSInlineLowering::IsRecursiveFunc(InlineTypeInfoAccessor &info, size_t calleeMethodOffset)
{
    GateRef caller = info.GetCallGate();
    GateRef frameArgs = GetFrameArgs(info);
    while (acc_.GetOpCode(caller) == OpCode::JS_BYTECODE) {
        auto callerMethodOffset = acc_.TryGetMethodOffset(caller);
        if (callerMethodOffset == calleeMethodOffset) {
            return true;
        }
        frameArgs = acc_.GetFrameArgs(frameArgs);
        if (frameArgs == Circuit::NullGate()) {
            break;
        }
        caller = acc_.GetValueIn(frameArgs);
    }
    return false;
}

void TSInlineLowering::CandidateAccessor(GateRef gate, ChunkQueue<InlineTypeInfoAccessor> &workList, CallKind kind)
{
    GateRef receiver = GetAccessorReceiver(gate);
    InlineTypeInfoAccessor tacc(thread_, circuit_, gate, receiver, kind);
    if (tacc.IsEnableAccessorInline()) {
        workList.emplace(thread_, circuit_, gate, receiver, kind);
        lastCallId_ = acc_.GetId(gate);
    }
}

void TSInlineLowering::CandidateNormalCall(GateRef gate, ChunkQueue<InlineTypeInfoAccessor> &workList, CallKind kind)
{
    size_t funcIndex = acc_.GetNumValueIn(gate) - 1;
    auto func = acc_.GetValueIn(gate, funcIndex);
    InlineTypeInfoAccessor tacc(thread_, circuit_, gate, func, kind);
    if (tacc.IsEnableNormalInline()) {
        workList.push(tacc);
        lastCallId_ = acc_.GetId(gate);
    }
}

GateRef TSInlineLowering::GetAccessorReceiver(GateRef gate)
{
    EcmaOpcode ecmaOpcode = acc_.GetByteCodeOpcode(gate);
    if (UNLIKELY(ecmaOpcode == EcmaOpcode::STTHISBYNAME_IMM8_ID16 ||
                 ecmaOpcode == EcmaOpcode::STTHISBYNAME_IMM16_ID16)) {
        return argAcc_.GetFrameArgsIn(gate, FrameArgIdx::THIS_OBJECT);
    }
    return acc_.GetValueIn(gate, 2); // 2: receiver
}

GateRef TSInlineLowering::GetCallSetterValue(GateRef gate)
{
    EcmaOpcode ecmaOpcode = acc_.GetByteCodeOpcode(gate);
    if (ecmaOpcode == EcmaOpcode::STTHISBYNAME_IMM8_ID16 ||
        ecmaOpcode == EcmaOpcode::STTHISBYNAME_IMM16_ID16) {
        return acc_.GetValueIn(gate, 2); // 2: value
    }
    return acc_.GetValueIn(gate, 3); // 3: value
}

GateRef TSInlineLowering::GetFrameState(InlineTypeInfoAccessor &info)
{
    GateRef gate = info.GetCallGate();
    ASSERT(info.IsCallAccessor() || info.IsNormalCall());
    return acc_.GetFrameState(gate);
}

GateRef TSInlineLowering::GetFrameArgs(InlineTypeInfoAccessor &info)
{
    GateRef frameState = GetFrameState(info);
    return acc_.GetValueIn(frameState);
}

void TSInlineLowering::SetInitCallTargetAndConstPoolId(InlineTypeInfoAccessor &info)
{
    if (initCallTarget_ == Circuit::NullGate()) {
        GateRef frameArgs = GetFrameArgs(info);
        initCallTarget_ = acc_.GetValueIn(frameArgs, 0);
        const JSPandaFile *pf = ctx_->GetJSPandaFile();
        initConstantPoolId_ = tsManager_->GetConstantPoolIDByMethodOffset(pf, initMethodOffset_);
    }
}

int32_t TSInlineLowering::GetAccessorConstpoolId(InlineTypeInfoAccessor &info)
{
    ASSERT(info.IsCallAccessor());
    uint32_t inlineMethodOffset = info.GetCallMethodId();
    const JSPandaFile *pf = ctx_->GetJSPandaFile();
    return tsManager_->GetConstantPoolIDByMethodOffset(pf, inlineMethodOffset);
}
}  // namespace panda::ecmascript
