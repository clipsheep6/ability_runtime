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

import { AsyncCallback, Callback } from './basic';
import { Want } from './ability/want';
import Context from './application/Context';
import { ValuesBucket, ResultSet, DataSharePredicates } from './@ohos.data.rdb';

declare namespace dataShare {
    /**
     * Obtains the dataShareHelper.
     * @since 9
     * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
     * @param context Indicates the application context.
     * @param want Indicates The element parameter of the service ability.
     * @param uri Indicates the path of the file to open.
     * @return Returns the dataShareHelper.
     * @systemapi Hide this for inner system use
     */
    function createDataShareHelper(context: Context, want: Want, uri: string): DataShareHelper;

    /**
     * DataShareHelper
     * @since 9
     * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
     * @systemapi Hide this for inner system use
     */
    interface DataShareHelper {
        /**
         * Opens a file in a specified remote path.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the file to open.
         * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
         *             (erasing whatever data is currently in the file), "wt" for write access that truncates any existing
         *             file, "wa" for write-only access to append to any existing data, "rw" for read and write access on
         *             any existing data, or "rwt" for read and write access that truncates any existing file.
         * @param callback Indicates the callback when openfile success
         * @systemapi Hide this for inner system use
         */
        openFile(uri: string, mode: string, callback: AsyncCallback<number>): void;
        openFile(uri: string, mode: string): Promise<number>;

        /**
         * Registers an observer to observe data specified by the given uri.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param type dataChange.
         * @param uri Indicates the path of the data to operate.
         * @param callback Indicates the callback when dataChange.
         * @systemapi Hide this for inner system use
         */
        on(type: 'dataChange', uri: string, callback: AsyncCallback<void>): void;

        /**
         * Deregisters an observer used for monitoring data specified by the given uri.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param type dataChange.
         * @param uri Indicates the path of the data to operate.
         * @param callback Indicates the registered callback.
         * @systemapi Hide this for inner system use
         */
        off(type: 'dataChange', uri: string, callback?: AsyncCallback<void>): void;

        /**
         * Inserts a single data record into the database.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the data to operate.
         * @param value Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
         * @return Returns the index of the inserted data record.
         * @systemapi Hide this for inner system use
         */
        insert(URI: string, values: ValuesBucket, callback: AsyncCallback<number>): void;
        insert(URI: string, values: ValuesBucket): Promise<number>;
     
        /**
         * Deletes one or more data records from the database.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the data to operate.
         * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
         * @return Returns the number of data records deleted.
         * @systemapi Hide this for inner system use
         */
        delete(URI: string, predicates: DataSharePredicates, callback: AsyncCallback<number>): void;
        delete(URI: string, predicates: DataSharePredicates): Promise<number>;

        /**
         * Queries data in the database.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of data to query.
         * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
         * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
         * @return Returns the query result.
         * @systemapi Hide this for inner system use
         */
        query(URI: string, columns: Array<String>, predicates: DataSharePredicates, callback: AsyncCallback<ResultSet>): void;
        query(URI: string, columns: Array<String>, predicates: DataSharePredicates): Promise<ResultSet>;

        /**
         * Updates data records in the database.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of data to update.
         * @param value Indicates the data to update. This parameter can be null.
         * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
         * @return Returns the number of data records updated.
         * @systemapi Hide this for inner system use
         */
        update(URI: string, values: ValuesBucket, predicates: DataSharePredicates, callback: AsyncCallback<number>): void;
        update(URI: string, values: ValuesBucket, predicates: DataSharePredicates): Promise<number>;

        /**
         * Inserts multiple data records into the database.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the data to operate.
         * @param values Indicates the data records to insert.
         * @return Returns the number of data records inserted.
         * @systemapi Hide this for inner system use
         */
        batchInsert(URI: string, values: Array<ValuesBucket>, callback: AsyncCallback<number>): void;
        batchInsert(URI: string, values: Array<ValuesBucket>): Promise<number>;

        /**
         * Obtains the MIME type of the date specified by the given URI.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the data to operate.
         * @return Returns the MIME type that matches the data specified by uri.
         * @systemapi Hide this for inner system use
         */
        getType(URI: string, callback: AsyncCallback<string>): void;
        getType(URI: string): Promise<string>;

        /**
         * Obtains the MIME types of files supported.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the path of the files to obtain.
         * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
         *                       <p>1. "&ast;/*": Obtains all types supported by Data abilities.
         *                       <p>2. "image/*": Obtains files whose main type is image of any subtype.
         *                       <p>3. "&ast;/jpg": Obtains files whose subtype is JPG of any main type.
         * @return Returns the matched MIME types. If there is no match, {@code null} is returned.
         * @systemapi Hide this for inner system use
         */
        getFileTypes(uri: string,  mimeTypeFilter:string, callback: AsyncCallback<Array<string>>): void;
        getFileTypes(uri: string,  mimeTypeFilter): Promise<Array<string>>;

        /**
         * Converts the given {@code uri} that refers to the Data ability into a normalized {@link ohos.utils.net.Uri}.
         * A normalized URI can be used across devices, persisted, backed up, and restored.
         * <p>To transfer a normalized URI from another environment to the current environment, you should call this
         * method again to re-normalize the URI for the current environment or call {@link #denormalizeUri(Uri)}
         * to convert it to a denormalized URI that can be used only in the current environment.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the {@link ohos.utils.net.Uri} object to normalize.
         * @return Returns the normalized {@code Uri} object if the Data ability supports URI normalization;
         * returns {@code null} otherwise.
         * @throws DataShareRemoteException Throws this exception if the remote process exits.
         * @throws NullPointerException Throws this exception if {@code uri} is null.
         * @see #denormalizeUri
         * @systemapi Hide this for inner system use
         */
        normalizeUri(uri: string, callback: AsyncCallback<string>): void;
        normalizeUri(uri: string): Promise<string>;

        /**
         * Converts the given normalized {@code uri} generated by {@link #normalizeUri(Uri)} into a denormalized one.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the {@link ohos.utils.net.Uri} object to denormalize.
         * @return Returns the denormalized {@code Uri} object if the denormalization is successful; returns the
         * original {@code Uri} passed to this method if there is nothing to do; returns {@code null} if the data
         * identified by the normalized {@code Uri} cannot be found in the current environment.
         * @throws DataShareRemoteException Throws this exception if the remote process exits.
         * @throws NullPointerException Throws this exception if {@code uri} is null.
         * @see #normalizeUri
         * @systemapi Hide this for inner system use
         */
        denormalizeUri(uri: string, callback: AsyncCallback<string>): void;
        denormalizeUri(uri: string): Promise<string>;

        /**
         * Notifies the registered observers of a change to the data resource specified by Uri.
         * @since 9
         * @sysCap SystemCapability.DistributedDataMgr.DataShare.Consumer
         * @param uri Indicates the {@link ohos.utils.net.Uri} object to notifyChange.
         * @systemapi Hide this for inner system use
         */
        notifyChange(URI: string, callback: AsyncCallback<void>): void;
        notifyChange(URI: string): Promise<void>;
    }
}

export default dataShare;
