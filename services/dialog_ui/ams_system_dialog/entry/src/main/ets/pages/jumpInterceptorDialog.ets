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

interface TipBtn {
  color:string
}

@Entry
@Component
struct JumpInterceptorDialog {
  @State private isDefaultSelector: boolean = true;
  @State private btn: TipBtn = { color: '#FFFFFF' }
  private TAG = 'JumpInterceptorDialog_Page'

  aboutToAppear() {
    console.log(this.TAG, 'dialog page appears');
    this.isDefaultSelector = globalThis.params.isDefaultSelector
    if (this.isDefaultSelector == false) {
      this.btn.color = '#F2F2F2';
    }
  }

  onCloseApp() {
    globalThis.jumpInterceptorExtensionContext.terminateSelf();
  }

  onJumpApp() {
    globalThis.abilityWant.bundleName = globalThis.params.bundleName;
    globalThis.abilityWant.abilityName = globalThis.params.abilityName;
    globalThis.abilityWant.moduleName = globalThis.params.moduleName;
    globalThis.jumpInterceptorExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (data, error) => {
      if (error) {
        console.error(this.TAG + 'startAbility finish, error: ' + JSON.stringify(error));
        return;
      }
      console.log(this.TAG + 'startAbility finish, data: ' + JSON.stringify(data));
      globalThis.jumpInterceptorExtensionContext.terminateSelf();
    });
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Text($r('app.string.message_title_jump_interceptor', globalThis.callerAppName, globalThis.targetAppName))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .height('29%')
          .textOverflow({overflow: TextOverflow.Ellipsis})
          .maxLines(2)
        .textAlign(TextAlign.Center)
      }
      Flex({ justifyContent: FlexAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.message_cancel_jump'))
            .fontSize('21fp')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Center)
        }
        .width(175)
        .height(50)
        .borderRadius(28)
        .backgroundColor(this.btn.color)
        .onClick(() => {
          this.onCloseApp();
        })
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.message_confirm_jump'))
            .fontSize('21fp')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Center)
        }
        .width(175)
        .height(50)
        .borderRadius(28)
        .backgroundColor(this.btn.color)
        .onClick(() => {
          this.onJumpApp();
        })
      }.margin({ top: 10})
    }
    .borderRadius(20)
    .borderWidth(1)
    .borderColor('#e9e9e9')
    .backgroundColor('#FFFFFF')
  }
}