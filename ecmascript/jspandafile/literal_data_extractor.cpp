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

#include "ecmascript/jspandafile/literal_data_extractor.h"

#include "ecmascript/base/string_helper.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/jspandafile/js_pandafile.h"
#include "ecmascript/module/js_module_manager.h"
#include "ecmascript/tagged_array-inl.h"

namespace panda::ecmascript {
using LiteralTag = panda_file::LiteralTag;
using StringData = panda_file::StringData;
using LiteralValue = panda_file::LiteralDataAccessor::LiteralValue;

void LiteralDataExtractor::ExtractObjectDatas(JSThread *thread, const JSPandaFile *jsPandaFile, size_t index,
                                              JSMutableHandle<TaggedArray> elements,
                                              JSMutableHandle<TaggedArray> properties,
                                              JSHandle<JSTaggedValue> constpool)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();

    LOG_ECMA(VERBOSE) << "Panda File" << jsPandaFile->GetJSPandaFileDesc();
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    panda_file::File::EntityId literalArraysId = pf->GetLiteralArraysId();
    panda_file::LiteralDataAccessor lda(*pf, literalArraysId);

    uint32_t num = lda.GetLiteralValsNum(index) / 2;  // 2: half
    elements.Update(factory->NewOldSpaceTaggedArray(num).GetTaggedValue());
    properties.Update(factory->NewOldSpaceTaggedArray(num).GetTaggedValue());
    uint32_t epos = 0;
    uint32_t ppos = 0;
    const uint8_t pairSize = 2;
    uint32_t methodId;
    FunctionKind kind;
    lda.EnumerateLiteralVals(
        index, [elements, properties, &epos, &ppos, factory, thread, jsPandaFile, pf, &methodId, &kind, &constpool]
        (const LiteralValue &value, const LiteralTag &tag) {
        JSTaggedValue jt = JSTaggedValue::Null();
        bool flag = false;
        switch (tag) {
            case LiteralTag::INTEGER: {
                jt = JSTaggedValue(std::get<uint32_t>(value));
                break;
            }
            case LiteralTag::DOUBLE: {
                jt = JSTaggedValue(std::get<double>(value));
                break;
            }
            case LiteralTag::BOOL: {
                jt = JSTaggedValue(std::get<bool>(value));
                break;
            }
            case LiteralTag::STRING: {
                StringData sd = pf->GetStringData(panda_file::File::EntityId(std::get<uint32_t>(value)));
                EcmaString *str = factory->GetRawStringFromStringTable(sd.data, sd.utf16_length, sd.is_ascii,
                                                                       MemSpaceType::OLD_SPACE);
                jt = JSTaggedValue(str);
                uint32_t index = 0;
                if (JSTaggedValue::ToElementIndex(jt, &index) && ppos % pairSize == 0) {
                    flag = true;
                }
                break;
            }
            case LiteralTag::METHOD: {
                methodId = std::get<uint32_t>(value);
                kind = FunctionKind::NORMAL_FUNCTION;
                break;
            }
            case LiteralTag::GENERATORMETHOD: {
                methodId = std::get<uint32_t>(value);
                kind = FunctionKind::GENERATOR_FUNCTION;
                break;
            }
            case LiteralTag::METHODAFFILIATE: {
                uint16_t length = std::get<uint16_t>(value);
                auto methodLiteral = jsPandaFile->FindMethodLiteral(methodId);
                ASSERT(methodLiteral != nullptr);

                JSHandle<Method> method = factory->NewMethod(methodLiteral);
                method->SetConstantPool(thread, constpool.GetTaggedValue());
                JSHandle<JSFunction> jsFunc = DefineMethodInLiteral(thread, jsPandaFile, method, kind, length);
                jt = jsFunc.GetTaggedValue();
                break;
            }
            case LiteralTag::ACCESSOR: {
                JSHandle<AccessorData> accessor = factory->NewAccessorData();
                jt = JSTaggedValue(accessor.GetTaggedValue());
                break;
            }
            case LiteralTag::NULLVALUE: {
                break;
            }
            default: {
                UNREACHABLE();
                break;
            }
        }
        if (tag != LiteralTag::METHOD && tag != LiteralTag::GENERATORMETHOD) {
            if (epos % pairSize == 0 && !flag) {
                properties->Set(thread, ppos++, jt);
            } else {
                elements->Set(thread, epos++, jt);
            }
        }
    });
}

JSHandle<TaggedArray> LiteralDataExtractor::GetDatasIgnoreTypeForClass(JSThread *thread,
    const JSPandaFile *jsPandaFile, size_t index, JSHandle<JSTaggedValue> constpool, const CString &entryPoint)
{
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    panda_file::File::EntityId literalArraysId = pf->GetLiteralArraysId();
    panda_file::LiteralDataAccessor lda(*pf, literalArraysId);
    uint32_t num = lda.GetLiteralValsNum(index) / 2;  // 2: half
    // The num is 1, indicating that the current class has no member variable.
    if (num == 1) {
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
        return factory->EmptyArray();
    }
    return EnumerateLiteralVals(thread, lda, jsPandaFile, index, constpool, entryPoint);
}

JSHandle<TaggedArray> LiteralDataExtractor::GetDatasIgnoreType(JSThread *thread, const JSPandaFile *jsPandaFile,
                                                               size_t index, JSHandle<JSTaggedValue> constpool,
                                                               const CString &entryPoint)
{
    LOG_ECMA(VERBOSE) << "Panda File" << jsPandaFile->GetJSPandaFileDesc();
    const panda_file::File *pf = jsPandaFile->GetPandaFile();
    panda_file::File::EntityId literalArraysId = pf->GetLiteralArraysId();
    panda_file::LiteralDataAccessor lda(*pf, literalArraysId);
    return EnumerateLiteralVals(thread, lda, jsPandaFile, index, constpool, entryPoint);
}

