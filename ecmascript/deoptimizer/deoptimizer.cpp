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
#include "ecmascript/deoptimizer/deoptimizer.h"

#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/compiler/gate_meta_data.h"
#include "ecmascript/dfx/stackinfo/js_stackinfo.h"
#include "ecmascript/frames.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/stubs/runtime_stubs-inl.h"

namespace panda::ecmascript {
class FrameWriter {
public:
    explicit FrameWriter(Deoptimizier *deoptimizier) : thread_(deoptimizier->GetThread())
    {
        JSTaggedType *prevSp = const_cast<JSTaggedType *>(thread_->GetCurrentSPFrame());
        start_ = top_ = EcmaInterpreter::GetInterpreterFrameEnd(thread_, prevSp);
    }

    void PushValue(JSTaggedType value)
    {
        *(--top_) = value;
    }

    void PushRawValue(uintptr_t value)
    {
        *(--top_) = value;
    }

    bool Reserve(size_t size)
    {
        return !thread_->DoStackOverflowCheck(top_ - size);
    }

    AsmInterpretedFrame *ReserveAsmInterpretedFrame()
    {
        auto frame = AsmInterpretedFrame::GetFrameFromSp(top_);
        top_ = reinterpret_cast<JSTaggedType *>(frame);
        return frame;
    }

    JSTaggedType *GetStart() const
    {
        return start_;
    }

