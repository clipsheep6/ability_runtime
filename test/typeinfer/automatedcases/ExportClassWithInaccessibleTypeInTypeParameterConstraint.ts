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

// === tests/cases/conformance/internalModules/exportDeclarations/ExportClassWithInaccessibleTypeInTypeParameterConstraint.ts ===
declare function AssertType(value:any, type:string):void;
module A {

    class Point {
        x: number;
        y: number;
    }

    export let Origin: Point = { x: 0, y: 0 };

    export class Point3d extends Point {
        z: number;
    }

    export let Origin3d: Point3d = { x: 0, y: 0, z: 0 };

    export class Line<TPoint extends Point>{
        constructor(public start: TPoint, public end: TPoint) { }

        static fromorigin2d(p: Point): Line<Point>{
AssertType(null, "null");
            return null;
        }
    }
}


