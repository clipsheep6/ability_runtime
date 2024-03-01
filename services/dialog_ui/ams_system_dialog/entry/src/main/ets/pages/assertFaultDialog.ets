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
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';

let session: UIExtensionContentSession = undefined;
let sessionId: string = '';
const TAG = "AssertFaultDialog_Page";

@CustomDialog
struct AssertFaultCustomDialog {
  controller?: CustomDialogController
  continueClick: () => void = () => {
    console.info('Continue button click default');
  }
  retryClick: () => void = () => {
    console.info('Retry button click default');
  }
  terminationClick: () => void = () => {
    console.info('Termination button click default');
  }
  private textDetail: string = 'File:\n../../third_party/musl/libc-test/src/functionalext/unittest/unit_test_assert_fail.c\nFunction: main\nLine: 23\n\nExpression:\n0';
  private detail: string = '(Press Retry to debug the application)';

  build() {
    Column() {

      Column() {
        Flex({ justifyContent: FlexAlign.Start, alignItems: ItemAlign.Start }) {
          Text($r('app.string.title_assertFault'))
            .fontSize(32)
            .fontColor($r('app.color.text_color'))
            .fontFamily('HarmonyHeiTi')
            .fontWeight(FontWeight.Medium)
            .textOverflow({ overflow: TextOverflow.Ellipsis })
            .margin({ top: '25%', left: '5%' })
        }
        .height('18%')

        Flex({ direction: FlexDirection.Column }) {
          Text(this.textDetail)
            .fontSize(16)
            .fontFamily('HarmonyHeiTi')
            .fontWeight(FontWeight.Medium)
            .fontColor($r('app.color.text_color'))
            .focusable(false)
            .backgroundColor("#FFFFFF")
            .height('67%')
            .width('95%')
            .margin({ left: '6%' })

          Text(this.detail)
            .fontSize(16)
            .fontFamily('HarmonyHeiTi')
            .fontWeight(FontWeight.Medium)
            .fontColor($r('app.color.text_color'))
            .focusable(false)
            .backgroundColor("#FFFFFF")
            .height('7%')
            .width('95%')
            .margin({ top: '2%', left: '6%' })

          Flex({ direction: FlexDirection.RowReverse }) {
            Button($r('app.string.button_continue'))
              .margin(20)
              .fontFamily('HarmonyHeiTi')
              .height(40)
              .width(80)
              .type(ButtonType.Capsule)
              .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
              .backgroundColor($r('app.color.button_color'))
              .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
              .fontSize($r('sys.float.ohos_id_text_size_button1'))
              .onClick(() => {
                console.info('AssertFaultCustomDialog', 'Continue button click');
                this.continueClick();
              })

            Button($r('app.string.button_retry'))
              .margin(20)
              .fontFamily('HarmonyHeiTi')
              .height(40)
              .width(80)
              .type(ButtonType.Capsule)
              .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
              .backgroundColor($r('app.color.button_color'))
              .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
              .fontSize($r('sys.float.ohos_id_text_size_button1'))
              .onClick(() => {
                console.info('AssertFaultCustomDialog', 'Retry button click');
                this.retryClick();
              })

            Button($r('app.string.button_termination'))
              .margin(20)
              .fontFamily('HarmonyHeiTi')
              .height(40)
              .width(80)
              .type(ButtonType.Capsule)
              .borderRadius($r('sys.float.ohos_id_corner_radius_button'))
              .backgroundColor($r('app.color.button_color'))
              .fontColor($r('sys.color.ohos_id_color_foreground_contrary'))
              .fontSize($r('sys.float.ohos_id_text_size_button1'))
              .onClick(() => {
                console.info('AssertFaultCustomDialog', 'Termination button click');
                this.terminationClick();
              })
          }
          .height('20%')
        }
        .height('82%')
      }
      .height('550px')
      .width('720px')
      .borderRadius(16)
      .borderWidth(1)
      .borderColor("#e9e9e9")
      .backgroundColor("#FFFFFF")
    }
  }
}

let storage = LocalStorage.GetShared();

@Entry(storage)
@Component
struct AssertFaultDialog {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: AssertFaultCustomDialog({
      continueClick: this.onContinueCall,
      retryClick: this.onRetryCall,
      terminationClick: this.onTerminationCall,
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    gridCount: 10,
    customStyle: false
  });

  aboutToAppear() {
    console.info(TAG, "dialog page appears");
    session = storage.get<UIExtensionContentSession>('session');
    sessionId = storage.get<string>('sessionId');
    console.log(TAG, `session is: ${session}`);
    console.log(TAG, `sessionId is: ${sessionId}`);
  }

  existApp() {
    console.info(TAG, 'Exist app called');
    session.terminateSelf().then(() => {
      console.log(TAG, 'terminateSelf success.');
    }).catch((err: BusinessError) => {
      console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(err)}`);
    })
  }

  onContinueCall() {
    console.info(TAG, 'On continue called');
    let status = abilityManager.UserStatus.ASSERT_CONTINUE;
    try {
      abilityManager.notifyDebugAssertResult(sessionId, status).then(() => {
        console.log(TAG, 'notifyDebugAssertResult continue status success.');
        AppStorage.SetOrCreate('isUserAction', true);
        session.terminateSelf().then(() => {
          console.log(TAG, 'terminateSelf success.');
        }).catch((err: BusinessError) => {
          console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(err)}`);
        })
      }).catch((err: BusinessError) => {
        console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(err)}`);
      })
    } catch (error) {
      console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(error)}`);
    }
  }
  onRetryCall() {
    console.info(TAG, 'On retry called');
    let status = abilityManager.UserStatus.ASSERT_RETRY;
    try {
      abilityManager.notifyDebugAssertResult(sessionId, status).then(() => {
        console.log(TAG, 'notifyDebugAssertResult retry status success.');
        AppStorage.SetOrCreate('isUserAction', true);
        session.terminateSelf().then(() => {
          console.log(TAG, 'terminateSelf success.');
        }).catch((err: BusinessError) => {
          console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(err)}`);
        })
      }).catch((err: BusinessError) => {
        console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(err)}`);
      })
    } catch (error) {
      console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(error)}`);
    }
  }
  onTerminationCall() {
    console.info(TAG, 'On termination called');
    let status = abilityManager.UserStatus.ASSERT_TERMINATE;
    try {
      abilityManager.notifyDebugAssertResult(sessionId, status).then(() => {
        console.log(TAG, 'notifyDebugAssertResult termination status success.');
        AppStorage.SetOrCreate('isUserAction', true);
        session.terminateSelf().then(() => {
          console.log(TAG, 'terminateSelf success.');
        }).catch((err: BusinessError) => {
          console.error(TAG, `terminateSelf failed, error: ${JSON.stringify(err)}`);
        })
      }).catch((err: BusinessError) => {
        console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(err)}`);
      })
    } catch (error) {
      console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(error)}`);
    }
  }

  build() {
    Column(this.dialogController.open()) {}
  }
}