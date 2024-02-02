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

#ifndef MAPLE_LITECG_LMIR_BUILDER_H
#define MAPLE_LITECG_LMIR_BUILDER_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <unordered_map>

/*
  LMIR API exported.

  LMIR, the low-level MIR, will serve as the canonical input for LiteCG.

  Currently, it only contains the minimum set of features enough to
  support eCompiler scenarioes. Later it will grow to be a complete core
  set, but still with "just enough" features for general cases.

  Additional features, will be defined with x-extension strategy.

  The motivation of LMIR, is to hide the whole complexity of MIR interfaces,
  and this should make integration of maple components easier.
 */
namespace maple {

/* import types for MIR: this is basically a simplification
   we just need declaration, not headers.
 */
class MIRBuilder;  // currently we just delegate MIRBuilder
class MIRModule;
class MIRFunction;
class MIRType;
class MIRStructType;
class MIRArrayType;
class MIRFuncType;
class MIRConst;
class MIRAggConst;
class MIRSymbol;
class StmtNode;
class BaseNode;
class BlockNode;
class MIRPreg;

namespace litecg {

// Our type abstraction. currently delegate to MIR

using String = std::string;

using Module = MIRModule;
using Function = MIRFunction;

// Note: Type is base class of all other Types
using Type = MIRType;              // base class of all Types
using StructType = MIRStructType;  //    |__ StructType
using ArrayType = MIRArrayType;
using Const = MIRConst;
using StructConst = MIRAggConst;
using ArrayConst = MIRAggConst;
using Var = MIRSymbol;
using Stmt = StmtNode;
using BB = BlockNode;  // A temporary faked BB

using Param = std::pair<const String, Type *>;
using Params = std::vector<Param>;
using FieldOffset = std::pair<int32_t, int32_t>;  // (byteoffset, bitoffset)
using PregIdx = int32_t;

// enumerations
enum class MIRIntrinsic {
#define DEF_MIR_INTRINSIC(STR, NAME, INTRN_CLASS, RETURN_TYPE, ...) INTRN_##STR,
#include "intrinsics.def"
#undef DEF_MIR_INTRINSIC
};
using IntrinsicId = MIRIntrinsic;
/* available intrinsic should be list here. like:
   INTRN_memcpy, INTRN_memset, etc.
 */
class Expr {
public:
    Expr(BaseNode *baseNode, Type *nodeType) : node(baseNode), type(nodeType) {}

    Expr() : node(nullptr), type(nullptr) {}

    ~Expr() = default;

    BaseNode *GetNode()
    {
        return node;
    }

    BaseNode *GetNode() const
    {
        return node;
    }

    Type *GetType()
    {
        return type;
    }

    Type *GetType() const
    {
        return type;
    }

