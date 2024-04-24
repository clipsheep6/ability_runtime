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


function ConcurrentFunc(asyncTask) {
  "use concurrent";
  console.log('AsyncTaskExcutor ConcurrentFunc start ');
  let taskPool = requireNapi('taskpool');
  let context = undefined;
  other.init(context);
  console.log('AsyncTaskExcutor ConcurrentFunc start 555555555555');
  asyncTask.init(context);
  taskPool.Task.sendData(1);
  console.log('AsyncTaskExcutor ConcurrentFunc end ');
}

function PushTask(asyncTask) {
  console.log('AsyncTaskExcutor PushTask start 44444444');
  let context = undefined;
  let taskPool = requireNapi('taskpool');
  let task = new taskPool.Task(ConcurrentFunc, asyncTask);
  task.onReceiveData((res: number) => {
    console.log('AsyncTaskExcutor onAsyncTaskCompleted res = ' + res);
  });
  taskPool.execute(task);
  console.log('AsyncTaskExcutor PushTask end');
}

class AsyncTaskExcutor {
  public AsyncPushTask(asyncTask) {
    console.log('AsyncTaskExcutor AsyncPushTask start 333333333333');
    PushTask(asyncTask);
    console.log('AsyncTaskExcutor AsyncPushTask end');
  }
}

export default AsyncTaskExcutor;