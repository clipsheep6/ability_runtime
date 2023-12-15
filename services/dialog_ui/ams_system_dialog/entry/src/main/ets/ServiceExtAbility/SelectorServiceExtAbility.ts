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
import common from '@ohos.app.ability.common';
import defaultAppManager from '@ohos.bundle.defaultAppManager';
import display from '@ohos.display';
import drawableDescriptor from '@ohos.arkui.drawableDescriptor';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import image from '@ohos.multimedia.image';
import want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import { GlobalThis } from '../utils/GlobalThis';
import { BusinessError } from '@ohos.base';
import PositionUtils from '../utils/PositionUtils';
import {Configuration} from '@ohos.app.ability.Configuration'

const TAG = 'SelectorDialog_Service';

let winNum = 1;
let win: window.Window;
let hapList: Record<string, Object> [];
let params = GlobalThis.getInstance().getObject('params')
if (params != undefined) {
  hapList = (params as Record<string, Object> [])['hapList'];
}

export default class SelectorServiceExtensionAbility extends extension {
  onCreate(want: want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setContext('selectExtensionContext', this.context);
    // ? namespace in GlobalThis
    GlobalThis.getInstance().setType('defaultAppManager', typeof defaultAppManager);
    GlobalThis.getInstance().setType('bundleManager', typeof bundleManager);
  }

