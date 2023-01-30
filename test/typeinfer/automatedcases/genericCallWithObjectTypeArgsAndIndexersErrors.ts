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

// === tests/cases/conformance/types/typeRelationships/typeInference/genericCallWithObjectTypeArgsAndIndexersErrors.ts ===
declare function AssertType(value:any, type:string):void;
// Type inference infers from indexers in target type, error cases

function foo<T>(x: T) {
AssertType(x, "T");
    return x;
}

function other<T>(arg: T) {
    let b: {
AssertType(b, "{ [string]: Object; [number]: T; }");

        [x: string]: Object;
AssertType(x, "string");

        [x: number]: T; // ok, T is a subtype of Object because its apparent type is {
AssertType(x, "number");
}

    };
    let r2 = foo(b); // T
AssertType(r2, "{ [string]: Object; [number]: T; }");
AssertType(foo(b), "{ [string]: Object; [number]: T; }");
AssertType(foo, "<T>(T) => T");
AssertType(b, "{ [string]: Object; [number]: T; }");
}

function other3<T extends U, U extends Date>(arg: T) {
    let b: {
AssertType(b, "{ [string]: Object; [number]: T; }");

        [x: string]: Object;
AssertType(x, "string");

        [x: number]: T;
AssertType(x, "number");

    };
    let r2 = foo(b);
AssertType(r2, "{ [string]: Object; [number]: T; }");
AssertType(foo(b), "{ [string]: Object; [number]: T; }");
AssertType(foo, "<T>(T) => T");
AssertType(b, "{ [string]: Object; [number]: T; }");

    let d = r2[1];
AssertType(d, "T");
AssertType(r2[1], "T");
AssertType(r2, "{ [string]: Object; [number]: T; }");
AssertType(1, "int");

    let e = r2['1'];
AssertType(e, "T");
AssertType(r2['1'], "T");
AssertType(r2, "{ [string]: Object; [number]: T; }");
AssertType('1', "string");

    let u: U = r2[1]; // ok
AssertType(u, "U");
AssertType(r2[1], "T");
AssertType(r2, "{ [string]: Object; [number]: T; }");
AssertType(1, "int");
}

