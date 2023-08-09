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

#include "ecmascript/builtins/builtins_typedarray.h"

#include "ecmascript/base/typed_array_helper-inl.h"
#include "ecmascript/base/typed_array_helper.h"
#include "ecmascript/builtins/builtins_array.h"
#include "ecmascript/builtins/builtins_object.h"
#include "ecmascript/ecma_runtime_call_info.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_array_iterator.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/object_operator.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::builtins;
using namespace panda::ecmascript::base;

namespace panda::test {
using Array = ecmascript::builtins::BuiltinsArray;
using TypedArray = ecmascript::builtins::BuiltinsTypedArray;
using TypedArrayHelper = ecmascript::base::TypedArrayHelper;

class BuiltinsTypedArrayTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

protected:
    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};

    class TestClass : public base::BuiltinsBase {
    public:
        static JSTaggedValue TestForEachFunc(EcmaRuntimeCallInfo *argv)
        {
            JSHandle<JSTaggedValue> key = GetCallArg(argv, 0);
            if (key->IsUndefined()) {
                return JSTaggedValue::Undefined();
            }
            JSArray *jsArray = JSArray::Cast(GetThis(argv)->GetTaggedObject());
            uint32_t length = jsArray->GetArrayLength() + 1U;
            jsArray->SetArrayLength(argv->GetThread(), length);
            return JSTaggedValue::Undefined();
        }

        static JSTaggedValue TestEveryFunc(EcmaRuntimeCallInfo *argv)
        {
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 0) {
                [[maybe_unused]] int aaa = GetCallArg(argv, 0)->GetInt();
                //  10 : test case
                if (GetCallArg(argv, 0)->GetInt() > 10) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }

        static JSTaggedValue TestFilterFunc(EcmaRuntimeCallInfo *argv)
        {
            ASSERT(argv);
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 0) {
                // 10 : test case
                if (GetCallArg(argv, 0)->GetInt() > 10) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }

        static JSTaggedValue TestMapFunc(EcmaRuntimeCallInfo *argv)
        {
            int accumulator = GetCallArg(argv, 0)->GetInt();
            accumulator = accumulator * 2; // 2 : mapped to 2 times the original value
            return BuiltinsBase::GetTaggedInt(accumulator);
        }

        static JSTaggedValue TestFindFunc(EcmaRuntimeCallInfo *argv)
        {
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 0) {
                // 10 : test case
                if (GetCallArg(argv, 0)->GetInt() > 10) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }

        static JSTaggedValue TestFindIndexFunc(EcmaRuntimeCallInfo *argv)
        {
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 0) {
                //  10 : test case
                if (GetCallArg(argv, 0)->GetInt() > 10) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }

        static JSTaggedValue TestToSortedFunc(EcmaRuntimeCallInfo *argv)
        {
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 1) {
                // x < y
                if (GetCallArg(argv, 0)->GetInt() < GetCallArg(argv, 1)->GetInt()) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }

        static JSTaggedValue TestReduceFunc(EcmaRuntimeCallInfo *argv)
        {
            int accumulator = GetCallArg(argv, 0)->GetInt();
            accumulator = accumulator + GetCallArg(argv, 1)->GetInt();
            return BuiltinsBase::GetTaggedInt(accumulator);
        }

        static JSTaggedValue TestReduceRightFunc(EcmaRuntimeCallInfo *argv)
        {
            int accumulator = GetCallArg(argv, 0)->GetInt();
            accumulator = accumulator + GetCallArg(argv, 1)->GetInt();
            return BuiltinsBase::GetTaggedInt(accumulator);
        }

        static JSTaggedValue TestSomeFunc(EcmaRuntimeCallInfo *argv)
        {
            uint32_t argc = argv->GetArgsNumber();
            if (argc > 0) {
                //  10 : test case
                if (GetCallArg(argv, 0)->GetInt() > 10) {
                    return GetTaggedBoolean(true);
                }
            }
            return GetTaggedBoolean(false);
        }
    };
};

JSTaggedValue CreateBuiltinsTypeArrayJSObject(JSThread *thread, const CString keyCStr)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();
    JSHandle<JSTaggedValue> hclass = env->GetObjectFunction();
    ObjectFactory *factory = ecmaVM->GetFactory();

    JSHandle<JSTaggedValue> obj(factory->NewJSObjectByConstructor(JSHandle<JSFunction>(hclass), hclass));
    JSHandle<JSTaggedValue> key(factory->NewFromASCII(&keyCStr[0]));
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(1));
    JSObject::SetProperty(thread, obj, key, value);
    return obj.GetTaggedValue();
}

