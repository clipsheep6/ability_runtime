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

import abilityManager from '@ohos.app.ability.abilityManager';
import type { BusinessError } from '@ohos.base';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import wantConstant from '@ohos.app.ability.wantConstant';
import type Want from '@ohos.app.ability.Want';
import type UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';

const TAG = 'AssertFaultDialog_UIExtension';
const TEXT_DETAIL = 'assertFaultDialogDetail';

export default class UiExtAbility extends UIExtensionAbility {
  storage: LocalStorage;
  sessionId: string;

  onCreate(): void {
    console.info(TAG, 'onCreate');
  }

  onForeground(): void {
    console.info(TAG, 'onForeground');
  }

  onBackground(): void {
    console.info(TAG, 'onBackground');
  }

  onSessionCreate(want: Want, session: UIExtensionContentSession): void {
    this.sessionId = want.parameters[wantConstant.Params.ASSERT_FAULT_SESSION_ID] as string,
    this.storage = new LocalStorage(
      {
        'session': session,
        'sessionId' : this.sessionId,
        'textDetail' : want.parameters[TEXT_DETAIL]
      });
    session.loadContent('pages/assertFaultDialog', this.storage);
    session.setWindowBackgroundColor('#00000000');
  }

  onDestroy(): void {
    console.info(TAG, 'onDestroy');
  }

  onSessionDestroy(session: UIExtensionContentSession): void {
    console.info(TAG, 'onSessionDestroy');
    console.info(TAG, `isUserAction: ${AppStorage.get('isUserAction')}`);
    let isUserAction = AppStorage.get<boolean>('isUserAction');
    if (isUserAction === undefined) {
      let status = abilityManager.UserStatus.ASSERT_TERMINATE;
      try {
        abilityManager.notifyDebugAssertResult(this.sessionId, status).then(() => {
          console.log(TAG, 'notifyDebugAssertResult success.');
        }).catch((err: BusinessError) => {
          console.error(TAG, `notifyDebugAssertResult failed, error: ${JSON.stringify(err)}`);
        });
      } catch (error) {
        console.error(TAG, `try notifyDebugAssertResult failed, error: ${JSON.stringify(error)}`);
      }
    }
  }
};
