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

interface SelectorPc {
  width: string,
  height: string
}

interface HapInfo {
  label: string,
  icon: boolean,
  bundle: string,
  ability: string,
  module: string
}

@Entry
@Component
struct SelectorPcDialog {
  @State private pcSelectorlist: SelectorPc = { width: "100%", height: "240vp"};
  @State private hapList: HapInfo[] = [];
  private TAG = "SelectorDialog_Pc_Page";

  aboutToAppear(): void {
    console.log(this.TAG, "dialog page appears");
    this.hapList = globalThis.params.hapList;
    this.getHapListStyle();
  }

  onSelectApp(item) {
    globalThis.abilityWant.bundleName = item.split("#")[0];
    globalThis.abilityWant.abilityName = item.split("#")[1];
    globalThis.abilityWant.moduleName = item.split("#")[4];
    globalThis.selectExtensionContext.startAbilityAsCaller(globalThis.abilityWant, (data, error) => {
      if (error) {
        console.error(this.TAG + " startAbility finish, error: " + JSON.stringify(error));
        return;
      }
      console.log(this.TAG + " startAbility finish, data: " + JSON.stringify(data));
    });
    globalThis.selectExtensionContext.terminateSelf();
  }

  getHapListStyle() {
    let heightTotalVp = 1;
    let heightVal = 120;
    if (this.hapList.length == 2) {
      heightTotalVp = this.hapList.length * heightVal;
    } else if (this.hapList.length == 3) {
      heightTotalVp = this.hapList.length * heightVal;
    } else if (this.hapList.length == 4) {
      heightTotalVp = this.hapList.length * heightVal;
    } else if (this.hapList.length > 4) {
      heightTotalVp = 4 * heightVal + 20;
    } else {
      ;
    }

    this.pcSelectorlist = {
      width: "100%",
      height: heightTotalVp + "vp",
    };
  }

  build() {
    Flex({ direction: FlexDirection.Column }) {
      Flex({ direction: FlexDirection.Column }) {
        Text($r("app.string.message_title_selector"))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .textAlign(TextAlign.Start)
          .margin({ top: 25, left: 20 })
      }
      .height(120)

      Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
        List() {
          ForEach(globalThis.pcShowHapList, (item: any) => {
            ListItem() {
              Flex({ direction: FlexDirection.Row }) {
                if (item.split("#")[3] != "") {
                  Image(item.split("#")[3])
                    .height(70)
                    .width(70)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left: 16 })
                } else {
                  Image($r("app.media.app_icon"))
                    .height(70)
                    .width(70)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left: 16 })
                }
                if (item.split("#")[2] != "") {
                  Text(item.split("#")[2])
                    .fontSize(16)
                    .width("80%")
                    .height(60)
                    .alignSelf(ItemAlign.Center)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .margin({ left: 16 })
                } else {
                  Text(item.split("#")[0])
                    .fontSize(16)
                    .width("80%")
                    .height(60)
                    .alignSelf(ItemAlign.Center)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .margin({ left: 16 })
                }
              }.onClick(() => {
                this.onSelectApp(item)
              })
            }.margin({ top: 4, bottom: 4 }).backgroundColor("#ffffff")
          }, item => item)
        }
        .margin({ left: 1, right: 1 })
        .backgroundColor("#ffffff")
        .scrollBar(globalThis.pcShowHapList.length > 4 ? BarState.Auto : BarState.Off)
        .divider({ strokeWidth: 1 })
      }
      .backgroundColor("#ffffff")
      .width("90%")
      .height(this.pcSelectorlist.height)
      .margin({ bottom: 10 })

      Flex({ alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
        Flex({ direction: FlexDirection.Column, justifyContent: FlexAlign.Center, alignItems: ItemAlign.Center }) {
          Text($r("app.string.message_cancel_selector"))
            .fontSize("21fp")
            .fontColor("#0A59F7")
            .fontWeight(FontWeight.Regular)
            .textAlign(TextAlign.Center)
        }
        .width("90%")
        .height(50)
        .margin({ top: 10 })
        .borderRadius(28)
        .backgroundColor("#F2F2F2")
      }
      .width("90%")
      .margin({ left: 15, bottom: 20 })
      .backgroundColor("#ffffff")
      .borderRadius(20)
      .onClick(() => {
        globalThis.selectExtensionContext.terminateSelf();
      })
    }
    .borderRadius(24)
    .borderWidth(1)
    .borderColor("#e9e9e9")
    .backgroundColor("#ffffff")
    .width("100%")
    .height("100%")
  }
}