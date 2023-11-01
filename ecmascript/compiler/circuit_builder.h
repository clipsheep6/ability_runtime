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

#ifndef ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H
#define ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H

#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/assembler/assembler.h"
#include "ecmascript/compiler/builtins/builtins_call_signature.h"
#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/call_signature.h"
#include "ecmascript/compiler/gate.h"
#include "ecmascript/compiler/gate_accessor.h"
#include "ecmascript/compiler/variable_type.h"
#include "ecmascript/global_env_constants.h"
#include "ecmascript/jspandafile/constpool_value.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_runtime_options.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/tagged_array.h"

namespace panda::ecmascript::kungfu {
using namespace panda::ecmascript;
#define DEFVAlUE(varname, cirBuilder, type, val) \
        Variable varname(cirBuilder, type, cirBuilder->NextVariableId(), val)

class BuiltinsStringStubBuilder;
class CompilationConfig;
class Environment;
class Label;
class NTypeBytecodeLowering;
class SlowPathLowering;
class StubBuilder;
class TypeBytecodeLowering;
class Variable;

#define BINARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(V)                    \
    V(Int16Add, Add, MachineType::I16)                                    \
    V(Int32Add, Add, MachineType::I32)                                    \
    V(Int64Add, Add, MachineType::I64)                                    \
    V(DoubleAdd, Add, MachineType::F64)                                   \
    V(PtrAdd, Add, MachineType::ARCH)                                     \
    V(Int16Sub, Sub, MachineType::I16)                                    \
    V(Int32Sub, Sub, MachineType::I32)                                    \
    V(Int64Sub, Sub, MachineType::I64)                                    \
    V(DoubleSub, Sub, MachineType::F64)                                   \
    V(PtrSub, Sub, MachineType::ARCH)                                     \
    V(Int32Mul, Mul, MachineType::I32)                                    \
    V(Int64Mul, Mul, MachineType::I64)                                    \
    V(DoubleMul, Mul, MachineType::F64)                                   \
    V(PtrMul, Mul, MachineType::ARCH)                                     \
    V(Int32Div, Sdiv, MachineType::I32)                                   \
    V(Int64Div, Sdiv, MachineType::I64)                                   \
    V(DoubleDiv, Fdiv, MachineType::F64)                                  \
    V(Int32Mod, Smod, MachineType::I32)                                   \
    V(DoubleMod, Smod, MachineType::F64)                                  \
    V(BoolAnd, And, MachineType::I1)                                      \
    V(Int8And, And, MachineType::I8)                                      \
    V(Int32And, And, MachineType::I32)                                    \
    V(Int64And, And, MachineType::I64)                                    \
    V(BoolOr, Or, MachineType::I1)                                        \
    V(Int32Or, Or, MachineType::I32)                                      \
    V(Int64Or, Or, MachineType::I64)                                      \
    V(Int32Xor, Xor, MachineType::I32)                                    \
    V(Int64Xor, Xor, MachineType::I64)                                    \
    V(Int16LSL, Lsl, MachineType::I16)                                    \
    V(Int32LSL, Lsl, MachineType::I32)                                    \
    V(Int64LSL, Lsl, MachineType::I64)                                    \
    V(Int8LSR, Lsr, MachineType::I8)                                      \
    V(Int32LSR, Lsr, MachineType::I32)                                    \
    V(Int64LSR, Lsr, MachineType::I64)                                    \
    V(Int32ASR, Asr, MachineType::I32)                                    \
    V(Int64ASR, Asr, MachineType::I64)

#define UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(V)                  \
    V(BoolNot, Rev, MachineType::I1)                                   \
    V(Int32Not, Rev, MachineType::I32)                                 \
    V(Int64Not, Rev, MachineType::I64)                                 \
    V(CastDoubleToInt64, Bitcast, MachineType::I64)                    \
    V(CastInt64ToFloat64, Bitcast, MachineType::F64)                   \
    V(CastInt32ToFloat32, Bitcast, MachineType::F32)                   \
    V(SExtInt32ToInt64, Sext, MachineType::I64)                        \
    V(SExtInt1ToInt64, Sext, MachineType::I64)                         \
    V(SExtInt1ToInt32, Sext, MachineType::I32)                         \
    V(ZExtInt8ToInt16, Zext, MachineType::I16)                         \
    V(ZExtInt32ToInt64, Zext, MachineType::I64)                        \
    V(ZExtInt1ToInt64, Zext, MachineType::I64)                         \
    V(ZExtInt1ToInt32, Zext, MachineType::I32)                         \
    V(ZExtInt8ToInt32, Zext, MachineType::I32)                         \
    V(ZExtInt8ToInt64, Zext, MachineType::I64)                         \
    V(ZExtInt8ToPtr, Zext, MachineType::ARCH)                          \
    V(ZExtInt16ToPtr, Zext, MachineType::ARCH)                         \
    V(ZExtInt32ToPtr, Zext, MachineType::ARCH)                         \
    V(SExtInt32ToPtr, Sext, MachineType::ARCH)                         \
    V(ZExtInt16ToInt32, Zext, MachineType::I32)                        \
    V(ZExtInt16ToInt64, Zext, MachineType::I64)                        \
    V(TruncInt16ToInt8, Trunc, MachineType::I8)                        \
    V(TruncInt64ToInt32, Trunc, MachineType::I32)                      \
    V(TruncPtrToInt32, Trunc, MachineType::I32)                        \
    V(TruncInt64ToInt1, Trunc, MachineType::I1)                        \
    V(TruncInt64ToInt16, Trunc, MachineType::I16)                      \
    V(TruncInt32ToInt1, Trunc, MachineType::I1)                        \
    V(TruncInt32ToInt8, Trunc, MachineType::I8)                        \
    V(TruncInt32ToInt16, Trunc, MachineType::I16)                      \
    V(TruncFloatToInt64, TruncFloatToInt64, MachineType::I64)          \
    V(TruncFloatToInt32, TruncFloatToInt32, MachineType::I32)          \
    V(ExtFloat32ToDouble, Fext, MachineType::F64)                      \
    V(TruncDoubleToFloat32, Ftrunc, MachineType::F32)                  \
    V(ChangeInt32ToFloat64, SignedIntToFloat, MachineType::F64)        \
    V(ChangeInt32ToFloat32, SignedIntToFloat, MachineType::F32)        \
    V(ChangeUInt32ToFloat64, UnsignedIntToFloat, MachineType::F64)     \
    V(ChangeFloat64ToInt32, FloatToSignedInt, MachineType::I32)        \
    V(SExtInt16ToInt64, Sext, MachineType::I64)                        \
    V(SExtInt16ToInt32, Sext, MachineType::I32)                        \
    V(SExtInt8ToInt32, Sext, MachineType::I32)                         \
    V(SExtInt8ToInt64, Sext, MachineType::I64)

#define UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH_PRIVATE(V)          \
    V(ChangeTaggedPointerToInt64, TaggedToInt64, MachineType::I64)

#define BINARY_CMP_METHOD_LIST_WITHOUT_BITWIDTH(V)                                      \
    V(DoubleLessThan, Fcmp, static_cast<BitField>(FCmpCondition::OLT))                  \
    V(DoubleLessThanOrEqual, Fcmp, static_cast<BitField>(FCmpCondition::OLE))           \
    V(DoubleGreaterThan, Fcmp, static_cast<BitField>(FCmpCondition::OGT))               \
    V(DoubleGreaterThanOrEqual, Fcmp, static_cast<BitField>(FCmpCondition::OGE))        \
    V(DoubleEqual, Fcmp, static_cast<BitField>(FCmpCondition::OEQ))                     \
    V(DoubleNotEqual, Fcmp, static_cast<BitField>(FCmpCondition::ONE))                  \
    V(Int32LessThan, Icmp, static_cast<BitField>(ICmpCondition::SLT))                   \
    V(Int32LessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SLE))            \
    V(Int32GreaterThan, Icmp, static_cast<BitField>(ICmpCondition::SGT))                \
    V(Int32GreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SGE))         \
    V(Int32UnsignedLessThan, Icmp, static_cast<BitField>(ICmpCondition::ULT))           \
    V(Int32UnsignedLessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::ULE))    \
    V(Int32UnsignedGreaterThan, Icmp, static_cast<BitField>(ICmpCondition::UGT))        \
    V(Int32UnsignedGreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::UGE)) \
    V(Int64LessThan, Icmp, static_cast<BitField>(ICmpCondition::SLT))                   \
    V(Int64LessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SLE))            \
    V(Int64GreaterThan, Icmp, static_cast<BitField>(ICmpCondition::SGT))                \
    V(Int64GreaterThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::SGE))         \
    V(Int64UnsignedLessThanOrEqual, Icmp, static_cast<BitField>(ICmpCondition::ULE))

