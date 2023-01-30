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

// === tests/cases/conformance/externalModules/foo_0.ts ===
declare function AssertType(value:any, type:string):void;

export class C1 {
	m1 = 42;
	static s1 = true;
}

export interface I1 {
	name: string;
	age: number;
}

export module M1 {
	export interface I2 {
		foo: string;
	}
}

export enum E1 {
	A,B,C
}


