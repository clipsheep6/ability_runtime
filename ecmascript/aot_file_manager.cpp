/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ecmascript/aot_file_manager.h"

#include "ecmascript/base/config.h"
#include "ecmascript/compiler/bc_call_signature.h"
#include "ecmascript/compiler/common_stubs.h"
#include "ecmascript/compiler/compiler_log.h"
#include "ecmascript/deoptimizer/deoptimizer.h"
#include "ecmascript/deoptimizer/relocator.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/message_string.h"
#include "ecmascript/jspandafile/constpool_value.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/js_file_path.h"
#include "ecmascript/js_runtime_options.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/snapshot/mem/snapshot.h"
#include "ecmascript/stackmap/ark_stackmap_parser.h"
#include "ecmascript/stackmap/llvm_stackmap_parser.h"
#include "ecmascript/mem/region.h"
#include "ecmascript/platform/file.h"
#include "ecmascript/platform/map.h"


extern const uint8_t _binary_stub_an_start[];
extern const uint32_t _binary_stub_an_length;

namespace panda::ecmascript {
using CommonStubCSigns = kungfu::CommonStubCSigns;
using BytecodeStubCSigns = kungfu::BytecodeStubCSigns;

void ModuleSectionDes::SaveSectionsInfo(std::ofstream &file)
{
    uint32_t secInfoSize = GetSecInfosSize();
    uint32_t secSize = 0;
    file.write(reinterpret_cast<char *>(&secInfoSize), sizeof(secInfoSize));
    std::multimap<std::string, double> SecMap;
    for (auto &s : sectionsInfo_) {
        uint8_t secName = static_cast<uint8_t>(s.first);
        uint32_t curSecSize = GetSecSize(s.first);
        uint64_t curSecAddr = GetSecAddr(s.first);
        secSize += curSecSize;
        SecMap.insert(make_pair(GetSecName(s.first), static_cast<double>(curSecSize)));
        file.write(reinterpret_cast<char *>(&secName), sizeof(secName));
        file.write(reinterpret_cast<char *>(&curSecSize), sizeof(curSecSize));
        file.write(reinterpret_cast<char *>(curSecAddr), curSecSize);
    }
    std::shared_ptr<uint8_t> ptr = GetArkStackMapSharePtr();
    uint32_t size = GetArkStackMapSize();
    file.write(reinterpret_cast<char *>(&size), sizeof(size));
    file.write(reinterpret_cast<char *>(ptr.get()), size);

    uint32_t index = GetStartIndex();
    uint32_t cnt = GetFuncCount();
    file.write(reinterpret_cast<char *>(&index), sizeof(index));
    file.write(reinterpret_cast<char *>(&cnt), sizeof(cnt));
    for (auto [key, val] : SecMap) {
        LOG_COMPILER(DEBUG) << key << " size is "
                            << std::fixed << std::setprecision(DECIMAL_LENS)
                            << (val / 1_KB) << "KB "<< "percentage:"
                            << std::fixed << std::setprecision(PERCENT_LENS)
                            << (val / secSize * HUNDRED_TIME) << "% ";
    }
    LOG_COMPILER(DEBUG) << "elf secitions size = " << (secSize / 1_KB) << "KB"
                        << ", ark stack map size = " << (size / 1_KB) << "KB";
}

void ModuleSectionDes::LoadStackMapSection(BinaryBufferParser &parser, uintptr_t secBegin, uint32_t &curUnitOffset)
{
    uint32_t size = 0;
    parser.ParseBuffer(&size, sizeof(size));
    parser.ParseBuffer(reinterpret_cast<void *>(secBegin), size);
    SetArkStackMapSize(size);
    SetArkStackMapPtr(reinterpret_cast<uint8_t *>(secBegin));
    curUnitOffset += size;
    uint32_t index = 0;
    uint32_t cnt = 0;
    parser.ParseBuffer(&index, sizeof(index));
    parser.ParseBuffer(&cnt, sizeof(cnt));
    SetStartIndex(index);
    SetFuncCount(cnt);
}

void ModuleSectionDes::LoadSectionsInfo(BinaryBufferParser &parser,
    uint32_t &curUnitOffset, uint64_t codeAddress)
{
    uint32_t secInfoSize = 0;
    parser.ParseBuffer(&secInfoSize, sizeof(secInfoSize));
    auto secBegin = codeAddress + static_cast<uintptr_t>(curUnitOffset);
    for (uint8_t i = 0; i < secInfoSize; i++) {
        uint8_t secName = 0;
        parser.ParseBuffer(&secName, sizeof(secName));
        auto secEnumName = static_cast<ElfSecName>(secName);
        uint32_t secSize = 0;
        parser.ParseBuffer(&secSize, sizeof(secSize));
        SetSecSize(secSize, secEnumName);
        parser.ParseBuffer(reinterpret_cast<void *>(secBegin), secSize);
        curUnitOffset += secSize;
        SetSecAddr(secBegin, secEnumName);
        secBegin += secSize;
    }
    LoadStackMapSection(parser, secBegin, curUnitOffset);
}

void ModuleSectionDes::LoadStackMapSection(std::ifstream &file, uintptr_t secBegin, uint32_t &curUnitOffset)
{
    uint32_t size = 0;
    file.read(reinterpret_cast<char *>(&size), sizeof(size));
    file.read(reinterpret_cast<char *>(secBegin), size);
    SetArkStackMapSize(size);
    SetArkStackMapPtr(reinterpret_cast<uint8_t *>(secBegin));
    curUnitOffset += size;
    uint32_t index = 0;
    uint32_t cnt = 0;
    file.read(reinterpret_cast<char *>(&index), sizeof(index));
    file.read(reinterpret_cast<char *>(&cnt), sizeof(cnt));
    SetStartIndex(index);
    SetFuncCount(cnt);
}

void ModuleSectionDes::LoadSectionsInfo(std::ifstream &file,
    uint32_t &curUnitOffset, uint64_t codeAddress)
{
    uint32_t secInfoSize;
    file.read(reinterpret_cast<char *>(&secInfoSize), sizeof(secInfoSize));
    auto secBegin = codeAddress + static_cast<uintptr_t>(curUnitOffset);
    for (uint8_t i = 0; i < secInfoSize; i++) {
        uint8_t secName;
        file.read(reinterpret_cast<char *>(&secName), sizeof(secName));
        auto secEnumName = static_cast<ElfSecName>(secName);
        uint32_t secSize;
        file.read(reinterpret_cast<char *>(&secSize), sizeof(secSize));
        SetSecSize(secSize, secEnumName);
        file.read(reinterpret_cast<char *>(secBegin), secSize);
        curUnitOffset += secSize;
        SetSecAddr(secBegin, secEnumName);
        secBegin += secSize;
    }
    LoadStackMapSection(file, secBegin, curUnitOffset);
}

void StubFileInfo::Save(const std::string &filename)
{
    std::string realPath;
    if (!RealPath(filename, realPath, false)) {
        return;
    }

    std::ofstream file(realPath.c_str(), std::ofstream::binary);
    SetStubNum(entries_.size());
    file.write(reinterpret_cast<char *>(&entryNum_), sizeof(entryNum_));
    file.write(reinterpret_cast<char *>(entries_.data()), sizeof(FuncEntryDes) * entryNum_);
    uint32_t moduleNum = GetCodeUnitsNum();
    file.write(reinterpret_cast<char *>(&moduleNum), sizeof(moduleNum_));
    file.write(reinterpret_cast<char *>(&totalCodeSize_), sizeof(totalCodeSize_));
    uint32_t asmStubSize = GetAsmStubSize();
    file.write(reinterpret_cast<char *>(&asmStubSize), sizeof(asmStubSize));
    uint64_t asmStubAddr = GetAsmStubAddr();
    file.write(reinterpret_cast<char *>(asmStubAddr), asmStubSize);
    for (size_t i = 0; i < moduleNum; i++) {
        des_[i].SaveSectionsInfo(file);
    }
    file.close();
}

bool StubFileInfo::Load()
{
    if (_binary_stub_an_length <= 1) {
        LOG_FULL(FATAL) << "stub.an length <= 1, is default and invalid.";
        return false;
    }

    BinaryBufferParser binBufparser(const_cast<uint8_t *>(_binary_stub_an_start), _binary_stub_an_length);
    binBufparser.ParseBuffer(&entryNum_, sizeof(entryNum_));
    entries_.resize(entryNum_);
    binBufparser.ParseBuffer(entries_.data(), sizeof(FuncEntryDes) * entryNum_);
    binBufparser.ParseBuffer(&moduleNum_, sizeof(moduleNum_));
    des_.resize(moduleNum_);
    binBufparser.ParseBuffer(&totalCodeSize_, sizeof(totalCodeSize_));

    ExecutedMemoryAllocator::AllocateBuf(totalCodeSize_, exeMem_);
    uint64_t codeAddress = reinterpret_cast<uint64_t>(exeMem_.addr_);
    uint32_t curUnitOffset = 0;
    uint32_t asmStubSize = 0;
    binBufparser.ParseBuffer(&asmStubSize, sizeof(asmStubSize));
    SetAsmStubSize(asmStubSize);
    binBufparser.ParseBuffer(reinterpret_cast<void *>(codeAddress), asmStubSize);
    SetAsmStubAddr(codeAddress);
    curUnitOffset += asmStubSize;

    for (size_t i = 0; i < moduleNum_; i++) {
        des_[i].LoadSectionsInfo(binBufparser, curUnitOffset, codeAddress);
    }

    for (auto &entry : entries_) {
        if (entry.IsGeneralRTStub()) {
            uint64_t begin = GetAsmStubAddr();
            entry.codeAddr_ += begin;
        } else {
            auto moduleDes = des_[entry.moduleIndex_];
            entry.codeAddr_ += moduleDes.GetSecAddr(ElfSecName::TEXT);
        }
    }
    LOG_COMPILER(INFO) << "loaded stub file successfully";
    return true;
}

void AnFileInfo::Save(const std::string &filename)
{
    std::string realPath;
    if (!RealPath(filename, realPath, false)) {
        return;
    }
    std::ofstream file(realPath.c_str(), std::ofstream::binary);
    SetStubNum(entries_.size());
    auto anVersion = AOTFileManager::AOT_VERSION;
    file.write(reinterpret_cast<char *>(anVersion.data()), sizeof(uint8_t) * AOTFileManager::AOT_VERSION_SIZE);
    file.write(reinterpret_cast<char *>(&entryNum_), sizeof(entryNum_));
    file.write(reinterpret_cast<char *>(entries_.data()), sizeof(FuncEntryDes) * entryNum_);
    uint32_t moduleNum = GetCodeUnitsNum();
    file.write(reinterpret_cast<char *>(&moduleNum), sizeof(moduleNum_));
    file.write(reinterpret_cast<char *>(&totalCodeSize_), sizeof(totalCodeSize_));
    LOG_COMPILER(DEBUG) << "total code size = " << (totalCodeSize_ / 1_KB) << "KB";
    for (size_t i = 0; i < moduleNum; i++) {
        des_[i].SaveSectionsInfo(file);
    }
    file.close();
}


void AnFileInfo::RewriteRelcateDeoptHandler([[maybe_unused]] EcmaVM *vm)
{
#if !WIN_OR_MAC_OR_IOS_PLATFORM
    JSThread *thread = vm->GetJSThread();
    uintptr_t patchAddr = thread->GetRTInterface(RTSTUB_ID(DeoptHandlerAsm));
    RewriteRelcateTextSection(LLVM_DEOPT_RELOCATE_SYMBOL, patchAddr);
#endif
}

void AnFileInfo::RewriteRelcateTextSection([[maybe_unused]] const char* symbol,
    [[maybe_unused]] uintptr_t patchAddr)
{
#if !WIN_OR_MAC_OR_IOS_PLATFORM
    for (auto &des: des_) {
        uint32_t relaTextSize = des.GetSecSize(ElfSecName::RELATEXT);
        if (relaTextSize != 0) {
            uint64_t relatextAddr = des.GetSecAddr(ElfSecName::RELATEXT);
            uint64_t textAddr = des.GetSecAddr(ElfSecName::TEXT);
            uint64_t symTabAddr = des.GetSecAddr(ElfSecName::SYMTAB);
            uint32_t symTabSize = des.GetSecSize(ElfSecName::SYMTAB);
            uint32_t strTabSize = des.GetSecSize(ElfSecName::STRTAB);
            uint64_t strTabAddr = des.GetSecAddr(ElfSecName::STRTAB);
            RelocateTextInfo relaText = {textAddr, relatextAddr, relaTextSize};
            SymAndStrTabInfo symAndStrTabInfo = {symTabAddr, symTabSize, strTabAddr, strTabSize};
            Relocator relocate(relaText, symAndStrTabInfo);
#ifndef NDEBUG
            relocate.DumpRelocateText();
#endif
            relocate.RelocateBySymbol(symbol, patchAddr);
        }
    }
#endif
}

bool AnFileInfo::Load(const std::string &filename)
{
    std::string realPath;
    if (!RealPath(filename, realPath, false)) {
        LOG_COMPILER(ERROR) << "Can not load aot file from path [ "  << filename << " ], "
                            << "please execute ark_aot_compiler with options --aot-file.";
        UNREACHABLE();
        return false;
    }

    std::ifstream file(realPath.c_str(), std::ofstream::binary);
    if (!file.good()) {
        LOG_COMPILER(INFO) << "Fail to load an file: " << realPath.c_str();
        file.close();
        return false;
    }

    std::array<uint8_t, AOTFileManager::AOT_VERSION_SIZE> anVersion;
    file.read(reinterpret_cast<char *>(anVersion.data()), sizeof(uint8_t) * AOTFileManager::AOT_VERSION_SIZE);
    if (!AnVersionCheck(anVersion)) {
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char *>(&entryNum_), sizeof(entryNum_));
    entries_.resize(entryNum_);
    file.read(reinterpret_cast<char *>(entries_.data()), sizeof(FuncEntryDes) * entryNum_);
    file.read(reinterpret_cast<char *>(&moduleNum_), sizeof(moduleNum_));
    des_.resize(moduleNum_);
    file.read(reinterpret_cast<char *>(&totalCodeSize_), sizeof(totalCodeSize_));

    ExecutedMemoryAllocator::AllocateBuf(totalCodeSize_, exeMem_);
    uint64_t codeAddress = reinterpret_cast<uint64_t>(exeMem_.addr_);
    uint32_t curUnitOffset = 0;
    for (size_t i = 0; i < moduleNum_; i++) {
        des_[i].LoadSectionsInfo(file, curUnitOffset, codeAddress);
    }

    size_t len = entries_.size();
    for (size_t i = 0; i < len; i++) {
        FuncEntryDes& funcDes = entries_[i];
        auto moduleDes = des_[funcDes.moduleIndex_];
        funcDes.codeAddr_ += moduleDes.GetSecAddr(ElfSecName::TEXT);
        if (funcDes.isMainFunc_) {
            mainEntryMap_[funcDes.indexInKindOrMethodId_] = funcDes.codeAddr_;
#ifndef NDEBUG
            LOG_COMPILER(INFO) << "AnFileInfo Load main method id: " << funcDes.indexInKindOrMethodId_
                               << " code addr: " << reinterpret_cast<void*>(funcDes.codeAddr_);
#endif
        }
    }

    LOG_COMPILER(INFO) << "loaded aot file: " << filename.c_str();
    isLoad_ = true;
    return true;
}

template<size_t Size>
bool AnFileInfo::AnVersionCheck(std::array<uint8_t, Size> anVersion)
{
    if (anVersion > AOTFileManager::AOT_VERSION) {
        auto convToStr = [] (std::array<uint8_t, AOTFileManager::AOT_VERSION_SIZE> version) -> std::string {
            std::string ret = "";
            for (size_t i = 0; i < AOTFileManager::AOT_VERSION_SIZE; ++i) {
                if (i) {
                    ret += ".";
                }
                ret += std::to_string(version[i]);
            }
            return ret;
        };
        LOG_COMPILER(ERROR) << "Load an file failed, an file version is incorrect, "
                            << "expected version should be less or equal than "
                            << convToStr(AOTFileManager::AOT_VERSION)
                            << ", but got " << convToStr(anVersion);
        return false;
    }
    return true;
}

bool AnFileInfo::IsLoadMain(const JSPandaFile *jsPandaFile, const CString &entry) const
{
    auto methodId = jsPandaFile->GetMainMethodIndex(entry);
#ifndef NDEBUG
    LOG_COMPILER(INFO) << "AnFileInfo IsLoadMain method id: " << methodId << " entry: " << entry;
#endif
    auto it = mainEntryMap_.find(methodId);
    if (it == mainEntryMap_.end()) {
        return false;
    }
    return true;
}

void AOTFileManager::Iterate(const RootVisitor &v)
{
    for (auto &iter : desCPs_) {
        for (auto &curCP : iter.second) {
            v(Root::ROOT_VM, ObjectSlot(reinterpret_cast<uintptr_t>(&iter.second.at(curCP.first))));
        }
    }
}

void AOTFileManager::LoadStubFile(const std::string &fileName)
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    if (!anFileDataManager->SafeLoad(fileName, AnFileDataManager::Type::STUB)) {
        return;
    }
    auto info = anFileDataManager->SafeGetStubFileInfo();
    auto stubs = info->GetStubs();
    InitializeStubEntries(stubs);
}

