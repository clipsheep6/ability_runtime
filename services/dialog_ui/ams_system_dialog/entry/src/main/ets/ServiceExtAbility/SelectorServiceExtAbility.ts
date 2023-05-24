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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import defaultAppManager from '@ohos.bundle.defaultAppManager';
import bundleManager from '@ohos.bundle.bundleManager';

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
    for (let i = 1; i <= globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i - 1], showHapList);
      if (i % lineNums === 0) {
        phoneShowHapList.push(showHapList);
        showHapList = [];
      }
      if (i >= globalThis.params.hapList.length && showHapList.length > 0) {
        phoneShowHapList.push(showHapList);
      }
    }
    globalThis.phoneShowHapList = phoneShowHapList;
    console.debug(TAG, 'phoneShowHapList: ' + JSON.stringify(phoneShowHapList));
  }

  async getPcShowHapList() {
    let pcShowHapList = [];
    for (let i = 0; i < globalThis.params.hapList.length; i++) {
      await this.getHapResource(globalThis.params.hapList[i], pcShowHapList);
    }
    globalThis.pcShowHapList = pcShowHapList;
    console.debug(TAG, 'pcShowHapList: ' + JSON.stringify(pcShowHapList));
  }

  async getHapResource(hap, showHapList) {
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
    }).catch(error => {
      console.error(TAG, 'getMediaBase64 error:' + JSON.stringify(error));
    });
    showHapList.push(bundleName + '#' + abilityName + '#' + appName +
      '#' + appIcon + '#' + moduleName + '#' + type + '#' + userId);
  }

  async onRequest(want, startId) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    globalThis.abilityWant = want;
    globalThis.params = JSON.parse(want['parameters']['params']);
    globalThis.position = JSON.parse(want['parameters']['position']);
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
    globalThis.callerToken = want['parameters']['callerToken'];
    console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
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
      console.debug(TAG, 'onRequest, params: ' + JSON.stringify(globalThis.params));
      this.createWindow('SelectorDialog' + startId, window.WindowType.TYPE_DIALOG, navigationBarRect);

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
      await win.bindDialogTarget(globalThis.callerToken.value, () => {
        win.destroyWindow();
        winNum--;
        if (winNum === 0) {
          globalThis.selectExtensionContext.terminateSelf();
        }
      });
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
};