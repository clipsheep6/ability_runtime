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
import {
    TargetInfo,
    TargetType,
    UIAbilityActionExecution,
    UIExtensionActionFunction,
    UIExtensionInfo
} from '../data/Model'
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import { AbilityInfoUtils } from '../Utils/AbilityInfoUtils';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import { LocalStorageKeyConstants } from '../utils/Constants'
import { AbilityMetaQuerier } from '../data/AbilityMetaQuerier';

const TAG = 'PhotoEditExtensionAbility.TargetComponent';

@Entry
@Component
export struct TargetComponent {
    @Consume level2Page: boolean;
    @Consume openUIExtension: boolean;
    @Consume uiExtensionInfo: UIExtensionInfo | null;
    @StorageProp('currentColorMode') currentColorMode: number = ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT;
    @StorageProp('currentLanguage') @Watch('refreshLabel') currentLang: string = '';
    @State targetIcon: DrawableDescriptor | ResourceStr | PixelMap | null = null;
    @State targetLabel: ResourceStr = "";
    private targetInfo?: TargetInfo;
    private storage: LocalStorage | null = null;

    aboutToAppear() {
        this.getIconAndLabel();
    }

    refreshLabel() {
        if (this.targetInfo && this.targetInfo.filterAbilityInfo) {
            try {
                AbilityInfoUtils.getAbilityLabelAndIcon(this.storage?.get<UIExtensionAbility>(LocalStorageKeyConstants.EXTENSION_ABILITY)?.context, {
                    bundleName: this.targetInfo.filterAbilityInfo.bundleName,
                    moduleName: this.targetInfo.filterAbilityInfo.moduleName,
                    abilityName: this.targetInfo.filterAbilityInfo.name,
                    abilityIconId: this.targetInfo.filterAbilityInfo.iconId,
                    abilityLabelId: this.targetInfo.filterAbilityInfo.labelId
                }, (labelAndIcon: Object[]) => {
                    if (labelAndIcon) {
                        this.targetIcon = labelAndIcon[0] as DrawableDescriptor;
                        this.targetLabel = labelAndIcon[1] as string;
                    }
                })
            } catch (e) {
                console.error(TAG, `Get service label and icon exception: ${e}`)
            }
        } else if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
            this.targetLabel = 'All';
            this.targetIcon = $r('app.media.ic_public_more');
        }
    }

    getIconAndLabel() {
        if (this.targetInfo && this.targetInfo.filterAbilityInfo) {
            let iconAndLabelContent = AbilityMetaQuerier.getIconAndLabelByIds({
                iconId: this.targetInfo.filterAbilityInfo.iconId,
                labelId: this.targetInfo.filterAbilityInfo.labelId
            });
            if (iconAndLabelContent) {
                this.targetIcon = iconAndLabelContent.icon;
                this.targetLabel = iconAndLabelContent.label;
                return;
            }
            try {
                AbilityInfoUtils.getAbilityLabelAndIcon(this.storage?.get<UIExtensionAbility>(LocalStorageKeyConstants.EXTENSION_ABILITY)?.context,
                    {
                        bundleName: this.targetInfo.filterAbilityInfo.bundleName,
                        moduleName: this.targetInfo.filterAbilityInfo.moduleName,
                        abilityName: this.targetInfo.filterAbilityInfo.name,
                        abilityIconId: this.targetInfo.filterAbilityInfo.iconId,
                        abilityLabelId: this.targetInfo.filterAbilityInfo.labelId
                    }, (labelAndIcon: Object[]) => {
                        if (labelAndIcon) {
                            this.targetIcon = labelAndIcon[0] as DrawableDescriptor;
                            this.targetLabel = labelAndIcon[1] as string;
                        }
                    })
            } catch (e) {
                console.error(TAG, `Get label and icon exception: ${JSON.stringify(e)}`)
            }
        } else if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
            this.targetLabel = 'All';
            this.targetIcon = $r('app.media.ic_public_more');
        }
    }

    build() {
        Column() {
            Button({ type: ButtonType.Normal, stateEffect: true }) {
                Stack() {
                    Column() {
                        Stack() {
                            Image(this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET ? "" : this.targetIcon)
                                .alt($r('sys.color.ohos_id_color_component_normal'))
                                .objectFit(ImageFit.Auto)
                                .backgroundColor(this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET || this.targetIcon
                                    ? $r('sys.color.ohos_id_color_card_bg') : $r('sys.color.ohos_id_color_component_normal'))
                                .height(48)
                                .width(48)
                                .borderRadius(10)
                                .draggable(false)
                                .margin({
                                    left: 16,
                                    right: 16
                                })

                            if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
                                Image(this.currentColorMode === ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT ?
                                $r('app.media.ic_public_more') : $r('app.media.ic_public_more_dark'))
                                    .objectFit(ImageFit.Fill)
                                    .height(24)
                                    .width(24)
                                    .margin({ top: 12 })
                                    .draggable(false)
                            }
                        }
                        .alignContent(Alignment.Top)

                        Text(this.targetLabel)
                            .fontSize($r('sys.float.ohos_id_text_size_body3'))
                            .fontColor($r('sys.color.ohos_id_color_text_primary'))
                            .textAlign(TextAlign.Center)
                            .align(Alignment.Top)
                            .fontWeight(FontWeight.Regular)
                            .width(64)
                            .maxLines(2)
                            .textOverflow({ overflow: TextOverflow.Ellipsis })
                            .margin({ top: 8 })
                    }
                }
            }
            .align(Alignment.Top)
            .constraintSize({ minHeight: 104 })
            .padding({
                top: 8,
                bottom: 8
            })
            .borderRadius($r('sys.float.ohos_id_corner_radius_clicked'))
            .backgroundColor(Color.Transparent)
            .onClick(() => {
                if (this.targetInfo?.targetAction?.type === TargetType.SHOW_ALL_TARGET) {
                    this.level2Page = true;
                    return;
                }
                if (!this.storage) {
                    return;
                }
                if (this.targetInfo?.targetAction?.type === TargetType.UI_EXTENSION) {
                    this.uiExtensionInfo =
                    (this.targetInfo.targetAction.actionExecution as UIExtensionActionFunction)(this.storage);
                    this.openUIExtension = true;
                    return;
                }
                if (this.targetInfo?.targetAction?.type === TargetType.UI_ABILITY ||
                this.targetInfo?.targetAction?.type === TargetType.DEEPLINK) {
                    (this.targetInfo.targetAction.actionExecution as UIAbilityActionExecution)(this.storage);
                    return;
                }
            })
            .stateStyles({
                pressed: {
                    .backgroundColor($r('sys.color.ohos_id_color_click_effect'))
                }
            })
        }
        .justifyContent(FlexAlign.Start)
    }
}