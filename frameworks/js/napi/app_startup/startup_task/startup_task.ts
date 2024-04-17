/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
//let taskPool = requireNapi('taskpool');
import abilityStageContext from "@ohos.application.AbilityStageContext"
import taskPool from "@taskpool"


class StartupTask {
  private result : ESObject = {};
  constructor() {
    "use sendable"
  }

  onDependenceCompleted(dependence : string, result: ESObject) {
    console.log('onDependenceCompleted');
  }

  init(context: abilityStageContext) {
    console.log('init');
  }

  ConcurrentFunc(context: abilityStageContext) {
    let res: ESObject = this.init(context);
    taskPool.Task.sendData(res);
  }
  
  SaveInitResult(obj : ESObject) {
    this.result = obj;
  }
  
  async AsyncPushTask(context : abilityStageContext) {
    try {
      let task: taskPool.Task = new taskPool.Task(this.ConcurrentFunc, context);
      task.onReceiveData(this.SaveInitResult);
      await taskPool.execute(task);
    } catch (e) {
      console.error(`taskPool: error code: ${e.code}, info: ${e.message}`);
    }
  }
}


export default StartupTask;
