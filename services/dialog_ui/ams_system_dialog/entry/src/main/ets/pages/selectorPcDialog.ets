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

import common from '@ohos.app.ability.common';
import bundleManager from '@ohos.bundle.bundleManager';
import drawableDescriptor from '@ohos.arkui.drawableDescriptor';
import want from '@ohos.app.ability.Want';
import { BusinessError } from '@ohos.base';
import { GlobalThis } from '../utils/GlobalThis';
import defaultAppManager from '@ohos.bundle.defaultAppManager';
import image from '@ohos.multimedia.image';


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
  @State private hapList: Record<string, Object>[] = [];
  @State private pcShowMoreHapList: string[] = [];
  private TAG = "SelectorDialog_Pc_Page";
  @State private moreOpenModeVisiable: number = 0;
  @State private moreOpenModeAreaHigh: number = 54;
  @State private moreAppsVisiable: number = 2;
  @State private moreAppsAreaHigh: number = 0;
  @State private recommendAppsVisiable: number = 0;
  @State private checkboxVisiable: number = 0;
  @State private moreHapListVisibility:number = 2;
  @State private totalHigh: number = 0;
  @State private isCheckSelected: boolean = false;
  @State private action: string = "";
  @State public hapListBackGround: number[] = [0xffffff];
  @State public hapListPngVisibility: number[] = [];
  @State public moreHapListBackGround: number[] = [0xffffff];
  @State public moreHapListPngVisibility: number[] = [];
  public moreHapListType: string = "reserved/wildcard";
  @State public isCheck: boolean = false;
  private selectItem: string = "";


  aboutToAppear(): void {
    console.log(this.TAG, "dialog page appears");
    this.hapList = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.hapList as Record<string, Object>[];
    this.action = GlobalThis.getInstance().getProperty('action') as string;
    this.getHapListStyle();
    this.cleanFirstListColorAndVisibility()
  }

  cleanFirstListColorAndVisibility() {
    for(let n = 0; n <= (GlobalThis.getInstance().getProperty('pcShowHapList') as string[]).length; n++) {
      this.hapListBackGround[n] = 0xffffff;
      this.hapListPngVisibility[n] = 1;
    }
  }

  cleanSecondListColorAndVisibility() {
    for(let n = 0; n <= this.pcShowMoreHapList.length; n++) {
      this.moreHapListBackGround[n] = 0xffffff;
      this.moreHapListPngVisibility[n] = 1;
    }
  }

  onSelectApp(item: string) {
    let abilityWant = GlobalThis.getInstance().getObject('abilityWant') as want
    if (abilityWant != undefined) {
      abilityWant.bundleName =  item.split("#")[0];
      abilityWant.abilityName = item.split("#")[1];
      abilityWant.moduleName = item.split("#")[4];
    }
    GlobalThis.getInstance().setObject('abilityWant', abilityWant);
    GlobalThis.getInstance().getContext('selectExtensionContext')?.startAbilityAsCaller(GlobalThis.getInstance().getObject('abilityWant') as want, (error : BusinessError) => {
      if (error) {
        console.error(this.TAG + " startAbility finish, error: " + JSON.stringify(error));
        return;
      }
      console.log(this.TAG + " startAbility finish");
    });
    let userId: string = item.split("#")[6];
    let elementName: bundleManager.ElementName = {
      bundleName: (GlobalThis.getInstance().getObject('abilityWant') as want)?.bundleName as string,
      moduleName: (GlobalThis.getInstance().getObject('abilityWant') as want)?.moduleName as string,
      abilityName: (GlobalThis.getInstance().getObject('abilityWant') as want)?.abilityName as string,
    }
    if (this.isCheckSelected) {
      let typeName: string = item.split("#")[5];
      console.log(this.TAG + "setDefaultApplication typeName:" + JSON.stringify(typeName));
      defaultAppManager.setDefaultApplication(typeName, elementName);
    }
    GlobalThis.getInstance().getContext('selectExtensionContext')?.terminateSelf();
  }

  onClickMoreApp() {
    try {
      bundleManager.queryAbilityInfo({
        action: this.action,
        type : this.moreHapListType,
      }, 0, (err: BusinessError, data: Array<bundleManager.AbilityInfo>) =>{
        if (data != undefined) {
          for (let i = 0; i < data.length; i++) {
            this.getHapResource(data[i], this.pcShowMoreHapList, i);
          }
          this.getMoreHapListStyle()
          console.debug("pcShowMoreHapList: " + JSON.stringify(this.pcShowMoreHapList));
        }
      });
    } catch { (error: BusinessError) => {
      console.error("queryAbilityInfo error:" + JSON.stringify(error));
    }
    }
  }

  getHapResource(hap: bundleManager.AbilityInfo, showHapList: string[], n: number) {
    this.moreHapListBackGround[n] = 0xffffff;
    this.moreHapListPngVisibility[n] = 1
    let bundleName = hap.bundleName;
    let moduleName = hap.moduleName;
    let abilityName = hap.name;
    let appName = "";
    let appIcon = "";
    let lableId = Number(hap.labelId);
    let moduleContext = GlobalThis.getInstance().getContext('selectExtensionContext')?.createModuleContext(bundleName, moduleName) as common.ServiceExtensionContext;
    moduleContext?.resourceManager.getStringValue(lableId).then((value: string) => {
      appName = value;
      let iconId = Number(hap.iconId);
      moduleContext?.resourceManager.getMediaContentBase64(iconId).then((value: string) => {
        appIcon = value;
        if (appIcon.indexOf('image/json') > -1) {
          try {
            const imageDescriptor = moduleContext?.resourceManager.getDrawableDescriptor(iconId) as DrawableDescriptor;
            if (imageDescriptor !== null && imageDescriptor !== undefined &&
              imageDescriptor instanceof drawableDescriptor.LayeredDrawableDescriptor) {
              let layeredDrawableDescriptor: drawableDescriptor.LayeredDrawableDescriptor = imageDescriptor;
              let foregroundDescriptor: drawableDescriptor.DrawableDescriptor = layeredDrawableDescriptor.getForeground();
              if (foregroundDescriptor !== null && foregroundDescriptor !== undefined) {
                (GlobalThis.getInstance().getObject('jsonIconMap') as Map<string, image.PixelMap>)?.set(bundleName + ':' + moduleName + ':' + abilityName,
                  foregroundDescriptor.getPixelMap());
              } else {
                console.error(this.TAG, 'get foregroundDescriptor is null');
              }
            }
          } catch (e) {
            console.error(this.TAG, 'get drawableDescriptor error:' + JSON.stringify(e));
          }
        }
        let moreHapList = ""
        moreHapList = bundleName as string + abilityName as string + moduleName as string
        if (this.hapList.find(item => String(item.bundleName as string + item.ability as string + item.moduleName as string) ==
        String(moreHapList)) == undefined) {
          showHapList.push(bundleName + "#" + abilityName + "#" + appName +
            "#" + appIcon + "#" + moduleName);
          this.getMoreHapListStyle()
        }
      }).catch((error: BusinessError) => {
        console.error("getMediaBase64 error:" + JSON.stringify(error));
      });
    }).catch((error: BusinessError) => {
      console.error("getString error:" + JSON.stringify(error));
    });
  }

  getMoreHapListStyle() {
    let heightTotalVp = 200;
    let heightVal = 64;
    let recommendAppAreaHigh = 48;
    let openModeHigh = 54;
    let morelist = 0;
    morelist ++;
    if (this.hapList.length == 2) {
      if(morelist == 1) {
        this.totalHigh = 447;
        heightTotalVp = 262;
      }else if(morelist >= 2) {
        this.totalHigh = 511;
        heightTotalVp = 326;
      } else {
        ;
      }
    } else if (this.hapList.length == 3) {
      this.totalHigh = 479;
      if (morelist >= 1) {
        this.totalHigh = 511;
        heightTotalVp = 326;
      }
    } else if (this.hapList.length == 4) {
      this.totalHigh = 543;
      heightTotalVp = this.hapList.length * heightVal + recommendAppAreaHigh + openModeHigh;
      if(morelist != 0) {
      } else {
      }
    } else if (this.hapList.length > 4) {
      this.totalHigh = 543;
      heightTotalVp = 4 * heightVal + recommendAppAreaHigh + openModeHigh - 32;
    }
    this.pcSelectorlist = {
      width: "100%",
      height: heightTotalVp + "vp",
    };
  }

  getHapListStyle() {
    let heightTotalVp = 0;
    let heightVal = 64;
    let recommendAppAreaHigh = 48;
    let openModeHigh = 54;
    let titleAreaHigh = 56;
    let buttonAreaHigh = 81;
    if (GlobalThis.getInstance().getProperty('modelFlag') == true) {
      this.recommendAppsVisiable = 2;
      this.moreOpenModeVisiable = 2;
      this.checkboxVisiable = 2;
      heightTotalVp = this.hapList.length * heightVal;
      this.totalHigh = this.hapList.length * heightVal + titleAreaHigh + buttonAreaHigh;
      if(this.hapList.length > 6) {
        heightTotalVp = 6 * heightVal;
        this.totalHigh = 6 * heightVal + titleAreaHigh + buttonAreaHigh;
      }
    }
    if(GlobalThis.getInstance().getProperty('modelFlag') == false) {
      heightTotalVp = this.hapList.length * heightVal + recommendAppAreaHigh + openModeHigh;
      if (this.hapList.length == 1 || this.hapList.length == 2) {
        this.totalHigh = 415;
      } else if (this.hapList.length == 3) {
        this.totalHigh = 479;
      } else if (this.hapList.length == 4) {
        this.totalHigh = 543;
      } else if (this.hapList.length > 4) {
        this.totalHigh = 543;
        heightTotalVp = 4 * heightVal + recommendAppAreaHigh + openModeHigh - 32;
      }
    }
    this.pcSelectorlist = {
      width: "100%",
      height: heightTotalVp + "vp",
    };
  }

  build() {
    Flex({ direction: FlexDirection.Column , justifyContent: FlexAlign.Center ,  alignItems: ItemAlign.Center }) {
      Column() {
        Text($r("app.string.message_title_pc_selector"))
          .fontSize(22)
          .fontWeight(FontWeight.Medium)
          .textAlign(TextAlign.Start)
          .margin({ left: 48 })
          .width(394)
          .height(56)
        Scroll() {
          List() {
            ListItem() {
              Text($r("app.string.message_recommendApps_selector"))
                .fontSize(14)
                .fontWeight(FontWeight.Medium)
                .width(394)
                .height(48)
                .visibility(this.recommendAppsVisiable)
            }
            ForEach(GlobalThis.getInstance().getProperty('pcShowHapList') as string[], (item: string, current: number) => {
              ListItem() {
                Flex({ direction: FlexDirection.Row }) {
                  if (item.split("#")[3] != "") {
                    if (item.split("#")[3].indexOf('image/json') > -1) {
                      Image((GlobalThis.getInstance().getObject('jsonIconMap') as Map<string, image.PixelMap>)?.get(item.split("#")[0] + ':' + item.split("#")[4]
                        + ':' + item.split("#")[1]))
                        .height(48)
                        .width(48)
                        .alignSelf(ItemAlign.Center)
                        .margin({ left : 16 })
                        .borderRadius(12);
                    } else {
                      Image(item.split("#")[3])
                        .height(48)
                        .width(48)
                        .alignSelf(ItemAlign.Center)
                        .margin({ left : 16 })
                        .borderRadius(12);
                    }
                  } else {
                    Image($r("app.media.app_icon"))
                      .height(48)
                      .width(48)
                      .alignSelf(ItemAlign.Center)
                      .margin({ left : 16 })
                      .borderRadius(12)
                  }
                  if (item.split("#")[2] != "") {
                    Text(item.split("#")[2])
                      .fontSize(16)
                      .width("60%")
                      .height(64)
                      .alignSelf(ItemAlign.Center)
                      .textOverflow({ overflow: TextOverflow.Ellipsis })
                      .margin({ left : 16 })
                  } else {
                    Text(item.split("#")[0])
                      .fontSize(16)
                      .width("60%")
                      .height(64)
                      .alignSelf(ItemAlign.Center)
                      .textOverflow({ overflow: TextOverflow.Ellipsis })
                      .margin({ left : 16 })
                  }
                  Image($r("app.media.selectedMark"))
                    .height(24)
                    .width(24)
                    .alignSelf(ItemAlign.Center)
                    .margin({ right : 16 })
                    .visibility(this.hapListPngVisibility[current])
                }
                .onClick(() => {
                  this.cleanFirstListColorAndVisibility();
                  this.cleanSecondListColorAndVisibility();
                  this.selectItem = item;
                  this.hapListBackGround[current] = 0x1A0A59F7;
                  this.hapListPngVisibility[current] = 0;
                  this.isCheck = true;
                })
              }
              .backgroundColor(this.hapListBackGround[current])
              .borderRadius(12)
              .height(64)
              .width(328)
            }, (item: string) => item)
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
                  this.onClickMoreApp()
                  this.moreOpenModeVisiable = 2
                  this.moreOpenModeAreaHigh = 0
                  this.moreHapListVisibility = 0
                  this.moreAppsVisiable = 0
                  this.moreAppsAreaHigh = 48
                })
            }
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
            .height(this.moreAppsAreaHigh)
            ForEach(this.pcShowMoreHapList, (item: string, current: number) => {
              ListItem() {
                Flex({ direction: FlexDirection.Row }) {
                  if (item.split("#")[3] != "") {
                    if (item.split("#")[3].indexOf('image/json') > -1) {
                      Image((GlobalThis.getInstance().getObject('jsonIconMap') as Map<string, image.PixelMap>)?.get(item.split("#")[0] + ':' + item.split("#")[4]
                        + ':' + item.split("#")[1]))
                        .height(48)
                        .width(48)
                        .alignSelf(ItemAlign.Center)
                        .margin({ left : 16 })
                        .borderRadius(12);
                    } else {
                      Image(item.split("#")[3])
                        .height(48)
                        .width(48)
                        .alignSelf(ItemAlign.Center)
                        .margin({ left : 16 })
                        .borderRadius(12)
                    }
                  } else {
                    Image($r("app.media.app_icon"))
                      .height(48)
                      .width(48)
                      .alignSelf(ItemAlign.Center)
                      .margin({ left : 16 })
                      .borderRadius(12)
                  }
                  if (item.split("#")[2] != "") {
                    Text(item.split("#")[2])
                      .fontSize(16)
                      .width("60%")
                      .height(64)
                      .alignSelf(ItemAlign.Center)
                      .textOverflow({ overflow: TextOverflow.Ellipsis })
                      .margin({ left : 16 })
                  } else {
                    Text(item.split("#")[0])
                      .fontSize(16)
                      .width("60%")
                      .height(64)
                      .alignSelf(ItemAlign.Center)
                      .height(56)
                      .width(394)
                  }
                  Image($r("app.media.selectedMark"))
                    .height(24)
                    .width(24)
                    .alignSelf(ItemAlign.Center)
                    .margin({ right : 16 })
                    .visibility(this.moreHapListPngVisibility[current])
                }
                .onClick(() => {
                  this.cleanFirstListColorAndVisibility();
                  this.cleanSecondListColorAndVisibility();
                  this.selectItem = item;
                  this.moreHapListBackGround[current] = 0x1A0A59F7;
                  this.moreHapListPngVisibility[current] = 0;
                  this.isCheck = true;
                })
              }
              .backgroundColor(this.moreHapListBackGround[current])
              .borderRadius(12)
              .height(64)
              .width(328)
              .visibility(this.moreHapListVisibility)
            }, (item: string) => item)
          }
        }
        .edgeEffect(EdgeEffect.None)
        .height(this.pcSelectorlist.height)
        .backgroundColor("#ffffff")
        .scrollBar(2)
        .width(346)
        Flex({ direction: FlexDirection.Row }) {
          Checkbox({name: 'checkbox'})
            .select(this.isCheckSelected)
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
            .onClick(() => { GlobalThis.getInstance().getContext('selectExtensionContext')?.terminateSelf(); })
          if(this.isCheck) {
            Button($r("app.string.message_sure_selector"), { type: ButtonType.Capsule , stateEffect : this.isCheck})
              .backgroundColor('#EAEAEA')
              .fontColor('#0A59F7')
              .fontWeight(FontWeight.Medium)
              .width(157)
              .height(40)
              .fontSize(16)
              .borderRadius(20)
              .margin({ top : 16 , left : 48 })
              .onClick(() => { this.onSelectApp(this.selectItem) })
          } else{
            Button($r("app.string.message_sure_selector"), { type: ButtonType.Capsule , stateEffect : this.isCheck})
              .backgroundColor('#EAEAEA')
              .fontColor('#999999')
              .fontWeight(FontWeight.Medium)
              .width(157)
              .height(40)
              .fontSize(16)
              .borderRadius(20)
              .margin({ top : 16 , left : 48 })
          }
        }
      }
      .height(this.totalHigh)
      .align(Alignment.Center)
      .borderRadius(24)
      .borderWidth(1)
      .borderColor('#e9e9e9')
      .width(394)
      .backgroundColor('#ffffffff')
    }
    .width('100%')
    .height('100%')
    .backgroundColor('#00ffffff')
  }
}
