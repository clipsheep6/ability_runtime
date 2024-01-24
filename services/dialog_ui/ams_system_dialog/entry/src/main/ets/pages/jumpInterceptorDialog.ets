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
import { BusinessError } from '@ohos.base';
import { GlobalThis } from '../utils/GlobalThis';
import want from '@ohos.app.ability.Want';

interface TipBtn {
  color:string
}

@Entry
@Component
struct JumpInterceptorDialog {
  @State private deviceType: string = "phone";
  @State private btn: TipBtn = { color: "#FFFFFF" }
  private TAG = "JumpInterceptorDialog_Page"

  aboutToAppear() {
    console.log(this.TAG, "dialog page appears");
    this.deviceType = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType as string
    if (this.deviceType == "pc") {
      this.btn.color = "#F2F2F2";
    }
  }

  onCloseApp() {
    GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext')?.terminateSelf();
  }

  onJumpApp() {
    let abilityWant = GlobalThis.getInstance().getObject('abilityWant') as Record<string, Object>
    if (abilityWant != undefined) {
      abilityWant.bundleName =  (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.bundleName as string;
      abilityWant.abilityName = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.abilityName as string;
      abilityWant.moduleName = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.moduleName as string;
      GlobalThis.getInstance().setObject('abilityWant', abilityWant);
    }

    GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext')?.startAbilityAsCaller(GlobalThis.getInstance().getObject('abilityWant') as want, (error : BusinessError) => {
      if (error) {
        console.error(this.TAG + "startAbility finish, error: " + JSON.stringify(error));
        return;
      }
      GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext')?.terminateSelf();
    });
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Text($r('app.string.message_title_jump_interceptor', GlobalThis.getInstance().getProperty('callerAppName'), GlobalThis.getInstance().getProperty('targetAppName')))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .height("29%")
          .textOverflow({overflow: TextOverflow.Ellipsis})
          .maxLines(2)
          .textAlign(TextAlign.Center)
      }
      Flex({ justifyContent: FlexAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r('app.string.message_cancel_jump'))
            .fontSize("21fp")
            .fontColor("#0A59F7")
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
            .fontSize("21fp")
            .fontColor("#0A59F7")
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
    .borderColor("#e9e9e9")
    .backgroundColor("#FFFFFF")
  }
}