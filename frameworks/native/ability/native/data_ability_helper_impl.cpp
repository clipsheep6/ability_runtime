/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "data_ability_helper_impl.h"

#include "ability_scheduler_interface.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "hitrace_meter.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
std::string SchemeOhos = "dataability";
using IAbilityScheduler = OHOS::AAFwk::IAbilityScheduler;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri,
    const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const std::shared_ptr<Uri> &uri, const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    token_ = context->GetToken();
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<Context> &context)
{
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri,
    const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy)
{
    token_ = token;
    uri_ = uri;
    tryBind_ = false;
    dataAbilityProxy_ = dataAbilityProxy;
    isSystemCaller_ = true;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const sptr<IRemoteObject> &token)
{
    token_ = token;
    isSystemCaller_ = true;
}

void DataAbilityHelperImpl::AddDataAbilityDeathRecipient(const sptr<IRemoteObject> &token)
{
    if (token != nullptr && callerDeathRecipient_ != nullptr) {
        HILOG_INFO("Remove death recipient.");
        token->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        std::weak_ptr<DataAbilityHelperImpl> thisWeakPtr(shared_from_this());
        callerDeathRecipient_ =
            new DataAbilityDeathRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto DataAbilityHelperImpl = thisWeakPtr.lock();
                if (DataAbilityHelperImpl) {
                    DataAbilityHelperImpl->OnSchedulerDied(remote);
                }
            });
    }
    HILOG_INFO("Add death recipient.");
    if (token == nullptr || !token->AddDeathRecipient(callerDeathRecipient_)) {
        HILOG_ERROR("AddDeathRecipient failed.");
    }
}

void DataAbilityHelperImpl::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("On scheduler died.");
    std::lock_guard<std::mutex> guard(lock_);
    auto object = remote.promote();
    object = nullptr;
    dataAbilityProxy_ = nullptr;
    uri_ = nullptr;
}

/**
 * @brief Creates a DataAbilityHelperImpl instance without specifying the Uri based on the given Context.
 *
 * @param context Indicates the Context object on OHOS.
 *
 * @return Returns the created DataAbilityHelperImpl instance where Uri is not specified.
 */
std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        HILOG_ERROR("Input param invalid, context is nullptr.");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context);
    if (ptrDataAbilityHelperImpl == nullptr) {
        HILOG_ERROR("New DataAbilityHelperImpl failed.");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelperImpl instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 * @param tryBind Specifies whether the exit of the corresponding Data ability process causes the exit of the
 * client process.
 *
 * @return Returns the created DataAbilityHelperImpl instance.
 */
std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    if (context == nullptr) {
        HILOG_ERROR("Input param invalid, context is nullptr.");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        HILOG_ERROR("uri is invalid.");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Acquire data ability failed.");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelperImpl == nullptr) {
        HILOG_ERROR("New DataAbilityHelperImpl failed.");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelperImpl instance.
 *
 * @param context Indicates the Context object on OHOS.
 * @param uri Indicates the database table or disk file to operate.
 * @param tryBind Specifies whether the exit of the corresponding Data ability process causes the exit of the
 * client process.
 *
 * @return Returns the created DataAbilityHelperImpl instance.
 */
std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    if (context == nullptr) {
        HILOG_ERROR("Input param invalid, context is nullptr.");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        HILOG_ERROR("uri is invalid.");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Acquire data ability failed.");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelperImpl == nullptr) {
        HILOG_ERROR("New DataAbilityHelperImpl failed.");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}

/**
 * @brief Creates a DataAbilityHelperImpl instance without specifying the Uri based.
 *
 * @param token Indicates the System token.
 *
 * @return Returns the created DataAbilityHelperImpl instance where Uri is not specified.
 */
std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_ERROR("Input param invalid, token is nullptr.");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(token);
    if (ptrDataAbilityHelperImpl == nullptr) {
        HILOG_ERROR("New DataAbilityHelperImpl failed.");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}

/**
 * @brief You can use this method to specify the Uri of the data to operate and set the binding relationship
 * between the ability using the Data template (Data ability for short) and the associated client process in
 * a DataAbilityHelperImpl instance.
 *
 * @param token Indicates the System token.
 * @param uri Indicates the database table or disk file to operate.
 *
 * @return Returns the created DataAbilityHelperImpl instance.
 */
std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri)
{
    if (token == nullptr) {
        HILOG_ERROR("Input param invalid, token is nullptr.");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        HILOG_ERROR("uri is invalid.");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), false, token);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Acquire data ability failed.");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(token, uri, dataAbilityProxy);
    if (ptrDataAbilityHelperImpl == nullptr) {
        HILOG_ERROR("New DataAbilityHelperImpl failed.");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}

/**
 * @brief Releases the client resource of the Data ability.
 * You should call this method to releases client resource after the data operations are complete.
 *
 * @return Returns true if the resource is successfully released; returns false otherwise.
 */
bool DataAbilityHelperImpl::Release()
{
    if (uri_ == nullptr) {
        HILOG_ERROR("Release failed, uri_ is nullptr.");
        return false;
    }

    int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Release data ability failed, err = %{public}d.", err);
        return false;
    }

    return true;
}

