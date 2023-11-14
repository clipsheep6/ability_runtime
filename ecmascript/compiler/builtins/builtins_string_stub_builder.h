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

#ifndef ECMASCRIPT_COMPILER_BUILTINS_STRING_STUB_BUILDER_H
#define ECMASCRIPT_COMPILER_BUILTINS_STRING_STUB_BUILDER_H
#include "ecmascript/compiler/stub_builder-inl.h"

namespace panda::ecmascript::kungfu {
class FlatStringStubBuilder;
struct StringInfoGateRef;

class BuiltinsStringStubBuilder : public BuiltinsStubBuilder {
public:
    explicit BuiltinsStringStubBuilder(StubBuilder *parent)
        : BuiltinsStubBuilder(parent) {}
    BuiltinsStringStubBuilder(CallSignature *callSignature, Environment *env)
        : BuiltinsStubBuilder(callSignature, env) {}
    ~BuiltinsStringStubBuilder() override = default;
    NO_MOVE_SEMANTIC(BuiltinsStringStubBuilder);
    NO_COPY_SEMANTIC(BuiltinsStringStubBuilder);
    void GenerateCircuit() override {}

    void FromCharCode(GateRef glue, GateRef thisValue, GateRef numArgs, Variable* res, Label *exit, Label *slowPath);
    void CharAt(GateRef glue, GateRef thisValue, GateRef numArgs, Variable* res, Label *exit, Label *slowPath);
    void CharCodeAt(GateRef glue, GateRef thisValue, GateRef numArgs, Variable* res, Label *exit, Label *slowPath);
    void IndexOf(GateRef glue, GateRef thisValue, GateRef numArgs, Variable* res, Label *exit, Label *slowPath);
    void Substring(GateRef glue, GateRef thisValue, GateRef numArgs, Variable* res, Label *exit, Label *slowPath);
    void Replace(GateRef glue, GateRef thisValue, GateRef numArgs, Variable *res, Label *exit, Label *slowPath);
    void Trim(GateRef glue, GateRef thisValue, GateRef numArgs, Variable *res, Label *exit, Label *slowPath);
    void Slice(GateRef glue, GateRef thisValue, GateRef numArgs, Variable *res, Label *exit, Label *slowPath);
    void LocaleCompare(GateRef glue, GateRef thisValue, GateRef numArgs, Variable *res, Label *exit, Label *slowPath);

