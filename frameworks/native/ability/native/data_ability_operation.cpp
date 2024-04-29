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

#include "data_ability_operation.h"

#include "data_ability_predicates.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
DataAbilityOperation::DataAbilityOperation(
    const std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, const std::shared_ptr<Uri> &withUri)
{
    uri_ = withUri;
    if (dataAbilityOperation != nullptr) {
        type_ = dataAbilityOperation->type_;
        valuesBucket_ = dataAbilityOperation->valuesBucket_;
        expectedCount_ = dataAbilityOperation->expectedCount_;
        dataAbilityPredicates_ = dataAbilityOperation->dataAbilityPredicates_;
        valuesBucketReferences_ = dataAbilityOperation->valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = dataAbilityOperation->dataAbilityPredicatesBackReferences_;
        interrupted_ = dataAbilityOperation->interrupted_;
    } else {
        type_ = 0;
        expectedCount_ = 0;
        valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>();
        dataAbilityPredicates_ = std::make_shared<NativeRdb::DataAbilityPredicates>();
        valuesBucketReferences_ = std::make_shared<NativeRdb::ValuesBucket>();
        dataAbilityPredicatesBackReferences_.clear();
        interrupted_ = false;
    }
}
DataAbilityOperation::DataAbilityOperation(Parcel &in)
{
    ReadFromParcel(in);
}
DataAbilityOperation::DataAbilityOperation(const std::shared_ptr<DataAbilityOperationBuilder> &builder)
{
    if (builder != nullptr) {
        type_ = builder->type_;
        uri_ = builder->uri_;
        valuesBucket_ = builder->valuesBucket_;
        expectedCount_ = builder->expectedCount_;
        dataAbilityPredicates_ = builder->dataAbilityPredicates_;
        valuesBucketReferences_ = builder->valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = builder->dataAbilityPredicatesBackReferences_;
        interrupted_ = builder->interrupted_;
    }
}

DataAbilityOperation::DataAbilityOperation()
{
    type_ = 0;
    uri_ = nullptr;
    expectedCount_ = 0;
    valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>();
    dataAbilityPredicates_ = std::make_shared<NativeRdb::DataAbilityPredicates>();
    valuesBucketReferences_ = std::make_shared<NativeRdb::ValuesBucket>();
    dataAbilityPredicatesBackReferences_.clear();
    interrupted_ = false;
}

DataAbilityOperation::~DataAbilityOperation()
{
    dataAbilityPredicatesBackReferences_.clear();
}

bool DataAbilityOperation::operator==(const DataAbilityOperation &other) const
{
    if (type_ != other.type_) {
        return false;
    }
    if ((uri_ != nullptr) && (other.uri_ != nullptr) && (uri_->ToString() != other.uri_->ToString())) {
        return false;
    }
    if (expectedCount_ != other.expectedCount_) {
        return false;
    }
    if (valuesBucket_ != other.valuesBucket_) {
        return false;
    }
    if (dataAbilityPredicates_ != other.dataAbilityPredicates_) {
        return false;
    }
    if (valuesBucketReferences_ != other.valuesBucketReferences_) {
        return false;
    }
    size_t backReferencesCount = dataAbilityPredicatesBackReferences_.size();
    size_t otherBackReferencesCount = other.dataAbilityPredicatesBackReferences_.size();
    if (backReferencesCount != otherBackReferencesCount) {
        return false;
    }

    std::map<int, int>::const_iterator it = dataAbilityPredicatesBackReferences_.begin();
    while (it != dataAbilityPredicatesBackReferences_.end()) {
        std::map<int, int>::const_iterator otherIt = other.dataAbilityPredicatesBackReferences_.find(it->first);
        if (otherIt != other.dataAbilityPredicatesBackReferences_.end()) {
            if (otherIt->second != it->second) {
                return false;
            }
        } else {
            return false;
        }
        it++;
    }

    if (interrupted_ != other.interrupted_) {
        return false;
    }
    return true;
}

DataAbilityOperation &DataAbilityOperation::operator=(const DataAbilityOperation &other)
{
    if (this != &other) {
        type_ = other.type_;
        uri_ = other.uri_;
        expectedCount_ = other.expectedCount_;
        valuesBucket_ = other.valuesBucket_;
        dataAbilityPredicates_ = other.dataAbilityPredicates_;
        valuesBucketReferences_ = other.valuesBucketReferences_;
        dataAbilityPredicatesBackReferences_ = other.dataAbilityPredicatesBackReferences_;
        interrupted_ = other.interrupted_;
    }
    return *this;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewInsertBuilder(const std::shared_ptr<Uri> &uri)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewInsertBuilder start");
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewInsertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_INSERT, uri);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewInsertBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewUpdateBuilder(const std::shared_ptr<Uri> &uri)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewUpdateBuilder start");
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewUpdateBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_UPDATE, uri);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewUpdateBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewDeleteBuilder(const std::shared_ptr<Uri> &uri)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewDeleteBuilder start");
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewDeleteBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_DELETE, uri);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewDeleteBuilder end");
    return builder;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperation::NewAssertBuilder(const std::shared_ptr<Uri> &uri)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewAssertBuilder start");
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewAssertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataAbilityOperationBuilder> builder =
        std::make_shared<DataAbilityOperationBuilder>(TYPE_ASSERT, uri);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::NewAssertBuilder end");
    return builder;
}

