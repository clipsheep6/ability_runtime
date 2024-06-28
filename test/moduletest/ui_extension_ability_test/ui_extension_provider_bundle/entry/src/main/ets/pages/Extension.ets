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

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';

let storage = LocalStorage.getShared();
const TAG: string = '[testTag] ExtensionPage'

@Entry(storage)
@Component
struct Extension {
  @State message: string = 'UIExtension Provider';
  private session: UIExtensionContentSession | undefined = storage.get<UIExtensionContentSession>('session');

  onPageShow() {
    console.info(TAG, 'show');
    // if u wanna use setReceiveDataCallback, you should be a system app.
    // this.session?.setReceiveDataCallback((data) => {
    //   this.message = JSON.stringify(data);
    // })
  }

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(20)
          .fontWeight(FontWeight.Bold)
          .textAlign(TextAlign.Center)

        Button("send data")
          .width('80%')
          .type(ButtonType.Capsule)
          .margin({
            top: 20
          })
          .onClick(() => {
            this.session?.sendData({ "data": 543321 });
          })

        Button("terminate self")
          .width('80%')
          .type(ButtonType.Capsule)
          .margin({
            top: 20
          })
          .onClick(() => {
            this.session?.terminateSelf();
            storage.clear();
          })

        Button("terminate self with result")
          .width('80%')
          .type(ButtonType.Capsule)
          .margin({
            top: 20
          })
          .onClick(() => {
            this.session?.terminateSelfWithResult({
              resultCode: 0,
              want: {
                bundleName: "com.ohos.uiextensionprovider",
                parameters: { "result": 123456 }
              }
            })
          })
      }
    }
    .height('100%')
  }
}