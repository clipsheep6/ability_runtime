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

#include "ecmascript/stubs/runtime_stubs-inl.h"
#include "ecmascript/accessor_data.h"
#include "ecmascript/base/number_helper.h"
#include "ecmascript/base/string_helper.h"
#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/compiler/rt_call_signature.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/frames.h"
#include "ecmascript/global_env.h"
#include "ecmascript/ic/ic_runtime.h"
#include "ecmascript/ic/profile_type_info.h"
#include "ecmascript/ic/properties_cache.h"
#include "ecmascript/interpreter/interpreter-inl.h"
#include "ecmascript/interpreter/interpreter_assembly.h"
#include "ecmascript/js_api/js_api_arraylist.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_object.h"
#include "ecmascript/js_proxy.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/jspandafile/program_object.h"
#include "ecmascript/layout_info.h"
#include "ecmascript/mem/space-inl.h"
#include "ecmascript/message_string.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_dictionary.h"
#include "ecmascript/ts_types/ts_manager.h"

namespace panda::ecmascript {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#define DEF_RUNTIME_STUBS(name) \
    JSTaggedType RuntimeStubs::name(uintptr_t argGlue, uint32_t argc, uintptr_t argv)

#define RUNTIME_STUBS_HEADER(name)                        \
    auto thread = JSThread::GlueToJSThread(argGlue);      \
    RUNTIME_TRACE(thread, name);                          \
    [[maybe_unused]] EcmaHandleScope handleScope(thread)  \

#define GET_ASM_FRAME(CurrentSp) \
    (reinterpret_cast<AsmInterpretedFrame *>(CurrentSp) - 1) // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

DEF_RUNTIME_STUBS(AddElementInternal)
{
    RUNTIME_STUBS_HEADER(AddElementInternal);
    JSHandle<JSObject> receiver = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue argIndex = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue argAttr = GetArg(argv, argc, 3);  // 3: means the third parameter
    auto attr = static_cast<PropertyAttributes>(argAttr.GetInt());
    auto result = JSObject::AddElementInternal(thread, receiver, argIndex.GetInt(), value, attr);
    return JSTaggedValue(result).GetRawData();
}

DEF_RUNTIME_STUBS(AllocateInYoung)
{
    RUNTIME_STUBS_HEADER(AllocateInYoung);
    JSTaggedValue allocateSize = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    auto size = static_cast<size_t>(allocateSize.GetInt());
    auto heap = const_cast<Heap*>(thread->GetEcmaVM()->GetHeap());
    auto space = heap->GetNewSpace();
    ASSERT(size <= MAX_REGULAR_HEAP_OBJECT_SIZE);
    auto result = reinterpret_cast<TaggedObject *>(space->Allocate(size));
    if (result == nullptr) {
        result = heap->AllocateYoungOrHugeObject(size);
        ASSERT(result != nullptr);
    }
    return JSTaggedValue(result).GetRawData();
}

DEF_RUNTIME_STUBS(CallInternalGetter)
{
    RUNTIME_STUBS_HEADER(CallInternalGetter);
    JSTaggedType argAccessor = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSObject> argReceiver = GetHArg<JSObject>(argv, argc, 1);  // 1: means the first parameter

    auto accessor = AccessorData::Cast(reinterpret_cast<TaggedObject *>(argAccessor));
    return accessor->CallInternalGet(thread, argReceiver).GetRawData();
}

DEF_RUNTIME_STUBS(CallInternalSetter)
{
    RUNTIME_STUBS_HEADER(CallInternalSetter);
    JSHandle<JSObject> receiver = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedType argSetter = GetTArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);
    auto setter = AccessorData::Cast((reinterpret_cast<TaggedObject *>(argSetter)));
    auto result = setter->CallInternalSet(thread, receiver, value, true);
    if (!result) {
        return JSTaggedValue::Exception().GetRawData();
    }
    return JSTaggedValue::Undefined().GetRawData();
}

DEF_RUNTIME_STUBS(JSProxySetProperty)
{
    RUNTIME_STUBS_HEADER(JSProxySetProperty);
    JSHandle<JSProxy> proxy = GetHArg<JSProxy>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> index = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);   // 2: means the second parameter
    JSHandle<JSTaggedValue> receiver = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue argMayThrow = GetArg(argv, argc, 4);   // 4: means the fourth parameter
    auto result = JSProxy::SetProperty(thread, proxy, index, value, receiver, argMayThrow.IsTrue());
    return JSTaggedValue(result).GetRawData();
}

DEF_RUNTIME_STUBS(GetHash32)
{
    JSTaggedValue argKey = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue len = GetArg(argv, argc, 1);  // 1: means the first parameter
    int key = argKey.GetInt();
    auto pkey = reinterpret_cast<uint8_t *>(&key);
    uint32_t result = panda::GetHash32(pkey, len.GetInt());
    return JSTaggedValue(static_cast<uint64_t>(result)).GetRawData();
}

DEF_RUNTIME_STUBS(ComputeHashcode)
{
    JSTaggedType ecmaString = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    auto string = reinterpret_cast<EcmaString *>(ecmaString);
    uint32_t result = string->ComputeHashcode(0);
    return JSTaggedValue(static_cast<uint64_t>(result)).GetRawData();
}

void RuntimeStubs::PrintHeapReginInfo(uintptr_t argGlue)
{
    auto thread = JSThread::GlueToJSThread(argGlue);
    thread->GetEcmaVM()->GetHeap()->GetNewSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "semispace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetOldSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetOldSpace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetNonMovableSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetNonMovableSpace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetMachineCodeSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetMachineCodeSpace region: " << current << std::endl;
    });
}

DEF_RUNTIME_STUBS(GetTaggedArrayPtrTest)
{
    RUNTIME_STUBS_HEADER(GetTaggedArrayPtrTest);
    // this case static static JSHandle<TaggedArray> arr don't free in first call
    // second call trigger gc.
    // don't call EcmaHandleScope handleScope(thread);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSTaggedType array = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    bool allocated = false;
    if (array == JSTaggedValue::VALUE_UNDEFINED) {
        // 2 : means construct 2 elements size taggedArray
        JSHandle<TaggedArray> arr = factory->NewTaggedArray(2);
        arr->Set(thread, 0, JSTaggedValue(3.5)); // 3.5: first element
        arr->Set(thread, 1, JSTaggedValue(4.5)); // 4.5: second element
        array = arr.GetTaggedValue().GetRawData();
        allocated = true;
    }
    JSHandle<TaggedArray> arr1(thread, JSTaggedValue(array));
#ifndef NDEBUG
    PrintHeapReginInfo(argGlue);
#endif
    if (!allocated) {
        thread->GetEcmaVM()->CollectGarbage(TriggerGCType::FULL_GC);
    }
    LOG_ECMA(INFO) << " arr->GetData() " << std::hex << "  " << arr1->GetData();
    return arr1.GetTaggedValue().GetRawData();
}

DEF_RUNTIME_STUBS(NewInternalString)
{
    RUNTIME_STUBS_HEADER(NewInternalString);
    JSHandle<JSTaggedValue> keyHandle = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return JSTaggedValue(thread->GetEcmaVM()->GetFactory()->InternString(keyHandle)).GetRawData();
}

DEF_RUNTIME_STUBS(NewTaggedArray)
{
    RUNTIME_STUBS_HEADER(NewTaggedArray);
    JSTaggedValue length = GetArg(argv, argc, 0);  // 0: means the zeroth parameter

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    return factory->NewTaggedArray(length.GetInt()).GetTaggedValue().GetRawData();
}

DEF_RUNTIME_STUBS(CopyArray)
{
    RUNTIME_STUBS_HEADER(CopyArray);
    JSHandle<TaggedArray> array = GetHArg<TaggedArray>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue length = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue capacity = GetArg(argv, argc, 2);  // 2: means the second parameter

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    return factory->CopyArray(array, length.GetInt(), capacity.GetInt()).GetTaggedValue().GetRawData();
}

DEF_RUNTIME_STUBS(NameDictPutIfAbsent)
{
    RUNTIME_STUBS_HEADER(NameDictPutIfAbsent);
    JSTaggedType receiver = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedType array = GetTArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> keyHandle = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> valueHandle = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue attr = GetArg(argv, argc, 4);   // 4: means the fourth parameter
    JSTaggedValue needTransToDict = GetArg(argv, argc, 5);  // 5: means the fifth parameter

    PropertyAttributes propAttr(attr.GetInt());
    if (needTransToDict.IsTrue()) {
        JSHandle<JSObject> objHandle(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(receiver)));
        JSHandle<NameDictionary> dictHandle(JSObject::TransitionToDictionary(thread, objHandle));
        return NameDictionary::
            PutIfAbsent(thread, dictHandle, keyHandle, valueHandle, propAttr).GetTaggedValue().GetRawData();
    } else {
        JSHandle<NameDictionary> dictHandle(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(array)));
        return NameDictionary::
            PutIfAbsent(thread, dictHandle, keyHandle, valueHandle, propAttr).GetTaggedValue().GetRawData();
    }
}