JSHandle<TaggedArray> LiteralDataExtractor::EnumerateLiteralVals(JSThread *thread, panda_file::LiteralDataAccessor &lda,
    const JSPandaFile *jsPandaFile, size_t index, JSHandle<JSTaggedValue> constpool, const CString &entryPoint)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    uint32_t num = lda.GetLiteralValsNum(index) / 2;  // 2: half
    JSHandle<TaggedArray> literals = factory->NewOldSpaceTaggedArray(num);
    uint32_t pos = 0;
    uint32_t methodId;
    FunctionKind kind;
    lda.EnumerateLiteralVals(
        index, [literals, &pos, factory, thread, jsPandaFile, &methodId, &kind, &constpool, &entryPoint]
        (const panda_file::LiteralDataAccessor::LiteralValue &value, const LiteralTag &tag) {
            JSTaggedValue jt = JSTaggedValue::Null();
            switch (tag) {
                case LiteralTag::INTEGER: {
                    jt = JSTaggedValue(std::get<uint32_t>(value));
                    break;
                }
                case LiteralTag::DOUBLE: {
                    jt = JSTaggedValue(std::get<double>(value));
                    break;
                }
                case LiteralTag::BOOL: {
                    jt = JSTaggedValue(std::get<bool>(value));
                    break;
                }
                case LiteralTag::STRING: {
                    const panda_file::File *pf = jsPandaFile->GetPandaFile();
                    StringData sd = pf->GetStringData(panda_file::File::EntityId(std::get<uint32_t>(value)));
                    EcmaString *str = factory->GetRawStringFromStringTable(sd.data, sd.utf16_length, sd.is_ascii,
                                                                           MemSpaceType::OLD_SPACE);
                    jt = JSTaggedValue(str);
                    break;
                }
                case LiteralTag::METHOD: {
                    methodId = std::get<uint32_t>(value);
                    kind = FunctionKind::NORMAL_FUNCTION;
                    break;
                }
                case LiteralTag::GENERATORMETHOD: {
                    methodId = std::get<uint32_t>(value);
                    kind = FunctionKind::GENERATOR_FUNCTION;
                    break;
                }
                case LiteralTag::METHODAFFILIATE: {
                    uint16_t length = std::get<uint16_t>(value);
                    auto methodLiteral = jsPandaFile->FindMethodLiteral(methodId);
                    ASSERT(methodLiteral != nullptr);

                    JSHandle<Method> method = factory->NewMethod(methodLiteral);
                    method->SetConstantPool(thread, constpool.GetTaggedValue());
                    JSHandle<JSFunction> jsFunc =
                        DefineMethodInLiteral(thread, jsPandaFile, method, kind, length, entryPoint);
                    jt = jsFunc.GetTaggedValue();
                    break;
                }
                case LiteralTag::ACCESSOR: {
                    JSHandle<AccessorData> accessor = factory->NewAccessorData();
                    jt = accessor.GetTaggedValue();
                    break;
                }
                case LiteralTag::NULLVALUE: {
                    break;
                }
                default: {
                    UNREACHABLE();
                    break;
                }
            }
            if (tag != LiteralTag::METHOD && tag != LiteralTag::GENERATORMETHOD) {
                literals->Set(thread, pos++, jt);
            } else {
                uint32_t oldLength = literals->GetLength();
                literals->Trim(thread, oldLength - 1);
            }
        });
    return literals;
}

JSHandle<JSFunction> LiteralDataExtractor::DefineMethodInLiteral(JSThread *thread, const JSPandaFile *jsPandaFile,
                                                                 JSHandle<Method> method,
                                                                 FunctionKind kind, uint16_t length,
                                                                 const CString &entryPoint)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<JSHClass> functionClass;
    if (kind == FunctionKind::NORMAL_FUNCTION) {
        functionClass = JSHandle<JSHClass>::Cast(env->GetFunctionClassWithoutProto());
    } else {
        functionClass = JSHandle<JSHClass>::Cast(env->GetGeneratorFunctionClass());
    }
    JSHandle<JSFunction> jsFunc =
        factory->NewJSFunctionByDynClass(method, functionClass, kind, MemSpaceType::OLD_SPACE);

    if (kind == FunctionKind::GENERATOR_FUNCTION) {
        JSHandle<JSFunction> objFun(env->GetObjectFunction());
        JSHandle<JSObject> initialGeneratorFuncPrototype = factory->NewJSObjectByConstructor(objFun);
        JSObject::SetPrototype(thread, initialGeneratorFuncPrototype, env->GetGeneratorPrototype());
        jsFunc->SetProtoOrDynClass(thread, initialGeneratorFuncPrototype);
    }
    jsFunc->SetPropertyInlinedProps(thread, JSFunction::LENGTH_INLINE_PROPERTY_INDEX, JSTaggedValue(length));
    if (jsPandaFile->IsModule()) {
        EcmaVM *vm = thread->GetEcmaVM();
        CString moduleName = jsPandaFile->GetJSPandaFileDesc();
        if (!entryPoint.empty()) {
            moduleName = entryPoint;
        }
        JSHandle<SourceTextModule> module = vm->GetModuleManager()->HostGetImportedModule(moduleName);
        jsFunc->SetModule(thread, module.GetTaggedValue());
    }
    return jsFunc;
}
}  // namespace panda::ecmascript
