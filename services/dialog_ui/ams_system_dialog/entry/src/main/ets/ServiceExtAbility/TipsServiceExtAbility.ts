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
import PositionUtils from '../utils/PositionUtils';

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
    GlobalThis.getInstance().setWant("abilityWant", want);
    GlobalThis.getInstance().setRecord('params', JSON.parse(want.parameters?.params as string));
    GlobalThis.getInstance().setPosition('position', PositionUtils.getTipsDialogPosition());
    GlobalThis.getInstance().setObject("callerToken", want.parameters?.callerToken);

    try {
      display.on('change', (data: number) => {
        let position = PositionUtils.getTipsDialogPosition();
        if (position.offsetX !== GlobalThis.getInstance().getPosition('position')?.offsetX || position.offsetY !== GlobalThis.getInstance().getPosition('position')?.offsetY) {
          win.moveTo(position.offsetX, position.offsetY);
        }
        if (position.width !== GlobalThis.getInstance().getPosition('position')?.width || position.height !== GlobalThis.getInstance().getPosition('position')?.height) {
          win.resetSize(position.width, position.height);
        }
        GlobalThis.getInstance().setPosition('position', position);
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
    }

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
      this.createWindow('TipsDialog' + startId, windowType, navigationBarRect);
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
      win = await window.create(GlobalThis.getInstance().getContext('tipsExtensionContext')  as common.BaseContext, name, windowType);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            GlobalThis.getInstance().getContext('tipsExtensionContext')?.terminateSelf();
          }
        });
      }
      await win.hideNonSystemFloatingWindows(true);
      await win.moveTo(rect.left, rect.top);
      await win.resetSize(rect.width, rect.height);
      await win.loadContent('pages/tipsDialog');
      await win.setBackgroundColor('#00000000');
      await win.show();
    } catch {
      console.error(TAG, 'window create failed!');
    }
  }
};