class CircuitBuilder {
public:
    explicit CircuitBuilder(Circuit *circuit) : circuit_(circuit), acc_(circuit) {}
    CircuitBuilder(Circuit *circuit, CompilationConfig *cmpCfg)
        : circuit_(circuit), acc_(circuit), cmpCfg_(cmpCfg)
    {
    }
    ~CircuitBuilder() = default;
    NO_MOVE_SEMANTIC(CircuitBuilder);
    NO_COPY_SEMANTIC(CircuitBuilder);
    
    // ************************************************************* Share IR **********************************************************************************
    GateRef Arguments(size_t index);
    GateRef DefaultCase(GateRef switchBranch);
    GateRef DependRelay(GateRef state, GateRef depend);
    GateRef DeoptCheck(GateRef condition, GateRef frameState, DeoptType type);
    GateRef GetElementsArray(GateRef object);
    GateRef GetSuperConstructor(GateRef ctor);
    GateRef Merge(const std::vector<GateRef> &inList);
    GateRef Selector(OpCode opcode, MachineType machineType, GateRef control, const std::vector<GateRef> &values,
        int valueCounts, VariableType type = VariableType::VOID());
    GateRef Selector(OpCode opcode, GateRef control, const std::vector<GateRef> &values,
        int valueCounts, VariableType type = VariableType::VOID());
    GateRef Return(GateRef state, GateRef depend, GateRef value);
    GateRef ReturnVoid(GateRef state, GateRef depend);
    GateRef Goto(GateRef state);
    GateRef LoopBegin(GateRef state);
    GateRef LoopEnd(GateRef state);
    GateRef LoopExit(GateRef state);
    GateRef LoopExitDepend(GateRef state, GateRef depend);
    GateRef LoopExitValue(GateRef state, GateRef value);
    GateRef IfTrue(GateRef ifBranch);
    GateRef IfFalse(GateRef ifBranch);
    GateRef IsJsCOWArray(GateRef obj);
    GateRef IsCOWArray(GateRef objectType);
    GateRef IsTaggedArray(GateRef object);
    GateRef SwitchCase(GateRef switchBranch, int64_t value);
    GateRef Int8(int8_t val);
    GateRef Int16(int16_t val);
    GateRef Int32(int32_t value);
    GateRef Int64(int64_t value);
    GateRef IntPtr(int64_t val);
    GateRef StringPtr(std::string_view str);
    GateRef StringPtr(const std::string &str);
    GateRef Boolean(bool value);
    GateRef Double(double value);
    GateRef UndefineConstant();
    GateRef HoleConstant();
    GateRef NullPtrConstant();
    GateRef NullConstant();
    GateRef ExceptionConstant();
    void ClearConstantCache(GateRef gate);
    GateRef NanValue();
    GateRef RelocatableData(uint64_t val);
    GateRef Branch(GateRef state, GateRef condition, uint32_t leftWeight = 1, uint32_t rightWeight = 1,
                   const char* comment = nullptr);  // 1: default branch weight
    GateRef SwitchBranch(GateRef state, GateRef index, int caseCounts);
    void AppendFrameArgs(std::vector<GateRef> &args, GateRef hirGate);
    inline GateRef True();
    inline GateRef False();
    inline GateRef Undefined();
    inline GateRef Hole();