    bool IsDread() const;
    bool IsRegread() const;
    bool IsConstValue() const;

private:
    BaseNode *node;
    Type *type;
};

enum LiteCGValueKind {
    kPregKind,
    kSymbolKind,
    kConstKind,
};

struct LiteCGValue {
    PregIdx pregIdx;
    MIRSymbol *symbol;
    MIRConst *constVal;
    LiteCGValueKind kind;
};

using Args = std::vector<Expr>;

enum FuncAttr {     // visibility of the defined function
    FUNC_global,    // function has global visibility
    FUNC_weak,      // weak function defined in this module
    FUNC_internal,  // function defined and only used in this module
};

enum GlobalRegister {
    kSregSp = -1,
    kSregFp = -2,
};

enum ConvAttr {
    CCall,
    Web_Kit_JS_Call,
    GHC_Call,
};

enum GlobalVarAttr {
    VAR_external,  // global variable declaration (no definition)
    VAR_weak,      // weak function defined in this module
    VAR_internal,  // variable defined and only used in this module
    VAR_global,    // exported variable defined in this module
    VAR_readonly,  // this is additional flag, default is R/W
};

enum IntCmpCondition {
    kEQ,
    kNE,
    kULT,
    kULE,
    kUGT,
    kUGE,
    kSLT,
    kSLE,
    kSGT,
    kSGE,
};

enum FloatCmpCondition {
    kOLT,
    kOLE,
    kOGT,
    kOGE,
    kONE,
    kOEQ,
};

enum LiteCGTypeKind {
    kLiteCGTypeInvalid,
    kLiteCGTypeUnknown,
    kLiteCGTypeScalar,
    kLiteCGTypeBitField,
    kLiteCGTypeArray,
    kLiteCGTypeFArray,
    kLiteCGTypeJArray,
    kLiteCGTypeStruct,
    kLiteCGTypeUnion,
    kLiteCGTypeClass,
    kLiteCGTypeInterface,
    kLiteCGTypeStructIncomplete,
    kLiteCGTypeClassIncomplete,
    kLiteCGTypeConstString,
    kLiteCGTypeInterfaceIncomplete,
    kLiteCGTypePointer,
    kLiteCGTypeFunction,
    kLiteCGTypeVoid,
    kLiteCGTypeByName,
    kLiteCGTypeParam,
    kLiteCGTypeInstantVector,
    kLiteCGTypeGenericInstant,
};

// FieldAttr: do we need to support volatile here?
// using FieldAttr = AttrKind
/* used attribute for field:
   ATTR_volatile
 */

using FieldId = uint32_t;
/* Use FieldId from MIR directly: it's a uint32_t, but with special meaning
   for FieldId == 0: refer to the "whole" of the type
   To avoid conflict with MIR type define, here using type name "FieldId"
 */

Module *CreateModuleWithName(const std::string &name);
void ReleaseModule(Module *module);

/**
 * a simplified, specialized MapleIR builder for LiteCG
 * The basic IR set
 *
 * General rule for the interface:
 *  + if function returns value type, then returns type is value type
 *  + otherwise if the return value can be null, return type is pointer
 *    caller should check for null
 *  + otherwise the return type is reference.
 *
 *  + for compound IR (need to be constructed with a sequence of calls,
 *      e.g., struct type/const, array const, function, switch), using
 *    specific builder class to do the "chained" construction.
 *
 */
class LMIRBuilder {
public:
    LMIRBuilder(Module &module);
    ~LMIRBuilder() = default;

    void DumpIRToFile(const std::string fileName);

    LiteCGTypeKind LiteCGGetTypeKind(Type *type) const;
    void SetCallStmtDeoptBundleInfo(Stmt &icallNode, const std::unordered_map<int, LiteCGValue> &deoptBundleInfo);

    // Type creation (currently all in global scope)
    /*
       For primitive types, using LMIRBuilder's public member:
       i8Type, u8Type, etc.
     */

    // derived type creation
    Type *CreatePtrType(Type *mirType);
    Type *CreateRefType(Type *mirType);

    bool IsHeapPointerType(Type *mirType) const;

    // (multi-dim) array of fixed size array
    ArrayType *CreateArrayType(Type *elemType, std::vector<uint32_t> &dimSize);

    /* using StructTypeBuilder interface for StructType creation
         auto structType = CreateStructType("mystruct")
                             .Field("field1", i32Type)
                             .Field("field2", i64Type)
                             .Done();
     */
    Type *GetStructType(const String &name);  // query for existing struct type

    // usage of this function has precondition, should be documented
    FieldOffset GetFieldOffset(StructType *structType, FieldId fieldId);

    // for function pointer
    Type *CreateFuncType(std::vector<Type *> params, Type *retType, bool isVarg);

    Type *LiteCGGetPointedType(Type *type);

    std::vector<Type *> LiteCGGetFuncParamTypes(Type *type);

    Type *LiteCGGetFuncReturnType(Type *type);
    // still need interface for AddressOfFunction

