/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ecmascript/debugger/dropframe_manager.h"

#include "ecmascript/frames.h"
#include "ecmascript/global_env.h"
#include "ecmascript/global_handle_collection.h"
#include "ecmascript/interpreter/frame_handler.h"
#include "ecmascript/interpreter/interpreter-inl.h"

namespace panda::ecmascript::tooling {
bool DropframeManager::IsNewlexenvOpcode(BytecodeInstruction::Opcode op)
{
    switch (op) {
        case BytecodeInstruction::Opcode::NEWLEXENV_IMM8:
        case BytecodeInstruction::Opcode::NEWLEXENVWITHNAME_IMM8_ID16:
        case BytecodeInstruction::Opcode::WIDE_NEWLEXENV_PREF_IMM16:
        case BytecodeInstruction::Opcode::WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16:
            return true;
        default:
            break;
    }
    return false;
}

bool DropframeManager::IsStlexvarOpcode(BytecodeInstruction::Opcode op)
{
    switch (op) {
        case BytecodeInstruction::Opcode::STLEXVAR_IMM4_IMM4:
        case BytecodeInstruction::Opcode::STLEXVAR_IMM8_IMM8:
        case BytecodeInstruction::Opcode::WIDE_STLEXVAR_PREF_IMM16_IMM16:
            return true;
        default:
            break;
    }
    return false;
}

std::pair<uint16_t, uint16_t> DropframeManager::ReadStlexvarParams(const uint8_t *pc, BytecodeInstruction::Opcode op)
{
    uint16_t level = 0;
    uint16_t slot = 0;
    switch (op) {
        case BytecodeInstruction::Opcode::STLEXVAR_IMM4_IMM4:
            level = READ_INST_4_0();
            slot = READ_INST_4_1();
            break;
        case BytecodeInstruction::Opcode::STLEXVAR_IMM8_IMM8:
            level = READ_INST_8_0();
            slot = READ_INST_8_1();
            break;
        case BytecodeInstruction::Opcode::WIDE_STLEXVAR_PREF_IMM16_IMM16:
            level = READ_INST_16_1();
            slot = READ_INST_16_3();
            break;
        default:
            break;
    }
    return std::make_pair(level, slot);
}

void DropframeManager::MethodEntry(JSThread *thread, JSHandle<Method> method)
{
    uint32_t codeSize = method->GetCodeSize();
    uint16_t newEnvCount = 0;
    FrameHandler frameHandler(thread);
    std::set<std::pair<uint16_t, uint16_t>> modifiedLexVarPos;
    NewLexModifyRecordLevel();
    auto bcIns = BytecodeInstruction(method->GetBytecodeArray());
    auto bcInsLast = bcIns.JumpTo(codeSize);
    while (bcIns.GetAddress() != bcInsLast.GetAddress()) {
        BytecodeInstruction::Opcode op = bcIns.GetOpcode();
        if (IsNewlexenvOpcode(op)) {
            newEnvCount++;
        } else if (IsStlexvarOpcode(op)) {
            std::pair<uint16_t, uint16_t> lexVarPos = ReadStlexvarParams(bcIns.GetAddress(), op);
            uint16_t level;
            uint16_t slot;
            std::tie(level, slot) = lexVarPos;
            JSTaggedValue env = frameHandler.GetEnv();
            for (uint16_t i = 0; ; i++) {
                if ((level < newEnvCount || i >= level - newEnvCount) &&
                    slot < LexicalEnv::Cast(env.GetTaggedObject())->GetLength() - LexicalEnv::RESERVED_ENV_LENGTH &&
                    !modifiedLexVarPos.count({i, slot})) {
                    JSTaggedValue value = LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot);
                    EmplaceLexModifyRecord(thread, env, slot, value);
                    modifiedLexVarPos.insert({i, slot});
                }
                if (i >= level) {
                    break;
                }
                JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
                if (taggedParentEnv.IsUndefined()) {
                    break;
                }
                env = taggedParentEnv;
            }
        }
        bcIns = bcIns.GetNext();
    }
}

void DropframeManager::MethodExit(JSThread *thread, [[maybe_unused]] JSHandle<Method> method)
{
    MergeLexModifyRecordOfTopFrame(thread);
}

