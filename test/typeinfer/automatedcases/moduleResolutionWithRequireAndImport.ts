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

// === /index.ts ===
declare function AssertType(value:any, type:string):void;
declare const require: any;
AssertType(require, "any");

const a: typeof import('./other') = null as any
AssertType(a, "typeof import("/other")");
AssertType(null as any, "any");
AssertType(null, "null");

function foo() {
    const a = require('../outside-of-rootdir/foo');
AssertType(a, "any");
AssertType(require('../outside-of-rootdir/foo'), "any");
AssertType(require, "any");
AssertType('../outside-of-rootdir/foo', "string");

    const { other }: { other: string } = require('./other');
AssertType(other, "string");
AssertType(other, "string");
AssertType(require('./other'), "any");
AssertType(require, "any");
AssertType('./other', "string");
}


// === /other.ts ===
declare function AssertType(value:any, type:string):void;
export const other = 123;
AssertType(other, "int");
AssertType(123, "int");


