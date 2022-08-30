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

#include "ecmascript/compiler/assembler_module.h"

#include "ecmascript/compiler/assembler/aarch64/assembler_aarch64.h"
#include "ecmascript/compiler/assembler/x64/assembler_x64.h"
#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/compiler/circuit_builder.h"
#include "ecmascript/compiler/trampoline/aarch64/common_call.h"
#include "ecmascript/compiler/trampoline/x64/common_call.h"
#include "ecmascript/compiler/rt_call_signature.h"

// TODO
#ifdef NEW_INSTRUCTION_DEFINE
#include "libpandafile/bytecode_instruction-inl.h"
#else
#include "ecmascript/jspandafile/bytecode_inst/new_instruction.h"
#endif

namespace panda::ecmascript::kungfu {
void AssemblerModule::Run(const CompilationConfig *cfg, Chunk* chunk)
{
    SetUpForAsmStubs();
    if (cfg->IsAmd64()) {
        GenerateStubsX64(chunk);
    } else if (cfg->IsAArch64()) {
        GenerateStubsAarch64(chunk);
    } else {
        UNREACHABLE();
    }
}

void AssemblerModule::GenerateStubsX64(Chunk* chunk)
{
    x64::ExtendedAssembler assembler(chunk, this);
    LOG_COMPILER(INFO) << "compiling asm stubs";
    for (size_t i = 0; i < asmCallSigns_.size(); i++) {
        auto cs = asmCallSigns_[i];
        ASSERT(cs->HasConstructor());
        LOG_COMPILER(INFO) << "Stub Name: " << cs->GetName();
        AssemblerStub *stub = static_cast<AssemblerStub*>(
            cs->GetConstructor()(nullptr));
        stub->GenerateX64(&assembler);
        delete stub;
    }
    buffer_ = assembler.GetBegin();
    bufferSize_ = assembler.GetCurrentPosition();
}

void AssemblerModule::GenerateStubsAarch64(Chunk* chunk)
{
    aarch64::ExtendedAssembler assembler(chunk, this);
    LOG_COMPILER(INFO) << "compiling asm stubs";
    for (size_t i = 0; i < asmCallSigns_.size(); i++) {
        auto cs = asmCallSigns_[i];
        ASSERT(cs->HasConstructor());
        LOG_COMPILER(INFO) << "Stub Name: " << cs->GetName();
        AssemblerStub *stub = static_cast<AssemblerStub*>(
            cs->GetConstructor()(nullptr));
        stub->GenerateAarch64(&assembler);
        delete stub;
    }
    buffer_ = assembler.GetBegin();
    bufferSize_ = assembler.GetCurrentPosition();
}

void AssemblerModule::SetUpForAsmStubs()
{
    RuntimeStubCSigns::GetASMCSigns(asmCallSigns_);
    for (auto cs : asmCallSigns_) {
        symbolTable_[cs->GetID()] = new panda::ecmascript::Label();
    }
}

int AssemblerModule::GetArgcFromJSCallMode(JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_ARG0:
        case JSCallMode::CALL_THIS_ARG0:
        case JSCallMode::DEPRECATED_CALL_ARG0:
            return 0;
        case JSCallMode::CALL_ARG1:
        case JSCallMode::CALL_THIS_ARG1:
        case JSCallMode::DEPRECATED_CALL_ARG1:
            return 1;
        case JSCallMode::CALL_ARG2:
        case JSCallMode::CALL_THIS_ARG2:
        case JSCallMode::DEPRECATED_CALL_ARG2:
            return 2; // 2: arg2
        case JSCallMode::CALL_ARG3:
        case JSCallMode::CALL_THIS_ARG3:
        case JSCallMode::DEPRECATED_CALL_ARG3:
            return 3; // 3: arg3
        case JSCallMode::CALL_THIS_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_ENTRY:
        case JSCallMode::CALL_FROM_AOT:
            return -1;
        case JSCallMode::CALL_GETTER:
            return 0;
        case JSCallMode::CALL_SETTER:
            return 1;
        default:
            UNREACHABLE();
    }
}