void AOTFileManager::LoadAnFile(const std::string &fileName)
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    if (!anFileDataManager->SafeLoad(fileName, AnFileDataManager::Type::AOT, vm_)) {
        return;
    }
}

void AOTFileManager::LoadAnFile(JSPandaFile *jsPandaFile)
{
    auto fileName = GetAotFileName(vm_, jsPandaFile, AOTFileManager::FILE_EXTENSION_AN);
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    if (!anFileDataManager->SafeLoad(fileName, AnFileDataManager::Type::AOT, vm_)) {
        return;
    }

    if (IsLoad(jsPandaFile)) {
        uint32_t index = GetAnFileIndex(jsPandaFile);
        jsPandaFile->SetAOTFileInfoIndex(index);
    }
}

void AOTFileManager::LoadAiFile(const JSPandaFile *jsPandaFile)
{
    Snapshot snapshot(vm_);
    auto filename = GetAotFileName(vm_, jsPandaFile, AOTFileManager::FILE_EXTENSION_AI);
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    std::string baseName = JSFilePath::GetFileName(filename.c_str());
    uint32_t anFileInfoIndex = anFileDataManager->SafeGetFileInfoIndex(baseName + FILE_EXTENSION_AN);
    if (anFileInfoIndex == INVALID_INDEX) {
        // The abc file does not have corresponding an and ai files
        return;
    }

    auto iter = desCPs_.find(anFileInfoIndex);
    if (iter == desCPs_.end()) {
#if !WIN_OR_MAC_OR_IOS_PLATFORM
        snapshot.Deserialize(SnapshotType::AI, filename.c_str());
#endif
    }
}

