
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

#include "ecmascript/interpreter/frame_handler.h"

#include "ecmascript/file_loader.h"
#include "ecmascript/llvm_stackmap_parser.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/mem/heap.h"

namespace panda::ecmascript {
FrameHandler::FrameHandler(const JSThread *thread)
    : sp_(const_cast<JSTaggedType *>(thread->GetCurrentFrame())), thread_(thread)
{
    AdvanceToInterpretedFrame();
}

FrameHandler::FrameHandler(const JSThread *thread, void *fp)
    : sp_(reinterpret_cast<JSTaggedType *>(fp)), thread_(thread)
{
    AdvanceToInterpretedFrame();
}

ARK_INLINE void FrameHandler::AdvanceToInterpretedFrame()
{
    if (!thread_->IsAsmInterpreter()) {
        return;
    }
    FrameIterator it(sp_, thread_);
    for (; !it.Done(); it.Advance()) {
        FrameType t = it.GetFrameType();
        if (IsInterpretedFrame(t) || IsInterpretedEntryFrame(t)) {
            break;
        }
    }
    sp_ = it.GetSp();
}

ARK_INLINE void FrameHandler::PrevInterpretedFrame()
{
    if (!thread_->IsAsmInterpreter()) {
        FrameIterator it(sp_, thread_);
        it.Advance();
        sp_ = it.GetSp();
        return;
    }
    AdvanceToInterpretedFrame();
    FrameIterator it(sp_, thread_);
    FrameType t = it.GetFrameType();
    if (t == FrameType::ASM_INTERPRETER_FRAME) {
        auto frame = it.GetFrame<AsmInterpretedFrame>();
        if (thread_->IsAsmInterpreter()) {
            fp_ = frame->GetCurrentFramePointer();
        }
    }
    it.Advance();
    sp_ = it.GetSp();
    AdvanceToInterpretedFrame();
}

JSTaggedType* FrameHandler::GetPrevInterpretedFrame()
{
    PrevInterpretedFrame();
    return GetSp();
}

uint32_t FrameHandler::GetNumberArgs()
{
    if (thread_->IsAsmInterpreter()) {
        auto *frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
        return static_cast<uint32_t>(frame->GetCurrentFramePointer() - sp_);
    }
    ASSERT(IsInterpretedFrame());
    JSTaggedType *prevSp = nullptr;
    FrameIterator it(sp_, thread_);
    if (IsAsmInterpretedFrame()) {
        auto *frame = it.GetFrame<AsmInterpretedFrame>();
        prevSp = frame->GetPrevFrameFp();
    } else {
        auto *frame = it.GetFrame<InterpretedFrame>();
        prevSp = frame->GetPrevFrameFp();
    }
    auto prevSpEnd = reinterpret_cast<JSTaggedType*>(GetInterpretedFrameEnd(prevSp));
    return static_cast<uint32_t>(prevSpEnd - sp_);
}

JSTaggedValue FrameHandler::GetVRegValue(size_t index) const
{
    ASSERT(IsInterpretedFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return JSTaggedValue(sp_[index]);
}

void FrameHandler::SetVRegValue(size_t index, JSTaggedValue value)
{
    ASSERT(IsInterpretedFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    sp_[index] = value.GetRawData();
}

JSTaggedValue FrameHandler::GetAcc() const
{
    ASSERT(IsInterpretedFrame());
    FrameIterator it(sp_, thread_);
    if (IsAsmInterpretedFrame()) {
        auto *frame = it.GetFrame<AsmInterpretedFrame>();
        return frame->acc;
    } else {
        auto *frame = it.GetFrame<InterpretedFrame>();
        return frame->acc;
    }
}

uint32_t FrameHandler::GetBytecodeOffset() const
{
    ASSERT(IsInterpretedFrame());
    Method *method = GetMethod();
    auto offset = GetPc() - method->GetBytecodeArray();
    return static_cast<uint32_t>(offset);
}

Method *FrameHandler::GetMethod() const
{
    ASSERT(IsInterpretedFrame());
    auto function = GetFunction();
    return ECMAObject::Cast(function.GetTaggedObject())->GetCallTarget();
}

Method *FrameHandler::CheckAndGetMethod() const
{
    ASSERT(IsInterpretedFrame());
    auto function = GetFunction();
    if (function.IsJSFunctionBase() || function.IsJSProxy()) {
        return ECMAObject::Cast(function.GetTaggedObject())->GetCallTarget();
    }
    return nullptr;
}

JSTaggedValue FrameHandler::GetThis() const
{
    ASSERT(IsInterpretedFrame());
    FrameIterator it(sp_, thread_);
    if (IsAsmInterpretedFrame()) {
        auto *frame = it.GetFrame<AsmInterpretedFrame>();
        return frame->thisObj;
    } else {
        auto *frame = it.GetFrame<InterpretedFrame>();
        return frame->thisObj;
    }
}

JSTaggedValue FrameHandler::GetFunction() const
{
    ASSERT(IsInterpretedFrame());
    if (thread_->IsAsmInterpreter()) {
        FrameType type = GetFrameType();
        switch (type) {
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
                auto frame = AsmInterpretedFrame::GetFrameFromSp(sp_);
                return frame->function;
            }
            case FrameType::BUILTIN_FRAME_WITH_ARGV: {
                auto *frame = BuiltinWithArgvFrame::GetFrameFromSp(sp_);
                return frame->GetFunction();
            }
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME: {
                auto *frame = BuiltinFrame::GetFrameFromSp(sp_);
                return frame->GetFunction();
            }
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME:
            case FrameType::INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_BRIDGE_FRAME:
            case FrameType::INTERPRETER_BUILTIN_FRAME:
            case FrameType::OPTIMIZED_FRAME:
            case FrameType::LEAVE_FRAME:
            case FrameType::LEAVE_FRAME_WITH_ARGV:
            case FrameType::BUILTIN_CALL_LEAVE_FRAME:
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            default: {
                LOG_FULL(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    } else {
        FrameType type = GetFrameType();
        if (type == FrameType::INTERPRETER_FRAME || type == FrameType::INTERPRETER_FAST_NEW_FRAME) {
            auto *frame = InterpretedFrame::GetFrameFromSp(sp_);
            return frame->function;
        } else {
            auto *frame = InterpretedBuiltinFrame::GetFrameFromSp(sp_);
            return frame->function;
        }
    }
}

const uint8_t *FrameHandler::GetPc() const
{
    ASSERT(IsInterpretedFrame());
    FrameIterator it(sp_, thread_);
    if (IsAsmInterpretedFrame()) {
        auto *frame = it.GetFrame<AsmInterpretedFrame>();
        return frame->GetPc();
    } else {
        auto *frame = it.GetFrame<InterpretedFrame>();
        return frame->GetPc();
    }
}

ConstantPool *FrameHandler::GetConstpool() const
{
    ASSERT(IsInterpretedFrame());
    auto method = GetMethod();
    JSTaggedValue constpool = method->GetConstantPool();
    return ConstantPool::Cast(constpool.GetTaggedObject());
}

JSTaggedValue FrameHandler::GetEnv() const
{
    ASSERT(IsInterpretedFrame());
    FrameIterator it(sp_, thread_);
    if (IsAsmInterpretedFrame()) {
        auto *frame = it.GetFrame<AsmInterpretedFrame>();
        return frame->GetEnv();
    } else {
        auto *frame = it.GetFrame<InterpretedFrame>();
        return frame->env;
    }
}

void FrameHandler::DumpStack(std::ostream &os) const
{
    size_t i = 0;
    FrameHandler frameHandler(thread_);
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        os << "[" << i++
        << "]:" << frameHandler.GetMethod()->ParseFunctionName()
        << "\n";
    }
}

void FrameHandler::DumpPC(std::ostream &os, const uint8_t *pc) const
{
    FrameHandler frameHandler(thread_);
    ASSERT(frameHandler.HasFrame());
    // NOLINTNEXTLINE(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
    int offset = pc - frameHandler.GetMethod()->GetBytecodeArray();
    os << "offset: " << offset << "\n";
}

ARK_INLINE uintptr_t FrameHandler::GetInterpretedFrameEnd(JSTaggedType *prevSp) const
{
    uintptr_t end = 0U;
    FrameIterator it(prevSp, thread_);
    FrameType type = it.GetFrameType();
    switch (type) {
        case FrameType::ASM_INTERPRETER_FRAME:
        case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
            auto frame = it.GetFrame<AsmInterpretedFrame>();
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::INTERPRETER_FRAME:
        case FrameType::INTERPRETER_FAST_NEW_FRAME: {
            auto frame = it.GetFrame<InterpretedFrame>();
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::INTERPRETER_ENTRY_FRAME: {
            auto frame = it.GetFrame<InterpretedEntryFrame>();
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::INTERPRETER_BUILTIN_FRAME: {
            auto frame = it.GetFrame<InterpretedBuiltinFrame>();
            end = ToUintPtr(frame);
            break;
        }
        case FrameType::BUILTIN_FRAME_WITH_ARGV:
        case FrameType::BUILTIN_ENTRY_FRAME:
        case FrameType::BUILTIN_FRAME:
        case FrameType::OPTIMIZED_FRAME:
        case FrameType::LEAVE_FRAME:
        case FrameType::LEAVE_FRAME_WITH_ARGV:
        case FrameType::BUILTIN_CALL_LEAVE_FRAME:
        case FrameType::OPTIMIZED_ENTRY_FRAME:
        case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
        case FrameType::ASM_INTERPRETER_BRIDGE_FRAME:
        default: {
            LOG_FULL(FATAL) << "frame type error!";
            UNREACHABLE();
        }
    }
    return end;
}

void FrameHandler::IterateAssembleStack(const RootVisitor &visitor, const RootRangeVisitor &rangeVisitor,
    const RootBaseAndDerivedVisitor &derivedVisitor)
{
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
    IterateFrameChain(current, visitor, rangeVisitor, derivedVisitor);
}

// We seperate InterpretedEntryFrame from assemble stack when asm interpreter is enable.
// To protect EcmaRuntimeCallInfo on InterpretedEntryFrame, we iterate InterpretedEntryFrame on thread sp individually.
// And only InterpretedEntryFrame is on thread sp when asm interpreter is enable.
void FrameHandler::IterateEcmaRuntimeCallInfo(const RootVisitor &visitor, const RootRangeVisitor &rangeVisitor)
{
    ASSERT(thread_->IsAsmInterpreter());
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetCurrentSPFrame());
    for (FrameIterator it(current, thread_); !it.Done(); it.Advance()) {
        ASSERT(it.GetFrameType() == FrameType::INTERPRETER_ENTRY_FRAME);
        auto frame = it.GetFrame<InterpretedEntryFrame>();
        frame->GCIterate(it, visitor, rangeVisitor);
    }
}

void FrameHandler::Iterate(const RootVisitor &visitor, const RootRangeVisitor &rangeVisitor,
    const RootBaseAndDerivedVisitor &derivedVisitor)
{
    if (thread_->IsAsmInterpreter()) {
        IterateEcmaRuntimeCallInfo(visitor, rangeVisitor);
        IterateAssembleStack(visitor, rangeVisitor, derivedVisitor);
        return;
    }
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetCurrentSPFrame());
    FrameType frameType = FrameHandler::GetFrameType(current);
    if (frameType != FrameType::INTERPRETER_ENTRY_FRAME) {
        auto leaveFrame = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
        if (leaveFrame != nullptr) {
            current = leaveFrame;
        }
    }
    // lazy assignment: only Iterate need arkStackMapParser_ in order to high improve performance
    if (arkStackMapParser_ == nullptr) {
        arkStackMapParser_ = thread_->GetEcmaVM()->GetFileLoader()->GetStackMapParser();
    }
    IterateFrameChain(current, visitor, rangeVisitor, derivedVisitor);
}

void FrameHandler::IterateFrameChain(JSTaggedType *start, const RootVisitor &visitor,
    const RootRangeVisitor &rangeVisitor, const RootBaseAndDerivedVisitor &derivedVisitor) const
{
    JSTaggedType *current = start;
    for (FrameIterator it(current, thread_); !it.Done(); it.Advance<GCVisitedFlag::VISITED>()) {
        FrameType type = it.GetFrameType();
        switch (type) {
            case FrameType::OPTIMIZED_FRAME: {
                auto frame = it.GetFrame<OptimizedFrame>();
                frame->GCIterate(it, visitor, rangeVisitor, derivedVisitor);
                break;
            }
            case FrameType::OPTIMIZED_JS_FUNCTION_FRAME: {
                auto frame = it.GetFrame<OptimizedJSFunctionFrame>();
                frame->GCIterate(it, visitor, rangeVisitor, derivedVisitor);
                break;
            }
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME: {
                auto frame = it.GetFrame<AsmInterpretedFrame>();
                frame->GCIterate(it, visitor, rangeVisitor, derivedVisitor);
                break;
            }
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME: {
                auto frame = it.GetFrame<InterpretedFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::INTERPRETER_BUILTIN_FRAME: {
                auto frame = it.GetFrame<InterpretedBuiltinFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::LEAVE_FRAME: {
                auto frame = it.GetFrame<OptimizedLeaveFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::LEAVE_FRAME_WITH_ARGV: {
                auto frame = it.GetFrame<OptimizedWithArgvLeaveFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::BUILTIN_CALL_LEAVE_FRAME: {
                auto frame = it.GetFrame<OptimizedBuiltinLeaveFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::BUILTIN_FRAME_WITH_ARGV: {
                auto frame = it.GetFrame<BuiltinWithArgvFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME: {
                auto frame = it.GetFrame<BuiltinFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::INTERPRETER_ENTRY_FRAME: {
                auto frame = it.GetFrame<InterpretedEntryFrame>();
                frame->GCIterate(it, visitor, rangeVisitor);
                break;
            }
            case FrameType::OPTIMIZED_JS_FUNCTION_UNFOLD_ARGV_FRAME:
            case FrameType::OPTIMIZED_JS_FUNCTION_ARGS_CONFIG_FRAME:
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_BRIDGE_FRAME: {
                break;
            }
            default: {
                LOG_FULL(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    }
}

std::string FrameBcCollector::GetAotExceptionFuncName(JSTaggedType* argv) const
{
    JSTaggedValue func = JSTaggedValue(*(argv)); // 3: skip returnaddr and argc
    Method *method = JSFunction::Cast(func.GetTaggedObject())->GetCallTarget();
    return method->GetMethodName();
}

void FrameBcCollector::CollectBCOffsetInfo()
{
    thread_->GetEcmaVM()->ClearExceptionBCList();
    JSTaggedType *current = const_cast<JSTaggedType *>(thread_->GetLastLeaveFrame());
    FrameIterator it(current, thread_);
    it.Advance<GCVisitedFlag::VISITED>();

    for (; !it.Done(); it.Advance<GCVisitedFlag::VISITED>()) {
        FrameType type = it.GetFrameType();
        switch (type) {
            case FrameType::OPTIMIZED_JS_FUNCTION_FRAME: {
                auto frame = it.GetFrame<OptimizedJSFunctionFrame>();
                kungfu::ConstInfo constInfo;
                frame->CollectBCOffsetInfo(it, constInfo);
                if (!constInfo.empty()) {
                    auto name = GetAotExceptionFuncName(frame->GetArgv(it));
                    thread_->GetEcmaVM()->StoreBCOffsetInfo(name, constInfo[0]);
                }
                break;
            }
            case FrameType::OPTIMIZED_JS_FUNCTION_ARGS_CONFIG_FRAME:
            case FrameType::BUILTIN_CALL_LEAVE_FRAME:
            case FrameType::LEAVE_FRAME:
            case FrameType::OPTIMIZED_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_BRIDGE_FRAME:
            case FrameType::ASM_INTERPRETER_ENTRY_FRAME:
            case FrameType::ASM_INTERPRETER_FRAME:
            case FrameType::INTERPRETER_CONSTRUCTOR_FRAME:
            case FrameType::INTERPRETER_FRAME:
            case FrameType::INTERPRETER_FAST_NEW_FRAME:
            case FrameType::OPTIMIZED_FRAME:
            case FrameType::OPTIMIZED_JS_FUNCTION_UNFOLD_ARGV_FRAME:
            case FrameType::LEAVE_FRAME_WITH_ARGV:
            case FrameType::BUILTIN_FRAME_WITH_ARGV:
            case FrameType::BUILTIN_ENTRY_FRAME:
            case FrameType::BUILTIN_FRAME:
            case FrameType::INTERPRETER_ENTRY_FRAME: {
                break;
            }
            default: {
                LOG_FULL(FATAL) << "frame type error!";
                UNREACHABLE();
            }
        }
    }
}
}  // namespace panda::ecmascript
