/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';

@Preview
@Component
export struct TitleComponent {
    @Consume @Watch('onLevel2PageChanged') level2Page: boolean;
    @Consume isChange: boolean;
    @State level10pacity: number = 0;
    @Consume titleHeight: number;
    @State notDisplayLevel1: boolean = false;
    private storage: LocalStorage | null = null;
    @StorageProp('currentColorMode') currentColorMode: number = ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT;

    onLevel2PageChanged() {
        this.isChange = true;
        animateTo({ duration: 300 }, () => {
            this.notDisplayLevel1 = this.level2Page;
            this.level10pacity = this.level2Page ? 1 : 0;
        })
    }

    onBackPress() {
        if (this.level2Page) {
            this.level2Page = false;
        }
    }

    build() {
        Row() {
            Stack() {
                Row() {
                    Button({ type: ButtonType.Normal }) {
                        Image(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ?
                        $r('app.media.ic_public_back') : $r('app.media.ic_public_back_dark'))
                            .width(24)
                            .height(24)
                            .draggable(false)
                    }
                    .opacity(this.level10pacity)
                    .width(24)
                    .height(24)
                    .backgroundColor($r('sys.color.ohos_id_color_panel_bg'))
                    .onClick((event) => {
                        this.level2Page = false;
                    })

                    Text('All open methods')
                        .fontColor($r('sys.color.ohos_id_color_text_primary'))
                        .fontSize($r('sys.float.ohos_id_text_size_headline8'))
                        .fontWeight(FontWeight.Medium)
                        .maxLines(1)
                        .textOverflow({ overflow: TextOverflow.Ellipsis })
                        .textAlign(TextAlign.Start)
                        .margin({ left: 16 })
                        .opacity(this.level10pacity)
                }

                if (!this.notDisplayLevel1) {
                    Text('Select open mode')
                        .fontColor($r('sys.color.ohos_id_color_text_primary'))
                        .fontSize($r('sys.float.ohos_id_text_size_headline8'))
                        .fontWeight(FontWeight.Medium)
                        .maxLines(1)
                        .textOverflow({ overflow: TextOverflow.Ellipsis })
                        .textAlign(TextAlign.Start)
                        .opacity(1 - this.level10pacity)
                }
            }.alignContent(Alignment.Start)

            Button({ type: ButtonType.Circle }) {
                Image(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ?
                $r('app.media.ic_public_close') : $r('app.media.ic_public_close_dark'))
                    .width(24)
                    .height(24)
                    .objectFit(ImageFit.Fill)
                    .draggable(false)
            }
            .stateStyles({
                pressed: {
                    .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                },
                normal: {
                    .backgroundColor($r('sys.color.ohos_id_color_panel_bg'))
                }
            })
            .width(24)
            .height(24)
            .onClick((event => {
                const session: UIExtensionContentSession = this.storage?.get('session') as UIExtensionContentSession;
                session?.terminateSelf();
                this.storage?.clear();
            }))
        }
        .height(56)
        .constraintSize({ minHeight: 32 })
        .justifyContent(FlexAlign.SpaceBetween)
        .alignItems(VerticalAlign.Center)
        .width('100%')
        .onAreaChange((oldValue: Area, newValue: Area) => {
            if (oldValue.height !== newValue.height) {
                this.titleHeight = Number(newValue.height);
            }
        })
    }
}