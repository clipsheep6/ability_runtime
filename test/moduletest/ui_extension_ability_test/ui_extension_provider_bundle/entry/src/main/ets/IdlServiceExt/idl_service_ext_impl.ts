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

import { processDataCallback } from './i_idl_service_ext';
import { insertDataToMapCallback } from './i_idl_service_ext';
import IdlServiceExtStub from './idl_service_ext_stub';

const ERR_OK = 0;
const TAG: string = "[IdlServiceExtImpl]";

// 开发者需要在这个类型里对接口进行实现
export default class ServiceExtImpl extends IdlServiceExtStub {
  processData(data: number, callback: processDataCallback): void {
    // 开发者自行实现业务逻辑
    console.info(TAG, `processData: ${data}`);
    callback(ERR_OK, data + 1);
  }

  insertDataToMap(key: string, val: number, callback: insertDataToMapCallback): void {
    // 开发者自行实现业务逻辑
    console.info(TAG, `insertDataToMap, key: ${key}  val: ${val}`);
    callback(ERR_OK);
  }
}
