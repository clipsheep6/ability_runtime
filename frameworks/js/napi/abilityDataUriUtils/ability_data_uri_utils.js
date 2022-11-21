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
const URI_SPLIT = '/'

const ERROR_CODE_INVALID_PARAM = 401;
const ERROR_CODE_INNER_ERROR = 16000050;

const ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
const ERROR_MSG_INNER_ERROR = "Inner Error.";

var errMap = new Map();
errMap.set(ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM);
errMap.set(ERROR_CODE_INNER_ERROR, ERROR_MSG_INNER_ERROR);

class DataUriError extends Error {
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

let dataUriUtils = {
    getId : (uri) => {
        console.debug("DataUriUtils getId called.");
        if (typeof uri !== 'string') {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        let index = uri.lastIndexOf(URI_SPLIT);
        if (index === -1) {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        let ret = uri.substring(index + 1);
        if (ret === "" || isNaN(Number(ret))) {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        return Number(ret);
    },
    updateId : (uri, id) => {
        console.debug("DataUriUtils updateId called.");
        if (typeof uri !== 'string' || typeof id !== 'number') {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        let ret = dataUriUtils.deleteId(uri);
        if (ret === uri) {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        return ret + URI_SPLIT + id;
    },
    deleteId : (uri) => {
        console.debug("DataUriUtils deleteId called.");
        if (typeof uri !== 'string') {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        let index = uri.lastIndexOf(URI_SPLIT);
        if (index === -1) {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        var id = uri.substring(index + 1);
        if (id === "" || isNaN(Number(id))) {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        return uri.substring(0, index);
    },
    attachId : (uri, id) => {
        console.debug("DataUriUtils attachId called.");
        if (typeof uri !== 'string' || typeof id !== 'number') {
            throw new DataUriError(ERROR_CODE_INVALID_PARAM);
        }
        return uri + URI_SPLIT + id;
    }
};

export default dataUriUtils