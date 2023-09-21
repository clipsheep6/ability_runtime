/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "ability_scheduler_proxy.h"

#include "abs_shared_result_set.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "ishared_result_set.h"
#include "pac_map.h"
#include "session_info.h"
#include "values_bucket.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
bool AbilitySchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilitySchedulerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AbilitySchedulerProxy::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &stateInfo,
    sptr<SessionInfo> sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&want);
    data.WriteParcelable(&stateInfo);
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            return;
        }
    }
    if (!SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleShareData(const int32_t &uniqueId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("write interface token failed.");
        return;
    }
    if (!data.WriteInt32(uniqueId)) {
        HILOG_ERROR("uniqueId write failed.");
        return;
    }
    if (!SendRequest(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::SendResult(int requestCode, int resultCode, const Want &resultWant)
{
    uint32_t code = IAbilityScheduler::SEND_RESULT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(requestCode);
    data.WriteInt32(resultCode);
    if (!data.WriteParcelable(&resultWant)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleConnectAbility(const Want &want)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_ABILITY_CONNECT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleDisconnectAbility(const Want &want)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_ABILITY_COMMAND;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!data.WriteBool(restart)) {
        HILOG_ERROR("fail to WriteBool");
        return;
    }
    HILOG_INFO("WriteInt32,startId:%{public}d", startId);
    if (!data.WriteInt32(startId)) {
        HILOG_ERROR("fail to WriteInt32");
        return;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

bool AbilitySchedulerProxy::SchedulePrepareTerminateAbility()
{
    uint32_t code = IAbilityScheduler::SCHEDULE_ABILITY_PREPARE_TERMINATE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("fail to write interface.");
        return false;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return false;
    }
    return reply.ReadBool();
}

void AbilitySchedulerProxy::ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_ABILITY_COMMAND_WINDOW;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!data.WriteParcelable(sessionInfo)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    if (!data.WriteInt32(winCmd)) {
        HILOG_ERROR("fail to WriteInt32");
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleSaveAbilityState()
{
    uint32_t code = IAbilityScheduler::SCHEDULE_SAVE_ABILITY_STATE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::ScheduleRestoreAbilityState(const PacMap &inState)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&inState)) {
        HILOG_ERROR("WriteParcelable error");
        return;
    }
    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> AbilitySchedulerProxy::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> types;
    uint32_t code = IAbilityScheduler::SCHEDULE_GETFILETYPES;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return types;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return types;
    }

    if (!data.WriteString(mimeTypeFilter)) {
        HILOG_ERROR("fail to WriteString mimeTypeFilter");
        return types;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return types;
    }

    if (!reply.ReadStringVector(&types)) {
        HILOG_ERROR("fail to ReadStringVector types");
    }

    return types;
}

/**
 * @brief Opens a file in a specified remote path.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 *  or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the file descriptor.
 */
int AbilitySchedulerProxy::OpenFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_OPENFILE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        HILOG_ERROR("fail to WriteString mode");
        return fd;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return fd;
    }

    fd = reply.ReadFileDescriptor();
    if (fd == -1) {
        HILOG_ERROR("fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int AbilitySchedulerProxy::OpenRawFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_OPENRAWFILE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        HILOG_ERROR("fail to WriteString mode");
        return fd;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return fd;
    }

    if (!reply.ReadInt32(fd)) {
        HILOG_ERROR("fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int AbilitySchedulerProxy::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_INSERT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!value.Marshalling(data)) {
        HILOG_ERROR("fail to WriteParcelable value");
        return index;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value  Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
std::shared_ptr<AppExecFwk::PacMap> AbilitySchedulerProxy::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_CALL;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteString(method)) {
        HILOG_ERROR("fail to WriteString method");
        return nullptr;
    }

    if (!data.WriteString(arg)) {
        HILOG_ERROR("fail to WriteString arg");
        return nullptr;
    }

    if (!data.WriteParcelable(&pacMap)) {
        HILOG_ERROR("fail to WriteParcelable pacMap");
        return nullptr;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::PacMap> result(reply.ReadParcelable<AppExecFwk::PacMap>());
    if (!result) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return nullptr;
    }
    return result;
}

/**
 * @brief Updates data records in the database.
 *
 * @param uri Indicates the path of data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records updated.
 */
int AbilitySchedulerProxy::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_UPDATE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!value.Marshalling(data)) {
        HILOG_ERROR("fail to WriteParcelable value");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return index;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the number of data records deleted.
 */
int AbilitySchedulerProxy::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_DELETE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return index;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return index;
    }

    if (!reply.ReadInt32(index)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return index;
    }

    return index;
}

