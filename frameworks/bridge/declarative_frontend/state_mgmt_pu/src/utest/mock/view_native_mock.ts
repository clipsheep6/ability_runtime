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

abstract class NativeView {

  static create(newView: View): void {
    console.error("NativeView(Mock).create - unimplemented");
  }

  static createRerender(): void {
    console.error("NativeView(Mock).createRerender - unimplemented");
  }

  constructor( ) {
    console.log(`${this.constructor.name}: new instance`);
  }

  abstract id__(): void;

  protected markNeedUpdate(): void {
    console.log(`${this.id__()}:${this.constructor.name}: markNeedUpdate`);
  }

  findChildById(compilerAssignedUniqueChildId: string): View {
    console.log(`${this.id__()}:${this.constructor.name}: findChildById ${compilerAssignedUniqueChildId}. Will not work!`);
    return undefined;
  }

  syncInstanceId(): void {
    console.warn("NativeView(Mock).syncInstanceId() -  unimplemented!");
  }
  restoreInstanceId(): void {
    console.warn("NativeView(Mock).restoreInstanceId() -  unimplemented!");
  }

  finishUpdateFunc(elmtId: number): void {
    console.warn("NativeView(Mock).restoreInstanceId() -  unimplemented!");
  }
  getDeletedElemtIds(elmtIds : number[]) : void {
    console.warn("NativeView(Mock).finishUpdateFunc() -  unimplemented!");
  }

  deletedElmtIdsHaveBeenPurged(elmtIds : number[]) : void {
    console.warn("NativeView(Mock).deletedElmtIdsHaveBeenPurged() -  unimplemented!");
  }

  isLazyItemRender(elmtId : number) : boolean {
    console.warn("NativeView(Mock).isLazyItemRender() -  unimplemented!");
    return false;
  }
}
