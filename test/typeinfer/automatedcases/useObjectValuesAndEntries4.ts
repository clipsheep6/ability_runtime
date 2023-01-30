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

// === tests/cases/conformance/es2017/useObjectValuesAndEntries4.ts ===
declare function AssertType(value:any, type:string):void;
let o = { a: 1, b: 2 };
AssertType(o, "{ a: number; b: number; }");
AssertType({ a: 1, b: 2 }, "{ a: number; b: number; }");
AssertType(a, "number");
AssertType(1, "int");
AssertType(b, "number");
AssertType(2, "int");

for (let x of Object.values(o)) {
    let y = x;
AssertType(y, "number");
AssertType(x, "number");
}

let entries = Object.entries(o);
AssertType(entries, "[string, number][]");
AssertType(Object.entries(o), "[string, number][]");
AssertType(Object.entries, "{ <T>(union): [string, T][]; ({}): [string, any][]; }");
AssertType(o, "{ a: number; b: number; }");


