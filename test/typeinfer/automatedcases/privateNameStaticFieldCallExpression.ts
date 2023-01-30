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

// === tests/cases/conformance/classes/members/privateNames/privateNameStaticFieldCallExpression.ts ===
declare function AssertType(value:any, type:string):void;
class A {
    static #fieldFunc = function () { this.x = 10; };
AssertType(this.x = 10, "int");
AssertType(this.x, "any");
AssertType(this, "any");
AssertType(10, "int");

    static #fieldFunc2 = function (a, ...b) {};
    x = 1;
    test() {
        A.#fieldFunc();
AssertType(A.#fieldFunc(), "void");
AssertType(A.#fieldFunc, "() => void");

        A.#fieldFunc?.();
AssertType(A.#fieldFunc?.(), "void");
AssertType(A.#fieldFunc, "() => void");

        const func = A.#fieldFunc;
AssertType(func, "() => void");
AssertType(A.#fieldFunc, "() => void");

        func();
AssertType(func(), "void");
AssertType(func, "() => void");

        new A.#fieldFunc();
AssertType(new A.#fieldFunc(), "any");
AssertType(A.#fieldFunc, "() => void");

        const arr = [ 1, 2 ];
AssertType(arr, "number[]");
AssertType([ 1, 2 ], "number[]");
AssertType(1, "int");
AssertType(2, "int");

        A.#fieldFunc2(0, ...arr, 3);
AssertType(A.#fieldFunc2(0, ...arr, 3), "void");
AssertType(A.#fieldFunc2, "(any, ...any[]) => void");
AssertType(0, "int");
AssertType(...arr, "number");
AssertType(arr, "number[]");
AssertType(3, "int");

        const b = new A.#fieldFunc2(0, ...arr, 3);
AssertType(b, "any");
AssertType(new A.#fieldFunc2(0, ...arr, 3), "any");
AssertType(A.#fieldFunc2, "(any, ...any[]) => void");
AssertType(0, "int");
AssertType(...arr, "number");
AssertType(arr, "number[]");
AssertType(3, "int");

        const str = A.#fieldFunc2`head${1}middle${2}tail`;
AssertType(str, "void");
AssertType(A.#fieldFunc2`head${1}middle${2}tail`, "void");
AssertType(A.#fieldFunc2, "(any, ...any[]) => void");
AssertType(`head${1}middle${2}tail`, "string");
AssertType(1, "int");
AssertType(2, "int");

        this.getClass().#fieldFunc2`test${1}and${2}`;
AssertType(this.getClass().#fieldFunc2`test${1}and${2}`, "void");
AssertType(this.getClass().#fieldFunc2, "(any, ...any[]) => void");
AssertType(this.getClass(), "typeof A");
AssertType(this.getClass, "() => typeof A");
AssertType(this, "this");
AssertType(`test${1}and${2}`, "string");
AssertType(1, "int");
AssertType(2, "int");
    }
    getClass() { 
AssertType(A, "typeof A");
return A; 
}
}


