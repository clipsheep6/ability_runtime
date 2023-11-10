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

#include "ecmascript/compiler/pgo_type/pgo_type_parser.h"

#include "ecmascript/compiler/pgo_type/pgo_hclass_generator.h"
#include "ecmascript/compiler/pgo_type/pgo_type_location.h"
#include "ecmascript/compiler/pgo_type/pgo_type_recorder.h"
#include "ecmascript/elements.h"
#include "ecmascript/jspandafile/js_pandafile_manager.h"

namespace panda::ecmascript::kungfu {
void BaseParser::Parse(const BytecodeInfoCollector &collector, const PGOTypeRecorder &typeRecorder,
    const PGOHClassGenerator &generator, uint32_t methodOffset)
{
    const JSPandaFile *jsPandaFile = collector.GetJSPandaFile();
    const PGOBCInfo *bcInfo = collector.GetPGOBCInfo();

    bcInfo->IterateInfoByType(methodOffset, type_,
        [this, jsPandaFile, &typeRecorder, &generator, methodOffset](
        const uint32_t bcIdx, const uint32_t bcOffset, const uint32_t) {
        auto defType = typeRecorder.GetPGODefOpType(bcOffset);
        if (defType.IsNone()) {
            return;
        }

        PGOTypeLocation loc(jsPandaFile, methodOffset, bcIdx);
        if (!RecordTypeInfo(defType, loc)) {
            return;
        }

        GenerateHClass(generator, loc);
    });
}

bool ClassParser::RecordTypeInfo(const PGODefineOpType &defType, const PGOTypeLocation &loc)
{
    auto rootType = defType.GetProfileType();
    auto ctorPt = defType.GetCtorPt();
    if (ctorPt.IsNone()) {
        return false;
    }
    PGOSampleType ctorSampleType(ctorPt);

    auto protoPt = defType.GetProtoTypePt();
    if (protoPt.IsNone()) {
        return false;
    }
    PGOSampleType protoSampleType(protoPt);

    ptManager_->RecordLocationToRootType(loc, rootType);
    auto ctorLoc = loc.ChangeType(PGOTypeLocation::Type::CONSTRUCTOR);
    ptManager_->RecordLocationToRootType(ctorLoc, ctorPt);

    auto protoLoc = loc.ChangeType(PGOTypeLocation::Type::PROTOTYPE);
    ptManager_->RecordLocationToRootType(protoLoc, protoPt);
    return true;
}

void ClassParser::GenerateHClass(const PGOHClassGenerator &generator, const PGOTypeLocation &loc)
{
    auto thread = ptManager_->GetJSThread();
    [[maybe_unused]] EcmaHandleScope scope(thread);

    auto rootType = ptManager_->GetRootIdByLocation(loc);
    PGOSampleType iSampleType(rootType);

    auto ctorLoc = loc.ChangeType(PGOTypeLocation::Type::CONSTRUCTOR);
    auto ctorPt = ptManager_->GetRootIdByLocation(ctorLoc);
    PGOSampleType ctorSampleType(ctorPt);

    auto protoLoc = loc.ChangeType(PGOTypeLocation::Type::PROTOTYPE);
    auto protoPt = ptManager_->GetRootIdByLocation(protoLoc);
    PGOSampleType protoSampleType(protoPt);

    // testcase: propertyaccessor2.ts. protoSampleType not find desc
    if (generator.FindHClassLayoutDesc(ctorSampleType) && generator.FindHClassLayoutDesc(protoSampleType)) {
        generator.GenerateHClass(ctorSampleType);
        generator.GenerateHClass(protoSampleType);

        auto phValue = ptManager_->QueryHClass(protoPt, protoPt);
        JSHandle<JSHClass> phclass(thread, phValue);
        JSHandle<JSObject> prototype = thread->GetEcmaVM()->GetFactory()->NewJSObject(phclass);
        generator.GenerateIHClass(iSampleType, prototype);
    }
}

bool ArrayParser::RecordTypeInfo(const PGODefineOpType &defType, const PGOTypeLocation &loc)
{
    auto rootType = defType.GetProfileType();
    if (!rootType.IsLiteralType()) {
        return false;
    }

    ElementsKind kind = defType.GetElementsKind();
    ptManager_->RecordLocationToElementsKind(loc, kind);

    auto traceId = rootType.GetId();
    auto hclassIdx = ptManager_->GetJSThread()->GetArrayHClassIndexMap().at(kind);
    ptManager_->RecordConstantIndex(traceId, static_cast<uint32_t>(hclassIdx));
    return true;
}

bool ObjectLiteralParser::RecordTypeInfo(const PGODefineOpType &defType, const PGOTypeLocation &loc)
{
    auto rootType = defType.GetProfileType();
    ptManager_->RecordLocationToRootType(loc, rootType);
    return true;
}

void ObjectLiteralParser::GenerateHClass(const PGOHClassGenerator &generator, const PGOTypeLocation &loc)
{
    auto thread = ptManager_->GetJSThread();
    [[maybe_unused]] EcmaHandleScope scope(thread);

    auto rootType = ptManager_->GetRootIdByLocation(loc);
    PGOSampleType rootSampleType(rootType);
    generator.GenerateHClass(rootSampleType);
}

PGOTypeParser::PGOTypeParser(const PGOProfilerDecoder &decoder, PGOTypeManager *ptManager)
    : decoder_(decoder), ptManager_(ptManager)
{
    parsers_.emplace_back(std::make_unique<ClassParser>(ptManager));
    parsers_.emplace_back(std::make_unique<EmptyArrayParser>(ptManager));
    parsers_.emplace_back(std::make_unique<ArrayLiteralParser>(ptManager));
    parsers_.emplace_back(std::make_unique<ObjectLiteralParser>(ptManager));
}

void PGOTypeParser::CreatePGOType(BytecodeInfoCollector &collector)
{
    const JSPandaFile *jsPandaFile = collector.GetJSPandaFile();
    collector.IterateAllMethods([this, jsPandaFile, &collector](uint32_t methodOffset) {
        PGOTypeRecorder typeRecorder(decoder_, jsPandaFile, methodOffset);
        const PGOHClassGenerator generator(typeRecorder, ptManager_);
        for (auto &parser : parsers_) {
            parser->Parse(collector, typeRecorder, generator, methodOffset);
        }
    });
}
}  // namespace panda::ecmascript
