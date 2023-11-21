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

#include "cg_irbuilder.h"
#include "isa.h"
#include "cg.h"
#include "cfi.h"
#include "dbg.h"

namespace maplebe {
Insn &InsnBuilder::BuildInsn(MOperator opCode, const InsnDesc &idesc)
{
    auto *newInsn = mp->New<Insn>(*mp, opCode);
    newInsn->SetInsnDescrption(idesc);
    IncreaseInsnNum();
    return *newInsn;
}

Insn &InsnBuilder::BuildInsn(MOperator opCode, Operand &o0)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    return BuildInsn(opCode, tMd).AddOpndChain(o0);
}
Insn &InsnBuilder::BuildInsn(MOperator opCode, Operand &o0, Operand &o1)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    return BuildInsn(opCode, tMd).AddOpndChain(o0).AddOpndChain(o1);
}
Insn &InsnBuilder::BuildInsn(MOperator opCode, Operand &o0, Operand &o1, Operand &o2)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    return BuildInsn(opCode, tMd).AddOpndChain(o0).AddOpndChain(o1).AddOpndChain(o2);
}

Insn &InsnBuilder::BuildInsn(MOperator opCode, Operand &o0, Operand &o1, Operand &o2, Operand &o3)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    return BuildInsn(opCode, tMd).AddOpndChain(o0).AddOpndChain(o1).AddOpndChain(o2).AddOpndChain(o3);
}

Insn &InsnBuilder::BuildInsn(MOperator opCode, Operand &o0, Operand &o1, Operand &o2, Operand &o3, Operand &o4)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    Insn &nI = BuildInsn(opCode, tMd);
    return nI.AddOpndChain(o0).AddOpndChain(o1).AddOpndChain(o2).AddOpndChain(o3).AddOpndChain(o4);
}

Insn &InsnBuilder::BuildInsn(MOperator opCode, std::vector<Operand *> &opnds)
{
    const InsnDesc &tMd = Globals::GetInstance()->GetTarget()->GetTargetMd(opCode);
    Insn &nI = BuildInsn(opCode, tMd);
    for (auto *opnd : opnds) {
        nI.AddOperand(*opnd);
    }
    return nI;
}

Insn &InsnBuilder::BuildCfiInsn(MOperator opCode)
{
    auto *nI = mp->New<cfi::CfiInsn>(*mp, opCode);
    IncreaseInsnNum();
    return *nI;
}
Insn &InsnBuilder::BuildDbgInsn(MOperator opCode)
{
    auto *nI = mp->New<mpldbg::DbgInsn>(*mp, opCode);
    IncreaseInsnNum();
    return *nI;
}

VectorInsn &InsnBuilder::BuildVectorInsn(MOperator opCode, const InsnDesc &idesc)
{
    auto *newInsn = mp->New<VectorInsn>(*mp, opCode);
    newInsn->SetInsnDescrption(idesc);
    IncreaseInsnNum();
    return *newInsn;
}

ImmOperand &OperandBuilder::CreateImm(uint32 size, int64 value, MemPool *mp)
{
    return mp ? *mp->New<ImmOperand>(value, size, false) : *alloc.New<ImmOperand>(value, size, false);
}

ImmOperand &OperandBuilder::CreateImm(const MIRSymbol &symbol, int64 offset, int32 relocs, MemPool *mp)
{
    return mp ? *mp->New<ImmOperand>(symbol, offset, relocs, false)
              : *alloc.New<ImmOperand>(symbol, offset, relocs, false);
}

MemOperand &OperandBuilder::CreateMem(uint32 size, MemPool *mp)
{
    return mp ? *mp->New<MemOperand>(size) : *alloc.New<MemOperand>(size);
}

MemOperand &OperandBuilder::CreateMem(RegOperand &baseOpnd, int64 offset, uint32 size)
{
    MemOperand *memOprand = &CreateMem(size);
    memOprand->SetBaseRegister(baseOpnd);
    memOprand->SetOffsetOperand(CreateImm(baseOpnd.GetSize(), offset));
    return *memOprand;
}

RegOperand &OperandBuilder::CreateVReg(uint32 size, RegType type, MemPool *mp)
{
    virtualRegNum++;
    regno_t vRegNO = baseVirtualRegNO + virtualRegNum;
    return mp ? *mp->New<RegOperand>(vRegNO, size, type) : *alloc.New<RegOperand>(vRegNO, size, type);
}

RegOperand &OperandBuilder::CreateVReg(regno_t vRegNO, uint32 size, RegType type, MemPool *mp)
{
    return mp ? *mp->New<RegOperand>(vRegNO, size, type) : *alloc.New<RegOperand>(vRegNO, size, type);
}

RegOperand &OperandBuilder::CreatePReg(regno_t pRegNO, uint32 size, RegType type, MemPool *mp)
{
    return mp ? *mp->New<RegOperand>(pRegNO, size, type) : *alloc.New<RegOperand>(pRegNO, size, type);
}

ListOperand &OperandBuilder::CreateList(MemPool *mp)
{
    return mp ? *mp->New<ListOperand>(alloc) : *alloc.New<ListOperand>(alloc);
}

FuncNameOperand &OperandBuilder::CreateFuncNameOpnd(MIRSymbol &symbol, MemPool *mp)
{
    return mp ? *mp->New<FuncNameOperand>(symbol) : *alloc.New<FuncNameOperand>(symbol);
}

LabelOperand &OperandBuilder::CreateLabel(const char *parent, LabelIdx idx, MemPool *mp)
{
    return mp ? *mp->New<LabelOperand>(parent, idx) : *alloc.New<LabelOperand>(parent, idx);
}

CommentOperand &OperandBuilder::CreateComment(const std::string &s, MemPool *mp)
{
    return mp ? *mp->New<CommentOperand>(s, *mp) : *alloc.New<CommentOperand>(s, *mp);
}

CommentOperand &OperandBuilder::CreateComment(const MapleString &s, MemPool *mp)
{
    return mp ? *mp->New<CommentOperand>(s.c_str(), *mp) : *alloc.New<CommentOperand>(s.c_str(), *mp);
}

}  // namespace maplebe