void AOTFileManager::LoadAiFile([[maybe_unused]] const std::string &filename)
{
    Snapshot snapshot(vm_);
#if !WIN_OR_MAC_OR_IOS_PLATFORM
    snapshot.Deserialize(SnapshotType::AI, filename.c_str());
#endif
}

const std::shared_ptr<AnFileInfo> AOTFileManager::GetAnFileInfo(const JSPandaFile *jsPandaFile) const
{
    uint32_t index = GetAnFileIndex(jsPandaFile);
    if (index == INVALID_INDEX) {
        return nullptr;
    }
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    return anFileDataManager->SafeGetAnFileInfo(index);
}

bool AOTFileManager::IsLoad(const JSPandaFile *jsPandaFile) const
{
    if (vm_->GetJSOptions().GetAOTOutputFile().empty()) {
        return false;
    }

    const std::shared_ptr<AnFileInfo> anFileInfo = GetAnFileInfo(jsPandaFile);
    if (anFileInfo == nullptr) {
        return false;
    }
    return anFileInfo->IsLoad();
}

bool AOTFileManager::IsLoadMain(const JSPandaFile *jsPandaFile, const CString &entry) const
{
    if (!jsPandaFile->IsLoadedAOT()) {
        return false;
    }

    const std::shared_ptr<AnFileInfo> anFileInfo = GetAnFileInfo(jsPandaFile);
    if (anFileInfo == nullptr) {
        return false;
    }

    return anFileInfo->IsLoadMain(jsPandaFile, entry);
}

