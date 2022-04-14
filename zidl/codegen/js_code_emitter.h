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

#ifndef OHOS_ZIDL_JSCODEEMITTER_H
#define OHOS_ZIDL_JSCODEEMITTER_H

#include <string>
#include <vector>

#include "codegen/code_emitter.h"
#include "util/string_builder.h"

namespace OHOS {
namespace Zidl {

class JsCodeEmitter : public CodeEmitter {
public:
    JsCodeEmitter(
        /* [in] */ MetaComponent* mc)
        : CodeEmitter(mc)
    {}

    void EmitInterface() override;

    void EmitInterfaceProxy() override;

    void EmitInterfaceStub() override;

    struct Parameter {
        bool operator< (const Parameter &para) const {
            return this->attr_ < para.attr_;
        }
        std::string name_;
        std::string type_;
        unsigned int attr_ = 0;
    };

    struct Method
    {
        Parameter retParameter_;
        std::vector<Parameter> parameters_;
        std::string callbackName_;
        std::string name_;
        std::string exportFunction_;
        unsigned int properties_;
    };

private:
    void EmitInterfaceImports(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceProxyImports(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceSelfDefinedTypeImports(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceDefinition(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceMethods(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethod(
        /* [in] */ MetaMethod* metaMethod,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitMethodInParameter(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const std::string& name,
        /* [in] */ const std::string& type,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodExportCallback(
        /* [in] */ Method& m,
        /* [in] */ const Parameter& para,
        /* [in] */ bool isLast);

    void EmitInterfaceProxyImpl(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceProxyConstructor(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodImpls(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodImpl(
        /* [in] */ MetaMethod* metaMethod,
        /* [in] */ int methodIndex,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceProxyMethodBody(
        /* [in] */ MetaMethod* metaMethod,
        /* [in] */ int methodIndex,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitWriteMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ const String& parcelName,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitReadMethodParameter(
        /* [in] */ MetaParameter* mp,
        /* [in] */ const String& parcelName,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubImpl(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitInterfaceStubConstructor(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubMethodImpls(
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceStubMethodImpl(
        /* [in] */ MetaMethod* metaMethod,
        /* [in] */ int methodIndex,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitInterfaceMethodCommands(/* [in] */ StringBuilder& stringBuilder);

    void EmitLicense(
        /* [in] */ StringBuilder& stringBuilder);

    void EmitWriteVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitReadVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ unsigned int attributes,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitReadOutVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitReadArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ unsigned int attributes,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitWriteArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    void EmitReadOutArrayVariable(
        /* [in] */ const String& parcelName,
        /* [in] */ const std::string& name,
        /* [in] */ MetaType* mt,
        /* [in] */ StringBuilder& stringBuilder,
        /* [in] */ const String& prefix);

    String EmitType(
        /* [in] */ MetaType* mt);

    String FileName(
        /* [in] */ const String& name);

    String MethodName(
        /* [in] */ const String& name);

    String ConstantName(
        /* [in] */ const String& name);

    String StubName(
      /* [in] */ const String& name);

    bool CheckInterfaceType();

    const std::string UnderlineAdded(const String& name);

    std::vector<Method> methods_;
};

}
}

#endif // OHOS_ZIDL_JSCODEEMITTER_H
