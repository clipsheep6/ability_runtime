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

#ifndef ECMASCRIPT_COMPILER_LCR_LOWERING_H
#define ECMASCRIPT_COMPILER_LCR_LOWERING_H

#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/circuit_builder-inl.h"
#include "ecmascript/compiler/gate_accessor.h"

namespace panda::ecmascript::kungfu {
class LCRLowering {
public:
    LCRLowering(Circuit *circuit, CompilationConfig *cmpCfg,
                        bool enableLog, const std::string& name)
        : circuit_(circuit), acc_(circuit), builder_(circuit, cmpCfg),
          enableLog_(enableLog), methodName_(name), glue_(acc_.GetGlueFromArgList())
    {
    }
    ~LCRLowering() = default;

    bool IsLogEnabled() const
    {
        return enableLog_;
    }
    void Run();
    StateDepend LowerCheckAndConvert(StateDepend stateDepend, GateRef gate, GateRef frameState);
    StateDepend LowerConvert(StateDepend stateDepend, GateRef gate);
private:
    const std::string& GetMethodName() const
    {
        return methodName_;
    }

    void DeleteStateSplit(GateRef gate);
    void LowerArrayGuardianCheck(GateRef gate);
    void LowerHeapObjectCheck(GateRef gate);
    void LowerHClassStableArrayCheck(GateRef gate);
    void LowerGetConstPool(GateRef gate);
    void LowerLoadConstOffset(GateRef gate);
    void LowerStoreConstOffset(GateRef gate);
    void LowerConvertHoleAsUndefined(GateRef gate);
    void LowerCheckTaggedIntAndConvert(GateRef gate, GateRef frameState);
    void LowerCheckTaggedDoubleAndConvert(GateRef gate, GateRef frameState, Label *exit);
    void LowerCheckTaggedNumberAndConvert(GateRef gate, GateRef frameState, Label *exit);
    void LowerCheckTaggedBoolAndConvert(GateRef gate, GateRef frameState);
    void LowerCheckSupportAndConvert(GateRef gate, GateRef frameState);
    void LowerGetGlobalEnv(GateRef gate);
    void LowerGetGlobalEnvObjHClass(GateRef gate);
    void LowerGetGlobalConstantValue(GateRef gate);
    void LowerHeapAllocate(GateRef gate);
    void LowerInt32CheckRightIsZero(GateRef gate);
    void LowerFloat64CheckRightIsZero(GateRef gate);
    void LowerValueCheckNegOverflow(GateRef gate);
    void LowerNegativeIndexCheck(GateRef gate);
    void LowerLargeIndexCheck(GateRef gate);
    void LowerOverflowCheck(GateRef gate);
    void LowerInt32UnsignedUpperBoundCheck(GateRef gate);
    void LowerInt32DivWithCheck(GateRef gate);

    GateRef ConvertBoolToTaggedBoolean(GateRef gate);
    GateRef ConvertInt32ToFloat64(GateRef gate);
    GateRef ConvertInt32ToTaggedInt(GateRef gate);
    GateRef ConvertFloat64ToBool(GateRef gate);
    GateRef ConvertFloat64ToInt32(GateRef gate, Label *exit);
    GateRef ConvertFloat64ToTaggedDouble(GateRef gate);
    GateRef ConvertTaggedIntToInt32(GateRef gate);
    GateRef ConvertTaggedIntToFloat64(GateRef gate);
    GateRef ConvertTaggedDoubleToInt32(GateRef gate, Label *exit);
    GateRef ConvertTaggedDoubleToFloat64(GateRef gate);
    GateRef ConvertTaggedNumberToBool(GateRef gate, Label *exit);
    GateRef ConvertTaggedNumberToInt32(GateRef gate, Label *exit);
    GateRef ConvertTaggedNumberToFloat64(GateRef gate, Label *exit);
    GateRef ConvertTaggedBooleanToBool(GateRef gate);
    void HeapAllocateInYoung(GateRef gate);
    void InitializeWithSpeicalValue(Label *exit, GateRef object, GateRef glue, GateRef value,
                                    GateRef start, GateRef end);

    Circuit *circuit_;
    GateAccessor acc_;
    CircuitBuilder builder_;
    bool enableLog_ {false};
    std::string methodName_;
    GateRef glue_ {Circuit::NullGate()};
};
}  // panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_LCR_LOWERING_H
