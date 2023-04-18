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
#include "ecmascript/compiler/pass_manager.h"
#include "ecmascript/compiler/bytecodes.h"
#include "ecmascript/compiler/pass.h"
#include "ecmascript/compiler/compilation_driver.h"
#include "ecmascript/ecma_handle_scope.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"
#include "ecmascript/jspandafile/panda_file_translator.h"
#include "ecmascript/snapshot/mem/snapshot.h"
#include "ecmascript/ts_types/ts_manager.h"

namespace panda::ecmascript::kungfu {
bool PassManager::ShouldCollect() const
{
    return passOptions_->EnableTypeInfer() && (profilerLoader_.IsLoaded() || vm_->GetTSManager()->AssertTypes());
}

bool PassManager::Compile(const std::string &fileName, AOTFileGenerator &generator)
{
    [[maybe_unused]] EcmaHandleScope handleScope(vm_->GetJSThread());

    JSPandaFile *jsPandaFile = CreateAndVerifyJSPandaFile(fileName.c_str());
    if (jsPandaFile == nullptr) {
        LOG_COMPILER(ERROR) << "Cannot execute panda file '" << fileName << "'";
        return false;
    }

    if (!profilerLoader_.LoadAndVerify(jsPandaFile->GetChecksum())) {
        LOG_COMPILER(ERROR) << "Load and verify profiler failure";
        return false;
    }

    if (!IsReleasedPandaFile(jsPandaFile)) {
        LOG_COMPILER(ERROR) << "The input panda file [" << fileName
                            << "] of AOT Compiler is debuggable version, do not use for performance test!";
    }

    ResolveModule(jsPandaFile, fileName);
    Module *m = generator.AddModule(fileName, triple_, LOptions(optLevel_, true, relocMode_));

    BytecodeInfoCollector collector(vm_, jsPandaFile, maxAotMethodSize_, ShouldCollect());
    PassContext ctx(triple_, log_, &collector, m->GetModule());
    CompilationDriver cmpDriver(profilerLoader_, &collector);

    cmpDriver.Run([this, &fileName, &ctx]
        (const CString recordName, const std::string &methodName, MethodLiteral *methodLiteral,
         uint32_t methodOffset, const MethodPcInfo &methodPCInfo, MethodInfo &methodInfo) {
        auto jsPandaFile = ctx.GetJSPandaFile();
        auto cmpCfg = ctx.GetCompilerConfig();
        auto tsManager = ctx.GetTSManager();
        // note: TSManager need to set current constantpool before all pass
        tsManager->SetCurConstantPool(jsPandaFile, methodOffset);

        log_->SetMethodLog(fileName, methodName, logList_);

        std::string fullName = methodName + "@" + fileName;
        bool enableMethodLog = log_->GetEnableMethodLog();
        if (enableMethodLog) {
            LOG_COMPILER(INFO) << "\033[34m" << "aot method [" << fullName << "] log:" << "\033[0m";
        }

        bool hasTypes = jsPandaFile->HasTSTypes(recordName);
        if (UNLIKELY(!hasTypes)) {
            LOG_COMPILER(INFO) << "record: " << recordName << " has no types";
        }

        Circuit circuit(vm_->GetNativeAreaAllocator(), ctx.GetAOTModule()->GetDebugInfo(), cmpCfg->Is64Bit());
        circuit.SetFrameType(FrameType::OPTIMIZED_JS_FUNCTION_FRAME);
        PGOProfilerLoader *loader = passOptions_->EnableOptPGOType() ? &profilerLoader_ : nullptr;

        BytecodeCircuitBuilder builder(jsPandaFile, methodLiteral, methodPCInfo, tsManager, &circuit,
                                       ctx.GetByteCodes(), hasTypes, enableMethodLog && log_->OutputCIR(),
                                       passOptions_->EnableTypeLowering(), fullName, recordName, loader);
        {
            TimeScope timeScope("BytecodeToCircuit", methodName, methodOffset, log_);
            builder.BytecodeToCircuit();
        }

        PassData data(&builder, &circuit, &ctx, log_, fullName, &methodInfo, hasTypes, recordName,
                      methodLiteral, methodOffset, vm_->GetNativeAreaAllocator());
        PassRunner<PassData> pipeline(&data);
        if (passOptions_->EnableTypeInfer()) {
            pipeline.RunPass<TypeInferPass>();
        }
        if (data.IsTypeAbort()) {
            return;
        }
        if (passOptions_->EnableOptInlining()) {
            pipeline.RunPass<TSInlineLoweringPass>();
        }
        pipeline.RunPass<AsyncFunctionLoweringPass>();
        if (passOptions_->EnableTypeLowering()) {
            pipeline.RunPass<TSTypeLoweringPass>();
            pipeline.RunPass<EarlyEliminationPass>();
            pipeline.RunPass<NumberSpeculativePass>();
            pipeline.RunPass<LaterEliminationPass>();
            pipeline.RunPass<ValueNumberingPass>();
            pipeline.RunPass<TypeLoweringPass>();
            pipeline.RunPass<EarlyEliminationPass>();
            pipeline.RunPass<LaterEliminationPass>();
            pipeline.RunPass<GenericTypeLoweringPass>();
        }
        pipeline.RunPass<SlowPathLoweringPass>();
        pipeline.RunPass<VerifierPass>();
        pipeline.RunPass<SchedulingPass>();
        pipeline.RunPass<LLVMIRGenPass>();
    });
    ProcessConstantPool(&collector);
    return true;
}

void PassManager::ProcessConstantPool(BytecodeInfoCollector *collector)
{
    LOG_COMPILER(INFO) << collector->GetBytecodeInfo().GetSkippedMethodSize()
                       << " methods have been skipped";
    vm_->GetTSManager()->ProcessSnapshotConstantPool(collector);
}

JSPandaFile *PassManager::CreateAndVerifyJSPandaFile(const CString &fileName)
{
    JSPandaFileManager *jsPandaFileManager = JSPandaFileManager::GetInstance();
    std::shared_ptr<JSPandaFile> jsPandaFile = jsPandaFileManager->OpenJSPandaFile(fileName);
    if (jsPandaFile == nullptr) {
        LOG_ECMA(ERROR) << "open file " << fileName << " error";
        return nullptr;
    }

    if (!jsPandaFile->IsNewVersion()) {
        LOG_COMPILER(ERROR) << "AOT only support panda file with new ISA, while the '" <<
            fileName << "' file is the old version";
        return nullptr;
    }

    JSPandaFileManager::GetInstance()->AddJSPandaFileVm(vm_, jsPandaFile);
    return jsPandaFile.get();
}

bool PassManager::IsReleasedPandaFile(const JSPandaFile *jsPandaFile) const
{
    MethodLiteral* methodLiteral = jsPandaFile->GetMethodLiterals();
    if (methodLiteral == nullptr) {
        LOG_COMPILER(ERROR) << "There is no mehtod literal in " << jsPandaFile->GetJSPandaFileDesc();
        return false;
    }

    panda_file::File::EntityId methodId = methodLiteral->GetMethodId();
    DebugInfoExtractor *debugInfoExtractor = JSPandaFileManager::GetInstance()->GetJSPtExtractor(jsPandaFile);
    LocalVariableTable lvt = debugInfoExtractor->GetLocalVariableTable(methodId);
    return lvt.empty();
}

void PassManager::ResolveModule(const JSPandaFile *jsPandaFile, const std::string &fileName)
{
    const auto &recordInfo = jsPandaFile->GetJSRecordInfo();
    ModuleManager *moduleManager = vm_->GetModuleManager();
    JSThread *thread = vm_->GetJSThread();
    for (auto info: recordInfo) {
        auto recordName = info.first;
        if (jsPandaFile->IsModule(thread, recordName)) {
            ASSERT(!thread->HasPendingException());
            moduleManager->HostResolveImportedModuleWithMerge(fileName.c_str(), recordName);
        }
    }
}
} // namespace panda::ecmascript::kungfu
