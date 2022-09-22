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

#include "ecmascript/jspandafile/scope_info_extractor.h"

#include "ecmascript/interpreter/frame_handler.h"
#include "ecmascript/jspandafile/literal_data_extractor.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/tagged_array-inl.h"

namespace panda::ecmascript {
JSTaggedValue ScopeInfoExtractor::GenerateScopeInfo(JSThread *thread, uint16_t scopeId)
{
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    Method *method = FrameHandler(thread).GetMethod();
    const JSPandaFile *jsPandaFile = method->GetJSPandaFile();
    JSHandle<JSTaggedValue> constpool(thread, method->GetConstantPool());

    JSHandle<TaggedArray> elementsLiteral;
    if (jsPandaFile->IsNewVersion()) {
#ifdef NEW_INSTRUCTION_DEFINE
        const ConstantPool *taggedPool = ConstantPool::Cast(constpool.GetTaggedValue().GetTaggedObject());
        panda_file::File::IndexHeader *indexHeader = taggedPool->GetIndexHeader();
        auto pf = jsPandaFile->GetPandaFile();
        Span<const panda_file::File::EntityId> indexs = pf->GetMethodIndex(indexHeader);
        panda_file::File::EntityId id = indexs[scopeId];
        elementsLiteral = LiteralDataExtractor::GetDatasIgnoreType(thread, jsPandaFile, id, constpool);
#endif
    } else {
        elementsLiteral = LiteralDataExtractor::GetDatasIgnoreType(thread, jsPandaFile, scopeId, constpool);
    }

    ASSERT(elementsLiteral->GetLength() > 0);
    size_t length = elementsLiteral->GetLength();

    auto buffer = ecmaVm->GetNativeAreaAllocator()->New<struct ScopeDebugInfo>();
    auto scopeDebugInfo = static_cast<struct ScopeDebugInfo *>(buffer);

    for (size_t i = 1; i < length; i += 2) {  // 2: Each literal buffer contains a pair of key-value.
        JSTaggedValue val = elementsLiteral->Get(i);
        ASSERT(val.IsString());
        CString name = ConvertToString(EcmaString::Cast(val.GetTaggedObject()));
        int32_t slot = elementsLiteral->Get(i + 1).GetInt();
        if (scopeDebugInfo == nullptr) {
            return JSTaggedValue::Hole();
        }
        scopeDebugInfo->scopeInfo.emplace(name, slot);
    }

    JSHandle<JSNativePointer> pointer = factory->NewJSNativePointer(
        buffer, NativeAreaAllocator::FreeObjectFunc<struct ScopeDebugInfo>, ecmaVm->GetNativeAreaAllocator());
    return pointer.GetTaggedValue();
}
}  // namespace panda::ecmascript
