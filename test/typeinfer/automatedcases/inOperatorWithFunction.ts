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

// === tests/cases/compiler/inOperatorWithFunction.ts ===
declare function AssertType(value:any, type:string):void;
let fn = function (val: boolean) { 
AssertType(fn, "(boolean) => boolean");
return val; 

AssertType(function (val: boolean) { return val; }, "(boolean) => boolean");

AssertType(val, "boolean");

AssertType(val, "boolean");
}

fn("a" in { "a": true });
AssertType(fn("a" in { "a": true }), "boolean");
AssertType(fn, "(boolean) => boolean");
AssertType("a" in { "a": true }, "boolean");
AssertType("a", "string");
AssertType({ "a": true }, "{ a: boolean; }");
AssertType("a", "boolean");
AssertType(true, "boolean");


