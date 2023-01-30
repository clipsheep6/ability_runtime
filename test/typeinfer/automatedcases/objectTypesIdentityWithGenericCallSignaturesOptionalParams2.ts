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

// === tests/cases/conformance/types/typeRelationships/typeAndMemberIdentity/objectTypesIdentityWithGenericCallSignaturesOptionalParams2.ts ===
declare function AssertType(value:any, type:string):void;
// Two call or construct signatures are considered identical when they have the same number of type parameters and, considering those 
// parameters pairwise identical, have identical type parameter constraints, identical number of parameters with identical kind(required, 
// optional or rest) and types, and identical return types.

class A {
    foo<T, U>(x: T, y?: U): T { 
AssertType(null, "null");
return null; 
}
}

class B<T, U> {
    foo(x: T, y?: U): T { 
AssertType(null, "null");
return null; 
}
}

class C<T, U> {
    foo(x: T, y?: U): T { 
AssertType(null, "null");
return null; 
}
}

interface I<T, U> {
    foo(x: T, y?: U): T;
}

interface I2 {
    foo<T, U>(x: T, y?: U): T;
}

let a: { foo<T, U>(x: T, y?: U): T 
AssertType(a, "{ foo<T, U>(T, ?U): T; }");

AssertType(foo, "<T, U>(T, ?U) => T");

AssertType(x, "T");

AssertType(y, "U");
}

let b = { foo<T, U>(x: T, y?: U) { 
AssertType(b, "{ foo<T, U>(T, ?U): T; }");
AssertType({ foo<T, U>(x: T, y?: U) { return x; } }, "{ foo<T, U>(T, ?U): T; }");
AssertType(foo, "<T, U>(T, ?U) => T");
AssertType(x, "T");
AssertType(y, "U");
AssertType(x, "T");
return x; } };

function foo1(x: A);
function foo1(x: A); // error
function foo1(x: any) { }

function foo1b(x: B<string, number>);
function foo1b(x: B<string, number>); // error
function foo1b(x: any) { }

function foo1c(x: C<string, number>);
function foo1c(x: C<string, number>); // error
function foo1c(x: any) { }

function foo2(x: I<string, number>);
function foo2(x: I<string, number>); // error
function foo2(x: any) { }

function foo3(x: typeof a);
function foo3(x: typeof a); // error
function foo3(x: any) { }

function foo4(x: typeof b);
function foo4(x: typeof b); // error
function foo4(x: any) { }

function foo5(x: A);
function foo5(x: B<string, number>); // ok
function foo5(x: any) { }

function foo5b(x: A);
function foo5b(x: C<string, number>); // ok
function foo5b(x: any) { }

function foo6(x: A);
function foo6(x: I<string, number>); // ok
function foo6(x: any) { }

function foo7(x: A);
function foo7(x: typeof a); // no error, bug?
function foo7(x: any) { }

function foo8(x: B<string, number>);
function foo8(x: I<string, number>); // error
function foo8(x: any) { }

function foo9(x: B<string, number>);
function foo9(x: C<string, number>); // error
function foo9(x: any) { }

function foo10(x: B<string, number>);
function foo10(x: typeof a); // ok
function foo10(x: any) { }

function foo11(x: B<string, number>);
function foo11(x: typeof b); // ok
function foo11(x: any) { }

function foo12(x: I<string, number>);
function foo12(x: C<string, number>); // error
function foo12(x: any) { }

function foo12b(x: I2);
function foo12b(x: C<string, number>); // ok
function foo12b(x: any) { }

function foo13(x: I<string, number>);
function foo13(x: typeof a); // ok
function foo13(x: any) { }

function foo14(x: I<string, number>);
function foo14(x: typeof b); // ok
function foo14(x: any) { }

function foo15(x: I2);
function foo15(x: C<string, number>); // ok
function foo15(x: any) { }