/**
 * @brief Deletes one or more data records from the database.
 *
 * @param uri Indicates the path of data to query.
 * @param columns Indicates the columns to query. If this parameter is null, all columns are queried.
 * @param predicates Indicates filter criteria. You should define the processing logic when this parameter is null.
 *
 * @return Returns the query result.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilitySchedulerProxy::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_QUERY;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteStringVector(columns)) {
        HILOG_ERROR("fail to WriteStringVector columns");
        return nullptr;
    }

    if (!data.WriteParcelable(&predicates)) {
        HILOG_ERROR("fail to WriteParcelable predicates");
        return nullptr;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return nullptr;
    }
    return OHOS::NativeRdb::ISharedResultSet::ReadFromParcel(reply);
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string AbilitySchedulerProxy::GetType(const Uri &uri)
{
    std::string type;
    uint32_t code = IAbilityScheduler::SCHEDULE_GETTYPE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return type;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return type;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return type;
    }

    type = reply.ReadString();
    if (type.empty()) {
        HILOG_ERROR("fail to ReadString type");
        return type;
    }

    return type;
}

/**
 * @brief Reloads data in the database.
 *
 * @param uri Indicates the position where the data is to reload. This parameter is mandatory.
 * @param extras Indicates the PacMap object containing the additional parameters to be passed in this call. This
 * parameter can be null. If a custom Sequenceable object is put in the PacMap object and will be transferred across
 * processes, you must call BasePacMap.setClassLoader(ClassLoader) to set a class loader for the custom object.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool AbilitySchedulerProxy::Reload(const Uri &uri, const PacMap &extras)
{
    bool ret = false;
    uint32_t code = IAbilityScheduler::SCHEDULE_RELOAD;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return ret;
    }

    if (!data.WriteParcelable(&extras)) {
        HILOG_ERROR("fail to WriteParcelable extras");
        return ret;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return ret;
    }

    ret = reply.ReadBool();
    if (!ret) {
        HILOG_ERROR("fail to ReadBool ret");
        return ret;
    }

    return ret;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int AbilitySchedulerProxy::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;
    uint32_t code = IAbilityScheduler::SCHEDULE_BATCHINSERT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return ret;
    }

    int count = (int)values.size();
    if (!data.WriteInt32(count)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ret;
    }

    for (int i = 0; i < count; i++) {
        if (!values[i].Marshalling(data)) {
            HILOG_ERROR("fail to WriteParcelable ret, index = %{public}d", i);
            return ret;
        }
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }

    if (!reply.ReadInt32(ret)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return ret;
    }

    return ret;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilitySchedulerProxy::ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_REGISTEROBSERVER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        HILOG_ERROR("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return false;
    }
    return true;
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
bool AbilitySchedulerProxy::ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_UNREGISTEROBSERVER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        HILOG_ERROR("%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return false;
    }
    return true;
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
bool AbilitySchedulerProxy::ScheduleNotifyChange(const Uri &uri)
{
    uint32_t code = IAbilityScheduler::SCHEDULE_NOTIFYCHANGE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return false;
    }
    return true;
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
 * any method that is called on the Data ability must require normalization verification and denormalization. The
 * default implementation of this method returns null, indicating that this Data ability does not support URI
 * normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri AbilitySchedulerProxy::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    uint32_t code = IAbilityScheduler::SCHEDULE_NORMALIZEURI;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return urivalue;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return Uri("");
    }
    return *info;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
 * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
 * in the current environment.
 */
