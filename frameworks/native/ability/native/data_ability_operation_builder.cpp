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

#include "data_ability_operation_builder.h"
#include "data_ability_predicates.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
DataAbilityOperationBuilder::DataAbilityOperationBuilder(const int type, const std::shared_ptr<Uri> &uri)
{
    type_ = type;
    uri_ = uri;
    expectedCount_ = 0;
    interrupted_ = false;
    valuesBucket_ = nullptr;
    dataAbilityPredicates_ = nullptr;
    valuesBucketReferences_ = nullptr;
    dataAbilityPredicatesBackReferences_.clear();
}
DataAbilityOperationBuilder::~DataAbilityOperationBuilder()
{
    dataAbilityPredicatesBackReferences_.clear();
}

std::shared_ptr<DataAbilityOperation> DataAbilityOperationBuilder::Build()
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::Build start");
    if (type_ != DataAbilityOperation::TYPE_UPDATE || (valuesBucket_ != nullptr && !valuesBucket_->IsEmpty())) {
        std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>(shared_from_this());
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::Build end");
        return operation;
    }
    TAG_LOGE(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::Build return nullptr");
    return nullptr;
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValuesBucket(
    std::shared_ptr<NativeRdb::ValuesBucket> &values)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithValuesBucket start");
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::WithValuesBucket only inserts, updates and assert can have values,"
            " type=%{public}d",
            type_);
        return nullptr;
    }

    valuesBucket_ = std::make_shared<NativeRdb::ValuesBucket>(*values);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithValuesBucket end");
    return shared_from_this();
}

std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicates(
    std::shared_ptr<NativeRdb::DataAbilityPredicates> &predicates)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithPredicates start");
    if (type_ != DataAbilityOperation::TYPE_DELETE && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::withPredicates only deletes, updates and assert can have selections,"
            " type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicates_ = predicates;
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithPredicates end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithExpectedCount(int count)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithExpectedCount start");
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithExpectedCount expectedCount:%{public}d", count);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::withExpectedCount only updates, deletes and assert "
            "can have expected counts, "
            "type=%{public}d",
            type_);
        return nullptr;
    }
    expectedCount_ = count;
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithExpectedCount end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithPredicatesBackReference(
    int requestArgIndex, int previousResult)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithPredicatesBackReference start");
    TAG_LOGI(AAFwkTag::DATA_ABILITY,
        "DataAbilityOperationBuilder::WithPredicatesBackReference requestArgIndex:%{public}d, "
        "previousResult:%{public}d",
        requestArgIndex, previousResult);
    if (type_ != DataAbilityOperation::TYPE_UPDATE && type_ != DataAbilityOperation::TYPE_DELETE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::withPredicatesBackReference only updates, deletes, "
            "and asserts can have select back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    dataAbilityPredicatesBackReferences_.insert(std::make_pair(requestArgIndex, previousResult));
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithPredicatesBackReference end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithValueBackReferences(
    std::shared_ptr<NativeRdb::ValuesBucket> &backReferences)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithValueBackReferences start");
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::withValueBackReferences only inserts, updates, and asserts can have "
            "value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    valuesBucketReferences_ = backReferences;
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithValueBackReferences end");
    return shared_from_this();
}
std::shared_ptr<DataAbilityOperationBuilder> DataAbilityOperationBuilder::WithInterruptionAllowed(bool interrupted)
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithInterruptionAllowed start");
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithInterruptionAllowed  interrupted=%{public}d",
        interrupted);
    if (type_ != DataAbilityOperation::TYPE_INSERT && type_ != DataAbilityOperation::TYPE_UPDATE &&
        type_ != DataAbilityOperation::TYPE_ASSERT && type_ != DataAbilityOperation::TYPE_DELETE) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY,
            "DataAbilityOperationBuilder::withInterruptionAllowed only inserts, updates, delete, "
            "and asserts can have value back-references, type=%{public}d",
            type_);
        return nullptr;
    }
    interrupted_ = interrupted;
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "DataAbilityOperationBuilder::WithInterruptionAllowed end");
    return shared_from_this();
}
}  // namespace AppExecFwk
}  // namespace OHOS
