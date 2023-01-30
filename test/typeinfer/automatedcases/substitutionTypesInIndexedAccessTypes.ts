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

// === tests/cases/compiler/substitutionTypesInIndexedAccessTypes.ts ===
declare function AssertType(value:any, type:string):void;
// Repro from #31086

type UserArgs = {
  select?: boolean
};

type Subset<T, U> = { [key in keyof T]: key extends keyof U ? T[key] : never };

declare function withBoundary<T extends UserArgs>(args?: Subset<T, UserArgs>): T;
declare function withoutBoundary<T extends UserArgs>(args?: T): T;

const boundaryResult = withBoundary({
AssertType(boundaryResult, "{ select: true; }");
AssertType(withBoundary({  select: true,}), "{ select: true; }");
AssertType(withBoundary, "<T extends UserArgs>(?union) => T");
AssertType({  select: true,}, "{ select: true; }");

  select: true,
AssertType(select, "boolean");
AssertType(true, "boolean");

});

const withoutBoundaryResult = withoutBoundary({
AssertType(withoutBoundaryResult, "{ select: true; }");
AssertType(withoutBoundary({  select: true,}), "{ select: true; }");
AssertType(withoutBoundary, "<T extends UserArgs>(?union) => T");
AssertType({  select: true,}, "{ select: true; }");

  select: true,
AssertType(select, "boolean");
AssertType(true, "boolean");

});