/**
 * @brief Obtains the MIME types of files supported.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME types of the files to obtain. This parameter cannot be null.
 *
 * @return Returns the matched MIME types. If there is no match, null is returned.
 */
std::vector<std::string> DataAbilityHelperImpl::GetFileTypes(Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> matchedMIMEs;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return matchedMIMEs;
    }

    matchedMIMEs = dataAbilityProxy->GetFileTypes(uri, mimeTypeFilter);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return matchedMIMEs size: %{public}zu.", matchedMIMEs.size());
    return matchedMIMEs;
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
int DataAbilityHelperImpl::OpenFile(Uri &uri, const std::string &mode)
{
    int fd = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return fd;
    }

    fd = dataAbilityProxy->OpenFile(uri, mode);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return fd: %{public}d.", fd);
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
int DataAbilityHelperImpl::OpenRawFile(Uri &uri, const std::string &mode)
{
    int fd = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return fd;
    }

    fd = dataAbilityProxy->OpenRawFile(uri, mode);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return fd: %{public}d.", fd);
    return fd;
}

/**
 * @brief Inserts a single data record into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param value Indicates the data record to insert. If this parameter is null, a blank row will be inserted.
 *
 * @return Returns the index of the inserted data record.
 */
int DataAbilityHelperImpl::Insert(Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return index;
    }

    index = dataAbilityProxy->Insert(uri, value);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return index: %{public}d.", index);
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityHelperImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return result;
    }

    result = dataAbilityProxy->Call(uri, method, arg, pacMap);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return result is or not nullptr: %{public}d.", result == nullptr);
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
int DataAbilityHelperImpl::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return index;
    }

    index = dataAbilityProxy->Update(uri, value, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return index: %{public}d.", index);
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
int DataAbilityHelperImpl::Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return index;
    }

    index = dataAbilityProxy->Delete(uri, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return index: %{public}d.", index);
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
std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityHelperImpl::Query(
    Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return resultset;
    }

    resultset = dataAbilityProxy->Query(uri, columns, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return resultset is or not nullptr: %{public}d.", resultset == nullptr);
    return resultset;
}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string DataAbilityHelperImpl::GetType(Uri &uri)
{
    std::string type;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return type;
    }

    type = dataAbilityProxy->GetType(uri);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return type: %{public}s.", type.c_str());
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
bool DataAbilityHelperImpl::Reload(Uri &uri, const PacMap &extras)
{
    bool ret = false;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return ret;
    }

    ret = dataAbilityProxy->Reload(uri, extras);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return ret: %{public}d.", ret);
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
int DataAbilityHelperImpl::BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return ret;
    }

    ret = dataAbilityProxy->BatchInsert(uri, values);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return ret: %{public}d.", ret);
    return ret;
}

