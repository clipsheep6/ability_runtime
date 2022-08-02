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

#ifndef ECMASCRIPT_COMPILER_FILE_GENERATORS_H
#define ECMASCRIPT_COMPILER_FILE_GENERATORS_H

#include <tuple>

#include "ecmascript/compiler/assembler_module.h"
#include "ecmascript/compiler/compiler_log.h"
#include "ecmascript/compiler/llvm_codegen.h"
#include "ecmascript/compiler/llvm_ir_builder.h"
#include "ecmascript/file_loader.h"
#include "ecmascript/jspandafile/js_pandafile.h"

namespace panda::ecmascript::kungfu {
class Module {
public:
    Module() = default;
    Module(LLVMModule *module, LLVMAssembler *assembler)
        : llvmModule_(module), assembler_(assembler)
    {
    }

    void CollectFuncEntryInfo(std::map<uintptr_t, std::string> &addr2name, StubModulePackInfo &stubInfo,
        uint32_t moduleIndex, const CompilerLog &log)
    {
        auto engine = assembler_->GetEngine();
        auto callSigns = llvmModule_->GetCSigns();
        std::map<uintptr_t, int> addr2FpToPrevFrameSpDelta;
        std::vector<uint64_t> funSizeVec;
        std::vector<uintptr_t> entrys;
        for (size_t j = 0; j < llvmModule_->GetFuncCount(); j++) {
            LLVMValueRef func = llvmModule_->GetFunction(j);
            ASSERT(func != nullptr);
            uintptr_t entry = reinterpret_cast<uintptr_t>(LLVMGetPointerToGlobal(engine, func));
            entrys.push_back(entry);
        }
        auto codeBuff = assembler_->GetSectionAddr(ElfSecName::TEXT);
        const size_t funcCount = llvmModule_->GetFuncCount();
        for (size_t j = 0; j < funcCount; j++) {
            auto cs = callSigns[j];
            LLVMValueRef func = llvmModule_->GetFunction(j);
            ASSERT(func != nullptr);
            int delta = assembler_->GetFpDeltaPrevFramSp(func, log);
            ASSERT(delta >= 0 && (delta % sizeof(uintptr_t) == 0));
            uint32_t funcSize = 0;
            if (j < funcCount - 1) {
                funcSize = entrys[j + 1] - entrys[j];
            } else {
                funcSize = codeBuff + assembler_->GetSectionSize(ElfSecName::TEXT) - entrys[j];
            }
            stubInfo.AddStubEntry(cs->GetTargetKind(), cs->GetID(), entrys[j] - codeBuff, moduleIndex, delta, funcSize);
            ASSERT(!cs->GetName().empty());
            addr2name[entrys[j]] = cs->GetName();
        }
    }

    void CollectFuncEntryInfo(std::map<uintptr_t, std::string> &addr2name, AOTModulePackInfo &aotInfo,
        uint32_t moduleIndex, const CompilerLog &log)
    {
        auto engine = assembler_->GetEngine();
        std::vector<std::tuple<uint64_t, size_t, int>> funcInfo; // entry、idx、delta
        llvmModule_->IteratefuncIndexMap([&](size_t idx, LLVMValueRef func) {
            uint64_t funcEntry = reinterpret_cast<uintptr_t>(LLVMGetPointerToGlobal(engine, func));
            uint64_t length = 0;
            std::string funcName(LLVMGetValueName2(func, &length));
            ASSERT(length != 0);
            LOG_COMPILER(INFO) << "CollectCodeInfo for AOT func: " << funcName.c_str();
            addr2name[funcEntry] = funcName;
            int delta = assembler_->GetFpDeltaPrevFramSp(func, log);
            ASSERT(delta >= 0 && (delta % sizeof(uintptr_t) == 0));
            funcInfo.emplace_back(std::tuple(funcEntry, idx, delta));
        });
        auto codeBuff = assembler_->GetSectionAddr(ElfSecName::TEXT);
        const size_t funcCount = funcInfo.size();
        for (size_t i = 0; i < funcInfo.size(); i++) {
            uint64_t funcEntry;
            size_t idx;
            int delta;
            uint32_t funcSize;
            std::tie(funcEntry, idx, delta) = funcInfo[i];
            if (i < funcCount - 1) {
                funcSize = std::get<0>(funcInfo[i + 1]) - funcEntry;
            } else {
                funcSize = codeBuff + assembler_->GetSectionSize(ElfSecName::TEXT) - funcEntry;
            }
            aotInfo.AddStubEntry(CallSignature::TargetKind::JSFUNCTION, idx,
                funcEntry - codeBuff, moduleIndex, delta, funcSize);
        }
    }

