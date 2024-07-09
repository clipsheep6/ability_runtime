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

let ExtensionContext = requireNapi('application.ExtensionContext');
let Caller = requireNapi('application.Caller');

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor() {
    super(ERROR_MSG_INVALID_PARAM);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class UIServiceExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  startAbility(want, options, callback) {
    console.log('startAbility');
    return this.__context_impl__.startAbility(want, options, callback);
  }

  terminateSelf(callback) {
    console.log('terminateSelf');
    return this.__context_impl__.terminateSelf(callback);
  }

  startAbilityByType(type, wantParam, abilityStartCallback, callback){
    console.log('startAbilityByType');
    return this.__context_impl__.startAbilityByType(type, wantParam, abilityStartCallback, callback);
  }

}

export default UIServiceExtensionContext;