    // Function declaration and definition
    /* using FunctionBuilder interface for Function creation:
         // i32 myfunc(i32 param1, i64 param2) { }
         auto structType = DefineFunction("myfunc")
                             .Param(i32Type, "param1")
                             .Param(i64Type, "param2")
                             .Ret(i32Type)               // optional for void
                             .Done();

         auto structType = DeclareFunction("myfunc1")
                             .Param(i32Type, "param1")
                             .Param(i64Type, "param2")
                             .Ret(i32Type)
                             .Done();
     */

    // This is to enable forwarded call before its definition
    // can return null. caller should check for null.
    Function *GetFunc(const String &name);  // get a function by its unique name

    // when a function is set as current function (of the module), local
    // declarations and statements are insert into it.
    void SetCurFunc(Function &function);

    Function &GetCurFunction() const;

    void RenameFormal2Preg(Function &func);

    MIRPreg *LiteCGGetPreg(Function &func, int32_t pRegNo);
    Expr LiteCGGetPregFP(Function &func);
    Expr LiteCGGetPregSP();

    // var creation
    // refine the interface for attributes here. and also storage-class?
    // initialized to zero if defined here, by default not exported
    Var &CreateGlobalVar(Type *type, const String &name, GlobalVarAttr attr = VAR_internal);
    // initialized to const, by default not exported
    Var &CreateGlobalVar(Type *type, const String &name, Const &init, GlobalVarAttr attr = VAR_internal);
    Var *GetGlobalVar(const String &name);

    Var &CreateLocalVar(Type *type, const String &name);
    Var *GetLocalVar(const String &name);
    Var *GetLocalVarFromExpr(Expr inExpr);
    void SetFunctionDerived2BaseRef(PregIdx derived, PregIdx base);
    PregIdx GetPregIdxFromExpr(const Expr &expr);
    Var &GetParam(Function &function, size_t index) const;
    Expr GenExprFromVar(Var &var);

    Const &CreateIntConst(Type *type, int64_t val);
    Const &CreateFloatConst(float val);
    Const &CreateDoubleConst(double val);
    Const &CreateStrConst(const String &constStr);
    Const *GetConstFromExpr(const Expr &expr);

    // In MIR, the const for struct & array are the same. But we separate it here.
    /* using StructConstBuilder interface for StructConst creation:
         auto structConst = CreateStructConst(structType)
                              .Field(1, CreateIntConst(i32Type, 0))
                              .Filed(2, CreateIntConst(i64Type, 0))
                              .Done();
     */

    /* using ArrayConstBuilder interface for ArrayConst creation:
       Note: the elements should be added consequentially, and match the dim size.
         auto arrayConst = CreateArrayConst(arrayType)
                              .Element(CreateIntConst(i32Type, 0))
                              .Element(CreateIntConst(i32Type, 0))
                              .Done();
       or using the following form:
         auto arrayConst = CreateArrayConst(arrayType).Dim({0, 0}).Done();
     */

    /*
      BB is the container node for a sequence or linear statements,
      if needLabel == true, implicitly create a label node as its first statement.
      BB also servers as target for Gotos, when it's a goto target, it
      should have needLabel == true
     */
    BB &CreateBB(bool needLabel = true);
    void AppendStmt(BB &bb, Stmt &stmt);              // append stmt to the back of BB
    void AppendStmtBeforeBranch(BB &bb, Stmt &stmt);  // append stmt after the first non-jump stmt in back of BB
    bool IsEmptyBB(BB &bb);
    void AppendBB(BB &bb);    // append BB to the back of current function;
    void AppendToLast(BB &bb);
    BB &GetLastPosBB();
    BB &GetLastAppendedBB();  // get last appended BB of current function

    void SetStmtCallConv(Stmt &stmt, ConvAttr convAttr);

