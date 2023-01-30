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

// === tests/cases/compiler/indexerA.ts ===
declare function AssertType(value:any, type:string):void;
class JQueryElement {
    id:string;
}

class JQuery {
    [n:number]:JQueryElement
}

let jq:JQuery={ 0: { id : "a" }, 1: { id : "b" } };
AssertType(jq, "JQuery");
AssertType({ 0: { id : "a" }, 1: { id : "b" } }, "{ 0: { id: string; }; 1: { id: string; }; }");
AssertType(0, "{ id: string; }");
AssertType({ id : "a" }, "{ id: string; }");
AssertType(id, "string");
AssertType("a", "string");
AssertType(1, "{ id: string; }");
AssertType({ id : "b" }, "{ id: string; }");
AssertType(id, "string");
AssertType("b", "string");

jq[0].id;
AssertType(jq[0].id, "string");


