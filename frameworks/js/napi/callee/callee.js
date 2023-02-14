/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
var rpc = requireNapi("rpc")
var accessControl = requireNapi("abilityAccessCtrl")

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;
const REQUEST_FAILED = 1;
const PERMISSION_ABILITY_BACKGROUND_COMMUNICATION = "ohos.permission.ABILITY_BACKGROUND_COMMUNICATION"

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_FUNC_REGISTERED = 16200004;
const ERROR_CODE_FUNC_NOT_EXIST = 16200005;
const ERROR_CODE_INNER_ERROR = 16000050;

const ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
const ERROR_MSG_FUNC_REGISTERED = "Method registered. The method has registered.";
const ERROR_MSG_FUNC_NOT_EXIST = "Method not registered. The method has not registered.";
const ERROR_MSG_INNER_ERROR = "Inner Error.";

var errMap = new Map();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_FUNC_REGISTERED, ERROR_MSG_FUNC_REGISTERED);
errMap.set(ERROR_CODE_FUNC_NOT_EXIST, ERROR_MSG_FUNC_NOT_EXIST);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);

class BusinessError extends Error {
    constructor(code) {
        let msg = "";
        if (errMap.has(code)) {
            msg = errMap.get(code);
        } else {
            msg = ERROR_MSG_INNER_ERROR;
        }
        super(msg);
        this.code = code;
    }
}

class Callee extends rpc.RemoteObject {
    constructor(des) {
        if (typeof des === 'string') {
            super(des);
            this.callList = new Map();
            this.startUpNewRule = false;
            console.log("Callee constructor is OK " + typeof des);
        } else {
            console.log("Callee constructor error, des is " + typeof des);
            return null;
        }
    }

    setNewRuleFlag(flag) {
        this.startUpNewRule = flag;
    }

    onRemoteMessageRequest(code, data, reply, option) {
        console.log("Callee onRemoteMessageRequest code [" + typeof code + " " + code + "]");
        if (this.startUpNewRule && rpc.IPCSkeleton.isLocalCalling()) {
            console.log("Use new start up rule, check caller permission.");
            let accessManger = accessControl.createAtManager();
            let accessTokenId = rpc.IPCSkeleton.getCallingTokenId();
            let grantStatus =
                accessManger.verifyAccessTokenSync(accessTokenId, PERMISSION_ABILITY_BACKGROUND_COMMUNICATION);
            if (grantStatus === accessControl.GrantStatus.PERMISSION_DENIED) {
                console.log(
                    "Callee onRemoteMessageRequest error, the Caller does not have PERMISSION_ABILITY_BACKGROUND_COMMUNICATION");
                return false;
            }
        }

        if (typeof code !== 'number' || typeof data !== 'object' ||
            typeof reply !== 'object' || typeof option !== 'object') {
            console.log("Callee onRemoteMessageRequest error, code is [" +
                typeof code + "], data is [" + typeof data + "], reply is [" +
                typeof reply + "], option is [" + typeof option + "]");
            return false;
        }

        console.log("Callee onRemoteMessageRequest code proc");
        if (code == EVENT_CALL_NOTIFY) {
            if (this.callList == null) {
                console.log("Callee onRemoteMessageRequest error, this.callList is nullptr");
                return false;
            }

            let method = data.readString();
            console.log("Callee onRemoteMessageRequest method [" + method + "]");
            let func = this.callList.get(method);
            if (typeof func !== 'function') {
                console.log("Callee onRemoteMessageRequest error, get func is " + typeof func);
                return false;
            }

            let result = func(data);
            if (typeof result === 'object' && result != null) {
                reply.writeInt(REQUEST_SUCCESS);
                reply.writeString(typeof result);
                reply.writeParcelable(result);
                console.log("Callee onRemoteMessageRequest code proc Packed data");
            } else {
                reply.writeInt(REQUEST_FAILED);
                reply.writeString(typeof result);
                console.log("Callee onRemoteMessageRequest error, retval is " + REQUEST_FAILED + ", type is " + typeof result);
            }
        } else {
            console.log("Callee onRemoteMessageRequest error, code is " + code);
            return false;
        }
        console.log("Callee onRemoteMessageRequest code proc success");
        return true;
    }

    on(method, callback) {
        if (typeof method !== 'string' || method == "" || typeof callback !== 'function') {
            console.log(
                "Callee on error, method is [" + typeof method + "], typeof callback [" + typeof callback + "]");
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (this.callList == null) {
            console.log("Callee on error, this.callList is nullptr");
            throw new BusinessError(ERROR_CODE_INNER_ERROR);;
        }

        if (this.callList.has(method)) {
            console.log("Callee on error, [" + method + "] has registered");
            throw new BusinessError(ERROR_CODE_FUNC_REGISTERED);
        }

        this.callList.set(method, callback);
        console.log("Callee on method [" + method + "]");
    }

    off(method) {
        if (typeof method !== 'string' || method == "") {
            console.log("Callee off error, method is [" + typeof method + "]");
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (this.callList == null) {
            console.log("Callee off error, this.callList is null");
            throw new BusinessError(ERROR_CODE_INNER_ERROR);
        }

        if (!this.callList.has(method)) {
            console.log("Callee off error, this.callList not found " + method);
            throw new BusinessError(ERROR_CODE_FUNC_NOT_EXIST);
        }

        this.callList.delete(method);
        console.log("Callee off method [" + method + "]");
    }
}

export default Callee