    // statements
    Stmt &Goto(BB &dest);  // jump without condition
    /* conditional goto:
       when inverseCond == true, using (!cond) as condition

       1. if(cond)-then form code should be generated this way:

         if(!cond) goto BB_end    // CondGoto(cond, BB_end, true);
         BB_ifTrue: {...}
         BB_end: {...}

       2. if-then-else form code should be generated this way:
         if(cond) goto BB_ifTrue  // CondGoto(cond, BB_ifTrue);
         BB_ifFalse: {
           ...
           goto BB_end            // should be generated in BB_ifFalse
         }
         BB_ifTrue: {...}
         BB_end: {...}
     */
    Stmt &CondGoto(Var &cond, BB &target, bool inverseCond = false);
    Stmt &CondGoto(Expr cond, BB &target, bool inverseCond = false);

    /* using SwitchBuilder interface for switch statement creation
         auto switchStmt = Switch(type, cond, defaultBB)
                             .Case(0, bb1)
                             .Case(1, bb2)
                             .Done();
     */

    // when result is nullptr, don't need the result (or no result)
    Stmt &Call(Function &func, Args &args, Var *result = nullptr);

    Stmt &Call(Function &func, Args &args, PregIdx pregIdx);

    Stmt &ICall(Expr funcAddr, Args &args, Var *result = nullptr);

    Stmt &ICall(Expr funcAddr, Args &args, PregIdx pregIdx);

    // when result is nullptr, don't need the result (or no result)
    Stmt &IntrinsicCall(IntrinsicId func, Args &valueArgs, Var *result = nullptr);

    Stmt &Return(Expr returnVal);

    // debug info
    Stmt &Comment(std::string comment);

    Stmt &Dassign(Expr src, Var &var, FieldId fieldId = 0);
    Stmt &Iassign(Expr src, Expr addr, Type *baseType, FieldId fieldId = 0);

    // expressions
    Expr Dread(Var &var);  // do we need other forms?
    inline Expr Dread(Var *var)
    {  // shortcut for read from local-var
        return Dread(*var);
    }

    Expr DreadWithField(Var &var, FieldId id);

    Expr Iread(Type *type, Expr addr, Type *baseType, FieldId fieldId = 0);
    PregIdx CreatePreg(Type *mtype);
    Stmt &Regassign(Expr src, PregIdx reg);
    Expr Regread(PregIdx pregIdx);
    Expr Addrof(Var &var);           // do we need other forms?
    Expr ConstVal(Const &constVal);  // a const operand

    Expr Lnot(Type *type, Expr src);
    Expr Bnot(Type *type, Expr src);
    Expr Sqrt(Type *type, Expr src);

    Expr Add(Type *type, Expr src1, Expr src2);
    Expr Sub(Type *type, Expr src1, Expr src2);
    Expr Mul(Type *type, Expr src1, Expr src2);
    Expr UDiv(Type *type, Expr src1, Expr src2);  // unsigned
    Expr SDiv(Type *type, Expr src1, Expr src2);  // signed
    Expr URem(Type *type, Expr src1, Expr src2);  // unsigned
    Expr SRem(Type *type, Expr src1, Expr src2);  // signed
    Expr Shl(Type *type, Expr src1, Expr src2);
    Expr LShr(Type *type, Expr src1, Expr src2);
    Expr AShr(Type *type, Expr src1, Expr src2);
    Expr And(Type *type, Expr src1, Expr src2);
    Expr Or(Type *type, Expr src1, Expr src2);
    Expr Xor(Type *type, Expr src1, Expr src2);

    Expr ICmpEQ(Type *type, Expr src1, Expr src2);
    Expr ICmpNE(Type *type, Expr src1, Expr src2);
    // unsigned compare
    Expr ICmpULT(Type *type, Expr src1, Expr src2);
    Expr ICmpULE(Type *type, Expr src1, Expr src2);
    Expr ICmpUGT(Type *type, Expr src1, Expr src2);
    Expr ICmpUGE(Type *type, Expr src1, Expr src2);
    // signed compare
    Expr ICmpSLT(Type *type, Expr src1, Expr src2);
    Expr ICmpSLE(Type *type, Expr src1, Expr src2);
    Expr ICmpSGT(Type *type, Expr src1, Expr src2);
    Expr ICmpSGE(Type *type, Expr src1, Expr src2);
    Expr ICmp(Type *type, Expr src1, Expr src2, IntCmpCondition cond);
    Expr FCmp(Type *type, Expr src1, Expr src2, FloatCmpCondition cond);

