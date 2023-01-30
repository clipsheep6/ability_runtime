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

// === tests/cases/conformance/salsa/privateConstructorFunction.js ===
declare function AssertType(value:any, type:string):void;
{
    // make sure not to crash when parent's a block rather than a source file or some other
    // symbol-having node.

    /** @private */
    function C() {
AssertType(C, "typeof C");

        this.x = 1
AssertType(this.x = 1, "int");
AssertType(this.x, "any");
AssertType(this, "this");
AssertType(1, "int");
    }
    new C()
AssertType(new C(), "C");
AssertType(C, "typeof C");
}