bool DataAbilityHelperImpl::CheckUriParam(const Uri &uri)
{
    Uri checkUri(uri.ToString());
    if (!CheckOhosUri(checkUri)) {
        HILOG_ERROR("Check ohos uri failed, uri: %{public}s.", uri.ToString().c_str());
        return false;
    }

    // do not directly use uri_ here, otherwise, it will probably crash.
    std::vector<std::string> segments;
    {
        std::lock_guard<std::mutex> guard(lock_);
        if (!uri_) {
            HILOG_INFO("uri_ is nullptr, no need check.");
            return true;
        }

        if (!CheckOhosUri(*uri_)) {
            HILOG_ERROR("Check ohos uri failed, uri_: %{public}s.", uri_->ToString().c_str());
            return false;
        }

        uri_->GetPathSegments(segments);
    }

    std::vector<std::string> checkSegments;
    checkUri.GetPathSegments(checkSegments);
    if (checkSegments.empty() || segments.empty() || checkSegments[0] != segments[0]) {
        HILOG_ERROR("The dataability in uri doesn't equal the one in uri_.");
        return false;
    }

    return true;
}

bool DataAbilityHelperImpl::CheckOhosUri(const Uri &uri)
{
    Uri checkUri(uri.ToString());
    if (checkUri.GetScheme() != SchemeOhos) {
        HILOG_ERROR("Input uri is not a dataability one, uri: %{public}s.", uri.ToString().c_str());
        return false;
    }

    std::vector<std::string> segments;
    checkUri.GetPathSegments(segments);
    if (segments.empty()) {
        HILOG_ERROR("There is no segments in the uri, uri: %{public}s.", uri.ToString().c_str());
        return false;
    }

    if (checkUri.GetPath() == "") {
        HILOG_ERROR("The path in the uri is empty, uri: %{public}s.", uri.ToString().c_str());
        return false;
    }

    return true;
}

/**
 * @brief Registers an observer to DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataAbilityHelperImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriAndDataObserver(uri, dataObserver)) {
        HILOG_ERROR("RegisterObserver param is invalid.");
        return;
    }

    Uri tmpUri(uri.ToString());

    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
            registerMap_.emplace(dataObserver, dataAbilityProxy);
            uriMap_.emplace(dataObserver, tmpUri.GetPath());
        } else {
            auto path = uriMap_.find(dataObserver);
            if (path == uriMap_.end()) {
                return;
            }
            if (path->second != tmpUri.GetPath()) {
                HILOG_ERROR("Input uri's path is not equal the one the observer used.");
                return;
            }
            dataAbilityProxy = dataability->second;
        }
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("dataAbilityProxy is nullptr.");
        registerMap_.erase(dataObserver);
        uriMap_.erase(dataObserver);
        return;
    }
    dataAbilityProxy->ScheduleRegisterObserver(uri, dataObserver);
}

/**
 * @brief Deregisters an observer used for DataObsMgr specified by the given Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 * @param dataObserver, Indicates the IDataAbilityObserver object.
 */
void DataAbilityHelperImpl::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriAndDataObserver(uri, dataObserver)) {
        HILOG_ERROR("UnregisterObserver param is invalid.");
        return;
    }

    Uri tmpUri(uri.ToString());
    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            return;
        }
        auto path = uriMap_.find(dataObserver);
        if (path == uriMap_.end()) {
            return;
        }
        if (path->second != tmpUri.GetPath()) {
            HILOG_ERROR("Input uri's path is not equal the one the observer used.");
            return;
        }
        dataAbilityProxy = dataability->second;
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("dataAbilityProxy is nullptr.");
        return;
    }

    dataAbilityProxy->ScheduleUnregisterObserver(uri, dataObserver);
    ReleaseDataAbility(dataAbilityProxy_);
    if (uri_ == nullptr) {
        dataAbilityProxy_ = nullptr;
    }
    registerMap_.erase(dataObserver);
    uriMap_.erase(dataObserver);
}