  async getPhoneShowHapList() {
    const lineNums = 8;
    let showHapList: string[] = [];
    let phoneShowHapList: string[][]= [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();

    for (let i = 1; i <= hapList.length; i++) {
      console.info(TAG, 'hapList[' + (i - 1).toString() + ']: ' + JSON.stringify(hapList[i]));
      await this.getHapResource(hapList[i - 1], showHapList, jsonIconMap);
      if (i % lineNums === 0) {
        phoneShowHapList.push(showHapList);
        showHapList = [];
      }
      if (i >= hapList.length && showHapList.length > 0) {
        phoneShowHapList.push(showHapList);
      }
    }
    GlobalThis.getInstance().setObject("phoneShowHapList", phoneShowHapList);
    console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(phoneShowHapList));

    const signalRowlineNums = 4;
    let signalRowShowHapList: string[] = [];
    let signalRowPhoneShowHapList: string[][] = [];
    for (let i = 1; i <= hapList.length; i++) {
      console.info(TAG, 'hapList[' + (i - 1).toString() + ']: ' + JSON.stringify(hapList[i]));
      await this.getHapResource(hapList[i - 1], signalRowShowHapList, jsonIconMap);
      if (i % signalRowlineNums === 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
        signalRowShowHapList = [];
      }
      if (i >= hapList.length && signalRowShowHapList.length > 0) {
        signalRowPhoneShowHapList.push(signalRowShowHapList);
      }
    }
    GlobalThis.getInstance().setObject("signalRowPhoneShowHapList", signalRowPhoneShowHapList);
    GlobalThis.getInstance().setMap("jsonIconMap", jsonIconMap);
    console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(signalRowPhoneShowHapList));
  }

  async getPcShowHapList() {
    let pcShowHapList: string[] = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    for (let i = 0; i < hapList.length; i++) {
      await this.getHapResource(hapList[i], pcShowHapList, jsonIconMap);
    }
    GlobalThis.getInstance().setProperty("pcShowHapList", pcShowHapList);
    GlobalThis.getInstance().setMap("jsonIconMap", jsonIconMap);
    console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(pcShowHapList));
  }

  async getHapResource(hap: Record<string, Object>, showHapList: string[], jsonIconMap: Map<string, image.PixelMap>) {
    let bundleName = hap.bundle as string;
    let moduleName = hap.module as string;
    let abilityName = hap.ability as string;
    let appName = '';
    let appIcon = '';
    let type = '';
    let userId = Number('0');
    if (GlobalThis.getInstance().getRecord('params')?.deviceType !== 'phone' && GlobalThis.getInstance().getRecord('params')?.deviceType !== 'default') {
      type = hap.type as string;
      userId = Number(hap.userId);
    }
    let lableId = Number(hap.label);
    let moduleContext = GlobalThis.getInstance().getContext('selectExtensionContext')?.createModuleContext(bundleName, moduleName);
    await moduleContext?.resourceManager.getString(lableId).then(value => {
      appName = value;
    }).catch((error: BusinessError) => {
      console.error(TAG, 'getString error:' + JSON.stringify(error));
    });

    let iconId = Number(hap.icon);
    await moduleContext?.resourceManager.getMediaBase64(iconId).then(value => {
      appIcon = value;
      if (appIcon.indexOf('image/json') > -1) {
        try {
          const imageDescriptor = moduleContext?.resourceManager.getDrawableDescriptor(iconId);
          if (imageDescriptor !== null && imageDescriptor !== undefined &&
            imageDescriptor instanceof drawableDescriptor.LayeredDrawableDescriptor) {
            let layeredDrawableDescriptor: drawableDescriptor.LayeredDrawableDescriptor = imageDescriptor;
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
    }).catch((error: BusinessError) => {
      console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(error));
    });
    showHapList.push(bundleName + '#' + abilityName + '#' + appName +
      '#' + appIcon + '#' + moduleName + '#' + type + '#' + userId);
  }

  async onRequest(want: want, startId: number) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setWant("abilityWant", want);
    GlobalThis.getInstance().setObject("params",JSON.parse(want.parameters?.params as string));
    let displayClass = display.getDefaultDisplaySync();
    let lineNums = hapList.length;
    GlobalThis.getInstance().setPosition('position', PositionUtils.getSelectorDialogPosition(lineNums));
    try {
      display.on('change', (data: number) => {
        let position = PositionUtils.getSelectorDialogPosition(lineNums);
        if (position.offsetX !== GlobalThis.getInstance().getRecord('position')?.offsetX || position.offsetY !== GlobalThis.getInstance().getRecord('position')?.offsetY) {
          win.moveTo(position.offsetX, position.offsetY);
        }
        if (position.width !== GlobalThis.getInstance().getRecord('position')?.width || position.height !== GlobalThis.getInstance().getRecord('position')?.height) {
          win.resetSize(position.width, position.height);
        }
        GlobalThis.getInstance().setPosition('position', position);
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
    }

    console.debug(TAG, 'onRequest display is' + JSON.stringify(displayClass));
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(GlobalThis.getInstance().getRecord('params')));
    GlobalThis.getInstance().setObject("callerToken", want.parameters?.callerToken);
    console.debug(TAG, 'onRequest, position: ' + JSON.stringify(GlobalThis.getInstance().getPosition('position') as string));
    if (GlobalThis.getInstance().getRecord('params')?.deviceType !== 'phone' && GlobalThis.getInstance().getRecord('params')?.deviceType !== 'default') {
      GlobalThis.getInstance().setProperty('modelFlag', Boolean(GlobalThis.getInstance().getRecord('params')?.modelFlag));
      GlobalThis.getInstance().setProperty('action', Boolean(GlobalThis.getInstance().getRecord('params')?.action));
    }
    if (GlobalThis.getInstance().getRecord('params')?.deviceType === 'phone' || GlobalThis.getInstance().getRecord('params')?.deviceType === 'default') {
      await this.getPhoneShowHapList();
    } else {
      await this.getPcShowHapList();
    }

    AppStorage.SetOrCreate('oversizeHeight', GlobalThis.getInstance().getRecord('position')?.oversizeHeight ? 'true' : 'false');
    display.getDefaultDisplay().then(dis => {
      let navigationBarRect : window.Rect = {
        left: GlobalThis.getInstance().getRecord('position')?.offsetX as number,
        top: GlobalThis.getInstance().getRecord('position')?.offsetY as number,
        width: GlobalThis.getInstance().getRecord('position')?.width as number,
        height: GlobalThis.getInstance().getRecord('position')?.height as number
      };
      if (winNum > 1) {
        win.destroy();
        winNum--;
      }
      let windowType = (typeof(GlobalThis.getInstance().getObject("callerToken")) === 'object' && GlobalThis.getInstance().getObject("callerToken") !== null) ?
      window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
      this.createWindow('SelectorDialog' + startId, windowType, navigationBarRect);
      winNum++;
    });
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
    if (win !== undefined) {
      win.destroy();
    }
  }

  private async createWindow(name: string, windowType: window.WindowType, rect: window.Rect) {
    console.info(TAG, 'create window');
    try {
      win = await window.create(GlobalThis.getInstance().getContext('selectExtensionContext') as common.BaseContext, name, windowType);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        // ? the type of callerToken
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            GlobalThis.getInstance().getContext('selectExtensionContext')?.terminateSelf();
          }
        });
      }
      await win.hideNonSystemFloatingWindows(true);
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (GlobalThis.getInstance().getRecord('params')?.deviceType === 'phone' || GlobalThis.getInstance().getRecord('params')?.deviceType === 'default') {
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

  private async moveWindow(rect: window.Rect): Promise<void> {
    try {
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      if (GlobalThis.getInstance().getRecord('params')?.deviceType === 'phone' || GlobalThis.getInstance().getRecord('params')?.deviceType === 'default') {
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

  onConfigurationUpdate(config: Configuration): void {
    console.debug(TAG, 'configuration is : ' + JSON.stringify(config));
    if (GlobalThis.getInstance().getRecord('params')?.deviceType !== 'phone' && GlobalThis.getInstance().getRecord('params')?.deviceType !== 'default') {
      console.debug(TAG, 'device is not phone');
      return;
    }
    let displayClass = display.getDefaultDisplaySync();
    console.debug(TAG, 'display is' + JSON.stringify(displayClass));
    if (displayClass.orientation === display.Orientation.PORTRAIT || displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
      GlobalThis.getInstance().setPosition('position', GlobalThis.getInstance().getPosition('verticalPosition'));
    } else {
      GlobalThis.getInstance().setPosition('position', GlobalThis.getInstance().getPosition('landScapePosition'));
    }
    let navigationBarRect : window.Rect = {
      left: GlobalThis.getInstance().getRecord('position')?.offsetX as number,
      top: GlobalThis.getInstance().getRecord('position')?.offsetY as number,
      width: GlobalThis.getInstance().getRecord('position')?.width as number,
      height: GlobalThis.getInstance().getRecord('position')?.height as number
    };
    AppStorage.SetOrCreate('oversizeHeight', GlobalThis.getInstance().getRecord('position')?.oversizeHeight ? 'true' : 'false');
    console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(navigationBarRect));
    this.moveWindow(navigationBarRect);
  }
};