JSTypedArray *CreateTypedArrayFromList(JSThread *thread, const JSHandle<TaggedArray> &array)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();

    JSHandle<JSTaggedValue> jsarray(JSArray::CreateArrayFromList(thread, array));
    JSHandle<JSFunction> int8_array(env->GetInt8ArrayFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());
    //  6 : test case
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue(*int8_array), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue(*int8_array));
    ecmaRuntimeCallInfo1->SetThis(JSTaggedValue(*globalObject));
    ecmaRuntimeCallInfo1->SetCallArg(0, jsarray.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = TypedArray::Int8ArrayConstructor(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    EXPECT_TRUE(result.IsECMAObject());
    JSTypedArray *int8arr = JSTypedArray::Cast(reinterpret_cast<TaggedObject *>(result.GetRawData()));
    return int8arr;
}


HWTEST_F_L0(BuiltinsTypedArrayTest, Species)
{
    auto ecmaVM = thread->GetEcmaVM();
    JSHandle<GlobalEnv> env = ecmaVM->GetGlobalEnv();
    JSHandle<JSFunction> array(env->GetArrayFunction());
    JSHandle<JSObject> globalObject(thread, env->GetGlobalObject());

    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
    ecmaRuntimeCallInfo1->SetFunction(array.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetThis(globalObject.GetTaggedValue());

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result = TypedArray::Species(ecmaRuntimeCallInfo1);
    ASSERT_TRUE(result.IsECMAObject());
}

HWTEST_F_L0(BuiltinsTypedArrayTest, Includes)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(2));
    array->Set(thread, 1, JSTaggedValue(3));
    array->Set(thread, 2, JSTaggedValue(4));

    [[maybe_unused]] JSHandle<JSTaggedValue> obj =
        JSHandle<JSTaggedValue>(thread, CreateTypedArrayFromList(thread, array));
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    [[maybe_unused]] JSTaggedValue result = TypedArray::Includes(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.JSTaggedValue::ToBoolean()); // new Int8Array[2,3,4].includes(2)

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(1)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    result = TypedArray::Includes(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(!result.JSTaggedValue::ToBoolean()); // new Int8Array[2,3,4].includes(1)

    auto ecmaRuntimeCallInfo3 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo3->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo3->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo3->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(3)));
    ecmaRuntimeCallInfo3->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(1)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo3);
    result = TypedArray::Includes(ecmaRuntimeCallInfo3);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.JSTaggedValue::ToBoolean()); // new Int8Array[2,3,4].includes(3, 1)

    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo4->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo4->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo4->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo4->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(5)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    result = Array::Includes(ecmaRuntimeCallInfo4);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(!result.JSTaggedValue::ToBoolean()); // new Int8Array[2,3,4].includes(2, 5)

    auto ecmaRuntimeCallInfo5 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8);
    ecmaRuntimeCallInfo5->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo5->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo5->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(2)));
    ecmaRuntimeCallInfo5->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(-2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo5);
    result = Array::Includes(ecmaRuntimeCallInfo5);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(!result.JSTaggedValue::ToBoolean()); // new Int8Array[2,3,4].includes(2, -2)
}

HWTEST_F_L0(BuiltinsTypedArrayTest, At)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    [[maybe_unused]] JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    array->Set(thread, 0, JSTaggedValue(2));
    array->Set(thread, 1, JSTaggedValue(3));
    array->Set(thread, 2, JSTaggedValue(4));

    [[maybe_unused]] JSHandle<JSTaggedValue> obj =
        JSHandle<JSTaggedValue>(thread, CreateTypedArrayFromList(thread, array));
    auto ecmaRuntimeCallInfo1 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(2)));

    [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    [[maybe_unused]] JSTaggedValue result = TypedArray::At(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_EQ(result, JSTaggedValue(4));

    auto ecmaRuntimeCallInfo2 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(-2)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    result = TypedArray::At(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_EQ(result, JSTaggedValue(3));

    auto ecmaRuntimeCallInfo3 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(-4)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo3);
    result = TypedArray::At(ecmaRuntimeCallInfo3);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.IsUndefined());

    auto ecmaRuntimeCallInfo4 = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(3)));

    prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo4);
    result = TypedArray::At(ecmaRuntimeCallInfo4);
    TestHelper::TearDownFrame(thread, prev);

    ASSERT_TRUE(result.IsUndefined());
}

