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
 * @tc.name:sharedcheck
 * @tc.desc:test sharedcheck
 * @tc.type: FUNC
 * @tc.require: issueI8QUU0
 */

// @ts-nocheck
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
 * @tc.name:sharedcheck
 * @tc.desc:test sharedcheck
 * @tc.type: FUNC
 * @tc.require: issueI8QUU0
 */

// @ts-nocheck
declare function print(str: any): string;

class SimpleStringSendable {
  propString: string = "I'm simple sendable's propString";
  constructor() {
    "use sendable"
    print(this.propString)
  }
}

class SimpleNumberSendable {
  propNumber: number = 2023;
  constructor() {
    "use sendable"
    print("I'm constructor for SimpleNumberSendable")
  }
}

class SuperClass {
  propString: string = "I'm propString"
  propNumber: number = 5
  propBool: boolean = false
  static staticPropString: string = "I'm staticPropString";
  publicPropString: string = "I'm privatePropString";

  get accessorPrivatePropString() {
    return this.publicPropString;
  }

  set accessorPrivatePropString(prop: string) {
    this.publicPropString = prop;
  }
  constructor() {
    "use sendable"
    print(this.propString)
  }
}

class SubClass extends SuperClass {
  static staticSubClassPropString: string = "I'm staticSubClassPropString";
  static staticFunc() {
    print("Hi static func");
  }
  static staticFunc1() {
    print("Hi static func1");
  }

  func() {
    print("Hi func");
    this.publicFunc();
  }

  publicFunc() {
    print("Hi public func");
  }

  subClassPropString: string = "I'm subClassPropString"
  subClassPropSendable: SimpleStringSendable
  constructor() {
    "use sendable"
    super()
  }
}

function testDelete(testObj: SubClass) {
  print("Start testDelete");
  try {
    delete testObj.propNumber;
    print("Success delete propNumber")
  } catch (error) {
    print("Fail to delete propNumber. err: " + error)
  }
}

function testExtend(testObj: SubClass) {
  print("Start testExtend");
  try {
    Object.defineProperty(testObj, "tmpProp", {value: 321, writable: true });
    print("Success to extend prop with defineProperty")
  } catch (error) {
    print("Fail to extend prop with defineProperty. err: " + error);
  }

  try {
    Object.defineProperty(testObj, "prop1", { writable: true });
    print("Success extend prop1 with defineProperty")
  } catch (error) {
    print("Fail to extend prop1 with defineProperty. err: " + error)
  }

  try {
    Object.defineProperties(testObj, { prop2: { writable: true } });
    print("Success extend prop2 with defineProperties")
  } catch (error) {
    print("Fail to extend prop2 with defineProperties. err: " + error)
  }

  try {
    testObj.prop3 = {};
    print("Success extend prop3 with stobjbyname")
  } catch (error) {
    print("Fail to extend prop3 with stobjbyname. err: " + error)
  }
}

function testUpdateInstanceFunction(testObj: SubClass) {
  print("Start testUpdateInstanceFunction");
  try {
    testObj.func = function () { }
    print("Success replace instance's func");
  } catch (error) {
    print("Fail to replace instance's func. err: " + error);
  }
}

function testUpdatePrototype(testObj: SubClass) {
  print("Start testUpdatePrototype");
  try {
    SuperClass.prototype.staticSubClassPropString = 1;
    print("Success update prototype")
  } catch (error) {
    print("Fail to update prototype. err: " + error)
  }

  try {
    SubClass.prototype.tmpProp = 123;
    print("Success to extend prop to constructor's prototype.");
  } catch (error) {
    print("Fail to extend prop to constructor's prototype. err: " + error);
  }

  let superClass = new SuperClass()
  try {
    testObj.__proto__.constructor = superClass.__proto__.constructor;
    print("Success to change constructor of instance's prototype.");
  } catch (error) {
    print("Fail to change constructor of instance's prototype. err: " + error);
  }

  try {
    testObj.__proto__ = superClass.__proto__;
    print("Success to replace instance's prototype.");
  } catch (error) {
    print("Fail to replace instance's prototype. err: " + error);
  }

  try {
    Object.defineProperty(testObj.__proto__, "abc", { value: 123 });
    print("Success to extend instance's prototype.");
  } catch (error) {
    print("Fail to extend instance's prototype. err: " + error);
  }
}