    // Type conversion
    // Type of opnd should be consistent with fromType: no implicient conversion
    Expr Trunc(Type *fromType, Type *toType, Expr opnd);
    Expr ZExt(Type *fromType, Type *toType, Expr opnd);
    Expr SExt(Type *fromType, Type *toType, Expr opnd);
    Expr BitCast(Type *fromType, Type *toType, Expr opnd);
    Expr Cvt(Type *fromType, Type *toType, Expr opnd);

    Expr Select(Type *type, Expr cond, Expr ifTrue, Expr ifFalse);

    void SetFuncFrameResverdSlot(int slot);
    void SetFuncFramePointer(const String &val);

public:
    // helper classes for compound IR entity building
    class SwitchBuilder {
    public:
        SwitchBuilder(LMIRBuilder &builder_, Type *type_, Expr cond_, BB &defaultBB_)
            : builder(builder_), type(type_), cond(cond_), defaultBB(defaultBB_)
        {
        }

        SwitchBuilder &Case(int64_t value, BB &bb)
        {
            cases.push_back(std::make_pair(value, &bb));
            return *this;
        }

        Stmt &Done()
        {
            return builder.CreateSwitchInternal(type, cond, defaultBB, cases);
        }

    private:
        LMIRBuilder &builder;
        Type *type;
        Expr cond;
        BB &defaultBB;
        std::vector<std::pair<int64_t, BB *>> cases;
    };

    SwitchBuilder Switch(Type *type, Expr cond, BB &defaultBB)
    {
        return SwitchBuilder(*this, type, cond, defaultBB);
    }

    class StructTypeBuilder {
    public:
        StructTypeBuilder(LMIRBuilder &builder_, const String &name_) : builder(builder_), name(name_) {}

        StructTypeBuilder &Field(std::string_view fieldName, Type *fieldType)
        {
            // field type attribute?
            fields.push_back(std::make_pair(fieldName, fieldType));
            return *this;
        }

        Type *Done()
        {
            return builder.CreateStructTypeInternal(name, fields);
        }

    private:
        LMIRBuilder &builder;
        const String &name;
        std::vector<std::pair<std::string_view, Type *>> fields;
    };

    StructTypeBuilder CreateStructType(const String &name)
    {
        return StructTypeBuilder(*this, name);
    }

    class StructConstBuilder {
    public:
        StructConstBuilder(LMIRBuilder &builder_, StructType *type_) : builder(builder_)
        {
            structConst = &builder_.CreateStructConstInternal(type_);
        }

        StructConstBuilder &Field(FieldId fieldId, Const &field)
        {
            builder.AddConstItemInternal(*structConst, fieldId, field);
            return *this;
        }

        StructConst &Done()
        {
            return *structConst;
        }

    private:
        LMIRBuilder &builder;
        StructConst *structConst;
    };

    StructConstBuilder CreateStructConst(StructType *type)
    {
        return StructConstBuilder(*this, type);
    }

    class ArrayConstBuilder {
    public:
        ArrayConstBuilder(LMIRBuilder &builder_, ArrayType *type_) : builder(builder_)
        {
            arrayConst = &builder.CreateArrayConstInternal(type_);
        }

        ArrayConstBuilder &Element(Const &element)
        {
            builder.AddConstItemInternal(*arrayConst, element);
            return *this;
        }

        template <class T>
        ArrayConstBuilder &Dim(const std::vector<T> init)
        {
            for (const auto &value : init) {
                // fix the element type.
                Const &element = builder.CreateIntConst(builder.i32Type, static_cast<int64_t>(value));
                Element(element);
            }
            return *this;
        }