/**
 * @brief Notifies the registered observers of a change to the data resource specified by Uri.
 *
 * @param uri, Indicates the path of the data to operate.
 */
void DataAbilityHelperImpl::NotifyChange(const Uri &uri)
{
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return;
    }

    dataAbilityProxy->ScheduleNotifyChange(uri);
    ReleaseDataAbility(dataAbilityProxy);
}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to any
 * method that is called on the Data ability must require normalization verification and denormalization. The default
 * implementation of this method returns null, indicating that this Data ability does not support URI normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri DataAbilityHelperImpl::NormalizeUri(Uri &uri)
{
    Uri urivalue("");
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return urivalue;
    }

    urivalue = dataAbilityProxy->NormalizeUri(uri);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return uri: %{public}s.", urivalue.ToString().c_str());
    return urivalue;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed to
 * this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found in the
 * current environment.
 */
Uri DataAbilityHelperImpl::DenormalizeUri(Uri &uri)
{
    Uri urivalue("");
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return urivalue;
    }

    urivalue = dataAbilityProxy->DenormalizeUri(uri);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return uri: %{public}s.", urivalue.ToString().c_str());
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityHelperImpl::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri, false);
    if (dataAbilityProxy == nullptr) {
        HILOG_ERROR("Get data ability proxy failed.");
        return results;
    }

    results = dataAbilityProxy->ExecuteBatch(operations);

    ReleaseDataAbility(dataAbilityProxy);
    HILOG_INFO("Return results size: %{public}zu.", results.size());
    return results;
}

sptr<AAFwk::IAbilityScheduler> DataAbilityHelperImpl::GetDataAbilityProxy(const Uri &uri, bool addDeathRecipient)
{
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("Check uri param failed.");
        return nullptr;
    }
    // if uri_ is nullptr, it indicates the operation(such as insert, delete and so on) is temporary,
    // so, we need acquire the dataability before the operation.
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        HILOG_INFO("The uri_ is nullptr, need acquire data ability.");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        if (dataAbilityProxy == nullptr) {
            HILOG_ERROR("Acquire data ability failed.");
            return nullptr;
        }
        if (addDeathRecipient && isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    return dataAbilityProxy;
}

void DataAbilityHelperImpl::ReleaseDataAbility(sptr<AAFwk::IAbilityScheduler> dataAbilityProxy)
{
    // if uri_ is nullptr, it indicates the operation(such as insert, delete and so on) is temporary,
    // so, we need release the dataability after the operation.
    HILOG_INFO("ReleaseDataAbility start.");
    if (!uri_ && dataAbilityProxy && token_) {
        int ret = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        HILOG_INFO("Release data ability, ret: %{public}d.", ret);
    }
    HILOG_INFO("ReleaseDataAbility end.");
}

bool DataAbilityHelperImpl::CheckUri(const std::shared_ptr<Uri> &uri)
{
    if (uri == nullptr) {
        HILOG_ERROR("Input param invalid, uri is nullptr.");
        return false;
    }

    if (uri->GetScheme() != SchemeOhos) {
        HILOG_ERROR("Input param invalid, the uri is not dataability, Scheme: %{private}s.", uri->GetScheme().c_str());
        return false;
    }

    return true;
}

bool DataAbilityHelperImpl::CheckUriAndDataObserver(const Uri &uri,
    const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriParam(uri)) {
        HILOG_ERROR("Check uri param failed.");
        return false;
    }

    if (dataObserver == nullptr) {
        HILOG_ERROR("Input param invalid, dataObserver is nullptr.");
        return false;
    }

    return true;
}

void DataAbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("recv DataAbilityDeathRecipient death notice.");
    if (handler_) {
        handler_(remote);
    }
    HILOG_INFO("OnRemoteDied end.");
}

DataAbilityDeathRecipient::DataAbilityDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataAbilityDeathRecipient::~DataAbilityDeathRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS

