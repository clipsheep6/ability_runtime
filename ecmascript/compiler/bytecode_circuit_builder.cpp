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

#include "ecmascript/compiler/bytecode_circuit_builder.h"

#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/ts_types/ts_manager.h"
#include "libpandafile/bytecode_instruction-inl.h"

namespace panda::ecmascript::kungfu {
void BytecodeCircuitBuilder::BytecodeToCircuit()
{
    ExceptionInfo exceptionInfo = {};

    // collect try catch block info
    CollectTryCatchBlockInfo(exceptionInfo);
    hasTryCatch_ = exceptionInfo.size() != 0;
    BuildRegionInfo();
    // Building the basic block diagram of bytecode
    BuildRegions(exceptionInfo);
}

void BytecodeCircuitBuilder::BuildRegionInfo()
{
    uint32_t size = pcOffsets_.size();
    uint32_t end = size - 1;  // 1: end
    BytecodeIterator iterator(this, 0, end);

    infoData_.resize(size);
    byteCodeToJSGates_.resize(size, std::vector<GateRef>(0));
    regionsInfo_.InsertHead(0); // 0: start pc
    iterator.GotoStart();
    while (!iterator.Done()) {
        auto index = iterator.Index();
        auto &info = infoData_[index];
        auto pc = pcOffsets_[index];
        info.metaData_ = bytecodes_->GetBytecodeMetaData(pc);
        ASSERT(!info.metaData_.IsInvalid());
        BytecodeInfo::InitBytecodeInfo(this, info, pc);
        CollectRegionInfo(index);
        ++iterator;
    }
}

void BytecodeCircuitBuilder::CollectRegionInfo(uint32_t bcIndex)
{
    auto pc = pcOffsets_[bcIndex];
    auto &info = infoData_[bcIndex];
    int32_t offset = 0;
    if (info.IsJump()) {
        switch (info.GetOpcode()) {
            case EcmaOpcode::JEQZ_IMM8:
            case EcmaOpcode::JNEZ_IMM8:
            case EcmaOpcode::JMP_IMM8:
                offset = static_cast<int8_t>(READ_INST_8_0());
                break;
            case EcmaOpcode::JNEZ_IMM16:
            case EcmaOpcode::JEQZ_IMM16:
            case EcmaOpcode::JMP_IMM16:
                offset = static_cast<int16_t>(READ_INST_16_0());
                break;
            case EcmaOpcode::JMP_IMM32:
            case EcmaOpcode::JNEZ_IMM32:
            case EcmaOpcode::JEQZ_IMM32:
                offset = static_cast<int32_t>(READ_INST_32_0());
                break;
            default:
                LOG_ECMA(FATAL) << "this branch is unreachable";
                UNREACHABLE();
                break;
        }
        auto nextIndex = bcIndex + 1; // 1: next pc
        auto targetIndex = FindBcIndexByPc(pc + offset);
        // condition branch current basic block end
        if (info.IsCondJump()) {
            regionsInfo_.InsertSplit(nextIndex);
            regionsInfo_.InsertJump(targetIndex, bcIndex, false);
        } else {
            if (bcIndex != GetLastBcIndex()) {
                regionsInfo_.InsertHead(nextIndex);
            }
            regionsInfo_.InsertJump(targetIndex, bcIndex, true);
        }
    } else if (info.IsReturn() || info.IsThrow()) {
        if (bcIndex != GetLastBcIndex()) {
            auto nextIndex = bcIndex + 1; // 1: next pc
            regionsInfo_.InsertHead(nextIndex);
        }
    }
}

void BytecodeCircuitBuilder::CollectTryCatchBlockInfo(ExceptionInfo &byteCodeException)
{
    auto pf = file_->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, method_->GetMethodId());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());

    cda.EnumerateTryBlocks([this, &byteCodeException](
        panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        auto tryStartOffset = tryBlock.GetStartPc();
        auto tryEndOffset = tryBlock.GetStartPc() + tryBlock.GetLength();

        auto tryStartPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + tryStartOffset);
        auto tryEndPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + tryEndOffset);
        // skip try blocks with same pc in start and end label
        if (tryStartPc == tryEndPc) {
            return true;
        }

        auto tryStartBcIndex = FindBcIndexByPc(tryStartPc);
        regionsInfo_.InsertSplit(tryStartBcIndex);
        if (tryEndPc <= GetLastPC()) {
            auto tryEndBcIndex = FindBcIndexByPc(tryEndPc);
            regionsInfo_.InsertSplit(tryEndBcIndex);
        }
        byteCodeException.emplace_back(ExceptionItem { tryStartPc, tryEndPc, {} });
        tryBlock.EnumerateCatchBlocks([&](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
            auto pcOffset = catchBlock.GetHandlerPc();
            auto catchBlockPc = const_cast<uint8_t *>(method_->GetBytecodeArray() + pcOffset);
            auto catchBlockBcIndex = FindBcIndexByPc(catchBlockPc);
            regionsInfo_.InsertHead(catchBlockBcIndex);
            // try block associate catch block
            byteCodeException.back().catches.emplace_back(catchBlockPc);
            return true;
        });
        return true;
    });
}

