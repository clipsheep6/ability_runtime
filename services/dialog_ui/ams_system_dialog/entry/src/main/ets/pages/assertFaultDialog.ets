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
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center , alignItems: ItemAlign.Center }) {
      Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Center , alignItems: ItemAlign.Center }) {
        Column() {
          Flex({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start }) {
            Text($r('app.string.title_assertFault'))
              .fontSize(32)
              .fontColor($r('app.color.text_color'))
              .fontFamily('HarmonyHeiTi')
              .fontWeight(FontWeight.Medium)
              .textOverflow({overflow: TextOverflow.Ellipsis})
          }
          .height('20%')

          Flex({ direction: FlexDirection.Column }) {
            Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start }) {
              Flex({ direction: FlexDirection.Row, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start }) {
                Image($r('app.media.icon'))
                .autoResize(true)
                .height('180px')
                .width('180px')
              }
                .margin(10)
                .width('30%')

              Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start }) {
                Text($r('app.string.message_title_assertFault'))
                  .fontSize(24)
                  .fontFamily('HarmonyHeiTi')
                  .fontWeight(FontWeight.Medium)
                  .fontColor($r('app.color.text_color'))
                  .textOverflow({overflow: TextOverflow.Ellipsis})
                  .height('20%')

                TextArea({text: $r('app.string.message_detail_assertFault')})
                  .fontSize(16)
                  .fontFamily('HarmonyHeiTi')
                  .fontWeight(FontWeight.Medium)
                  .fontColor($r('app.color.text_color'))
                  .focusable(false)
                  .backgroundColor("#FFFFFF")
                  .height('80%')
            }
              .width('70%')
            }
            .height('80%')

            Flex({ direction: FlexDirection.RowReverse }) {
              Button($r('app.string.button_continue'))
                .margin(10)
                .fontFamily('HarmonyHeiTi')
                .height(40)
                .width(80)
                .type(ButtonType.Capsule)
                .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
                .backgroundColor($r('app.color.button_color'))
                .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
                .fontSize($r('sys.float.ohos_id_text_size_button1'))
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
                .fontFamily('HarmonyHeiTi')
                .height(40)
                .width(80)
                .type(ButtonType.Capsule)
                .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
                .backgroundColor($r('app.color.button_color'))
                .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
                .fontSize($r('sys.float.ohos_id_text_size_button1'))
                .onClick(() => {
                  let status = abilityManager.UserStatus.ASSERT_RETRY;
                  abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
                    console.log(this.TAG, 'NotifyUserActionResult termination status success.');
                  }).catch((err: BusinessError) => {
                    console.error(this.TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
                  })
                })

              Button($r('app.string.button_termination'))
                .margin(10)
                .fontFamily('HarmonyHeiTi')
                .height(40)
                .width(80)
                .type(ButtonType.Capsule)
                .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
                .backgroundColor($r('app.color.button_color'))
                .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
                .fontSize($r('sys.float.ohos_id_text_size_button1'))
                .onClick(() => {
                  let status = abilityManager.UserStatus.ASSERT_TERMINATE;
                  abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
                    console.log(this.TAG, 'NotifyUserActionResult termination status success.');
                  }).catch((err: BusinessError) => {
                    console.error(this.TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
                  })
                })
            }
            .height('20%')
          }
          .height('80%')
        }
        .height('600px')
        .width('720px')
        .borderRadius(16)
        .borderWidth(1)
        .borderColor("#e9e9e9")
        .backgroundColor("#FFFFFF")
      }
      .width('100%')
    }
    .height('100%')
  }
}