    JSTaggedType *GetTop() const
    {
        return top_;
    }

private:
    JSThread *thread_ {nullptr};
    JSTaggedType *start_;
    JSTaggedType *top_;
};

void Deoptimizier::CollectVregs(const std::vector<ARKDeopt>& deoptBundle)
{
    deoptVregs_.clear();
    for (size_t i = 0; i < deoptBundle.size(); i++) {
        ARKDeopt deopt = deoptBundle.at(i);
        JSTaggedType v;
        VRegId id = deopt.id;
        if (std::holds_alternative<DwarfRegAndOffsetType>(deopt.value)) {
            ASSERT(deopt.kind == LocationTy::Kind::INDIRECT);
            auto value = std::get<DwarfRegAndOffsetType>(deopt.value);
            DwarfRegType dwarfReg = value.first;
            OffsetType offset = value.second;
            ASSERT (dwarfReg == GCStackMapRegisters::FP || dwarfReg == GCStackMapRegisters::SP);
            uintptr_t addr;
            if (dwarfReg == GCStackMapRegisters::SP) {
                addr = context_.callsiteSp + offset;
            } else {
                addr = context_.callsiteFp + offset;
            }
            v = *(reinterpret_cast<JSTaggedType *>(addr));
        } else if (std::holds_alternative<LargeInt>(deopt.value)) {
            ASSERT(deopt.kind == LocationTy::Kind::CONSTANTNDEX);
            v = JSTaggedType(static_cast<int64_t>(std::get<LargeInt>(deopt.value)));
        } else {
            ASSERT(std::holds_alternative<IntType>(deopt.value));
            ASSERT(deopt.kind == LocationTy::Kind::CONSTANT);
            v = JSTaggedType(static_cast<int64_t>(std::get<IntType>(deopt.value)));
        }
        if (id != static_cast<VRegId>(SpecVregIndex::PC_OFFSET_INDEX)) {
            if (id == static_cast<VRegId>(SpecVregIndex::ENV_INDEX)) {
                env_ = JSTaggedValue(v);
            } else {
                deoptVregs_[id] = JSTaggedValue(v);
            }
        } else {
            pc_ = static_cast<uint32_t>(v);
        }
    }
}

// when AOT trigger deopt, frame layout as the following
// * OptimizedJSFunctionFrame layout description as the following:
//               +--------------------------+ ---------------
//               |        ......            |               ^
//               |        ......            |       callerFunction
//               |        ......            |               |
//               |--------------------------|               |
//               |        args              |               v
//               +--------------------------+ ---------------
//               |       returnAddr         |               ^
//               |--------------------------|               |
//               |       callsiteFp         |               |
//               |--------------------------|   OptimizedJSFunction  FrameType:OPTIMIZED_JS_FUNCTION_FRAME
//               |       frameType          |               |
//               |--------------------------|               |
//               |       call-target        |               |
//               |--------------------------|               |
//               |       lexEnv             |               |
//               |--------------------------|               |
//               |       ...........        |               v
//               +--------------------------+ ---------------
//               |       returnAddr         |               ^
//               |--------------------------|               |
//               |       callsiteFp         |               |
//               |--------------------------|   __llvm_deoptimize  FrameType:OPTIMIZED_FRAME
//               |       frameType          |               |
//               |--------------------------|               |
//               |       No CalleeSave      |               |
//               |       Registers          |               v
//               +--------------------------+ ---------------
//               |       returnAddr         |               ^
//               |--------------------------|               |
//               |       callsiteFp         |               |
//               |--------------------------|   DeoptHandlerAsm  FrameType:ASM_BRIDGE_FRAME
//               |       frameType          |               |
//               |--------------------------|               |
//               |       glue               |               |
//               |--------------------------|               |
//               | CalleeSave Registers     |               v
//               +--------------------------+ ---------------
//               |       .........          |               ^
//               |       .........          |     CallRuntime   FrameType:LEAVE_FRAME
//               |       .........          |               |
//               |       .........          |               v
//               |--------------------------| ---------------

void Deoptimizier::CollectDeoptBundleVec(std::vector<ARKDeopt>& deoptBundle)
{
    JSTaggedType *lastLeave = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
    FrameIterator it(lastLeave, thread_);
    // note: last deopt bridge frame is generated by DeoptHandlerAsm, callee Regs is grow from this frame
    for (; !it.Done() && deoptBundle.empty(); it.Advance<GCVisitedFlag::VISITED>()) {
        FrameType type = it.GetFrameType();
        switch (type) {
            case FrameType::OPTIMIZED_JS_FUNCTION_FRAME: {
                auto frame = it.GetFrame<OptimizedJSFunctionFrame>();
                frame->GetDeoptBundleInfo(it, deoptBundle);
                CalleeRegAndOffsetVec calleeRegInfo;
                frame->GetFuncCalleeRegAndOffset(it, calleeRegInfo);
                context_.calleeRegAndOffset = calleeRegInfo;
                context_.callsiteSp = it.GetCallSiteSp();
                context_.callsiteFp = reinterpret_cast<uintptr_t>(it.GetSp());
                auto preFrameSp = frame->ComputePrevFrameSp(it);
                frameArgc_ = frame->GetArgc(preFrameSp);
                frameArgvs_ = frame->GetArgv(preFrameSp);
                stackContext_.callFrameTop_ = it.GetPrevFrameCallSiteSp();
                stackContext_.returnAddr_ = frame->GetReturnAddr();
                stackContext_.callerFp_ = reinterpret_cast<uintptr_t>(frame->GetPrevFrameFp());
                break;
            }
            case FrameType::ASM_BRIDGE_FRAME: {
                auto sp = reinterpret_cast<uintptr_t*>(it.GetSp());
                static constexpr size_t TYPE_GLUE_SLOT = 2; // 2: skip type & glue
                sp -= TYPE_GLUE_SLOT;
                calleeRegAddr_ = sp - numCalleeRegs_;
                break;
            }
            case FrameType::OPTIMIZED_FRAME:
            case FrameType::LEAVE_FRAME:
                break;
            default: {
                LOG_FULL(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    }
    ASSERT(!it.Done());
}

Method* Deoptimizier::GetMethod(JSTaggedValue &target)
{
    ECMAObject *callTarget = reinterpret_cast<ECMAObject*>(target.GetTaggedObject());
    ASSERT(callTarget != nullptr);
    Method *method = callTarget->GetCallTarget();
    return method;
}

void Deoptimizier::RelocateCalleeSave()
{
    CalleeReg callreg;
    for (auto &it: context_.calleeRegAndOffset) {
        auto reg = it.first;
        auto offset = it.second;
        uintptr_t value = *(reinterpret_cast<uintptr_t *>(context_.callsiteFp + offset));
        int order = callreg.FindCallRegOrder(reg);
        calleeRegAddr_[order] = value;
    }
}

bool Deoptimizier::CollectVirtualRegisters(Method* method, FrameWriter *frameWriter)
{
    int32_t actualNumArgs = static_cast<int32_t>(frameArgc_) - NUM_MANDATORY_JSFUNC_ARGS;
    bool haveExtra = method->HaveExtraWithCallField();
    int32_t declaredNumArgs = static_cast<int32_t>(method->GetNumArgsWithCallField());
    int32_t callFieldNumVregs = static_cast<int32_t>(method->GetNumVregsWithCallField());
    // layout of frame:
    // [maybe argc] [actual args] [reserved args] [call field virtual regs]

    // [maybe argc]
    if (declaredNumArgs != actualNumArgs && haveExtra) {
        auto value = JSTaggedValue(actualNumArgs);
        frameWriter->PushValue(value.GetRawData());
    }
    int32_t reservedCount = std::max(actualNumArgs, declaredNumArgs);
    int32_t virtualIndex = reservedCount + callFieldNumVregs +
        static_cast<int32_t>(method->GetNumRevervedArgs()) - 1;
    if (!frameWriter->Reserve(static_cast<size_t>(virtualIndex))) {
        return false;
    }
    // [actual args]
    if (declaredNumArgs > actualNumArgs) {
        for (int32_t i = 0; i < declaredNumArgs - actualNumArgs; i++) {
            frameWriter->PushValue(JSTaggedValue::Undefined().GetRawData());
            virtualIndex--;
        }
    }
    for (int32_t i = actualNumArgs - 1; i >= 0; i--) {
        JSTaggedValue value = JSTaggedValue::Undefined();
        // deopt value
        if (HasDeoptValue(virtualIndex)) {
            value = deoptVregs_.at(static_cast<VRegId>(virtualIndex));
        } else {
            value = GetActualFrameArgs(i);
        }
        frameWriter->PushValue(value.GetRawData());
        virtualIndex--;
    }

    // [reserved args]
    if (method->HaveThisWithCallField()) {
        JSTaggedValue value = deoptVregs_.at(static_cast<VRegId>(SpecVregIndex::THIS_OBJECT_INDEX));
        frameWriter->PushValue(value.GetRawData());
        virtualIndex--;
    }
    if (method->HaveNewTargetWithCallField()) {
        JSTaggedValue value = deoptVregs_.at(static_cast<VRegId>(SpecVregIndex::NEWTARGET_INDEX));
        frameWriter->PushValue(value.GetRawData());
        virtualIndex--;
    }
    if (method->HaveFuncWithCallField()) {
        JSTaggedValue value = deoptVregs_.at(static_cast<VRegId>(SpecVregIndex::FUNC_INDEX));
        frameWriter->PushValue(value.GetRawData());
        virtualIndex--;
    }

    // [call field virtual regs]
    for (int32_t i = virtualIndex; i >= 0; i--) {
        JSTaggedValue value = GetDeoptValue(virtualIndex);
        frameWriter->PushValue(value.GetRawData());
        virtualIndex--;
    }
    return true;
}

void Deoptimizier::Dump(Method* method, DeoptType type)
{
    if (traceDeopt_) {
        std::string checkType = DisplayItems(type);
        LOG_COMPILER(INFO) << "Check Type: " << checkType;
        std::string data = JsStackInfo::BuildJsStackTrace(thread_, true);
        LOG_COMPILER(INFO) << "Deoptimize" << data;
        const uint8_t *pc = method->GetBytecodeArray() + pc_;
        BytecodeInstruction inst(pc);
        LOG_COMPILER(INFO) << inst;
    }
}

std::string Deoptimizier::DisplayItems(DeoptType type)
{
    switch (type) {
        case DeoptType::NOTINT:
            return "NOT INT";
        case DeoptType::DIVZERO:
            return "DIV ZERO";
        case DeoptType::NOTDOUBLE:
            return "NOT DOUBLE";
        case DeoptType::NOTNUMBER:
            return "NOT NUMBER";
        case DeoptType::NOTBOOL:
            return "NOT BOOL";
        case DeoptType::NOTHEAPOBJECT:
            return "NOT HEAP OBJECT";
        case DeoptType::NOTSARRAY:
            return "NOT SARRAY";
        case DeoptType::NOTF32ARRAY:
            return "NOT F32ARRAY";
        case DeoptType::INCONSISTENTHCLASS:
            return "INCONSISTENT HCLASS";
        case kungfu::DeoptType::NOTNEWOBJ:
            return "NOT NEWOBJ TYPE";
        case DeoptType::NOTARRAYIDX:
            return "NOT ARRAY IDX";
        case DeoptType::NOTF32ARRAYIDX:
            return "NOT F32 ARRAY IDX";
        case DeoptType::NOTINCOV:
            return "NOT INC OVERFLOW";
        case DeoptType::NOTDECOV:
            return "NOT DEC OVERFLOW";
        case DeoptType::NOTNEGOV:
            return "NOT NEG OVERFLOW";
        case DeoptType::NOTCALLTGT:
            return "NOT CALL TARGET";
        default: {
            return "NOT CHECK";
        }
    }
}

JSTaggedType Deoptimizier::ConstructAsmInterpretFrame(DeoptType type)
{
    JSTaggedValue callTarget = GetDeoptValue(static_cast<int32_t>(SpecVregIndex::FUNC_INDEX));
    auto method = GetMethod(callTarget);
    Dump(method, type);
    ASSERT(thread_ != nullptr);
    uint8_t deoptThreshold = method->GetDeoptThreshold();
    if (deoptThreshold > 0) {
        method->SetDeoptType(type);
        method->SetDeoptThreshold(--deoptThreshold);
    } else {
        method->ClearAOTFlags();
    }

    FrameWriter frameWriter(this);
    // Push asm interpreter frame
    if (!CollectVirtualRegisters(method, &frameWriter)) {
        return JSTaggedValue::Exception().GetRawData();
    }
    AsmInterpretedFrame *statePtr = frameWriter.ReserveAsmInterpretedFrame();
    const uint8_t *resumePc = method->GetBytecodeArray() + pc_;

    JSTaggedValue thisObj = GetDeoptValue(static_cast<int32_t>(SpecVregIndex::THIS_OBJECT_INDEX));
    auto acc = GetDeoptValue(static_cast<int32_t>(SpecVregIndex::ACC_INDEX));
    statePtr->function = callTarget;
    statePtr->acc = acc;
    statePtr->env = env_;
    statePtr->callSize = 0;
    statePtr->fp = 0;  // need update
    statePtr->thisObj = thisObj;
    statePtr->pc = resumePc;
    // -uintptr_t skip lr
    statePtr->base.prev = reinterpret_cast<JSTaggedType *>(
        stackContext_.callFrameTop_ - sizeof(uintptr_t));
    statePtr->base.type = FrameType::ASM_INTERPRETER_FRAME;

    // construct stack context
    auto start = frameWriter.GetStart();
    auto end = frameWriter.GetTop();
    auto outputCount = start - end;

    RelocateCalleeSave();

    frameWriter.PushRawValue(stackContext_.callerFp_);
    frameWriter.PushRawValue(stackContext_.returnAddr_);
    frameWriter.PushRawValue(stackContext_.callFrameTop_);
    frameWriter.PushRawValue(outputCount);
    return reinterpret_cast<JSTaggedType>(frameWriter.GetTop());
}
}  // namespace panda::ecmascript
