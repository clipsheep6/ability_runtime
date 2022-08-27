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

        &&DEBUG_HANDLE_LDNAN_PREF,
        &&DEBUG_HANDLE_LDINFINITY_PREF,
        &&DEBUG_HANDLE_LDGLOBALTHIS_PREF,
        &&DEBUG_HANDLE_LDUNDEFINED_PREF,
        &&DEBUG_HANDLE_LDNULL_PREF,
        &&DEBUG_HANDLE_LDSYMBOL_PREF,
        &&DEBUG_HANDLE_LDGLOBAL_PREF,
        &&DEBUG_HANDLE_LDTRUE_PREF,
        &&DEBUG_HANDLE_LDFALSE_PREF,
        &&DEBUG_HANDLE_THROWDYN_PREF,
        &&DEBUG_HANDLE_TYPEOFDYN_PREF,
        &&DEBUG_HANDLE_LDLEXENVDYN_PREF,
        &&DEBUG_HANDLE_POPLEXENVDYN_PREF,
        &&DEBUG_HANDLE_GETUNMAPPEDARGS_PREF,
        &&DEBUG_HANDLE_GETPROPITERATOR_PREF,
        &&DEBUG_HANDLE_ASYNCFUNCTIONENTER_PREF,
        &&DEBUG_HANDLE_LDHOLE_PREF,
        &&DEBUG_HANDLE_RETURNUNDEFINED_PREF,
        &&DEBUG_HANDLE_CREATEEMPTYOBJECT_PREF,
        &&DEBUG_HANDLE_CREATEEMPTYARRAY_PREF,
        &&DEBUG_HANDLE_GETITERATOR_PREF,
        &&DEBUG_HANDLE_THROWTHROWNOTEXISTS_PREF,
        &&DEBUG_HANDLE_THROWPATTERNNONCOERCIBLE_PREF,
        &&DEBUG_HANDLE_LDHOMEOBJECT_PREF,
        &&DEBUG_HANDLE_THROWDELETESUPERPROPERTY_PREF,
        &&DEBUG_HANDLE_DEBUGGER_PREF,
        &&DEBUG_HANDLE_ADD2DYN_PREF_V8,
        &&DEBUG_HANDLE_SUB2DYN_PREF_V8,
        &&DEBUG_HANDLE_MUL2DYN_PREF_V8,
        &&DEBUG_HANDLE_DIV2DYN_PREF_V8,
        &&DEBUG_HANDLE_MOD2DYN_PREF_V8,
        &&DEBUG_HANDLE_EQDYN_PREF_V8,
        &&DEBUG_HANDLE_NOTEQDYN_PREF_V8,
        &&DEBUG_HANDLE_LESSDYN_PREF_V8,
        &&DEBUG_HANDLE_LESSEQDYN_PREF_V8,
        &&DEBUG_HANDLE_GREATERDYN_PREF_V8,
        &&DEBUG_HANDLE_GREATEREQDYN_PREF_V8,
        &&DEBUG_HANDLE_SHL2DYN_PREF_V8,
        &&DEBUG_HANDLE_ASHR2DYN_PREF_V8,
        &&DEBUG_HANDLE_SHR2DYN_PREF_V8,
        &&DEBUG_HANDLE_AND2DYN_PREF_V8,
        &&DEBUG_HANDLE_OR2DYN_PREF_V8,
        &&DEBUG_HANDLE_XOR2DYN_PREF_V8,
        &&DEBUG_HANDLE_TONUMBER_PREF_V8,
        &&DEBUG_HANDLE_NEGDYN_PREF_V8,
        &&DEBUG_HANDLE_NOTDYN_PREF_V8,
        &&DEBUG_HANDLE_INCDYN_PREF_V8,
        &&DEBUG_HANDLE_DECDYN_PREF_V8,
        &&DEBUG_HANDLE_EXPDYN_PREF_V8,
        &&DEBUG_HANDLE_ISINDYN_PREF_V8,
        &&DEBUG_HANDLE_INSTANCEOFDYN_PREF_V8,
        &&DEBUG_HANDLE_STRICTNOTEQDYN_PREF_V8,
        &&DEBUG_HANDLE_STRICTEQDYN_PREF_V8,
        &&DEBUG_HANDLE_RESUMEGENERATOR_PREF_V8,
        &&DEBUG_HANDLE_GETRESUMEMODE_PREF_V8,
        &&DEBUG_HANDLE_CREATEGENERATOROBJ_PREF_V8,
        &&DEBUG_HANDLE_THROWCONSTASSIGNMENT_PREF_V8,
        &&DEBUG_HANDLE_GETTEMPLATEOBJECT_PREF_V8,
        &&DEBUG_HANDLE_GETNEXTPROPNAME_PREF_V8,
        &&DEBUG_HANDLE_CALLARG0DYN_PREF_V8,
        &&DEBUG_HANDLE_THROWIFNOTOBJECT_PREF_V8,
        &&DEBUG_HANDLE_ITERNEXT_PREF_V8,
        &&DEBUG_HANDLE_CLOSEITERATOR_PREF_V8,
        &&DEBUG_HANDLE_COPYMODULE_PREF_V8,
        &&DEBUG_HANDLE_SUPERCALLSPREAD_PREF_V8,
        &&DEBUG_HANDLE_DELOBJPROP_PREF_V8_V8,
        &&DEBUG_HANDLE_NEWOBJSPREADDYN_PREF_V8_V8,
        &&DEBUG_HANDLE_CREATEITERRESULTOBJ_PREF_V8_V8,
        &&DEBUG_HANDLE_SUSPENDGENERATOR_PREF_V8_V8,
        &&DEBUG_HANDLE_ASYNCFUNCTIONAWAITUNCAUGHT_PREF_V8_V8,
        &&DEBUG_HANDLE_THROWUNDEFINEDIFHOLE_PREF_V8_V8,
        &&DEBUG_HANDLE_CALLARG1DYN_PREF_V8_V8,
        &&DEBUG_HANDLE_COPYDATAPROPERTIES_PREF_V8_V8,
        &&DEBUG_HANDLE_STARRAYSPREAD_PREF_V8_V8,
        &&DEBUG_HANDLE_GETITERATORNEXT_PREF_V8_V8,
        &&DEBUG_HANDLE_SETOBJECTWITHPROTO_PREF_V8_V8,
        &&DEBUG_HANDLE_LDOBJBYVALUE_PREF_V8_V8,
        &&DEBUG_HANDLE_STOBJBYVALUE_PREF_V8_V8,
        &&DEBUG_HANDLE_STOWNBYVALUE_PREF_V8_V8,
        &&DEBUG_HANDLE_LDSUPERBYVALUE_PREF_V8_V8,
        &&DEBUG_HANDLE_STSUPERBYVALUE_PREF_V8_V8,
        &&DEBUG_HANDLE_LDOBJBYINDEX_PREF_V8_IMM32,
        &&DEBUG_HANDLE_STOBJBYINDEX_PREF_V8_IMM32,
        &&DEBUG_HANDLE_STOWNBYINDEX_PREF_V8_IMM32,
        &&DEBUG_HANDLE_CALLSPREADDYN_PREF_V8_V8_V8,
        &&DEBUG_HANDLE_ASYNCFUNCTIONRESOLVE_PREF_V8_V8_V8,
        &&DEBUG_HANDLE_ASYNCFUNCTIONREJECT_PREF_V8_V8_V8,
        &&DEBUG_HANDLE_CALLARGS2DYN_PREF_V8_V8_V8,
        &&DEBUG_HANDLE_CALLARGS3DYN_PREF_V8_V8_V8_V8,
        &&DEBUG_HANDLE_DEFINEGETTERSETTERBYVALUE_PREF_V8_V8_V8_V8,
        &&DEBUG_HANDLE_NEWOBJDYNRANGE_PREF_IMM16_V8,
        &&DEBUG_HANDLE_CALLIRANGEDYN_PREF_IMM16_V8,
        &&DEBUG_HANDLE_CALLITHISRANGEDYN_PREF_IMM16_V8,
        &&DEBUG_HANDLE_SUPERCALL_PREF_IMM16_V8,
        &&DEBUG_HANDLE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8,
        &&DEBUG_HANDLE_DEFINEFUNCDYN_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_DEFINENCFUNCDYN_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_DEFINEGENERATORFUNC_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_DEFINEASYNCFUNC_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_DEFINEMETHOD_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_NEWLEXENVDYN_PREF_IMM16,
        &&DEBUG_HANDLE_COPYRESTARGS_PREF_IMM16,
        &&DEBUG_HANDLE_CREATEARRAYWITHBUFFER_PREF_IMM16,
        &&DEBUG_HANDLE_CREATEOBJECTHAVINGMETHOD_PREF_IMM16,
        &&DEBUG_HANDLE_THROWIFSUPERNOTCORRECTCALL_PREF_IMM16,
        &&DEBUG_HANDLE_CREATEOBJECTWITHBUFFER_PREF_IMM16,
        &&DEBUG_HANDLE_LDLEXVARDYN_PREF_IMM4_IMM4,
        &&DEBUG_HANDLE_LDLEXVARDYN_PREF_IMM8_IMM8,
        &&DEBUG_HANDLE_LDLEXVARDYN_PREF_IMM16_IMM16,
        &&DEBUG_HANDLE_STLEXVARDYN_PREF_IMM4_IMM4_V8,
        &&DEBUG_HANDLE_STLEXVARDYN_PREF_IMM8_IMM8_V8,
        &&DEBUG_HANDLE_STLEXVARDYN_PREF_IMM16_IMM16_V8,
        &&DEBUG_HANDLE_DEFINECLASSWITHBUFFER_PREF_ID16_IMM16_IMM16_V8_V8,
        &&DEBUG_HANDLE_GETMODULENAMESPACE_PREF_ID32,
        &&DEBUG_HANDLE_STMODULEVAR_PREF_ID32,
        &&DEBUG_HANDLE_TRYLDGLOBALBYNAME_PREF_ID32,
        &&DEBUG_HANDLE_TRYSTGLOBALBYNAME_PREF_ID32,
        &&DEBUG_HANDLE_LDGLOBALVAR_PREF_ID32,
        &&DEBUG_HANDLE_STGLOBALVAR_PREF_ID32,
        &&DEBUG_HANDLE_LDOBJBYNAME_PREF_ID32_V8,
        &&DEBUG_HANDLE_STOBJBYNAME_PREF_ID32_V8,
        &&DEBUG_HANDLE_STOWNBYNAME_PREF_ID32_V8,
        &&DEBUG_HANDLE_LDSUPERBYNAME_PREF_ID32_V8,
        &&DEBUG_HANDLE_STSUPERBYNAME_PREF_ID32_V8,
        &&DEBUG_HANDLE_LDMODULEVAR_PREF_ID32_IMM8,
        &&DEBUG_HANDLE_CREATEREGEXPWITHLITERAL_PREF_ID32_IMM8,
        &&DEBUG_HANDLE_ISTRUE_PREF,
        &&DEBUG_HANDLE_ISFALSE_PREF,
        &&DEBUG_HANDLE_STCONSTTOGLOBALRECORD_PREF_ID32,
        &&DEBUG_HANDLE_STLETTOGLOBALRECORD_PREF_ID32,
        &&DEBUG_HANDLE_STCLASSTOGLOBALRECORD_PREF_ID32,
        &&DEBUG_HANDLE_STOWNBYVALUEWITHNAMESET_PREF_V8_V8,
        &&DEBUG_HANDLE_STOWNBYNAMEWITHNAMESET_PREF_ID32_V8,
        &&DEBUG_HANDLE_LDFUNCTION_PREF,
        &&DEBUG_HANDLE_NEWLEXENVWITHNAMEDYN_PREF_IMM16_IMM16,
        &&DEBUG_HANDLE_LDBIGINT_PREF_ID32,
        &&DEBUG_HANDLE_TONUMERIC_PREF_V8,
        &&DEBUG_HANDLE_CREATEASYNCGENERATOROBJ_PREF_V8,
        &&DEBUG_HANDLE_ASYNCGENERATORRESOLVE_PREF_V8_V8_V8,
        &&DEBUG_HANDLE_DEFINEASYNCGENERATORFUNC_PREF_ID16_IMM16_V8,
        &&DEBUG_HANDLE_DYNAMICIMPORT_PREF_V8,
        &&DEBUG_HANDLE_MOV_DYN_V8_V8,
        &&DEBUG_HANDLE_MOV_DYN_V16_V16,
        &&DEBUG_HANDLE_LDA_STR_ID32,
        &&DEBUG_HANDLE_LDAI_DYN_IMM32,
        &&DEBUG_HANDLE_FLDAI_DYN_IMM64,
        &&DEBUG_HANDLE_JMP_IMM8,
        &&DEBUG_HANDLE_JMP_IMM16,
        &&DEBUG_HANDLE_JMP_IMM32,
        &&DEBUG_HANDLE_JEQZ_IMM8,
        &&DEBUG_HANDLE_JEQZ_IMM16,
        &&DEBUG_HANDLE_LDA_DYN_V8,
        &&DEBUG_HANDLE_STA_DYN_V8,
        &&DEBUG_HANDLE_RETURN_DYN,
        &&DEBUG_HANDLE_MOV_V4_V4,
        &&DEBUG_HANDLE_JNEZ_IMM8,
        &&DEBUG_HANDLE_JNEZ_IMM16,
        &&DEBUG_EXCEPTION_HANDLER,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,
        &&DEBUG_HANDLE_OVERFLOW,