void BytecodeCircuitBuilder::BuildEntryBlock()
{
    BytecodeRegion &entryBlock = RegionAt(0);
    BytecodeRegion &nextBlock = RegionAt(1);
    entryBlock.succs.emplace_back(&nextBlock);
    nextBlock.preds.emplace_back(&entryBlock);
    entryBlock.bytecodeIterator_.Reset(this, 0, BytecodeIterator::INVALID_INDEX);
}

void BytecodeCircuitBuilder::BuildRegions(const ExceptionInfo &byteCodeException)
{
    auto &items = regionsInfo_.GetBlockItems();
    auto blockSize = items.size();

    // 1 : entry block. if the loop head is in the first bb block, the variables used in the head cannot correctly
    // generate Phi nodes through the dominator-tree algorithm, resulting in an infinite loop. Therefore, an empty
    // BB block is generated as an entry block
    graph_.resize(blockSize + 1, nullptr);
    for (size_t i = 0; i < graph_.size(); i++) {
        graph_[i] = circuit_->chunk()->New<BytecodeRegion>(circuit_->chunk());
    }

    // build entry block
    BuildEntryBlock();

    // build basic block
    size_t blockId = 1;
    for (const auto &item : items) {
        auto &curBlock = GetBasicBlockById(blockId);
        curBlock.id = blockId;
        curBlock.start = item.GetStartBcIndex();
        if (blockId != 1) {
            auto &prevBlock = RegionAt(blockId - 1);
            prevBlock.end = curBlock.start - 1;
            prevBlock.bytecodeIterator_.Reset(this, prevBlock.start, prevBlock.end);
            // fall through
            if (!item.IsHeadBlock()) {
                curBlock.preds.emplace_back(&prevBlock);
                prevBlock.succs.emplace_back(&curBlock);
            }
        }
        blockId++;
    }
    auto &lastBlock = RegionAt(blockId - 1); // 1: last block
    lastBlock.end = GetLastBcIndex();
    lastBlock.bytecodeIterator_.Reset(this, lastBlock.start, lastBlock.end);

    auto &splitItems = regionsInfo_.GetSplitItems();
    for (const auto &item : splitItems) {
        auto curIndex = regionsInfo_.FindBBIndexByBcIndex(item.startBcIndex);
        auto &curBlock = GetBasicBlockById(curIndex);
        auto predIndex = regionsInfo_.FindBBIndexByBcIndex(item.predBcIndex);
        auto &predBlock = GetBasicBlockById(predIndex);
        curBlock.preds.emplace_back(&predBlock);
        predBlock.succs.emplace_back(&curBlock);
    }

    if (byteCodeException.size() != 0) {
        BuildCatchBlocks(byteCodeException);
    }
    UpdateCFG();
    if (HasTryCatch()) {
        CollectTryPredsInfo();
    }
    RemoveUnreachableRegion();
    if (IsLogEnabled()) {
        PrintGraph("Update CFG");
    }
    BuildCircuit();
}

void BytecodeCircuitBuilder::BuildCatchBlocks(const ExceptionInfo &byteCodeException)
{
    // try catch block associate
    for (size_t i = 0; i < graph_.size(); i++) {
        auto &bb = RegionAt(i);
        auto startIndex = bb.start;
        const auto pc = pcOffsets_[startIndex];
        for (auto it = byteCodeException.cbegin(); it != byteCodeException.cend(); it++) {
            if (pc < it->startPc || pc >= it->endPc) {
                continue;
            }
            // try block interval
            const auto &catches = it->catches; // catches start pc
            for (size_t j = i + 1; j < graph_.size(); j++) {
                auto &catchBB = RegionAt(j);
                const auto catchStart = pcOffsets_[catchBB.start];
                if (std::find(catches.cbegin(), catches.cend(), catchStart) != catches.cend()) {
                    bb.catches.insert(bb.catches.cbegin(), &catchBB);
                    bb.succs.emplace_back(&catchBB);
                    catchBB.preds.emplace_back(&bb);
                }
            }
        }

        // When there are multiple catch blocks in the current block, the set of catch blocks
        // needs to be sorted to satisfy the order of execution of catch blocks.
        bb.SortCatches();
    }
}

