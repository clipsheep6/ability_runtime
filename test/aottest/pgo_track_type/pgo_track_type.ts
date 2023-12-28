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

declare function print(arg:any, arg1?: any):string;

class Vec3 {
  public static set(out: Vec3, x: number, y: number, z: number): Vec3 {
    out.x = x;
    out.y = y;
    out.z = z;
    return out;
  }
  public x: number = 0;
  public y: number = 0;
  public z: number = 0;
  constructor(x: number, y: number, z: number) {
    this.x = x;
    this.y = y;
    this.z = z;
  }
}
class A {
  public a0: Vec3;
  public a1: Vec3;
  public a2: Vec3;
  public a3: Vec3;
  public a4: Vec3;
  public a5: Vec3;

  constructor() {
    this.a0 = new Vec3(0, 0, 0);
    this.a1 = new Vec3(0, 0, 0);
    this.a2 = new Vec3(0, 0, 0);
    this.a3 = new Vec3(0, 0, 0);
    this.a4 = new Vec3(0, 0, 0);
    this.a5 = new Vec3(0, 0, 0);
  }
}

let a:A = new A();
for (let i = 0; i < 100; i++) {
  a = new A();
}

Vec3.set(a.a0, 0.5, 0.5, 0.5);
print(a.a0.x)
print(a.a0.y)
print(a.a0.z)