    GateRef ConvertAndClampRelativeIndex(GateRef index, GateRef length);
    GateRef StringAt(const StringInfoGateRef &stringInfoGate, GateRef index);
    GateRef FastSubString(GateRef glue, GateRef thisValue, GateRef from, GateRef len,
        const StringInfoGateRef &stringInfoGate);
    GateRef FastSubUtf8String(GateRef glue, GateRef from, GateRef len, const StringInfoGateRef &stringInfoGate);
    GateRef FastSubUtf16String(GateRef glue, GateRef from, GateRef len, const StringInfoGateRef &stringInfoGate);
    GateRef GetSubstitution(GateRef glue, GateRef searchString, GateRef thisString,
        GateRef pos, GateRef replaceString);
    void CopyChars(GateRef glue, GateRef dst, GateRef source, GateRef sourceLength, GateRef size, VariableType type);
    void CopyUtf16AsUtf8(GateRef glue, GateRef dst, GateRef src, GateRef sourceLength);
    void CopyUtf8AsUtf16(GateRef glue, GateRef dst, GateRef src, GateRef sourceLength);
    GateRef StringIndexOf(GateRef lhsData, bool lhsIsUtf8, GateRef rhsData, bool rhsIsUtf8,
                          GateRef pos, GateRef max, GateRef rhsCount);
    GateRef StringIndexOf(const StringInfoGateRef &lStringInfoGate,
        const StringInfoGateRef &rStringInfoGate, GateRef pos);
    GateRef GetSingleCharCodeByIndex(GateRef str, GateRef index);
    GateRef CreateStringBySingleCharCode(GateRef glue, GateRef charCode);
    GateRef CreateFromEcmaString(GateRef glue, GateRef index, const StringInfoGateRef &stringInfoGate);
    GateRef StringConcat(GateRef glue, GateRef leftString, GateRef rightString);
    GateRef EcmaStringTrim(GateRef glue, GateRef srcString, GateRef trimMode);
    GateRef EcmaStringTrimBody(GateRef glue, GateRef thisValue, StringInfoGateRef srcStringInfoGate,
        GateRef trimMode, GateRef isUtf8);
    void StoreParent(GateRef glue, GateRef object, GateRef parent);
    void StoreStartIndex(GateRef glue, GateRef object, GateRef startIndex);
private:
    GateRef ChangeStringTaggedPointerToInt64(GateRef x)
    {
        return GetEnvironment()->GetBuilder()->ChangeTaggedPointerToInt64(x);
    }
    GateRef GetStringDataFromLineOrConstantString(GateRef str);
    GateRef CanBeCompressed(GateRef utf16Data, GateRef utf16Len, bool isUtf16);
    GateRef GetUtf16Data(GateRef stringData, GateRef index);
    GateRef IsASCIICharacter(GateRef data);
    GateRef GetUtf8Data(GateRef stringData, GateRef index);
    GateRef GetSingleCharCodeFromConstantString(GateRef str, GateRef index);
    GateRef GetSingleCharCodeFromLineString(GateRef str, GateRef index);
    GateRef GetSingleCharCodeFromSlicedString(GateRef str, GateRef index);
    GateRef GetSubString(GateRef glue, GateRef thisValue, GateRef from, GateRef len);
};

class FlatStringStubBuilder : public StubBuilder {
public:
    explicit FlatStringStubBuilder(StubBuilder *parent)
        : StubBuilder(parent) {}
    ~FlatStringStubBuilder() override = default;
    NO_MOVE_SEMANTIC(FlatStringStubBuilder);
    NO_COPY_SEMANTIC(FlatStringStubBuilder);
    void GenerateCircuit() override {}

    void FlattenString(GateRef glue, GateRef str, Label *fastPath);
    GateRef GetParentFromSlicedString(GateRef string)
    {
        GateRef offset = IntPtr(SlicedString::PARENT_OFFSET);
        return Load(VariableType::JS_POINTER(), string, offset);
    }
    GateRef GetStartIndexFromSlicedString(GateRef string)
    {
        GateRef offset = IntPtr(SlicedString::STARTINDEX_OFFSET);
        return Load(VariableType::INT32(), string, offset);
    }

    GateRef GetFlatString()
    {
        return flatString_.ReadVariable();
    }

    GateRef GetStartIndex()
    {
        return startIndex_.ReadVariable();
    }

    GateRef GetLength()
    {
        return length_;
    }

private:
    Variable flatString_ { GetEnvironment(), VariableType::JS_POINTER(), NextVariableId(), Undefined() };
    Variable startIndex_ { GetEnvironment(), VariableType::INT32(), NextVariableId(), Int32(0) };
    GateRef length_ { Circuit::NullGate() };
};

struct StringInfoGateRef {
    GateRef string_ { Circuit::NullGate() };
    GateRef startIndex_ { Circuit::NullGate() };
    GateRef length_ { Circuit::NullGate() };
    StringInfoGateRef(FlatStringStubBuilder *flatString) : string_(flatString->GetFlatString()),
                                                           startIndex_(flatString->GetStartIndex()),
                                                           length_(flatString->GetLength()) {}
    GateRef GetString() const
    {
        return string_;
    }

    GateRef GetStartIndex() const
    {
        return startIndex_;
    }

    GateRef GetLength() const
    {
        return length_;
    }
};
}  // namespace panda::ecmascript::kungfu
#endif  // ECMASCRIPT_COMPILER_BUILTINS_STRING_STUB_BUILDER_H