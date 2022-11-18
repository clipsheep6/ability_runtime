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

#include "ecmascript/ecma_string.h"
#include "ecmascript/containers/containers_private.h"
#include "ecmascript/ecma_string.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_env.h"
#include "ecmascript/js_api/js_api_lightweightmap.h"
#include "ecmascript/js_api/js_api_lightweightmap_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_iterator.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda;

using namespace panda::ecmascript;

using namespace panda::ecmascript::containers;

namespace panda::test {
class JSAPILightWeightMapIteratorTest : public testing::Test {
public:
    const static int DEFAULT_SIZE = 8;
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
    JSAPILightWeightMap *CreateLightWeightMap()
    {
        JSHandle<GlobalEnv> env = thread->GetEcmaVM()->GetGlobalEnv();
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();

        JSHandle<JSTaggedValue> globalObject = env->GetJSGlobalObject();
        JSHandle<JSTaggedValue> key(factory->NewFromASCII("ArkPrivate"));
        JSHandle<JSTaggedValue> value = JSObject::GetProperty(thread,
                                                              JSHandle<JSTaggedValue>(globalObject), key).GetValue();

        auto objCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 6);
        objCallInfo->SetFunction(JSTaggedValue::Undefined());
        objCallInfo->SetThis(value.GetTaggedValue());
        objCallInfo->SetCallArg(0, JSTaggedValue(static_cast<int>(containers::ContainerTag::LightWeightMap)));

        auto prev = TestHelper::SetupFrame(thread, objCallInfo);
        JSHandle<JSTaggedValue> constructor =
            JSHandle<JSTaggedValue>(thread, containers::ContainersPrivate::Load(objCallInfo));
        TestHelper::TearDownFrame(thread, prev);
        JSHandle<JSAPILightWeightMap> lightWeightMap = JSHandle<JSAPILightWeightMap>::
            Cast(factory->NewJSObjectByConstructor(JSHandle<JSFunction>(constructor), constructor));
        JSHandle<JSTaggedValue> hashArray = JSHandle<JSTaggedValue>(factory->NewTaggedArray(DEFAULT_SIZE));
        JSHandle<JSTaggedValue> keyArray = JSHandle<JSTaggedValue>(factory->NewTaggedArray(DEFAULT_SIZE));
        JSHandle<JSTaggedValue> valueArray = JSHandle<JSTaggedValue>(factory->NewTaggedArray(DEFAULT_SIZE));
        lightWeightMap->SetHashes(thread, hashArray);
        lightWeightMap->SetKeys(thread, keyArray);
        lightWeightMap->SetValues(thread, valueArray);
        lightWeightMap->SetLength(0);
        return *lightWeightMap;
    }
};

/**
 * @tc.name: Next and CreateLightWeightMapIterator
 * @tc.desc: test special return of Next and CreateLightWeightMapIterator,
 *           including throw exception and return undefined
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F_L0(JSAPILightWeightMapIteratorTest, SpecailReturnOfNextCreateLightWeightMapIterator)
{
    JSHandle<JSAPILightWeightMap> jsLightWeightMap(thread, CreateLightWeightMap());
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<JSAPILightWeightMapIterator> lightWeightMapIterator = factory->NewJSAPILightWeightMapIterator(
        jsLightWeightMap, IterationKind::KEY_AND_VALUE);
    lightWeightMapIterator->SetIteratedLightWeightMap(thread, JSTaggedValue::Undefined());
    
    // test Next exception
    {
        auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
        ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
        ecmaRuntimeCallInfo->SetThis(JSTaggedValue::Undefined());

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
        JSTaggedValue result = JSAPILightWeightMapIterator::Next(ecmaRuntimeCallInfo);
        TestHelper::TearDownFrame(thread, prev);
        EXPECT_EQ(result, JSTaggedValue::Exception());
        EXPECT_EXCEPTION();
    }

    // test Next return undefined
    {
        auto ecmaRuntimeCallInfo = TestHelper::CreateEcmaRuntimeCallInfo(thread, JSTaggedValue::Undefined(), 4);
        ecmaRuntimeCallInfo->SetFunction(JSTaggedValue::Undefined());
        ecmaRuntimeCallInfo->SetThis(lightWeightMapIterator.GetTaggedValue());

        [[maybe_unused]] auto prev = TestHelper::SetupFrame(thread, ecmaRuntimeCallInfo);
        JSTaggedValue result = JSAPILightWeightMapIterator::Next(ecmaRuntimeCallInfo);
        TestHelper::TearDownFrame(thread, prev);
        EXPECT_EQ(result, thread->GlobalConstants()->GetUndefinedIterResult());
    }

    // test CreateLightWeightMapIterator exception
    {
        JSHandle<JSTaggedValue> undefined(thread, JSTaggedValue::Undefined());
        JSHandle<JSTaggedValue> result =
            JSAPILightWeightMapIterator::CreateLightWeightMapIterator(thread, undefined, IterationKind::KEY_AND_VALUE);
        EXPECT_EQ(result.GetTaggedValue(), JSTaggedValue::Exception());
        EXPECT_EXCEPTION();
    }
}
}  // namespace panda::test
