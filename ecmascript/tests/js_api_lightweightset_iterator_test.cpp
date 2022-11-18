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
 
#include "ecmascript/js_api/js_api_lightweightset.h"
#include "ecmascript/containers/containers_private.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_api/js_api_lightweightset_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_iterator.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda;
using namespace panda::ecmascript;

namespace panda::test {
class JSAPILightWeightSetIteratorTest : public testing::Test {
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

    EcmaVM *instance {nullptr};
    ecmascript::EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};

protected:
    JSAPILightWeightSet *CreateLightWeightSet()
    {
        JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();

        JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
        JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
        JSHandle<JSTaggedValue> value =
            JSObject::GetProperty(thread, JSHandle<JSTaggedValue>(globalObject), key).GetValue();

        auto objCallInfo =
            TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6); // 3 means the value
        objCallInfo->SetFunction(JSTaggedValue::Undefined());
        objCallInfo->SetThis(value.GetTaggedValue());
        objCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::LightWeightSet)));

        auto prev = TestHelper::SetupFrame(thread, objCallInfo);
        JSHandle<JSTaggedValue> constructor =
            JSHandle<JSTaggedValue>(thread, containers::ContainersPrivate::Load(objCallInfo));
        TestHelper::TearDownFrame(thread, prev);
        JSHandle<JSAPILightWeightSet> lightweightSet =
            JSHandle<JSAPILightWeightSet>::Cast(factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor),
                                                                                  constructor));
        JSHandle<JSTaggedValue> hashArray = JSHandle<JSTaggedValue>(factory->NewTaggedArray(8)); // 8 means the value
        JSHandle<JSTaggedValue> valueArray = JSHandle<JSTaggedValue>(factory->NewTaggedArray(8)); // 8 means the value
        lightweightSet->SetHashes(thread, hashArray);
        lightweightSet->SetValues(thread, valueArray);
        lightweightSet->SetLength(0); // 0 means the value
        return *lightweightSet;
    }
};

/**
 * @tc.name: Next
 * @tc.desc: test special return of Next,
 *           including throw exception and return undefined
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPILightWeightSetIteratorTest, SpecailReturnOfNextCreateLightWeightSetIterator)
{
    JSHandle<JSAPILightWeightSet> jsLightWeightSet(thread, CreateLightWeightSet());
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPILightWeightSetIterator> lightWeightSetIterator = factory->NewJSAPILightWeightSetIterator(
        jsLightWeightSet, IterationKind::KEY);
    lightWeightSetIterator->SetIteratedLightWeightSet(thread, JSTaggedValue::Undefined());
    
    // test Next exception
    {
        auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
        ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
        ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
        JSTaggedValue result = JSAPILightWeightSetIterator::Next(ecmaRuntimeCallInfo);
        TestHelper::TearDownFrame(thread, prev);
        EXPECT_EQ(result, JSTaggedValue::Exception());
        EXPECT_EXCEPTION();
    }

    // test Next return undefined
    {
        auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
        ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
        ecmaRuntimeCallInfo->SetThis(lightWeightSetIterator.GetTaggedValue());

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
        JSTaggedValue result = JSAPILightWeightSetIterator::Next(ecmaRuntimeCallInfo);
        TestHelper::TearDownFrame(thread, prev);
        EXPECT_EQ(result, thread->GlobalConstants()->GetUndefinedIterResult());
    }
}
}  // namespace panda::test