bool AssemblerModule::IsCallNew(JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_ARG0:
        case JSCallMode::CALL_THIS_ARG0:
        case JSCallMode::DEPRECATED_CALL_ARG0:
        case JSCallMode::CALL_ARG1:
        case JSCallMode::CALL_THIS_ARG1:
        case JSCallMode::DEPRECATED_CALL_ARG1:
        case JSCallMode::CALL_ARG2:
        case JSCallMode::CALL_THIS_ARG2:
        case JSCallMode::DEPRECATED_CALL_ARG2:
        case JSCallMode::CALL_ARG3:
        case JSCallMode::CALL_THIS_ARG3:
        case JSCallMode::DEPRECATED_CALL_ARG3:
        case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_WITH_ARGV:
        case JSCallMode::CALL_GETTER:
        case JSCallMode::CALL_SETTER:
        case JSCallMode::CALL_ENTRY:
        case JSCallMode::CALL_FROM_AOT:
            return false;
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
            return true;
        default:
            UNREACHABLE();
    }
    return false;
}

bool AssemblerModule::JSModeHaveThisArg(JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_ARG0:
        case JSCallMode::CALL_ARG1:
        case JSCallMode::CALL_ARG2:
        case JSCallMode::CALL_ARG3:
        case JSCallMode::DEPRECATED_CALL_ARG0:
        case JSCallMode::DEPRECATED_CALL_ARG1:
        case JSCallMode::DEPRECATED_CALL_ARG2:
        case JSCallMode::DEPRECATED_CALL_ARG3:
        case JSCallMode::CALL_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
            return false;
        case JSCallMode::CALL_THIS_ARG0:
        case JSCallMode::CALL_THIS_ARG1:
        case JSCallMode::CALL_THIS_ARG2:
        case JSCallMode::CALL_THIS_ARG3:
        case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_ENTRY:
        case JSCallMode::CALL_FROM_AOT:
        case JSCallMode::CALL_GETTER:
        case JSCallMode::CALL_SETTER:
            return true;
        default:
            UNREACHABLE();
    }
}

bool AssemblerModule::JSModeHaveNewTargetArg(JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_ARG0:
        case JSCallMode::CALL_ARG1:
        case JSCallMode::CALL_ARG2:
        case JSCallMode::CALL_ARG3:
        case JSCallMode::DEPRECATED_CALL_ARG0:
        case JSCallMode::DEPRECATED_CALL_ARG1:
        case JSCallMode::DEPRECATED_CALL_ARG2:
        case JSCallMode::DEPRECATED_CALL_ARG3:
        case JSCallMode::CALL_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_GETTER:
        case JSCallMode::CALL_SETTER:
        case JSCallMode::CALL_THIS_ARG0:
        case JSCallMode::CALL_THIS_ARG1:
        case JSCallMode::CALL_THIS_ARG2:
        case JSCallMode::CALL_THIS_ARG3:
            return false;
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_ENTRY:
        case JSCallMode::CALL_FROM_AOT:
            return true;
        default:
            UNREACHABLE();
    }
}

