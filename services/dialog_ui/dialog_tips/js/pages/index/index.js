/*
    Copyright (c) 2022 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

import resourceManager from '@ohos.resourceManager';

export default {
    data: {
        btn: {
            color: "#FFFFFF",
        },
        labeloffsetX: 16,
        labeloffsetY: 190,
        labelwidth: 328,
        labelheight: 192,
        pcDisplay: 'flex',
        phoneDisplay: 'none',
    },
    onInit() {
        console.info('onInit');
        this.labeloffsetX = parseInt(this.offsetX);
        this.labeloffsetY = parseInt(this.offsetY);
        this.labelwidth = parseInt(this.width);
        this.labelheight = parseInt(this.height);
        if (this.deviceType === "phone") {
            this.phoneDisplay = 'flex';
            this.pcDisplay = 'none';
        } else if (this.deviceType === "pc") {
            this.btn.color = "#F2F2F2";
            this.phoneDisplay = 'none';
            this.pcDisplay = 'flex';
        }
    },
    onShow() {
        console.info('onshow');
    },
    onCloseApp() {
        console.info('click close app');
        callNativeHandler('EVENT_CLOSE', '');
    }
}