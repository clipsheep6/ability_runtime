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

#ifndef ECMASCRIPT_COMPILER_INTERPRETER_STUB_DEFINE_H
#define ECMASCRIPT_COMPILER_INTERPRETER_STUB_DEFINE_H

namespace panda::ecmascript::kungfu {
#define INTERPRETER_STUB_HELPER_LIST(V) \
    V(AsmInterpreterEntry, 7)           \
    V(SingleStepDebugging, 7)

#define IGNORE_STUB(...)

#define INTERPRETER_STUB_LIST(V) \
    ASM_INTERPRETER_STUB_LIST(IGNORE_STUB, V)

#define ASM_INTERPRETER_ID_LIST(V) \
    ASM_INTERPRETER_STUB_LIST(V, V)

#define ASM_INTERPRETER_STUB_LIST(V, T)                     \
    T(HandleLdNanPref, 7)                                   \
    T(HandleLdInfinityPref, 7)                              \
    T(HandleLdGlobalThisPref, 7)                            \
    T(HandleLdUndefinedPref, 7)                             \
    T(HandleLdNullPref, 7)                                  \
    T(HandleLdSymbolPref, 7)                                \
    T(HandleLdGlobalPref, 7)                                \
    T(HandleLdTruePref, 7)                                  \
    T(HandleLdFalsePref, 7)                                 \
    T(HandleThrowDynPref, 7)                                \
    T(HandleTypeOfDynPref, 7)                               \
    T(HandleLdLexEnvDynPref, 7)                             \
    T(HandlePopLexEnvDynPref, 7)                            \
    T(HandleGetUnmappedArgsPref, 7)                         \
    T(HandleGetPropIteratorPref, 7)                         \
    T(HandleAsyncFunctionEnterPref, 7)                      \
    T(HandleLdHolePref, 7)                                  \
    T(HandleReturnUndefinedPref, 7)                         \
    T(HandleCreateEmptyObjectPref, 7)                       \
    T(HandleCreateEmptyArrayPref, 7)                        \
    T(HandleGetIteratorPref, 7)                             \
    T(HandleThrowThrowNotExistsPref, 7)                     \
    T(HandleThrowPatternNonCoerciblePref, 7)                \
    T(HandleLdHomeObjectPref, 7)                            \
    T(HandleThrowDeleteSuperPropertyPref, 7)                \
    T(HandleDebuggerPref, 7)                                \
    T(HandleAdd2DynPrefV8, 7)                               \
    T(HandleSub2DynPrefV8, 7)                               \
    T(HandleMul2DynPrefV8, 7)                               \
    T(HandleDiv2DynPrefV8, 7)                               \
    T(HandleMod2DynPrefV8, 7)                               \
    T(HandleEqDynPrefV8, 7)                                 \
    T(HandleNotEqDynPrefV8, 7)                              \
    T(HandleLessDynPrefV8, 7)                               \
    T(HandleLessEqDynPrefV8, 7)                             \
    T(HandleGreaterDynPrefV8, 7)                            \
    T(HandleGreaterEqDynPrefV8, 7)                          \
    T(HandleShl2DynPrefV8, 7)                               \
    T(HandleShr2DynPrefV8, 7)                               \
    T(HandleAshr2DynPrefV8, 7)                              \
    T(HandleAnd2DynPrefV8, 7)                               \
    T(HandleOr2DynPrefV8, 7)                                \
    T(HandleXOr2DynPrefV8, 7)                               \
    T(HandleToNumberPrefV8, 7)                              \
    T(HandleNegDynPrefV8, 7)                                \
    T(HandleNotDynPrefV8, 7)                                \
    T(HandleIncDynPrefV8, 7)                                \
    T(HandleDecDynPrefV8, 7)                                \
    T(HandleExpDynPrefV8, 7)                                \
    T(HandleIsInDynPrefV8, 7)                               \
    T(HandleInstanceOfDynPrefV8, 7)                         \
    T(HandleStrictNotEqDynPrefV8, 7)                        \
    T(HandleStrictEqDynPrefV8, 7)                           \
    T(HandleResumeGeneratorPrefV8, 7)                       \
    T(HandleGetResumeModePrefV8, 7)                         \
    T(HandleCreateGeneratorObjPrefV8, 7)                    \
    T(HandleThrowConstAssignmentPrefV8, 7)                  \
    T(HandleGetTemplateObjectPrefV8, 7)                     \
    T(HandleGetNextPropNamePrefV8, 7)                       \
    V(HandleCallArg0DynPrefV8, 7)                           \
    T(HandleThrowIfNotObjectPrefV8, 7)                      \
    T(HandleIterNextPrefV8, 7)                              \
    T(HandleCloseIteratorPrefV8, 7)                         \
    T(HandleCopyModulePrefV8, 7)                            \
    T(HandleSuperCallSpreadPrefV8, 7)                       \
    T(HandleDelObjPropPrefV8V8, 7)                          \
    T(HandleNewObjSpreadDynPrefV8V8, 7)                     \
    T(HandleCreateIterResultObjPrefV8V8, 7)                 \
    T(HandleSuspendGeneratorPrefV8V8, 7)                    \
    T(HandleAsyncFunctionAwaitUncaughtPrefV8V8, 7)          \
    T(HandleThrowUndefinedIfHolePrefV8V8, 7)                \
    V(HandleCallArg1DynPrefV8V8, 7)                         \
    T(HandleCopyDataPropertiesPrefV8V8, 7)                  \
    T(HandleStArraySpreadPrefV8V8, 7)                       \
    T(HandleGetIteratorNextPrefV8V8, 7)                     \
    T(HandleSetObjectWithProtoPrefV8V8, 7)                  \
    T(HandleLdObjByValuePrefV8V8, 7)                        \
    T(HandleStObjByValuePrefV8V8, 7)                        \
    T(HandleStOwnByValuePrefV8V8, 7)                        \
    T(HandleLdSuperByValuePrefV8V8, 7)                      \
    T(HandleStSuperByValuePrefV8V8, 7)                      \
    T(HandleLdObjByIndexPrefV8Imm32, 7)                     \
    T(HandleStObjByIndexPrefV8Imm32, 7)                     \
    T(HandleStOwnByIndexPrefV8Imm32, 7)                     \
    T(HandleCallSpreadDynPrefV8V8V8, 7)                     \
    T(HandleAsyncFunctionResolvePrefV8V8V8, 7)              \
    T(HandleAsyncFunctionRejectPrefV8V8V8, 7)               \
    V(HandleCallArgs2DynPrefV8V8V8, 7)                      \
    V(HandleCallArgs3DynPrefV8V8V8V8, 7)                    \
    T(HandleDefineGetterSetterByValuePrefV8V8V8V8, 7)       \
    T(HandleNewObjDynRangePrefImm16V8, 7)                   \
    V(HandleCallIRangeDynPrefImm16V8, 7)                    \
    V(HandleCallIThisRangeDynPrefImm16V8, 7)                \
    T(HandleSuperCallPrefImm16V8, 7)                        \
    T(HandleCreateObjectWithExcludedKeysPrefImm16V8V8, 7)   \
    T(HandleDefineFuncDynPrefId16Imm16V8, 7)                \
    T(HandleDefineNCFuncDynPrefId16Imm16V8, 7)              \
    T(HandleDefineGeneratorFuncPrefId16Imm16V8, 7)          \
    T(HandleDefineAsyncFuncPrefId16Imm16V8, 7)              \
    T(HandleDefineMethodPrefId16Imm16V8, 7)                 \
    T(HandleNewLexEnvDynPrefImm16, 7)                       \
    T(HandleCopyRestArgsPrefImm16, 7)                       \
    T(HandleCreateArrayWithBufferPrefImm16, 7)              \
    T(HandleCreateObjectHavingMethodPrefImm16, 7)           \
    T(HandleThrowIfSuperNotCorrectCallPrefImm16, 7)         \
    T(HandleCreateObjectWithBufferPrefImm16, 7)             \
    T(HandleLdLexVarDynPrefImm4Imm4, 7)                     \
    T(HandleLdLexVarDynPrefImm8Imm8, 7)                     \
    T(HandleLdLexVarDynPrefImm16Imm16, 7)                   \
    T(HandleStLexVarDynPrefImm4Imm4V8, 7)                   \
    T(HandleStLexVarDynPrefImm8Imm8V8, 7)                   \
    T(HandleStLexVarDynPrefImm16Imm16V8, 7)                 \
    T(HandleDefineClassWithBufferPrefId16Imm16Imm16V8V8, 7) \
    T(HandleGetModuleNamespacePrefId32, 7)                  \
    T(HandleStModuleVarPrefId32, 7)                         \
    T(HandleTryLdGlobalByNamePrefId32, 7)                   \
    T(HandleTryStGlobalByNamePrefId32, 7)                   \
    T(HandleLdGlobalVarPrefId32, 7)                         \
    T(HandleStGlobalVarPrefId32, 7)                         \
    T(HandleLdObjByNamePrefId32V8, 7)                       \
    T(HandleStObjByNamePrefId32V8, 7)                       \
    T(HandleStOwnByNamePrefId32V8, 7)                       \
    T(HandleLdSuperByNamePrefId32V8, 7)                     \
    T(HandleStSuperByNamePrefId32V8, 7)                     \
    T(HandleLdModuleVarPrefId32Imm8, 7)                     \
    T(HandleCreateRegExpWithLiteralPrefId32Imm8, 7)         \
    T(HandleIsTruePref, 7)                                  \
    T(HandleIsFalsePref, 7)                                 \
    T(HandleStConstToGlobalRecordPrefId32, 7)               \
    T(HandleStLetToGlobalRecordPrefId32, 7)                 \
    T(HandleStClassToGlobalRecordPrefId32, 7)               \
    T(HandleStOwnByValueWithNameSetPrefV8V8, 7)             \
    T(HandleStOwnByNameWithNameSetPrefId32V8, 7)            \
    T(HandleLdFunctionPref, 7)                              \
    V(HandleNewLexEnvWithNameDynPrefImm16Imm16, 7)          \
    T(HandleMovDynV8V8, 7)                                  \
    T(HandleMovDynV16V16, 7)                                \
    T(HandleLdaStrId32, 7)                                  \
    T(HandleLdaiDynImm32, 7)                                \
    T(HandleFldaiDynImm64, 7)                               \
    T(HandleJmpImm8, 7)                                     \
    T(HandleJmpImm16, 7)                                    \
    T(HandleJmpImm32, 7)                                    \
    T(HandleJeqzImm8, 7)                                    \
    T(HandleJeqzImm16, 7)                                   \
    T(HandleLdaDynV8, 7)                                    \
    T(HandleStaDynV8, 7)                                    \
    T(HandleReturnDyn, 7)                                   \
    T(HandleMovV4V4, 7)                                     \
    T(HandleJnezImm8, 7)                                    \
    T(HandleJnezImm16, 7)                                   \
    T(ExceptionHandler, 7)                                  \

enum InterpreterStubId {
#define DEF_STUB(name, counter) name##Id,
    ASM_INTERPRETER_ID_LIST(DEF_STUB) INTERPRETER_STUB_MAXCOUNT,
#undef DEF_STUB
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_INTERPRETER_STUB_DEFINE_H
