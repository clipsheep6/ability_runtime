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

async function concurrentFunc(startup, asyncCallback, context, startupName) {
  'use concurrent';
  console.log('concurrentFunc start.');
  let startupResult;
  await startup.init(context).then((result: Object) => {
    startupResult = result
    let taskPool = requireNapi('taskpool');
    taskPool.Task.sendData(asyncCallback, startupName, startupResult);
  });
  console.log('concurrentFunc end.');
}

function receiveResult(asyncCallback, startupName, startupResult) {
  console.log('receiveResult called.');
  asyncCallback.onAsyncTaskCompleted(startupName, startupResult);
  console.log('receiveResult end.');
}

function pushTask(startup, asyncCallback, context, startupName) {
  console.log('pushTask start.');
  let taskPool = requireNapi('taskpool');
  try {
    let task = new taskPool.Task(concurrentFunc, startup, asyncCallback, context, startupName);
    task.onReceiveData(receiveResult);
    taskPool.execute(task);
  } catch (error) {
    console.log('new taskPool failed message:' + error);
  }
  console.log('pushTask end.');
}

class AsyncTaskExcutor {
  public asyncPushTask(startup, asyncCallback, context, startupName) {
    console.log('asyncPushTask AsyncPushTask start.');
    pushTask(startup, asyncCallback, context, startupName);
    console.log('asyncPushTask AsyncPushTask end.');
  }
}

export default AsyncTaskExcutor;