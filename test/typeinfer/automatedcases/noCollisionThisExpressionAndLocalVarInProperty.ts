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

// === tests/cases/compiler/noCollisionThisExpressionAndLocalVarInProperty.ts ===
declare function AssertType(value:any, type:string):void;
class class1 {
    public prop1 = {
        doStuff: (callback) => () => {
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

class class2 {
    constructor() {
        let _this = 2;
AssertType(_this, "number");
AssertType(2, "int");
    }
    public prop1 = {
        doStuff: (callback) => () => {
AssertType(callback(10), "any");
AssertType(callback, "any");
AssertType(10, "int");
            return callback(10);
        }
    }
}

