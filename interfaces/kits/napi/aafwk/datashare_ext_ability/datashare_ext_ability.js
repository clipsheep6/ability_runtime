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

class DataShareExtAbility {
    onCreate(want) {
        console.log('onCreate, want:' + want.abilityName);
    }

    onConnect(want) {
        console.log('onConnect, want:' + want.abilityName);
    }

    insert(want) {
        console.log('insert, want:' + want.abilityName);
    }

    delete(want) {
        console.log('delete, want:' + want.abilityName);
    }

    update(want) {
        console.log('update, want:' + want.abilityName);
    }

    query(want) {
        console.log('query, want:' + want.abilityName);
    }
}

export default DataShareExtAbility