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

#include "ecmascript/tagged_list.h"

#include "ecmascript/base/array_helper.h"
#include "ecmascript/base/number_helper.h"
#include "ecmascript/base/typed_array_helper-inl.h"
#include "ecmascript/interpreter/interpreter.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_tagged_number.h"
#include "ecmascript/object_factory.h"

namespace panda::ecmascript {
template <typename Derived>
JSHandle<Derived> TaggedList<Derived>::Create(const JSThread *thread, int numberOfNodes)
{
    ASSERT_PRINT(numberOfNodes > 0, "size must be a non-negative integer");
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    int length = ELEMENTS_START_INDEX + Derived::ENTRY_SIZE + numberOfNodes * Derived::ENTRY_SIZE;
    JSHandle<TaggedArray> taggedArray = factory->NewTaggedArray(length);
    auto taggedList = JSHandle<Derived>::Cast(taggedArray);
    JSTaggedValue data = JSTaggedValue(ELEMENTS_START_INDEX);
    taggedList->SetNumberOfNodes(thread, 0);
    taggedList->SetNumberOfDeletedNodes(thread, 0);
    taggedList->SetElement(thread, HEAD_TABLE_INDEX, data);
    taggedList->SetElement(thread, TAIL_TABLE_INDEX, data);
    taggedList->SetElement(thread, ELEMENTS_START_INDEX, JSTaggedValue::Hole());
    taggedList->SetElement(thread, ELEMENTS_START_INDEX + NEXT_PTR_OFFSET, data);
    return taggedList;
}

template <typename Derived>
void TaggedList<Derived>::CopyArray(const JSThread *thread, JSHandle<Derived> &taggedList)
{
    int capacity = GetCapacityFromTaggedArray();
    JSMutableHandle<JSTaggedValue> value(thread, JSTaggedValue::Undefined());
    for (int i = 0; i < capacity; i++) {
        value.Update(GetElement(i));
        taggedList->SetElement(thread, i, value.GetTaggedValue());
    }
    taggedList->SetNumberOfDeletedNodes(thread, NumberOfDeletedNodes());
}

template <typename Derived>
JSHandle<Derived> TaggedList<Derived>::GrowCapacity(const JSThread *thread, const JSHandle<Derived> &taggedList)
{
    int actualNodeNum = taggedList->NumberOfNodes();
    int deleteNodeNum = taggedList->NumberOfDeletedNodes();
    int needCapacity = actualNodeNum + 1;
    int taggedArrayLength = taggedList->GetCapacityFromTaggedArray();
    int actualArrayCapacity = (taggedArrayLength - ELEMENTS_START_INDEX - (deleteNodeNum + 1) * Derived::ENTRY_SIZE);
    if (needCapacity * Derived::ENTRY_SIZE < actualArrayCapacity) {
        return taggedList;
    }
    uint32_t length = static_cast<uint32_t>(actualNodeNum);
    uint32_t newCapacity = length + (length >> 1UL);
    JSHandle<Derived> list = Create(thread, newCapacity < DEFAULT_ARRAY_LENGHT ? DEFAULT_ARRAY_LENGHT : newCapacity);
    taggedList->CopyArray(thread, list);
    return list;
}

template <typename Derived>
JSTaggedValue TaggedList<Derived>::AddNode(const JSThread *thread, const JSHandle<Derived> &taggedList,
                                           const JSHandle<JSTaggedValue> &value, const int index, int prevDataIndex)
{
    JSHandle<Derived> list = GrowCapacity(thread, taggedList);
    int deleteNodeLength = list->NumberOfDeletedNodes();
    int nodeLength = list->NumberOfNodes();
    int finalDataIndex = ELEMENTS_START_INDEX + (nodeLength + 1 + deleteNodeLength) * Derived::ENTRY_SIZE;
    
    list->InsertNode(thread, value, prevDataIndex, finalDataIndex);
    if (index == -1 || nodeLength == index) {
        list->SetElement(thread, TAIL_TABLE_INDEX, JSTaggedValue(finalDataIndex));
    }
    return list.GetTaggedValue();
}

template <typename Derived>
void TaggedList<Derived>::Clear(const JSThread *thread)
{
    int numberOfNodes = NumberOfNodes();
    int dataIndex = ELEMENTS_START_INDEX;
    for (int i = 0; i < numberOfNodes; i++) {
        dataIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        SetElement(thread, dataIndex, JSTaggedValue::Hole());
    }
    JSTaggedValue data = JSTaggedValue(ELEMENTS_START_INDEX);
    SetNumberOfNodes(thread, 0);
    SetNumberOfDeletedNodes(thread, 0);
    SetElement(thread, HEAD_TABLE_INDEX, data);
    SetElement(thread, TAIL_TABLE_INDEX, data);
    SetElement(thread, ELEMENTS_START_INDEX, JSTaggedValue::Hole());
    SetElement(thread, ELEMENTS_START_INDEX + NEXT_PTR_OFFSET, data);
}

template <typename Derived>
JSTaggedValue TaggedList<Derived>::TaggedListToArray(const JSThread *thread, const JSHandle<Derived> &list)
{
    uint32_t length = static_cast<uint32_t>(list->NumberOfNodes());
    JSHandle<JSArray> array = thread->GetEcmaVM()->GetFactory()->NewJSArray();
    array->SetArrayLength(thread, length);
    JSHandle<TaggedArray> arrayElements(thread, array->GetElements());
    uint32_t oldLength = arrayElements->GetLength();
    JSHandle<TaggedArray> newElements =
        thread->GetEcmaVM()->GetFactory()->CopyArray(arrayElements, oldLength, length);
    int dataIndex = ELEMENTS_START_INDEX;
    for (uint32_t i = 0; i < length; i++) {
        dataIndex = list->GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        newElements->Set(thread, i, list->GetElement(dataIndex));
    }
    array->SetElements(thread, newElements);
    return array.GetTaggedValue();
}

template <typename Derived>
JSHandle<TaggedArray> TaggedList<Derived>::OwnKeys(JSThread *thread, const JSHandle<Derived> &list)
{
    uint32_t length = static_cast<uint32_t>(list->NumberOfNodes());
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    JSHandle<TaggedArray> keys = factory->NewTaggedArray(length);

    for (uint32_t i = 0; i < length; i++) {
        JSTaggedValue elementData = JSTaggedValue(i);
        keys->Set(thread, i, elementData);
    }

    return keys;
}

template<typename Derived>
int TaggedList<Derived>::FindIndexByElement(const JSTaggedValue &element)
{
    int dataIndex = ELEMENTS_START_INDEX;
    int nextDataIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    while (nextDataIndex != ELEMENTS_START_INDEX) {
        dataIndex = nextDataIndex;
        JSTaggedValue data = GetElement(dataIndex);
        nextDataIndex = GetElement(nextDataIndex + NEXT_PTR_OFFSET).GetInt();
        if (JSTaggedValue::SameValue(data, element)) {
            return nodeSum;
        }
        nodeSum++;
    }
    return -1;
}

template<typename Derived>
int TaggedList<Derived>::FindLastIndexByElement(const JSTaggedValue &element)
{
    int dataIndex = ELEMENTS_START_INDEX;
    int nextIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    int lastIndex = -1;
    while (nextIndex != ELEMENTS_START_INDEX) {
        dataIndex = nextIndex;
        JSTaggedValue data = GetElement(dataIndex);
        if (JSTaggedValue::SameValue(data, element)) {
            lastIndex = nodeSum;
        }
        nextIndex = GetElement(nextIndex + NEXT_PTR_OFFSET).GetInt();
        nodeSum++;
    }
    return lastIndex;
}

template<typename Derived>
int TaggedList<Derived>::FindDataIndexByNodeIndex(int index) const
{
    int dataIndex = ELEMENTS_START_INDEX;
    int nextIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    while (nextIndex != ELEMENTS_START_INDEX) {
        dataIndex = nextIndex;
        if (nodeSum == index) {
            return dataIndex;
        }
        nextIndex = GetElement(nextIndex + NEXT_PTR_OFFSET).GetInt();
        nodeSum++;
    }
    return -1;
}

template<typename Derived>
void TaggedList<Derived>::MapNodeIndexToDataIndex(int* nodeIndexMapToDataIndex, int length)
{
    int i = 0;
    int nextIndex = ELEMENTS_START_INDEX;
    while (i < length) {
        nextIndex = GetElement(nextIndex + NEXT_PTR_OFFSET).GetInt();
        nodeIndexMapToDataIndex[i] = nextIndex;
        i++;
    }
}

template<typename Derived>
void TaggedList<Derived>::RemoveNode(JSThread *thread, int prevDataIndex)
{
    int tailTableIndex = GetElement(TAIL_TABLE_INDEX).GetInt();
    if (tailTableIndex != GetElement(HEAD_TABLE_INDEX).GetInt()) {
        int dataIndex = GetElement(prevDataIndex + NEXT_PTR_OFFSET).GetInt();
        int nextDataIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        if (dataIndex == tailTableIndex) {
            SetElement(thread, TAIL_TABLE_INDEX, JSTaggedValue(prevDataIndex));
        }
        if (std::is_same_v<TaggedDoubleList, Derived>) {
            SetElement(thread, nextDataIndex + PREV_PTR_OFFSET, JSTaggedValue(prevDataIndex));
        }
        SetElement(thread, dataIndex, JSTaggedValue::Hole());
        SetElement(thread, prevDataIndex + NEXT_PTR_OFFSET, JSTaggedValue(nextDataIndex));
        SetNumberOfNodes(thread, NumberOfNodes() - 1);
        SetNumberOfDeletedNodes(thread, NumberOfDeletedNodes() + 1);
    }
}

template<typename Derived>
int TaggedList<Derived>::FindPrevNodeByIndex(int index) const
{
    int prevDataIndex = ELEMENTS_START_INDEX;
    int nodeSum = 0;
    int len = GetElement(NUMBER_OF_NODE_INDEX).GetInt();
    while (nodeSum <= len) {
        if (nodeSum == index) {
            return prevDataIndex;
        }
        prevDataIndex = GetElement(prevDataIndex + NEXT_PTR_OFFSET).GetInt();
        nodeSum++;
    }
    return -1;
}

template<typename Derived>
int TaggedList<Derived>::FindPrevNodeByValue(const JSTaggedValue &element)
{
    int dataIndex = ELEMENTS_START_INDEX;
    int nodeSum = 0;
    int len = GetElement(NUMBER_OF_NODE_INDEX).GetInt();
    while (nodeSum <= len) {
        int nextDataIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        JSTaggedValue data = GetElement(nextDataIndex);
        if (JSTaggedValue::SameValue(data, element)) {
            return dataIndex;
        }
        dataIndex = nextDataIndex;
        nodeSum++;
    }
    return -1;
}

template<typename Derived>
JSTaggedValue TaggedList<Derived>::FindElementByIndex(int index) const
{
    int dataIndex = GetElement(ELEMENTS_START_INDEX + NEXT_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    while (dataIndex != ELEMENTS_START_INDEX) {
        if (nodeSum == index) {
            return GetElement(dataIndex);
        }
        dataIndex = GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        nodeSum++;
    }
    return JSTaggedValue::Undefined();
}

template<typename Derived>
std::pair<int, JSTaggedValue> TaggedList<Derived>::FindElementByDataIndex(int dataindex) const
{
    int targetDataIndex = GetElement(dataindex + NEXT_PTR_OFFSET).GetInt();
    JSTaggedValue value = GetElement(targetDataIndex);
    while (value.IsHole() && targetDataIndex != ELEMENTS_START_INDEX) {
        targetDataIndex = GetElement(targetDataIndex + NEXT_PTR_OFFSET).GetInt();
        value = GetElement(targetDataIndex);
    }
    if (targetDataIndex == ELEMENTS_START_INDEX) {
        return std::make_pair(-1, JSTaggedValue::Undefined());
    }
    return std::make_pair(targetDataIndex, value);
}

template<typename Derived>
JSTaggedValue TaggedList<Derived>::RemoveByIndex(JSThread *thread, const int &index)
{
    int prevDataIndex = FindPrevNodeByIndex(index);
    int curDataIndex = GetElement(prevDataIndex + NEXT_PTR_OFFSET).GetInt();
    JSTaggedValue data = GetElement(curDataIndex);
    RemoveNode(thread, prevDataIndex);
    return data;
}

// TaggedSingleList
JSTaggedValue TaggedSingleList::Create(const JSThread *thread, int numberOfElements)
{
    return TaggedList<TaggedSingleList>::Create(thread, numberOfElements).GetTaggedValue();
}

JSTaggedValue TaggedSingleList::Add(const JSThread *thread, const JSHandle<TaggedSingleList> &taggedList,
                                    const JSHandle<JSTaggedValue> &value)
{
    int prevDataIndex = taggedList->GetElement(TAIL_TABLE_INDEX).GetInt();
    return TaggedList<TaggedSingleList>::AddNode(thread, taggedList, value, -1, prevDataIndex);
}

JSTaggedValue TaggedSingleList::ConvertToArray(const JSThread *thread, const JSHandle<TaggedSingleList> &taggedList)
{
    return JSTaggedValue(TaggedList<TaggedSingleList>::TaggedListToArray(thread, taggedList));
}

JSTaggedValue TaggedSingleList::Insert(JSThread *thread, const JSHandle<TaggedSingleList> &taggedList,
                                       const JSHandle<JSTaggedValue> &value, const int index)
{
    int tailIndex = taggedList->GetElement(TAIL_TABLE_INDEX).GetInt();
    int prevDataIndex = (index == -1) ? tailIndex : taggedList->FindPrevNodeByIndex(index);
    return TaggedList<TaggedSingleList>::AddNode(thread, taggedList, value, index, prevDataIndex);
}

void TaggedSingleList::InsertNode(const JSThread *thread, const JSHandle<JSTaggedValue> &value, const int prevDataIndex,
                                  const int finalDataIndex)
{
    int prevNextIndex = prevDataIndex + NEXT_PTR_OFFSET;
    int nextDataIndex = GetElement(prevNextIndex).GetInt();
    SetElement(thread, prevNextIndex, JSTaggedValue(finalDataIndex));
    SetElement(thread, finalDataIndex, value.GetTaggedValue());
    SetElement(thread, finalDataIndex + 1, JSTaggedValue(nextDataIndex));
    SetNumberOfNodes(thread, NumberOfNodes() + 1);
}

bool TaggedSingleList::Has(const JSTaggedValue &element)
{
    int dataIndex = FindIndexByElement(element);
    return dataIndex != -1;
}

bool TaggedSingleList::IsEmpty() const
{
    return NumberOfNodes() == 0;
}

JSTaggedValue TaggedSingleList::Get(const int index)
{
    return FindElementByIndex(index);
}

std::pair<int, JSTaggedValue> TaggedSingleList::GetByDataIndex(const int dataIndex)
{
    return FindElementByDataIndex(dataIndex);
}

int TaggedSingleList::GetIndexOf(const JSTaggedValue &element)
{
    return FindIndexByElement(element);
}

int TaggedSingleList::GetLastIndexOf(const JSTaggedValue &element)
{
    return FindLastIndexByElement(element);
}

JSTaggedValue TaggedSingleList::Set(JSThread *thread, const JSHandle<TaggedSingleList> &taggedList,
                                    const int index, const JSHandle<JSTaggedValue> &value)
{
    int dataIndex = taggedList->FindDataIndexByNodeIndex(index);
    if (dataIndex == -1) {
        return taggedList.GetTaggedValue();
    }
    taggedList->SetElement(thread, dataIndex, value.GetTaggedValue());
    return taggedList.GetTaggedValue();
}

JSTaggedValue TaggedSingleList::ReplaceAllElements(JSThread *thread, const JSHandle<JSTaggedValue> &thisHandle,
                                                   const JSHandle<JSTaggedValue> &callbackFn,
                                                   const JSHandle<JSTaggedValue> &thisArg,
                                                   const JSHandle<TaggedSingleList> &taggedList)
{
    int length = taggedList->NumberOfNodes();
    int dataIndex = ELEMENTS_START_INDEX;
    JSHandle<JSTaggedValue> undefined = thread->GlobalConstants()->GetHandledUndefined();
    for (int k = 0; k < length; k++) {
        dataIndex = taggedList->GetNextDataIndex(dataIndex);
        JSTaggedValue kValue = taggedList->GetElement(dataIndex);
        JSTaggedValue key = JSTaggedValue(k);
        EcmaRuntimeCallInfo *info =
            EcmaInterpreter::NewRuntimeCallInfo(thread, callbackFn, thisArg, undefined, 3); // 3:three args
        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
        info->SetCallArg(kValue, key, thisHandle.GetTaggedValue());
        JSTaggedValue funcResult = JSFunction::Call(info);
        RETURN_VALUE_IF_ABRUPT_COMPLETION(thread, funcResult);
        JSHandle<JSTaggedValue> funcResultValue = JSHandle<JSTaggedValue>(thread, funcResult);
        TaggedSingleList::Set(thread, taggedList, k, funcResultValue);
        RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
    }
    return JSTaggedValue::Undefined();
}

JSTaggedValue TaggedSingleList::Sort(JSThread *thread, const JSHandle<JSTaggedValue> &callbackFn,
                                     const JSHandle<TaggedSingleList> &taggedList)
{
    int length = taggedList->NumberOfNodes();
    ASSERT(length > 0);
    JSMutableHandle<JSTaggedValue> presentValue(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> middleValue(thread, JSTaggedValue::Undefined());
    JSMutableHandle<JSTaggedValue> previousValue(thread, JSTaggedValue::Undefined());
    // create index map
    int* nodeIndexMapToDataIndex = new int[length];
    taggedList->MapNodeIndexToDataIndex(nodeIndexMapToDataIndex, length);
    
    int beginIndex = 0;
    int endIndex = 0;
    int middleIndex = 0;
    int compareResult = 0;
    for (int i = 1; i < length; i++) {
        beginIndex = 0;
        endIndex = i;
        presentValue.Update(taggedList->GetElement(nodeIndexMapToDataIndex[i]));
        while (beginIndex < endIndex) {
            middleIndex = (beginIndex + endIndex) / 2; // 2 : half
            middleValue.Update(taggedList->GetElement(nodeIndexMapToDataIndex[middleIndex]));
            compareResult = base::ArrayHelper::SortCompare(thread, callbackFn, middleValue, presentValue);
            RETURN_EXCEPTION_IF_ABRUPT_COMPLETION(thread);
            if (compareResult > 0) {
                endIndex = middleIndex;
            } else {
                beginIndex = middleIndex + 1;
            }
        }

        if (endIndex < i) {
            for (int j = i; j > endIndex; j--) {
                previousValue.Update(taggedList->GetElement(nodeIndexMapToDataIndex[j - 1]));
                taggedList->SetElement(thread, nodeIndexMapToDataIndex[j], previousValue.GetTaggedValue());
            }
            taggedList->SetElement(thread, nodeIndexMapToDataIndex[endIndex], presentValue.GetTaggedValue());
        }
    }
    
    delete[] nodeIndexMapToDataIndex;
    return JSTaggedValue::Undefined();
}

JSTaggedValue TaggedSingleList::GetSubList(JSThread *thread, const JSHandle<TaggedSingleList> &taggedList,
                                           const int fromIndex, const int toIndex,
                                           const JSHandle<TaggedSingleList> &subList)
{
    int dataIndex = taggedList->FindDataIndexByNodeIndex(fromIndex);
    int endIndex = taggedList->FindDataIndexByNodeIndex(toIndex);
    int preDataIndex = ELEMENTS_START_INDEX;
    int num = 0;
    JSMutableHandle<JSTaggedValue> dataHandle(thread, JSTaggedValue::Undefined());
    JSHandle<JSTaggedValue> nextHandle(thread, JSTaggedValue(ELEMENTS_START_INDEX));
    while (dataIndex != endIndex) {
        int elementDataIndex = ELEMENTS_START_INDEX + TaggedSingleList::ENTRY_SIZE + num * TaggedSingleList::ENTRY_SIZE;
        dataHandle.Update(taggedList->GetElement(dataIndex));
        subList->SetElement(thread, preDataIndex + NEXT_PTR_OFFSET, JSTaggedValue(elementDataIndex));
        subList->SetElement(thread, elementDataIndex, dataHandle.GetTaggedValue());
        subList->SetElement(thread, elementDataIndex + NEXT_PTR_OFFSET, nextHandle.GetTaggedValue());
        subList->SetElement(thread, TAIL_TABLE_INDEX, JSTaggedValue(elementDataIndex));
        subList->SetNumberOfNodes(thread, num + 1);
        dataIndex = taggedList->GetElement(dataIndex + NEXT_PTR_OFFSET).GetInt();
        preDataIndex = elementDataIndex;
        num++;
    }
    subList->SetNumberOfDeletedNodes(thread, 0);

    return subList.GetTaggedValue();
}

JSTaggedValue TaggedSingleList::Equal(const JSHandle<TaggedSingleList> &compareList)
{
    int compareListLength = compareList->NumberOfNodes();
    if (compareListLength != NumberOfNodes()) {
        return JSTaggedValue::False();
    }
    int nodeSum = 0;
    int compareNode = ELEMENTS_START_INDEX;
    int valueNode = ELEMENTS_START_INDEX;
    while (nodeSum < compareListLength) {
        compareNode = compareList->GetNextDataIndex(compareNode);
        valueNode = GetNextDataIndex(valueNode);
        JSTaggedValue compareValue = compareList->GetElement(compareNode);
        JSTaggedValue value = GetElement(valueNode);
        if (!JSTaggedValue::SameValue(compareValue, value)) {
            return JSTaggedValue::False();
        }
        nodeSum++;
    }
    return JSTaggedValue::True();
}

void TaggedSingleList::Clear(const JSThread *thread)
{
    TaggedList<TaggedSingleList>::Clear(thread);
}

JSTaggedValue TaggedSingleList::RemoveByIndex(JSThread *thread, const int &index)
{
    return TaggedList<TaggedSingleList>::RemoveByIndex(thread, index);
}

JSTaggedValue TaggedSingleList::Remove(JSThread *thread, const JSTaggedValue &element)
{
    int prevDataIndex = FindPrevNodeByValue(element);
    if (prevDataIndex == -1) {
        return JSTaggedValue::False();
    }
    RemoveNode(thread, prevDataIndex);
    return JSTaggedValue::True();
}

JSHandle<TaggedArray> TaggedSingleList::OwnKeys(JSThread *thread, const JSHandle<TaggedSingleList> &taggedList)
{
    return TaggedList<TaggedSingleList>::OwnKeys(thread, taggedList);
}

// TaggedDoubleList
JSTaggedValue TaggedDoubleList::Create(const JSThread *thread, int numberOfElements)
{
    JSHandle<TaggedDoubleList> taggedList = TaggedList<TaggedDoubleList>::Create(thread, numberOfElements);
    taggedList->SetElement(thread, ELEMENTS_START_INDEX + PREV_PTR_OFFSET, JSTaggedValue(ELEMENTS_START_INDEX));
    return taggedList.GetTaggedValue();
}

JSTaggedValue TaggedDoubleList::Add(const JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList,
                                    const JSHandle<JSTaggedValue> &value)
{
    int prevDataIndex = taggedList->GetElement(TAIL_TABLE_INDEX).GetInt();
    return TaggedList<TaggedDoubleList>::AddNode(thread, taggedList, value, -1, prevDataIndex);
}

JSTaggedValue TaggedDoubleList::AddFirst(const JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList,
                                         const JSHandle<JSTaggedValue> &value)
{
    int prevDataIndex = taggedList->FindPrevNodeByIndex(0);
    return TaggedList<TaggedDoubleList>::AddNode(thread, taggedList, value, 0, prevDataIndex);
}

JSTaggedValue TaggedDoubleList::ConvertToArray(const JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList)
{
    return JSTaggedValue(TaggedList<TaggedDoubleList>::TaggedListToArray(thread, taggedList));
}

JSTaggedValue TaggedDoubleList::Insert(JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList,
                                       const JSHandle<JSTaggedValue> &value, const int index)
{
    int prevDataIndex = taggedList->GetPrevNode(index);
    return TaggedList<TaggedDoubleList>::AddNode(thread, taggedList, value, index, prevDataIndex);
}

void TaggedDoubleList::InsertNode(const JSThread *thread, const JSHandle<JSTaggedValue> &value, const int prevDataIndex,
                                  const int finalDataIndex)
{
    int prevNextIndex = prevDataIndex + NEXT_PTR_OFFSET;
    int nextDataIndex = GetElement(prevNextIndex).GetInt();
    int nextPrevIndex = nextDataIndex + PREV_PTR_OFFSET;
    SetElement(thread, prevNextIndex, JSTaggedValue(finalDataIndex));
    SetElement(thread, nextPrevIndex, JSTaggedValue(finalDataIndex));
    SetElement(thread, finalDataIndex, value.GetTaggedValue());
    SetElement(thread, finalDataIndex + NEXT_PTR_OFFSET, JSTaggedValue(nextDataIndex));
    SetElement(thread, finalDataIndex + PREV_PTR_OFFSET, JSTaggedValue(prevDataIndex));
    SetNumberOfNodes(thread, NumberOfNodes() + 1);
}

bool TaggedDoubleList::Has(const JSTaggedValue &element)
{
    int dataIndex = FindIndexByElement(element);
    return dataIndex != -1;
}

JSTaggedValue TaggedDoubleList::Get(const int index)
{
    int len = NumberOfNodes();
    // 2 : 2 MEANS the half
    if (len / 2 > index) {
        return FindElementByIndex(index);
    } else {
        return FindElementByIndexAtLast(index);
    }
}

std::pair<int, JSTaggedValue> TaggedDoubleList::GetByDataIndex(const int dataIndex)
{
    return FindElementByDataIndex(dataIndex);
}

int TaggedDoubleList::GetPrevNode(const int index)
{
    int prevDataIndex = 0;
    int len = NumberOfNodes();
    // When index < (len / 2), search doubleList from the beginning
    if ((len / 2) > index) {
        return FindPrevNodeByIndex(index);
    } else {
        int leftNodeLen = len - 1 - index;
        // When insert at last
        if (leftNodeLen == -1) {
            return prevDataIndex = GetElement(TAIL_TABLE_INDEX).GetInt();
        }
        // when index >= (len / 2), search doubleList from the end
        return FindPrevNodeByIndexAtLast(leftNodeLen);
    }
}

int TaggedDoubleList::GetIndexOf(const JSTaggedValue &element)
{
    return FindIndexByElement(element);
}

int TaggedDoubleList::GetLastIndexOf(const JSTaggedValue &element)
{
    return FindLastIndexByElement(element);
}

JSTaggedValue TaggedDoubleList::Set(JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList, const int index,
                                    const JSHandle<JSTaggedValue> &value)
{
    int nodeLength = taggedList->NumberOfNodes();
    if (index < 0 || index >= nodeLength) {
        THROW_RANGE_ERROR_AND_RETURN(thread, "Set index out-of-bounds", JSTaggedValue::Exception());
    }
    int dataIndex = taggedList->FindDataIndexByNodeIndex(index);
    if (dataIndex == -1) {
        THROW_RANGE_ERROR_AND_RETURN(thread, "Set index not exist", JSTaggedValue::Exception());
    }
    taggedList->SetElement(thread, dataIndex, value.GetTaggedValue());
    return taggedList.GetTaggedValue();
}

void TaggedDoubleList::Clear(const JSThread *thread)
{
    TaggedList<TaggedDoubleList>::Clear(thread);
    SetElement(thread, ELEMENTS_START_INDEX + PREV_PTR_OFFSET, JSTaggedValue(ELEMENTS_START_INDEX));
}

JSTaggedValue TaggedDoubleList::RemoveFirst(JSThread *thread)
{
    int firstDataIndex = GetElement(ELEMENTS_START_INDEX + NEXT_PTR_OFFSET).GetInt();
    JSTaggedValue firstData = GetElement(firstDataIndex);
    RemoveNode(thread, ELEMENTS_START_INDEX);
    return firstData;
}

JSTaggedValue TaggedDoubleList::RemoveLast(JSThread *thread)
{
    int lastDataIndex = GetElement(ELEMENTS_START_INDEX + 2).GetInt();
    int prevDataIndex = GetElement(lastDataIndex + 2).GetInt();
    JSTaggedValue lastData = GetElement(lastDataIndex);
    RemoveNode(thread, prevDataIndex);
    return lastData;
}

JSTaggedValue TaggedDoubleList::RemoveByIndex(JSThread *thread, const int &index)
{
    int prevDataIndex = GetPrevNode(index);
    int curDataIndex = GetElement(prevDataIndex + NEXT_PTR_OFFSET).GetInt();
    JSTaggedValue data = GetElement(curDataIndex);
    RemoveNode(thread, prevDataIndex);
    return data;
}

JSTaggedValue TaggedDoubleList::Remove(JSThread *thread, const JSTaggedValue &element)
{
    int prevDataIndex = FindPrevNodeByValue(element);
    if (prevDataIndex == -1) {
        return JSTaggedValue::False();
    }
    RemoveNode(thread, prevDataIndex);
    return JSTaggedValue::True();
}

JSTaggedValue TaggedDoubleList::RemoveFirstFound(JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList,
                                                 const JSTaggedValue &element)
{
    int prevDataIndex = taggedList->FindPrevNodeByValue(element);
    if (prevDataIndex == -1) {
        THROW_RANGE_ERROR_AND_RETURN(thread, "this element is not exist in this container", JSTaggedValue::Exception());
    }
    taggedList->RemoveNode(thread, prevDataIndex);
    return JSTaggedValue::True();
}

JSTaggedValue TaggedDoubleList::RemoveLastFound(JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList,
                                                const JSTaggedValue &element)
{
    int prevDataIndex = taggedList->FindPrevNodeByValueAtLast(element);
    if (prevDataIndex == -1) {
        THROW_RANGE_ERROR_AND_RETURN(thread, "this element is not exist in this container", JSTaggedValue::Exception());
    }
    taggedList->RemoveNode(thread, prevDataIndex);
    return JSTaggedValue::True();
}

JSHandle<TaggedArray> TaggedDoubleList::OwnKeys(JSThread *thread, const JSHandle<TaggedDoubleList> &taggedList)
{
    return TaggedList<TaggedDoubleList>::OwnKeys(thread, taggedList);
}

JSTaggedValue TaggedDoubleList::FindElementByIndexAtLast(int index) const
{
    int dataIndex = ELEMENTS_START_INDEX;
    int preDataIndex = GetElement(dataIndex + PREV_PTR_OFFSET).GetInt();
    int nodeSum = GetElement(NUMBER_OF_NODE_INDEX).GetInt() - 1;
    while (preDataIndex != ELEMENTS_START_INDEX) {
        dataIndex = preDataIndex;
        JSTaggedValue dataValue = GetElement(dataIndex);
        if (nodeSum == index) {
            return dataValue;
        }
        preDataIndex = GetElement(preDataIndex + PREV_PTR_OFFSET).GetInt();
        nodeSum--;
    }
    return JSTaggedValue::Undefined();
}

int TaggedDoubleList::FindPrevNodeByIndexAtLast(const int index) const
{
    int prevDataIndex = GetElement(ELEMENTS_START_INDEX + PREV_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    int len = GetElement(NUMBER_OF_NODE_INDEX).GetInt();
    while (nodeSum <= len) {
        int prePreDataIndex = GetElement(prevDataIndex + PREV_PTR_OFFSET).GetInt();
        if (nodeSum == index) {
            return prePreDataIndex;
        }
        prevDataIndex = prePreDataIndex;
        nodeSum++;
    }
    return -1;
}

int TaggedDoubleList::FindPrevNodeByValueAtLast(const JSTaggedValue &element)
{
    int prevDataIndex = GetElement(ELEMENTS_START_INDEX + PREV_PTR_OFFSET).GetInt();
    int nodeSum = 0;
    int len = GetElement(NUMBER_OF_NODE_INDEX).GetInt();
    while (nodeSum <= len) {
        int prePreDataIndex = GetElement(prevDataIndex + PREV_PTR_OFFSET).GetInt();
        JSTaggedValue data = GetElement(prevDataIndex);
        if (JSTaggedValue::SameValue(data, element)) {
            return prePreDataIndex;
        }
        prevDataIndex = prePreDataIndex;
        nodeSum++;
    }
    return -1;
}
} // namespace panda::ecmascript