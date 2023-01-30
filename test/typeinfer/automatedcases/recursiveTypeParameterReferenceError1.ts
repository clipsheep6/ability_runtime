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

// === tests/cases/compiler/recursiveTypeParameterReferenceError1.ts ===
declare function AssertType(value:any, type:string):void;
class X<T> { }
interface Foo<T> {
    z: Foo<X<T>>; // error
}
let f: Foo<number>;
AssertType(f, "Foo<number>");

let r = f.z; 
AssertType(r, "Foo<X<number>>");
AssertType(f.z, "Foo<X<number>>");


class C2<T> {
    x: T;
}
interface Foo2<T> {
    ofC4: C2<{ x: T }> // ok
}
let f2: Foo2<number>;
AssertType(f2, "Foo2<number>");

let r2 = f2.ofC4;
AssertType(r2, "C2<{ x: number; }>");
AssertType(f2.ofC4, "C2<{ x: number; }>");