uint32_t AOTFileManager::GetAnFileIndex(const JSPandaFile *jsPandaFile) const
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();

    // run via command line
    if (vm_->GetJSOptions().WasAOTOutputFileSet()) {
        std::string jsPandaFileDesc = jsPandaFile->GetJSPandaFileDesc().c_str();
        std::string baseName = JSFilePath::GetFileName(jsPandaFileDesc);
        if (baseName.empty()) {
            return INVALID_INDEX;
        }
        std::string anFileName = baseName + FILE_EXTENSION_AN;
        return anFileDataManager->SafeGetFileInfoIndex(anFileName);
    }

    // run from app hap
    std::string hapName = JSFilePath::GetHapName(jsPandaFile);
    if (hapName.empty()) {
        return INVALID_INDEX;
    }
    std::string anFileName = hapName + FILE_EXTENSION_AN;
    return anFileDataManager->SafeGetFileInfoIndex(anFileName);
}

bool AOTFileManager::InsideStub(uintptr_t pc) const
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    return anFileDataManager->SafeInsideStub(pc);
}

bool AOTFileManager::InsideAOT(uintptr_t pc) const
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    return anFileDataManager->SafeInsideAOT(pc);
}

AOTFileInfo::CallSiteInfo AOTFileManager::CalCallSiteInfo(uintptr_t retAddr) const
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    return anFileDataManager->SafeCalCallSiteInfo(retAddr);
}

