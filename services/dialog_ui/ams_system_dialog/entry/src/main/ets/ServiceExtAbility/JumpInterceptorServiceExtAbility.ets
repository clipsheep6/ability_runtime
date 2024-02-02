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

import common from '@ohos.app.ability.common';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import deviceInfo from '@ohos.deviceInfo';
import { GlobalThis } from '../utils/GlobalThis';
import { Position } from '../utils/PositionUtils';

const TAG = 'JumpInterceptorDialog_Service';

let winNum = 1;
let win: window.Window;

export default class JumpInterceptorServiceExtAbility extends extension {
  onCreate(want: want): void {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setContext('jumpInterceptorExtensionContext', this.context);
  }

  async onRequest(want: want, startId: number): Promise<void> {
    GlobalThis.getInstance().setObject('abilityWant', want);
    if (want.parameters !== undefined) {
      GlobalThis.getInstance().setProperty('params', JSON.parse(want.parameters.params as string));
      GlobalThis.getInstance().setObject('position', JSON.parse(want.parameters.position as string));
      GlobalThis.getInstance().setProperty('interceptor_callerBundleName', want.parameters.interceptor_callerBundleName as string);
      GlobalThis.getInstance().setProperty('interceptor_callerModuleName', want.parameters.interceptor_callerModuleName as string);
      GlobalThis.getInstance().setProperty('interceptor_callerLabelId', want.parameters.interceptor_callerLabelId as number);
      GlobalThis.getInstance().setProperty('interceptor_targetModuleName', want.parameters.interceptor_targetModuleName as string);
      GlobalThis.getInstance().setProperty('interceptor_targetLabelId', want.parameters.interceptor_targetLabelId as number);
    }

    await this.getHapResource();
    let navigationBarRect: window.Rect = {
      left: (GlobalThis.getInstance().getObject('position') as Position)?.offsetX as number,
      top: (GlobalThis.getInstance().getObject('position') as Position)?.offsetY as number,
      width: (GlobalThis.getInstance().getObject('position') as Position)?.width as number,
      height: (GlobalThis.getInstance().getObject('position') as Position)?.height as number
    };
    if (winNum > 1) {
      win.destroyWindow();
      winNum--;
    }
    this.createWindow('JumpInterceptorDialog' + startId, window.WindowType.TYPE_FLOAT, navigationBarRect);
    winNum++;
  }

  async getHapResource(): Promise<void> {
    console.debug(TAG, 'start getHapResource');
    let callerAppName: string = await this.loadAppName(
      GlobalThis.getInstance().getProperty('interceptor_callerBundleName') as string,
      GlobalThis.getInstance().getProperty('interceptor_callerModuleName') as string,
      GlobalThis.getInstance().getProperty('interceptor_callerLabelId') as number
    );
    GlobalThis.getInstance().setProperty('callerAppName', callerAppName);
    let targetAppName: string = await this.loadAppName(
      (GlobalThis.getInstance().getProperty('params') as Record<string, Object>)?.bundleName as string,
      GlobalThis.getInstance().getProperty('interceptor_targetModuleName') as string,
      GlobalThis.getInstance().getProperty('interceptor_targetLabelId') as number
    );
    GlobalThis.getInstance().setProperty('targetAppName', targetAppName);
    console.debug(TAG, 'getHapResource load finished');
  }

  async loadAppName(bundleName: string, moduleName: string, labelId: number): Promise<string> {
    let moduleContext = GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext')?.createModuleContext(bundleName, moduleName) as common.ServiceExtensionContext;
    let appName: string = '';
    try {
      appName = await moduleContext?.resourceManager.getStringValue(labelId) as string;
    } catch (error) {
      console.error(TAG, `getMediaBase64 error:${JSON.stringify(error)}`);
    }
    return appName;
  }

  onDestroy(): void {
    console.info(TAG, 'onDestroy.');
    if (win !== undefined) {
      win.destroyWindow();
    }
  }

  private async createWindow(name: string, windowType: window.WindowType, rect: window.Rect): Promise<void> {
    let deviceTypeInfo = deviceInfo.deviceType;
    console.info(TAG, 'create window');
    try {
      if (GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext') !== undefined) {
        let configuration: window.Configuration = {
          name: name,
          windowType: windowType,
          ctx: GlobalThis.getInstance().getContext('jumpInterceptorExtensionContext') as common.BaseContext,
        };
        win = await window.createWindow(configuration);
        if (deviceTypeInfo !== 'default') {
          await win.hideNonSystemFloatingWindows(true);
        }
        await win.moveWindowTo(rect.left, rect.top);
        await win.resize(rect.width, rect.height);
        await win.setUIContent('pages/jumpInterceptorDialog');
        await win.setWindowBackgroundColor('#00000000');
        await win.showWindow();
      }
    } catch {
      console.error(TAG, 'window create failed!');
    }
  }
};