void BytecodeCircuitBuilder::CollectTryPredsInfo()
{
    for (size_t i = 0; i < graph_.size(); i++) {
        auto &bb = RegionAt(i);
        if (bb.catches.empty()) {
            continue;
        } else if (bb.catches.size() > 1) { // 1: cache size
            for (auto it = bb.catches.begin() + 1; it != bb.catches.end();) { // 1: invalid catch bb
                bb.EraseThisBlock((*it)->trys);
                it = bb.catches.erase(it);
            }
        }

        EnumerateBlock(bb, [&bb](const BytecodeInfo &bytecodeInfo) -> bool {
            if (bytecodeInfo.IsGeneral()) {
                // if block which can throw exception has serval catchs block,
                // only the innermost catch block is useful
                ASSERT(bb.catches.size() == 1); // 1: cache size
                if (!bytecodeInfo.NoThrow()) {
                    bb.catches.at(0)->numOfStatePreds++;
                }
            }
            return true;
        });
    }
}

void BytecodeCircuitBuilder::RemoveUnusedPredsInfo(BytecodeRegion& bb)
{
    EnumerateBlock(bb, [&bb](const BytecodeInfo &bytecodeInfo) -> bool {
        if (bytecodeInfo.IsGeneral()) {
            ASSERT(bb.catches.size() == 1); // 1: cache size
            if (!bytecodeInfo.NoThrow()) {
                bb.catches.at(0)->numOfStatePreds--;
            }
        }
        return true;
    });
}

void BytecodeCircuitBuilder::ClearUnreachableRegion(ChunkVector<BytecodeRegion*>& pendingList)
{
    auto bb = pendingList.back();
    pendingList.pop_back();
    for (auto it = bb->preds.begin(); it != bb->preds.end(); it++) {
        if ((*it)->numOfStatePreds != 0) {
            bb->EraseThisBlock((*it)->succs);
        }
    }
    for (auto it = bb->succs.begin(); it != bb->succs.end(); it++) {
        auto bbNext = *it;
        if (bbNext->numOfStatePreds != 0) {
            bb->EraseThisBlock(bbNext->preds);
            bbNext->numOfStatePreds--;
            if (bbNext->numOfStatePreds == 0) {
                pendingList.emplace_back(bbNext);
            }
        }
    }
    for (auto it = bb->trys.begin(); it != bb->trys.end(); it++) {
        if ((*it)->numOfStatePreds != 0) {
            bb->EraseThisBlock((*it)->catches);
        }
    }
    for (auto it = bb->catches.begin(); it != bb->catches.end(); it++) {
        auto bbNext = *it;
        if (bbNext->numOfStatePreds != 0) {
            RemoveUnusedPredsInfo(*bb);
            bb->EraseThisBlock(bbNext->trys);
            if (bbNext->numOfStatePreds == 0) {
                pendingList.emplace_back(bbNext);
            }
        }
    }
    bb->preds.clear();
    bb->succs.clear();
    bb->trys.clear();
    bb->catches.clear();
    numOfLiveBB_--;
}

void BytecodeCircuitBuilder::RemoveUnreachableRegion()
{
    numOfLiveBB_ = graph_.size();
    ChunkVector<BytecodeRegion*> pendingList(circuit_->chunk());
    for (size_t i = 1; i < graph_.size(); i++) { // 1: skip entry bb
        auto &bb = RegionAt(i);
        if (bb.numOfStatePreds == 0) {
            pendingList.emplace_back(&bb);
        }
    }
    while (!pendingList.empty()) {
        ClearUnreachableRegion(pendingList);
    }
}

// Update CFG's predecessor, successor and try catch associations
void BytecodeCircuitBuilder::UpdateCFG()
{
    for (size_t i = 0; i < graph_.size(); i++) {
        auto &bb = RegionAt(i);
        bb.preds.clear();
        bb.trys.clear();
        ChunkVector<BytecodeRegion *> newSuccs(circuit_->chunk());
        for (const auto &succ: bb.succs) {
            if (std::count(bb.catches.cbegin(), bb.catches.cend(), succ)) {
                continue;
            }
            newSuccs.emplace_back(succ);
        }
        bb.succs.clear();
        bb.succs.insert(bb.succs.end(), newSuccs.begin(), newSuccs.end());
    }
    for (size_t i = 0; i < graph_.size(); i++) {
        auto &bb = RegionAt(i);
        for (auto &succ: bb.succs) {
            succ->preds.emplace_back(&bb);
            succ->numOfStatePreds++;
        }
        for (auto &catchBlock: bb.catches) {
            catchBlock->trys.emplace_back(&bb);
        }
    }
}

