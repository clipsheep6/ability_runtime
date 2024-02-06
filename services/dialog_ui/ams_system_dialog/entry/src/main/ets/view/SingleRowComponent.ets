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
import { TargetInfo } from '../data/Model';
import { TargetComponent } from './TargetComponent';

const TAG = 'SelectorExtensionAbility.SingleRowComponent';

@Preview
@Component
export struct SingleRowComponent {
    private targetInfoArray: Array<TargetInfo> = [];
    private storage?: LocalStorage;
    @Consume singleRowHeight: number;

    build() {
        Row() {
            ForEach(this.targetInfoArray, (item: TargetInfo, index?: number) => {
                TargetComponent({ targetInfo: item, storage: this.storage })
                    .visibility(item.visible ? Visibility.Visible : Visibility.Hidden)
            })
        }
        .width('100%')
        .alignItems(VerticalAlign.Top)
        .justifyContent(FlexAlign.SpaceBetween)
        .onAreaChange((oldValue: Area, newValue: Area) => {
            if (oldValue.height !== newValue.height) {
                this.singleRowHeight = Number(newValue.height);
            }
        })
    }
}