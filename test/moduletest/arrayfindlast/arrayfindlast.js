/*
 * Copyright (c) 2023 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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
 * @tc.name:arrayfindlast
 * @tc.desc:test Array.findLast and Array.findLastIndex
 * @tc.type: FUNC
 * @tc.require: issueI7LP2E
 */

var arr = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
var result = -1;
result = arr.findLast((element, index, array) => {
    array.length = 5;
    return (element == 5);
});
print(result);
result = arr.findLast((element) => {
    return element > 2;
});
print(result);

result = arr.findLastIndex((element, index, array) => {
    if (array.length == 5) {
        array.push(100);
    }
    return (element == 100);
});
print(result);
result = arr.findLastIndex((element, index, array) => {
    return (element == 100);
});
print(result);

var arr2 = new Array(1025);
result = arr2.findLast((element, index, array) => {
    return (element == undefined);
});
print(result);

result = arr2.findLastIndex((element, index, array) => {
    return (element == undefined);
});
print(result);

result = arr2.findLastIndex((element, index, array) => {
    array[5] = 100;
    return (element == 100);
});
print(result);