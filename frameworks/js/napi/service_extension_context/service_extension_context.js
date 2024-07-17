/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

class ServiceExtensionContext extends ExtensionContext {
  constructor(obj) {
    super(obj);
  }

  startAbility(want, options, callback) {
    console.log('startAbility');
    return this.__context_impl__.startAbility(want, options, callback);
  }

  openLink(link, options) {
    console.log('openLink');
    return this.__context_impl__.openLink(link, options);
  }

  startAbilityAsCaller(want, options, callback) {
    console.log('startAbilityAsCaller');
    return this.__context_impl__.startAbilityAsCaller(want, options, callback);
  }

  startRecentAbility(want, options, callback) {
    console.log('startRecentAbility');
    return this.__context_impl__.startRecentAbility(want, options, callback);
  }

  connectServiceExtensionAbility(want, options) {
    console.log('connectServiceExtensionAbility');
    return this.__context_impl__.connectServiceExtensionAbility(want, options);
  }

  startAbilityWithAccount(want, accountId, options, callback) {
    console.log('startAbilityWithAccount');
    return this.__context_impl__.startAbilityWithAccount(want, accountId, options, callback);
  }

  startServiceExtensionAbility(want, callback) {
    console.log('startServiceExtensionAbility');
    return this.__context_impl__.startServiceExtensionAbility(want, callback);
  }

  startUIServiceExtensionAbility(want, callback) {
    console.log('startUIServiceExtensionAbility');
    return this.__context_impl__.startUIServiceExtensionAbility(want, callback);
  }

  startServiceExtensionAbilityWithAccount(want, accountId, callback) {
    console.log('startServiceExtensionAbilityWithAccount');
    return this.__context_impl__.startServiceExtensionAbilityWithAccount(want, accountId, callback);
  }

  stopServiceExtensionAbility(want, callback) {
    console.log('stopServiceExtensionAbility');
    return this.__context_impl__.stopServiceExtensionAbility(want, callback);
  }

  stopServiceExtensionAbilityWithAccount(want, accountId, callback) {
    console.log('stopServiceExtensionAbilityWithAccount');
    return this.__context_impl__.stopServiceExtensionAbilityWithAccount(want, accountId, callback);
  }

  connectAbilityWithAccount(want, accountId, options) {
    console.log('connectAbilityWithAccount');
    return this.__context_impl__.connectAbilityWithAccount(want, accountId, options);
  }

  connectServiceExtensionAbilityWithAccount(want, accountId, options) {
    console.log('connectServiceExtensionAbilityWithAccount');
    return this.__context_impl__.connectServiceExtensionAbilityWithAccount(want, accountId, options);
  }

  disconnectAbility(connection, callback) {
    console.log('disconnectAbility');
    return this.__context_impl__.disconnectAbility(connection, callback);
  }

  disconnectServiceExtensionAbility(connection, callback) {
    console.log('disconnectServiceExtensionAbility');
    return this.__context_impl__.disconnectServiceExtensionAbility(connection, callback);
  }

  terminateSelf(callback) {
    console.log('terminateSelf');
    return this.__context_impl__.terminateSelf(callback);
  }

  requestModalUIExtension(want, callback) {
    return this.__context_impl__.requestModalUIExtension(want, callback);
  }
  
  startAbilityByCall(want) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null) {
        console.log('ServiceExtensionContext::startAbilityByCall input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want);
      } catch (error) {
        console.log('ServiceExtensionContext::startAbilityByCall Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('ServiceExtensionContext::startAbilityByCall success');
      return;
    });
  }

  startAbilityByCallWithAccount(want, accountId) {
    return new Promise(async (resolve, reject) => {
      if (typeof want !== 'object' || want == null || typeof accountId !== 'number') {
        console.log('ServiceExtensionContext::startAbilityByCall With accountId input param error');
        reject(new ParamError());
        return;
      }

      let callee = null;
      try {
        callee = await this.__context_impl__.startAbilityByCall(want, accountId);
      } catch (error) {
        console.log('ServiceExtensionContext::startAbilityByCall With accountId Obtain remoteObject failed');
        reject(error);
        return;
      }

      resolve(new Caller(callee));
      console.log('ServiceExtensionContext::startAbilityByCall With accountId success');
      return;
    });
  }

  preStartMission(bundleName, moduleName, abilityName, startTime) {
    console.log('preStartMission');
    return this.__context_impl__.preStartMission(bundleName, moduleName, abilityName, startTime);
  }
}

export default ServiceExtensionContext;
