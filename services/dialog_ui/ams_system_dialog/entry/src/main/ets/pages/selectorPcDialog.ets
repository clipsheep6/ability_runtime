/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
  @State private moreOpenModeVisiable: number = 0;
  @State private moreOpenModeAreaHigh: number = 54;
  @State private moreAppsVisiable: number = 2;
  @State private moreAppsAreaHigh: number = 0;
  @State private recommendAppsVisiable: number = 0;
  @State private checkboxVisiable: number = 0;
  @State private secondListVisiable:number = 2;
  @State private totalHigh: number = 0;
  @State private selectFlag: boolean = true;
  @State private isCheckSelected: boolean = true;
  private selectItem;

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
      globalThis.selectExtensionContext.terminateSelf();
    });
    let type = item.split("#")[5];
    let userId = item.split("#")[6];
    let elementName = {
      bundleName: globalThis.abilityWant.bundleName,
      moduleName: globalThis.abilityWant.moduleName,
      abilityName: globalThis.abilityWant.abilityName,
    }
    if (this.isCheckSelected) {
      globalThis.defaultAppManager.setDefaultApplication(type, elementName, userId);
    }
    globalThis.selectExtensionContext.terminateSelf();
  }

  getHapListStyle() {
    let heightTotalVp = 0;
    let heightVal = 64;
    let recommendAppAreaHigh = 48;
    let openModeHigh = 54;
    if (this.hapList.length == 2) {
        this.totalHigh = 415;
        heightTotalVp = this.hapList.length * heightVal + recommendAppAreaHigh + openModeHigh;
    } else if (this.hapList.length == 3) {
        this.totalHigh = 479;
        heightTotalVp = this.hapList.length * heightVal + recommendAppAreaHigh + openModeHigh;
    } else if (this.hapList.length == 4) {
        this.totalHigh = 543;
        heightTotalVp = this.hapList.length * heightVal + recommendAppAreaHigh + openModeHigh;
    } else if (this.hapList.length > 4) {
        this.totalHigh = 543;
        heightTotalVp = 4 * heightVal + recommendAppAreaHigh + openModeHigh;
    } else if (this.hapList.length == 0) {
        this.recommendAppsVisiable = 2;
        this.moreOpenModeVisiable = 2;
        this.checkboxVisiable = 2;
        heightTotalVp = 406;
        this.totalHigh = 543;
    } else {
      ;
    }
    this.pcSelectorlist = {
      width: "100%",
      height: heightTotalVp + "vp",
    };
  }

  build() {
    Column() {
        Text($r("app.string.message_title_selector"))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .textAlign(TextAlign.Start)
          .margin({ left: 48 })
          .width(394)
          .height(56)
        List() {
          ListItem() {
            Text($r("app.string.message_recommendApps_selector"))
              .fontSize(14)
              .fontWeight(FontWeight.Medium)
              .width(394)
              .height(48)
              .visibility(this.recommendAppsVisiable)
          }
          .selectable(false)
          ForEach(globalThis.pcShowHapList, (item: any) => {
            ListItem() {
              Flex({ direction: FlexDirection.Row }) {
                if (item.split("#")[3] != "") {
                  Image(item.split("#")[3])
                    .height(48)
                    .width(48)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left : 16 })
                } else {
                  Image($r("app.media.app_icon"))
                    .height(48)
                    .width(48)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left : 16 })
                }
                if (item.split("#")[2] != "") {
                  Text(item.split("#")[2])
                    .fontSize(16)
                    .width("80%")
                    .height(64)
                    .alignSelf(ItemAlign.Center)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .margin({ left : 16 })
                } else {
                  Text(item.split("#")[0])
                    .fontSize(16)
                    .width("80%")
                    .height(64)
                    .alignSelf(ItemAlign.Center)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .margin({ left : 16 })
                }
              }
              .onClick(() => {
                this.selectItem = item
                this.selectFlag = true
              })
            }
            .height(64)
            .width(346)
            .selectable(this.selectFlag)
            .onSelect(() => {
                this.selectFlag = false;
            })
          }, item => item)
          ListItem() {
            Button($r("app.string.message_moreOpenMode_selector"),{ type: ButtonType.Capsule })
              .fontSize(16)
              .backgroundColor('#ffffffff')
              .fontWeight(FontWeight.Medium)
              .width(128)
              .height(40)
              .fontColor('#0A59F7')
              .margin({ right : 218 })
              .onClick(() => {
                this.moreOpenModeVisiable = 2
                this.moreOpenModeAreaHigh = 0
                this.secondListVisiable = 0
                this.moreAppsVisiable = 0
                this.moreAppsAreaHigh = 48
            })
          }
          .selectable(false)
          .visibility(this.moreOpenModeVisiable)
          .width(346)
          .height(this.moreOpenModeAreaHigh)
          ListItem() {
            Text($r("app.string.message_moreApps_selector"))
              .fontSize(14)
              .fontWeight(FontWeight.Medium)
              .visibility(this.moreAppsVisiable)
              .margin({ right : 234 })
          }
          .width(346)
          .selectable(false)
          .height(this.moreAppsAreaHigh)
          ForEach(globalThis.pcShowOtherHapList, (item) => {
            ListItem() {
              Flex({ direction: FlexDirection.Row }) {
                if (item.split("#")[3] != "") {
                  Image(item.split("#")[3])
                    .height(48)
                    .width(48)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left : 16 })
                } else {
                  Image($r("app.media.app_icon"))
                    .height(48)
                    .width(48)
                    .alignSelf(ItemAlign.Center)
                    .margin({ left : 16 })
                }
                if (item.split("#")[2] != "") {
                  Text(item.split("#")[2])
                    .fontSize(16)
                    .width("80%")
                    .height(64)
                    .alignSelf(ItemAlign.Center)
                    .textOverflow({ overflow: TextOverflow.Ellipsis })
                    .margin({ left : 16 })
                } else {
                  Text(item.split("#")[0])
                    .fontSize(16)
                    .width("80%")
                    .height(64)
                    .alignSelf(ItemAlign.Center)      
                    .height(56)
                    .width(394)
                }
              }
              .onClick(() => {
                this.selectItem = item
                this.selectFlag = true
              })
            }
            .height(64)
            .width(346)
            .onSelect(() => {
                this.selectFlag = false;
            })
            .selectable(this.selectFlag)
            .visibility(this.secondListVisiable)
          }, item => item)
        }
        .edgeEffect(EdgeEffect.None)
        .height(this.pcSelectorlist.height)
        .multiSelectable(true)
        .backgroundColor("#ffffff")
        .scrollBar(globalThis.pcShowHapList.length > 4 ? BarState.Auto : BarState.Off)
        .width(346)
        Flex({ direction: FlexDirection.Row }) {
          Checkbox({name: 'checkbox'})
           .select(true)
           .selectedColor(0x39a2db)
           .margin({ left : 8 })
           .onChange((value: boolean) => {
            this.isCheckSelected = value;
           })
          Text($r("app.string.message_openThisWay_selector"))
            .fontSize(14)
            .fontColor('#000000')
            .fontWeight(FontWeight.Regular)
        }
        .visibility(this.checkboxVisiable)
        .height(48)
        .width(394)
        Flex({ direction: FlexDirection.Row }) {
          Button($r("app.string.message_cancel_selector"), { type: ButtonType.Capsule })
            .backgroundColor('#0d000000')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Medium)
            .width(157)
            .height(40)
            .fontSize(16)
            .borderRadius(20)
            .margin({ top : 16 , left : 16 })
            .onClick(() => { globalThis.selectExtensionContext.terminateSelf(); })
          Button($r("app.string.message_sure_selector"), { type: ButtonType.Capsule })
            .backgroundColor('#0d000000')
            .fontColor('#0A59F7')
            .fontWeight(FontWeight.Medium)
            .width(157)
            .height(40)
            .fontSize(16)
            .borderRadius(20)
            .margin({ top : 16 , left : 48 })
            .onClick(() => { this.onSelectApp(this.selectItem) })
        }
    }
    .height(this.totalHigh)
    .borderRadius(24)
    .borderWidth(1)
    .borderColor('#e9e9e9')
    .width(394)
    .backgroundColor('#ffffffff')
  }
}