bool AssemblerModule::IsJumpToCallCommonEntry(JSCallMode mode)
{
    switch (mode) {
        case JSCallMode::CALL_ARG0:
        case JSCallMode::CALL_ARG1:
        case JSCallMode::CALL_ARG2:
        case JSCallMode::CALL_ARG3:
        case JSCallMode::DEPRECATED_CALL_ARG0:
        case JSCallMode::DEPRECATED_CALL_ARG1:
        case JSCallMode::DEPRECATED_CALL_ARG2:
        case JSCallMode::DEPRECATED_CALL_ARG3:
        case JSCallMode::CALL_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_WITH_ARGV:
        case JSCallMode::CALL_THIS_WITH_ARGV:
        case JSCallMode::DEPRECATED_CALL_THIS_WITH_ARGV:
        case JSCallMode::CALL_CONSTRUCTOR_WITH_ARGV:
        case JSCallMode::CALL_THIS_ARG0:
        case JSCallMode::CALL_THIS_ARG1:
        case JSCallMode::CALL_THIS_ARG2:
        case JSCallMode::CALL_THIS_ARG3:
            return true;
        case JSCallMode::CALL_GETTER:
        case JSCallMode::CALL_SETTER:
        case JSCallMode::CALL_ENTRY:
        case JSCallMode::CALL_GENERATOR:
        case JSCallMode::CALL_FROM_AOT:
            return false;
        default:
            UNREACHABLE();
    }
    return false;
}

#define DECLARE_ASM_STUB_X64_GENERATE(name)                                                       \
void name##Stub::GenerateX64(Assembler *assembler)                                                \
{                                                                                                 \
    x64::ExtendedAssembler *assemblerX64 = static_cast<x64::ExtendedAssembler*>(assembler);       \
    x64::AssemblerStubsX64::name(assemblerX64);                                                   \
    assemblerX64->Align16();                                                                      \
}

#define DECLARE_JSCALL_TRAMPOLINE_X64_GENERATE(name)                                              \
void name##Stub::GenerateX64(Assembler *assembler)                                                \
{                                                                                                 \
    x64::ExtendedAssembler *assemblerX64 = static_cast<x64::ExtendedAssembler*>(assembler);       \
    x64::OptimizedCall::name(assemblerX64);                                                       \
    assemblerX64->Align16();                                                                      \
}

#define DECLARE_ASM_INTERPRETER_TRAMPOLINE_X64_GENERATE(name)                                     \
void name##Stub::GenerateX64(Assembler *assembler)                                                \
{                                                                                                 \
    x64::ExtendedAssembler *assemblerX64 = static_cast<x64::ExtendedAssembler*>(assembler);       \
    x64::AsmInterpreterCall::name(assemblerX64);                                                  \
    assemblerX64->Align16();                                                                      \
}


#define DECLARE_JSCALL_TRAMPOLINE_AARCH64_GENERATE(name)                                                \
void name##Stub::GenerateAarch64(Assembler *assembler)                                                  \
{                                                                                                       \
    aarch64::ExtendedAssembler *assemblerAarch64 = static_cast<aarch64::ExtendedAssembler*>(assembler); \
    aarch64::OptimizedCall::name(assemblerAarch64);                                                     \
}

#define DECLARE_ASM_INTERPRETER_TRAMPOLINE_AARCH64_GENERATE(name)                                       \
void name##Stub::GenerateAarch64(Assembler *assembler)                                                  \
{                                                                                                       \
    aarch64::ExtendedAssembler *assemblerAarch64 = static_cast<aarch64::ExtendedAssembler*>(assembler); \
    aarch64::AsmInterpreterCall::name(assemblerAarch64);                                                \
}

JS_CALL_TRAMPOLINE_LIST(DECLARE_JSCALL_TRAMPOLINE_X64_GENERATE)
ASM_INTERPRETER_TRAMPOLINE_LIST(DECLARE_ASM_INTERPRETER_TRAMPOLINE_X64_GENERATE)
JS_CALL_TRAMPOLINE_LIST(DECLARE_JSCALL_TRAMPOLINE_AARCH64_GENERATE)
ASM_INTERPRETER_TRAMPOLINE_LIST(DECLARE_ASM_INTERPRETER_TRAMPOLINE_AARCH64_GENERATE)
#undef DECLARE_JSCALL_TRAMPOLINE_AARCH64_GENERATE
#undef DECLARE_ASM_INTERPRETER_TRAMPOLINE_AARCH64_GENERATE
}  // namespace panda::ecmascript::kunfu
