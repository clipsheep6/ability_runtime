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

import bundleManager from '@ohos.bundle.bundleManager';
import defaultAppManager from '@ohos.bundle.defaultAppManager';
import display from '@ohos.display';
import drawableDescriptor from '@ohos.arkui.drawableDescriptor';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import type image from '@ohos.multimedia.image';
import window from '@ohos.window';

const TAG = 'SelectorDialog_Service';

let winNum = 1;
let win;

export default class SelectorServiceExtensionAbility extends extension {
  onCreate(want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    globalThis.selectExtensionContext = this.context;
    globalThis.defaultAppManager = defaultAppManager;
    globalThis.bundleManager = bundleManager;
  }

  async getPhoneShowHapList() {
    const lineNums = 8;
    let showHapList = [];
    let phoneShowHapList = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    for (let i = 1; i <= globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i - 1], showHapList, jsonIconMap);
      if (i % lineNums === 0) {
        phoneShowHapList.push(showHapList);
        showHapList = [];
      }
      if (i >= globalThis.params.hapList.length && showHapList.length > 0) {
        phoneShowHapList.push(showHapList);
      }
    }
    globalThis.phoneShowHapList = phoneShowHapList;
    globalThis.jsonIconMap = jsonIconMap;
    console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(phoneShowHapList));

    const signalRowlineNums = 4;
    let signalRowShowHapList = [];
    let signalRowPhoneShowHapList = [];
    let jsonIconMapData: Map<string, image.PixelMap> = new Map();
    for (let i = 1; i <= globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i - 1], signalRowShowHapList, jsonIconMapData);
      if (i % signalRowlineNums === 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
        signalRowShowHapList = [];
      }
      if (i >= globalThis.params.hapList.length && signalRowShowHapList.length > 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
      }
    }
    globalThis.signalRowPhoneShowHapList = signalRowPhoneShowHapList;
    console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(signalRowPhoneShowHapList));
  }

  async getPcShowHapList() {
    let pcShowHapList = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    for (let i = 0; i < globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i], pcShowHapList, jsonIconMap);
    }
    globalThis.pcShowHapList = pcShowHapList;
    globalThis.jsonIconMap = jsonIconMap;
    console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(pcShowHapList));
  }

  async getHapResource(hap, showHapList, jsonIconMap) {
    let bundleName = hap.bundle;
    let moduleName = hap.module;
    let abilityName = hap.ability;
    let appName = '';
    let appIcon = '';
    let type = '';
    let userId = Number('0');
    if (globalThis.params.deviceType !== 'phone' && globalThis.params.deviceType !== 'default') {
      type = hap.type;
      userId = Number(hap.userId);
    }
    let lableId = Number(hap.label);
    let moduleContext = globalThis.selectExtensionContext.createModuleContext(bundleName, moduleName);
    await moduleContext.resourceManager.getString(lableId).then(value => {
      appName = value;
    }).catch(error => {
      console.error(TAG, 'getString error:' + JSON.stringify(error));
    });

    let iconId = Number(hap.icon);
    await moduleContext.resourceManager.getMediaBase64(iconId).then(value => {
      appIcon = value;
      if (appIcon.indexOf('image/json') > -1) {
        try {
          const imageDescriptor = moduleContext.resourceManager.getDrawableDescriptor(iconId);
          if (imageDescriptor !== null && imageDescriptor !== undefined &&
            imageDescriptor instanceof drawableDescriptor.LayeredDrawableDescriptor) {
            let layeredDrawableDescriptor: drawableDescriptor.LayeredDrawableDescriptor =
              <drawableDescriptor.LayeredDrawableDescriptor> imageDescriptor;
            let foregroundDescriptor: drawableDescriptor.DrawableDescriptor = layeredDrawableDescriptor.getForeground();
            if (foregroundDescriptor !== null && foregroundDescriptor !== undefined) {
              jsonIconMap.set(bundleName + ':' + moduleName + ':' + abilityName, foregroundDescriptor.getPixelMap());
            } else {
              console.error(TAG, 'get foregroundDescriptor is null');
            }
          }
        } catch (e) {
          console.error(TAG, 'get drawableDescriptor error:' + JSON.stringify(e));
        }
      }
    }).catch(error => {
      console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(error));
    });
    showHapList.push(bundleName + '#' + abilityName + '#' + appName +
      '#' + appIcon + '#' + moduleName + '#' + type + '#' + userId);
  }

  async onRequest(want, startId) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    globalThis.abilityWant = want;
    globalThis.params = JSON.parse(want.parameters.params);
    let displayClass = display.getDefaultDisplaySync();
    if (globalThis.params.deviceType === 'phone' || globalThis.params.deviceType === 'default') {
      globalThis.landScapePosition = JSON.parse(want.parameters.landscapeScreen);
      globalThis.verticalPosition = JSON.parse(want.parameters.position);
      if (displayClass.orientation === display.Orientation.PORTRAIT || displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
        globalThis.position = globalThis.verticalPosition;
        console.debug(TAG, 'screen position is vertical');
      } else {
        globalThis.position = globalThis.landScapePosition;
        console.debug(TAG, 'screen position is landscape');
      }
    } else {
      globalThis.position = JSON.parse(want.parameters.position);
    }

    console.debug(TAG, 'onRequest display is' + JSON.stringify(displayClass));
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
    globalThis.callerToken = want.parameters.callerToken;
    console.debug(TAG, 'onRequest, position: ' + JSON.stringify(globalThis.position));
    if (globalThis.params.deviceType !== 'phone' && globalThis.params.deviceType !== 'default') {
      globalThis.modelFlag = Boolean(globalThis.params.modelFlag);
      globalThis.action = Boolean(globalThis.params.action);
    }
    if (globalThis.params.deviceType === 'phone' || globalThis.params.deviceType === 'default') {
      await this.getPhoneShowHapList();
    } else {
      await this.getPcShowHapList();
    }

    AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
    display.getDefaultDisplay().then(dis => {
      let navigationBarRect = {
        left: globalThis.position.offsetX,
        top: globalThis.position.offsetY,
        width: globalThis.position.width,
        height: globalThis.position.height
      };
      if (winNum > 1) {
        win.destroy();
        winNum--;
      }
      if (globalThis.params.deviceType === 'phone' || globalThis.params.deviceType === 'default') {
        this.createWindow('SelectorDialog' + startId, window.WindowType.TYPE_FLOAT, navigationBarRect);
      } else {
        console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
        let windowType = (typeof(globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
          window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
        this.createWindow('SelectorDialog' + startId, windowType, navigationBarRect);
      }
      winNum++;
    });
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
  }

  private async createWindow(name: string, windowType: number, rect) {
    console.info(TAG, 'create window');
    try {
      win = await window.create(globalThis.selectExtensionContext, name, windowType);
      if (typeof(globalThis.callerToken) === 'object' && globalThis.callerToken !== null) {
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            globalThis.selectExtensionContext.terminateSelf();
          }
        });
      }
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (globalThis.params.deviceType === 'phone' || globalThis.params.deviceType === 'default') {
        await win.loadContent('pages/selectorPhoneDialog');
      } else {
        await win.loadContent('pages/selectorPcDialog');
      }
      await win.setBackgroundColor('#00000000');
      await win.show();
    } catch (e) {
      console.error(TAG, 'window create failed: ' + JSON.stringify(e));
    }
  }

  private async moveWindow(rect): Promise<void> {
    try {
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (globalThis.params.deviceType === 'phone' || globalThis.params.deviceType === 'default') {
        try {
          await win.loadContent('pages/selectorPhoneDialog');
          await win.setBackgroundColor('#00000000');
        } catch (e) {
          console.error(TAG, 'window loadContent failed: ' + JSON.stringify(e));
        }
      }
      await win.show();
    } catch (e) {
      console.error(TAG, 'window move failed: ' + JSON.stringify(e));
    }
  }

  onConfigurationUpdate(config): void {
    console.debug(TAG, 'configuration is : ' + JSON.stringify(config));
    if (globalThis.params.deviceType !== 'phone' && globalThis.params.deviceType !== 'default') {
      console.debug(TAG, 'device is not phone');
      return;
    }
    let displayClass = display.getDefaultDisplaySync();
    console.debug(TAG, 'display is' + JSON.stringify(displayClass));
    if (displayClass.orientation === display.Orientation.PORTRAIT || displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
      globalThis.position = globalThis.verticalPosition;
    } else {
      globalThis.position = globalThis.landScapePosition;
    }
    let navigationBarRect = {
      left: globalThis.position.offsetX,
      top: globalThis.position.offsetY,
      width: globalThis.position.width,
      height: globalThis.position.height
    };
    AppStorage.SetOrCreate('oversizeHeight', globalThis.position.oversizeHeight ? 'true' : 'false');
    console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(navigationBarRect));
    this.moveWindow(navigationBarRect);
  }
};