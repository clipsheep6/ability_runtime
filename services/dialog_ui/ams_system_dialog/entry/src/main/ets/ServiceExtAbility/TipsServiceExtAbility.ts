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
import deviceInfo from '@ohos.deviceInfo';

const TAG = "TipsDialog_Service";

var winNum = 1;
var win;

export default class TipsServiceExtensionAbility extends extension {
    onCreate(want) {
        console.debug(TAG, "onCreate, want: " + JSON.stringify(want));
        globalThis.tipsExtensionContext = this.context;
    }

    onRequest(want, startId) {
        globalThis.abilityWant = want;
        globalThis.params = JSON.parse(want["parameters"]["params"]);
        globalThis.position = JSON.parse(want["parameters"]["position"]);

        display.getDefaultDisplay().then(dis => {
            let navigationBarRect = {
                left: globalThis.position.offsetX,
                top: globalThis.position.offsetY,
                width: globalThis.position.width,
                height: globalThis.position.height
            }
            if (winNum > 1) {
                win.destroy();
                winNum--;
            }
            if (deviceInfo.deviceType == "phone") {
                this.createWindow("TipsDialog" + startId, window.WindowType.TYPE_SYSTEM_ALERT, navigationBarRect);
            } else {
                this.createWindow("TipsDialog" + startId, window.WindowType.TYPE_FLOAT, navigationBarRect);
            }
            winNum++;
        })
    }

    onDestroy() {
        console.info(TAG, "onDestroy.");
    }

    private async createWindow(name: string, windowType: number, rect) {
        console.info(TAG, "create window");
        try {
            win = await window.create(globalThis.tipsExtensionContext, name, windowType);
            await win.moveTo(rect.left, rect.top);
            await win.resetSize(rect.width, rect.height);
            await win.loadContent('pages/tipsDialog');
            await win.setBackgroundColor("#00000000");
            await win.show();
        } catch {
            console.error(TAG, "window create failed!");
        }
    }
};
