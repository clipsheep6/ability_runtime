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

import type common from '@ohos.app.ability.common';
import display from '@ohos.display';
import drawableDescriptor from '@ohos.arkui.drawableDescriptor';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import type image from '@ohos.multimedia.image';
import type want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import { GlobalThis } from '../utils/GlobalThis';
import type { BusinessError } from '@ohos.base';
import type PositionUtils from '../utils/PositionUtils';
import type { Position } from '../utils/PositionUtils';
import {Configuration} from '@ohos.app.ability.Configuration';

const TAG = 'SelectorDialog_Service';

let winNum = 1;
let win: window.Window;

export default class SelectorServiceExtensionAbility extends extension {
  onCreate(want: want): void {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setContext('selectExtensionContext', this.context);
  }

  async getPhoneShowHapList() {
    const lineNums = 8;
    let showHapList: string[] = [];
    let phoneShowHapList: string[][] = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    let hapList = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.hapList as Record<string, Object>[];
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
    GlobalThis.getInstance().setProperty('phoneShowHapList', phoneShowHapList);
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
    GlobalThis.getInstance().setProperty('signalRowPhoneShowHapList', signalRowPhoneShowHapList);
    GlobalThis.getInstance().setObject('jsonIconMap', jsonIconMap);
    console.debug(TAG, 'signalRowPhoneShowHapList: ' + JSON.stringify(signalRowPhoneShowHapList));
  }