// build circuit
void BytecodeCircuitBuilder::BuildCircuitArgs()
{
    argAcc_.NewCommonArg(CommonArgIdx::GLUE, MachineType::I64, GateType::NJSValue());
    if (!method_->IsFastCall()) {
        argAcc_.NewCommonArg(CommonArgIdx::ACTUAL_ARGC, MachineType::I64, GateType::NJSValue());
        auto funcIdx = static_cast<size_t>(CommonArgIdx::FUNC);
        const size_t actualNumArgs = argAcc_.GetActualNumArgs();
        // new actual argument gates
        for (size_t argIdx = funcIdx; argIdx < actualNumArgs; argIdx++) {
            argAcc_.NewArg(argIdx);
        }
    } else {
        auto funcIdx = static_cast<size_t>(FastCallArgIdx::FUNC);
        size_t actualNumArgs = static_cast<size_t>(FastCallArgIdx::NUM_OF_ARGS) + method_->GetNumArgsWithCallField();
        for (size_t argIdx = funcIdx; argIdx < actualNumArgs; argIdx++) {
            argAcc_.NewArg(argIdx);
        }
    }
    argAcc_.CollectArgs();
    if (HasTypes()) {
        argAcc_.FillArgsGateType(&typeRecorder_);
    }

    BuildFrameArgs();
}

void BytecodeCircuitBuilder::BuildFrameArgs()
{
    UInt32PairAccessor accessor(0, 0);
    auto metaData = circuit_->FrameArgs(accessor.ToValue());
    size_t numArgs = metaData->GetNumIns();
    std::vector<GateRef> args(numArgs, Circuit::NullGate());
    size_t idx = 0;
    args[idx++] = argAcc_.GetCommonArgGate(CommonArgIdx::FUNC);
    args[idx++] = argAcc_.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
    args[idx++] = argAcc_.GetCommonArgGate(CommonArgIdx::THIS_OBJECT);
    args[idx++] = argAcc_.GetCommonArgGate(CommonArgIdx::ACTUAL_ARGC);
    args[idx++] = GetPreFrameArgs();
    GateRef frameArgs = circuit_->NewGate(metaData, args);
    argAcc_.SetFrameArgs(frameArgs);
}

std::vector<GateRef> BytecodeCircuitBuilder::CreateGateInList(
    const BytecodeInfo &info, const GateMetaData *meta)
{
    auto numValues = meta->GetNumIns();
    const size_t length = meta->GetInValueStarts();
    std::vector<GateRef> inList(numValues, Circuit::NullGate());
    auto inputSize = info.inputs.size();
    for (size_t i = 0; i < inputSize; i++) {
        auto &input = info.inputs[i];
        if (std::holds_alternative<ConstDataId>(input)) {
            inList[i + length] = circuit_->GetConstantGate(MachineType::I64,
                                                           std::get<ConstDataId>(input).GetId(),
                                                           GateType::NJSValue());
        } else if (std::holds_alternative<Immediate>(input)) {
            inList[i + length] = circuit_->GetConstantGate(MachineType::I64,
                                                           std::get<Immediate>(input).GetValue(),
                                                           GateType::NJSValue());
        } else if (std::holds_alternative<ICSlotId>(input)) {
            inList[i + length] = circuit_->GetConstantGate(MachineType::I16,
                                                           std::get<ICSlotId>(input).GetId(),
                                                           GateType::NJSValue());
        } else {
            ASSERT(std::holds_alternative<VirtualRegister>(input));
            continue;
        }
    }
    if (info.AccIn()) {
        inputSize++;
    }
    if (meta->HasFrameState()) {
        inList[inputSize + length] = GetFrameArgs();
    }
    return inList;
}