    // Get
    GateRef GetConstPool(GateRef jsFunc);
    GateRef GetConstPoolFromFunction(GateRef jsFunc);
    GateRef GetCodeAddr(GateRef method);
    GateRef GetObjectFromConstPool(GateRef glue, GateRef hirGate, GateRef jsFunc, GateRef index, ConstPoolType type);
    GateRef GetObjectFromConstPool(GateRef glue, GateRef hirGate, GateRef constPool, GateRef module, GateRef index,
                                   ConstPoolType type);
    GateRef GetFunctionLexicalEnv(GateRef function);
    GateRef GetGlobalEnv();
    GateRef GetGlobalEnvObj(GateRef env, size_t index);
    GateRef GetGlobalEnvObjHClass(GateRef env, size_t index);
    GateRef GetGlobalConstantValue(ConstantIndex index);
    GateRef GetGlobalEnvValue(VariableType type, GateRef env, size_t index);
    GateRef GetGlobalObject(GateRef glue);
    GateRef GetMethodFromFunction(GateRef function);
    GateRef GetModuleFromFunction(GateRef function);
    GateRef GetHomeObjectFromFunction(GateRef function);
    inline GateRef GetExpectedNumOfArgs(GateRef method);
    inline GateRef GetGlobalConstantOffset(ConstantIndex index); // shareir
    GateRef GetEmptyArray(GateRef glue);
    GateRef GetPrototypeFromHClass(GateRef hClass);
    GateRef GetEnumCacheFromHClass(GateRef hClass);
    GateRef GetProtoChangeMarkerFromHClass(GateRef hClass);
    GateRef GetLengthFromForInIterator(GateRef iter);
    GateRef GetIndexFromForInIterator(GateRef iter);
    GateRef GetKeysFromForInIterator(GateRef iter);
    GateRef GetObjectFromForInIterator(GateRef iter);
    GateRef GetCachedHclassFromForInIterator(GateRef iter);
    void SetLengthOfForInIterator(GateRef glue, GateRef iter, GateRef length);
    void SetIndexOfForInIterator(GateRef glue, GateRef iter, GateRef index);
    void SetKeysOfForInIterator(GateRef glue, GateRef iter, GateRef keys);
    void SetObjectOfForInIterator(GateRef glue, GateRef iter, GateRef object);
    void SetCachedHclassOfForInIterator(GateRef glue, GateRef iter, GateRef hclass);
    void IncreaseInteratorIndex(GateRef glue, GateRef iter, GateRef index);
    GateRef GetHasChanged(GateRef object);
    GateRef HasDeleteProperty(GateRef hClass);
    GateRef IsEcmaObject(GateRef obj);

    // Set
    void SetLexicalEnvToFunction(GateRef glue, GateRef function, GateRef value);
    void SetHomeObjectToFunction(GateRef glue, GateRef function, GateRef value);

    inline GateRef LogicAnd(GateRef x, GateRef y);
    inline GateRef LogicOr(GateRef x, GateRef y);
    GateRef FunctionIsResolved(GateRef function);
    GateRef HasPendingException(GateRef glue); // shareir
    GateRef IsUtf16String(GateRef string);

