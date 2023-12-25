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

@Entry
@Component
struct SwitchUserDialog {
  private TAG = "SwitchUserDialog_Page"

  aboutToAppear(): void {
    console.info(this.TAG, "dialog page appears");
  }

  build() {
    Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
      Flex({ justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        Text($r('app.string.message_title_switchUser'))
          .fontSize(16)
          .fontColor("#000000")
          .fontFamily('HarmonyHeiTi')
          .fontWeight(FontWeight.Medium)
          .textOverflow({overflow: TextOverflow.Ellipsis})
          .textAlign(TextAlign.Center)
      }
      .height(64)
    }
    .borderRadius(16)
    .borderWidth(1)
    .borderColor("#e9e9e9")
    .backgroundColor("#FFFFFF")
  }
}