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
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility'
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession'
import Want from '@ohos.app.ability.Want'

const TAG = 'SelectorExtensionAbility';

export default class SelectorExtensionAbility extends UIExtensionAbility {
    onCreate() {
        console.info(TAG, 'SelectorExtensionAbility onCreate');
    }

    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        const storage: LocalStorage = new LocalStorage({
            "session": session,
            "extensionAbility": this,
            "callerWant": want
        } as Record<string, object>);

        session.loadContent("pages/PhonePage", storage);
        try {
            const bgColor: string = '#40FFFFFF';
        } catch (e) {
            console.info(TAG, `Set window background color fail, ${JSON.stringify(e)}`);
        }
    }

    onSessionDestroy(session: UIExtensionContentSession) {
        console.info(TAG, 'SelectorExtensionAbility onSessionDestroy');
    }

    onForeground() {
        console.info(TAG, 'SelectorExtensionAbility onForeground');
    }

    onBackground() {
        console.info(TAG, 'SelectorExtensionAbility onBackground');
    }

    onDestroy() {
        console.info(TAG, 'SelectorExtensionAbility onDestroy');
    }
}