GateRef BytecodeCircuitBuilder::NewConst(const BytecodeInfo &info)
{
    auto opcode = info.GetOpcode();
    GateRef gate = 0;
    switch (opcode) {
        case EcmaOpcode::LDNAN:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             base::NumberHelper::GetNaN(),
                                             GateType::NumberType());
            break;
        case EcmaOpcode::LDINFINITY:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             base::NumberHelper::GetPositiveInfinity(),
                                             GateType::NumberType());
            break;
        case EcmaOpcode::LDUNDEFINED:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             JSTaggedValue::VALUE_UNDEFINED,
                                             GateType::UndefinedType());
            break;
        case EcmaOpcode::LDNULL:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             JSTaggedValue::VALUE_NULL,
                                             GateType::NullType());
            break;
        case EcmaOpcode::LDTRUE:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             JSTaggedValue::VALUE_TRUE,
                                             GateType::BooleanType());
            break;
        case EcmaOpcode::LDFALSE:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             JSTaggedValue::VALUE_FALSE,
                                             GateType::BooleanType());
            break;
        case EcmaOpcode::LDHOLE:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             JSTaggedValue::VALUE_HOLE,
                                             GateType::TaggedValue());
            break;
        case EcmaOpcode::LDAI_IMM32:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             std::get<Immediate>(info.inputs[0]).ToJSTaggedValueInt(),
                                             GateType::IntType());
            break;
        case EcmaOpcode::FLDAI_IMM64:
            gate = circuit_->GetConstantGate(MachineType::I64,
                                             std::get<Immediate>(info.inputs.at(0)).ToJSTaggedValueDouble(),
                                             GateType::DoubleType());
            break;
        case EcmaOpcode::LDFUNCTION:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::FUNC);
            break;
        case EcmaOpcode::LDNEWTARGET:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::NEW_TARGET);
            break;
        case EcmaOpcode::LDTHIS:
            gate = argAcc_.GetCommonArgGate(CommonArgIdx::THIS_OBJECT);
            break;
        default:
            LOG_ECMA(FATAL) << "this branch is unreachable";
            UNREACHABLE();
    }
    return gate;
}

void BytecodeCircuitBuilder::MergeThrowGate(BytecodeRegion &bb, uint32_t bcIndex)
{
    auto state = frameStateBuilder_.GetCurrentState();
    auto depend = frameStateBuilder_.GetCurrentDepend();
    if (!bb.catches.empty()) {
        auto ifSuccess = circuit_->NewGate(circuit_->IfSuccess(), {state});
        auto ifException = circuit_->NewGate(circuit_->IfException(), {state, depend});
        frameStateBuilder_.UpdateStateDepend(ifException, ifException);
        ASSERT(bb.catches.size() == 1); // 1: one catch
        auto bbNext = bb.catches.at(0);
        frameStateBuilder_.MergeIntoSuccessor(bb, *bbNext);
        bbNext->expandedPreds.push_back({bb.id, bcIndex, true});
        state = ifSuccess;
    }
    auto constant = circuit_->GetConstantGate(MachineType::I64,
                                              JSTaggedValue::VALUE_EXCEPTION,
                                              GateType::TaggedValue());
    circuit_->NewGate(circuit_->Return(),
        { state, depend, constant, circuit_->GetReturnRoot() });
}

void BytecodeCircuitBuilder::MergeExceptionGete(BytecodeRegion &bb,
    const BytecodeInfo& bytecodeInfo, uint32_t bcIndex)
{
    auto state = frameStateBuilder_.GetCurrentState();
    auto depend = frameStateBuilder_.GetCurrentDepend();
    auto ifSuccess = circuit_->NewGate(circuit_->IfSuccess(), {state});
    ASSERT(bb.catches.size() == 1); // 1: one catch
    auto bbNext = bb.catches.at(0);
    auto ifException = circuit_->NewGate(circuit_->IfException(), {state, depend});
    frameStateBuilder_.UpdateStateDepend(ifException, ifException);
    frameStateBuilder_.MergeIntoSuccessor(bb, *bbNext);
    if (bytecodeInfo.GetOpcode() == EcmaOpcode::CREATEASYNCGENERATOROBJ_V8) {
        bbNext->expandedPreds.push_back({bb.id, bcIndex + 1, true}); // 1: next pc
    } else {
        bbNext->expandedPreds.push_back({bb.id, bcIndex, true});
    }
    frameStateBuilder_.UpdateStateDepend(ifSuccess, depend);
}

void BytecodeCircuitBuilder::NewJSGate(BytecodeRegion &bb)
{
    auto &iterator = bb.GetBytecodeIterator();
    const BytecodeInfo& bytecodeInfo = iterator.GetBytecodeInfo();
    GateRef state = frameStateBuilder_.GetCurrentState();
    GateRef depend = frameStateBuilder_.GetCurrentDepend();
    size_t numValueInputs = bytecodeInfo.ComputeValueInputCount();
    GateRef gate = 0;
    bool writable = !bytecodeInfo.NoSideEffects();
    bool hasFrameState = bytecodeInfo.HasFrameState();
    size_t pcOffset = GetPcOffset(iterator.Index());
    auto meta = circuit_->JSBytecode(numValueInputs, bytecodeInfo.GetOpcode(), pcOffset, writable, hasFrameState);
    std::vector<GateRef> inList = CreateGateInList(bytecodeInfo, meta);
    if (bytecodeInfo.IsDef()) {
        gate = circuit_->NewGate(meta, MachineType::I64, inList.size(),
                                 inList.data(), GateType::AnyType());
    } else {
        gate = circuit_->NewGate(meta, MachineType::NOVALUE, inList.size(),
                                 inList.data(), GateType::Empty());
    }
    byteCodeToJSGates_[iterator.Index()].emplace_back(gate);
    jsGatesToByteCode_[gate] = iterator.Index();
    gateAcc_.NewIn(gate, 0, state);
    gateAcc_.NewIn(gate, 1, depend);
    frameStateBuilder_.UpdateStateDepend(gate, gate);
    frameStateBuilder_.UpdateFrameValues(bytecodeInfo, iterator.Index(), gate);
    if (bytecodeInfo.IsThrow()) {
        MergeThrowGate(bb, iterator.Index());
        return;
    }

    if (!bb.catches.empty() && !bytecodeInfo.NoThrow()) {
        MergeExceptionGete(bb, bytecodeInfo, iterator.Index());
    }
    if (bytecodeInfo.IsGeneratorRelative()) {
        suspendAndResumeGates_.emplace_back(gate);
    }
}

