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

/*
 * @tc.name:typedarraysort
 * @tc.desc:test TypedArray.sort
 * @tc.type: FUNC
 * @tc.issue: I8NI6U
 */
[
Int8Array
].forEach((ctr)=>{
    let arr=new ctr();
    arr[0]=1;
    arr[1]=2;
    arr[2]=3;
    try {
        arr.sort(null);
    } catch (error) {
        print(ctr.name,error.name)   ;
    }
})