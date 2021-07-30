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

#ifndef FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_H
#define FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_H

#include <map>
#include <memory>
#include "dummy_data_ability_predicates.h"
#include "dummy_values_bucket.h"
#include "data_ability_operation_builder.h"
#include "uri.h"
#include "parcel.h"

using Uri = OHOS::Uri;

namespace OHOS {
namespace AppExecFwk {
class DataAbilityOperationBuilder;
class DataAbilityOperation final : public std::enable_shared_from_this<DataAbilityOperation> {
public:
    ~DataAbilityOperation();

    DataAbilityOperation(
        const std::shared_ptr<DataAbilityOperation> &dataAbilityOperation, const std::shared_ptr<Uri> &withUri);
    DataAbilityOperation(Parcel &in);
    DataAbilityOperation(const std::shared_ptr<DataAbilityOperationBuilder> &builder);
    DataAbilityOperation();
    /**
     * @brief Creates an operation for inserting data.
     * @param uri Indicates the path of data to operate.
     * @return Returns an insert DataAbilityOperationBuilder object.
     */
    static std::shared_ptr<DataAbilityOperationBuilder> NewInsertBuilder(const std::shared_ptr<Uri> &uri);
    /**
     * @brief Creates an operation for updating data.
     * @param uri Indicates the path of data to operate.
     * @return Returns an update DataAbilityOperationBuilder object.
     */
    static std::shared_ptr<DataAbilityOperationBuilder> NewUpdateBuilder(const std::shared_ptr<Uri> &uri);
    /**
     * @brief Creates an operation for deleting data.
     * @param uri Indicates the path of data to operate.
     * @return Returns an delete DataAbilityOperationBuilder object.
     */
    static std::shared_ptr<DataAbilityOperationBuilder> NewDeleteBuilder(const std::shared_ptr<Uri> &uri);
    /**
     * @brief Creates an operation for asserting data.
     * @param uri Indicates the path of data to operate.
     * @return Returns an assert DataAbilityOperationBuilder object.
     */
    static std::shared_ptr<DataAbilityOperationBuilder> NewAssertBuilder(const std::shared_ptr<Uri> &uri);
    /**
     * @brief Obtains the value of the type attribute included in this DataAbilityOperation.
     * @return Returns the type included in this DataAbilityOperation.
     */
    int GetType() const;
    /**
     * @brief Obtains the value of the uri attribute included in this DataAbilityOperation.
     * @return Returns the uri included in this DataAbilityOperation.
     */
    std::shared_ptr<Uri> GetUri() const;
    /**
     * @brief Obtains the value of the valuesBucket attribute included in this DataAbilityOperation.
     * @return Returns the valuesBucket included in this DataAbilityOperation.
     */
    std::shared_ptr<ValuesBucket> GetValuesBucket() const;
    /**
     * @brief Obtains the value of the expectedCount attribute included in this DataAbilityOperation.
     * @return Returns the expectedCount included in this DataAbilityOperation.
     */
    int GetExpectedCount() const;
    /**
     * @brief Obtains the value of the dataAbilityPredicates attribute included in this DataAbilityOperation.
     * @return Returns the dataAbilityPredicates included in this DataAbilityOperation.
     */
    std::shared_ptr<DataAbilityPredicates> GetDataAbilityPredicates() const;
    /**
     * @brief Obtains the value of the valuesBucketReferences attribute included in this DataAbilityOperation.
     * @return Returns the valuesBucketReferences included in this DataAbilityOperation.
     */
    std::shared_ptr<ValuesBucket> GetValuesBucketReferences() const;
    /**
     * @brief Obtains the value of the dataAbilityPredicatesBackReferences attribute included in this
     * DataAbilityOperation.
     * @return Returns the dataAbilityPredicatesBackReferences included in this DataAbilityOperation.
     */
    std::map<int, int> GetDataAbilityPredicatesBackReferences() const;
    /**
     * @brief Checks whether an insert operation is created.
     * @return Returns true if it is an insert operation; returns false otherwise.
     */
    bool IsInsertOperation() const;
    /**
     * @brief Checks whether an delete operation is created.
     * @return Returns true if it is an delete operation; returns false otherwise.
     */
    bool IsDeleteOperation() const;
    /**
     * @brief Checks whether an update operation is created.
     * @return Returns true if it is an update operation; returns false otherwise.
     */
    bool IsUpdateOperation() const;
    /**
     * @brief Checks whether an assert operation is created.
     * @return Returns true if it is an assert operation; returns false otherwise.
     */
    bool IsAssertOperation() const;
    /**
     * @brief Checks whether an operation can be interrupted.
     * @return Returns true if the operation can be interrupted; returns false otherwise.
     */
    bool IsInterruptionAllowed() const;

    bool operator==(const DataAbilityOperation &other) const;
    DataAbilityOperation &operator=(const DataAbilityOperation &other);
    bool Marshalling(Parcel &out) const;
    static DataAbilityOperation *Unmarshalling(Parcel &in);

    /**
     * @brief Creates a DataAbilityOperation instance based on the given Parcel object
     * @param in Indicates the Parcel object.
     * @return Returns the DataAbilityOperation object.
     */
    static std::shared_ptr<DataAbilityOperation> CreateFromParcel(Parcel &in);

public:
    static constexpr int TYPE_INSERT = 1;
    static constexpr int TYPE_UPDATE = 2;
    static constexpr int TYPE_DELETE = 3;
    static constexpr int TYPE_ASSERT = 4;

private:
    void PutMap(Parcel &in);
    bool ReadFromParcel(Parcel &in);

private:
    // no object in parcel
    static constexpr int VALUE_NULL = 0;
    // object exist in parcel
    static constexpr int VALUE_OBJECT = 1;
    static constexpr int REFERENCE_THRESHOLD = 3 * 1024 * 1024;
    int type_;
    int expectedCount_;
    bool interrupted_;
    std::shared_ptr<Uri> uri_;
    std::shared_ptr<ValuesBucket> valuesBucket_;
    std::shared_ptr<DataAbilityPredicates> dataAbilityPredicates_;
    std::shared_ptr<ValuesBucket> valuesBucketReferences_;
    std::map<int, int> dataAbilityPredicatesBackReferences_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_DATA_ABILITY_OPERATION_H