void BytecodeCircuitBuilder::NewJump(BytecodeRegion &bb)
{
    auto &iterator = bb.GetBytecodeIterator();
    const BytecodeInfo& bytecodeInfo = iterator.GetBytecodeInfo();
    GateRef state = frameStateBuilder_.GetCurrentState();
    GateRef depend = frameStateBuilder_.GetCurrentDepend();
    size_t numValueInputs = bytecodeInfo.ComputeValueInputCount();
    if (bytecodeInfo.IsCondJump() && bb.succs.size() == 2) { // 2: two succ
        size_t pcOffset = GetPcOffset(iterator.Index());
        auto meta = circuit_->JSBytecode(numValueInputs, bytecodeInfo.GetOpcode(), pcOffset, false, false);
        auto numValues = meta->GetNumIns();
        GateRef gate = circuit_->NewGate(meta, std::vector<GateRef>(numValues, Circuit::NullGate()));
        gateAcc_.NewIn(gate, 0, state);
        gateAcc_.NewIn(gate, 1, depend);
        frameStateBuilder_.UpdateStateDepend(gate, gate);
        frameStateBuilder_.UpdateFrameValues(bytecodeInfo, iterator.Index(), gate);

        auto ifTrue = circuit_->NewGate(circuit_->IfTrue(), {gate});
        auto trueRelay = circuit_->NewGate(circuit_->DependRelay(), {ifTrue, gate});
        auto ifFalse = circuit_->NewGate(circuit_->IfFalse(), {gate});
        auto falseRelay = circuit_->NewGate(circuit_->DependRelay(), {ifFalse, gate});
        for (auto &bbNext: bb.succs) {
            if (bbNext->id == bb.id + 1) {
                frameStateBuilder_.UpdateStateDepend(ifFalse, falseRelay);
                frameStateBuilder_.MergeIntoSuccessor(bb, *bbNext);
                bbNext->expandedPreds.push_back({bb.id, iterator.Index(), false});
            } else {
                frameStateBuilder_.UpdateStateDepend(ifTrue, trueRelay);
                frameStateBuilder_.MergeIntoSuccessor(bb, *bbNext);
                bbNext->expandedPreds.push_back({bb.id, iterator.Index(), false});
            }
        }
        byteCodeToJSGates_[iterator.Index()].emplace_back(gate);
        jsGatesToByteCode_[gate] = iterator.Index();
    } else {
        ASSERT(bb.succs.size() == 1);
        auto &bbNext = bb.succs.at(0);
        frameStateBuilder_.MergeIntoSuccessor(bb, *bbNext);
        bbNext->expandedPreds.push_back({bb.id, iterator.Index(), false});
    }
}

GateRef BytecodeCircuitBuilder::NewReturn(BytecodeRegion &bb)
{
    ASSERT(bb.succs.empty());
    auto &iterator = bb.GetBytecodeIterator();
    const BytecodeInfo& bytecodeInfo = iterator.GetBytecodeInfo();
    GateRef state = frameStateBuilder_.GetCurrentState();
    GateRef depend = frameStateBuilder_.GetCurrentDepend();
    GateRef gate = Circuit::NullGate();
    if (bytecodeInfo.GetOpcode() == EcmaOpcode::RETURN) {
        // handle return.dyn bytecode
        gate = circuit_->NewGate(circuit_->Return(),
            { state, depend, Circuit::NullGate(), circuit_->GetReturnRoot() });
        byteCodeToJSGates_[iterator.Index()].emplace_back(gate);
        jsGatesToByteCode_[gate] = iterator.Index();
    } else if (bytecodeInfo.GetOpcode() == EcmaOpcode::RETURNUNDEFINED) {
        // handle returnundefined bytecode
        auto constant = circuit_->GetConstantGate(MachineType::I64,
                                                  JSTaggedValue::VALUE_UNDEFINED,
                                                  GateType::TaggedValue());
        gate = circuit_->NewGate(circuit_->Return(),
            { state, depend, constant, circuit_->GetReturnRoot() });
        byteCodeToJSGates_[iterator.Index()].emplace_back(gate);
        jsGatesToByteCode_[gate] = iterator.Index();
    }
    return gate;
}

