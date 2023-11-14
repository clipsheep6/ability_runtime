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

#ifndef MAPLEBE_INCLUDE_CG_X86_64_EMITTER_H
#define MAPLEBE_INCLUDE_CG_X86_64_EMITTER_H

#include "emit.h"
#include "assembler/asm_assembler.h"
#include "assembler/elf_assembler.h"

namespace maplebe {
class X64Emitter : public Emitter {
public:
    X64Emitter(CG &cg, assembler::Assembler &newAssembler) : Emitter(cg, ""), assmbler(newAssembler) {}
    ~X64Emitter() = default;

    assembler::Assembler &GetAssembler() const
    {
        return assmbler;
    }

    assembler::Reg TransferReg(Operand *opnd) const;
    std::pair<int64, bool> TransferImm(Operand *opnd);
    assembler::Mem TransferMem(Operand *opnd, uint32 funcUniqueId);
    int64 TransferLabel(Operand *opnd, uint32 funcUniqueId);
    uint32 TransferFuncName(Operand *opnd);

    void EmitFunctionHeader(maplebe::CGFunc &cgFunc);
    void EmitBBHeaderLabel(maplebe::CGFunc &cgFunc, LabelIdx labIdx, uint32 freq);
    void EmitInsn(Insn &insn, uint32 funcUniqueId);
    void EmitJmpTable(const maplebe::CGFunc &cgFunc);
    void EmitFunctionFoot(maplebe::CGFunc &cgFunc);
    uint8 GetSymbolAlign(const maple::MIRSymbol &mirSymbol, bool isComm = false);
    uint64 GetSymbolSize(maple::TyIdx typeIndex);
    void EmitLocalVariable(maplebe::CGFunc &cgFunc);
    void EmitGlobalVariable(maplebe::CG &cg);
    uint64 EmitStructure(maple::MIRConst &mirConst, maplebe::CG &cg, bool belongsToDataSec = true);
    uint64 EmitStructure(maple::MIRConst &mirConst, maplebe::CG &cg, uint32 &subStructFieldCounts,
                         bool belongsToDataSec = true);
    uint64 EmitVector(maple::MIRConst &mirConst, bool belongsToDataSec = true);
    uint64 EmitArray(maple::MIRConst &mirConst, maplebe::CG &cg, bool belongsToDataSec = true);
    void EmitAddrofElement(MIRConst &mirConst, bool belongsToDataSec);
    uint32 EmitSingleElement(maple::MIRConst &mirConst, bool belongsToDataSec = true, bool isIndirect = false);
    void EmitBitField(maplebe::StructEmitInfo &structEmitInfo, maple::MIRConst &mirConst,
                      const maple::MIRType *nextType, uint64 fieldOffset, bool belongsToDataSec = true);
    void EmitCombineBfldValue(maplebe::StructEmitInfo &structEmitInfo, bool belongsToDataSec = true);
    void EmitStringPointers();
    void Run(maplebe::CGFunc &cgFunc);

    /* Dwarf debug info */
    void EmitDIHeaderFileInfo();
    void UpdateAttrAndEmit(const std::string &sfile, DebugInfo &mirdi, DBGAbbrevEntry &diae, DBGDie &die,
                           const std::string &spath);
    void EmitDIDebugInfoSection(maplebe::DebugInfo &mirdi);
    void EmitDwFormAddr(const DBGDie &die, const DBGDieAttr &attr, DwAt attrName, DwTag tagName, DebugInfo &di);
    void EmitDwFormRef4(DBGDie &die, const DBGDieAttr &attr, DwAt attrName, DwTag tagName, DebugInfo &di);
    void EmitDwFormData8(const DBGDieAttr &attr, DwAt attrName, DwTag tagName, DebugInfo &di,
                         MapleVector<DBGDieAttr *> &attrvec);
    void EmitDIAttrValue(DBGDie &die, DBGDieAttr &attr, DwAt attrName, DwTag tagName, DebugInfo &di);
    void EmitDIDebugAbbrevSection(maplebe::DebugInfo &mirdi);
    void EmitDIDebugStrSection();
    void EmitDebugInfo(maplebe::CG &cg);

private:
    assembler::Assembler &assmbler;
    std::vector<uint32> stringPtr;
};
} /* namespace maplebe */

#endif /* MAPLEBE_INCLUDE_CG_X86_64_EMITTER_H */
