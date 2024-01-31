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
import HashMap from '@ohos.util.HashMap';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import common from '@ohos.app.ability.common';
import { AbilityInfoParam, IconAndLabelContent } from './Model';
import { AbilityInfoUtils } from '../utils/AbilityInfoUtils';
import taskpool from '@ohos.taskpool';

const TAG = 'PhotoEditExtensionAbility.AbilityMetaQuerier';

export namespace AbilityMetaQuerier {
    export interface IconAndLabelId {
        iconId: number;
        labelId: number;
    }

    export const ICON_LABEL_CONTENT_MAP: HashMap<String, IconAndLabelContent> = new HashMap();

    export const HANDLE_LIMIT_PER_TASK: number = 20;

    @Concurrent
    function queryIconAndLabel(context: common.UIExtensionContext,
                               abilityInfoParams: Array<AbilityInfoParam>): Map<string, IconAndLabelContent> {
        const TAG = 'PhotoEditExtensionAbility.AbilityMetaQuerier';
        let iconLabelContextHash: Map<string, IconAndLabelContent> = new Map();
        abilityInfoParams.map(abilityInfoParam => {
            let keyId: string = abilityInfoParam.abilityIconId + "##" +
            abilityInfoParam.abilityLabelId;
            try {
                AbilityInfoUtils.getAbilityLabelAndIconSync(context,
                    {
                        bundleName: abilityInfoParam.bundleName,
                        moduleName: abilityInfoParam.moduleName,
                        abilityName: abilityInfoParam.abilityName || undefined,
                        abilityIconId: abilityInfoParam.abilityIconId,
                        abilityLabelId: abilityInfoParam.abilityLabelId
                    }, (labelAndIcon: Object[] | null) => {
                        if (labelAndIcon && labelAndIcon.length > 1) {
                            let targetIcon = labelAndIcon[0] as DrawableDescriptor;
                            let targetLabel = labelAndIcon[1] as string;
                            iconLabelContextHash.set(keyId, { icon: targetIcon.getPixelMap(), label: targetLabel });
                        }
                    })
            } catch (e) {
                console.error(TAG, `Get atomic service label and icon failed! exception: ${JSON.stringify(e)}`)
            }
        })
        return iconLabelContextHash;
    }

    export function preloadIconAndLabel(uiExtensionAbility: UIExtensionAbility,
                                        abilityInfoParams: Array<AbilityInfoParam>, finish: () => void): void {
        let quotient: number = Math.floor(abilityInfoParams.length / HANDLE_LIMIT_PER_TASK);
        let taskGroup: taskpool.TaskGroup = new taskpool.TaskGroup();
        if (quotient === 0) {
            taskGroup.addTask(queryIconAndLabel, uiExtensionAbility?.context, abilityInfoParams);
            taskpool.execute(taskGroup).then((iconLabelContextHashs: Array<Map<string, IconAndLabelContent>>) => {
                iconLabelContextHashs.map(iconLabelContextHash => {
                    iconLabelContextHash.forEach((value, key, map) => {
                        ICON_LABEL_CONTENT_MAP.set(key, value);
                    })
                })
                finish();
            })
            return;
        }
        for (let i = 0; i < quotient; i++) {
            let abilityInfoParamsForTask: Array<AbilityInfoParam>;
            if (i === quotient - 1) {
                abilityInfoParamsForTask = abilityInfoParams.slice(i * HANDLE_LIMIT_PER_TASK, abilityInfoParams.length);
            } else {
                abilityInfoParamsForTask = abilityInfoParams.slice(i * HANDLE_LIMIT_PER_TASK, (i + 1) * HANDLE_LIMIT_PER_TASK);
            }
            taskGroup.addTask(queryIconAndLabel, uiExtensionAbility.context, abilityInfoParamsForTask);
        }
        taskpool.execute(taskGroup).then((iconAndLabelHashs: Array<Map<string, IconAndLabelContent>>) => {
            iconAndLabelHashs.map(iconLabelContextHash => {
                iconLabelContextHash.forEach((value, key, map) => {
                    ICON_LABEL_CONTENT_MAP.set(key, value);
                })
            })
            finish();
        })
    }

    export function getIconAndLabelByIds(iconAndLabelId: IconAndLabelId): IconAndLabelContent | null {
        let keyId: string = iconAndLabelId.iconId + '##' + iconAndLabelId.labelId;
        let iconAndLabelContent: IconAndLabelContent = ICON_LABEL_CONTENT_MAP.get(keyId);
        if (iconAndLabelContent) {
            return iconAndLabelContent;
        }
        return null;
    }
}