void AOTFileManager::PrintAOTEntry(const JSPandaFile *file, const Method *method, uintptr_t entry)
{
    uint32_t mId = method->GetMethodId().GetOffset();
    std::string mName = method->GetMethodName(file);
    std::string fileName = file->GetFileName();
    LOG_COMPILER(INFO) << "Bind " << mName << "@" << mId << "@" << fileName
                       << " -> AOT-Entry = " << reinterpret_cast<void*>(entry);
}

void AOTFileManager::SetAOTMainFuncEntry(JSHandle<JSFunction> mainFunc, const JSPandaFile *jsPandaFile,
    std::string_view entryPoint)
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    uint32_t anFileInfoIndex = jsPandaFile->GetAOTFileInfoIndex();
    const std::shared_ptr<AnFileInfo> anFileInfo = anFileDataManager->SafeGetAnFileInfo(anFileInfoIndex);
    // get main func method
    auto mainFuncMethodId = jsPandaFile->GetMainMethodIndex(entryPoint.data());
    auto mainEntry = anFileInfo->GetMainFuncEntry(mainFuncMethodId);
    MethodLiteral *mainMethod = jsPandaFile->FindMethodLiteral(mainFuncMethodId);
    mainMethod->SetAotCodeBit(true);
    mainMethod->SetNativeBit(false);
    Method *method = mainFunc->GetCallTarget();
    method->SetDeoptThreshold(vm_->GetJSOptions().GetDeoptThreshold());
    method->SetCodeEntryAndMarkAOT(reinterpret_cast<uintptr_t>(mainEntry));
#ifndef NDEBUG
    PrintAOTEntry(jsPandaFile, method, mainEntry);
#endif
}

void AOTFileManager::SetAOTFuncEntry(const JSPandaFile *jsPandaFile, Method *method, uint32_t entryIndex)
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    uint32_t anFileInfoIndex = jsPandaFile->GetAOTFileInfoIndex();
    const std::shared_ptr<AnFileInfo> anFileInfo = anFileDataManager->SafeGetAnFileInfo(anFileInfoIndex);
    const AOTFileInfo::FuncEntryDes &entry = anFileInfo->GetStubDes(entryIndex);
    uint64_t codeEntry = entry.codeAddr_;
#ifndef NDEBUG
    PrintAOTEntry(jsPandaFile, method, codeEntry);
#endif
    if (!codeEntry) {
        return;
    }
    method->SetDeoptThreshold(vm_->GetJSOptions().GetDeoptThreshold());
    method->SetCodeEntryAndMarkAOT(codeEntry);
}

void AOTFileManager::SetAOTFuncEntryForLiteral(const JSPandaFile *jsPandaFile, const TaggedArray *literal,
                                               const AOTLiteralInfo *entryIndexes)
{
    size_t elementsLen = literal->GetLength();
    JSTaggedValue value = JSTaggedValue::Undefined();
    int pos = 0;
    for (size_t i = 0; i < elementsLen; i++) {
        value = literal->Get(i);
        if (value.IsJSFunction()) {
            JSTaggedValue index = entryIndexes->Get(pos++);
            int entryIndex = index.GetInt();
            // -1 : this jsfunction is a large function
            if (entryIndex == -1) {
                continue;
            }
            SetAOTFuncEntry(jsPandaFile, JSFunction::Cast(value)->GetCallTarget(), static_cast<uint32_t>(entryIndex));
        }
    }
}

kungfu::ArkStackMapParser* AOTFileManager::GetStackMapParser() const
{
    return arkStackMapParser_;
}