int DataAbilityOperation::GetType() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetType");
    return type_;
}

std::shared_ptr<Uri> DataAbilityOperation::GetUri() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetUri");
    return uri_;
}

std::shared_ptr<NativeRdb::ValuesBucket> DataAbilityOperation::GetValuesBucket() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetValuesBucket");
    return valuesBucket_;
}

int DataAbilityOperation::GetExpectedCount() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetExpectedCount");
    return expectedCount_;
}

std::shared_ptr<NativeRdb::DataAbilityPredicates> DataAbilityOperation::GetDataAbilityPredicates() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetDataAbilityPredicates");
    return dataAbilityPredicates_;
}

std::shared_ptr<NativeRdb::ValuesBucket> DataAbilityOperation::GetValuesBucketReferences() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetValuesBucketReferences");
    return valuesBucketReferences_;
}
std::map<int, int> DataAbilityOperation::GetDataAbilityPredicatesBackReferences() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::GetDataAbilityPredicatesBackReferences");
    return dataAbilityPredicatesBackReferences_;
}
bool DataAbilityOperation::IsValidOperation() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsValidOperation: type is %{public}d", type_);
    return (type_ == TYPE_INSERT || type_ == TYPE_UPDATE || type_ == TYPE_DELETE || type_ == TYPE_ASSERT);
}
bool DataAbilityOperation::IsInsertOperation() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsInsertOperation: %{public}d", type_ == TYPE_INSERT);
    return type_ == TYPE_INSERT;
}
bool DataAbilityOperation::IsUpdateOperation() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsUpdateOperation: %{public}d", type_ == TYPE_UPDATE);
    return type_ == TYPE_UPDATE;
}
bool DataAbilityOperation::IsDeleteOperation() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsDeleteOperation: %{public}d", type_ == TYPE_DELETE);
    return type_ == TYPE_DELETE;
}
bool DataAbilityOperation::IsAssertOperation() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsAssertOperation: %{public}d", type_ == TYPE_ASSERT);
    return type_ == TYPE_ASSERT;
}
bool DataAbilityOperation::IsInterruptionAllowed() const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::IsInterruptionAllowed: %{public}d", interrupted_);
    return interrupted_;
}

bool DataAbilityOperation::WriteUri(Parcel &out) const
{
    if (uri_ == nullptr) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "Uri is nullptr");
        return out.WriteInt32(VALUE_NULL);
    }
    if (!out.WriteInt32(VALUE_OBJECT)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write VALUE_OBJECT error");
        return false;
    }
    if (!out.WriteParcelable(uri_.get())) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write Uri error");
        return false;
    }
    return true;
}

bool DataAbilityOperation::WriteValuesBucket(Parcel &out) const
{
    if (valuesBucket_ == nullptr) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "ValuesBucket is nullptr");
        return out.WriteInt32(VALUE_NULL);
    }
    if (!out.WriteInt32(VALUE_OBJECT)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write VALUE_OBJECT error");
        return false;
    }
    if (!valuesBucket_->Marshalling(out)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write ValuesBucket error");
        return false;
    }
    return true;
}

bool DataAbilityOperation::WritePredicates(Parcel &out) const
{
    if (dataAbilityPredicates_ == nullptr) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityPredicates is nullptr");
        return out.WriteInt32(VALUE_NULL);
    }
    if (!out.WriteInt32(VALUE_OBJECT)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write VALUE_OBJECT error");
        return false;
    }
    if (!out.WriteParcelable(dataAbilityPredicates_.get())) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write DataAbilityPredicates error");
        return false;
    }
    return true;
}

bool DataAbilityOperation::WriteValuesBucketReferences(Parcel &out) const
{
    if (valuesBucketReferences_ == nullptr) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "ValuesBucketReferences is nullptr");
        return out.WriteInt32(VALUE_NULL);
    }
    if (!out.WriteInt32(VALUE_OBJECT)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write VALUE_OBJECT error");
        return false;
    }
    if (!valuesBucketReferences_->Marshalling(out)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "ValuesBucketReferences Marshalling error");
        return false;
    }
    return true;
}

