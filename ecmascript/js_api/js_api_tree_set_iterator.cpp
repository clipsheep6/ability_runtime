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

#include "ecmascript/js_api/js_api_tree_set_iterator.h"

#include "ecmascript/base/builtins_base.h"
#include "ecmascript/js_api/js_api_tree_set.h"
#include "ecmascript/js_array.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_tree.h"

namespace panda::ecmascript {
using BuiltinsBase = base::BuiltinsBase;
JSTaggedValue JSAPITreeSetIterator::Next(EcmaRuntimeCallInfo *argv)
{
    ASSERT(argv);
    JSThread *thread = argv->GetThread();
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    // Let input be the this value
    JSHandle<JSTaggedValue> input(BuiltinsBase::GetThis(argv));

    if (!input->IsJSAPITreeSetIterator()) {
        THROW_TYPE_ERROR_AND_RETURN(thread, "this value is not a tree set iterator", JSTaggedValue::Exception());
    }
    JSHandle<JSAPITreeSetIterator> iter(input);
    // Let it be [[IteratedSet]].
    JSHandle<JSTaggedValue> iteratedSet(thread, iter->GetIteratedSet());

    // If it is undefined, return CreateIterResultObject(undefined, true).
    const GlobalEnvConstants *globalConst = thread->GlobalConstants();
    if (iteratedSet->IsUndefined()) {
        return JSIterator::CreateIterResultObject(thread, globalConst->GetHandledUndefined(), true).GetTaggedValue();
    }
    JSHandle<TaggedTreeSet> set(thread, JSHandle<JSAPITreeSet>::Cast(iteratedSet)->GetTreeSet());
    int elements = set->NumberOfElements();

    JSMutableHandle<TaggedArray> entries(thread, iter->GetEntries());
    if (elements != static_cast<int>(entries->GetLength())) {
        entries.Update(TaggedTreeSet::GetArrayFromSet(thread, set).GetTaggedValue());
        iter->SetEntries(thread, entries);
    }

    // Let index be Set.[[NextIndex]].
    int index = static_cast<int>(iter->GetNextIndex());
    if (index < elements) {
        IterationKind itemKind = IterationKind(iter->GetIterationKind());

        int keyIndex = entries->Get(index).GetInt();
        iter->SetNextIndex(index + 1);

        JSHandle<JSTaggedValue> key(thread, set->GetKey(keyIndex));
        // If itemKind is key or value, let result be e.[[Key]].
        if (itemKind == IterationKind::VALUE || itemKind == IterationKind::KEY) {
            return JSIterator::CreateIterResultObject(thread, key, false).GetTaggedValue();
        }
        ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
        JSHandle<TaggedArray> array = factory->NewTaggedArray(2);  // 2 means the length of array
        array->Set(thread, 0, key);
        array->Set(thread, 1, key);
        JSHandle<JSTaggedValue> keyAndValue(JSArray::CreateArrayFromList(thread, array));
        return JSIterator::CreateIterResultObject(thread, keyAndValue, false).GetTaggedValue();
    }

    // Set [[IteratedSet]] to undefined.
    iter->SetIteratedSet(thread, JSTaggedValue::Undefined());
    return JSIterator::CreateIterResultObject(thread, globalConst->GetHandledUndefined(), true).GetTaggedValue();
}

JSHandle<JSTaggedValue> JSAPITreeSetIterator::CreateTreeSetIterator(JSThread *thread,
                                                                    JSHandle<JSTaggedValue> &obj,
                                                                    IterationKind kind)
{
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    if (!obj->IsJSAPITreeSet()) {
        if (obj->IsJSProxy() && JSHandle<JSProxy>::Cast(obj)->GetTarget().IsJSAPITreeSet()) {
            obj = JSHandle<JSTaggedValue>(thread, JSHandle<JSProxy>::Cast(obj)->GetTarget());
        } else {
            THROW_TYPE_ERROR_AND_RETURN(thread, "obj is not TreeSet", thread->GlobalConstants()->GetHandledUndefined());
        }
    }
    JSHandle<JSTaggedValue> iter(factory->NewJSAPITreeSetIterator(JSHandle<JSAPITreeSet>(obj), kind));
    return iter;
}
}  // namespace panda::ecmascript