Uri AbilitySchedulerProxy::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    uint32_t code = IAbilityScheduler::SCHEDULE_DENORMALIZEURI;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("fail to WriteParcelable uri");
        return urivalue;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        HILOG_ERROR("ReadParcelable value is nullptr.");
        return Uri("");
    }
    return *info;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> AbilitySchedulerProxy::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("AbilitySchedulerProxy::ExecuteBatch start");
    uint32_t code = IAbilityScheduler::SCHEDULE_EXECUTEBATCH;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results;
    results.clear();

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to Writer token");
        return results;
    }

    int count = (int)operations.size();
    if (!data.WriteInt32(count)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to WriteInt32 ret");
        return results;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(operations[i].get())) {
            HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to WriteParcelable ret, index = %{public}d", i);
            return results;
        }
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return results;
    }

    int total = 0;
    if (!reply.ReadInt32(total)) {
        HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch fail to ReadInt32 count %{public}d", total);
        return results;
    }

    for (int i = 0; i < total; i++) {
        std::shared_ptr<AppExecFwk::DataAbilityResult> dataAbilityResult(
            reply.ReadParcelable<AppExecFwk::DataAbilityResult>());
        if (dataAbilityResult == nullptr) {
            HILOG_ERROR("AbilitySchedulerProxy::ExecuteBatch dataAbilityResult is nullptr, index = %{public}d", i);
            return results;
        }
        results.push_back(dataAbilityResult);
    }
    HILOG_INFO("AbilitySchedulerProxy::ExecuteBatch end %{public}d", total);
    return results;
}

void AbilitySchedulerProxy::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    uint32_t code = IAbilityScheduler::CONTINUE_ABILITY;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("ContinueAbility fail to write token");
        return;
    }
    if (!data.WriteString(deviceId)) {
        HILOG_ERROR("ContinueAbility fail to write deviceId");
        return;
    }
    if (!data.WriteUint32(versionCode)) {
        HILOG_ERROR("ContinueAbility fail to write versionCode");
        return;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::NotifyContinuationResult(int32_t result)
{
    uint32_t code = IAbilityScheduler::NOTIFY_CONTINUATION_RESULT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("NotifyContinuationResult fail to write token");
        return;
    }
    if (!data.WriteInt32(result)) {
        HILOG_ERROR("NotifyContinuationResult fail to write result");
        return;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    uint32_t code = IAbilityScheduler::DUMP_ABILITY_RUNNER_INNER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("DumpAbilityRunner fail to write token");
        return;
    }

    if (!data.WriteStringVector(params)) {
        HILOG_ERROR("DumpAbilityRunner fail to write params");
        return;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
    }
}

void AbilitySchedulerProxy::CallRequest()
{
    HILOG_INFO("AbilitySchedulerProxy::CallRequest start");
    uint32_t code = IAbilityScheduler::REQUEST_CALL_REMOTE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!SendRequest(code, data, reply, option)) {
        HILOG_ERROR("failed to SendRequest.");
        return;
    }

    HILOG_INFO("AbilitySchedulerProxy::CallRequest end");
}

bool AbilitySchedulerProxy::SendRequest(uint32_t code, MessageParcel &data,
                                        MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t err = remote->SendRequest(code, data, reply, option);
    if (err != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, err is %{public}d.", code, err);
        return false;
    }
    return true;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilitySchedulerProxy::BlockAbility()
{
    HILOG_INFO("AbilitySchedulerProxy::BlockAbility start");
    int ret = -1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }
    if (SendRequest(IAbilityScheduler::BLOCK_ABILITY_INNER, data, reply, option) == false) {
        HILOG_ERROR("SendRequest failed.");
        return ret;
    }
    if (!reply.ReadInt32(ret)) {
        HILOG_ERROR("fail to ReadInt32 ret");
        return ret;
    }
    return ret;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
