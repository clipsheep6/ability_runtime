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

#include "codegen/js_code_emitter.h"

#include <cctype>
#include <cstdio>

#include "securec.h"
#include "util/file.h"

namespace OHOS {
namespace Zidl {


void JsCodeEmitter::EmitInterface()
{
}

void JsCodeEmitter::EmitInterfaceImports(
    /* [in] */ StringBuilder& sb)
{
    sb.Append("import rpc from \"@ohos.rpc\";\n");
}

void JsCodeEmitter::EmitInterfaceCorelibImports(
    /* [in] */ StringBuilder& sb)
{
    bool includeList = false;
    bool includeMap = false;
    for (int i = 0; i < metaComponent_->typeNumber_; i++) {
        MetaType* mt = metaComponent_->types_[i];
        switch (mt->kind_) {
            case TypeKind::List: {
                if (!includeList) {
                    sb.Append("import java.util.List;\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::Map: {
                if (!includeMap) {
                    sb.Append("import java.util.Map;\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void JsCodeEmitter::EmitInterfaceDBinderImports(
    /* [in] */ StringBuilder& sb)
{
    sb.Append("import ohos.rpc.IRemoteBroker;\n");
    sb.Append("import ohos.rpc.RemoteException;\n");
}

void JsCodeEmitter::EmitInterfaceSelfDefinedTypeImports(
    /* [in] */ StringBuilder& sb)
{
    for (int i = 0; i < metaComponent_->sequenceableNumber_; i++) {
        MetaSequenceable* mp = metaComponent_->sequenceables_[i];
        sb.AppendFormat("import %s%s;\n", mp->namespace_, mp->name_);
    }

    for (int i = 0; i < metaComponent_->interfaceNumber_; i++) {
        MetaInterface* mi = metaComponent_->interfaces_[i];
        if (mi->external_) {
            sb.AppendFormat("import %s%s;\n", mi->namespace_, mi->name_);
        }
    }
}

void JsCodeEmitter::EmitInterfaceDefinition(
    /* [in] */ StringBuilder& sb)
{
    sb.AppendFormat("public interface %s extends IRemoteBroker {\n", metaInterface_->name_);
    EmitInterfaceMethods(sb, TAB);
    sb.Append("};\n");
}

void JsCodeEmitter::EmitInterfaceMethods(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    if (metaInterface_->methodNumber_ > 0) {
        sb.Append("\n");
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            EmitInterfaceMethod(mm, sb, prefix);
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void JsCodeEmitter::EmitInterfaceMethod(
    /* [in] */ MetaMethod* mm,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (mm->parameterNumber_ == 0) {
        sb.Append(prefix).AppendFormat(
            "%s %s() throws RemoteException;\n", EmitType(returnType).string(), MethodName(mm->name_).string());
    } else {
        sb.Append(prefix).AppendFormat("%s %s(\n", EmitType(returnType).string(), MethodName(mm->name_).string());
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            EmitInterfaceMethodParameter(mp, sb, prefix + TAB);
            if (i != mm->parameterNumber_ - 1) {
                sb.Append(",\n");
            }
        }
        sb.Append(") throws RemoteException;\n");
    }
}

void JsCodeEmitter::EmitInterfaceMethodParameter(
    /* [in] */ MetaParameter* mp,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    std::string name = UnderlineAdded(mp->name_);
    sb.AppendFormat("%s", name.c_str());
}

void JsCodeEmitter::EmitInterfaceProxy()
{
    String filePath = String::Format("%s/%s.js", directory_.string(), FileName(proxyName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;

    EmitInterfaceImports(sb);
    sb.Append("\n");
    EmitInterfaceStubConstants(sb);
    sb.Append("\n");
    EmitJsCallBack(sb);
    sb.Append("\n");
    EmitInterfaceProxyImpl(sb);
    sb.Append("\n");

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void JsCodeEmitter::EmitInterfaceProxyImports(
    /* [in] */ StringBuilder& sb)
{
    EmitInterfaceProxyCorelibImports(sb);
    EmitInterfaceProxySelfDefinedTypeImports(sb);
    EmitInterfaceProxyDBinderImports(sb);
    EmitInterfaceProxyParametersImports(sb);
}

void JsCodeEmitter::EmitInterfaceProxyCorelibImports(
    /* [in] */ StringBuilder& sb)
{
    bool includeList = false;
    bool includeMap = false;
    for (int i = 0; i < metaComponent_->typeNumber_; i++) {
        MetaType* mt = metaComponent_->types_[i];
        switch(mt->kind_) {
            case TypeKind::List: {
                if (!includeList) {
                    sb.Append("import java.util.List;\n");
                    sb.Append("import java.util.ArrayList;\n");
                    includeList = true;
                }
                break;
            }
            case TypeKind::Map: {
                if (!includeMap) {
                    sb.Append("import java.util.Map;\n");
                    sb.Append("import java.util.HashMap;\n");
                    includeMap = true;
                }
                break;
            }
            default:
                break;
        }
    }
}

void JsCodeEmitter::EmitInterfaceProxySelfDefinedTypeImports(
    /* [in] */ StringBuilder& sb)
{
    for (int i = 0; i < metaComponent_->sequenceableNumber_; i++) {
        MetaSequenceable* mp = metaComponent_->sequenceables_[i];
        sb.AppendFormat("import %s%s;\n", mp->namespace_, mp->name_);
    }

    for (int i = 0; i < metaComponent_->interfaceNumber_; i++) {
        MetaInterface* mi = metaComponent_->interfaces_[i];
        if (mi->external_) {
            sb.AppendFormat("import %s%s;\n", mi->namespace_, mi->name_);
            sb.AppendFormat("import %s%s;\n", mi->namespace_, StubName(String(mi->name_)).string());
        }
    }
}

void JsCodeEmitter::EmitInterfaceProxyDBinderImports(
    /* [in] */ StringBuilder& sb)
{
    sb.Append("import ohos.rpc.IRemoteObject;\n");
    sb.Append("import ohos.rpc.RemoteException;\n");
    sb.Append("import ohos.rpc.MessageParcel;\n");
    sb.Append("import ohos.rpc.MessageOption;\n");
}

void JsCodeEmitter::EmitInterfaceProxyParametersImports(
    /* [in] */ StringBuilder& sb)
{
    sb.Append("import ohos.system.version.SystemVersion;\n");
}

void JsCodeEmitter::EmitInterfaceProxyImpl(
    /* [in] */ StringBuilder& sb)
{
    sb.AppendFormat("export default {\n", proxyName_.string(), interfaceName_.string());
    EmitInterfaceProxyMethodImpls(sb, TAB);
    sb.Append("}\n");
}

void JsCodeEmitter::EmitInterfaceProxyConstants(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).AppendFormat(
        "private static final String DESCRIPTOR = \"%s\";\n\n", interfaceFullName_.string());
    EmitInterfaceMethodCommands(sb);
}

void JsCodeEmitter::EmitInterfaceProxyConstructor(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).AppendFormat("public %s(\n", proxyName_.string());
    sb.Append(prefix + TAB).Append("/* [in] */ IRemoteObject remote) {\n");
    sb.Append(prefix + TAB).Append("this.remote = remote;\n");
    sb.Append(prefix).Append("}\n");
    sb.Append("\n");
    sb.Append(prefix).AppendFormat("@Override\n");
    sb.Append(prefix).Append("public IRemoteObject asObject() {\n");
    sb.Append(prefix + TAB).Append("return remote;\n");
    sb.Append(prefix).Append("}\n");
}

void JsCodeEmitter::EmitInterfaceProxyMethodImpls(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    if (metaInterface_->methodNumber_ > 0) {
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            EmitInterfaceProxyMethodImpl(mm, sb, prefix);
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void JsCodeEmitter::EmitInterfaceProxyMethodImpl(
    /* [in] */ MetaMethod* mm,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    if (mm->parameterNumber_ == 0) {
        sb.Append(prefix).AppendFormat("%s() ", MethodName(mm->name_).string());
    } else {
        sb.Append(prefix).AppendFormat("%s(", MethodName(mm->name_).string());
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            EmitInterfaceMethodParameter(mp, sb, prefix + TAB);
            if (i != mm->parameterNumber_ - 1) {
                sb.Append(", ");
            }
        }
        sb.Append(") ");
    }
    EmitInterfaceProxyMethodBody(mm, sb, prefix);
}

void JsCodeEmitter::EmitInterfaceProxyMethodBody(
    /* [in] */ MetaMethod* mm,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append("{\n");
    sb.Append(prefix + TAB).AppendFormat("let option = new rpc.MessageOption();\n");
    sb.Append(prefix + TAB).Append("let data = new rpc.MessageParcel();\n");
    sb.Append(prefix + TAB).Append("let reply = new rpc.MessageParcel();\n");
    sb.Append("\n");

    bool needBlankLine = false;
    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            EmitWriteMethodParameter(mp, "data", sb, prefix + TAB);
            needBlankLine = true;
        } else if ((mp->attributes_ & ATTR_OUT) != 0) {
            MetaType* mt = metaComponent_->types_[mp->typeIndex_];
            if (mt->kind_ == TypeKind::Array) {
                std::string name = UnderlineAdded(mp->name_);
                EmitWriteOutArrayVariable("data", name, mt, sb, prefix + TAB);
            }
        }
    }
    if (needBlankLine) {
        sb.Append("\n");
    }

    if ((mm->properties_ & METHOD_PROPERTY_ONEWAY) == 0) {
        sb.Append(prefix + TAB).AppendFormat("let ret = proxy.sendRequest(COMMAND_%s, data, reply, option)\n",
            ConstantName(mm->name_).string());
        sb.Append(prefix + TAB + TAB).AppendFormat(".then(function (result) {\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("if (result.errCode === 0) {\n");
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            if ((mp->attributes_ & ATTR_OUT) != 0) {
                EmitReadMethodParameter(mp, "reply", sb, prefix + TAB + TAB);
            }
        }
        MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
        if (returnType->kind_ != TypeKind::Void) {
            EmitReadVariable("reply", "result", "msg", returnType, ATTR_IN, sb, prefix + TAB + TAB + TAB);
        } else {
            sb.Append(prefix + TAB + TAB + TAB).Append("console.log(\"void function call success\");\n");
        }
        if (returnType->kind_ != TypeKind::Void) {
            sb.Append(prefix + TAB + TAB + TAB).Append("console.log(\"reply msg: \" + msg);\n");
        }
        sb.Append(prefix + TAB + TAB).AppendFormat("} else {\n");
        sb.Append(prefix + TAB + TAB + TAB).Append(
            "console.log(\"sendRequest failed, errCode: \" + result.errCode);\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("}\n");
        sb.Append(prefix + TAB).AppendFormat("}).catch(function (e) {\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("console.log(\"sendRequest exception:\" + e);\n");
        sb.Append(prefix + TAB).AppendFormat("}).finally (() => {\n");
        sb.Append(prefix + TAB).AppendFormat("console.log(\"sendRequest ends, reclaim parcels\");\n");
        sb.Append(prefix + TAB + TAB).Append("data.reclaim();\n");
        sb.Append(prefix + TAB + TAB).Append("reply.reclaim();\n");
        sb.Append(prefix + TAB).AppendFormat("});\n");
    } else {
        sb.Append(prefix + TAB).Append("try {\n");
        sb.Append(prefix + TAB + TAB).AppendFormat(
            "let ret = proxy.sendRequest(COMMAND_%s, data, reply, option, %sCallback)\n",
            ConstantName(mm->name_).string(), MethodName(mm->name_).string());
        sb.Append(prefix + TAB).Append("} catch(e) {\n");
        sb.Append(prefix + TAB + TAB).AppendFormat("console.log(\"sendRequest exception:\" + e);\n");
        sb.Append(prefix + TAB).AppendFormat("}\n");
    }
    sb.Append(prefix).Append("},\n");
}

void JsCodeEmitter::EmitWriteMethodParameter(
    /* [in] */ MetaParameter* mp,
    /* [in] */ const String& parcelName,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    std::string name = UnderlineAdded(mp->name_);
    EmitWriteVariable(parcelName, name, mt, sb, prefix);
}

void JsCodeEmitter::EmitReadMethodParameter(
    /* [in] */ MetaParameter* mp,
    /* [in] */ const String& parcelName,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    std::string name = UnderlineAdded(mp->name_);
    EmitReadOutVariable(parcelName, name, mt, sb, prefix);
}

void JsCodeEmitter::EmitInterfaceStub()
{
    String filePath = String::Format("%s/%s.js", directory_.string(), FileName(stubName_).string());
    File file(filePath, File::WRITE);

    StringBuilder sb;
    EmitInterfaceImports(sb);
    sb.Append("\n");
    EmitInterfaceStubConstants(sb);
    sb.Append("\n");
    EmitInterfaceStubImpl(sb);
    sb.Append("\n");

    String data = sb.ToString();
    file.WriteData(data.string(), data.GetLength());
    file.Flush();
    file.Close();
}

void JsCodeEmitter::EmitInterfaceStubImpl(
    /* [in] */ StringBuilder& sb)
{
    sb.AppendFormat("class %s extends rpc.RemoteObject {\n", stubName_.string());
    EmitInterfaceStubConstructor(sb, TAB);
    sb.Append("\n");
    EmitInterfaceStubMethodImpls(sb, TAB);
    sb.Append("}\n");
}

void JsCodeEmitter::EmitInterfaceStubConstants(/* [in] */ StringBuilder& sb)
{
    EmitInterfaceMethodCommands(sb);
}

void JsCodeEmitter::EmitJsCallBack(/* [in] */ StringBuilder& sb)
{
    if (metaInterface_->methodNumber_ > 0) {
        for (int i = 0; i < metaInterface_->methodNumber_; i++) {
            MetaMethod* mm = metaInterface_->methods_[i];
            if ((mm->properties_ & METHOD_PROPERTY_ONEWAY) == 1) {
                sb.AppendFormat("function %sCallback(result) {\n",  MethodName(mm->name_).string());
                sb.Append(TAB).AppendFormat("if (result.errCode === 0) {\n");

                MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
                if (returnType->kind_ != TypeKind::Void) {
                    EmitReadVariable("reply", "msg", returnType, ATTR_IN, sb, String(TAB) + TAB + TAB);
                } else {
                sb.Append(String(TAB) + TAB + TAB).Append("console.log(\"void function call success\");\n");
                }
                if (returnType->kind_ != TypeKind::Void) {
                    sb.Append(String(TAB) + TAB + TAB).Append("console.log(\"reply msg: \" + msg);\n");
                }
                sb.Append(String(TAB)).AppendFormat("} else {\n");
                sb.Append(String(TAB) + TAB).Append(
                    "console.log(\"sendRequest failed, errCode: \" + result.errCode);\n");
                sb.Append(String(TAB)).AppendFormat("}\n");
                sb.Append(String(TAB)).AppendFormat("console.log(\"sendRequest ends, reclaim parcels\");\n");
                sb.Append(String(TAB)).Append("data.reclaim();\n");
                sb.Append(String(TAB)).Append("reply.reclaim();\n");
                sb.Append("}\n");
            }
            if (i != metaInterface_->methodNumber_ - 1) {
                sb.Append("\n");
            }
        }
    }
}

void JsCodeEmitter::EmitInterfaceStubConstructor(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).AppendFormat("constructor(des) {\n");
    sb.Append(prefix + TAB).Append("super(des);\n");
    sb.Append(prefix).Append("}\n");
}

void JsCodeEmitter::EmitInterfaceStubMethodImpls(
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).Append("onRemoteRequest(code, data, reply, option) {\n");
    sb.Append(prefix + TAB).Append("switch (code) {\n");
    for (int i = 0; i < metaInterface_->methodNumber_; i++) {
        MetaMethod* mm = metaInterface_->methods_[i];
        EmitInterfaceStubMethodImpl(mm, sb, prefix + TAB + TAB);
    }
    sb.Append(prefix + TAB).Append(TAB).Append("default:\n");
    sb.Append(prefix + TAB).Append(TAB).Append(TAB).Append(
        "return super.onRemoteRequest(code, data, reply, option);\n");
    sb.Append(prefix + TAB).Append("}\n");
    sb.Append(prefix).Append("}\n");
}

void JsCodeEmitter::EmitInterfaceStubMethodImpl(
    /* [in] */ MetaMethod* mm,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).AppendFormat("case COMMAND_%s: {\n", ConstantName(mm->name_).string());
    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_IN) != 0) {
            MetaType* mt = metaComponent_->types_[mp->typeIndex_];
            EmitReadVariable("data", UnderlineAdded(mp->name_), mt, ATTR_IN, sb, prefix + TAB);
        } else if ((mp->attributes_ & ATTR_OUT) != 0) {
            EmitLocalVariable(mp, sb, prefix + TAB);
        }
    }

    MetaType* returnType = metaComponent_->types_[mm->returnTypeIndex_];
    if (mm->parameterNumber_ == 0) {
        if (returnType->kind_ == TypeKind::Void) {
            sb.Append(prefix + TAB).AppendFormat("%s();\n", mm->name_);
        } else {
            sb.Append(prefix + TAB).AppendFormat("let result = %s();\n", mm->name_);
        }
    } else {
        if (returnType->kind_ == TypeKind::Void) {
            sb.Append(prefix + TAB).AppendFormat("%s(", mm->name_);
        } else {
            sb.Append(prefix + TAB).AppendFormat("let result = %s(", mm->name_);
        }
        for (int i = 0; i < mm->parameterNumber_; i++) {
            MetaParameter* mp = mm->parameters_[i];
            sb.Append(UnderlineAdded(mp->name_).c_str());
            if (i != mm->parameterNumber_ - 1) {
                sb.Append(", ");
            }
        }
        sb.AppendFormat(");\n", mm->name_);
    }

    for (int i = 0; i < mm->parameterNumber_; i++) {
        MetaParameter* mp = mm->parameters_[i];
        if ((mp->attributes_ & ATTR_OUT) != 0) {
            EmitWriteMethodParameter(mp, "reply", sb, prefix + TAB);
        }
    }
    if (returnType->kind_ != TypeKind::Void) {
        EmitWriteVariable("reply", "result", returnType, sb, prefix + TAB);
    }
    sb.Append(prefix + TAB).Append("return true;\n");
    sb.Append(prefix).Append("}\n");
}

void JsCodeEmitter::EmitInterfaceMethodCommands(/* [in] */ StringBuilder& sb)
{
    for (int i = 0; i < metaInterface_->methodNumber_; i++) {
        MetaMethod* mm = metaInterface_->methods_[i];
        sb.AppendFormat("const COMMAND_%s = %d;\n", ConstantName(mm->name_).string(), i);
    }
}

void JsCodeEmitter::EmitLicense(
    /* [in] */ StringBuilder& sb)
{
    sb.Append(metaInterface_->license_).Append("\n");
}

void JsCodeEmitter::EmitPackage(
    /* [in] */ StringBuilder& sb)
{
    String package = metaInterface_->namespace_;
    int index = package.LastIndexOf('.');
    sb.AppendFormat("package %s;\n", index != -1 ? package.Substring(0, index).string() : package.string());
}

void JsCodeEmitter::EmitWriteVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("%s.writeInt(%s ? 1 : 0);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("%s.writeInt(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("%s.writeLong(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("%s.writeFloat(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("%s.writeDouble(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("%s.writeString(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable:
            if (EmitType(mt).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("%s.writeRemoteObject(%s);\n", parcelName.string(), name.c_str());
                break;
            }
            sb.Append(prefix).AppendFormat("%s.writeSequenceable(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Interface:
            sb.Append(prefix).AppendFormat("%s.writeRemoteObject(%s.asObject());\n", parcelName.string(),
                name.c_str());
            break;
        case TypeKind::List: {
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            sb.Append(prefix).AppendFormat("%s.writeInt(%s.size());\n", parcelName.string(), name.c_str());
            sb.Append(prefix).AppendFormat("for (%s element : %s) {\n",
                EmitType(innerType).string(), name.c_str());
            EmitWriteVariable(parcelName, "element", innerType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            sb.Append(prefix).AppendFormat("%s.writeInt(%s.size());\n", parcelName.string(), name.c_str());
            sb.Append(prefix).AppendFormat("for (Map.Entry<%s, %s> entry : %s.entrySet()) {\n",
                EmitType(keyType).string(), EmitType(valueType).string(), name.c_str());
            EmitWriteVariable(parcelName, "entry.getKey()", keyType, sb, prefix + TAB);
            EmitWriteVariable(parcelName, "entry.getValue()", valueType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            sb.Append(prefix).AppendFormat("if (%s == null) {\n", name.c_str());
            sb.Append(prefix).AppendFormat("    %s.writeInt(-1);\n", parcelName.string());
            sb.Append(prefix).Append("} else { \n");
            EmitWriteArrayVariable(parcelName, name, innerType, sb, prefix + TAB);
            sb.Append(prefix).Append("}\n");
            break;
        }
        default:
            break;
    }
}

void JsCodeEmitter::EmitWriteArrayVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("%s.writeBooleanArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Char:
            sb.Append(prefix).AppendFormat("%s.writeCharArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Byte:
            sb.Append(prefix).AppendFormat("%s.writeByteArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("%s.writeShortArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("%s.writeIntArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("%s.writeLongArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("%s.writeFloatArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("%s.writeDoubleArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("%s.writeStringArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable:
            sb.Append(prefix).AppendFormat("%s.writeSequenceableArray(%s);\n", parcelName.string(), name.c_str());
            break;
        default:
            break;
    }
}

void JsCodeEmitter::EmitWriteOutArrayVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    sb.Append(prefix).AppendFormat("if (%s == null) {\n", name.c_str());
    sb.Append(prefix).AppendFormat("    %s.writeInt(-1);\n", parcelName.string());
    sb.Append(prefix).Append("} else {\n");
    sb.Append(prefix).AppendFormat("    %s.writeInt(%s.length);\n", parcelName.string(), name.c_str());
    sb.Append(prefix).Append("}\n");
}

void JsCodeEmitter::EmitReadVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ unsigned int attributes,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("let %s = %s.readInt() == 1 ? true : false;\n", name.c_str(),
                parcelName.string());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("let %s = %s.readInt();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("let %s = %s.readInt();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("let %s = %s.readLong();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("let %s = %s.readFloat();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("let %s = %s.readDouble();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("let %s = %s.readString();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Sequenceable:
            if ((attributes & ATTR_OUT) == 0 && EmitType(mt).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("IRemoteObject %s = %s.readRemoteObject();\n",
                    name.c_str(), parcelName.string());
                break;
            }
            if ((attributes & ATTR_OUT) == 0) {
                sb.Append(prefix).AppendFormat("%s %s = new %s();\n",
                    EmitType(mt).string(), name.c_str(), EmitType(mt).string());
            }
            sb.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.c_str());

            break;
        case TypeKind::Interface:
            sb.Append(prefix).AppendFormat("let %s = %s.asInterface(%s.readRemoteObject());\n", name.c_str(),
                StubName(EmitType(mt)).string(), parcelName.string());
            break;
        case TypeKind::List: {
            sb.Append(prefix).AppendFormat("let %s = new Array%s();\n", name.c_str(), EmitType(mt).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitReadVariable(parcelName, "value", innerType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.add(value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            sb.Append(prefix).AppendFormat("let %s = new Hash%s();\n", name.c_str(), EmitType(mt).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, ATTR_IN, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.put(key, value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            if ((attributes & ATTR_MASK) == ATTR_OUT) {
                EmitReadOutArrayVariable(parcelName, name, mt, sb, prefix);
            } else {
                EmitReadArrayVariable(parcelName, name, mt, attributes, sb, prefix);
            }
            break;
        }
        default:
            break;
    }
}

void JsCodeEmitter::EmitReadVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& returnName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ unsigned int attributes,
        /* [in] */ StringBuilder& sb,
        /* [in] */ const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readInt() == 1 ? true : false;\n", name.c_str(),
                returnName.c_str(), parcelName.string());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readInt();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readInt();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readLong();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readFloat();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readDouble();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.readString();\n", name.c_str(), returnName.c_str(),
                parcelName.string());
            break;
        case TypeKind::Sequenceable:
            if ((attributes & ATTR_OUT) == 0 && EmitType(mt).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("IRemoteObject %s = %s.%s.readRemoteObject();\n",
                    name.c_str(), parcelName.string());
                break;
            }
            if ((attributes & ATTR_OUT) == 0) {
                sb.Append(prefix).AppendFormat("%s %s = new %s();\n",
                    EmitType(mt).string(), name.c_str(), EmitType(mt).string());
            }
            sb.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.c_str());

            break;
        case TypeKind::Interface:
            sb.Append(prefix).AppendFormat("let %s = %s.%s.asInterface(%s.readRemoteObject());\n", name.c_str(),
                StubName(EmitType(mt)).string(), parcelName.string());
            break;
        case TypeKind::List: {
            sb.Append(prefix).AppendFormat("let %s = new Array%s();\n", name.c_str(), EmitType(mt).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitReadVariable(parcelName, "value", innerType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.add(value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            sb.Append(prefix).AppendFormat("let %s = new Hash%s();\n", name.c_str(), EmitType(mt).string());
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, ATTR_IN, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.put(key, value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            if ((attributes & ATTR_MASK) == ATTR_OUT) {
                EmitReadOutArrayVariable(parcelName, name, mt, sb, prefix);
            } else {
                EmitReadArrayVariable(parcelName, name, mt, attributes, sb, prefix);
            }
            break;
        }
        default:
            break;
    }
}

void JsCodeEmitter::EmitReadArrayVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ unsigned int attributes,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
    switch (innerType->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("let %s = %s.readBooleanArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Char:
            sb.Append(prefix).AppendFormat("let %s = %s.readCharArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Byte:
            sb.Append(prefix).AppendFormat("let %s = %s.readByteArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("let %s = %s.readShortArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("let %s = %s.readIntArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("let %s = %s.readLongArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("let %s = %s.readFloatArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("let %s = %s.readDoubleArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("let %s = %s.readStringArray();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Sequenceable:
            sb.Append(prefix).AppendFormat("int size = %s.readInt();\n", parcelName.string());
            sb.Append(prefix).AppendFormat("%s %s = new %s[size];\n",
                EmitType(mt).string(), name.c_str(), EmitType(innerType).string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < size; ++i) {\n");
            EmitReadVariable(parcelName, "value", innerType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s[i] = value;\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        default:
            break;
    }
}

void JsCodeEmitter::EmitReadOutArrayVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
    switch (innerType->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("%s.readBooleanArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Char:
            sb.Append(prefix).AppendFormat("%s.readCharArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Byte:
            sb.Append(prefix).AppendFormat("%s.readByteArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("%s.readShortArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("%s.readIntArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("%s.readLongArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("%s.readFloatArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("%s.readDoubleArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("%s.readStringArray(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Sequenceable:
            sb.Append(prefix).AppendFormat("%s.readSequenceableArray(%s);\n", parcelName.string(), name.c_str());
            break;
        default:
            break;
    }
}

void JsCodeEmitter::EmitReadOutVariable(
    /* [in] */ const String& parcelName,
    /* [in] */ const std::string& name,
    /* [in] */ MetaType* mt,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    switch (mt->kind_) {
        case TypeKind::Boolean:
            sb.Append(prefix).AppendFormat("%s = %s.readInt() == 1 ? true : false;\n",
                name.c_str(), parcelName.string());
            break;
        case TypeKind::Char:
        case TypeKind::Byte:
        case TypeKind::Short:
            sb.Append(prefix).AppendFormat("%s = (%s)%s.readInt();\n",
                name.c_str(), EmitType(mt).string(), parcelName.string());
            break;
        case TypeKind::Integer:
            sb.Append(prefix).AppendFormat("%s = %s.readInt();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Long:
            sb.Append(prefix).AppendFormat("%s = %s.readLong();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Float:
            sb.Append(prefix).AppendFormat("%s = %s.readFloat();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Double:
            sb.Append(prefix).AppendFormat("%s = %s.readDouble();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::String:
            sb.Append(prefix).AppendFormat("%s = %s.readString();\n", name.c_str(), parcelName.string());
            break;
        case TypeKind::Sequenceable:
            if (EmitType(mt).Equals("IRemoteObject")) {
                sb.Append(prefix).AppendFormat("%s = %s.readRemoteObject();\n", name.c_str(), parcelName.string());
                break;
            }
            sb.Append(prefix).AppendFormat("%s.readSequenceable(%s);\n", parcelName.string(), name.c_str());
            break;
        case TypeKind::Interface:
            sb.Append(prefix).AppendFormat("%s = %s.asInterface(%s.readRemoteObject());\n", name.c_str(),
                StubName(EmitType(mt)).string(), parcelName.string());
            break;
        case TypeKind::List: {
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* innerType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            EmitReadVariable(parcelName, "value", innerType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.add(value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Map: {
            sb.Append(prefix).AppendFormat("int %sSize = %s.readInt();\n", name.c_str(), parcelName.string());
            sb.Append(prefix).AppendFormat("for (int i = 0; i < %sSize; ++i) {\n", name.c_str());
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            EmitReadVariable(parcelName, "key", keyType, ATTR_IN, sb, prefix + TAB);
            EmitReadVariable(parcelName, "value", valueType, ATTR_IN, sb, prefix + TAB);
            sb.Append(prefix + TAB).AppendFormat("%s.put(key, value);\n", name.c_str());
            sb.Append(prefix).Append("}\n");
            break;
        }
        case TypeKind::Array: {
            EmitReadOutArrayVariable(parcelName, name, mt, sb, prefix);
            break;
        }
        default:
            break;
    }
}

void JsCodeEmitter::EmitLocalVariable(
    /* [in] */ MetaParameter* mp,
    /* [in] */ StringBuilder& sb,
    /* [in] */ const String& prefix)
{
    MetaType* mt = metaComponent_->types_[mp->typeIndex_];
    std::string name = UnderlineAdded(mp->name_);
    if (mt->kind_ == TypeKind::Sequenceable) {
        sb.Append(prefix).AppendFormat("let %s = new %s();\n", name.c_str(), EmitType(mt).string());
    } else if (mt->kind_ == TypeKind::List) {
        sb.Append(prefix).AppendFormat("let %s = new Array%s();\n", name.c_str(), EmitType(mt).string());
    } else if (mt->kind_ == TypeKind::Map) {
        sb.Append(prefix).AppendFormat("let %s = new Hash%s();\n", name.c_str(), EmitType(mt).string());
    } else {
        sb.Append(prefix).AppendFormat("let %s;\n", name.c_str());
    }
}

String JsCodeEmitter::EmitType(
    /* [in] */ MetaType* mt)
{
    switch(mt->kind_) {
        case TypeKind::Char:
            return "char";
        case TypeKind::Boolean:
            return "boolean";
        case TypeKind::Byte:
            return "byte";
        case TypeKind::Short:
            return "short";
        case TypeKind::Integer:
            return "int";
        case TypeKind::Long:
            return "long";
        case TypeKind::Float:
            return "float";
        case TypeKind::Double:
            return "double";
        case TypeKind::String:
            return "String";
        case TypeKind::Void:
            return "void";
        case TypeKind::Sequenceable: {
            MetaSequenceable* mp = metaComponent_->sequenceables_[mt->index_];
            return mp->name_;
        }
        case TypeKind::Interface: {
            MetaInterface* mi = metaComponent_->interfaces_[mt->index_];
            return mi->name_;
        }
        case TypeKind::List: {
            MetaType* elementType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            return String::Format("List<%s>", EmitType(elementType).string());
        }
        case TypeKind::Map: {
            MetaType* keyType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            MetaType* valueType = metaComponent_->types_[mt->nestedTypeIndexes_[1]];
            return String::Format("Map<%s, %s>", EmitType(keyType).string(), EmitType(valueType).string());
        }
        case TypeKind::Array: {
            MetaType* elementType = metaComponent_->types_[mt->nestedTypeIndexes_[0]];
            return String::Format("%s[]", EmitType(elementType).string());
        }
        default:
            return "unknown type";
    }
}

String JsCodeEmitter::FileName(
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    return name.Replace('.', '/');
}

String JsCodeEmitter::MethodName(
    /* [in] */ const String& name)
{
    if (name.IsEmpty() || islower(name[0])) {
        return name;
    }
    return String::Format("%c%s", tolower(name[0]), name.Substring(1).string());
}

String JsCodeEmitter::ConstantName(
    /* [in] */ const String& name)
{
    if (name.IsEmpty()) {
        return name;
    }

    StringBuilder sb;

    for (int i = 0; i < name.GetLength(); i++) {
        char c = name[i];
        if (isupper(c) != 0) {
            if (i > 1) {
                sb.Append('_');
            }
            sb.Append(c);
        } else {
            sb.Append(toupper(c));
        }
    }

    return sb.ToString();
}

String JsCodeEmitter::StubName(
    /* [in] */ const String& name)
{
    return name.StartsWith("I") ? (name.Substring(1) + "Stub") : (name + "Stub");
}

const std::string JsCodeEmitter::UnderlineAdded(const String& originName)
{
    std::string underline("_");
    return underline + std::string(originName.string());
}

}
}