    void CollectModuleSectionDes(ModuleSectionDes &moduleDes) const
    {
        ASSERT(assembler_ != nullptr);
        assembler_->IterateSecInfos([&](size_t i, std::pair<uint8_t *, size_t> secInfo) {
            auto curSec = ElfSection(i);
            moduleDes.SetSecAddr(reinterpret_cast<uint64_t>(secInfo.first), curSec.GetElfEnumValue());
            moduleDes.SetSecSize(secInfo.second, curSec.GetElfEnumValue());
        });
    }

    const CompilationConfig *GetCompilationConfig()
    {
        return llvmModule_->GetCompilationConfig();
    }

    uint32_t GetSectionSize(ElfSecName sec) const
    {
        return assembler_->GetSectionSize(sec);
    }

    uintptr_t GetSectionAddr(ElfSecName sec) const
    {
        return assembler_->GetSectionAddr(sec);
    }

    void RunAssembler(const CompilerLog &log)
    {
        assembler_->Run(log);
    }

    void DisassemblerFunc(std::map<uintptr_t, std::string> &addr2name,
        const CompilerLog &log, const MethodLogList &logList)
    {
        assembler_->Disassemble(addr2name, log, logList);
    }

    void DestoryModule()
    {
        if (llvmModule_ != nullptr) {
            delete llvmModule_;
            llvmModule_ = nullptr;
        }
        if (assembler_ != nullptr) {
            delete assembler_;
            assembler_ = nullptr;
        }
    }
private:
    LLVMModule *llvmModule_ {nullptr};
    LLVMAssembler *assembler_ {nullptr};
};

class FileGenerator {
public:
    FileGenerator(const CompilerLog *log, const MethodLogList *logList) : log_(log), logList_(logList) {};
    virtual ~FileGenerator() = default;

    const CompilerLog GetLog() const
    {
        return *log_;
    }
protected:
    std::vector<Module> modulePackage_ {};
    const CompilerLog *log_ {nullptr};
    const MethodLogList *logList_ {nullptr};

    void RunLLVMAssembler()
    {
        for (auto m : modulePackage_) {
            m.RunAssembler(*(log_));
        }
    }

    void DisassembleEachFunc(std::map<uintptr_t, std::string> &addr2name)
    {
        for (auto m : modulePackage_) {
            m.DisassemblerFunc(addr2name, *(log_), *(logList_));
        }
    }

    void DestoryModule()
    {
        for (auto m : modulePackage_) {
            m.DestoryModule();
        }
    }
};

class AOTFileGenerator : public FileGenerator {
public:
    AOTFileGenerator(const CompilerLog *log, const MethodLogList *logList,
        EcmaVM* vm) : FileGenerator(log, logList), vm_(vm) {};
    ~AOTFileGenerator() override = default;

    void AddModule(LLVMModule *llvmModule, LLVMAssembler *assembler, const JSPandaFile *jsPandaFile)
    {
        modulePackage_.emplace_back(Module(llvmModule, assembler));
        auto hash = jsPandaFile->GetFileUniqId();
        aotfileHashs_.emplace_back(hash);
    }

    // save function for aot files containing normal func translated from JS/TS
    void SaveAOTFile(const std::string &filename);
    void SaveSnapshotFile();
private:
    AOTModulePackInfo aotInfo_;
    std::vector<uint32_t> aotfileHashs_ {};
    EcmaVM* vm_;

    // collect aot component info
    void CollectCodeInfo();
};

class StubFileGenerator : public FileGenerator {
public:
    StubFileGenerator(const CompilerLog *log, const MethodLogList *logList,
        const std::string &triple) : FileGenerator(log, logList), cfg_(triple) {};
    ~StubFileGenerator() override = default;
    void AddModule(LLVMModule *llvmModule, LLVMAssembler *assembler)
    {
        modulePackage_.emplace_back(Module(llvmModule, assembler));
    }
    // save function funcs for aot files containing stubs
    void SaveStubFile(const std::string &filename);
private:
    StubModulePackInfo stubInfo_;
    AssemblerModule asmModule_;
    CompilationConfig cfg_;

    void RunAsmAssembler();
    void CollectAsmStubCodeInfo(std::map<uintptr_t, std::string> &addr2name, uint32_t bridgeModuleIdx);
    void CollectCodeInfo();
};
}  // namespace panda::ecmascript::kungfu
#endif // ECMASCRIPT_COMPILER_FILE_GENERATORS_H