void AOTFileManager::AdjustBCStubAndDebuggerStubEntries(JSThread *thread,
    const std::vector<AOTFileInfo::FuncEntryDes> &stubs,
    const AsmInterParsedOption &asmInterOpt)
{
    auto defaultBCStubDes = stubs[BytecodeStubCSigns::SingleStepDebugging];
    auto defaultBCDebuggerStubDes = stubs[BytecodeStubCSigns::BCDebuggerEntry];
    auto defaultBCDebuggerExceptionStubDes = stubs[BytecodeStubCSigns::BCDebuggerExceptionEntry];
    ASSERT(defaultBCStubDes.kind_ == CallSignature::TargetKind::BYTECODE_HELPER_HANDLER);
    if (asmInterOpt.handleStart >= 0 && asmInterOpt.handleStart <= asmInterOpt.handleEnd) {
        for (int i = asmInterOpt.handleStart; i <= asmInterOpt.handleEnd; i++) {
            thread->SetBCStubEntry(static_cast<size_t>(i), defaultBCStubDes.codeAddr_);
        }
#define DISABLE_SINGLE_STEP_DEBUGGING(name) \
        thread->SetBCStubEntry(BytecodeStubCSigns::ID_##name, stubs[BytecodeStubCSigns::ID_##name].codeAddr_);
        INTERPRETER_DISABLE_SINGLE_STEP_DEBUGGING_BC_STUB_LIST(DISABLE_SINGLE_STEP_DEBUGGING)
#undef DISABLE_SINGLE_STEP_DEBUGGING
    }
    for (size_t i = 0; i < BCStubEntries::EXISTING_BC_HANDLER_STUB_ENTRIES_COUNT; i++) {
        if (i == BytecodeStubCSigns::ID_ExceptionHandler) {
            thread->SetBCDebugStubEntry(i, defaultBCDebuggerExceptionStubDes.codeAddr_);
            continue;
        }
        thread->SetBCDebugStubEntry(i, defaultBCDebuggerStubDes.codeAddr_);
    }
}

void AOTFileManager::InitializeStubEntries(const std::vector<AnFileInfo::FuncEntryDes>& stubs)
{
    auto thread = vm_->GetAssociatedJSThread();
    size_t len = stubs.size();
    for (size_t i = 0; i < len; i++) {
        auto des = stubs[i];
        if (des.IsCommonStub()) {
            thread->SetFastStubEntry(des.indexInKindOrMethodId_, des.codeAddr_);
        } else if (des.IsBCStub()) {
            thread->SetBCStubEntry(des.indexInKindOrMethodId_, des.codeAddr_);
#if ECMASCRIPT_ENABLE_ASM_FILE_LOAD_LOG
            auto start = GET_MESSAGE_STRING_ID(HandleLdundefined);
            std::string format = MessageString::GetMessageString(des.indexInKindOrMethodId_ + start);
            LOG_ECMA(DEBUG) << "bytecode index: " << des.indexInKindOrMethodId_ << " :" << format
                << " addr: 0x" << std::hex << des.codeAddr_;
#endif
        } else if (des.IsBuiltinsStub()) {
            thread->SetBuiltinStubEntry(des.indexInKindOrMethodId_, des.codeAddr_);
#if ECMASCRIPT_ENABLE_ASM_FILE_LOAD_LOG
            int start = GET_MESSAGE_STRING_ID(CharCodeAt);
            std::string format = MessageString::GetMessageString(des.indexInKindOrMethodId_ + start - 1);  // -1: NONE
            LOG_ECMA(DEBUG) << "builtins index: " << std::dec << des.indexInKindOrMethodId_ << " :" << format
                << " addr: 0x" << std::hex << des.codeAddr_;
#endif
        } else {
            thread->RegisterRTInterface(des.indexInKindOrMethodId_, des.codeAddr_);
#if ECMASCRIPT_ENABLE_ASM_FILE_LOAD_LOG
                int start = GET_MESSAGE_STRING_ID(CallRuntime);
                std::string format = MessageString::GetMessageString(des.indexInKindOrMethodId_ + start);
                LOG_ECMA(DEBUG) << "runtime index: " << std::dec << des.indexInKindOrMethodId_ << " :" << format
                    << " addr: 0x" << std::hex << des.codeAddr_;
#endif
        }
    }
    AsmInterParsedOption asmInterOpt = vm_->GetJSOptions().GetAsmInterParsedOption();
    AdjustBCStubAndDebuggerStubEntries(thread, stubs, asmInterOpt);
}

bool AOTFileManager::RewriteDataSection(uintptr_t dataSec, size_t size,
    uintptr_t newData, size_t newSize)
{
    if (memcpy_s(reinterpret_cast<void *>(dataSec), size,
                 reinterpret_cast<void *>(newData), newSize) != EOK) {
        LOG_FULL(FATAL) << "memset failed";
        return false;
    }
    return true;
}

void AOTFileManager::AddConstantPool(const CString &snapshotFileName, JSTaggedValue deserializedCPList)
{
    AnFileDataManager *anFileDataManager = AnFileDataManager::GetInstance();
    std::string baseName = JSFilePath::GetFileName(snapshotFileName.c_str());
    uint32_t anFileInfoIndex = anFileDataManager->SafeGetFileInfoIndex(baseName + FILE_EXTENSION_AN);

    desCPs_.insert({anFileInfoIndex, CMap<int32_t, JSTaggedValue>{}});
    CMap<int32_t, JSTaggedValue> &cpMap = desCPs_[anFileInfoIndex];

    JSHandle<TaggedArray> cpList(vm_->GetJSThread(), deserializedCPList);
    uint32_t len = cpList->GetLength();
    for (uint32_t pos = 0; pos < len; pos += DESERI_CP_ITEM_SIZE) {
        int32_t constantPoolID = cpList->Get(pos).GetInt();
        JSTaggedValue cp = cpList->Get(pos + 1);
        cpMap.insert({constantPoolID, cp});
    }
}

JSHandle<JSTaggedValue> AOTFileManager::GetDeserializedConstantPool(const JSPandaFile *jsPandaFile, int32_t cpID)
{
    // The deserialization of the 'ai' data used by the multi-work
    // is not implemented yet, so there may be a case where
    // desCPs_ is empty, in which case the Hole will be returned
    if (desCPs_.size() == 0) {
        return JSHandle<JSTaggedValue>(vm_->GetJSThread(), JSTaggedValue::Hole());
    }
    uint32_t anFileInfoIndex = jsPandaFile->GetAOTFileInfoIndex();
    CMap<int32_t, JSTaggedValue> &cpMap = desCPs_.at(anFileInfoIndex);
    auto iter = cpMap.find(cpID);
    if (iter == cpMap.end()) {
        LOG_COMPILER(FATAL) << "can not find deserialized constantpool in anFileInfo, constantPoolID is " << cpID;
        UNREACHABLE();
    }
    return JSHandle<JSTaggedValue>(uintptr_t(&iter->second));
}

AOTFileManager::~AOTFileManager()
{
    if (arkStackMapParser_ != nullptr) {
        delete arkStackMapParser_;
        arkStackMapParser_ = nullptr;
    }
}

AOTFileManager::AOTFileManager(EcmaVM *vm) : vm_(vm), factory_(vm->GetFactory())
{
    bool enableLog = vm->GetJSOptions().WasSetCompilerLogOption();
    arkStackMapParser_ = new kungfu::ArkStackMapParser(enableLog);
}

JSTaggedValue AOTFileManager::GetAbsolutePath(JSThread *thread, JSTaggedValue relativePathVal)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    CString relativePath = ConvertToString(relativePathVal);
    CString absPath;
    if (!GetAbsolutePath(relativePath, absPath)) {
        LOG_FULL(FATAL) << "Get Absolute Path failed";
        return JSTaggedValue::Hole();
    }
    JSTaggedValue absPathVal = factory->NewFromUtf8(absPath).GetTaggedValue();
    return absPathVal;
}

