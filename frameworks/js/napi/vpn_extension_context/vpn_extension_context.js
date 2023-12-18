/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

class VpnExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  startAbility(want, options, callback) {
    console.log('startAbility');
    return this.__context_impl__.startAbility(want, options, callback);
  }

  startAbilityAsCaller(want, options, callback) {
    console.log('startAbilityAsCaller');
    return this.__context_impl__.startAbilityAsCaller(want, options, callback);
  }

  startRecentAbility(want, options, callback) {
    console.log('startRecentAbility');
    return this.__context_impl__.startRecentAbility(want, options, callback);
  }

  connectAbility(want, options) {
    console.log('connectAbility');
    return this.__context_impl__.connectAbility(want, options);
  }

  connectVpnExtensionAbility(want, options) {
    console.log('connectVpnExtensionAbility');
    return this.__context_impl__.connectVpnExtensionAbility(want, options);
  }

  startAbilityWithAccount(want, accountId, options, callback) {
    console.log('startAbilityWithAccount');
    return this.__context_impl__.startAbilityWithAccount(want, accountId, options, callback);
  }

  startVpnExtensionAbility(want, callback) {
    console.log('startVpnExtensionAbility');
    return this.__context_impl__.startVpnExtensionAbility(want, callback);
  }

  startVpnExtensionAbilityWithAccount(want, accountId, callback) {
    console.log('startVpnExtensionAbilityWithAccount');
    return this.__context_impl__.startVpnExtensionAbilityWithAccount(want, accountId, callback);
  }

  stopVpnExtensionAbility(want, callback) {
    console.log('stopVpnExtensionAbility');
    return this.__context_impl__.stopVpnExtensionAbility(want, callback);
  }

  stopVpnExtensionAbilityWithAccount(want, accountId, callback) {
    console.log('stopVpnExtensionAbilityWithAccount');
    return this.__context_impl__.stopVpnExtensionAbilityWithAccount(want, accountId, callback);
  }

  connectAbilityWithAccount(want, accountId, options) {
    console.log('connectAbilityWithAccount');
    return this.__context_impl__.connectAbilityWithAccount(want, accountId, options);
  }

  connectVpnExtensionAbilityWithAccount(want, accountId, options) {
    console.log('connectVpnExtensionAbilityWithAccount');
    return this.__context_impl__.connectVpnExtensionAbilityWithAccount(want, accountId, options);
  }

  disconnectAbility(connection, callback) {
    console.log('disconnectAbility');
    return this.__context_impl__.disconnectAbility(connection, callback);
  }

  stopVpnExtensionAbility(connection, callback) {
    console.log('stopVpnExtensionAbility');
    return this.__context_impl__.stopVpnExtensionAbility(connection, callback);
  }

  terminateSelf(callback) {
    console.log('terminateSelf');
    return this.__context_impl__.terminateSelf(callback);
  }

  startAbilityByCall(want) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null) {
        console.log('VpnExtensionContext::startAbilityByCall input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want);
      } catch (error) {
        console.log('VpnExtensionContext::startAbilityByCall Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('VpnExtensionContext::startAbilityByCall success');
      return;
    });
  }

  startAbilityByCallWithAccount(want, accountId) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null || typeof accountId !== 'number') {
        console.log('VpnExtensionContext::startAbilityByCall With accountId input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want, accountId);
      } catch (error) {
        console.log('VpnExtensionContext::startAbilityByCall With accountId Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('VpnExtensionContext::startAbilityByCall With accountId success');
      return;
    });
  }
}

export default VpnExtensionContext;
