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

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_CALLER_RELEASED = 16200001;
const ERROR_CODE_CLAAEE_INVALID = 16200002;
const ERROR_CODE_INNER_ERROR = 16000050;

const ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
const ERROR_MSG_CALLER_RELEASED = "Caller released. The caller has been released.";
const ERROR_MSG_CLAAEE_INVALID = "Callee invalid. The callee does not exist.";
const ERROR_MSG_INNER_ERROR = "Inner Error.";

var errMap = new Map();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_CALLER_RELEASED, ERROR_MSG_CALLER_RELEASED);
errMap.set(ERROR_CODE_CLAAEE_INVALID, ERROR_MSG_CLAAEE_INVALID);
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

class Caller {
    constructor(obj) {
        console.log("Caller::constructor obj is " + typeof obj);
        this.__call_obj__ = obj;
        this.releaseState = false;
    }

    call(method, data) {
        return new Promise(async (resolve, reject) => {
            console.log("Caller call method [" + method + "]");
            if (typeof method !== 'string' || typeof data !== 'object') {
                console.log("Caller call " + typeof method + " " + typeof data);
                reject(new BusinessError(ERROR_CODE_INVALID_PARAM));
                return;
            }

            if (method == '' || data == null) {
                console.log("Caller call " + method + ", " + data);
                reject(new BusinessError(ERROR_CODE_INVALID_PARAM));
                return;
            }

            if (this.releaseState == true) {
                console.log("Caller call this.callee release");
                reject(new BusinessError(ERROR_CODE_CALLER_RELEASED));
                return;
            }

            if (this.__call_obj__.callee == null) {
                console.log("Caller call this.callee is nullptr");
                reject(new BusinessError(ERROR_CODE_CLAAEE_INVALID));
                return;
            }

            console.log("Caller call msgData rpc.MessageSequence create");
            let msgData = rpc.MessageSequence.create();
            msgData.writeString(method);
            let msgReply = rpc.MessageSequence.create();
            let option = rpc.MessageOption();
            msgData.writeParcelable(data);

            let retData = undefined;
            try {
                retData = await this.__call_obj__.callee.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
                console.log("Caller call msgData rpc.sendMessageRequest called");
                if (retData.errCode != 0) {
                    msgData.reclaim();
                    msgReply.reclaim();
                    console.log("Caller call return errCode " + retData.errCode);
                    reject(new BusinessError(retData.errCode));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData rpc.sendMessageRequest error " + e);
            }

            try {
                let retval = retData.reply.readInt();
                let str = retData.reply.readString();
                if (retval === REQUEST_SUCCESS && str === 'object') {
                    console.log("Caller call return str " + str);
                    msgData.reclaim();
                    msgReply.reclaim();
                } else {
                    console.log("Caller call retval is [" + retval + "], str [" + str + "]");
                    msgData.reclaim();
                    msgReply.reclaim();
                    reject(new BusinessError(retval));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData sendMessageRequest retval error");
                reject(new BusinessError(ERROR_CODE_INNER_ERROR));
                return;
            }

            console.log("Caller call msgData sendMessageRequest end");
            resolve(undefined);
            return;
        });
    }

    callWithResult(method, data) {
        return new Promise(async (resolve, reject) => {
            console.log("Caller callWithResult method [" + method + "]");
            if (typeof method !== 'string' || typeof data !== 'object') {
                console.log("Caller callWithResult " + typeof method + " " + typeof data);
                reject(new BusinessError(ERROR_CODE_INVALID_PARAM));
                return;
            }

            if (method == '' || data == null) {
                console.log("Caller callWithResult " + method + ", " + data);
                reject(new BusinessError(ERROR_CODE_INVALID_PARAM));
                return;
            }

            if (this.releaseState == true) {
                console.log("Caller callWithResult this.callee release");
                reject(new BusinessError(ERROR_CODE_CALLER_RELEASED));
                return;
            }

            if (this.__call_obj__.callee == null) {
                console.log("Caller callWithResult this.callee is nullptr");
                reject(new BusinessError(ERROR_CODE_CLAAEE_INVALID));
                return;
            }

            console.log("Caller callWithResult msgData rpc.MessageSequence create");
            let msgData = rpc.MessageSequence.create();
            msgData.writeString(method);
            let msgReply = rpc.MessageSequence.create();
            let option = rpc.MessageOption();
            msgData.writeParcelable(data);

            let reply = undefined;
            let retData = undefined;
            try {
                retData = await this.__call_obj__.callee.sendMessageRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
                console.log("Caller callWithResult msgData rpc.sendMessageRequest called");
                if (retData.errCode != 0) {
                    msgData.reclaim();
                    msgReply.reclaim();
                    console.log("Caller callWithResult return errCode " + retData.errCode);
                    reject(new BusinessError(retData.errCode));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData rpc.MessageSequence error " + e);
            }

            try {
                let retval = retData.reply.readInt();
                let str = retData.reply.readString();
                if (retval === REQUEST_SUCCESS && str === 'object') {
                    console.log("Caller callWithResult return str " + str);
                    msgData.reclaim();
                    reply = retData.reply;
                } else {
                    console.log("Caller callWithResult retval is [" + retval + "], str [" + str + "]");
                    msgData.reclaim();
                    msgReply.reclaim();
                    reject(new BusinessError(retval));
                    return;
                }
            } catch (e) {
                console.log("Caller callWithResult msgData sendMessageRequest retval error");
                reject(new BusinessError(ERROR_CODE_INNER_ERROR));
                return;
            }

            console.log("Caller callWithResult msgData sendMessageRequest end");
            resolve(reply);
            return;
        });
    }

    release() {
        console.log("Caller release js called.");
        if (this.releaseState == true) {
            console.log("Caller release remoteObj releaseState is true");
            throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
        }

        if (this.__call_obj__.callee == null) {
            console.log("Caller release call remoteObj is released");
            throw new BusinessError(ERROR_CODE_CLAAEE_INVALID);
        }

        this.releaseState = true;
        this.__call_obj__.release();
    }

    onRelease(callback) {
        console.log("Caller onRelease jscallback called.");
        if (typeof callback !== 'function') {
            console.log("Caller onRelease " + typeof callback);
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (this.releaseState == true) {
            console.log("Caller onRelease remoteObj releaseState is true");
            throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
        }

        this.__call_obj__.onRelease(callback);
    }

    on(type, callback) {
        console.log("Caller onRelease jscallback called.");
        if (typeof type !== 'string' || type !== "release") {
            console.log(
                "Caller onRelease error, input [type] is invalid.");
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (typeof callback !== 'function') {
            console.log("Caller onRelease error " + typeof callback);
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (this.releaseState == true) {
            console.log("Caller onRelease error, remoteObj releaseState is true");
            throw new BusinessError(ERROR_CODE_CALLER_RELEASED);
        }

        this.__call_obj__.onRelease(callback);
    }

    off(type, callback) {
        if (typeof type !== 'string' || type !== "release") {
            console.log(
                "Caller onRelease error, input [type] is invalid.");
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }

        if (typeof callback !== 'function') {
            console.log("Caller onRelease error " + typeof callback);
            throw new BusinessError(ERROR_CODE_INVALID_PARAM);
        }
        // Empty
    }
}

export default Caller
