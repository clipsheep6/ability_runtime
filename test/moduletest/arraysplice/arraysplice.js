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

let arr = [1, 2, 3, 4, 5, 6, 7, 8];
arr.splice(0, 1, 9)
print(arr)
arr.splice(10, 1)
print(arr)
arr = [1, 2, 3, 4, 5, 6, 7, 8];
arr.splice(8, 0, 1, 1)
print(arr)
arr.splice(0, 8, 1, 1)
print(arr)
arr = [1, 2, 3, 4, 5, 6, 7, 8];
arr.splice(1, 8)
print(arr)
arr = new Array(100)
arr.splice(0, 20)
print(arr.length)