    // label related
    void NewEnvironment(GateRef hir);
    void DeleteCurrentEnvironment();
    inline int NextVariableId();
    inline void HandleException(GateRef result, Label *success, Label *exception, Label *exit);
    inline void HandleException(GateRef result, Label *success, Label *fail, Label *exit, GateRef exceptionVal);
    inline void SubCfgEntry(Label *entry);
    inline void SubCfgExit();
    inline GateRef Return(GateRef value);
    inline GateRef Return();
    inline void Bind(Label *label);
    inline void Bind(Label *label, bool justSlowPath);
    void Jump(Label *label);
    void Branch(GateRef condition, Label *trueLabel, Label *falseLabel,
                uint32_t trueWeight = 1, uint32_t falseWeight = 1);   // 1: default branch weight
    void Switch(GateRef index, Label *defaultLabel, int64_t *keysValue, Label *keysLabel, int numberOfKeys);
    void LoopBegin(Label *loopHead);
    void LoopEnd(Label *loopHead);
    void LoopExit(const std::vector<Variable*> &vars, size_t diff = 1);
    inline Label *GetCurrentLabel() const;
    inline GateRef GetState() const;
    inline GateRef GetDepend() const;
    inline StateDepend GetStateDepend() const;

    Circuit *GetCircuit() const
    {
        return circuit_;
    }

    void SetEnvironment(Environment *env)
    {
        env_ = env;
    }
    Environment *GetCurrentEnvironment() const
    {
        return env_;
    }
    void SetCompilationConfig(CompilationConfig *cmpCfg)
    {
        cmpCfg_ = cmpCfg;
    }
    CompilationConfig *GetCompilationConfig() const
    {
        return cmpCfg_;
    }

    // ************************************************************* High IR **********************************************************************************
    GateRef CreateArray(ElementsKind kind, uint32_t arraySize);
    GateRef CreateArrayWithBuffer(ElementsKind kind, ArrayMetaDataAccessor::Mode mode,
                                  GateRef constPoolIndex, GateRef elementIndex);
    GateRef Construct(GateRef hirGate, std::vector<GateRef> args);
    GateRef TypedCallNative(GateRef hirGate, GateRef thisObj, GateRef funcId);
    GateRef IsBase(GateRef ctor);
    GateRef ToLength(GateRef receiver);

    inline GateRef GetMethodId(GateRef func);
    inline GateRef IsAOTLiteralInfo(GateRef x);

    // call operation
    GateRef CallGetter(GateRef hirGate, GateRef receiver, GateRef propertyLookupResult, const char* comment = nullptr);
    GateRef CallSetter(GateRef hirGate, GateRef receiver, GateRef propertyLookupResult,
                       GateRef value, const char* comment = nullptr);
    GateRef CallBCHandler(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                          const char* comment = nullptr);
    GateRef CallBCDebugger(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                           const char* comment = nullptr);
    GateRef CallBuiltin(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                        const char* comment = nullptr);
    GateRef CallBuiltinWithArgv(GateRef glue, GateRef target, const std::vector<GateRef> &args,
                                const char* comment = nullptr);
    GateRef CallRuntimeVarargs(GateRef glue, int index, GateRef argc, GateRef argv, const char* comment = nullptr);
    GateRef CallRuntime(GateRef glue, int index, GateRef depend, const std::vector<GateRef> &args, GateRef hirGate,
                        const char* comment = nullptr);
    GateRef CallNGCRuntime(GateRef glue, GateRef gate, int index, const std::vector<GateRef> &args, bool useLabel);

    GateRef CallNGCRuntime(GateRef glue, int index, GateRef depend, const std::vector<GateRef> &args,
                           GateRef hirGate, const char* comment = nullptr);
    GateRef FastCallOptimized(GateRef glue, GateRef code, GateRef depend, const std::vector<GateRef> &args,
                              GateRef hirGate);
    GateRef CallOptimized(GateRef glue, GateRef code, GateRef depend, const std::vector<GateRef> &args,
                          GateRef hirGate);
    GateRef CallStub(GateRef glue, GateRef hirGate, int index, const std::vector<GateRef> &args,
                     const char* comment = nullptr);
    GateRef CallBuiltinRuntime(GateRef glue, GateRef depend, const std::vector<GateRef> &args,
                               bool isNew = false, const char* comment = nullptr);
    GateRef Call(const CallSignature* cs, GateRef glue, GateRef target, GateRef depend,
                 const std::vector<GateRef> &args, GateRef hirGate, const char* comment = nullptr);
    GateRef NoLabelCallRuntime(GateRef glue, GateRef depend, size_t index, std::vector<GateRef> &args, GateRef hirGate);

    void StartCallTimer(GateRef glue, GateRef gate, const std::vector<GateRef> &args, bool useLabel);
    void EndCallTimer(GateRef glue, GateRef gate, const std::vector<GateRef> &args, bool useLabel);
    GateRef GetCallBuiltinId(GateRef method);

    // FastCall
    inline GateRef CanFastCall(GateRef obj);
    inline GateRef CanFastCallWithBitField(GateRef bitfield);

