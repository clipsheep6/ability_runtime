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

// === tests/cases/conformance/statements/VariableStatements/everyTypeWithInitializer.ts ===
declare function AssertType(value:any, type:string):void;
interface I {
    id: number;
}

class C implements I {
    id: number;
}

class D<T>{
    source: T;
    recurse: D<T>;
    wrapped: D<D<T>>
}

function F(x: string): number { 
AssertType(42, "int");
return 42; 
}

module M {
    export class A {
        name: string;
    }

    export function F2(x: number): string { 
AssertType(x.toString(), "string");
return x.toString(); 

AssertType(x.toString, "(?number) => string");
}
}

let aNumber = 9.9;
AssertType(aNumber, "number");
AssertType(9.9, "double");

let aString = 'this is a string';
AssertType(aString, "string");
AssertType('this is a string', "string");

let aDate = new Date(12);
AssertType(aDate, "Date");
AssertType(new Date(12), "Date");
AssertType(Date, "DateConstructor");
AssertType(12, "int");

let anObject = new Object();
AssertType(anObject, "Object");
AssertType(new Object(), "Object");
AssertType(Object, "ObjectConstructor");

let anAny = null;
AssertType(anAny, "any");
AssertType(null, "null");

let anOtherAny = <any> new C();
AssertType(anOtherAny, "any");
AssertType(<any> new C(), "any");
AssertType(new C(), "C");
AssertType(C, "typeof C");

let anUndefined = undefined;
AssertType(anUndefined, "any");
AssertType(undefined, "undefined");


let aClass = new C();
AssertType(aClass, "C");
AssertType(new C(), "C");
AssertType(C, "typeof C");

let aGenericClass = new D<string>();
AssertType(aGenericClass, "D<string>");
AssertType(new D<string>(), "D<string>");
AssertType(D, "typeof D");

let anObjectLiteral = { id: 12 };
AssertType(anObjectLiteral, "{ id: number; }");
AssertType({ id: 12 }, "{ id: number; }");
AssertType(id, "number");
AssertType(12, "int");

let aFunction = F;
AssertType(aFunction, "(string) => number");
AssertType(F, "(string) => number");

let aLambda = (x) => 2;
AssertType(aLambda, "(any) => number");
AssertType((x) => 2, "(any) => number");
AssertType(x, "any");
AssertType(2, "int");

let aModule = M;
AssertType(aModule, "typeof M");
AssertType(M, "typeof M");

let aClassInModule = new M.A();
AssertType(aClassInModule, "M.A");
AssertType(new M.A(), "M.A");
AssertType(M.A, "typeof M.A");

let aFunctionInModule = M.F2;
AssertType(aFunctionInModule, "(number) => string");
AssertType(M.F2, "(number) => string");

// no initializer or annotation, so this is an 'any'
let x;
AssertType(x, "any");



