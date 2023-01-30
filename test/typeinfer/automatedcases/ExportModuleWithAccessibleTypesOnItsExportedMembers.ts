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

// === tests/cases/conformance/internalModules/exportDeclarations/ExportModuleWithAccessibleTypesOnItsExportedMembers.ts ===
declare function AssertType(value:any, type:string):void;
module A {

    export class Point {
        constructor(public x: number, public y: number) { }
    }

    export module B {
        export let Origin: Point = new Point(0, 0);

        export class Line {
            constructor(start: Point, end: Point) {

            }

            static fromOrigin(p: Point) {
AssertType(new Line({ x: 0, y: 0 }, p), "Line");
AssertType(Line, "typeof Line");
AssertType({ x: 0, y: 0 }, "{ x: number; y: number; }");
AssertType(x, "number");
AssertType(0, "int");
AssertType(y, "number");
AssertType(0, "int");
AssertType(p, "Point");
                return new Line({ x: 0, y: 0 }, p);
            }
        }
    }
}

