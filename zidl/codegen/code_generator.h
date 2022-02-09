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

#ifndef OHOS_ZIDL_CODEGENERATOR_H
#define OHOS_ZIDL_CODEGENERATOR_H

#include "codegen/code_emitter.h"
#include "metadata/metadata.h"
#include "util/autoptr.h"
#include "util/string.h"

namespace OHOS {
namespace Zidl {

class CodeGenerator {
public:
    CodeGenerator(
        /* [in] */ MetaComponent* mc,
        /* [in] */ const String& language,
        /* [in] */ const String& dir);

    ~CodeGenerator() = default;

    bool Generate();

private:
    bool ResolveDirectory();


    static const char* TAG;
    String targetLanguage_;
    String targetDirectory_;
    MetaComponent* metaComponent_;
    AutoPtr<CodeEmitter> emitter_;
};

}
}

#endif // OHOS_ZIDL_CODEGENERATOR_H
