/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import { AsyncCallback } from '../basic';

/**
 * Provides the context of an object in the application and obtains application environment information.
 * @name Context
 * @since 6
 * @sysCap AAFwk
 * @devices phone
 * @permission N/A
 */
export interface Context {
    /**
     * Checks whether the calling or current process has the given permission.
     * @devices phone
     * @since 6
     * @sysCap AAFwk
     * @param -
     * @return Returns 0 if the calling or current process has the permission; returns -1 otherwise.
     */
    verifyCallingOrSelfPermission(permission: string, callback: AsyncCallback<number>): void;
    verifyCallingOrSelfPermission(permission: string): Promise<number>;

    /**
     * Checks whether the calling process for inter-process communication has the given permission.
     * @devices phone
     * @since 6
     * @sysCap AAFwk
     * @param -
     * @return Returns 0 if the calling process has the permission; returns -1 otherwise.
     */
    verifyCallingPermission(permission: string, callback: AsyncCallback<number>): void;
    verifyCallingPermission(permission: string): Promise<number>;

    /**
     * Checks whether a process ID (PID) and user ID (UID) have the given permission.
     * @devices phone
     * @since 6
     * @sysCap AAFwk
     * @param -
     * @return Returns 0 if the PID and UID have the permission; returns -1 otherwise.
     */
    verifyPermission(permission: string, pid: number, uid: number, callback: AsyncCallback<number>): void;
    verifyPermission(permission: string, pid: number, uid: number): Promise<number>;

    /**
     * Checks whether the current process has the given permission.
     * @devices phone
     * @since 6
     * @sysCap AAFwk
     * @param -
     * @return Returns 0 if the current process has the permission; returns -1 otherwise.
     */
    verifySelfPermission(permission: string, callback: AsyncCallback<number>): void;
    verifySelfPermission(permission: string): Promise<number>;

    /**
     * Confirms with the permission management module to check whether a request prompt is
     * required for granting a certain permission.
     * @devices phone
     * @since 6
     * @sysCap AAFwk
     * @param -
     * @return Returns true if the current application does not have the permission and the user does not 
     * turn off further requests; returns false if the current application already has the permission, 
     * the permission is rejected by the system, or the permission is denied by the user and 
     * the user has turned off further requests.
     */
    canRequestPermission(permission: string, callback: AsyncCallback<boolean>): void;
    canRequestPermission(permission: string): Promise<boolean>;
}