/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_THREAD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_THREAD_H

#include "ability_scheduler_stub.h"
#include "context.h"
#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
using AbilitySchedulerStub = OHOS::AAFwk::AbilitySchedulerStub;
class AbilityHandler;
class AbilityLocalRecord;
class AbilityThread : public AbilitySchedulerStub {
public:
    /**
     * @brief Default constructor used to create a AbilityThread instance.
     */
    AbilityThread() = default;
    virtual ~AbilityThread() = default;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     * @param appContext the AbilityRuntime context
     */
    static void AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
        const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param appContext the AbilityRuntime context
     */
    static void AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AbilityRuntime::Context> &appContext);

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param mainRunner The runner which main_thread holds.
     * @param appContext the AbilityRuntime context
     */
    virtual void Attach(const std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
        const std::shared_ptr<AbilityRuntime::Context> &appContext) = 0;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord Indicates the abilityRecord.
     * @param appContext the AbilityRuntime context
     */
    virtual void Attach(const std::shared_ptr<OHOSApplication> &application,
        const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AbilityRuntime::Context> &appContext) = 0;

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     * @param config Indicates the updated configuration information
     */
    virtual void ScheduleUpdateConfiguration(const Configuration &config) = 0;

    /**
     * @brief notify this ability current memory level.
     * @param level Current memory level
     */
    virtual void NotifyMemoryLevel(int32_t level) = 0;

    /**
     * @brief  Provide operating system AbilityTransaction information to the observer
     * @param want Indicates the structure containing Transaction information about the ability.
     * @param targetState Indicates the lifecycle state.
     * @param sessionInfo Indicates the session info.
     */
    void ScheduleAbilityTransaction(
        const Want &want, const LifeCycleStateInfo &targetState, sptr<SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Provide operating system ShareData information to the observer
     * @param requestCode Indicates the Ability request code.
     */
    void ScheduleShareData(const int32_t &requestCode) override;

    /**
     * @brief Provide operating system ConnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleConnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system DisconnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleDisconnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system CommandAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void ScheduleCommandAbility(const Want &want, bool restart, int startId) override;

    /**
     * @brief Schedule Command AbilityWindow
     * @param want The Want object to command to.
     * @param sessionInfo Indicates the session info.
     * @param winCmd Indicates the WindowCommand of winCmd
     */
    void ScheduleCommandAbilityWindow(
        const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    /**
     * @brief Provide operating system PrepareTerminateAbility information to the observer
     */
    bool SchedulePrepareTerminateAbility() override;

    /**
     * @brief Provide operating system SaveabilityState information to the observer
     */
    void ScheduleSaveAbilityState() override;

    /**
     * @brief Provide operating system RestoreAbilityState information to the observer
     * @param state Indicates resotre ability state used to dispatchRestoreAbilityState.
     */
    void ScheduleRestoreAbilityState(const PacMap &state) override;

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
     * receive the result set in the current method. This method can be called only after the ability has been
     * initialized.
     * @param requestCode Indicates the request code for send.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param resultData Indicates the data returned after the ability is destroyed. You can define the data returned.
     * This parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief Obtains the MIME types of files supported.
     * @param uri Indicates the path of the files to obtain.
     * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
     * @return Returns the matched MIME types. If there is no match, null is returned.
     */
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override;

    /**
     * @brief Opens a file in a specified remote path.
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
     * or "rwt" for read and write access that truncates any existing file.
     * @return Returns the file descriptor.
     */
    int OpenFile(const Uri &uri, const std::string &mode) override;

    /**
     * @brief This is like openFile, open a file that need to be able to return sub-sections of files that often assets
     * inside of their .hap.
     * @param uri Indicates the path of the file to open.
     * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
     * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
     * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
     * data, or "rwt" for read and write access that truncates any existing file.
     * @return Returns the RawFileDescriptor object containing file descriptor.
     */
    int OpenRawFile(const Uri &uri, const std::string &mode) override;

    /**
     * @brief Inserts a single data record into the database.
     * @param uri Indicates the path of the data to operate.
     * @param value Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
     * @return Returns the index of the inserted data record.
     */
    int Insert(const Uri &uri, const NativeRdb::ValuesBucket &value) override;

    /**
     * @brief Calls the method of the Data ability.
     * @param uri Indicates the path of the data to operate.
     * @param method Indicates the method to call
     * @param arg Indicates the parameter of the String type.
     * @param pacMap Defines a PacMap object for storing a series of values.
     * @return Returns the call result.
     */
    std::shared_ptr<AppExecFwk::PacMap> Call(
        const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap) override;

    /**
     * @brief Updates data records in the database.
     * @param uri Indicates the path of data to update.
     * @param value Indicates the data to update. This parameter can be null.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the number of data records updated.
     */
    int Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
        const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Deletes one or more data records from the database.
     * @param uri Indicates the path of the data to operate.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the number of data records deleted.
     */
    int Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Deletes one or more data records from the database.
     * @param uri Indicates the path of data to query.
     * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
     * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
     * @return Returns the query result.
     */
    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(
        const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates) override;

    /**
     * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
     * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
     * @param uri Indicates the URI of the data.
     * @return Returns the MIME type that matches the data specified by uri.
     */
    std::string GetType(const Uri &uri) override;

    /**
     * @brief Reloads data in the database.
     * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
     * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
     * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
     * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
     * @return Returns true if the data is successfully reloaded; returns false otherwise.
     */
    bool Reload(const Uri &uri, const PacMap &extras) override;

    /**
     * @brief Inserts multiple data records into the database.
     * @param uri Indicates the path of the data to operate.
     * @param values Indicates the data records to insert.
     * @return Returns the number of data records inserted.
     */
    int BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) override;

    /**
     * @brief continue ability to target device.
     * @param deviceId target deviceId
     * @param versionCode Target bundle version.
     */
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) override;

    /**
     * @brief notify this ability continuation result.
     * @param result: Continuation result
     */
    void NotifyContinuationResult(int32_t result) override;

    /**
     * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
     * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
     * context has changed. If you implement URI normalization for a Data ability, you must also implement
     * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
     * any method that is called on the Data ability must require normalization verification and denormalization. The
     * default implementation of this method returns null, indicating that this Data ability does not support URI
     * normalization.
     * @param uri Indicates the Uri object to normalize.
     * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
     */
    Uri NormalizeUri(const Uri &uri) override;

    /**
     * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
     * The default implementation of this method returns the original URI passed to it.
     * @param uri uri Indicates the Uri object to denormalize.
     * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
     * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
     * in the current environment.
     */
    Uri DenormalizeUri(const Uri &uri) override;

    /**
     * @brief Registers an observer to DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     */
    bool ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    /**
     * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
     * @param uri Indicates the path of the data to operate.
     * @param dataObserver Indicates the IDataAbilityObserver object.
     */
    bool ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override;

    /**
     * @brief Notifies the registered observers of a change to the data resource specified by Uri.
     * @param uri Indicates the path of the data to operate.
     */
    bool ScheduleNotifyChange(const Uri &uri) override;

    /**
     * @brief Dump ability runner info.
     * @param params Indicates the params
     * @param info ability runner info.
     */
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    /**
     * @brief Call Request
     */
    void CallRequest() override;

    void OnExecuteIntent(const Want &want) override;

    /**
     * @brief Execute Batch
     * @param operations Indicates the operations
     */
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations) override;

    /**
     * @brief Dump ability runner info.
     * @param want Create modalUIExtension with want object.
     */
    int CreateModalUIExtension(const Want &want) override;

    /**
     * @brief Update sessionToken.
     * @param sessionToken The token of session.
     */
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override;

    /**
     * Requesting the component to end itself.
     */
    virtual void RequestTerminateSelf() {}

#ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * @brief Block ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int BlockAbility() override;
#endif
    sptr<IRemoteObject> token_;
    std::shared_ptr<AbilityHandler> abilityHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_THREAD_H
