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

import {processDataCallback} from "./i_idl_service_ext";
import {insertDataToMapCallback} from "./i_idl_service_ext";
import IIdlServiceExt from "./i_idl_service_ext";
import rpc from "@ohos.rpc";

export default class IdlServiceExtStub extends rpc.RemoteObject implements IIdlServiceExt {
    constructor(des: string) {
        super(des);
    }

    async onRemoteMessageRequest(code: number, data, reply, option): Promise<boolean> {
        console.log("onRemoteMessageRequest called, code = " + code);
        switch(code) {
            case IdlServiceExtStub.COMMAND_PROCESS_DATA: {
                let _data = data.readInt();
                this.processData(_data, (errCode, returnValue) => {
                    reply.writeInt(errCode);
                    if (errCode == 0) {
                        reply.writeInt(returnValue);
                    }
                });
                return true;
            }
            case IdlServiceExtStub.COMMAND_INSERT_DATA_TO_MAP: {
                let _key = data.readString();
                let _val = data.readInt();
                this.insertDataToMap(_key, _val, (errCode) => {
                    reply.writeInt(errCode);
                });
                return true;
            }
            default: {
                console.log("invalid request code" + code);
                break;
            }
        }
        return false;
    }

    processData(data: number, callback: processDataCallback): void{}
    insertDataToMap(key: string, val: number, callback: insertDataToMapCallback): void{}

    static readonly COMMAND_PROCESS_DATA = 1;
    static readonly COMMAND_INSERT_DATA_TO_MAP = 2;
}
