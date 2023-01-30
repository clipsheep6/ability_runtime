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

// === tests/cases/compiler/noCollisionThisExpressionAndLocalVarInConstructor.ts ===
declare function AssertType(value:any, type:string):void;
class class1 {
    constructor() {
        let x2 = {
AssertType(x2, "{ doStuff: (any) => () => any; }");
AssertType({            doStuff: (callback) => () => {                let _this = 2;                return callback(_this);            }        }, "{ doStuff: (any) => () => any; }");

            doStuff: (callback) => () => {
AssertType(doStuff, "(any) => () => any");
AssertType(callback, "any");
AssertType(() => {                let _this = 2;                return callback(_this);            }, "() => any");
AssertType((callback) => () => {                let _this = 2;                return callback(_this);            }, "(any) => () => any");

                let _this = 2;
AssertType(_this, "number");
AssertType(2, "int");

AssertType(callback(_this), "any");
AssertType(callback, "any");
AssertType(_this, "number");
                return callback(_this);
            }
        }
    }
}

class class2 {
    constructor() {
        let _this = 2;
AssertType(_this, "number");
AssertType(2, "int");

        let x2 = {
AssertType(x2, "{ doStuff: (any) => () => any; }");
AssertType({            doStuff: (callback) => () => {                return callback(_this);            }        }, "{ doStuff: (any) => () => any; }");

            doStuff: (callback) => () => {
AssertType(doStuff, "(any) => () => any");
AssertType(callback, "any");
AssertType(() => {                return callback(_this);            }, "() => any");
AssertType((callback) => () => {                return callback(_this);            }, "(any) => () => any");

AssertType(callback(_this), "any");
AssertType(callback, "any");
AssertType(_this, "number");
                return callback(_this);
            }
        }
    }
}