function testUpdateInstancePropsToNull(testObj: SubClass) {
  print("Start testUpdateInstancePropsToNull");
  try {
    testObj.propString = null
    print("Success update propString to null with stobjbyname")
  } catch (error) {
    print("Fail to update propString to null with stobjbyname. err: " + error)
  }

  try {
    testObj.subClassPropSendable = null
    print("Success update subClassPropSendable to null with stobjbyname")
  } catch (error) {
    print("Fail to update subClassPropSendable to null with stobjbyname. err: " + error)
  }

  try {
    testObj.propNumber = null
    print("Success update propNumber to null with stobjbyname")
  } catch (error) {
    print("Fail to update propNumber to null with stobjbyname. err: " + error)
  }

  try {
    testObj.propBool = null
    print("Success update propNumber to null with stobjbyname")
  } catch (error) {
    print("Fail to update propNumber to null with stobjbyname. err: " + error)
  }
}

function testUpdateInstancePropsToUndefined(testObj: SubClass) {
  print("Start testUpdateInstancePropsToUndefined");
  try {
    testObj.propString = undefined
    print("Success update propString to undefined with stobjbyname")
  } catch (error) {
    print("Fail to update propString to undefined with stobjbyname. err: " + error)
  }

  try {
    testObj.subClassPropSendable = undefined
    print("Success update subClassPropSendable to undefined with stobjbyname")
  } catch (error) {
    print("Fail to update subClassPropSendable to undefined with stobjbyname. err: " + error)
  }

  try {
    testObj.propNumber = undefined
    print("Success update propNumber to undefined with stobjbyname")
  } catch (error) {
    print("Fail to update propNumber to undefined with stobjbyname. err: " + error)
  }

  try {
    testObj.propBool = undefined
    print("Success update propNumber to undefined with stobjbyname")
  } catch (error) {
    print("Fail to update propNumber to undefined with stobjbyname. err: " + error)
  }
}

function testUpdateInstanceProps(testObj: SubClass) {
  testUpdateInstancePropsToNull(testObj);
  testUpdateInstancePropsToUndefined(testObj);
  try {
    Object.defineProperties(testObj, { subClassPropString: { value: "hello", writable: true } });
    print("Success update subClassPropString with defineProperties")
  } catch (error) {
    print("Fail to update subClassPropString with defineProperties. err: " + error)
  }

  try {
    Object.defineProperty(testObj, "propNumber", { value: 100, configurable: false });
    print("Success update propNumber with defineProperty")
  } catch (error) {
    print("Fail to update propNumber with defineProperty. err: " + error)
  }

  try {
    testObj.subClassPropString = 33;
    print("Success update subClassPropString with stobjbyname")
  } catch (error) {
    print("Fail to update subClassPropString with stobjbyname. err: " + error)
  }
}

function testUpdateInstanceAccessor(testObj: SubClass) {
  print("Start testUpdateInstanceAccessor");
  try {
    Object.defineProperty(testObj, "accessorPrivatePropString",
      {
        get accessorPrivatePropString() { print("Replaced get accessor") },
        set accessorPrivatePropString(prop: string) { print("Replaced set accessor") }
      })
    print("Success replace accessor");
  } catch (error) {
    print("Fail to replace accessor. err: " + error);
  }

  try {
    testObj.accessorPrivatePropString = "123"
    print("Success set prop through accessor with matched type");
  } catch (error) {
    print("Fail to set prop through accessor with matched type. err: " + error);
  }

  try {
    testObj.accessorPrivatePropString = 123
    print("Success set prop through accessor with mismatched type");
  } catch (error) {
    print("Fail to set prop through accessor with mismatched type. err: " + error);
  }
}

