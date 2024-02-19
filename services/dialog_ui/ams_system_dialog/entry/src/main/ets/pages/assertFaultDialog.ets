/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import abilityManager from '@ohos.app.ability.abilityManager';
import { BusinessError } from '@ohos.base';

@Entry
@Component
struct AssertFaultDialog {
  @LocalStorageLink('sessionId') sessionId: string = '';
  private TAG = "AssertFaultDialog_Page";

  aboutToAppear() {
    console.info(this.TAG, "dialog page appears");
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Text($r('app.string.message_title_assertFault'))
          .fontSize(16)
          .fontColor("#000000")
          .fontFamily('HarmonyHeiTi')
          .fontWeight(FontWeight.Medium)
          .textOverflow({overflow: TextOverflow.Ellipsis})
          .textAlign(TextAlign.Center)
        // Text('信息标题')
        // Text('信息')
      }
      .height(64)

      Flex({ direction: FlexDirection.Row }) {
        Button($r('app.string.button_termination'))
          .margin(10)
          .onClick(() => {
            let status = abilityManager.UserStatus.ASSERT_TERMINATE;
            abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
              console.log(this.TAG, 'NotifyUserActionResult termination status success.');
            }).catch((err: BusinessError) => {
              console.error(this.TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
            })
          })
        Button($r('app.string.button_continue'))
          .margin(10)
          .onClick(() => {
            let status = abilityManager.UserStatus.ASSERT_CONTINUE;
            abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
              console.log(this.TAG, 'NotifyUserActionResult termination status success.');
            }).catch((err: BusinessError) => {
              console.error(this.TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
            })
          })
        Button($r('app.string.button_retry'))
          .margin(10)
          .onClick(() => {
            let status = abilityManager.UserStatus.ASSERT_RETRY;
            abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
              console.log(this.TAG, 'NotifyUserActionResult termination status success.');
            }).catch((err: BusinessError) => {
              console.error(this.TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
            })
          })
      }
    }
    .borderRadius(16)
    .borderWidth(1)
    .borderColor("#e9e9e9")
    .backgroundColor("#FFFFFF")
  }
}