/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 * @tc.name:setobjectwithproto
 * @tc.desc:test set object with proto
 * @tc.type: FUNC
 * @tc.require: issueI5NO8G
 */
var object = {
    __proto__: null
};

print(Object.getPrototypeOf(object));

class C32 extends String {

}
let obj1 = new C32();
print(obj1.__proto__ == C32.prototype);
print(C32.__proto__ == String);
C32.__proto__ = Array;
let obj2 = new C32();
print(obj2.__proto__ == C32.prototype);
print(C32.__proto__ == Array);

Float64Array.__proto__ = Float32Array
new Float64Array();
print("test new Float64Array Suc")
