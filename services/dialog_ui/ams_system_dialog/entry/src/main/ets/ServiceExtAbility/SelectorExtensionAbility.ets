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
import systemparameter from '@ohos.systemParameterEnhance';
import dataPreferences from '@ohos.data.preferences';

const TAG = 'SelectorExtensionAbility';

export default class SelectorExtensionAbility extends UIExtensionAbility {
    onCreate() {
        console.info(TAG, 'SelectorExtensionAbility onCreate');
        globalThis.currentExtensionContext = this.context;
        let options = {name:'dialogStore'};
        globalThis.preferences = dataPreferences.getPreferencesSync(this.context, options);
    }

    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        const storage: LocalStorage = new LocalStorage({
            "session": session,
            "extensionAbility": this,
            "callerWant": want
        } as Record<string, object>);
        globalThis.ExtensionType = 'UIExtension';
        if (systemparameter.getSync('persist.sys.abilityms.isdialogconfirmpermission', 'false') === 'false' &&
            globalThis.preferences.getSync('isdialogconfirmpermission', 'false') === 'false') {
            globalThis.currentURL = 'pages/PhonePage';
            session.loadContent("pages/permissionConfirmDialog", storage);
        } else {
            session.loadContent("pages/PhonePage", storage);
        }
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