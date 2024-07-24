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

let ExtensionContext = requireNapi('application.ExtensionContext');

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_MSG_INVALID_PARAM = 'Invalid input parameter.';
class ParamError extends Error {
  constructor() {
    super(ERROR_MSG_INVALID_PARAM);
    this.code = ERROR_CODE_INVALID_PARAM;
  }
}

class UIExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  startAbility(want, options, callback) {
    console.log('startAbility');
    return this.__context_impl__.startAbility(want, options, callback);
  }

  startUIServiceExtensionAbility(want, callback) {
    console.log('startUIServiceExtensionAbility');
    return this.__context_impl__.startUIServiceExtensionAbility(want, callback);
  }

  connectUIServiceExtensionAbility(want, callback) {
    return this.__context_impl__.connectUIServiceExtensionAbility(want, callback);
  }

  disconnectUIServiceExtensionAbility(proxy) {
    return this.__context_impl__.disconnectUIServiceExtensionAbility(proxy);
  }

  openLink(link, options, callback) {
    console.log('openLink');
    return this.__context_impl__.openLink(link, options, callback);
  }

  terminateSelf(callback) {
    console.log('terminateSelf');
    return this.__context_impl__.terminateSelf(callback);
  }

  startAbilityForResult(want, options, callback) {
    console.log('startAbilityForResult');
    return this.__context_impl__.startAbilityForResult(want, options, callback);
  }

  startAbilityForResultAsCaller(want, options, callback) {
    console.log('startAbilityForResultAsCaller');
    return this.__context_impl__.startAbilityForResultAsCaller(want, options, callback);
  }

  terminateSelfWithResult(abilityResult, callback) {
    console.log('terminateSelfWithResult');
    return this.__context_impl__.terminateSelfWithResult(abilityResult, callback);
  }

  connectServiceExtensionAbility(want, options) {
    return this.__context_impl__.connectServiceExtensionAbility(want, options);
  }

  disconnectServiceExtensionAbility(connection, callback) {
    return this.__context_impl__.disconnectServiceExtensionAbility(connection, callback);
  }

  reportDrawnCompleted(callback) {
    return this.__context_impl__.reportDrawnCompleted(callback);
  }

  openAtomicService(appId, options, callback) {
    return this.__context_impl__.openAtomicService(appId, options, callback);
  }
}

export default UIExtensionContext;