bool AOTFileManager::GetAbsolutePath(const CString &relativePathCstr, CString &absPathCstr)
{
    std::string relativePath = ConvertToStdString(relativePathCstr);
    std::string absPath = "";
    if (RealPath(relativePath, absPath)) {
        absPathCstr = ConvertToString(absPath);
        return true;
    }
    return false;
}

std::string AOTFileManager::GetAotFileName(EcmaVM *vm, const JSPandaFile *jsPandaFile,
                                           const std::string &extensionName) const
{
    auto option = vm->GetJSOptions();
    std::string aotFileName;
    if (option.WasAOTOutputFileSet()) {
        std::string fullPathName = JSFilePath::GetAotFullPathName(jsPandaFile);
        aotFileName = fullPathName + extensionName;
    } else {
        std::string hapName = JSFilePath::GetHapName(jsPandaFile);
        aotFileName = hapName + extensionName;
    }
    return aotFileName;
}

void BinaryBufferParser::ParseBuffer(void *dst, uint32_t count)
{
    if (count > 0 && count + offset_ <= length_) {
        if (memcpy_s(dst, count, buffer_ + offset_, count) != EOK) {
            LOG_FULL(FATAL) << "memcpy_s failed";
        }
        offset_ = offset_ + count;
    } else {
        LOG_FULL(FATAL) << "parse buffer error, length is 0 or overflow";
    }
}

void BinaryBufferParser::ParseBuffer(uint8_t *dst, uint32_t count, uint8_t *src)
{
    if (src >= buffer_ && src + count <= buffer_ + length_) {
        if (memcpy_s(dst, count, src, count) != EOK) {
            LOG_FULL(FATAL) << "memcpy_s failed";
        }
    } else {
        LOG_FULL(FATAL) << "parse buffer error, length is 0 or overflow";
    }
}

bool AOTFileInfo::CalCallSiteInfo(uintptr_t retAddr,
    std::tuple<uint64_t, uint8_t *, int, kungfu::CalleeRegAndOffsetVec>& ret) const
{
    uint64_t textStart = 0;
    uint8_t *stackmapAddr = nullptr;
    int delta = 0;
    auto& des = GetCodeUnits();
    auto& funcEntryDes = GetStubs();

    auto cmp = [](const AOTFileInfo::FuncEntryDes &a, const AOTFileInfo::FuncEntryDes &b) {
                    return a.codeAddr_ < b.codeAddr_;
                };
    size_t len = des.size();
    for (size_t i = 0; i < len; i++) {
        auto d = des[i];
        uint64_t addr = d.GetSecAddr(ElfSecName::TEXT);
        uint32_t size = d.GetSecSize(ElfSecName::TEXT);
        if (retAddr < addr || retAddr >= addr + size) {
            continue;
        }
        stackmapAddr = d.GetArkStackMapRawPtr();
        ASSERT(stackmapAddr != nullptr);
        textStart = addr;
        auto startIndex = d.GetStartIndex();
        auto funcCount = d.GetFuncCount();
        auto s = funcEntryDes.begin() + startIndex;
        auto t = funcEntryDes.begin() + startIndex + funcCount;
        AOTFileInfo::FuncEntryDes target;
        target.codeAddr_ = retAddr - 1; // -1: for pc
        auto it = std::upper_bound(s, t, target, cmp);
        --it;
        ASSERT(it != t);
        ASSERT((it->codeAddr_ <= target.codeAddr_) && (target.codeAddr_ < it->codeAddr_ + it->funcSize_));
        delta = it->fpDeltaPrevFrameSp_;
        kungfu::CalleeRegAndOffsetVec calleeRegInfo;
        for (uint32_t j = 0; j < it->calleeRegisterNum_; j++) {
            kungfu::DwarfRegType reg = static_cast<kungfu::DwarfRegType>(it->CalleeReg2Offset_[2 * j]);
            kungfu::OffsetType offset = static_cast<kungfu::OffsetType>(it->CalleeReg2Offset_[2 * j + 1]);
            kungfu::DwarfRegAndOffsetType regAndOffset = std::make_pair(reg, offset);
            calleeRegInfo.emplace_back(regAndOffset);
        }
        ret = std::make_tuple(textStart, stackmapAddr, delta, calleeRegInfo);
        return true;
    }
    return false;
}

