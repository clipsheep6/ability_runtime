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

#ifndef ECMASCRIPT_COMPILER_NEW_OBJECT_STUB_BUILDER_H
#define ECMASCRIPT_COMPILER_NEW_OBJECT_STUB_BUILDER_H
#include "ecmascript/compiler/stub_builder.h"

namespace panda::ecmascript::kungfu {
class NewObjectStubBuilder : public StubBuilder {
public:
    explicit NewObjectStubBuilder(StubBuilder *parent)
        : StubBuilder(parent) {}
    explicit NewObjectStubBuilder(Environment *env)
        : StubBuilder(env) {}
    ~NewObjectStubBuilder() = default;
    NO_MOVE_SEMANTIC(NewObjectStubBuilder);
    NO_COPY_SEMANTIC(NewObjectStubBuilder);
    void GenerateCircuit() override {}

    void SetParameters(GateRef glue, GateRef size)
    {
        glue_ = glue;
        size_ = size;
    }

    void NewLexicalEnv(Variable *result, Label *exit, GateRef numSlots, GateRef parent);
    void NewJSObject(Variable *result, Label *exit, GateRef hclass);
    void NewArgumentsList(Variable *result, Label *exit, GateRef sp, GateRef startIdx, GateRef numArgs);
    void NewArgumentsObj(Variable *result, Label *exit, GateRef argumentsList, GateRef numArgs);
    void AllocLineStringObject(Variable *result, Label *exit, GateRef length, bool compressed);
    void HeapAlloc(Variable *result, Label *exit, RegionSpaceFlag spaceType);
    void NewJSArrayLiteral(Variable *result, Label *exit, RegionSpaceFlag spaceType, GateRef obj, GateRef hclass,
                           bool isEmptyArray);
    void InitializeWithSpeicalValue(Label *exit, GateRef object, GateRef value, GateRef start, GateRef end);
private:
    void AllocateInYoung(Variable *result, Label *exit);
    void InitializeTaggedArrayWithSpeicalValue(Label *exit,
        GateRef array, GateRef value, GateRef start, GateRef length);
    GateRef glue_ {0};
    GateRef size_ {0};
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_NEW_OBJECT_STUB_BUILDER_H