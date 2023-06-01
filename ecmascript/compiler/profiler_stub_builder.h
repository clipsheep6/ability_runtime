/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_COMPILER_PROFILER_STUB_BUILDER_H
#define ECMASCRIPT_COMPILER_PROFILER_STUB_BUILDER_H

#include "ecmascript/compiler/profiler_operation.h"
#include "ecmascript/compiler/stub_builder.h"

namespace panda::ecmascript::kungfu {
class ProfilerStubBuilder : public StubBuilder {
public:
    explicit ProfilerStubBuilder(StubBuilder *parent) : StubBuilder(parent) {}
    explicit ProfilerStubBuilder(Environment *env) : StubBuilder(env) {}
    ~ProfilerStubBuilder() override = default;
    NO_MOVE_SEMANTIC(ProfilerStubBuilder);
    NO_COPY_SEMANTIC(ProfilerStubBuilder);
    void GenerateCircuit() override {}

    void PGOProfiler(
        GateRef glue, GateRef pc, GateRef func, GateRef profileTypeInfo, GateRef value, OperationType type);
    void ProfileCall(GateRef glue, GateRef func);
    void ProfileOpType(GateRef glue, GateRef pc, GateRef func, GateRef profileTypeInfo, GateRef type);
    void ProfileDefineClass(GateRef glue, GateRef pc, GateRef func, GateRef constructor);
    void ProfileObjLayout(GateRef glue, GateRef pc, GateRef func, GateRef receiver, GateRef store);

    GateRef UpdateTrackTypeInPropAttr(GateRef attr, GateRef value, ProfileOperation callback);
    void UpdatePropAttrIC(GateRef glue, GateRef receiver, GateRef value, GateRef handler, ProfileOperation callback);
    void UpdatePropAttrWithValue(GateRef glue, GateRef receiver, GateRef layout, GateRef attr, GateRef attrIndex,
        GateRef value, ProfileOperation callback);

private:
    GateRef TaggedToTrackType(GateRef value);
};
} // namespace panda::ecmascript::kungfu
#endif // ECMASCRIPT_COMPILER_PROFILER_STUB_BUILDER_H
