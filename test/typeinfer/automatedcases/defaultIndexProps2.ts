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

// === tests/cases/compiler/defaultIndexProps2.ts ===
declare function AssertType(value:any, type:string):void;
class Foo {
	public v = "Yo";
}

let f = new Foo();
AssertType(f, "Foo");
AssertType(new Foo(), "Foo");
AssertType(Foo, "typeof Foo");

// WScript.Echo(f[0]);

let o = {v:"Yo2"};
AssertType(o, "{ v: string; }");
AssertType({v:"Yo2"}, "{ v: string; }");
AssertType(v, "string");
AssertType("Yo2", "string");

// WScript.Echo(o[0]);

1[0];
AssertType(1[0], "error");
AssertType(1, "int");
AssertType(0, "int");

let q = "s"[0];
AssertType(q, "string");
AssertType("s"[0], "string");
AssertType("s", "string");
AssertType(0, "int");


