/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

class EventHub {
  constructor() {
    this.eventMap = {};
  }

  on(event, callback) {
    if ((typeof (event) !== 'string') || (typeof (callback) !== 'function')) {
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
      return;
    }
    if (this.eventMap[event]) {
      if (callback) {
        let index = this.eventMap[event].indexOf(callback);
        if (index > -1) {
          this.eventMap[event].splice(index, 1);
        }
      } else {
        this.eventMap[event].length = 0;
      }
    }
  }

  emit(event, ...args) {
    if (typeof (event) !== 'string') {
      return;
    }
    if (this.eventMap[event]) {
      this.eventMap[event].map((callback) => {
        callback(...args);
      });
    }
  }
}

class Context {
  constructor(obj) {
    this.__context_impl__ = obj;
    this.__context_impl__.eventHub = new EventHub();
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

  getApplicationContext() {
    return this.__context_impl__.getApplicationContext();
  }

  getGroupDir(groupId, callback) {
    return this.__context_impl__.getGroupDir(groupId, callback);
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

export default Context;
