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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';
import PositionUtils from '../utils/PositionUtils';

const TAG = 'TipsDialog_Service';

let winNum = 1;
let win;

export default class TipsServiceExtensionAbility extends extension {
  onCreate(want) {
    console.debug(TAG, 'onCreate, want: ' + JSON.stringify(want));
    globalThis.tipsExtensionContext = this.context;
  }

  onRequest(want, startId) {
    console.debug(TAG, 'onRequest, want: ' + JSON.stringify(want));
    try {
      globalThis.params = JSON.parse(want.parameters.params);
      globalThis.callerToken = want.parameters.callerToken;
    } catch (err) {
      console.error(TAG, 'Invalid parameters.');
      this.context.terminateSelf();
      return;
    }
    globalThis.abilityWant = want;
    globalThis.position = PositionUtils.getTipsDialogPosition();

    try {
      display.on('change', (data: number) => {
        let position = PositionUtils.getTipsDialogPosition();
        if (position.offsetX !== globalThis.position.offsetX || position.offsetY !== globalThis.position.offsetY) {
          win.moveTo(position.offsetX, position.offsetY);
        }
        if (position.width !== globalThis.position.width || position.height !== globalThis.position.height) {
          win.resetSize(position.width, position.height);
        }
        globalThis.position = position;
      });
    } catch (exception) {
      console.error('Failed to register callback. Code: ' + JSON.stringify(exception));
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
      let windowType = (typeof(globalThis.callerToken) === 'object' && globalThis.callerToken !== null) ?
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

  private async createWindow(name: string, windowType: number, rect) {
    console.info(TAG, 'create window');
    try {
      win = await window.create(globalThis.tipsExtensionContext, name, windowType);
      if (windowType === window.WindowType.TYPE_DIALOG) {
        await win.bindDialogTarget(globalThis.callerToken.value, () => {
          win.destroyWindow();
          winNum--;
          if (winNum === 0) {
            globalThis.tipsExtensionContext.terminateSelf();
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