function testUpdateConstructor() {
  print("Start testUpdateConstructor");
  try {
    SubClass.staticFunc = function () { };
    print("Success to modify constructor's method.");
  } catch (error) {
    print("Fail to modify constructor's method. err: " + error);
  }

  try {
    SubClass.staticSubClassPropString = "modify static";
    print("Success to modify property to constructor's property.");
  } catch (error) {
    print("Fail to modify property to constructor's property. err: " + error);
  }
}

function testObjectProtoFunc(testObj: SubClass) {
  print("Start testObjectProtoFunc");
  testObjectAssign(testObj);
  testObjectCreate(testObj);
  testObjectSetPrototypeOf(testObj);
  testObjectAttributesAndExtensible(testObj);
}

function testObjectCreate(testObj: SubClass)
{
  print("Start testObjectCreate");
  try {
    let sendableSimple: SimpleStringSendable = Object.create(SimpleStringSendable);
    print("Success to call Object.create");
  } catch (error) {
    print("Fail to call Object.create. err: " + error);
  }
}

function testObjectSetPrototypeOf(testObj: SubClass)
{
  print("Start testObjectSetPrototypeOf");
  try {
    Object.setPrototypeOf(testObj, new Object)
    print("Success to call Object.setPrototypeOf")
  } catch (error) {
    print("Fail to call Object.setPrototypeOf. err: " + error)
  }
}

function testObjectAttributesAndExtensible(testObj: SubClass)
{
  print("Start testObjectAttributesAndExtensible");
  try {
    Object.defineProperty(testObj, "propNumber", { configurable: true });
    print("Success to update propNumber to configurable with defineProperty")
  } catch (error) {
    print("Fail to update propNumber to configurable with defineProperty. err: " + error)
  }
  print("isFrozen: " + Object.isFrozen(testObj))
  try {
    Object.freeze(testObj);
    print("Success to call Object.freeze")
  } catch (error) {
    print("Fail to call Object.freeze. err: " + error)
  }
  print("isSealed: " + Object.isSealed(testObj))
  try {
    Object.seal(testObj);
    print("Success to call Object.seal in sealed state")
  } catch (error) {
    print("Fail to call Object.seal in sealed state. err: " + error)
  }
  print("isExtensible: " + Object.isExtensible(testObj))
  try {
    Object.preventExtensions(testObj);
    print("Success to call Object.preventExtensions in preventExtensions state.")
  } catch (error) {
    print("Fail to to call Object.preventExtensions in preventExtensions state. err: " + error)
  }
}

function testObjectAssign(testObj: SubClass)
{
  print("Start testObjectAssign");
  try {
    Object.assign(testObj, new Object({ a: 1, b: "abc" }));
    print("Success to call Object.assign to extend target");
  } catch (error) {
    print("Fail to call Object.assign to extend target. err: " + error);
  }

  try {
    Object.assign(testObj, new Object({ propString: undefined }));
    print("Success to call Object.assign to update propString with mismatched type");
  } catch (error) {
    print("Fail to call Object.assign to update propString with mismatched type. err: " + error);
  }

  try {
    Object.assign(testObj, new Object({ propString: "abc" }));
    print("Success to call Object.assign to update propString");
  } catch (error) {
    print("Fail to call Object.assign to update propString. err: " + error);
  }
}

function testKeywords(testObj: SubClass)
{
  print("Start testKeywords");
  print("typeof sendable object: " + (typeof testObj))
  print("typeof sendable function: " + (typeof testObj.func))
  print("sendable instanceof Object: " + (testObj instanceof Object))
  print("sendable instanceof SubClass: " + (testObj instanceof SubClass))
  print("sendable instanceof SuperClass: " + (testObj instanceof SuperClass))
}

function testUpdate(testObj: SubClass) {
  testUpdateInstanceProps(testObj);
  testUpdateInstanceAccessor(testObj);
  testUpdateInstanceFunction(testObj);
  testUpdatePrototype(testObj);
  testUpdateConstructor();
}

