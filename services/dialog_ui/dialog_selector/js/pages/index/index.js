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
        selectorHapList: [],
        selector: {
            backIcon: "common/app_icon.png",
            swiper: {
                height: "100vp",
                indicator: false,
                contentHeight: "100vp",
                gridColumns: "1fr 1fr",
                gridRows: "1fr",
            },
            btn: {
                marginTop: "20vp",
            }
        },
        lineNums: 8,
        pcSelectorlist: {},
        pcDisplay: 'none',
        phoneDisplay: 'none',
        labeloffsetX: 0,
        labeloffsetY: 0,
        labelwidth: 320,
        labelheight: 350,
    },
    onInit() {
        console.log("dialog service selector onInit");
        this.labeloffsetX = parseInt(this.offsetX);
        this.labeloffsetY = parseInt(this.offsetY);
        this.labelwidth = parseInt(this.width);
        this.labelheight = parseInt(this.height);
        if (this.deviceType === "phone") {
            this.phoneDisplay = 'flex';
            this.pcDisplay = 'none';
            this.initPhoneData();
        } else if (this.deviceType === "pc") {
            this.phoneDisplay = 'none';
            this.pcDisplay = 'flex';
            this.initPcData();
        }

        this.getHapResource();
    },
    initPhoneData() {
        this.getHapListStyle();

        var selectorHap = [];
        for (let i = 0; i < this.hapList.length; i++) {
            selectorHap.push(this.hapList[i]);
            if (i % this.lineNums == this.lineNums - 1 || i == this.hapList.length - 1) {
                this.selectorHapList.push(selectorHap);
                selectorHap = [];
            }
        }

        console.log("dialog service this.lineNums:" + this.lineNums);
        console.log("dialog service selectorHapList.length:" + this.selectorHapList.length);
    },
    initPcData() {
        this.selectorHapList = [];
        for (let i = 0; i < this.hapList.length; i++) {
            this.selectorHapList.push(this.hapList[i]);
            this.selectorHapList[i].name = this.hapList[i].bundle;
            this.selectorHapList[i].icon = "common/app_icon.png";
            console.log("dialog service onInit bundle " + this.hapList[i].bundle);
        }

        this.getHapListStyle();
    },
    getHapListStyle() {
        if (this.deviceType == "phone") {
            if (this.hapList.length > 8) {
                this.selector.swiper.height = "220vp";
                this.selector.swiper.indicator = true;
                this.selector.swiper.contentHeight = "200vp";
                this.selector.swiper.gridColumns = "1fr 1fr 1fr 1fr";
                this.selector.swiper.gridRows = "1fr 1fr";
                this.selector.btn.marginTop = "10vp";
            } else if (this.hapList.length > 3) {
                this.selector.swiper.height = "200vp";
                this.selector.swiper.contentHeight = "200vp";
                this.selector.swiper.gridColumns = "1fr 1fr 1fr 1fr";
                this.selector.swiper.gridRows = "1fr 1fr";
            } else if (this.hapList.length > 2) {
                this.selector.swiper.gridColumns = "1fr 1fr 1fr";
            } else {
                ;
            }
        } else if (this.deviceType == "pc") {
            let heightTotalVp = 1;
            let heightVal = 70;
            let scrollbar = "off"
            if (this.selectorHapList.length == 2) {
                heightTotalVp = this.selectorHapList.length * heightVal;
            } else if (this.selectorHapList.length == 3) {
                heightTotalVp = this.selectorHapList.length * heightVal;
            } else if (this.selectorHapList.length == 4) {
                heightTotalVp = this.selectorHapList.length * heightVal;
            } else if (this.selectorHapList.length > 4) {
                heightTotalVp = 4 * heightVal + 36;
                scrollbar = "auto";
            } else {
                ;
            }

            this.pcSelectorlist = {
                width: "100%",
                height: heightTotalVp + "vp",
                scrollbar:scrollbar
            };
        }
    },
    onSelector: function (item) {
        let param = item.bundle + ";" + item.ability;
        console.log("dialog service selector to :" + item.ability);
        callNativeHandler('EVENT_CHOOSE_APP', param);
    },
    onCancel: function () {
        console.log("dialog service close");
        callNativeHandler('EVENT_CLOSE', "");
    },
    getHapResource() {
        console.log("dialog service hapList.length:" + this.hapList.length);
        for (let i = 0; i < this.hapList.length; i++) {
            let lableId = Number(this.hapList[i].name);
            console.log("dialog service lableId:" + lableId + "bundle:" + this.hapList[i].bundle);
            resourceManager.getResourceManager(this.hapList[i].bundle).then(mgr =>{
                console.log("dialog service bundle:" + this.hapList[i].bundle + "---lableId:" + lableId);
                mgr.getString(lableId).then(value => {
                    console.log("dialog service get label(" + lableId + ") value:" + value);
                    this.updateHapName(this.hapList[i].ability, this.hapList[i].bundle, value);
                }).catch(error => {
                    console.log("dialog service resource getString error:" + error);
                    this.updateHapName(this.hapList[i].ability, this.hapList[i].bundle, this.hapList[i].bundle);
                })
            }).catch(error => {
                console.log("dialog service getResourceManager error:" + error);
                this.updateHapName(this.hapList[i].ability, this.hapList[i].bundle);
            });

            let iconId = Number(this.hapList[i].icon);
            resourceManager.getResourceManager(this.hapList[i].bundle).then(mgr =>{
                console.log("dialog service bundle:" + this.hapList[i].bundle + "---iconId:" + iconId);
                mgr.getMediaBase64(iconId).then(value => {
                    console.log("dialog service get icon(" + iconId + ") value:" + value);
                    this.updateHapIcon(this.hapList[i].ability, this.hapList[i].bundle, value);
                }).catch(error => {
                    console.log("dialog service resource getString error:" + error);
                    this.updateHapIcon(this.hapList[i].ability, this.hapList[i].bundle, this.selector.backIcon);
                })
            }).catch(error => {
                console.log("dialog service getResourceManager error:" + error);
                this.updateHapIcon(this.hapList[i].ability, this.hapList[i].bundle, this.selector.backIcon);
            });
        }
    },
    updateHapName(ability, bundle, hapLabel) {
        for (let i = 0; i < this.selectorHapList.length; i++) {
            if (this.deviceType == "phone") {
                if (this.selectorHapList[i] != null) {
                    for (let j = 0; j < this.selectorHapList[i].length; j++) {
                        if (this.selectorHapList[i][j].ability == ability && this.selectorHapList[i][j].bundle == bundle) {
                            this.selectorHapList[i][j].name = hapLabel;
                            console.log("dialog service update ability:" + ability + " bundle:" + bundle + " to lable:" + hapLabel);
                        }
                    }
                }
            } else if (this.deviceType == "pc") {
                if (this.selectorHapList[i] == undefined) {
                    return;
                }
    
                if (this.selectorHapList[i].ability == ability && this.selectorHapList[i][j].bundle == bundle) {
                    this.selectorHapList[i].name = hapLabel;
                    console.log("dialog service update ability:" + ability + " bundle:" + bundle + " to lable:" + hapLabel);
                }
            }
        }
    },
    updateHapIcon(ability, bundle, hapIcon) {
        for (let i = 0; i < this.selectorHapList.length; i++) {
            if (this.deviceType == "phone") {
                if (this.selectorHapList[i] != null) {
                    for (let j = 0; j < this.selectorHapList[i].length; j++) {
                        if (this.selectorHapList[i][j].ability == ability && this.selectorHapList[i][j].bundle == bundle) {
                            this.selectorHapList[i][j].icon = hapIcon;
                            console.log("dialog service update ability:" + ability + " bundle:" + bundle + " to icon:" + hapIcon);
                        }
                    }
                }
            } else if (this.deviceType == "pc") {
                if (this.selectorHapList[i] == undefined) {
                    return;
                }
    
                if (this.selectorHapList[i].ability == ability && this.selectorHapList[i][j].bundle == bundle) {
                    this.selectorHapList[i].icon = hapIcon;
                    console.log("dialog service update ability:" + ability + " bundle:" + bundle + " to icon:" + hapIcon);
                }
            }
        }
    }
}
