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

import type common from '@ohos.app.ability.common';

export class GlobalThis {
  private constructor() {}
  private static instance: GlobalThis;
  private _serviceExtensionContexts = new Map<string, common.ServiceExtensionContext>();
  private _properties = new Map<string, string | number | Boolean | string[] | string[][] | Record<string, Object> | Record<string, Object>[]>();
  private _objects = new Map<string, Object>();


  public static getInstance(): GlobalThis {
    if (!GlobalThis.instance) {
      GlobalThis.instance = new GlobalThis();
    }
    return GlobalThis.instance;
  }

  getContext(key: string): common.ServiceExtensionContext | undefined {
    return this._serviceExtensionContexts.get(key);
  }

  setContext(key: string, value: common.ServiceExtensionContext): void {
    this._serviceExtensionContexts.set(key, value);
  }

  getProperty(key: string): string | number | Boolean | string[] | string[][] | Record<string, Object> | Record<string, Object>[] | undefined {
    return this._properties.get(key);
  }

  setProperty(key: string, value: string | number | Boolean | string[] | string[][] | Record<string, Object> | Record<string, Object>[]): void {
    this._properties.set(key, value);
  }

  getObject(key: string): Object | undefined {
    return this._objects.get(key);
  }

  setObject(key: string, value: Object): void {
    this._objects.set(key, value);
  }
}