void BytecodeCircuitBuilder::NewByteCode(BytecodeRegion &bb)
{
    auto &iterator = bb.GetBytecodeIterator();
    const BytecodeInfo& bytecodeInfo = iterator.GetBytecodeInfo();
    FrameLiveOut* liveout;
    auto bcId = iterator.Index();
    if (iterator.IsInRange(bcId - 1)) {
        liveout = frameStateBuilder_.GetOrOCreateBCEndLiveOut(bcId - 1);
    } else {
        liveout = frameStateBuilder_.GetOrOCreateBBLiveOut(bb.id);
    }
    frameStateBuilder_.AdvanceToNextBc(bytecodeInfo, liveout, bcId);
    GateRef gate = Circuit::NullGate();
    if (bytecodeInfo.IsSetConstant()) {
        // handle bytecode command to get constants
        gate = NewConst(bytecodeInfo);
        byteCodeToJSGates_[iterator.Index()].emplace_back(gate);
        jsGatesToByteCode_[gate] = iterator.Index();
    } else if (bytecodeInfo.IsGeneral()) {
        // handle general ecma.* bytecodes
        NewJSGate(bb);
    } else if (bytecodeInfo.IsJump()) {
        // handle conditional jump and unconditional jump bytecodes
        NewJump(bb);
    } else if (bytecodeInfo.IsReturn()) {
        // handle return.dyn and returnundefined bytecodes
        gate = NewReturn(bb);
    } else if (bytecodeInfo.IsMov()) {
        frameStateBuilder_.UpdateMoveValues(bytecodeInfo, iterator.Index());
    } else if (!bytecodeInfo.IsDiscarded()) {
        LOG_ECMA(FATAL) << "this branch is unreachable";
        UNREACHABLE();
    }
    if (gate != Circuit::NullGate()) {
        frameStateBuilder_.UpdateFrameValues(bytecodeInfo, iterator.Index(), gate);
    }
}

void BytecodeCircuitBuilder::BuildSubCircuit()
{
    auto &entryBlock = RegionAt(0);
    frameStateBuilder_.InitEntryBB(entryBlock);
    auto& rpoList = frameStateBuilder_.GetRpoList();
    for (auto &bbId: rpoList) {
        auto &bb = RegionAt(bbId);
        frameStateBuilder_.AdvanceToNextBB(bb);
        if (IsEntryBlock(bb.id)) {
            if (NeedCheckSafePointAndStackOver()) {
                GateRef state = frameStateBuilder_.GetCurrentState();
                GateRef depend = frameStateBuilder_.GetCurrentDepend();
                auto stackCheck = circuit_->NewGate(circuit_->CheckSafePointAndStackOver(), {state, depend});
                bb.dependCache = stackCheck;
                frameStateBuilder_.UpdateStateDepend(stackCheck, stackCheck);
            }
            auto &bbNext = RegionAt(bb.id + 1);
            frameStateBuilder_.MergeIntoSuccessor(bb, bbNext);
            bbNext.expandedPreds.push_back({bb.id, bb.end, false});
            continue;
        }
        if (!bb.trys.empty()) {
            GateRef state = frameStateBuilder_.GetCurrentState();
            GateRef depend = frameStateBuilder_.GetCurrentDepend();
            auto getException = circuit_->NewGate(circuit_->GetException(),
                MachineType::I64, {state, depend}, GateType::AnyType());
            frameStateBuilder_.UpdateAccumulator(getException);
            frameStateBuilder_.UpdateStateDepend(state, getException);
        }
        EnumerateBlock(bb, [this, &bb]
            (const BytecodeInfo &bytecodeInfo) -> bool {
            NewByteCode(bb);
            if (bytecodeInfo.IsJump() || bytecodeInfo.IsThrow()) {
                return false;
            }
            return true;
        });
        bool needFallThrough = true;
        if (!bb.IsEmptryBlock()) {
            const BytecodeInfo& bytecodeInfo = GetBytecodeInfo(bb.end);
            needFallThrough = bytecodeInfo.needFallThrough();
        }
        // fallThrough or empty merge bb
        if (needFallThrough) {
            ASSERT(bb.succs.size() == 1); // 1: fall through
            auto &bbNext = RegionAt(bb.succs[0]->id);
            frameStateBuilder_.MergeIntoSuccessor(bb, bbNext);
            bbNext.expandedPreds.push_back({bb.id, bb.end, false});
        }
    }
}

