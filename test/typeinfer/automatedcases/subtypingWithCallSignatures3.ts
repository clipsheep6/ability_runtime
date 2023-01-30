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

// === tests/cases/conformance/types/typeRelationships/subtypesAndSuperTypes/subtypingWithCallSignatures3.ts ===
declare function AssertType(value:any, type:string):void;

// checking subtype relations for function types as it relates to contextual signature instantiation
// error cases, so function calls will all result in 'any'

module Errors {
    class Base { foo: string; }
    class Derived extends Base { bar: string; }
    class Derived2 extends Derived { baz: string; }
    class OtherDerived extends Base { bing: string; }

    declare function foo2(a2: (x: number) => string[]): typeof a2;
    declare function foo2(a2: any): any;

    declare function foo7(a2: (x: (arg: Base) => Derived) => (r: Base) => Derived2): typeof a2;
    declare function foo7(a2: any): any;

    declare function foo8(a2: (x: (arg: Base) => Derived, y: (arg2: Base) => Derived) => (r: Base) => Derived): typeof a2;
    declare function foo8(a2: any): any;

    declare function foo10(a2: (...x: Base[]) => Base): typeof a2;
    declare function foo10(a2: any): any;

    declare function foo11(a2: (x: { foo: string }, y: { foo: string; bar: string }) => Base): typeof a2;
    declare function foo11(a2: any): any;

    declare function foo12(a2: (x: Array<Base>, y: Array<Derived2>) => Array<Derived>): typeof a2;
    declare function foo12(a2: any): any;

    declare function foo15(a2: (x: { a: string; b: number }) => number): typeof a2;
    declare function foo15(a2: any): any;

    declare function foo16(a2: {
        // type of parameter is overload set which means we can't do inference based on this type
        (x: {
            (a: number): number;
            (a?: number): number;
        }): number[];
        (x: {
            (a: boolean): boolean;
            (a?: boolean): boolean;
        }): boolean[];
    }): typeof a2;
    declare function foo16(a2: any): any;

    declare function foo17(a2: {
        (x: {
            <T extends Derived>(a: T): T;
            <T extends Base>(a: T): T;
        }): any[];
        (x: {
            <T extends Derived2>(a: T): T;
            <T extends Base>(a: T): T;
        }): any[];
    }): typeof a2;
    declare function foo17(a2: any): any;

    let r1 = foo2(<T, U>(x: T) => <U[]>null); // any
    let r1a = [(x: number) => [''], <T, U>(x: T) => <U[]>null];
    let r1b = [<T, U>(x: T) => <U[]>null, (x: number) => ['']];

    let r2arg = <T extends Base, U extends Derived, V extends Derived2>(x: (arg: T) => U) => (r: T) => <V>null;
    let r2arg2 = (x: (arg: Base) => Derived) => (r: Base) => <Derived2>null;
    let r2 = foo7(r2arg); // any
    let r2a = [r2arg2, r2arg];
    let r2b = [r2arg, r2arg2];

    let r3arg = <T extends Base, U extends Derived>(x: (arg: T) => U, y: (arg2: { foo: number; }) => U) => (r: T) => <U>null;
    let r3arg2 = (x: (arg: Base) => Derived, y: (arg2: Base) => Derived) => (r: Base) => <Derived>null;
    let r3 = foo8(r3arg); // any
    let r3a = [r3arg2, r3arg];
    let r3b = [r3arg, r3arg2];

    let r4arg = <T extends Derived>(...x: T[]) => <T>null;
    let r4arg2 = (...x: Base[]) => <Base>null;
    let r4 = foo10(r4arg); // any
    let r4a = [r4arg2, r4arg];
    let r4b = [r4arg, r4arg2];

    let r5arg = <T extends Derived>(x: T, y: T) => <T>null;
    let r5arg2 = (x: { foo: string }, y: { foo: string; bar: string }) => <Base>null;
    let r5 = foo11(r5arg); // any
    let r5a = [r5arg2, r5arg];
    let r5b = [r5arg, r5arg2];

    let r6arg = (x: Array<Base>, y: Array<Derived2>) => <Array<Derived>>null;
    let r6arg2 = <T extends Array<Derived2>>(x: Array<Base>, y: Array<Base>) => <T>null;
    let r6 = foo12(r6arg); // (x: Array<Base>, y: Array<Derived2>) => Array<Derived>
    let r6a = [r6arg2, r6arg];
    let r6b = [r6arg, r6arg2];

    let r7arg = <T>(x: { a: T; b: T }) => <T>null;
    let r7arg2 = (x: { a: string; b: number }) => 1;
    let r7 = foo15(r7arg); // any
    let r7a = [r7arg2, r7arg];
    let r7b = [r7arg, r7arg2];

    let r7arg3 = <T extends Base>(x: { a: T; b: T }) => 1;
    let r7c = foo15(r7arg3); // (x: { a: string; b: number }) => number): number;
    let r7d = [r7arg2, r7arg3];
    let r7e = [r7arg3, r7arg2];

    let r8arg = <T>(x: (a: T) => T) => <T[]>null;
    let r8 = foo16(r8arg); // any

    let r9arg = <T>(x: (a: T) => T) => <any[]>null;
    let r9 = foo17(r9arg); // (x: { <T extends Derived >(a: T): T; <T extends Base >(a: T): T; }): any[]; (x: { <T extends Derived2>(a: T): T; <T extends Base>(a: T): T; }): any[];
}

module WithGenericSignaturesInBaseType {
    declare function foo2(a2: <T>(x: T) => T[]): typeof a2;
    declare function foo2(a2: any): any;
    let r2arg2 = <T>(x: T) => [''];
    let r2 = foo2(r2arg2); // <T>(x:T) => T[] since we can infer from generic signatures now

    declare function foo3(a2: <T>(x: T) => string[]): typeof a2;
    declare function foo3(a2: any): any;
    let r3arg2 = <T>(x: T) => <T[]>null;
    let r3 = foo3(r3arg2); // any
}

