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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';

const TAG = 'SwitchUserDialog_Service';
const LEFT_COEFFICIENT = 0.1;
const HELF = 2;
const WIDTH_COEFFICIENT = 0.9;
const HEIGHT = 64;
const marginBotton = 20;

let winNum = 1;
let win;

export default class SwitchUserServiceExtensionAbility extends extension {
  onCreate(want) {
    console.info(TAG, 'onCreate, want: ' + JSON.stringify(want));
  }

  onRequest(want, startId) {
    console.info(TAG, 'onRequest, want: ' + JSON.stringify(want));
    globalThis.abilityWant = want;

    display.getDefaultDisplay().then(dis => {
      let navigationBarRect = this.getDialogSize(dis);
      if (winNum > 1) {
        win.destroy();
        winNum--;
      }
      this.createWindow('SwitchUserDialog' + startId, window.WindowType.TYPE_SCREENSHOT, navigationBarRect);
      winNum++;
    });
  }

  onDestroy() {
    console.info(TAG, 'onDestroy.');
  }

  private getDialogSize(dis: display.Display): Object {
    let height = dis.height;
    let width = dis.width;
    let densityPixels = dis.densityPixels;
    return {
      left: width * LEFT_COEFFICIENT / HELF,
      top: height - HEIGHT * densityPixels - marginBotton,
      width: width * WIDTH_COEFFICIENT,
      height: HEIGHT * densityPixels
    };
  }

  private async createWindow(name: string, windowType: number, rect) :Promise<void> {
    console.info(TAG, 'create window rect is ' + JSON.stringify(rect));
    try {
      win = await window.create(this.context, name, windowType);
      await win.hideNonSystemFloatingWindows(true);
      await win.moveWindowTo(rect.left, rect.top);
      await win.resize(rect.width, rect.height);
      await win.loadContent('pages/switchUserDialog');
      await win.setWindowBackgroundColor('#00000000');
      await win.showWindow();
    } catch (err) {
      console.error(TAG, `window create failed! ${err}`);
    }
  }
};
