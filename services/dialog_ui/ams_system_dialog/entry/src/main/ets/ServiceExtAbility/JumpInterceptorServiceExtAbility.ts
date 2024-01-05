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

import display from '@ohos.display';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';

const TAG = 'JumpInterceptorDialog_Service';

let winNum = 1;
let win;

export default class JumpInterceptorServiceExtAbility extends extension {
  onCreate(want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    globalThis.jumpInterceptorExtensionContext = this.context;
  }

  async onRequest(want, startId) {
    globalThis.abilityWant = want;
    try {
      globalThis.params = JSON.parse(want.parameters.params);
      globalThis.position = JSON.parse(want.parameters.position);
      globalThis.interceptor_callerBundleName = want.parameters.interceptor_callerBundleName;
      globalThis.interceptor_callerModuleName = want.parameters.interceptor_callerModuleName;
      globalThis.interceptor_callerLabelId = want.parameters.interceptor_callerLabelId;
      globalThis.interceptor_targetModuleName = want.parameters.interceptor_targetModuleName;
      globalThis.interceptor_targetLabelId = want.parameters.interceptor_targetLabelId;
    } catch (err) {
      console.error(TAG, 'Invalid parameters.');
      this.context.terminateSelf();
      return;
    }
    await this.getHapResource();
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
        this.createWindow('JumpInterceptorDialog' + startId, window.WindowType.TYPE_SYSTEM_ALERT, navigationBarRect);
      } else {
        this.createWindow('JumpInterceptorDialog' + startId, window.WindowType.TYPE_FLOAT, navigationBarRect);
      }
      winNum++;
    });
  }

  async getHapResource() {
    console.debug(TAG, 'start getHapResource');
    globalThis.callerAppName = await this.loadAppName(
      globalThis.interceptor_callerBundleName,
      globalThis.interceptor_callerModuleName,
      globalThis.interceptor_callerLabelId
    );
    globalThis.targetAppName = await this.loadAppName(
      globalThis.params.bundleName,
      globalThis.interceptor_targetModuleName,
      globalThis.interceptor_targetLabelId
    );
    console.debug(TAG, 'getHapResource load finished');
  }

  async loadAppName(bundleName: string, moduleName: string, labelId: number) {
    let moduleContext = globalThis.jumpInterceptorExtensionContext.createModuleContext(bundleName, moduleName);
    let appName: string = '';
    try {
      appName = await moduleContext.resourceManager.getString(labelId);
    } catch (error) {
      console.error(TAG, `getMediaBase64 error:${JSON.stringify(error)}`);
    }
    return appName;
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
    if (win !== undefined) {
      win.destroy();
    }
  }

  private async createWindow(name: string, windowType: number, rect) {
    console.info(TAG, 'create window');
    try {
      win = await window.create(globalThis.jumpInterceptorExtensionContext, name, windowType);
      await win.hideNonSystemFloatingWindows(true);
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.loadContent('pages/jumpInterceptorDialog');
      await win.setBackgroundColor('#00000000');
      await win.show();
    } catch {
      console.error(TAG, 'window create failed!');
    }
  }
};