AnFileDataManager *AnFileDataManager::GetInstance()
{
    static AnFileDataManager anFileDataManager;
    return &anFileDataManager;
}

AnFileDataManager::~AnFileDataManager()
{
    SafeDestoryAllData();
}

void AnFileDataManager::SafeDestoryAllData()
{
    os::memory::WriteLockHolder lock(lock_);
    if (loadedStub_ != nullptr) {
        ExecutedMemoryAllocator::DestoryBuf(loadedStub_->GetExeMem());
        loadedStub_ = nullptr;
    }

    for (auto &iter : loadedAn_) {
        ExecutedMemoryAllocator::DestoryBuf(iter->GetExeMem());
    }
    loadedAn_.clear();
}

bool AnFileDataManager::SafeLoad(const std::string &fileName, Type type, EcmaVM* vm)
{
    os::memory::WriteLockHolder lock(lock_);
    if (type == Type::STUB) {
        if (loadedStub_ != nullptr) {
                return true;
            }
        return UnsafeLoadFromStub();
    } else {
        const std::shared_ptr<const AOTFileInfo> aotFileInfo = UnsafeFind(fileName);
        if (aotFileInfo != nullptr) {
            return true;
        }
        return UnsafeLoadFromAOT(fileName, vm);
    }
}

std::shared_ptr<AnFileInfo> AnFileDataManager::UnsafeFind(const std::string &fileName) const
{
    // note: This method is not thread-safe
    // need to ensure that the instance of AnFileDataManager has been locked before use
    const auto iter = anFileNameToIndexMap_.find(fileName);
    if (iter == anFileNameToIndexMap_.end()) {
        return nullptr;
    }
    uint32_t index = iter->second;
    return loadedAn_.at(index);
}

bool AnFileDataManager::UnsafeLoadFromStub()
{
    // note: This method is not thread-safe
    // need to ensure that the instance of AnFileDataManager has been locked before use
    loadedStub_ = std::make_shared<StubFileInfo>(StubFileInfo());
    if (!loadedStub_->Load()) {
        return false;
    }
    return true;
}

bool AnFileDataManager::UnsafeLoadFromAOT(const std::string &fileName, EcmaVM *vm)
{
    // note: This method is not thread-safe
    // need to ensure that the instance of AnFileDataManager has been locked before use
    std::shared_ptr<AnFileInfo> info = std::make_shared<AnFileInfo>(AnFileInfo());
    if (!info->Load(fileName)) {
        return false;
    }
    info->RewriteRelcateDeoptHandler(vm);
    std::string anBasename = JSFilePath::GetBaseName(fileName);
    anFileNameToIndexMap_.insert({anBasename, loadedAn_.size()});
    loadedAn_.emplace_back(info);
    return true;
}

uint32_t AnFileDataManager::SafeGetFileInfoIndex(const std::string &fileName)
{
    os::memory::ReadLockHolder lock(lock_);
    auto iter = anFileNameToIndexMap_.find(fileName);
    if (iter == anFileNameToIndexMap_.end()) {
        return INVALID_INDEX;
    }
    return anFileNameToIndexMap_.at(fileName);
}

std::shared_ptr<AnFileInfo> AnFileDataManager::SafeGetAnFileInfo(uint32_t index)
{
    os::memory::ReadLockHolder lock(lock_);
    return loadedAn_.at(index);
}

std::shared_ptr<StubFileInfo> AnFileDataManager::SafeGetStubFileInfo()
{
    os::memory::ReadLockHolder lock(lock_);
    return loadedStub_;
}

bool AnFileDataManager::SafeInsideStub(uintptr_t pc)
{
    os::memory::ReadLockHolder lock(lock_);
    if (loadedStub_ == nullptr) {
        LOG_COMPILER(ERROR) << "SafeInsideStub: The stub file is not loaded.";
        return false;
    }

    uint64_t stubStartAddr = loadedStub_->GetAsmStubAddr();
    uint64_t stubEndAddr = stubStartAddr + loadedStub_->GetAsmStubSize();
    if (pc >= stubStartAddr && pc <= stubEndAddr) {
        return true;
    }

    const std::vector<ModuleSectionDes> &des = loadedStub_->GetCodeUnits();
    for (auto &curDes : des) {
        if (curDes.ContainCode(pc)) {
            return true;
        }
    }

    return false;
}

bool AnFileDataManager::SafeInsideAOT(uintptr_t pc)
{
    os::memory::ReadLockHolder lock(lock_);
    for (auto &info : loadedAn_) {
        const std::vector<ModuleSectionDes> &des = info->GetCodeUnits();
        for (auto &curDes : des) {
            if (curDes.ContainCode(pc)) {
                return true;
            }
        }
    }
    return false;
}

AOTFileInfo::CallSiteInfo AnFileDataManager::SafeCalCallSiteInfo(uintptr_t retAddr)
{
    os::memory::ReadLockHolder lock(lock_);
    AOTFileInfo::CallSiteInfo callsiteInfo;

    bool ans = false;
    if (loadedStub_ != nullptr) {
        ans = loadedStub_->CalCallSiteInfo(retAddr, callsiteInfo);
    }
    if (ans) {
        return callsiteInfo;
    }
    // aot
    for (auto &info : loadedAn_) {
        ans = info->CalCallSiteInfo(retAddr, callsiteInfo);
        if (ans) {
            return callsiteInfo;
        }
    }
    return callsiteInfo;
}
}
