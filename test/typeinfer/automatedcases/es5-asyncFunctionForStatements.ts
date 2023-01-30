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

// === tests/cases/compiler/es5-asyncFunctionForStatements.ts ===
declare function AssertType(value:any, type:string):void;
declare let x, y, z, a, b, c;
AssertType(x, "any");
AssertType(y, "any");
AssertType(z, "any");
AssertType(a, "any");
AssertType(b, "any");
AssertType(c, "any");

async function forStatement0() {
    for (x; y; z) { a; 
AssertType(x, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

async function forStatement1() {
    for (await x; y; z) { a; 
AssertType(await x, "any");

AssertType(x, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

async function forStatement2() {
    for (x; await y; z) { a; 
AssertType(x, "any");

AssertType(await y, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

async function forStatement3() {
    for (x; y; await z) { a; 
AssertType(x, "any");

AssertType(y, "any");

AssertType(await z, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

async function forStatement4() {
    for (x; y; z) { await a; 
AssertType(x, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(await a, "any");

AssertType(a, "any");
}
}

async function forStatement5() {
    for (let b; y; z) { a; 
AssertType(b, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

async function forStatement6() {
    for (let c = x; y; z) { a; 
AssertType(c, "any");

AssertType(x, "any");

AssertType(y, "any");

AssertType(z, "any");

AssertType(a, "any");
}
}

