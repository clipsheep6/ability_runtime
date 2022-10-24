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

#ifndef ECMASCRIPT_COMPILER_BUILTINS_CONTAINERS_STUB_BUILDER_H
#define ECMASCRIPT_COMPILER_BUILTINS_CONTAINERS_STUB_BUILDER_H
#include "ecmascript/compiler/builtins/containers_deque_stub_builder.h"
#include "ecmascript/compiler/builtins/containers_plainarray_stub_builder.h"
#include "ecmascript/compiler/builtins/containers_queue_stub_builder.h"
#include "ecmascript/compiler/builtins/containers_stack_stub_builder.h"
#include "ecmascript/compiler/builtins/containers_vector_stub_builder.h"
#include "ecmascript/compiler/builtins/builtins_stubs.h"
#include "ecmascript/js_api/js_api_vector.h"

namespace panda::ecmascript::kungfu {
// enumerate container functions that use function call
enum class ContainersType : uint8_t {
    VECTOR_FOREACH = 0,
    VECTOR_REPLACEALLELEMENTS,
    STACK_FOREACH,
    PLAINARRAY_FOREACH,
    QUEUE_FOREACH,
    DEQUE_FOREACH,
};

class ContainersStubBuilder : public BuiltinsStubBuilder {
public:
    explicit ContainersStubBuilder(StubBuilder *parent)
        : BuiltinsStubBuilder(parent) {}
    ~ContainersStubBuilder() = default;
    NO_MOVE_SEMANTIC(ContainersStubBuilder);
    NO_COPY_SEMANTIC(ContainersStubBuilder);
    void GenerateCircuit() override {}

    void ContainersCommonFuncCall(GateRef glue, GateRef thisValue, GateRef numArgs,
        Variable* result, Label *exit, Label *slowPath, ContainersType type);

    void QueueCommonFuncCall(GateRef glue, GateRef thisValue, GateRef numArgs,
        Variable* result, Label *exit, Label *slowPath, ContainersType type);

    void DequeCommonFuncCall(GateRef glue, GateRef thisValue, GateRef numArgs,
        Variable* result, Label *exit, Label *slowPath, ContainersType type);

    GateRef IsContainer(GateRef obj, ContainersType type)
    {
        switch (type) {
            case ContainersType::VECTOR_FOREACH:
            case ContainersType::VECTOR_REPLACEALLELEMENTS:
                return IsJSAPIVector(obj);
            case ContainersType::STACK_FOREACH:
                return IsJSAPIStack(obj);
            case ContainersType::PLAINARRAY_FOREACH:
                return IsJSAPIPlainArray(obj);
            case ContainersType::QUEUE_FOREACH:
                return IsJSAPIQueue(obj);
            case ContainersType::DEQUE_FOREACH:
                return IsJSAPIDeque(obj);
            default:
                UNREACHABLE();
        }
        return False();
    }

    bool IsReplaceAllElements(ContainersType type)
    {
        switch (type) {
            case ContainersType::STACK_FOREACH:
            case ContainersType::VECTOR_FOREACH:
            case ContainersType::PLAINARRAY_FOREACH:
            case ContainersType::QUEUE_FOREACH:
            case ContainersType::DEQUE_FOREACH:
                return false;
            case ContainersType::VECTOR_REPLACEALLELEMENTS:
                return true;
            default:
                UNREACHABLE();
        }
        return false;
    }

    bool IsPlainArray(ContainersType type)
    {
        switch (type) {
            case ContainersType::STACK_FOREACH:
            case ContainersType::VECTOR_FOREACH:
            case ContainersType::VECTOR_REPLACEALLELEMENTS:
            case ContainersType::QUEUE_FOREACH:
            case ContainersType::DEQUE_FOREACH:
                return false;
            case ContainersType::PLAINARRAY_FOREACH:
                return true;
            default:
                UNREACHABLE();
        }
        return false;
    }

    void ContainerSet(GateRef glue, GateRef obj, GateRef index, GateRef value, ContainersType type)
    {
        ContainersVectorStubBuilder vectorBuilder(this);
        switch (type) {
            case ContainersType::VECTOR_REPLACEALLELEMENTS:
                vectorBuilder.Set(glue, obj, index, value);
                break;
            default:
                UNREACHABLE();
        }
    }

    GateRef ContainerGetSize(GateRef obj, ContainersType type)
    {
        switch (type) {
            case ContainersType::VECTOR_FOREACH:
            case ContainersType::VECTOR_REPLACEALLELEMENTS: {
                ContainersVectorStubBuilder vectorBuilder(this);
                return vectorBuilder.GetSize(obj);
            }
            case ContainersType::STACK_FOREACH: {
                ContainersStackStubBuilder stackBuilder(this);
                return stackBuilder.GetSize(obj);
            }
            case ContainersType::PLAINARRAY_FOREACH: {
                ContainersPlainArrayStubBuilder plainArrayBuilder(this);
                return plainArrayBuilder.GetSize(obj);
            }
            case ContainersType::QUEUE_FOREACH: {
                ContainersQueueStubBuilder queueBuilder(this);
                return queueBuilder.GetArrayLength(obj);
            }
            case ContainersType::DEQUE_FOREACH: {
                ContainersDequeStubBuilder dequeBuilder(this);
                return dequeBuilder.GetSize(obj);
            }
            default:
                UNREACHABLE();
        }
        return False();
    }

    GateRef ContainerGetValue(GateRef obj, GateRef index, ContainersType type)
    {
        switch (type) {
            case ContainersType::VECTOR_FOREACH:
            case ContainersType::VECTOR_REPLACEALLELEMENTS: {
                ContainersVectorStubBuilder vectorBuilder(this);
                return vectorBuilder.Get(obj, index);
            }
            case ContainersType::STACK_FOREACH: {
                ContainersStackStubBuilder stackBuilder(this);
                return stackBuilder.Get(obj, index);
            }
            case ContainersType::PLAINARRAY_FOREACH: {
                ContainersPlainArrayStubBuilder plainArrayBuilder(this);
                return plainArrayBuilder.Get(obj, index);
            }
            case ContainersType::QUEUE_FOREACH: {
                ContainersQueueStubBuilder queueBuilder(this);
                return queueBuilder.Get(obj, index);
            }
            case ContainersType::DEQUE_FOREACH: {
                ContainersDequeStubBuilder dequeBuilder(this);
                return dequeBuilder.Get(obj, index);
            }
            default:
                UNREACHABLE();
        }
        return False();
    }
    GateRef PlainArrayGetKey(GateRef obj, GateRef index)
    {
        ContainersPlainArrayStubBuilder plainArrayBuilder(this);
        return plainArrayBuilder.GetKey(obj, index);
    }

    GateRef QueueGetNextPosition(GateRef obj, GateRef index)
    {
        ContainersQueueStubBuilder queueBuilder(this);
        return queueBuilder.GetNextPosition(obj, index);
    }

    GateRef QueueGetCurrentFront(GateRef obj)
    {
        ContainersQueueStubBuilder queueBuilder(this);
        return queueBuilder.GetCurrentFront(obj);
    }
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BUILTINS_CONTAINERS_STUB_BUILDER_H