void DropframeManager::DropLastFrame(JSThread *thread)
{
    std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>> lexModifyRecord;
    lexModifyRecord = GetLexModifyRecordOfTopFrame();
    for (const auto &item : lexModifyRecord) {
        JSHandle<JSTaggedValue> envHandle;
        uint16_t slot;
        JSHandle<JSTaggedValue> valueHandle;
        std::tie(envHandle, slot, valueHandle) = item;
        JSTaggedValue env = envHandle.GetTaggedValue();
        ASSERT(slot < LexicalEnv::Cast(env.GetTaggedObject())->GetLength() - LexicalEnv::RESERVED_ENV_LENGTH);
        LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot, valueHandle.GetTaggedValue());
    }
    RemoveLexModifyRecordOfTopFrame(thread);

    FrameHandler frameHandler(thread);
    bool isEntryFrameDropped = false;
    while (frameHandler.HasFrame()) {
        frameHandler.PrevJSFrame();
        if (frameHandler.IsEntryFrame()) {
            isEntryFrameDropped = true;
            continue;
        }
        if (frameHandler.IsBuiltinFrame()) {
            continue;
        }
        if (!thread->IsAsmInterpreter()) {
            JSTaggedType *prevSp = frameHandler.GetSp();
            thread->SetCurrentFrame(prevSp);
        }
        if (isEntryFrameDropped) {
            thread->SetEntryFrameDroppedState();
        }
        thread->SetFrameDroppedState();
        break;
    }
}

void DropframeManager::NewLexModifyRecordLevel()
{
    modifiedLexVar_.push(std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>>());
}

void DropframeManager::EmplaceLexModifyRecord(JSThread *thread, JSTaggedValue env, uint16_t slot, JSTaggedValue value)
{
    GlobalHandleCollection globalHandleCollection(thread);
    for (const auto &item : modifiedLexVar_.top()) {
        JSHandle<JSTaggedValue> envHandleRecord = std::get<0>(item);
        uint16_t slotRecord = std::get<1>(item);
        if (envHandleRecord.GetTaggedType() == env.GetRawData() && slotRecord == slot) {
            return;
        }
    }
    JSHandle<JSTaggedValue> envHandle = globalHandleCollection.NewHandle<JSTaggedValue>(env.GetRawData());
    JSHandle<JSTaggedValue> valueHandle = globalHandleCollection.NewHandle<JSTaggedValue>(value.GetRawData());
    modifiedLexVar_.top().emplace_back(envHandle, slot, valueHandle);
}

uint32_t DropframeManager::GetLexModifyRecordLevel()
{
    return modifiedLexVar_.size();
}

std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>>
    DropframeManager::GetLexModifyRecordOfTopFrame()
{
    if (modifiedLexVar_.empty()) {
        return std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>>();
    }
    return modifiedLexVar_.top();
}

void DropframeManager::RemoveLexModifyRecordOfTopFrame(JSThread *thread)
{
    if (modifiedLexVar_.empty()) {
        return;
    }
    GlobalHandleCollection globalHandleCollection(thread);
    for (const auto &item : modifiedLexVar_.top()) {
        JSHandle<JSTaggedValue> envHandle = std::get<0>(item);
        JSHandle<JSTaggedValue> valueHandle = std::get<2>(item); // get the third item of the tuple
        globalHandleCollection.Dispose(envHandle);
        globalHandleCollection.Dispose(valueHandle);
    }
    modifiedLexVar_.pop();
}

void DropframeManager::MergeLexModifyRecordOfTopFrame(JSThread *thread)
{
    if (modifiedLexVar_.empty()) {
        return;
    }
    GlobalHandleCollection globalHandleCollection(thread);
    std::vector<std::tuple<JSHandle<JSTaggedValue>, uint16_t, JSHandle<JSTaggedValue>>> lexModifyRecord;
    lexModifyRecord = modifiedLexVar_.top();
    modifiedLexVar_.pop();
    for (const auto &item : lexModifyRecord) {
        JSHandle<JSTaggedValue> envHandle;
        uint16_t slot;
        JSHandle<JSTaggedValue> valueHandle;
        std::tie(envHandle, slot, valueHandle) = item;
        bool existRecord = false;
        if (!modifiedLexVar_.empty()) {
            for (const auto &itemLast : modifiedLexVar_.top()) {
                JSHandle<JSTaggedValue> envHandleRecord = std::get<0>(itemLast);
                uint16_t slotRecord = std::get<1>(itemLast);
                if (envHandleRecord.GetTaggedType() == envHandle.GetTaggedType() && slotRecord == slot) {
                    existRecord = true;
                    break;
                }
            }
        }
        if (modifiedLexVar_.empty() || existRecord) {
            globalHandleCollection.Dispose(envHandle);
            globalHandleCollection.Dispose(valueHandle);
        } else {
            modifiedLexVar_.top().emplace_back(envHandle, slot, valueHandle);
        }
    }
}
}