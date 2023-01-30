/*
* Copyright (c) Microsoft Corporation. All rights reserved.
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
*
* This file has been modified by Huawei to verify type inference by adding verification statements.
*/

// === tests/cases/conformance/classes/members/privateNames/privateNameConstructorSignature.ts ===
declare function AssertType(value:any, type:string):void;
interface D {
    x: number;
}
class C {
    #x;
    static test() {
        new C().#x = 10;
AssertType(new C().#x = 10, "int");
AssertType(new C().#x, "any");
AssertType(new C(), "C");
AssertType(C, "typeof C");
AssertType(10, "int");

        const y = new C();
AssertType(y, "C");
AssertType(new C(), "C");
AssertType(C, "typeof C");

        const z = new y();
AssertType(z, "D");
AssertType(new y(), "D");
AssertType(y, "C");

        z.x = 123;
AssertType(z.x = 123, "int");
AssertType(z.x, "number");
AssertType(123, "int");
    }
}
interface C {
    new (): D;
}



