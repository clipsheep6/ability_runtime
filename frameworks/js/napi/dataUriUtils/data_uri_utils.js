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

let dataUriUtils = {
    getId : (uri) => {
        console.debug("DataUriUtils getId called.");
        if (typeof uri !== 'string') {
            return -1;
        }
        let index = uri.lastIndexOf(URI_SPLIT);
        if (index === -1) {
            return -1;
        }
        let ret = uri.substring(index + 1);
        if (ret === "" || isNaN(Number(ret))) {
            return -1;
        }
        return Number(ret);
    },
    updateId : (uri, id) => {
        console.debug("DataUriUtils updateId called.");
        if (typeof uri !== 'string' || typeof id !== 'number') {
            return uri;
        }
        let ret = dataUriUtils.deleteId(uri);
        if (ret === uri) {
            return uri;
        }
        return ret + URI_SPLIT + id;
    },
    deleteId : (uri) => {
        console.debug("DataUriUtils deleteId called.");
        if (typeof uri !== 'string') {
            return uri;
        }
        let index = uri.lastIndexOf(URI_SPLIT);
        if (index === -1) {
            return uri;
        }
        var id = uri.substring(index + 1);
        if (id === "" || isNaN(Number(id))) {
            return uri;
        }
        return uri.substring(0, index);
    },
    attachId : (uri, id) => {
        console.debug("DataUriUtils attachId called.");
        if (typeof uri !== 'string' || typeof id !== 'number') {
            return uri;
        }
        return uri + URI_SPLIT + id;
    }
};

export default dataUriUtils