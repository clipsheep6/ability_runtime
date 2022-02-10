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
import { AsyncCallback } from './basic';
import { StartAbilityParameter } from './ability/startAbilityParameter';
import { Want } from './ability/want';
import { ConnectOptions } from './ability/connectOptions';
import { NotificationRequest } from './notification/notificationRequest';

/**
 * A Particle Ability represents an ability with service.
 * @name particleAbility
 * @since 7
 * @sysCap AAFwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace particleAbility {

  /**
   * Service ability uses this method to start a specific ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbility(parameter: StartAbilityParameter, callback: AsyncCallback<void>): void;
  function startAbility(parameter: StartAbilityParameter): Promise<void>;

  /**
   * Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
   * @default -
   * @devices phone, tablet
   * @since 7
   * @SysCap aafwk
   * @param request The element name of the service ability
   * @param options The remote object instance
   * @return Returns the number of the ability connected
   */
   function connectAbility(request: Want, options:ConnectOptions ): number;

   /**
    * The callback interface was connect successfully.
    * @default -
    * @devices phone, tablet
    * @since 7
    * @SysCap aafwk
    * @param connection The number of the ability connected
    */
   function disconnectAbility(connection: number, callback:AsyncCallback<void>): void;
   function disconnectAbility(connection: number): Promise<void>;

  /**
   * Keep this Service ability in the background and display a notification bar.
   *
   * @since 7
   * @syscap SystemCapability.ResourceSchedule.BackgroundTaskManager.ContinuousTask
   * @permission ohos.permission.KEEP_BACKGROUND_RUNNING
   * @param id Identifies the notification bar information.
   * @param request Indicates the notificationRequest instance containing information for displaying a notification bar.
   * @deprecated
   */
  function startBackgroundRunning(id: number, request: NotificationRequest, callback: AsyncCallback<void>): void;
  function startBackgroundRunning(id: number, request: NotificationRequest): Promise<void>;

  /**
   * Cancel background running of this ability to free up system memory.
   *
   * @since 7
   * @syscap SystemCapability.ResourceSchedule.BackgroundTaskManager.ContinuousTask
   * @deprecated
   */
  function cancelBackgroundRunning(callback: AsyncCallback<void>): void;
  function cancelBackgroundRunning(): Promise<void>;
}
export default particleAbility;