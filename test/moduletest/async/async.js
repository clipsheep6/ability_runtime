/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

async function foo() {
    var a = await 1
    print(a)
}

var s = foo()
var arrow = () => {}
var async_arrow = async () => {}

print(foo.name)
print(foo.call != undefined)
print(foo.__proto__.constructor.name);
print(foo.__proto__.__proto__.constructor.name);
print(foo.__proto__.__proto__.__proto__.constructor.name);
print(foo.length)
print(arrow.name)
print(async_arrow.name)

s.then(msg=>{
    print(msg)
})

// should return Promise
async function testAsync() {
    throw new Error("hello world!!!");
}
var a = testAsync();
print(a instanceof Promise);
a.then(
    function (result) {
        print("testAsync success!!!");
    },
    function (result) {
        print("testAsync failed: " + result);
    }
)
print("main over");