    // Js world
    inline GateRef GetObjectType(GateRef hClass);
    inline GateRef HasConstructor(GateRef object);
    inline GateRef IsSpecial(GateRef x, JSTaggedType type);
    inline GateRef IsJSFunction(GateRef obj);
    inline GateRef IsJSFunctionWithBit(GateRef obj);
    inline GateRef IsDictionaryMode(GateRef object);
    inline GateRef IsJsType(GateRef object, JSType type);
    inline GateRef IsStableElements(GateRef hClass);
    inline GateRef IsStableArguments(GateRef hClass);
    inline GateRef IsStableArray(GateRef hClass);
    inline GateRef IsDictionaryElement(GateRef hClass);
    inline GateRef IsClassConstructor(GateRef object);
    inline GateRef IsClassConstructorWithBitField(GateRef bitfield);
    inline GateRef IsConstructor(GateRef object);
    inline GateRef IsClassPrototype(GateRef object);
    inline GateRef IsClassPrototypeWithBitField(GateRef object);
    inline GateRef IsExtensible(GateRef object);
    inline GateRef IsJSObject(GateRef obj);
    inline GateRef IsCallable(GateRef obj);
    inline GateRef IsCallableFromBitField(GateRef bitfield);
    GateRef IsJSHClass(GateRef obj);
    inline void StoreHClass(GateRef glue, GateRef object, GateRef hClass);

    // WeakRef
    inline GateRef CreateWeakRef(GateRef x);
    inline GateRef LoadObjectFromWeakRef(GateRef x);


    // hClass
    inline GateRef GetElementsKindByHClass(GateRef hClass);
    inline GateRef GetObjectSizeFromHClass(GateRef hClass);
    inline GateRef HasConstructorByHClass(GateRef hClass);
    inline GateRef IsDictionaryModeByHClass(GateRef hClass);
    inline GateRef LoadHClass(GateRef object);
    inline GateRef LoadHClassByConstOffset(GateRef object);
    inline GateRef LoadPrototype(GateRef hclass);
    inline GateRef LoadPrototypeHClass(GateRef object);
    void SetPropertyInlinedProps(GateRef glue, GateRef obj, GateRef hClass,
                                 GateRef value, GateRef attrOffset, VariableType type);