  async getPcShowHapList() {
    let pcShowHapList: string[] = [];
    let jsonIconMap: Map<string, image.PixelMap> = new Map();
    let hapList = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.hapList as Record<string, Object>[];
    for (let i = 0; i < hapList.length; i++) {
      await this.getHapResource(hapList[i], pcShowHapList, jsonIconMap);
    }
    GlobalThis.getInstance().setProperty('pcShowHapList', pcShowHapList);
    GlobalThis.getInstance().setObject('jsonIconMap', jsonIconMap);
    console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(pcShowHapList));
  }

  async getHapResource(hap: Record<string, Object>, showHapList: string[], jsonIconMap: Map<string, image.PixelMap>): Promise<void> {
    let bundleName = hap.bundle as string;
    let moduleName = hap.module as string;
    let abilityName = hap.ability as string;
    let appName = '';
    let appIcon = '';
    let type = '';
    let userId = Number('0');
    if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'phone' && (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'default') {
      type = hap.type as string;
      userId = Number(hap.userId);
    }
    let lableId = Number(hap.label);
    let moduleContext = GlobalThis.getInstance().getContext('selectExtensionContext')?.createModuleContext(bundleName, moduleName) as common.ServiceExtensionContext;
    await moduleContext?.resourceManager.getString(lableId).then((value: string) => {
      appName = value;
    }).catch((error: BusinessError) => {
      console.error(TAG, 'getString error:' + JSON.stringify(error));
    });

    let iconId = Number(hap.icon);
    await moduleContext?.resourceManager.getMediaBase64(iconId).then((value: string) => {
      appIcon = value;
      if (appIcon.indexOf('image/json') > -1) {
        try {
          const imageDescriptor = moduleContext?.resourceManager.getDrawableDescriptor(iconId) as DrawableDescriptor;
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

  async onRequest(want: want, startId: number): Promise<void> {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setObject('abilityWant', want);
    GlobalThis.getInstance().setObject('params', JSON.parse(want.parameters?.params as string));
    let displayClass = display.getDefaultDisplaySync();
    let hapList = (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.hapList as Record<string, Object>[];
    let lineNums = hapList.length;
    GlobalThis.getInstance().setObject('position', PositionUtils.getSelectorDialogPosition(lineNums));
    try {
      display.on('change', (data: number) => {
        let position = PositionUtils.getSelectorDialogPosition(lineNums);
        if (position.offsetX !== (GlobalThis.getInstance().getObject('position') as Position)?.offsetX || position.offsetY !== (GlobalThis.getInstance().getObject('position') as Position)?.offsetY) {
          win.moveWindowTo(position.offsetX, position.offsetY);
        }
        if (position.width !== (GlobalThis.getInstance().getObject('position') as Position)?.width || position.height !== (GlobalThis.getInstance().getObject('position') as Position)?.height) {
          win.resize(position.width, position.height);
        }
        GlobalThis.getInstance().setObject('position', position);
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
    }

    console.debug(TAG, 'onRequest display is' + JSON.stringify(displayClass));
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(GlobalThis.getInstance().getProperty('params')));
    GlobalThis.getInstance().setObject('callerToken', want.parameters?.callerToken);
    console.debug(TAG, 'onRequest, position: ' + JSON.stringify(GlobalThis.getInstance().getObject('position')));
    if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'phone' && (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'default') {
      GlobalThis.getInstance().setProperty('modelFlag', Boolean((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.modelFlag));
      GlobalThis.getInstance().setProperty('action', Boolean((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.action));
    }
    if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'phone' || (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'default') {
      await this.getPhoneShowHapList();
    } else {
      await this.getPcShowHapList();
    }

    AppStorage.SetOrCreate('oversizeHeight', (GlobalThis.getInstance().getObject('position') as Position)?.oversizeHeight ? 'true' : 'false');
    let dis = display.getDefaultDisplaySync();
    let navigationBarRect : window.Rect = {
      left: (GlobalThis.getInstance().getObject('position') as Position)?.offsetX as number,
      top: (GlobalThis.getInstance().getObject('position') as Position)?.offsetY as number,
      width: (GlobalThis.getInstance().getObject('position') as Position)?.width as number,
      height: (GlobalThis.getInstance().getObject('position') as Position)?.height as number
    };
    if (winNum > 1) {
      win.destroyWindow();
      winNum--;
    }
    let windowType = (GlobalThis.getInstance().getProperty('callerToken') === 'object' && GlobalThis.getInstance().getProperty('callerToken') !== null) ?
    window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
    this.createWindow('SelectorDialog' + startId, windowType, navigationBarRect);
    winNum++;
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
    if (win !== undefined) {
      win.destroyWindow();
    }
  }

  private async createWindow(name: string, windowType: window.WindowType, rect: window.Rect): Promise<void> {
    console.info(TAG, 'create window');
    try {
      let configuration: window.Configuration = {
        name: name,
        windowType: windowType,
        ctx: GlobalThis.getInstance().getContext('selectExtensionContext') as common.BaseContext,
      };
      win = await window.createWindow(configuration);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget((GlobalThis.getInstance().getProperty('callerToken') as Record<string, Object>)?.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            GlobalThis.getInstance().getContext('selectExtensionContext')?.terminateSelf();
          }
        });
      }
      await win.hideNonSystemFloatingWindows(true);
      await win.moveWindowTo(rect.left, rect.top);
      await win.resize(rect.width, rect.height);
      if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'phone' || (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'default') {
        await win.setUIContent('pages/selectorPhoneDialog');
      } else {
        await win.setUIContent('pages/selectorPcDialog');
      }
      await win.setWindowBackgroundColor('#00000000');
      await win.showWindow();
    } catch (e) {
      console.error(TAG, 'window create failed: ' + JSON.stringify(e));
    }
  }

  private async moveWindow(rect: window.Rect): Promise<void> {
    try {
      await win.moveWindowTo(rect.left, rect.top);
      await win.resize(rect.width, rect.height);
      if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'phone' || (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType === 'default') {
        try {
          await win.setUIContent('pages/selectorPhoneDialog');
          await win.setWindowBackgroundColor('#00000000');
        } catch (e) {
          console.error(TAG, 'window loadContent failed: ' + JSON.stringify(e));
        }
      }
      await win.showWindow();
    } catch (e) {
      console.error(TAG, 'window move failed: ' + JSON.stringify(e));
    }
  }

  onConfigurationUpdate(config: Configuration): void {
    console.debug(TAG, 'configuration is : ' + JSON.stringify(config));
    if ((GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'phone' && (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.deviceType !== 'default') {
      console.debug(TAG, 'device is not phone');
      return;
    }
    let displayClass = display.getDefaultDisplaySync();
    console.debug(TAG, 'display is' + JSON.stringify(displayClass));
    if (displayClass.orientation === display.Orientation.PORTRAIT || displayClass.orientation === display.Orientation.PORTRAIT_INVERTED) {
      GlobalThis.getInstance().setObject('position', GlobalThis.getInstance().getObject('verticalPosition'));
    } else {
      GlobalThis.getInstance().setObject('position', GlobalThis.getInstance().getObject('landScapePosition'));
    }
    let navigationBarRect : window.Rect = {
      left: (GlobalThis.getInstance().getObject('position') as Position)?.offsetX as number,
      top: (GlobalThis.getInstance().getObject('position') as Position)?.offsetY as number,
      width: (GlobalThis.getInstance().getObject('position') as Position)?.width as number,
      height: (GlobalThis.getInstance().getObject('position') as Position)?.height as number
    };
    AppStorage.SetOrCreate('oversizeHeight', (GlobalThis.getInstance().getObject('position') as Position)?.oversizeHeight ? 'true' : 'false');
    console.debug(TAG, 'onConfigurationUpdate navigationBarRect is' + JSON.stringify(navigationBarRect));
    this.moveWindow(navigationBarRect);
  }
};