bool DataAbilityOperation::Marshalling(Parcel &out) const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling start");
    if (!out.WriteInt32(type_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(type) error");
        return false;
    }
    if (!out.WriteInt32(expectedCount_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(expectedCount) error");
        return false;
    }
    if (!out.WriteBool(interrupted_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(interrupted) error");
        return false;
    }
    if (!WriteUri(out) || !WriteValuesBucket(out) || !WritePredicates(out) || !WriteValuesBucketReferences(out)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Marshalling error");
        return false;
    }
    int referenceSize = (int)dataAbilityPredicatesBackReferences_.size();
    if (dataAbilityPredicatesBackReferences_.empty()) {
        TAG_LOGD(
            AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling dataAbilityPredicatesBackReferences_ is empty");
        if (!out.WriteInt32(referenceSize)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling end");
        return true;
    }
    if (!out.WriteInt32(referenceSize)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
        return false;
    }
    if (referenceSize >= REFERENCE_THRESHOLD) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling referenceSize >= REFERENCE_THRESHOLD");
        return true;
    }
    for (auto it = dataAbilityPredicatesBackReferences_.begin(); it != dataAbilityPredicatesBackReferences_.end();
        it++) {
        if (!out.WriteInt32(it->first)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (!out.WriteInt32(it->second)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    }
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Marshalling end");
    return true;
}
DataAbilityOperation *DataAbilityOperation::Unmarshalling(Parcel &in)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Unmarshalling start");
    DataAbilityOperation *dataAbilityOperation = new (std::nothrow) DataAbilityOperation();
    if (dataAbilityOperation != nullptr && !dataAbilityOperation->ReadFromParcel(in)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Unmarshalling dataAbilityOperation error");
        delete dataAbilityOperation;
        dataAbilityOperation = nullptr;
    }
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::Unmarshalling end");
    return dataAbilityOperation;
}

bool DataAbilityOperation::ReadUriFromParcel(Parcel &in)
{
    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty is VALUE_OBJECT");
        uri_.reset(in.ReadParcelable<Uri>());
        return true;
    }
    uri_.reset();
    return true;
}

bool DataAbilityOperation::ReadValuesBucketFromParcel(Parcel &in)
{
    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty is VALUE_OBJECT");
        valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>(NativeRdb::ValuesBucket::Unmarshalling(in));
        return true;
    }
    valuesBucket_.reset();
    return true;
}

bool DataAbilityOperation::ReadDataAbilityPredicatesFromParcel(Parcel &in)
{
    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty is VALUE_OBJECT");
        dataAbilityPredicates_.reset(in.ReadParcelable<NativeRdb::DataAbilityPredicates>());
        return true;
    }
    dataAbilityPredicates_.reset();
    return true;
}

bool DataAbilityOperation::ReadValuesBucketReferencesFromParcel(Parcel &in)
{
    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty is VALUE_OBJECT");
        valuesBucketReferences_ = std::make_shared<NativeRdb::ValuesBucket>(
            NativeRdb::ValuesBucket::Unmarshalling(in));
        return true;
    }
    valuesBucketReferences_.reset();
    return true;
}

bool DataAbilityOperation::ReadFromParcel(Parcel &in)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel start");
    if (!in.ReadInt32(type_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(type) error");
        return false;
    }
    if (!in.ReadInt32(expectedCount_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel ReadInt32(expectedCount) error");
        return false;
    }
    interrupted_ = in.ReadBool();
    if (!ReadUriFromParcel(in) || !ReadValuesBucketFromParcel(in) || !ReadDataAbilityPredicatesFromParcel(in) ||
        !ReadValuesBucketReferencesFromParcel(in)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel error");
        return false;
    }
    int referenceSize = 0;
    if (!in.ReadInt32(referenceSize)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel end");
        return false;
    }
    if (referenceSize >= REFERENCE_THRESHOLD) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperation::ReadFromParcel referenceSize:%{public}d >= REFERENCE_THRESHOLD:%{public}d",
            referenceSize, REFERENCE_THRESHOLD);
        return true;
    }

    for (int i = 0; i < REFERENCE_THRESHOLD && i < referenceSize; ++i) {
        int first = 0;
        int second = 0;
        if (!in.ReadInt32(first)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel end");
            return false;
        }
        if (!in.ReadInt32(second)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel end");
            return false;
        }
        dataAbilityPredicatesBackReferences_.insert(std::make_pair(first, second));
    }

    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::ReadFromParcel end");
    return true;
}
std::shared_ptr<DataAbilityOperation> DataAbilityOperation::CreateFromParcel(Parcel &in)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::CreateFromParcel start");
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>(in);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::CreateFromParcel end");
    return operation;
}
void DataAbilityOperation::PutMap(Parcel &in)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::PutMap start");
    int count = in.ReadInt32();
    if (count > 0 && count < REFERENCE_THRESHOLD) {
        for (int i = 0; i < count; ++i) {
            dataAbilityPredicatesBackReferences_.insert(std::make_pair(in.ReadInt32(), in.ReadInt32()));
        }
    }
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperation::PutMap end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