    // ************************************************************* Middle IR **********************************************************************************
    GateRef HeapObjectCheck(GateRef gate, GateRef frameState);
    GateRef StableArrayCheck(GateRef gate, ElementsKind kind, ArrayMetaDataAccessor::Mode mode);
    GateRef COWArrayCheck(GateRef gate);
    GateRef EcmaStringCheck(GateRef gate);
    GateRef FlattenTreeStringCheck(GateRef gate);
    GateRef HClassStableArrayCheck(GateRef gate, GateRef frameState, ArrayMetaDataAccessor accessor);
    GateRef ArrayGuardianCheck(GateRef frameState);
    GateRef TypedArrayCheck(GateType type, GateRef gate);
    GateRef LoadTypedArrayLength(GateType type, GateRef gate);
    GateRef RangeGuard(GateRef gate, uint32_t left, uint32_t right);
    GateRef BuiltinPrototypeHClassCheck(GateRef gate, BuiltinTypeId type);
    GateRef IndexCheck(GateType type, GateRef gate, GateRef index);
    GateRef ObjectTypeCheck(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex);
    GateRef ObjectTypeCompare(GateType type, bool isHeapObject, GateRef gate, GateRef hclassIndex);
    GateRef TryPrimitiveTypeCheck(GateType type, GateRef gate);
    GateRef CallTargetCheck(GateRef gate, GateRef function, GateRef id, GateRef param, const char* comment = nullptr);
    GateRef JSCallTargetFromDefineFuncCheck(GateType type, GateRef func, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    GateRef JSCallTargetTypeCheck(GateType type, GateRef func, GateRef methodIndex, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    GateRef JSCallThisTargetTypeCheck(GateType type, GateRef func, GateRef gate);
    template<TypedCallTargetCheckOp Op>
    inline GateRef JSNoGCCallThisTargetTypeCheck(GateType type, GateRef func, GateRef methodId, GateRef gate);
    GateRef TypeOfCheck(GateRef gate, GateType type);
    GateRef TypedTypeOf(GateType type);
    GateRef IteratorFunctionCheck(GateRef obj, GateRef kind);
    GateRef GetFixedIterator(GateRef obj, GateRef kind);
    GateRef NativeCallTargetCheck(GateRef func, GateRef funcId);
    GateRef TypedCallOperator(GateRef hirGate, MachineType type, const std::vector<GateRef>& inList);
    inline GateRef TypedCallBuiltin(GateRef hirGate, const std::vector<GateRef> &args, BuiltinsStubCSigns::ID id);
    GateRef TypeConvert(MachineType type, GateType typeFrom, GateType typeTo, const std::vector<GateRef>& inList);
    GateRef Int32CheckRightIsZero(GateRef right);
    GateRef Float64CheckRightIsZero(GateRef right);
    GateRef ValueCheckNegOverflow(GateRef value);
    GateRef OverflowCheck(GateRef value);
    GateRef LexVarIsHoleCheck(GateRef value);
    GateRef Int32UnsignedUpperBoundCheck(GateRef value, GateRef upperBound);
    GateRef Int32DivWithCheck(GateRef left, GateRef right);
    GateType GetGateTypeOfValueType(ValueType type);
    GateRef InsertStableArrayCheck(GateRef array);
    GateRef InsertLoadArrayLength(GateRef array, bool isTypedArray);
    GateRef InsertTypedArrayCheck(GateType type, GateRef array);
    GateRef InsertTypedBinaryop(GateRef left, GateRef right, GateType leftType, GateType rightType,
                                GateType gateType, PGOSampleType sampleType, TypedBinOp op);
    GateRef InsertRangeCheckPredicate(GateRef left, TypedBinOp cond, GateRef right);
    GateRef TypedConditionJump(MachineType type, TypedJumpOp jumpOp, uint32_t weight, GateType typeVal,
                               const std::vector<GateRef>& inList);
    GateRef TypedNewAllocateThis(GateRef ctor, GateRef hclassIndex, GateRef frameState);
    GateRef TypedSuperAllocateThis(GateRef superCtor, GateRef newTarget, GateRef frameState);
    template<TypedBinOp Op>
    inline GateRef TypedBinaryOp(GateRef x, GateRef y, GateType xType, GateType yType, GateType gateType,
                                 PGOSampleType sampleType);
    template<TypedUnOp Op>
    inline GateRef TypedUnaryOp(GateRef x, GateType xType, GateType gateType);
    template<TypedJumpOp Op>
    inline GateRef TypedConditionJump(GateRef x, GateType xType, uint32_t weight);
    GateRef Convert(GateRef gate, ValueType src, ValueType dst);
    GateRef ConvertBoolToTaggedBoolean(GateRef gate);
    GateRef ConvertTaggedBooleanToBool(GateRef gate);
    GateRef ConvertInt32ToBool(GateRef gate);
    GateRef ConvertFloat64ToBool(GateRef gate);
    GateRef ConvertInt32ToTaggedInt(GateRef gate);
    GateRef ConvertFloat64ToTaggedDouble(GateRef gate);
    GateRef ConvertFloat64ToInt32(GateRef gate);
    GateRef ConvertInt32ToFloat64(GateRef gate);
    GateRef ConvertBoolToInt32(GateRef gate, ConvertSupport support);
    GateRef ConvertBoolToFloat64(GateRef gate, ConvertSupport support);
    GateRef ConvertUInt32ToBool(GateRef gate);
    GateRef ConvertUInt32ToTaggedNumber(GateRef gate);
    GateRef ConvertUInt32ToFloat64(GateRef gate);
    GateRef ConvertCharToEcmaString(GateRef gate);
    GateRef CheckAndConvert(
        GateRef gate, ValueType src, ValueType dst, ConvertSupport support = ConvertSupport::ENABLE);
    GateRef ConvertHoleAsUndefined(GateRef receiver);
    GateRef CheckUInt32AndConvertToInt32(GateRef gate);
    GateRef CheckTaggedIntAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedDoubleAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedIntAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedDoubleAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedNumberAndConvertToBool(GateRef gate);
    GateRef CheckTaggedBooleanAndConvertToBool(GateRef gate);
    GateRef CheckNullAndConvertToInt32(GateRef gate);
    GateRef CheckTaggedBooleanAndConvertToInt32(GateRef gate);
    GateRef CheckNullAndConvertToFloat64(GateRef gate);
    GateRef CheckTaggedBooleanAndConvertToFloat64(GateRef gate);
    GateRef CheckUndefinedAndConvertToFloat64(GateRef gate);
    GateRef CheckUndefinedAndConvertToBool(GateRef gate);
    GateRef CheckNullAndConvertToBool(GateRef gate);
    GateRef CheckUndefinedAndConvertToInt32(GateRef gate);
    GateRef StartAllocate();
    GateRef FinishAllocate();

    inline GateRef PrimitiveToNumber(GateRef x, VariableType type);
    inline GateRef GetValueFromTaggedArray(GateRef array, GateRef index);
    template<TypedLoadOp Op>
    GateRef LoadElement(GateRef receiver, GateRef index);
    GateRef LoadProperty(GateRef receiver, GateRef propertyLookupResult, bool isFunction);
    GateRef LoadArrayLength(GateRef array);
    inline GateRef LoadFromTaggedArray(GateRef array, size_t index);
    GateRef LoadStringLength(GateRef string);
    GateRef LoadConstOffset(VariableType type, GateRef receiver, size_t offset);
    GateRef TypedCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC);
    GateRef TypedFastCall(GateRef hirGate, std::vector<GateRef> args, bool isNoGC);
    inline void SetValueToTaggedArray(VariableType valType, GateRef glue, GateRef array, GateRef index, GateRef val);
    GateRef StoreConstOffset(VariableType type, GateRef receiver, size_t offset, GateRef value);
    inline GateRef StoreToTaggedArray(GateRef array, size_t index, GateRef value);
    GateRef StringEqual(GateRef x, GateRef y);
    template<TypedStoreOp Op>
    GateRef StoreElement(GateRef receiver, GateRef index, GateRef value);
    GateRef StoreMemory(MemoryType Op, VariableType type, GateRef receiver, GateRef index, GateRef value);
    GateRef StoreProperty(GateRef receiver, GateRef propertyLookupResult, GateRef value);
    GateRef IsOptimizedAndNotFastCall(GateRef obj);
    GateRef IsOptimized(GateRef obj);
    GateRef IsOptimizedWithBitField(GateRef bitfield);
    GateRef ComputeTaggedArraySize(GateRef length);
    GateRef HeapAlloc(GateRef size, GateType type, RegionSpaceFlag flag);
    GateRef IsRegExpReplaceDetectorValid(GateRef glue);
    GateRef IsRegExpSplitDetectorValid(GateRef glue);
    GateRef IsMapIteratorDetectorValid(GateRef glue);
    GateRef IsSetIteratorDetectorValid(GateRef glue);
    GateRef IsStringIteratorDetectorValid(GateRef glue);
    GateRef IsArrayIteratorDetectorValid(GateRef glue);
    GateRef IsTypedArrayIteratorDetectorValid(GateRef glue);
    
    // bit operation
    inline GateRef TaggedIsInt(GateRef x);
    inline GateRef TaggedIsDouble(GateRef x);
    inline GateRef TaggedIsObject(GateRef x);
    inline GateRef TaggedIsNumber(GateRef x);
    inline GateRef TaggedIsNumeric(GateRef x);
    inline GateRef TaggedIsNotHole(GateRef x);
    inline GateRef TaggedIsHole(GateRef x);
    inline GateRef TaggedIsNullPtr(GateRef x);
    inline GateRef TaggedIsUndefined(GateRef x);
    inline GateRef TaggedIsException(GateRef x);
    inline GateRef TaggedIsSpecial(GateRef x);
    inline GateRef TaggedIsHeapObject(GateRef x);
    inline GateRef TaggedIsAsyncGeneratorObject(GateRef x);
    inline GateRef TaggedIsJSGlobalObject(GateRef x);
    inline GateRef TaggedIsGeneratorObject(GateRef x);
    inline GateRef TaggedIsJSArray(GateRef obj);
    inline GateRef TaggedIsPropertyBox(GateRef x);
    inline GateRef TaggedIsWeak(GateRef x);
    inline GateRef TaggedIsPrototypeHandler(GateRef x);
    inline GateRef TaggedIsTransitionHandler(GateRef x);
    inline GateRef TaggedIsStoreTSHandler(GateRef x);
    inline GateRef TaggedIsTransWithProtoHandler(GateRef x);
    inline GateRef TaggedIsUndefinedOrNull(GateRef x);
    inline GateRef TaggedIsTrue(GateRef x);
    inline GateRef TaggedIsFalse(GateRef x);
    inline GateRef TaggedIsNull(GateRef x);
    inline GateRef TaggedIsBoolean(GateRef x);
    inline GateRef TaggedIsBigInt(GateRef obj);
    inline GateRef TaggedIsString(GateRef obj);
    inline GateRef TaggedIsStringOrSymbol(GateRef obj);
    inline GateRef TaggedIsSymbol(GateRef obj);
    inline GateRef TaggedIsProtoChangeMarker(GateRef obj);
    inline GateRef TaggedIsJSMap(GateRef obj);
    inline GateRef TaggedIsJSSet(GateRef obj);
    inline GateRef TaggedIsTypedArray(GateRef obj);
    inline GateRef TaggedGetInt(GateRef x);
    inline GateRef TaggedObjectIsString(GateRef obj);
    inline GateRef TaggedObjectBothAreString(GateRef x, GateRef y);
    inline GateRef TaggedObjectIsEcmaObject(GateRef obj);
    inline GateRef TaggedTrue();
    inline GateRef TaggedFalse();

    // String
    inline GateRef BothAreString(GateRef x, GateRef y);
    inline GateRef IsTreeString(GateRef obj);
    inline GateRef IsSlicedString(GateRef obj);
    inline GateRef TreeStringIsFlat(GateRef string);
    inline GateRef GetFirstFromTreeString(GateRef string);
    inline GateRef GetSecondFromTreeString(GateRef string);
    GateRef GetLengthFromString(GateRef value);
    GateRef GetHashcodeFromString(GateRef glue, GateRef value);
    GateRef TryGetHashcodeFromString(GateRef string);

    // for in
    GateRef GetEnumCacheKind(GateRef glue, GateRef enumCache);
    GateRef IsEnumCacheValid(GateRef receiver, GateRef cachedHclass, GateRef kind);
    GateRef NeedCheckProperty(GateRef receiver);

    // ************************************************************* Low IR **********************************************************************************
    inline GateRef Equal(GateRef x, GateRef y, const char* comment = nullptr);
    inline GateRef NotEqual(GateRef x, GateRef y, const char* comment = nullptr);
    inline GateRef IntPtrDiv(GateRef x, GateRef y);
    inline GateRef IntPtrOr(GateRef x, GateRef y);
    inline GateRef IntPtrLSL(GateRef x, GateRef y);
    inline GateRef IntPtrLSR(GateRef x, GateRef y);
    inline GateRef Int64NotEqual(GateRef x, GateRef y);
    inline GateRef Int32NotEqual(GateRef x, GateRef y);
    inline GateRef Int64Equal(GateRef x, GateRef y);
    inline GateRef Int8Equal(GateRef x, GateRef y);
    inline GateRef Int32Equal(GateRef x, GateRef y);
    inline GateRef IntPtrGreaterThan(GateRef x, GateRef y);
    GateRef AddWithOverflow(GateRef left, GateRef right);
    GateRef SubWithOverflow(GateRef left, GateRef right);
    GateRef MulWithOverflow(GateRef left, GateRef right);
    GateRef ExtractValue(MachineType mt, GateRef pointer, GateRef index);
    GateRef Sqrt(GateRef param);
    MachineType GetMachineTypeOfValueType(ValueType type);
    GateRef Alloca(size_t size);
    GateRef ReadSp();
    GateRef BinaryArithmetic(const GateMetaData* meta, MachineType machineType, GateRef left,
                             GateRef right, GateType gateType = GateType::Empty(), const char* comment = nullptr);
    GateRef BinaryCmp(const GateMetaData* meta, GateRef left, GateRef right, const char* comment = nullptr);
    GateRef Load(VariableType type, GateRef base, GateRef offset);
    GateRef Load(VariableType type, GateRef base, GateRef offset, GateRef depend);
    void Store(VariableType type, GateRef glue, GateRef base, GateRef offset, GateRef value);
    void StoreWithNoBarrier(VariableType type, GateRef base, GateRef offset, GateRef value);

    // cast operation
    inline GateRef GetInt64OfTInt(GateRef x);
    inline GateRef GetInt32OfTInt(GateRef x);
    inline GateRef TaggedCastToIntPtr(GateRef x);
    inline GateRef GetDoubleOfTDouble(GateRef x);
    inline GateRef GetBooleanOfTBoolean(GateRef x);
    inline GateRef GetDoubleOfTNumber(GateRef x);
    inline GateRef DoubleToInt(GateRef x, Label *exit);
    inline GateRef Int32ToTaggedPtr(GateRef x);
    inline GateRef Int64ToTaggedPtr(GateRef x);
    inline GateRef Int32ToTaggedInt(GateRef x);
    inline GateRef ToTaggedInt(GateRef x);
    inline GateRef ToTaggedIntPtr(GateRef x);
    inline GateRef DoubleToTaggedDoublePtr(GateRef x);
    inline GateRef BooleanToTaggedBooleanPtr(GateRef x);
    inline GateRef BooleanToInt32(GateRef x);
    inline GateRef BooleanToFloat64(GateRef x);
    inline GateRef Float32ToTaggedDoublePtr(GateRef x);
    inline GateRef TaggedDoublePtrToFloat32(GateRef x);
    inline GateRef TaggedIntPtrToFloat32(GateRef x);
    inline GateRef DoubleToTaggedDouble(GateRef x);
    inline GateRef DoubleToTagged(GateRef x);
    inline GateRef DoubleIsNAN(GateRef x);
    inline GateRef DoubleIsINF(GateRef x);
    static MachineType GetMachineTypeFromVariableType(VariableType type);

    // Unary / BinaryOp
    template<OpCode Op, MachineType Type>
    inline GateRef BinaryOp(GateRef x, GateRef y);
    template<OpCode Op, MachineType Type>
    inline GateRef BinaryOpWithOverflow(GateRef x, GateRef y);

#define ARITHMETIC_BINARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                                        \
    inline GateRef NAME(GateRef x, GateRef y, GateType type = GateType::Empty(), const char* comment = nullptr)  \
    {                                                                                                            \
        return BinaryArithmetic(circuit_->OPCODEID(), MACHINETYPEID, x, y, type, comment);                       \
    }

    BINARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(ARITHMETIC_BINARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_BINARY_OP_WITH_BITWIDTH

#define ARITHMETIC_UNARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                                     \
    inline GateRef NAME(GateRef x, const char* comment = nullptr)                                            \
    {                                                                                                        \
        return circuit_->NewGate(circuit_->OPCODEID(), MACHINETYPEID, { x }, GateType::NJSValue(), comment); \
    }

    UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH(ARITHMETIC_UNARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_UNARY_OP_WITH_BITWIDTH

#define CMP_BINARY_OP_WITHOUT_BITWIDTH(NAME, OPCODEID, CONDITION)                                \
    inline GateRef NAME(GateRef x, GateRef y, const char* comment = nullptr)                     \
    {                                                                                            \
        return BinaryCmp(circuit_->OPCODEID(static_cast<uint64_t>(CONDITION)), x, y, comment);   \
    }

    BINARY_CMP_METHOD_LIST_WITHOUT_BITWIDTH(CMP_BINARY_OP_WITHOUT_BITWIDTH)
#undef CMP_BINARY_OP_WITHOUT_BITWIDTH

private:
    static constexpr uint32_t GATE_TWO_VALUESIN = 2;

    inline void SetDepend(GateRef depend);
    inline void SetState(GateRef state);

#define ARITHMETIC_UNARY_OP_WITH_BITWIDTH(NAME, OPCODEID, MACHINETYPEID)                                     \
    inline GateRef NAME(GateRef x, const char* comment = nullptr)                                            \
    {                                                                                                        \
        return circuit_->NewGate(circuit_->OPCODEID(), MACHINETYPEID, { x }, GateType::NJSValue(), comment); \
    }

    UNARY_ARITHMETIC_METHOD_LIST_WITH_BITWIDTH_PRIVATE(ARITHMETIC_UNARY_OP_WITH_BITWIDTH)
#undef ARITHMETIC_UNARY_OP_WITH_BITWIDTH

    Circuit *circuit_ {nullptr};
    GateAccessor acc_;
    Environment *env_ {nullptr};
    CompilationConfig *cmpCfg_ {nullptr};
    friend StubBuilder;
    friend BuiltinsStringStubBuilder;
    friend TypeBytecodeLowering;
    friend NTypeBytecodeLowering;
    friend SlowPathLowering;
};

}  // namespace panda::ecmascript::kungfu

#endif  // ECMASCRIPT_COMPILER_CIRCUIT_BUILDER_H
