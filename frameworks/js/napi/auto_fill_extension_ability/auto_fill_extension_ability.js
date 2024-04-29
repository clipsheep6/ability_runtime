/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

let ExtensionAbility = requireNapi('app.ability.ExtensionAbility');

class AutoFillExtensionAbility extends ExtensionAbility {
  onCreate() {
    console.log('onCreate');
  }

  onFillRequest(session, request, callback) {
    console.log('onFillRequest');
  }

  onSaveRequest(session, request, callback) {
    console.log('onSaveRequest');
  }

  onUpdateRequest(request) {
    console.log('onUpdateRequest');
  }

  onSessionDestroy(session) {
    console.log('onSessionDestroy');
  }

  onForeground() {
    console.log('onForeground');
  }

  onBackground() {
    console.log('onBackground');
  }

  onDestroy() {
    console.log('onDestroy');
  }
}

export default AutoFillExtensionAbility;