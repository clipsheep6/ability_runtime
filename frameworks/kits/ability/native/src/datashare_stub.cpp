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

#include "datashare_stub.h"

#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "ishared_result_set.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
DataShareStub::DataShareStub()
{
    stubFuncMap_[CMD_GET_FILE_TYPES] = &DataShareStub::CmdGetFileTypes;
    stubFuncMap_[CMD_OPEN_FILE] = &DataShareStub::CmdOpenFile;
    stubFuncMap_[CMD_OPEN_RAW_FILE] = &DataShareStub::CmdOpenRawFile;
    stubFuncMap_[CMD_INSERT] = &DataShareStub::CmdInsert;
    stubFuncMap_[CMD_UPDATE] = &DataShareStub::CmdUpdate;
    stubFuncMap_[CMD_DELETE] = &DataShareStub::CmdDelete;
    stubFuncMap_[CMD_QUERY] = &DataShareStub::CmdQuery;
    stubFuncMap_[CMD_GET_TYPE] = &DataShareStub::CmdGetType;
    stubFuncMap_[CMD_BATCH_INSERT] = &DataShareStub::CmdBatchInsert;
    stubFuncMap_[CMD_REGISTER_OBSERVER] = &DataShareStub::CmdRegisterObserver;
    stubFuncMap_[CMD_UNREGISTER_OBSERVER] = &DataShareStub::CmdUnregisterObserver;
    stubFuncMap_[CMD_NOTIFY_CHANGE] = &DataShareStub::CmdNotifyChange;
    stubFuncMap_[CMD_NORMALIZE_URI] = &DataShareStub::CmdNormalizeUri;
    stubFuncMap_[CMD_DENORMALIZE_URI] = &DataShareStub::CmdDenormalizeUri;
    stubFuncMap_[CMD_EXECUTE_BATCH] = &DataShareStub::CmdExecuteBatch;
}

DataShareStub::~DataShareStub()
{
    stubFuncMap_.clear();
}

int DataShareStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    HILOG_INFO("%{public}s Received stub message: %{public}d", __func__, code);
    std::u16string descriptor = DataShareStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    const auto &itFunc = stubFuncMap_.find(code);
    if (itFunc != stubFuncMap_.end()) {
        return (this->*(itFunc->second))(data, reply);
    }

    HILOG_INFO("%{public}s remote request unhandled: %{public}d", __func__, code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

ErrCode DataShareStub::CmdGetFileTypes(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mimeTypeFilter = data.ReadString();
    if (mimeTypeFilter.empty()) {
        HILOG_ERROR("DataShareStub mimeTypeFilter is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> types = GetFileTypes(*uri, mimeTypeFilter);
    if (!reply.WriteStringVector(types)) {
        HILOG_ERROR("fail to WriteStringVector types");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdOpenFile(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        HILOG_ERROR("DataShareStub mode is nullptr");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenFile(*uri, mode);
    if (fd < 0) {
        HILOG_ERROR("OpenFile fail, fd is %{pubilc}d", fd);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteFileDescriptor(fd)) {
        HILOG_ERROR("fail to WriteFileDescriptor fd");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdOpenRawFile(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string mode = data.ReadString();
    if (mode.empty()) {
        HILOG_ERROR("DataShareStub mode is nullptr");
        return ERR_INVALID_VALUE;
    }
    int fd = OpenRawFile(*uri, mode);
    if (!reply.WriteInt32(fd)) {
        HILOG_ERROR("fail to WriteInt32 fd");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdInsert(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Insert(*uri, *value);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("DataShareStub::CmdInsertInner end");
    return NO_ERROR;
}

ErrCode DataShareStub::CmdUpdate(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
    if (value == nullptr) {
        HILOG_ERROR("ReadParcelable value is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Update(*uri, *value, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdDelete(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    int index = Delete(*uri, *predicates);
    if (!reply.WriteInt32(index)) {
        HILOG_ERROR("fail to WriteInt32 index");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdQuery(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::vector<std::string> columns;
    if (!data.ReadStringVector(&columns)) {
        HILOG_ERROR("fail to ReadStringVector columns");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates(
        data.ReadParcelable<NativeRdb::DataAbilityPredicates>());
    if (predicates == nullptr) {
        HILOG_ERROR("ReadParcelable predicates is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto resultSet = Query(*uri, columns, *predicates);
    if (resultSet == nullptr) {
        HILOG_ERROR("fail to WriteParcelable resultSet");
        return ERR_INVALID_VALUE;
    }
    auto result = NativeRdb::ISharedResultSet::WriteToParcel(std::move(resultSet), reply);
    if (result == nullptr) {
        HILOG_ERROR("!resultSet->Marshalling(reply)");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("DataShareStub::CmdQueryInner end");
    return NO_ERROR;
}

ErrCode DataShareStub::CmdGetType(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string type = GetType(*uri);
    if (!reply.WriteString(type)) {
        HILOG_ERROR("fail to WriteString type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdBatchInsert(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    int count = 0;
    if (!data.ReadInt32(count)) {
        HILOG_ERROR("fail to ReadInt32 index");
        return ERR_INVALID_VALUE;
    }

    std::vector<NativeRdb::ValuesBucket> values;
    for (int i = 0; i < count; i++) {
        std::unique_ptr<NativeRdb::ValuesBucket> value(data.ReadParcelable<NativeRdb::ValuesBucket>());
        if (value == nullptr) {
            HILOG_ERROR("DataShareStub value is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        values.emplace_back(*value);
    }

    int ret = BatchInsert(*uri, values);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}


ErrCode DataShareStub::CmdRegisterObserver(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<AAFwk::IDataAbilityObserver>(data.ReadParcelable<IRemoteObject>());
    if (obServer == nullptr) {
        HILOG_ERROR("DataShareStub obServer is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = RegisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdUnregisterObserver(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto obServer = iface_cast<AAFwk::IDataAbilityObserver>(data.ReadParcelable<IRemoteObject>());
    if (obServer == nullptr) {
        HILOG_ERROR("DataShareStub obServer is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = UnregisterObserver(*uri, obServer);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdNotifyChange(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool ret = NotifyChange(*uri);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdNormalizeUri(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = NormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        HILOG_ERROR("fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdDenormalizeUri(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (uri == nullptr) {
        HILOG_ERROR("DataShareStub uri is nullptr");
        return ERR_INVALID_VALUE;
    }

    Uri ret("");
    ret = DenormalizeUri(*uri);
    if (!reply.WriteParcelable(&ret)) {
        HILOG_ERROR("fail to WriteParcelable type");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode DataShareStub::CmdExecuteBatch(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("DataShareStub::CmdExecuteBatchInner start");
    int count = 0;
    if (!data.ReadInt32(count)) {
        HILOG_ERROR("DataShareStub::CmdExecuteBatchInner fail to ReadInt32 count");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("DataShareStub::CmdExecuteBatchInner count:%{public}d", count);
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> operations;
    for (int i = 0; i < count; i++) {
        AppExecFwk::DataAbilityOperation *operation = data.ReadParcelable<AppExecFwk::DataAbilityOperation>();
        if (operation == nullptr) {
            HILOG_ERROR("DataShareStub::CmdExecuteBatchInner operation is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        std::shared_ptr<AppExecFwk::DataAbilityOperation> dataAbilityOperation(operation);
        operations.push_back(dataAbilityOperation);
    }

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results = ExecuteBatch(operations);
    int total = results.size();
    if (!reply.WriteInt32(total)) {
        HILOG_ERROR("DataShareStub::CmdExecuteBatchInner fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("DataShareStub::CmdExecuteBatchInner total:%{public}d", total);
    for (int i = 0; i < total; i++) {
        if (results[i] == nullptr) {
            HILOG_ERROR("DataShareStub::CmdExecuteBatchInner results[i] is nullptr, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
        if (!reply.WriteParcelable(results[i].get())) {
            HILOG_ERROR(
                "DataShareStub::CmdExecuteBatchInner fail to WriteParcelable operation, index = %{public}d", i);
            return ERR_INVALID_VALUE;
        }
    }
    HILOG_INFO("DataShareStub::CmdExecuteBatchInner end");
    return NO_ERROR;
}
} // namespace AAFwk
} // namespace OHOS
