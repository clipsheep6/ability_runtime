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

// === tests/cases/conformance/es6/unicodeExtendedEscapes/unicodeExtendedEscapesInRegularExpressions06_ES6.ts ===
declare function AssertType(value:any, type:string):void;
// ES6 Spec - 10.1.1 Static Semantics: UTF16Encoding (cp)
//  1. Assert: 0 ≤ cp ≤ 0x10FFFF.
let x = /\u{10FFFF}/g;
AssertType(x, "RegExp");
AssertType(/\u{10FFFF}/g, "RegExp");