function testUpdateWithType(testObj: SubClass) {
  print("Start testUpdateWithType");
  try {
    testObj.propString = 1;
    print("Success update string to int with stobjbynamme.")
  } catch (error) {
    print("Fail to update string to int with stobjbynamme. err: " + error);
  }

  try {
    Object.defineProperty(testObj, "subClassPropSendable", { value: 123, writable: true });
    print("Success update subClassPropSendable to number with defineProperty.")
  } catch (error) {
    print("Fail to update subClassPropSendable to number with defineProperty. err: " + error);
  }

  try {
    Object.defineProperty(testObj, "subClassPropSendable", { value: new SimpleNumberSendable(), writable: true });
    print("Success update subClassPropSendable to numberSendable with defineProperty.")
  } catch (error) {
    print("Fail to update subClassPropSendable to numberSendable with defineProperty. err: " + error);
  }
}

function testNormInherit() {
  print("Start testNormInherit");
  try {
    class NormalClass extends SimpleStringSendable {
      constructor() {
        super()
      }
    }
    print("Success to define normal class inherit from sendable class")
  } catch (error) {
    print("Fail to define normal class inherit from sendable class, err: " + error)
  }
}

function testICCheckingForUpdateInstanceProps(testObj: SubClass) {
  let loopIndex: number = 0;
  try {
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
      testObj.subClassPropString = loopIndex < 1000 ? "hello" : 1;
    }
    print("[IC] Success update subClassPropString with mismatch type")
  } catch (error) {
    print("[IC] Fail to update subClassPropString with mismatch type. err: " + error + ", loopIndex: " + loopIndex)
  }

  try {
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
      testObj.propNumber = loopIndex < 1000 ? 100 : "Hi";
    }
    print("[IC] Success update propNumber with mismatch type")
  } catch (error) {
    print("[IC] Fail to update propNumber with mismatch type. err: " + error + ", loopIndex: " + loopIndex);
  }

  try {
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
      testObj.subClassPropString = loopIndex < 1000 ? 33 : "Hi";
    }
    print("[IC] Success update subClassPropString with mismatch type")
  } catch (error) {
    print("[IC] Fail to update subClassPropString with mismatch type. err: " + error + ", loopIndex: " + loopIndex);
  }

  try {
    let simpleSendable = new SimpleStringSendable();
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
      testObj.subClassPropSendable = loopIndex < 1000 ? simpleSendable : 1;
    }
    print("[IC] Success to update subClassPropSendable with mismatch type.");
  } catch (error) {
    print("[IC] Fail to update subClassPropSendable with mismatch type. err: " + error + ", loopIndex: " + loopIndex);
  }

  try {
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
      SubClass.staticSubClassPropString = loopIndex < 1000 ? "modify static" : 1;
    }
    print("[IC] Success to modify constructor's property with mismatch type.");
  } catch (error) {
    print("[IC] Fail to modify constructor's property with mismatch type. err: " + error + ", loopIndex: " + loopIndex);
  }
}

function testHotFunction(testObj: SubClass, loopIndex: number) {
  testObj.accessorPrivatePropString = loopIndex < 1000 ? "123" : 1;
}

function testICCheckingUpdateInstanceAccessor(testObj: SubClass) {
  let loopIndex: number = 0;
  try {
    for (loopIndex = 0; loopIndex < 2000; loopIndex++) {
        testHotFunction(testObj, loopIndex);
    }
    print("[IC] Success set prop through accessor with matched type");
  } catch (error) {
    print("[IC] Fail to set prop through accessor with matched type. err: " + error);
  }
}

function testICChecking(testObj: SubClass)
{
  print("Start testICChecking");
  testICCheckingForUpdateInstanceProps(testObj);
  testICCheckingUpdateInstanceAccessor(testObj);
}

let b = new SubClass()
b.subClassPropSendable = new SimpleStringSendable()
testUpdate(b)
testDelete(b)
testExtend(b)
testObjectProtoFunc(b)
testUpdateWithType(b)
testKeywords(b)
testNormInherit()
testICChecking(b);