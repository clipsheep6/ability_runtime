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

import common from '@ohos.app.ability.common';
import extension from '@ohos.app.ability.ServiceExtensionAbility';
import want from '@ohos.app.ability.Want';
import window from '@ohos.window';
import display from '@ohos.display';
import { GlobalThis } from '../utils/GlobalThis';
import PositionUtils, { Position } from '../utils/PositionUtils';

const TAG = 'TipsDialog_Service';

let winNum = 1;
let win: window.Window;

export default class TipsServiceExtensionAbility extends extension {
  onCreate(want: want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setContext('tipsExtensionContext', this.context);
  }

  onRequest(want: want, startId: number) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    GlobalThis.getInstance().setObject('abilityWant', want);
    GlobalThis.getInstance().setProperty('params', JSON.parse(want.parameters?.params as string));
    GlobalThis.getInstance().setObject('position', PositionUtils.getTipsDialogPosition());
    GlobalThis.getInstance().setObject('callerToken', want.parameters?.callerToken);

    try {
      display.on('change', (data: number) => {
        let position = PositionUtils.getTipsDialogPosition();
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

    let result = display.getDefaultDisplaySync();
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
    let windowType = (typeof(GlobalThis.getInstance().getProperty('callerToken')) === 'object' && GlobalThis.getInstance().getProperty('callerToken') !== null) ?
    window.WindowType.TYPE_DIALOG : window.WindowType.TYPE_SYSTEM_ALERT;
    this.createWindow('TipsDialog' + startId, windowType, navigationBarRect);
    winNum++;
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
    if (win !== undefined) {
      win.destroyWindow();
    }
  }

  private async createWindow(name: string, windowType: window.WindowType, rect: window.Rect) {
    console.info(TAG, 'create window');
    try {
      let configuration: window.Configuration = {
        name: name,
        windowType: windowType,
        ctx: GlobalThis.getInstance().getContext('tipsExtensionContext') as common.BaseContext,
      };
      win = await window.createWindow(configuration);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget((GlobalThis.getInstance().getProperty('callerToken') as Record<string, Object>)?.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            GlobalThis.getInstance().getContext('tipsExtensionContext')?.terminateSelf();
          }
        });
      }
      await win.hideNonSystemFloatingWindows(true);
      await win.moveWindowTo(rect.left, rect.top);
      await win.resize(rect.width, rect.height);
      await win.setUIContent('pages/tipsDialog');
      await win.setWindowBackgroundColor('#00000000');
      await win.showWindow();
    } catch {
      console.error(TAG, 'window create failed!');
    }
  }
};