        template <class T>
        ArrayConstBuilder &Dim(std::initializer_list<T> literal)
        {
            return Dim(std::vector<T>(literal));
        }

        ArrayConst &Done()
        {
            return *arrayConst;
        }

    private:
        LMIRBuilder &builder;
        ArrayConst *arrayConst;
    };

    ArrayConstBuilder CreateArrayConst(ArrayType *type)
    {
        return ArrayConstBuilder(*this, type);
    }

    class FunctionBuilder {
    public:
        FunctionBuilder(LMIRBuilder &builder_, const String &name_, bool needBody_)
            : builder(builder_), name(name_), needBody(needBody_)
        {
            attr = FUNC_internal;
            convAttr = CCall;
            isVargs = false;
        }

        // optional: indicate the function has variable args
        FunctionBuilder &Vargs()
        {
            isVargs = true;
            return *this;
        }

        FunctionBuilder &Param(Type *type, const String paramName)
        {
            params.push_back(std::make_pair(paramName, type));
            return *this;
        }

        // optional: if not called, return nothing (return void)
        FunctionBuilder &Return(Type *type)
        {
            retType = type;
            return *this;
        }

        // optional: if not called, default to FUNC_local
        FunctionBuilder &Attribute(FuncAttr attr_)
        {
            attr = attr_;
            return *this;
        }

        // optional: if not called, default to Func_CCall
        FunctionBuilder &CallConvAttribute(ConvAttr convAttr_)
        {
            convAttr = convAttr_;
            return *this;
        }

        Function &Done()
        {
            return builder.CreateFunctionInternal(name, retType, params, isVargs, needBody, attr, convAttr);
        }

    private:
        LMIRBuilder &builder;
        const String &name;
        Type *retType;
        FuncAttr attr;
        ConvAttr convAttr;
        bool isVargs;
        bool needBody;  // indicate whether is a declaration or definition.
        Params params;
    };

    // only declare the function in current module (means it's an external function)
    FunctionBuilder DeclareFunction(const String &name)
    {
        return FunctionBuilder(*this, name, false);
    }

    // define the function in current module
    FunctionBuilder DefineFunction(const String &name)
    {
        return FunctionBuilder(*this, name, true);
    }

public:
    // builtin types: primitive types (all MIR primitive types except PTY_ptr)
    Type *i8Type;
    Type *i16Type;
    Type *i32Type;
    Type *i64Type;
    Type *i128Type;
    Type *u1Type;
    Type *u8Type;
    Type *u16Type;
    Type *u32Type;
    Type *u64Type;
    Type *u128Type;
    Type *voidType;
    Type *f32Type;
    Type *f64Type;

    // builtin types: commonly used derived types
    Type *strType;
    Type *i64PtrType;
    Type *i64RefType;

private:
    Stmt &CreateSwitchInternal(Type *type, Expr cond, BB &defaultBB, std::vector<std::pair<int64_t, BB *>> &cases);
    Type *CreateStructTypeInternal(const String &name, std::vector<std::pair<std::string_view, Type *>> &fields);
    StructConst &CreateStructConstInternal(StructType *type);
    void AddConstItemInternal(StructConst &structConst, FieldId fieldId, Const &field);
    void AddConstItemInternal(ArrayConst &structConst, Const &element);
    ArrayConst &CreateArrayConstInternal(ArrayType *type);
    Function &CreateFunctionInternal(const String &name, Type *retType, Params &params, bool isVargs, bool needBody,
                                     FuncAttr attr, ConvAttr convAttr);

private:
    MIRBuilder &mirBuilder;  // The real IR-builder: current implementation
    Module &module;          // and the module to process
};

}  // namespace litecg
}  // namespace maple
#endif  // MAPLE_LITECG_LMIR_BUILDER_H
