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
import { BusinessError } from '@ohos.base';
import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import wantConstant from '@ohos.app.ability.wantConstant';

const TAG = 'AssertFaultDialog_UIExtension';
const TEXT_DETAIL : string = "File:\n../../third_party/musl/libc-test/src/functionalext/unittest/unit_test_assert_fail.c\nFunction: main\nLine: 23\n\nExpression:\n0\n\n(Press Retry to debug the application)";

export default class UiExtAbility extends UIExtensionAbility {
  storage: LocalStorage;
  message: string;
  sessionId: string;

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
    this.sessionId = want.parameters[wantConstant.Params.ASSERT_FAULT_SESSION_ID],
    console.info(TAG, `onSessionCreate, want: ${JSON.stringify(want)},`, `session: ${session}`);
    this.storage = new LocalStorage(
      {
        'session': session,
        'sessionId' : want.parameters[wantConstant.Params.ASSERT_FAULT_SESSION_ID],
        'textDetail' : TEXT_DETAIL
      });
    session.loadContent('pages/assertFaultDialog', this.storage);
    session.setWindowBackgroundColor('#00000000');
  }

  onDestroy() {
    console.info(TAG, 'onDestroy');
  }

  onSessionDestroy(session) {
    console.info(TAG, 'onSessionDestroy');
    console.info(TAG, `isUserAction: ${AppStorage.get('isUserAction')}`);
    let isUserAction = AppStorage.get<boolean>('isUserAction');
    if(isUserAction !== true || isUserAction === undefined) {
      let status = abilityManager.UserStatus.ASSERT_TERMINATE;
      try {
        abilityManager.notifyUserActionResult(this.sessionId, status).then(() => {
          console.log(TAG, 'NotifyUserActionResult success.');
        }).catch((err: BusinessError) => {
          console.error(TAG, `NotifyUserActionResult failed, error: ${JSON.stringify(err)}`);
        })
      } catch (error) {
        console.error(TAG, `try NotifyUserActionResult failed, error: ${JSON.stringify(error)}`);
      }
    }
  }
};
