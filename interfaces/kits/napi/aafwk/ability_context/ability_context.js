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

var Context = requireNapi("application.Context")

class AbilityContext extends Context {
    constructor(obj) {
        super(obj)
        this.abilityInfo = obj.abilityInfo
        this.currentHapModuleInfo = obj.currentHapModuleInfo
    }

    startAbility(want, options, callback) {
        return this.__context_impl__.startAbility(want, options, callback)
    }

    startAbilityForResult(want, callback) {
        return this.__context_impl__.startAbilityForResult(want, callback)
    }

    connectAbility(want, options) {
        return this.__context_impl__.connectAbility(want, options);
    }

    disconnectAbility(connection, callback) {
        return this.__context_impl__.disconnectAbility(connection, callback);
    }

    terminateSelf(callback) {
        return this.__context_impl__.terminateSelf(callback)
    }

    terminateSelfWithResult(abilityResult, callback) {
        return this.__context_impl__.terminateSelfWithResult(abilityResult, callback)
    }

    requestPermissionsFromUser(permissions, requestCode, resultCallback) {
        return this.__context_impl__.requestPermissionsFromUser(permissions, requestCode, resultCallback)
    }

    restoreWindowStage(contentStorage) {
        return this.__context_impl__.restoreWindowStage(contentStorage)
    }

    SwitchArea(mode) {
        return this.__context_impl__.SwitchArea(mode)
    }
}

export default AbilityContext
