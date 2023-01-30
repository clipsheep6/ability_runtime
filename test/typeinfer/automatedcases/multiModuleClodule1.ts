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

// === tests/cases/compiler/multiModuleClodule1.ts ===
declare function AssertType(value:any, type:string):void;
class C {
    constructor(x: number) { }
    foo() { }
    bar() { }
    static boo() { }
}

module C {
    export let x = 1;
    let y = 2;
}
module C {
    export function foo() { }
    function baz() { 
AssertType('', "string");
return ''; 
}
}

let c = new C(C.x);
AssertType(c, "C");
AssertType(new C(C.x), "C");
AssertType(C, "typeof C");
AssertType(C.x, "number");

c.foo = C.foo;
AssertType(c.foo = C.foo, "() => void");
AssertType(c.foo, "() => void");
AssertType(C.foo, "() => void");