DEF_RUNTIME_STUBS(PropertiesSetValue)
{
    RUNTIME_STUBS_HEADER(PropertiesSetValue);
    JSHandle<JSObject> objHandle = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> valueHandle = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<TaggedArray> arrayHandle = GetHArg<TaggedArray>(argv, argc, 2);   // 2: means the second parameter
    JSTaggedValue taggedCapacity = GetArg(argv, argc, 3);
    JSTaggedValue taggedIndex = GetArg(argv, argc, 4);
    int capacity = taggedCapacity.GetInt();
    int index = taggedIndex.GetInt();

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> properties;
    if (capacity == 0) {
        properties = factory->NewTaggedArray(JSObject::MIN_PROPERTIES_LENGTH);
    } else {
        properties = factory->CopyArray(arrayHandle, capacity, JSObject::ComputePropertyCapacity(capacity));
    }
    properties->Set(thread, index, valueHandle);
    objHandle->SetProperties(thread, properties);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(TaggedArraySetValue)
{
    RUNTIME_STUBS_HEADER(TaggedArraySetValue);
    JSTaggedType argReceiver = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue value = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedType argElement = GetTArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue taggedElementIndex = GetArg(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue taggedCapacity = GetArg(argv, argc, 4);  // 4: means the fourth parameter

    int elementIndex = taggedElementIndex.GetInt();
    int capacity = taggedCapacity.GetInt();
    auto elements = reinterpret_cast<TaggedArray *>(argElement);
    if (elementIndex >= capacity) {
        if (JSObject::ShouldTransToDict(capacity, elementIndex)) {
            return JSTaggedValue::Hole().GetRawData();
        }
        JSHandle<JSObject> receiverHandle(thread, reinterpret_cast<JSObject *>(argReceiver));
        JSHandle<JSTaggedValue> valueHandle(thread, value);
        elements = *JSObject::GrowElementsCapacity(thread, receiverHandle,
                                                   JSObject::ComputeElementCapacity(elementIndex + 1));
        receiverHandle->SetElements(thread, JSTaggedValue(elements));
        elements->Set(thread, elementIndex, valueHandle);
        return JSTaggedValue::Undefined().GetRawData();
    }
    elements->Set(thread, elementIndex, value);
    return JSTaggedValue::Undefined().GetRawData();
}

DEF_RUNTIME_STUBS(NewEcmaDynClass)
{
    RUNTIME_STUBS_HEADER(NewEcmaDynClass);
    JSTaggedValue size = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue type = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue inlinedProps = GetArg(argv, argc, 2);  // 2: means the second parameter
    return (thread->GetEcmaVM()->GetFactory()->NewEcmaDynClass(
        size.GetInt(), JSType(type.GetInt()), inlinedProps.GetInt())).GetTaggedValue().GetRawData();
}

DEF_RUNTIME_STUBS(UpdateLayOutAndAddTransition)
{
    RUNTIME_STUBS_HEADER(UpdateLayOutAndAddTransition);
    JSHandle<JSHClass> oldHClassHandle = GetHArg<JSHClass>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSHClass> newHClassHandle = GetHArg<JSHClass>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> keyHandle = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue attr = GetArg(argv, argc, 3);  // 3: means the third parameter

    auto factory = thread->GetEcmaVM()->GetFactory();
    PropertyAttributes attrValue(attr.GetInt());
    uint32_t offset = attrValue.GetOffset();
    newHClassHandle->IncNumberOfProps();

    {
        JSMutableHandle<LayoutInfo> layoutInfoHandle(thread, newHClassHandle->GetLayout());

        if (layoutInfoHandle->NumberOfElements() != static_cast<int>(offset)) {
            layoutInfoHandle.Update(factory->CopyAndReSort(layoutInfoHandle, offset, offset + 1));
            newHClassHandle->SetLayout(thread, layoutInfoHandle);
        } else if (layoutInfoHandle->GetPropertiesCapacity() <= static_cast<int>(offset)) {  // need to Grow
            layoutInfoHandle.Update(
                factory->ExtendLayoutInfo(layoutInfoHandle, LayoutInfo::ComputeGrowCapacity(offset)));
            newHClassHandle->SetLayout(thread, layoutInfoHandle);
        }
        layoutInfoHandle->AddKey(thread, offset, keyHandle.GetTaggedValue(), attrValue);
    }

    // 5. Add newDynclass to old dynclass's transitions.
    JSHClass::AddTransitions(thread, oldHClassHandle, newHClassHandle, keyHandle, attrValue);
    return JSTaggedValue::Hole().GetRawData();
}

void RuntimeStubs::DebugPrint(int fmtMessageId, ...)
{
    std::string format = MessageString::GetMessageString(fmtMessageId);
    va_list args;
    va_start(args, fmtMessageId);
    std::string result = base::StringHelper::Vformat(format.c_str(), args);
    std::cerr << result << std::endl;
    va_end(args);
}

void RuntimeStubs::FatalPrint(int fmtMessageId, ...)
{
    std::string format = MessageString::GetMessageString(fmtMessageId);
    va_list args;
    va_start(args, fmtMessageId);
    std::string result = base::StringHelper::Vformat(format.c_str(), args);
    std::cerr << result << std::endl;
    va_end(args);
    UNREACHABLE();
}

DEF_RUNTIME_STUBS(NoticeThroughChainAndRefreshUser)
{
    RUNTIME_STUBS_HEADER(NoticeThroughChainAndRefreshUser);
    JSHandle<JSHClass> oldHClassHandle = GetHArg<JSHClass>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSHClass> newHClassHandle = GetHArg<JSHClass>(argv, argc, 1);  // 1: means the first parameter

    JSHClass::NoticeThroughChain(thread, oldHClassHandle);
    JSHClass::RefreshUsers(thread, oldHClassHandle, newHClassHandle);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(IncDyn)
{
    RUNTIME_STUBS_HEADER(IncDyn);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeIncDyn(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(DecDyn)
{
    RUNTIME_STUBS_HEADER(DecDyn);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDecDyn(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(ExpDyn)
{
    RUNTIME_STUBS_HEADER(ExpDyn);
    JSTaggedValue baseValue = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue exponentValue = GetArg(argv, argc, 1);  // 1: means the first parameter

    if (baseValue.IsNumber() && exponentValue.IsNumber()) {
        // fast path
        double doubleBase = baseValue.IsInt() ? baseValue.GetInt() : baseValue.GetDouble();
        double doubleExponent = exponentValue.IsInt() ? exponentValue.GetInt() : exponentValue.GetDouble();
        if (std::abs(doubleBase) == 1 && std::isinf(doubleExponent)) {
            return JSTaggedValue(base::NAN_VALUE).GetRawData();
        }
        if ((doubleBase == 0 &&
            ((bit_cast<uint64_t>(doubleBase)) & base::DOUBLE_SIGN_MASK) == base::DOUBLE_SIGN_MASK) &&
            std::isfinite(doubleExponent) && base::NumberHelper::TruncateDouble(doubleExponent) == doubleExponent &&
            base::NumberHelper::TruncateDouble(doubleExponent / 2) + base::HALF ==  // 2 : half
            (doubleExponent / 2)) {  // 2 : half
            if (doubleExponent > 0) {
                return JSTaggedValue(-0.0).GetRawData();
            }
            if (doubleExponent < 0) {
                return JSTaggedValue(-base::POSITIVE_INFINITY).GetRawData();
            }
        }
        return JSTaggedValue(std::pow(doubleBase, doubleExponent)).GetRawData();
    }
    // Slow path
    JSTaggedValue res = RuntimeExpDyn(thread, baseValue, exponentValue);
    return res.GetRawData();
}

DEF_RUNTIME_STUBS(IsInDyn)
{
    RUNTIME_STUBS_HEADER(IsInDyn);
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeIsInDyn(thread, prop, obj).GetRawData();
}

DEF_RUNTIME_STUBS(InstanceOfDyn)
{
    RUNTIME_STUBS_HEADER(InstanceOfDyn);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> target = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeInstanceofDyn(thread, obj, target).GetRawData();
}

DEF_RUNTIME_STUBS(FastStrictNotEqual)
{
    RUNTIME_STUBS_HEADER(FastStrictNotEqual);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter
    bool result = FastRuntimeStub::FastStrictEqual(left, right);
    if (result) {
        return JSTaggedValue::False().GetRawData();
    }
    return JSTaggedValue::True().GetRawData();
}

DEF_RUNTIME_STUBS(FastStrictEqual)
{
    RUNTIME_STUBS_HEADER(FastStrictEqual);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter
    bool result = FastRuntimeStub::FastStrictEqual(left, right);
    if (result) {
        return JSTaggedValue::True().GetRawData();
    }
    return JSTaggedValue::False().GetRawData();
}

DEF_RUNTIME_STUBS(CreateGeneratorObj)
{
    RUNTIME_STUBS_HEADER(CreateGeneratorObj);
    JSHandle<JSTaggedValue> genFunc = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeCreateGeneratorObj(thread, genFunc).GetRawData();
}

DEF_RUNTIME_STUBS(CreateAsyncGeneratorObj)
{
    RUNTIME_STUBS_HEADER(CreateAsyncGeneratorObj);
    JSHandle<JSTaggedValue> genFunc = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeCreateAsyncGeneratorObj(thread, genFunc).GetRawData();
}

DEF_RUNTIME_STUBS(GetTemplateObject)
{
    RUNTIME_STUBS_HEADER(GetTemplateObject);
    JSHandle<JSTaggedValue> literal = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeGetTemplateObject(thread, literal).GetRawData();
}

DEF_RUNTIME_STUBS(GetNextPropName)
{
    RUNTIME_STUBS_HEADER(GetNextPropName);
    JSHandle<JSTaggedValue> iter = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeGetNextPropName(thread, iter).GetRawData();
}

DEF_RUNTIME_STUBS(IterNext)
{
    RUNTIME_STUBS_HEADER(IterNext);
    JSHandle<JSTaggedValue> iter = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeIterNext(thread, iter).GetRawData();
}

DEF_RUNTIME_STUBS(CloseIterator)
{
    RUNTIME_STUBS_HEADER(CloseIterator);
    JSHandle<JSTaggedValue> iter = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeCloseIterator(thread, iter).GetRawData();
}

DEF_RUNTIME_STUBS(CopyModule)
{
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(SuperCallSpread)
{
    RUNTIME_STUBS_HEADER(SuperCallSpread);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> array = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    auto sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    JSTaggedValue function = InterpreterAssembly::GetNewTarget(sp);
    return RuntimeSuperCallSpread(thread, func, JSHandle<JSTaggedValue>(thread, function), array).GetRawData();
}

DEF_RUNTIME_STUBS(OptSuperCallSpread)
{
    RUNTIME_STUBS_HEADER(OptSuperCallSpread);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);
    JSHandle<JSTaggedValue> newTarget = GetHArg<JSTaggedValue>(argv, argc, 1);
    JSHandle<JSTaggedValue> array = GetHArg<JSTaggedValue>(argv, argc, 2);
    return RuntimeSuperCallSpread(thread, func, newTarget, array).GetRawData();
}

DEF_RUNTIME_STUBS(DelObjProp)
{
    RUNTIME_STUBS_HEADER(DelObjProp);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeDelObjProp(thread, obj, prop).GetRawData();
}

DEF_RUNTIME_STUBS(NewObjSpreadDyn)
{
    RUNTIME_STUBS_HEADER(NewObjSpreadDyn);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> newTarget = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> array = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeNewObjSpreadDyn(thread, func, newTarget, array).GetRawData();
}

DEF_RUNTIME_STUBS(CreateIterResultObj)
{
    RUNTIME_STUBS_HEADER(CreateIterResultObj);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue flag = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeCreateIterResultObj(thread, value, flag).GetRawData();
}

DEF_RUNTIME_STUBS(AsyncFunctionAwaitUncaught)
{
    RUNTIME_STUBS_HEADER(AsyncFunctionAwaitUncaught);
    JSHandle<JSTaggedValue> asyncFuncObj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeAsyncFunctionAwaitUncaught(thread, asyncFuncObj, value).GetRawData();
}

DEF_RUNTIME_STUBS(AsyncFunctionResolveOrReject)
{
    RUNTIME_STUBS_HEADER(AsyncFunctionResolveOrReject);
    JSHandle<JSTaggedValue> asyncFuncObj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue is_resolve = GetArg(argv, argc, 2);  // 2: means the second parameter
    return RuntimeAsyncFunctionResolveOrReject(thread, asyncFuncObj, value, is_resolve.IsTrue()).GetRawData();
}

DEF_RUNTIME_STUBS(AsyncGeneratorResolve)
{
    RUNTIME_STUBS_HEADER(AsyncGeneratorResolve);
    JSHandle<JSTaggedValue> asyncGenerator = GetHArg<JSTaggedValue>(argv, argc, 0); // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1); // 1: means the first parameter
    JSTaggedValue flag = GetArg(argv, argc, 2); // 2: means the second parameter
    return RuntimeAsyncGeneratorResolve(thread, asyncGenerator, value, flag).GetRawData();
}

DEF_RUNTIME_STUBS(CopyDataProperties)
{
    RUNTIME_STUBS_HEADER(CopyDataProperties);
    JSHandle<JSTaggedValue> dst = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> src = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeCopyDataProperties(thread, dst, src).GetRawData();
}

DEF_RUNTIME_STUBS(StArraySpread)
{
    RUNTIME_STUBS_HEADER(StArraySpread);
    JSHandle<JSTaggedValue> dst = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue index = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> src = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStArraySpread(thread, dst, index, src).GetRawData();
}

DEF_RUNTIME_STUBS(GetIteratorNext)
{
    RUNTIME_STUBS_HEADER(GetIteratorNext);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> method = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeGetIteratorNext(thread, obj, method).GetRawData();
}

DEF_RUNTIME_STUBS(SetObjectWithProto)
{
    RUNTIME_STUBS_HEADER(SetObjectWithProto);
    JSHandle<JSTaggedValue> proto = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSObject> obj = GetHArg<JSObject>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeSetObjectWithProto(thread, proto, obj).GetRawData();
}

DEF_RUNTIME_STUBS(LoadICByValue)
{
    RUNTIME_STUBS_HEADER(LoadICByValue);
    JSHandle<JSTaggedValue> profileTypeInfo = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> receiver = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> propKey = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue slotId = GetArg(argv, argc, 3);  // 3: means the third parameter

    if (profileTypeInfo->IsUndefined()) {
        return RuntimeLdObjByValue(thread, receiver, propKey, false, JSTaggedValue::Undefined()).GetRawData();
    }
    LoadICRuntime icRuntime(thread, JSHandle<ProfileTypeInfo>::Cast(profileTypeInfo), slotId.GetInt(), ICKind::LoadIC);
    return icRuntime.LoadMiss(receiver, propKey).GetRawData();
}

DEF_RUNTIME_STUBS(StoreICByValue)
{
    RUNTIME_STUBS_HEADER(StoreICByValue);
    JSHandle<JSTaggedValue> profileTypeInfo = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> receiver = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> propKey = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue slotId = GetArg(argv, argc, 4);   // 4: means the fourth parameter

    if (profileTypeInfo->IsUndefined()) {
        return RuntimeStObjByValue(thread, receiver, propKey, value).GetRawData();
    }
    StoreICRuntime icRuntime(thread, JSHandle<ProfileTypeInfo>::Cast(profileTypeInfo), slotId.GetInt(),
                             ICKind::StoreIC);
    return icRuntime.StoreMiss(receiver, propKey, value).GetRawData();
}

DEF_RUNTIME_STUBS(StOwnByValue)
{
    RUNTIME_STUBS_HEADER(StOwnByValue);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> key = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter

    return RuntimeStOwnByValue(thread, obj, key, value).GetRawData();
}

DEF_RUNTIME_STUBS(LdSuperByValue)
{
    RUNTIME_STUBS_HEADER(LdSuperByValue);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> key = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    auto sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    JSTaggedValue thisFunc = InterpreterAssembly::GetThisFunction(sp);
    return RuntimeLdSuperByValue(thread, obj, key, thisFunc).GetRawData();
}

DEF_RUNTIME_STUBS(OptLdSuperByValue)
{
    RUNTIME_STUBS_HEADER(OptLdSuperByValue);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> key = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue thisFunc = GetArg(argv, argc, 2);  // 2: means the second parameter
    return RuntimeLdSuperByValue(thread, obj, key, thisFunc).GetRawData();
}

DEF_RUNTIME_STUBS(StSuperByValue)
{
    RUNTIME_STUBS_HEADER(StSuperByValue);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> key = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    auto sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    JSTaggedValue thisFunc = InterpreterAssembly::GetThisFunction(sp);
    return RuntimeStSuperByValue(thread, obj, key, value, thisFunc).GetRawData();
}

DEF_RUNTIME_STUBS(OptStSuperByValue)
{
    RUNTIME_STUBS_HEADER(OptStSuperByValue);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> key = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue thisFunc = GetArg(argv, argc, 3);  // 3: means the third parameter
    return RuntimeStSuperByValue(thread, obj, key, value, thisFunc).GetRawData();
}

DEF_RUNTIME_STUBS(LdObjByIndex)
{
    RUNTIME_STUBS_HEADER(LdObjByIndex);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue idx = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue callGetter = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue receiver = GetArg(argv, argc, 3);  // 3: means the third parameter
    return RuntimeLdObjByIndex(thread, obj, idx.GetInt(), callGetter.IsTrue(), receiver).GetRawData();
}

DEF_RUNTIME_STUBS(StObjByIndex)
{
    RUNTIME_STUBS_HEADER(StObjByIndex);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue idx = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStObjByIndex(thread, obj, idx.GetInt(), value).GetRawData();
}

DEF_RUNTIME_STUBS(StOwnByIndex)
{
    RUNTIME_STUBS_HEADER(StOwnByIndex);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> idx = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStOwnByIndex(thread, obj, idx, value).GetRawData();
}

DEF_RUNTIME_STUBS(StGlobalRecord)
{
    RUNTIME_STUBS_HEADER(StGlobalRecord);
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue isConst = GetArg(argv, argc, 2);
    return RuntimeStGlobalRecord(thread, prop, value, isConst.IsTrue()).GetRawData();
}

DEF_RUNTIME_STUBS(NegDyn)
{
    RUNTIME_STUBS_HEADER(NegDyn);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeNegDyn(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(NotDyn)
{
    RUNTIME_STUBS_HEADER(NotDyn);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeNotDyn(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(Shl2Dyn)
{
    RUNTIME_STUBS_HEADER(Shl2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::Shl2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(Shr2Dyn)
{
    RUNTIME_STUBS_HEADER(Shr2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::Shr2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(Ashr2Dyn)
{
    RUNTIME_STUBS_HEADER(Ashr2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::Ashr2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(And2Dyn)
{
    RUNTIME_STUBS_HEADER(And2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::And2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(Xor2Dyn)
{
    RUNTIME_STUBS_HEADER(Xor2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::Xor2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(Or2Dyn)
{
    RUNTIME_STUBS_HEADER(Or2Dyn);
    JSTaggedValue left = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue right = GetArg(argv, argc, 1);  // 1: means the first parameter

    auto res = SlowRuntimeStub::Or2Dyn(thread, left, right);
    return JSTaggedValue(res).GetRawData();
}

DEF_RUNTIME_STUBS(ResolveClass)
{
    RUNTIME_STUBS_HEADER(ResolveClass);
    JSHandle<JSFunction> ctor = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<TaggedArray> literal = GetHArg<TaggedArray>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> base = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> lexenv = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    return RuntimeResolveClass(thread, ctor, literal, base, lexenv).GetRawData();
}

DEF_RUNTIME_STUBS(CloneClassFromTemplate)
{
    RUNTIME_STUBS_HEADER(CloneClassFromTemplate);
    JSHandle<JSFunction> ctor = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> base = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> lexenv = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeCloneClassFromTemplate(thread, ctor, base, lexenv).GetRawData();
}

DEF_RUNTIME_STUBS(CreateClassWithBuffer)
{
    RUNTIME_STUBS_HEADER(CreateClassWithBuffer);
    JSHandle<JSTaggedValue> base = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> lexenv = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> constpool = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue methodId = GetArg(argv, argc, 3);  // 3: means the third parameter
    return RuntimeCreateClassWithBuffer(thread, base, lexenv, constpool,
                                        static_cast<uint16_t>(methodId.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(SetClassConstructorLength)
{
    RUNTIME_STUBS_HEADER(SetClassConstructorLength);
    JSTaggedValue ctor = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue length = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeSetClassConstructorLength(thread, ctor, length).GetRawData();
}


DEF_RUNTIME_STUBS(UpdateHotnessCounter)
{
    RUNTIME_STUBS_HEADER(UpdateHotnessCounter);
    JSHandle<JSFunction> thisFunc = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    thread->CheckSafepoint();
    auto profileTypeInfo = thisFunc->GetProfileTypeInfo();
    if (profileTypeInfo == JSTaggedValue::Undefined()) {
        uint32_t slotSize = thisFunc->GetCallTarget()->GetSlotSize();
        auto res = RuntimeNotifyInlineCache(thread, thisFunc, slotSize);
        return res.GetRawData();
    }
    return profileTypeInfo.GetRawData();
}

DEF_RUNTIME_STUBS(LoadICByName)
{
    RUNTIME_STUBS_HEADER(LoadICByName);
    JSHandle<JSTaggedValue> profileHandle = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> receiverHandle = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> keyHandle = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue slotId = GetArg(argv, argc, 3);  // 3: means the third parameter

    if (profileHandle->IsUndefined()) {
        auto res = JSTaggedValue::GetProperty(thread, receiverHandle, keyHandle).GetValue().GetTaggedValue();
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception().GetRawData());
        return res.GetRawData();
    }
    LoadICRuntime icRuntime(
        thread, JSHandle<ProfileTypeInfo>::Cast(profileHandle), slotId.GetInt(), ICKind::NamedLoadIC);
    return icRuntime.LoadMiss(receiverHandle, keyHandle).GetRawData();
}

DEF_RUNTIME_STUBS(StoreICByName)
{
    RUNTIME_STUBS_HEADER(StoreICByName);
    JSHandle<JSTaggedValue> profileHandle = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> receiverHandle = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> keyHandle = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> valueHandle = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue slotId = GetArg(argv, argc, 4);   // 4: means the fourth parameter

    if (profileHandle->IsUndefined()) {
        JSTaggedValue::SetProperty(thread, receiverHandle, keyHandle, valueHandle, true);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception().GetRawData());
        return JSTaggedValue::True().GetRawData();
    }
    StoreICRuntime icRuntime(
        thread, JSHandle<ProfileTypeInfo>::Cast(profileHandle), slotId.GetInt(), ICKind::NamedStoreIC);
    return icRuntime.StoreMiss(receiverHandle, keyHandle, valueHandle).GetRawData();
}

DEF_RUNTIME_STUBS(SetFunctionNameNoPrefix)
{
    RUNTIME_STUBS_HEADER(SetFunctionNameNoPrefix);
    JSTaggedType argFunc = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue argName = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSFunction::SetFunctionNameNoPrefix(thread, reinterpret_cast<JSFunction *>(argFunc), argName);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(StOwnByValueWithNameSet)
{
    RUNTIME_STUBS_HEADER(StOwnByValueWithNameSet);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStOwnByValueWithNameSet(thread, obj, prop, value).GetRawData();
}

DEF_RUNTIME_STUBS(StOwnByName)
{
    RUNTIME_STUBS_HEADER(StOwnByName);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStOwnByName(thread, obj, prop, value).GetRawData();
}

DEF_RUNTIME_STUBS(StOwnByNameWithNameSet)
{
    RUNTIME_STUBS_HEADER(StOwnByNameWithNameSet);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeStOwnByValueWithNameSet(thread, obj, prop, value).GetRawData();
}

DEF_RUNTIME_STUBS(SuspendGenerator)
{
    RUNTIME_STUBS_HEADER(SuspendGenerator);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeSuspendGenerator(thread, obj, value).GetRawData();
}

DEF_RUNTIME_STUBS(OptSuspendGenerator)
{
    RUNTIME_STUBS_HEADER(OptSuspendGenerator);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeOptSuspendGenerator(thread, obj, value).GetRawData();
}

DEF_RUNTIME_STUBS(UpFrame)
{
    RUNTIME_STUBS_HEADER(UpFrame);
    FrameHandler frameHandler(thread);
    uint32_t pcOffset = panda_file::INVALID_OFFSET;
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        if (frameHandler.IsEntryFrame() || frameHandler.IsBuiltinFrame()) {
            thread->SetCurrentFrame(frameHandler.GetSp());
            thread->SetLastFp(frameHandler.GetFp());
            return JSTaggedValue(static_cast<uint64_t>(0)).GetRawData();
        }
        auto method = frameHandler.GetMethod();
        pcOffset = InterpreterAssembly::FindCatchBlock(method, frameHandler.GetBytecodeOffset());
        if (pcOffset != panda_file::INVALID_OFFSET) {
            thread->SetCurrentFrame(frameHandler.GetSp());
            thread->SetLastFp(frameHandler.GetFp());
            uintptr_t pc = reinterpret_cast<uintptr_t>(method->GetBytecodeArray() + pcOffset);
            return JSTaggedValue(static_cast<uint64_t>(pc)).GetRawData();
        }
    }
    return JSTaggedValue(static_cast<uint64_t>(0)).GetRawData();
}

DEF_RUNTIME_STUBS(GetModuleNamespace)
{
    RUNTIME_STUBS_HEADER(GetModuleNamespace);
    JSTaggedValue localName = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeGetModuleNamespace(thread, localName).GetRawData();
}

DEF_RUNTIME_STUBS(GetModuleNamespaceOnJSFunc)
{
    RUNTIME_STUBS_HEADER(GetModuleNamespaceOnJSFunc);
    JSTaggedValue localName = GetArg(argv, argc, 0);
    JSTaggedValue jsFunc = GetArg(argv, argc, 1);
    return RuntimeGetModuleNamespace(thread, localName, jsFunc).GetRawData();
}

DEF_RUNTIME_STUBS(StModuleVar)
{
    RUNTIME_STUBS_HEADER(StModuleVar);
    JSTaggedValue key = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue value = GetArg(argv, argc, 1);  // 1: means the first parameter
    RuntimeStModuleVar(thread, key, value);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(StModuleVarOnJSFunc)
{
    RUNTIME_STUBS_HEADER(StModuleVarOnJSFunc);
    JSTaggedValue key = GetArg(argv, argc, 0);
    JSTaggedValue value = GetArg(argv, argc, 1);
    JSTaggedValue jsFunc = GetArg(argv, argc, 2);
    RuntimeStModuleVar(thread, key, value, jsFunc);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(LdModuleVar)
{
    RUNTIME_STUBS_HEADER(LdModuleVar);
    JSTaggedValue key = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue taggedFlag = GetArg(argv, argc, 1);  // 1: means the first parameter
    bool innerFlag = taggedFlag.GetInt() != 0;
    return RuntimeLdModuleVar(thread, key, innerFlag).GetRawData();
}

DEF_RUNTIME_STUBS(LdModuleVarOnJSFunc)
{
    RUNTIME_STUBS_HEADER(LdModuleVarOnJSFunc);
    JSTaggedValue key = GetArg(argv, argc, 0);
    JSTaggedValue taggedFlag = GetArg(argv, argc, 1);
    JSTaggedValue jsFunc = GetArg(argv, argc, 2);
    bool innerFlag = taggedFlag.GetInt() != 0;
    return RuntimeLdModuleVar(thread, key, innerFlag, jsFunc).GetRawData();
}

DEF_RUNTIME_STUBS(GetPropIterator)
{
    RUNTIME_STUBS_HEADER(GetPropIterator);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeGetPropIterator(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(AsyncFunctionEnter)
{
    RUNTIME_STUBS_HEADER(AsyncFunctionEnter);
    return RuntimeAsyncFunctionEnter(thread).GetRawData();
}

DEF_RUNTIME_STUBS(GetIterator)
{
    RUNTIME_STUBS_HEADER(GetIterator);
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeGetIterator(thread, obj).GetRawData();
}

DEF_RUNTIME_STUBS(ThrowDyn)
{
    RUNTIME_STUBS_HEADER(ThrowDyn);
    JSTaggedValue value = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    RuntimeThrowDyn(thread, value);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowThrowNotExists)
{
    RUNTIME_STUBS_HEADER(ThrowThrowNotExists);
    RuntimeThrowThrowNotExists(thread);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowPatternNonCoercible)
{
    RUNTIME_STUBS_HEADER(ThrowPatternNonCoercible);
    RuntimeThrowPatternNonCoercible(thread);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowDeleteSuperProperty)
{
    RUNTIME_STUBS_HEADER(ThrowDeleteSuperProperty);
    RuntimeThrowDeleteSuperProperty(thread);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowUndefinedIfHole)
{
    RUNTIME_STUBS_HEADER(ThrowUndefinedIfHole);
    JSHandle<EcmaString> obj = GetHArg<EcmaString>(argv, argc, 0);  // 0: means the zeroth parameter
    RuntimeThrowUndefinedIfHole(thread, obj);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowIfNotObject)
{
    RUNTIME_STUBS_HEADER(ThrowIfNotObject);
    RuntimeThrowIfNotObject(thread);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowConstAssignment)
{
    RUNTIME_STUBS_HEADER(ThrowConstAssignment);
    JSHandle<EcmaString> value = GetHArg<EcmaString>(argv, argc, 0);  // 0: means the zeroth parameter
    RuntimeThrowConstAssignment(thread, value);
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowTypeError)
{
    RUNTIME_STUBS_HEADER(ThrowTypeError);
    JSTaggedValue argMessageStringId = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    std::string message = MessageString::GetMessageString(argMessageStringId.GetInt());
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR, message.c_str());
    THROW_NEW_ERROR_AND_RETURN_VALUE(thread, error.GetTaggedValue(), JSTaggedValue::Hole().GetRawData());
}

DEF_RUNTIME_STUBS(LdGlobalRecord)
{
    RUNTIME_STUBS_HEADER(LdGlobalRecord);
    JSTaggedValue key = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeLdGlobalRecord(thread, key).GetRawData();
}

DEF_RUNTIME_STUBS(GetGlobalOwnProperty)
{
    RUNTIME_STUBS_HEADER(GetGlobalOwnProperty);
    JSTaggedValue key = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    JSTaggedValue globalObj = globalEnv->GetGlobalObject();
    return FastRuntimeStub::GetGlobalOwnProperty(thread, globalObj, key).GetRawData();
}

DEF_RUNTIME_STUBS(TryLdGlobalByName)
{
    RUNTIME_STUBS_HEADER(TryLdGlobalByName);
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    JSHandle<JSTaggedValue> globalObj(thread, globalEnv->GetGlobalObject());
    return RuntimeTryLdGlobalByName(thread, globalObj, prop).GetRawData(); // After checking not on global obj.
}

DEF_RUNTIME_STUBS(LoadMiss)
{
    RUNTIME_STUBS_HEADER(LoadMiss);
    JSTaggedType profileTypeInfo = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue receiver = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue key = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue slotId = GetArg(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue kind = GetArg(argv, argc, 4);   // 4: means the fourth parameter
    return ICRuntimeStub::LoadMiss(thread, reinterpret_cast<ProfileTypeInfo *>(profileTypeInfo), receiver, key,
        slotId.GetInt(), static_cast<ICKind>(kind.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(StoreMiss)
{
    RUNTIME_STUBS_HEADER(StoreMiss);
    JSTaggedType profileTypeInfo = GetTArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue receiver = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue key = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue value = GetArg(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue slotId = GetArg(argv, argc, 4);  // 4: means the fourth parameter
    JSTaggedValue kind = GetArg(argv, argc, 5);  // 5: means the fifth parameter
    return ICRuntimeStub::StoreMiss(thread, reinterpret_cast<ProfileTypeInfo *>(profileTypeInfo), receiver, key, value,
        slotId.GetInt(), static_cast<ICKind>(kind.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(TryUpdateGlobalRecord)
{
    RUNTIME_STUBS_HEADER(TryUpdateGlobalRecord);
    JSTaggedValue prop = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue value = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeTryUpdateGlobalRecord(thread, prop, value).GetRawData();
}

DEF_RUNTIME_STUBS(ThrowReferenceError)
{
    RUNTIME_STUBS_HEADER(ThrowReferenceError);
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeThrowReferenceError(thread, prop, " is not defined").GetRawData();
}

DEF_RUNTIME_STUBS(LdGlobalVar)
{
    RUNTIME_STUBS_HEADER(LdGlobalVar);
    JSHandle<JSTaggedValue> global = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeLdGlobalVarFromProto(thread, global, prop).GetRawData(); // After checked global itself.
}

DEF_RUNTIME_STUBS(StGlobalVar)
{
    RUNTIME_STUBS_HEADER(StGlobalVar);
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeStGlobalVar(thread, prop, value).GetRawData();
}

DEF_RUNTIME_STUBS(ToNumber)
{
    RUNTIME_STUBS_HEADER(ToNumber);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeToNumber(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(ToBoolean)
{
    RUNTIME_STUBS_HEADER(ToBoolean);
    JSTaggedValue value = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    bool result = value.ToBoolean();
    return JSTaggedValue(result).GetRawData();
}

DEF_RUNTIME_STUBS(EqDyn)
{
    RUNTIME_STUBS_HEADER(EqDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeEqDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(NotEqDyn)
{
    RUNTIME_STUBS_HEADER(NotEqDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeNotEqDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(LessDyn)
{
    RUNTIME_STUBS_HEADER(LessDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeLessDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(LessEqDyn)
{
    RUNTIME_STUBS_HEADER(LessEqDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeLessEqDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(GreaterDyn)
{
    RUNTIME_STUBS_HEADER(GreaterDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeGreaterDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(GreaterEqDyn)
{
    RUNTIME_STUBS_HEADER(GreaterEqDyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeGreaterEqDyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(Add2Dyn)
{
    RUNTIME_STUBS_HEADER(Add2Dyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue res = RuntimeAdd2Dyn(thread, left, right);
    return res.GetRawData();
}

DEF_RUNTIME_STUBS(Sub2Dyn)
{
    RUNTIME_STUBS_HEADER(Sub2Dyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeSub2Dyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(Mul2Dyn)
{
    RUNTIME_STUBS_HEADER(Mul2Dyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeMul2Dyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(Div2Dyn)
{
    RUNTIME_STUBS_HEADER(Div2Dyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeDiv2Dyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(Mod2Dyn)
{
    RUNTIME_STUBS_HEADER(Mod2Dyn);
    JSHandle<JSTaggedValue> left = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> right = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeMod2Dyn(thread, left, right).GetRawData();
}

DEF_RUNTIME_STUBS(JumpToCInterpreter)
{
    RUNTIME_STUBS_HEADER(JumpToCInterpreter);
    JSTaggedValue constpool = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue profileTypeInfo = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue acc = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue hotnessCounter = GetArg(argv, argc, 3);  // 3: means the third parameter

    auto sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    const uint8_t *currentPc = reinterpret_cast<const uint8_t*>(GET_ASM_FRAME(sp)->pc);

    uint8_t opcode = currentPc[0];
    asmDispatchTable[opcode](thread, currentPc, sp, constpool, profileTypeInfo, acc, hotnessCounter.GetInt());
    sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    return JSTaggedValue(reinterpret_cast<uint64_t>(sp)).GetRawData();
}

DEF_RUNTIME_STUBS(NotifyBytecodePcChanged)
{
    RUNTIME_STUBS_HEADER(NotifyBytecodePcChanged);
    FrameHandler frameHandler(thread);
    for (; frameHandler.HasFrame(); frameHandler.PrevInterpretedFrame()) {
        if (frameHandler.IsEntryFrame() || frameHandler.IsBuiltinFrame()) {
            continue;
        }
        Method *method = frameHandler.GetMethod();
        // Skip builtins method
        if (method->IsNativeWithCallField()) {
            continue;
        }
        auto bcOffset = frameHandler.GetBytecodeOffset();
        auto *debuggerMgr = thread->GetEcmaVM()->GetJsDebuggerManager();
        debuggerMgr->GetNotificationManager()->BytecodePcChangedEvent(thread, method, bcOffset);
        return JSTaggedValue::Hole().GetRawData();
    }
    return JSTaggedValue::Hole().GetRawData();
}

DEF_RUNTIME_STUBS(CreateEmptyObject)
{
    RUNTIME_STUBS_HEADER(CreateEmptyObject);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    return RuntimeCreateEmptyObject(thread, factory, globalEnv).GetRawData();
}

DEF_RUNTIME_STUBS(CreateEmptyArray)
{
    RUNTIME_STUBS_HEADER(CreateEmptyArray);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    return RuntimeCreateEmptyArray(thread, factory, globalEnv).GetRawData();
}

DEF_RUNTIME_STUBS(GetSymbolFunction)
{
    RUNTIME_STUBS_HEADER(GetSymbolFunction);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    JSHandle<GlobalEnv> globalEnv = ecmaVm->GetGlobalEnv();
    return globalEnv->GetSymbolFunction().GetTaggedValue().GetRawData();
}

DEF_RUNTIME_STUBS(GetUnmapedArgs)
{
    RUNTIME_STUBS_HEADER(GetUnmapedArgs);
    auto sp = const_cast<JSTaggedType*>(thread->GetCurrentInterpretedFrame());
    uint32_t startIdx = 0;
    // 0: means restIdx
    uint32_t actualNumArgs = InterpreterAssembly::GetNumArgs(sp, 0, startIdx);
    return RuntimeGetUnmapedArgs(thread, sp, actualNumArgs, startIdx).GetRawData();
}

DEF_RUNTIME_STUBS(CopyRestArgs)
{
    RUNTIME_STUBS_HEADER(CopyRestArgs);
    JSTaggedValue restIdx = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    auto sp = const_cast<JSTaggedType*>(thread->GetCurrentInterpretedFrame());
    uint32_t startIdx = 0;
    uint32_t restNumArgs = InterpreterAssembly::GetNumArgs(sp, restIdx.GetInt(), startIdx);
    return RuntimeCopyRestArgs(thread, sp, restNumArgs, startIdx).GetRawData();
}

DEF_RUNTIME_STUBS(CreateArrayWithBuffer)
{
    RUNTIME_STUBS_HEADER(CreateArrayWithBuffer);
    JSHandle<JSTaggedValue> argArray = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    return RuntimeCreateArrayWithBuffer(thread, factory, argArray).GetRawData();
}

DEF_RUNTIME_STUBS(CreateObjectWithBuffer)
{
    RUNTIME_STUBS_HEADER(CreateObjectWithBuffer);
    JSHandle<JSObject> argObj = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    return RuntimeCreateObjectWithBuffer(thread, factory, argObj).GetRawData();
}

DEF_RUNTIME_STUBS(NewLexicalEnvDyn)
{
    RUNTIME_STUBS_HEADER(NewLexicalEnvDyn);
    JSTaggedValue numVars = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeNewLexicalEnvDyn(thread, static_cast<uint16_t>(numVars.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(NewThisObject)
{
    RUNTIME_STUBS_HEADER(NewThisObject);
    JSHandle<JSFunction> ctor(GetHArg<JSTaggedValue>(argv, argc, 0));  // 0: means the zeroth parameter

    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSObject> obj = factory->NewJSObjectByConstructor(ctor);
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception().GetRawData());
    return obj.GetTaggedType();  // state is not set here
}

DEF_RUNTIME_STUBS(NewObjDynRange)
{
    RUNTIME_STUBS_HEADER(NewObjDynRange);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> newTarget = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue firstArgIdx = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue length = GetArg(argv, argc, 3);  // 3: means the third parameter
    return RuntimeNewObjDynRange(thread, func, newTarget, static_cast<uint16_t>(firstArgIdx.GetInt()),
                                 static_cast<uint16_t>(length.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(DefinefuncDyn)
{
    RUNTIME_STUBS_HEADER(DefinefuncDyn);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDefinefuncDyn(thread, func).GetRawData();
}

DEF_RUNTIME_STUBS(CreateRegExpWithLiteral)
{
    RUNTIME_STUBS_HEADER(CreateRegExpWithLiteral);
    JSHandle<JSTaggedValue> pattern = GetHArg<JSTaggedValue>(argv, argc, 0);
    JSTaggedValue flags = GetArg(argv, argc, 1);
    return RuntimeCreateRegExpWithLiteral(thread, pattern, static_cast<uint8_t>(flags.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(ThrowIfSuperNotCorrectCall)
{
    RUNTIME_STUBS_HEADER(ThrowIfSuperNotCorrectCall);
    JSTaggedValue index = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue thisValue = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeThrowIfSuperNotCorrectCall(thread, static_cast<uint16_t>(index.GetInt()), thisValue).GetRawData();
}

DEF_RUNTIME_STUBS(CreateObjectHavingMethod)
{
    RUNTIME_STUBS_HEADER(CreateObjectHavingMethod);
    JSHandle<JSObject> literal = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> env = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    return RuntimeCreateObjectHavingMethod(thread, factory, literal, env).GetRawData();
}

DEF_RUNTIME_STUBS(CreateObjectWithExcludedKeys)
{
    RUNTIME_STUBS_HEADER(CreateObjectWithExcludedKeys);
    JSTaggedValue numKeys = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> objVal = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue firstArgRegIdx = GetArg(argv, argc, 2);  // 2: means the second parameter
    return RuntimeCreateObjectWithExcludedKeys(thread, static_cast<uint16_t>(numKeys.GetInt()), objVal,
        static_cast<uint16_t>(firstArgRegIdx.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(DefineNCFuncDyn)
{
    RUNTIME_STUBS_HEADER(DefineNCFuncDyn);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDefineNCFuncDyn(thread, func).GetRawData();
}

DEF_RUNTIME_STUBS(DefineGeneratorFunc)
{
    RUNTIME_STUBS_HEADER(DefineGeneratorFunc);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDefineGeneratorFunc(thread, func).GetRawData();
}

DEF_RUNTIME_STUBS(DefineAsyncGeneratorFunc)
{
    RUNTIME_STUBS_HEADER(DefineAsyncGeneratorFunc);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0); // 0: means the zeroth parameter
    return RuntimeDefineAsyncGeneratorFunc(thread, func).GetRawData();
}

DEF_RUNTIME_STUBS(DefineAsyncFunc)
{
    RUNTIME_STUBS_HEADER(DefineAsyncFunc);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDefineAsyncFunc(thread, func).GetRawData();
}

DEF_RUNTIME_STUBS(DefineMethod)
{
    RUNTIME_STUBS_HEADER(DefineMethod);
    JSHandle<JSFunction> func = GetHArg<JSFunction>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> homeObject = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeDefineMethod(thread, func, homeObject).GetRawData();
}

DEF_RUNTIME_STUBS(CallSpreadDyn)
{
    RUNTIME_STUBS_HEADER(CallSpreadDyn);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> obj = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> array = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    return RuntimeCallSpreadDyn(thread, func, obj, array).GetRawData();
}

DEF_RUNTIME_STUBS(DefineGetterSetterByValue)
{
    RUNTIME_STUBS_HEADER(DefineGetterSetterByValue);
    JSHandle<JSObject> obj = GetHArg<JSObject>(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> prop = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> getter = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> setter = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    JSTaggedValue flag = GetArg(argv, argc, 4);  // 4: means the fourth parameter
    bool bFlag = flag.ToBoolean();
    return RuntimeDefineGetterSetterByValue(thread, obj, prop, getter, setter, bFlag).GetRawData();
}

DEF_RUNTIME_STUBS(SuperCall)
{
    RUNTIME_STUBS_HEADER(SuperCall);
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue firstVRegIdx = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue length = GetArg(argv, argc, 2);  // 2: means the second parameter
    auto sp = const_cast<JSTaggedType*>(thread->GetCurrentInterpretedFrame());
    JSTaggedValue newTarget = InterpreterAssembly::GetNewTarget(sp);
    return RuntimeSuperCall(thread, func, JSHandle<JSTaggedValue>(thread, newTarget),
        static_cast<uint16_t>(firstVRegIdx.GetInt()),
        static_cast<uint16_t>(length.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(OptSuperCall)
{
    RUNTIME_STUBS_HEADER(OptSuperCall);
    return RuntimeOptSuperCall(thread, argv, argc).GetRawData();
}

DEF_RUNTIME_STUBS(ThrowNotCallableException)
{
    RUNTIME_STUBS_HEADER(ThrowNotCallableException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR, "is not callable");
    thread->SetException(error.GetTaggedValue());
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowSetterIsUndefinedException)
{
    RUNTIME_STUBS_HEADER(ThrowSetterIsUndefinedException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
        "Cannot set property when setter is undefined");
    thread->SetException(error.GetTaggedValue());
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowCallConstructorException)
{
    RUNTIME_STUBS_HEADER(ThrowCallConstructorException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
                                                   "class constructor cannot called without 'new'");
    thread->SetException(error.GetTaggedValue());
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowNonConstructorException)
{
    RUNTIME_STUBS_HEADER(ThrowNonConstructorException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
                                                   "function is non-constructor");
    thread->SetException(error.GetTaggedValue());
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowStackOverflowException)
{
    RUNTIME_STUBS_HEADER(ThrowStackOverflowException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::RANGE_ERROR, "Stack overflow!", false);
    if (LIKELY(!thread->HasPendingException())) {
        thread->SetException(error.GetTaggedValue());
    }
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(ThrowDerivedMustReturnException)
{
    RUNTIME_STUBS_HEADER(ThrowDerivedMustReturnException);
    EcmaVM *ecmaVm = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR,
                                                   "Derived constructor must return object or undefined");
    thread->SetException(error.GetTaggedValue());
    return JSTaggedValue::Exception().GetRawData();
}

DEF_RUNTIME_STUBS(CallNative)
{
    RUNTIME_STUBS_HEADER(CallNative);
    auto sp = const_cast<JSTaggedType *>(thread->GetCurrentInterpretedFrame());
    auto state = reinterpret_cast<AsmInterpretedFrame *>(sp) - 1;
    // leave frame prev is prevSp now, change it to current sp
    auto leaveFrame = const_cast<JSTaggedType *>(thread->GetLastLeaveFrame());
    OptimizedLeaveFrame *frame = OptimizedLeaveFrame::GetFrameFromSp(leaveFrame);
    auto cachedFpValue = frame->callsiteFp;
    frame->callsiteFp = reinterpret_cast<uintptr_t>(sp);

    Method *method = ECMAObject::Cast(state->function.GetTaggedObject())->GetCallTarget();
    EcmaRuntimeCallInfo *ecmaRuntimeCallInfo = reinterpret_cast<EcmaRuntimeCallInfo *>(sp);
    JSTaggedValue retValue = reinterpret_cast<EcmaEntrypoint>(
        const_cast<void *>(method->GetNativePointer()))(ecmaRuntimeCallInfo);
    frame->callsiteFp = cachedFpValue;
    RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, JSTaggedValue::Exception().GetRawData());
    return retValue.GetRawData();
}

DEF_RUNTIME_STUBS(LdBigInt)
{
    RUNTIME_STUBS_HEADER(LdBigInt);
    JSHandle<JSTaggedValue> numberBigInt = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeLdBigInt(thread, numberBigInt).GetRawData();
}

DEF_RUNTIME_STUBS(ToNumeric)
{
    RUNTIME_STUBS_HEADER(ToNumeric);
    JSHandle<JSTaggedValue> value = GetHArg<JSTaggedValue>(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeToNumeric(thread, value).GetRawData();
}

DEF_RUNTIME_STUBS(DynamicImport)
{
    RUNTIME_STUBS_HEADER(DynamicImport);
    JSTaggedValue specifier = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeDynamicImport(thread, specifier).GetRawData();
}

DEF_RUNTIME_STUBS(NewLexicalEnvWithNameDyn)
{
    RUNTIME_STUBS_HEADER(NewLexicalEnvWithNameDyn);
    JSTaggedValue numVars = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue scopeId = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeNewLexicalEnvWithNameDyn(thread,
        static_cast<uint16_t>(numVars.GetInt()),
        static_cast<uint16_t>(scopeId.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(OptGetUnmapedArgs)
{
    RUNTIME_STUBS_HEADER(OptGetUnmapedArgs);
    JSTaggedValue actualNumArgs = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    return RuntimeOptGetUnmapedArgs(thread, actualNumArgs.GetInt()).GetRawData();
}

DEF_RUNTIME_STUBS(OptGetLexicalEnv)
{
    RUNTIME_STUBS_HEADER(OptGetLexicalEnv);
    return RuntimeOptGetLexEnv(thread).GetRawData();
}

DEF_RUNTIME_STUBS(OptNewLexicalEnvDyn)
{
    RUNTIME_STUBS_HEADER(OptNewLexicalEnvDyn);
    JSTaggedValue numVars = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSHandle<JSTaggedValue> currentLexEnv = GetHArg<JSTaggedValue>(argv, argc, 1);  // 1: means the first parameter
    return RuntimeOptNewLexicalEnvDyn(thread, static_cast<uint16_t>(numVars.GetInt()), currentLexEnv).GetRawData();
}

DEF_RUNTIME_STUBS(OptNewLexicalEnvWithNameDyn)
{
    RUNTIME_STUBS_HEADER(OptNewLexicalEnvWithNameDyn);
    JSTaggedValue taggedNumVars = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue taggedScopeId = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSHandle<JSTaggedValue> currentLexEnv = GetHArg<JSTaggedValue>(argv, argc, 2);  // 2: means the second parameter
    JSHandle<JSTaggedValue> func = GetHArg<JSTaggedValue>(argv, argc, 3);  // 3: means the third parameter
    uint16_t numVars = static_cast<uint16_t>(taggedNumVars.GetInt());
    uint16_t scopeId = static_cast<uint16_t>(taggedScopeId.GetInt());
    return RuntimeOptNewLexicalEnvWithNameDyn(thread, numVars, scopeId, currentLexEnv, func).GetRawData();
}

DEF_RUNTIME_STUBS(OptPopLexicalEnv)
{
    RUNTIME_STUBS_HEADER(OptPopLexicalEnv);
    JSTaggedValue currentLexenv = RuntimeOptGetLexEnv(thread);
    JSTaggedValue parentLexenv = LexicalEnv::Cast(currentLexenv.GetTaggedObject())->GetParentEnv();
    RuntimeOptSetLexEnv(thread, parentLexenv);
    return JSTaggedValue::VALUE_HOLE;
}

DEF_RUNTIME_STUBS(OptCopyRestArgs)
{
    RUNTIME_STUBS_HEADER(OptCopyRestArgs);
    JSTaggedValue actualArgc = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue restIndex = GetArg(argv, argc, 1);  // 1: means the first parameter
    return RuntimeOptCopyRestArgs(thread, actualArgc.GetInt(), restIndex.GetInt()).GetRawData();
}

DEF_RUNTIME_STUBS(OptNewObjDynRange)
{
    RUNTIME_STUBS_HEADER(OptNewObjDynRange);
    return RuntimeOptNewObjDynRange(thread, argv, argc).GetRawData();
}

DEF_RUNTIME_STUBS(GetTypeArrayPropertyByIndex)
{
    RUNTIME_STUBS_HEADER(GetTypeArrayPropertyByIndex);
    JSTaggedValue obj = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue idx = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue jsType = GetArg(argv, argc, 2); // 2: means the second parameter
    return JSTypedArray::FastGetPropertyByIndex(thread, obj, idx.GetInt(), JSType(jsType.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(SetTypeArrayPropertyByIndex)
{
    RUNTIME_STUBS_HEADER(SetTypeArrayPropertyByIndex);
    JSTaggedValue obj = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue idx = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue value = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue jsType = GetArg(argv, argc, 3); // 3: means the third parameter
    return JSTypedArray::FastSetPropertyByIndex(thread, obj, idx.GetInt(), value, JSType(jsType.GetInt())).GetRawData();
}

DEF_RUNTIME_STUBS(DebugAOTPrint)
{
    RUNTIME_STUBS_HEADER(DebugAOTPrint);
    JSTaggedValue fmtMessageId = GetArg(argv, argc, 0);
    std::string result = MessageString::GetMessageString(fmtMessageId.GetInt());
    std::cerr << "aot slowpath " << result << std::endl;
    return JSTaggedValue::Undefined().GetRawData();
}

DEF_RUNTIME_STUBS(OptNewObjWithIHClass)
{
    RUNTIME_STUBS_HEADER(OptNewObjWithIHClass);
    return RuntimeOptNewObjWithIHClass(thread, argv, argc).GetRawData();
}

DEF_RUNTIME_STUBS(OptLdLexVarDyn)
{
    RUNTIME_STUBS_HEADER(OptLdLexVarDyn);
    JSTaggedValue level = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue slot = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue env = RuntimeOptGetLexEnv(thread);
    for (int32_t i = 0; i < level.GetInt(); i++) {
        JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
        ASSERT(!taggedParentEnv.IsUndefined());
        env = taggedParentEnv;
    }
    return LexicalEnv::Cast(env.GetTaggedObject())->GetProperties(slot.GetInt()).GetRawData();
}

DEF_RUNTIME_STUBS(OptStLexVarDyn)
{
    RUNTIME_STUBS_HEADER(OptStLexVarDyn);
    JSTaggedValue level = GetArg(argv, argc, 0);  // 0: means the zeroth parameter
    JSTaggedValue slot = GetArg(argv, argc, 1);  // 1: means the first parameter
    JSTaggedValue value = GetArg(argv, argc, 2);  // 2: means the second parameter
    JSTaggedValue env = RuntimeOptGetLexEnv(thread);

    for (int32_t i = 0; i < level.GetInt(); i++) {
        JSTaggedValue taggedParentEnv = LexicalEnv::Cast(env.GetTaggedObject())->GetParentEnv();
        ASSERT(!taggedParentEnv.IsUndefined());
        env = taggedParentEnv;
    }
    LexicalEnv::Cast(env.GetTaggedObject())->SetProperties(thread, slot.GetInt(), value);
    return JSTaggedValue::VALUE_HOLE;
}

DEF_RUNTIME_STUBS(JSObjectGetMethod)
{
    RUNTIME_STUBS_HEADER(JSObjectGetMethod);
    JSHandle<JSTaggedValue> obj(thread, GetArg(argv, argc, 0));
    JSHandle<JSTaggedValue> key(thread, GetArg(argv, argc, 1));
    JSHandle<JSTaggedValue> result = JSObject::GetMethod(thread, obj, key);
    return result->GetRawData();
}

JSTaggedType RuntimeStubs::CreateArrayFromList([[maybe_unused]]uintptr_t argGlue, int32_t argc, JSTaggedValue *argvPtr)
{
    auto thread = JSThread::GlueToJSThread(argGlue);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> taggedArray = factory->NewTaggedArray(argc - NUM_MANDATORY_JSFUNC_ARGS);
    for (int index = NUM_MANDATORY_JSFUNC_ARGS; index < argc; ++index) {
        taggedArray->Set(thread, index - NUM_MANDATORY_JSFUNC_ARGS, argvPtr[index]);
    }
    JSHandle<JSArray> arrHandle = JSArray::CreateArrayFromList(thread, taggedArray);
    return arrHandle.GetTaggedValue().GetRawData();
}

int32_t RuntimeStubs::FindElementWithCache(uintptr_t argGlue, JSTaggedType hClass,
                                           JSTaggedType key, int32_t num)
{
    auto thread = JSThread::GlueToJSThread(argGlue);
    auto cls  = reinterpret_cast<JSHClass *>(hClass);
    JSTaggedValue propKey = JSTaggedValue(key);
    auto layoutInfo = LayoutInfo::Cast(cls->GetLayout().GetTaggedObject());
    PropertiesCache *cache = thread->GetPropertiesCache();
    int index = cache->Get(cls, propKey);
    if (index == PropertiesCache::NOT_FOUND) {
        index = layoutInfo->BinarySearch(propKey, num);
        cache->Set(cls, propKey, index);
    }
    return index;
}

JSTaggedType RuntimeStubs::FloatMod(double x, double y)
{
    double result = std::fmod(x, y);
    return JSTaggedValue(result).GetRawData();
}

int32_t RuntimeStubs::DoubleToInt(double x)
{
    return base::NumberHelper::DoubleToInt(x, base::INT32_BITS);
}

void RuntimeStubs::InsertOldToNewRSet([[maybe_unused]]uintptr_t argGlue, Region* region, uintptr_t addr)
{
    return region->InsertOldToNewRSet(addr);
}

void RuntimeStubs::MarkingBarrier([[maybe_unused]]uintptr_t argGlue, uintptr_t slotAddr,
                                  Region *objectRegion, TaggedObject *value, Region *valueRegion)
{
    if (!valueRegion->IsMarking()) {
        return;
    }
    Barriers::Update(slotAddr, objectRegion, value, valueRegion);
}

bool RuntimeStubs::StringsAreEquals(EcmaString *str1, EcmaString *str2)
{
    return EcmaString::StringsAreEqualSameUtfEncoding(str1, str2);
}

bool RuntimeStubs::BigIntEquals(JSTaggedType left, JSTaggedType right)
{
    return BigInt::Equal(JSTaggedValue(left), JSTaggedValue(right));
}

JSTaggedValue RuntimeStubs::NewObject(EcmaRuntimeCallInfo *info)
{
    ASSERT(info);
    JSThread *thread = info->GetThread();
    JSHandle<JSTaggedValue> func(info->GetFunction());
    if (!func->IsHeapObject()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "function is nullptr", JSTaggedValue::Exception());
    }

    if (!func->IsJSFunction()) {
        if (func->IsBoundFunction()) {
            JSTaggedValue result = JSBoundFunction::ConstructInternal(info);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            return result;
        }

        if (func->IsJSProxy()) {
            JSTaggedValue jsObj = JSProxy::ConstructInternal(info);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            return jsObj;
        }
        THROW_TYPE_ERROR_AND_RETURN(thread, "Constructed NonConstructable", JSTaggedValue::Exception());
    }

    JSTaggedValue result = JSFunction::ConstructInternal(info);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    return result;
}

void RuntimeStubs::SaveFrameToContext(JSThread *thread, JSHandle<GeneratorContext> context)
{
    FrameHandler frameHandler(thread);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    uint32_t nregs = frameHandler.GetNumberArgs();
    JSHandle<TaggedArray> regsArray = factory->NewTaggedArray(nregs);
    for (uint32_t i = 0; i < nregs; i++) {
        JSTaggedValue value = frameHandler.GetVRegValue(i);
        regsArray->Set(thread, i, value);
    }
    context->SetRegsArray(thread, regsArray.GetTaggedValue());
    context->SetMethod(thread, frameHandler.GetFunction());

    context->SetAcc(thread, frameHandler.GetAcc());
    context->SetLexicalEnv(thread, thread->GetCurrentLexenv());
    context->SetNRegs(nregs);
    context->SetBCOffset(frameHandler.GetBytecodeOffset());
}

JSTaggedValue RuntimeStubs::CallBoundFunction(EcmaRuntimeCallInfo *info)
{
    JSThread *thread = info->GetThread();
    JSHandle<JSBoundFunction> boundFunc(info->GetFunction());
    JSHandle<JSFunction> targetFunc(thread, boundFunc->GetBoundTarget());
    if (targetFunc->IsClassConstructor()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "class constructor cannot called without 'new'",
                                    JSTaggedValue::Exception());
    }

    JSHandle<TaggedArray> boundArgs(thread, boundFunc->GetBoundArguments());
    const int32_t boundLength = static_cast<int32_t>(boundArgs->GetLength());
    const int32_t argsLength = static_cast<int32_t>(info->GetArgsNumber()) + boundLength;
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    EcmaRuntimeCallInfo *runtimeInfo = EcmaInterpreter::NewRuntimeCallInfo(thread, JSHandle<JSTaggedValue>(targetFunc),
        info->GetThis(), undefined, argsLength);
    RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    if (boundLength == 0) {
        runtimeInfo->SetCallArg(argsLength, 0, info, 0);
    } else {
        // 0 ~ boundLength is boundArgs; boundLength ~ argsLength is args of EcmaRuntimeCallInfo.
        runtimeInfo->SetCallArg(boundLength, boundArgs);
        runtimeInfo->SetCallArg(argsLength, boundLength, info, 0);
    }
    return EcmaInterpreter::Execute(runtimeInfo);
}

void RuntimeStubs::Initialize(JSThread *thread)
{
#define DEF_RUNTIME_STUB(name) kungfu::RuntimeStubCSigns::ID_##name
#define INITIAL_RUNTIME_FUNCTIONS(name) \
    thread->RegisterRTInterface(DEF_RUNTIME_STUB(name), reinterpret_cast<uintptr_t>(name));
    RUNTIME_STUB_WITHOUT_GC_LIST(INITIAL_RUNTIME_FUNCTIONS)
    RUNTIME_STUB_WITH_GC_LIST(INITIAL_RUNTIME_FUNCTIONS)
    TEST_RUNTIME_STUB_GC_LIST(INITIAL_RUNTIME_FUNCTIONS)
#undef INITIAL_RUNTIME_FUNCTIONS
#undef DEF_RUNTIME_STUB
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}  // namespace panda::ecmascript
