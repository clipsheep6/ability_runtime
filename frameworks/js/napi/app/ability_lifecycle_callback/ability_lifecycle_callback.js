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

class AbilityLifecycleCallback {
  constructor() {}

  onAbilityCreate(ability) {
    console.log('onAbilityCreate');
  }

  onWindowStageCreate(ability, windowStage) {
    console.log('onWindowStageCreate');
  }

  onWindowStageActive(ability, windowStage) {
    console.log('onWindowStageActive');
  }

  onWindowStageInactive(ability, windowStage) {
    console.log('onWindowStageInactive');
  }

  onWindowStageDestroy(ability, windowStage) {
    console.log('onWindowStageDestroy');
  }

  onAbilityDestroy(ability) {
    console.log('onAbilityDestroy');
  }

  onAbilityForeground(ability) {
    console.log('onAbilityForeground');
  }

  onAbilityBackground(ability) {
    console.log('onAbilityBackground');
  }

  onAbilityContinue(ability) {
    console.log('onAbilityContinue');
  }

  onNewWant(ability) {
    console.log('onNewWant');
  }

  onWillNewWant(ability) {
    console.log('onWillNewWant');
  }

  onAbilityWillCreate(ability) {
    console.log('onAbilityWillCreate');
  }

  onWindowStageWillCreate(ability, windowStage) {
    console.log('onWindowStageWillCreate');
  }

  onWindowStageWillDestroy(ability, windowStage) {
    console.log('onWindowStageWillDestroy');
  }

  onAbilityWillDestroy(ability) {
    console.log('onAbilityWillDestroy');
  }

  onAbilityWillForeground(ability) {
    console.log('onAbilityWillForeground');
  }

  onAbilityWillBackground(ability) {
    console.log('onAbilityWillBackground');
  }
}

export default AbilityLifecycleCallback;