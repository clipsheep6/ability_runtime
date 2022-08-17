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

#include "ecmascript/compiler/stub_compiler.h"

#include "ecmascript/base/config.h"
#include "ecmascript/compiler/common_stubs.h"
#include "ecmascript/compiler/file_generators.h"
#include "ecmascript/compiler/interpreter_stub-inl.h"
#include "ecmascript/compiler/llvm_codegen.h"
#include "ecmascript/compiler/pass.h"
#include "ecmascript/compiler/scheduler.h"
#include "ecmascript/compiler/stub.h"
#include "ecmascript/compiler/stub_builder-inl.h"
#include "ecmascript/compiler/verifier.h"
#include "ecmascript/js_runtime_options.h"
#include "ecmascript/log.h"
#include "ecmascript/napi/include/jsnapi.h"

#include "libpandabase/utils/pandargs.h"
#include "libpandabase/utils/span.h"

namespace panda::ecmascript::kungfu {
class StubPassData : public PassData {
public:
    explicit StubPassData(Stub *stub, LLVMModule *module)
        : PassData(nullptr), module_(module), stub_(stub) {}
    ~StubPassData() = default;

    const CompilationConfig *GetCompilationConfig() const
    {
        return module_->GetCompilationConfig();
    }

    Circuit *GetCircuit() const
    {
        return stub_->GetEnvironment()->GetCircuit();
    }

    LLVMModule *GetStubModule() const
    {
        return module_;
    }

    Stub *GetStub() const
    {
        return stub_;
    }

private:
    LLVMModule *module_;
    Stub *stub_;
};

class StubBuildCircuitPass {
public:
    bool Run(StubPassData *data, [[maybe_unused]] bool enableLog)
    {
        auto stub = data->GetStub();
        LOG_COMPILER(INFO) << "Stub Name: " << stub->GetMethodName();
        stub->GenerateCircuit(data->GetCompilationConfig());
        return true;
    }
};

class StubLLVMIRGenPass {
public:
    void CreateCodeGen(LLVMModule *module, bool enableLog)
    {
        llvmImpl_ = std::make_unique<LLVMIRGeneratorImpl>(module, enableLog);
    }
    bool Run(StubPassData *data, bool enableLog, size_t index)
    {
        auto stubModule = data->GetStubModule();
        CreateCodeGen(stubModule, enableLog);
        CodeGenerator codegen(llvmImpl_);
        codegen.RunForStub(data->GetCircuit(), data->GetScheduleResult(), index, data->GetCompilationConfig());
        return true;
    }
private:
    std::unique_ptr<CodeGeneratorImpl> llvmImpl_ {nullptr};
};

void StubCompiler::RunPipeline(LLVMModule *module) const
{
    auto callSigns = module->GetCSigns();
    const CompilerLog *log = GetLog();
    auto logList = GetLogList();

    bool enableLog = !log->NoneMethod();
    for (size_t i = 0; i < callSigns.size(); i++) {
        Circuit circuit;
        Stub stub(callSigns[i], &circuit);
        ASSERT(callSigns[i]->HasConstructor());
        StubBuilder* stubBuilder = static_cast<StubBuilder*>(
            callSigns[i]->GetConstructor()(reinterpret_cast<void*>(stub.GetEnvironment())));
        stub.SetStubBuilder(stubBuilder);

        if (log->CertainMethod()) {
            enableLog = logList->IncludesMethod(stub.GetMethodName());
        }

        StubPassData data(&stub, module);
        PassRunner<StubPassData> pipeline(&data, enableLog && log->OutputCIR());
        pipeline.RunPass<StubBuildCircuitPass>();
        pipeline.RunPass<VerifierPass>();
        pipeline.RunPass<SchedulingPass>();
        pipeline.RunPass<StubLLVMIRGenPass>(i);
        delete stubBuilder;
    }
}

void StubCompiler::InitializeCS() const
{
    BytecodeStubCSigns::Initialize();
    CommonStubCSigns::Initialize();
    BuiltinsStubCSigns::Initialize();
    RuntimeStubCSigns::Initialize();
}

bool StubCompiler::BuildStubModuleAndSave() const
{
    InitializeCS();
    size_t res = 0;
    const CompilerLog *log = GetLog();
    const MethodLogList *logList = GetLogList();
    StubFileGenerator generator(log, logList, triple_);
    if (!filePath_.empty()) {
        LOG_COMPILER(INFO) << "compiling bytecode handler stubs";
        LLVMModule bcStubModule("bc_stub", triple_);
        LLVMAssembler bcStubAssembler(bcStubModule.GetModule(), LOptions(optLevel_, false, relocMode_));
        bcStubModule.SetUpForBytecodeHandlerStubs();
        RunPipeline(&bcStubModule);
        generator.AddModule(&bcStubModule, &bcStubAssembler);
        res++;
        LOG_COMPILER(INFO) << "compiling common stubs";
        LLVMModule comStubModule("com_stub", triple_);
        LLVMAssembler comStubAssembler(comStubModule.GetModule(), LOptions(optLevel_, true, relocMode_));
        comStubModule.SetUpForCommonStubs();
        RunPipeline(&comStubModule);
        generator.AddModule(&comStubModule, &comStubAssembler);
        res++;
        LOG_COMPILER(INFO) << "compiling builtins stubs";
        LLVMModule builtinsStubModule("builtins_stub", triple_);
        LLVMAssembler builtinsStubAssembler(builtinsStubModule.GetModule(), LOptions(optLevel_, true, relocMode_));
        builtinsStubModule.SetUpForBuiltinsStubs();
        RunPipeline(&builtinsStubModule);
        generator.AddModule(&builtinsStubModule, &builtinsStubAssembler);
        res++;
        generator.SaveStubFile(filePath_);
    }
    return (res > 0);
}
}  // namespace panda::ecmascript::kungfu

int main(const int argc, const char **argv)
{
    panda::Span<const char *> sp(argv, argc);
    panda::ecmascript::JSRuntimeOptions runtimeOptions;
    panda::PandArg<bool> help("help", false, "Print this message and exit");
    panda::PandArg<bool> options("options", false, "Print options");
    panda::PandArgParser paParser;

    runtimeOptions.AddOptions(&paParser);

    paParser.Add(&help);
    paParser.Add(&options);

    if (!paParser.Parse(argc, argv) || help.GetValue()) {
        std::cerr << paParser.GetErrorString() << std::endl;
        std::cerr << "Usage: " << "ark_stub_compiler" << " [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "optional arguments:" << std::endl;

        std::cerr << paParser.GetHelpString() << std::endl;
        return 1;
    }

    panda::ecmascript::Log::Initialize(runtimeOptions);

    std::string triple = runtimeOptions.GetTargetTriple();
    std::string stubFile = runtimeOptions.GetStubFile();
    size_t optLevel = runtimeOptions.GetOptLevel();
    size_t relocMode = runtimeOptions.GetRelocMode();
    std::string logOption = runtimeOptions.GetCompilerLogOption();
    std::string methodsList = runtimeOptions.GetMethodsListForLog();
    panda::ecmascript::kungfu::CompilerLog logOpt(logOption);
    panda::ecmascript::kungfu::MethodLogList logList(methodsList);
    panda::ecmascript::kungfu::StubCompiler compiler(triple, stubFile, optLevel, relocMode, &logOpt, &logList);

    bool res = compiler.BuildStubModuleAndSave();
    LOG_COMPILER(INFO) << "stub compiler run finish, result condition(T/F):" << std::boolalpha << res;
    return res ? 0 : -1;
}