void BytecodeCircuitBuilder::BuildCircuit()
{
    // create arg gates array
    BuildCircuitArgs();
    frameStateBuilder_.DoBytecodeAnalysis();
    // build states sub-circuit of each block
    BuildSubCircuit();
    if (IsLogEnabled()) {
        PrintGraph("Bytecode2Gate");
        LOG_COMPILER(INFO) << "\033[34m" << "============= "
                           << "After bytecode2circuit lowering ["
                           << methodName_ << "]"
                           << " =============" << "\033[0m";
        circuit_->PrintAllGatesWithBytecode();
        LOG_COMPILER(INFO) << "\033[34m" << "=========================== End ===========================" << "\033[0m";
    }
}

void BytecodeCircuitBuilder::PrintGraph(const char* title)
{
    LOG_COMPILER(INFO) << "======================== " << title << " ========================";
    for (size_t i = 0; i < graph_.size(); i++) {
        BytecodeRegion& bb = RegionAt(i);
        if (!IsEntryBlock(bb.id) && bb.numOfStatePreds == 0) {
            LOG_COMPILER(INFO) << "B" << bb.id << ":                               ;preds= invalid BB";
            LOG_COMPILER(INFO) << "\tBytecodePC: [" << std::to_string(bb.start) << ", "
                               << std::to_string(bb.end) << ")";
            continue;
        }
        std::string log("B" + std::to_string(bb.id) + ":                               ;preds= ");
        for (size_t k = 0; k < bb.preds.size(); ++k) {
            log += std::to_string(bb.preds[k]->id) + ", ";
        }
        LOG_COMPILER(INFO) << log;
        if (IsEntryBlock(bb.id)) {
            LOG_COMPILER(INFO) << "\tBytecodePC: Empty";
        } else {
            LOG_COMPILER(INFO) << "\tBytecodePC: [" << std::to_string(bb.start) << ", "
                << std::to_string(bb.end) << ")";
        }

        std::string log1("\tSucces: ");
        for (size_t j = 0; j < bb.succs.size(); j++) {
            log1 += std::to_string(bb.succs[j]->id) + ", ";
        }
        LOG_COMPILER(INFO) << log1;

        for (size_t j = 0; j < bb.catches.size(); j++) {
            LOG_COMPILER(INFO) << "\tcatch [: " << std::to_string(bb.catches[j]->start) << ", "
                               << std::to_string(bb.catches[j]->end) << ")";
        }

        std::string log2("\tTrys: ");
        for (auto tryBlock: bb.trys) {
            log2 += std::to_string(tryBlock->id) + " , ";
        }
        LOG_COMPILER(INFO) << log2;

        PrintBytecodeInfo(bb);
        LOG_COMPILER(INFO) << "";
    }
}

void BytecodeCircuitBuilder::PrintBytecodeInfo(BytecodeRegion& bb)
{
    if (IsEntryBlock(bb.id)) {
        LOG_COMPILER(INFO) << "\tBytecode[] = Empty";
        return;
    }
    LOG_COMPILER(INFO) << "\tBytecode[] = ";
    EnumerateBlock(bb, [&](const BytecodeInfo &bytecodeInfo) -> bool {
        auto &iterator = bb.GetBytecodeIterator();
        std::string log;
        log += std::string("\t\t< ") + std::to_string(iterator.Index()) + ": ";
        log += GetEcmaOpcodeStr(iterator.GetBytecodeInfo().GetOpcode()) + ", " + "In=[";
        if (bytecodeInfo.AccIn()) {
            log += "acc,";
        }
        for (const auto &in: bytecodeInfo.inputs) {
            if (std::holds_alternative<VirtualRegister>(in)) {
                log += std::to_string(std::get<VirtualRegister>(in).GetId()) + ",";
            }
        }
        log += "], Out=[";
        if (bytecodeInfo.AccOut()) {
            log += "acc,";
        }
        for (const auto &out: bytecodeInfo.vregOut) {
            log += std::to_string(out) + ",";
        }
        log += "] >";
        LOG_COMPILER(INFO) << log;

        auto gate = GetGateByBcIndex(iterator.Index());
        if (gate != Circuit::NullGate()) {
            this->gateAcc_.ShortPrint(gate);
        }
        return true;
    });
}
}  // namespace panda::ecmascript::kungfu
