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

function ConcurrentFunc(Startup, AsyncCallback, context) {
  "use concurrent";
  console.log('AsyncTaskExcutor ConcurrentFunc start ');
  let taskPool = requireNapi('taskpool');
  Startup.init(context);
  taskPool.Task.sendData(AsyncCallback);
  console.log('AsyncTaskExcutor ConcurrentFunc end ');
}

function SaveInitResult(AsyncCallback) {
  console.log('AsyncTaskExcutor SaveInitResult');
  AsyncCallback.onAsyncTaskCompleted();
}

function PushTask(Startup, AsyncCallback, context) {
  console.log('AsyncTaskExcutor PushTask start');
  let taskPool = requireNapi('taskpool');
  let task = new taskPool.Task(ConcurrentFunc, Startup, AsyncCallback, context);
  task.onReceiveData(SaveInitResult);

  taskPool.execute(task);
  console.log('AsyncTaskExcutor PushTask end');
}

class AsyncTaskExcutor {
  public AsyncPushTask(Startup, AsyncCallback, context) {
    console.log('AsyncTaskExcutor AsyncPushTask start');
    PushTask(Startup, AsyncCallback, context);
    console.log('AsyncTaskExcutor AsyncPushTask end');
  }
}

export default AsyncTaskExcutor;