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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';

const TAG = 'AssertFaultDialog_UIExtension';

export default class UiExtAbility extends UIExtensionAbility {
  storage: LocalStorage;
  message: string;
  onCreate() {
    console.info(TAG, 'onCreate');
  }

  onForeground() {
    console.info(TAG, 'onForeground');
  }

  onBackground() {
    console.info(TAG, 'onBackground');
  }

  onSessionCreate(want, session) {
    console.info(TAG, `onSessionCreate, want: ${JSON.stringify(want)},`, `session: ${session}`);
    AppStorage.setOrCreate('sessionId', want.parameters.REQUEST_ASSERT_FAULT_ID);
    this.storage = new LocalStorage(
      {
        'session': session,
      });
    session.loadContent('pages/assertFaultDialog', this.storage);
  }

  onDestroy() {
    console.info(TAG, 'onDestroy');
  }

  onSessionDestroy(session) {
    console.info(TAG, 'onSessionDestroy');
  }
};
