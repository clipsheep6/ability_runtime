/*
 *
 *  * Copyright (c) 2024 Huawei Device Co., Ltd.
 *  * Licensed under the Apache License, Version 2.0 (the "License");
 *  * you may not use this file except in compliance with the License.
 *  * You may obtain a copy of the License at
 *  *
 *  *     http://www.apache.org/licenses/LICENSE-2.0
 *  *
 *  * Unless required by applicable law or agreed to in writing, software
 *  * distributed under the License is distributed on an "AS IS" BASIS,
 *  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  * See the License for the specific language governing permissions and
 *  * limitations under the License.
 *
 */
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import { LocalStorageKeyConstants } from '../utils/Constants';

@Preview
@CustomDialog
export struct TipsDialogComponent {
    private storage?: LocalStorage;
    private controller?: CustomDialogController;

    build() {
        Column() {
            Row() {
                Text('No available applications targets')
                    .fontSize($r('sys.float.ohos_id_text_size_body1'))
                    .fontWeight(FontWeight.Regular)
                    .fontColor($r('sys.color.ohos_id_color_text_primary'))
            }
            .padding({
                left: 24,
                right: 24
            })
            .margin({
                top: 24
            })

            Row() {
                Button({ type: ButtonType.Normal, stateEffect: false }) {
                    Text('Got it')
                        .fontSize($r('sys.float.ohos_id_text_size_button1'))
                        .fontWeight(FontWeight.Medium)
                        .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
                        .maxLines(1)
                }
                .height(40)
                .constraintSize({ minHeight: 40 })
                .backgroundColor(Color.Transparent)
                .onClick(() => {
                    this.controller?.close();
                    const session: UIExtensionContentSession = this.storage?.get(LocalStorageKeyConstants.SESSION) as UIExtensionContentSession;
                    session?.terminateSelf();
                    this.storage?.clear();
                })
            }
            .padding({
                left: 16,
                right: 16
            })
            .margin({
                top: 8,
                bottom: 16
            })
        }
    }
}