HWTEST_F_L0(BuiltinsTypedArrayTest, ToSorted)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
    JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    // array [10, 8, 30]
    array->Set(thread, 0, JSTaggedValue(10));
    array->Set(thread, 1, JSTaggedValue(8));
    array->Set(thread, 2, JSTaggedValue(30));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArrayFromList(thread, array));
    JSHandle<JSFunction> func = factory->NewJSFunction(env, reinterpret_cast<void *>(TestClass::TestToSortedFunc));
    auto ecmaRuntimeCallInfo1 =
        TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6); // 6 means 1 call arg
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, func.GetTaggedValue());

    [[maybe_unused]] auto prev1 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = TypedArray::ToSorted(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev1);

    EXPECT_TRUE(result1.IsTypedArray());
    JSHandle<JSTaggedValue> resultArr1 = JSHandle<JSTaggedValue>(thread, result1);
    // [30, 10, 8]
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 0).GetValue()->GetInt(), 30);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 1).GetValue()->GetInt(), 10);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 2).GetValue()->GetInt(), 8);

    auto ecmaRuntimeCallInfo2 =
        TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4); // 4 means 0 call arg
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());

    [[maybe_unused]] auto prev2 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = TypedArray::ToSorted(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev2);

    EXPECT_TRUE(result2.IsTypedArray());
    JSHandle<JSTaggedValue> resultArr2 = JSHandle<JSTaggedValue>(thread, result2);
    // [8, 10 ,30]
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 0).GetValue()->GetInt(), 8);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 1).GetValue()->GetInt(), 10);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 2).GetValue()->GetInt(), 30);
}

HWTEST_F_L0(BuiltinsTypedArrayTest, With)
{
    ASSERT_NE(thread, nullptr);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> array(factory->NewTaggedArray(3));
    // array [1, 2, 3]
    array->Set(thread, 0, JSTaggedValue(1));
    array->Set(thread, 1, JSTaggedValue(2));
    array->Set(thread, 2, JSTaggedValue(3));

    JSHandle<JSTaggedValue> obj = JSHandle<JSTaggedValue>(thread, CreateTypedArrayFromList(thread, array));
    auto ecmaRuntimeCallInfo1 =
        TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8); // 8 means 2 call args
    ecmaRuntimeCallInfo1->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo1->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo1->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(-1)));
    ecmaRuntimeCallInfo1->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(30))); // with(-1, 30)

    [[maybe_unused]] auto prev1 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo1);
    JSTaggedValue result1 = TypedArray::With(ecmaRuntimeCallInfo1);
    TestHelper::TearDownFrame(thread, prev1);

    EXPECT_TRUE(result1.IsTypedArray());
    JSHandle<JSTaggedValue> resultArr1 = JSHandle<JSTaggedValue>(thread, result1);
    // [1, 2, 30]
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 0).GetValue()->GetInt(), 1);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 1).GetValue()->GetInt(), 2);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr1, 2).GetValue()->GetInt(), 30);

    auto ecmaRuntimeCallInfo2 =
        TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 8); // 8 means 2 call args
    ecmaRuntimeCallInfo2->SetFunction(JSTaggedValue::Undefined());
    ecmaRuntimeCallInfo2->SetThis(obj.GetTaggedValue());
    ecmaRuntimeCallInfo2->SetCallArg(0, JSTaggedValue(static_cast<int32_t>(1)));
    ecmaRuntimeCallInfo2->SetCallArg(1, JSTaggedValue(static_cast<int32_t>(-100))); // with(1, -100)

    [[maybe_unused]] auto prev2 = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo2);
    JSTaggedValue result2 = TypedArray::With(ecmaRuntimeCallInfo2);
    TestHelper::TearDownFrame(thread, prev2);

    EXPECT_TRUE(result2.IsTypedArray());
    JSHandle<JSTaggedValue> resultArr2 = JSHandle<JSTaggedValue>(thread, result2);
    // [1, -100, 3]
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 0).GetValue()->GetInt(), 1);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 1).GetValue()->GetInt(), -100);
    EXPECT_EQ(JSTypedArray::GetProperty(thread, resultArr2, 2).GetValue()->GetInt(), 3); 
}
}  // namespace panda::test
