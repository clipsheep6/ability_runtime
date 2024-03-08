/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
class BusinessError extends Error {
  constructor(code) {
    let msg = '';
    if (errMap.has(code)) {
      msg = errMap.get(code);
    } else {
      msg = ERROR_MSG_INNER_ERROR;
    }
    super(msg);
    this.code = code;
  }
}

class EventHub {
  constructor() {
    this.eventMap = {};
  }

  on(event, callback) {
    if ((typeof (event) !== 'string') || (typeof (callback) !== 'function')) {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (!this.eventMap[event]) {
      this.eventMap[event] = [];
    }
    if (this.eventMap[event].indexOf(callback) === -1) {
      this.eventMap[event].push(callback);
    }
  }

  off(event, callback) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      if (callback) {
        let cbArray = this.eventMap[event];
        let index = cbArray.indexOf(callback);
        if (index > -1) {
          for (; index + 1 < cbArray.length; index++) {
            cbArray[index] = cbArray[index + 1];
          }
          cbArray.pop();
        }
      } else {
        delete this.eventMap[event];
      }
    }
  }

  emit(event, ...args) {
    if (typeof (event) !== 'string') {
      throw new BusinessError(ERROR_CODE_INVALID_PARAM);
      return;
    }
    if (this.eventMap[event]) {
      const cloneArray = [...this.eventMap[event]];
      const len = cloneArray.length;
      for (let i = 0; i < len; ++i) {
        cloneArray[i].apply(this, args);
      }
    }
  }
}

class ApplicationContext {
  constructor(obj) {
    this.__context_impl__ = obj;
    this.__context_impl__.eventHub = new EventHub();
  }

  registerAbilityLifecycleCallback(abilityLifecycleCallback) {
    return this.__context_impl__.registerAbilityLifecycleCallback(abilityLifecycleCallback);
  }

  unregisterAbilityLifecycleCallback(callbackId, callback) {
    return this.__context_impl__.unregisterAbilityLifecycleCallback(callbackId, callback);
  }

  registerEnvironmentCallback(environmentCallback) {
    return this.__context_impl__.registerEnvironmentCallback(environmentCallback);
  }

  unregisterEnvironmentCallback(callbackId, envcallback) {
    return this.__context_impl__.unregisterEnvironmentCallback(callbackId, envcallback);
  }

  on(type, callback) {
    return this.__context_impl__.on(type, callback);
  }

  off(type, callbackId, callback) {
    return this.__context_impl__.off(type, callbackId, callback);
  }

  createBundleContext(bundleName) {
    return this.__context_impl__.createBundleContext(bundleName);
  }

  createModuleContext(moduleName) {
    return this.__context_impl__.createModuleContext(moduleName);
  }

  createModuleContext(bundleName, moduleName) {
    return this.__context_impl__.createModuleContext(bundleName, moduleName);
  }

  createSystemHspModuleResourceManager(bundleName, moduleName) {
    return this.__context_impl__.createSystemHspModuleResourceManager(bundleName, moduleName);
  }

  createModuleResourceManager(bundleName, moduleName) {
    return this.__context_impl__.createModuleResourceManager(bundleName, moduleName);
  }

  getGroupDir(groupId, callback) {
    return this.__context_impl__.getGroupDir(groupId, callback);
  }

  getApplicationContext() {
    return this.__context_impl__.getApplicationContext();
  }

  killAllProcesses(callback) {
    return this.__context_impl__.killAllProcesses(callback);
  }

  getProcessRunningInformation(callback) {
    return this.__context_impl__.getProcessRunningInformation(callback);
  }

  getRunningProcessInformation(callback) {
    return this.__context_impl__.getProcessRunningInformation(callback);
  }

  setColorMode(colorMode) {
    return this.__context_impl__.setColorMode(colorMode);
  }

  setLanguage(language) {
    return this.__context_impl__.setLanguage(language);
  }

  setAutoStartup(info, callback) {
    return this.__context_impl__.setAutoStartup(info, callback);
  }

  cancelAutoStartup(info, callback) {
    return this.__context_impl__.cancelAutoStartup(info, callback);
  }

  isAutoStartup(info, callback) {
    return this.__context_impl__.isAutoStartup(info, callback);
  }

  clearUpApplicationData() {
    return this.__context_impl__.clearUpApplicationData();
  }

  clearUpApplicationData(callback) {
    return this.__context_impl__.clearUpApplicationData(callback);
  }

  restartApp(want) {
    return this.__context_impl__.restartApp(want);
  }

  set area(mode) {
    return this.__context_impl__.switchArea(mode);
  }

  get area() {
    return this.__context_impl__.getArea();
  }

  get resourceManager() {
    return this.__context_impl__.resourceManager;
  }

  get applicationInfo() {
    return this.__context_impl__.applicationInfo;
  }

  get cacheDir() {
    return this.__context_impl__.cacheDir;
  }

  get tempDir() {
    return this.__context_impl__.tempDir;
  }

  get resourceDir() {
    return this.__context_impl__.resourceDir;
  }

  get filesDir() {
    return this.__context_impl__.filesDir;
  }

  get distributedFilesDir() {
    return this.__context_impl__.distributedFilesDir;
  }

  get databaseDir() {
    return this.__context_impl__.databaseDir;
  }

  get preferencesDir() {
    return this.__context_impl__.preferencesDir;
  }

  get bundleCodeDir() {
    return this.__context_impl__.bundleCodeDir;
  }

  get eventHub() {
    return this.__context_impl__.eventHub;
  }

  get stageMode() {
    return true;
  }
}

export default ApplicationContext;
