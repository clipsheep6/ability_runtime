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

// === tests/cases/compiler/commentsemitComments.ts ===
declare function AssertType(value:any, type:string):void;
/** Variable comments*/
let myVariable = 10;
AssertType(myVariable, "number");
AssertType(10, "int");

/** function comments*/
function foo(/** parameter comment*/p: number) {
}

/** letiable with function type comment*/
let fooVar: () => void;
AssertType(fooVar, "() => void");

foo(50);
AssertType(foo(50), "void");
AssertType(foo, "(number) => void");
AssertType(50, "int");

fooVar();
AssertType(fooVar(), "void");
AssertType(fooVar, "() => void");

/**class comment*/
class c {
    /** constructor comment*/
    constructor() {
    }

    /** property comment */
    public b = 10;

    /** function comment */
    public myFoo() {
AssertType(this.b, "number");
AssertType(this, "this");
        return this.b;
    }

    /** getter comment*/
    public get prop1() {
AssertType(this.b, "number");
AssertType(this, "this");
        return this.b;
    }

    /** setter comment*/
    public set prop1(val: number) {
        this.b = val;
AssertType(this.b = val, "number");
AssertType(this.b, "number");
AssertType(this, "this");
AssertType(val, "number");
    }

    /** overload signature1*/
    public foo1(a: number): string;
    /** Overload signature 2*/
    public foo1(b: string): string;
    /** overload implementation signature*/
    public foo1(aOrb) {
AssertType(aOrb.toString(), "any");
AssertType(aOrb.toString, "any");
        return aOrb.toString();
    }
}

/**instance comment*/
let i = new c();
AssertType(i, "c");
AssertType(new c(), "c");
AssertType(c, "typeof c");

/** interface comments*/
interface i1 {
    /** caller comments*/
    (a: number): number;

    /** new comments*/
    new (b: string);

    /**indexer property*/
    [a: number]: string;

    /** function property;*/
    myFoo(/*param prop*/a: number): string;

    /** prop*/
    prop: string;
}

/**interface instance comments*/
let i1_i: i1;
AssertType(i1_i, "i1");

/** this is module comment*/
module m1 {
    /** class b */
    export class b {
        constructor(public x: number) {

        }
    }

    /// module m2
    export module m2 {
    }
}

/// this is x
declare let x;
AssertType(x, "any");


