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

// === tests/cases/conformance/internalModules/DeclarationMerging/ModuleAndEnumWithSameNameAndCommonRoot.ts ===
declare function AssertType(value:any, type:string):void;
module enumdule {

    export class Point {
        constructor(public x: number, public y: number) { }
    }
}

enum enumdule {
    Red, Blue
}

let x: enumdule;
AssertType(x, "enumdule");

let x = enumdule.Red;
AssertType(x, "enumdule");
AssertType(enumdule.Red, "enumdule.Red");

let y: { x: number; y: number };
AssertType(y, "{ x: number; y: number; }");
AssertType(x, "number");
AssertType(y, "number");

let y = new enumdule.Point(0, 0);
AssertType(y, "{ x: number; y: number; }");
AssertType(new enumdule.Point(0, 0), "enumdule.Point");
AssertType(enumdule.Point, "typeof enumdule.Point");
AssertType(0, "int");
AssertType(0, "int");


