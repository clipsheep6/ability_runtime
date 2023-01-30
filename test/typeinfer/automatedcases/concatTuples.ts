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

// === tests/cases/compiler/concatTuples.ts ===
declare function AssertType(value:any, type:string):void;
let ijs: [number, number][] = [[1, 2]];
AssertType(ijs, "[number, number][]");
AssertType([[1, 2]], "[number, number][]");
AssertType([1, 2], "[number, number]");
AssertType(1, "int");
AssertType(2, "int");

ijs = ijs.concat([[3, 4], [5, 6]]);
AssertType(ijs = ijs.concat([[3, 4], [5, 6]]), "[number, number][]");
AssertType(ijs, "[number, number][]");
AssertType(ijs.concat([[3, 4], [5, 6]]), "[number, number][]");
AssertType(ijs.concat, "{ (...ConcatArray<[number, number]>[]): [number, number][]; (...(union)[]): [number, number][]; }");
AssertType([[3, 4], [5, 6]], "[number, number][]");
AssertType([3, 4], "[number, number]");
AssertType(3, "int");
AssertType(4, "int");
AssertType([5, 6], "[number, number]");
AssertType(5, "int");
AssertType(6, "int");


