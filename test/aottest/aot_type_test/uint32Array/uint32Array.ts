/*
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
 */

declare function print(arg:any):string;
{
    let typedArray : Uint32Array = new Uint32Array([1, 2, 4294967290, 5, 5, 5, 6, 6, 4294967295]);
    print(typedArray[2]);
    typedArray[0] = typedArray[2] + typedArray[4]; //upper bound
    print(typedArray[0]);
    typedArray[1] = typedArray[3] - typedArray[5]; //lower bound
    print(typedArray[1]);
    typedArray[0] = typedArray[2] + typedArray[6]; //over flow
    print(typedArray[0]);
    typedArray[1] = typedArray[3] - typedArray[7]; //under spill
    print(typedArray[1]);
    let s = typedArray[2] + typedArray[6];
    print(s);
    s = typedArray[3] - typedArray[7];
    print(s);

    // and
    print(typedArray[8] & 1);
    print(typedArray[8] & -1);

    // or
    print(typedArray[8] | 1);
    print(typedArray[8] | -1);

    // ashr
    print(typedArray[8] >>> 1);
    print(typedArray[8] >>> -1);

    // shr
    print(typedArray[8] >> 1);
    print(typedArray[8] >> -1);

    // shl
    print(typedArray[8] << 1);
    print(typedArray[8] << -1);

    // xor
    print(typedArray[8] ^ 1);
    print(typedArray[8] ^ -1);
}