/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "configuration.h"

#include <mutex>
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
Configuration::Configuration()
{}

Configuration::Configuration(const Configuration &other) : defaultDisplayId_(other.defaultDisplayId_)
{
    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    configParameter_ = other.configParameter_;
}

Configuration& Configuration::operator=(const Configuration &other)
{
    if (this == &other) {
        return *this;
    }

    defaultDisplayId_ = other.defaultDisplayId_;

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    configParameter_.clear();
    configParameter_ = other.configParameter_;
    return *this;
}

Configuration::~Configuration()
{}

bool Configuration::MakeTheKey(std::string &getKey, int id, const std::string &param) const
{
    if (param.empty()) {
        return false;
    }

    if (std::find(ConfigurationInner::SystemConfigurationKeyStore.begin(),
        ConfigurationInner::SystemConfigurationKeyStore.end(), param) ==
        ConfigurationInner::SystemConfigurationKeyStore.end()) {
        return false;
    }

    getKey.clear();
    getKey += std::to_string(id);
    getKey += ConfigurationInner::CONNECTION_SYMBOL;
    getKey += param;
    HILOG_DEBUG(" getKey [%{public}s]", getKey.c_str());

    return true;
}

bool Configuration::AddItem(int displayId, const std::string &key, const std::string &value)
{
    if (key.empty() || value.empty()) {
        return false;
    }

    std::string getKey;
    if (!MakeTheKey(getKey, displayId, key)) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    configParameter_[getKey] = value;
    return true;
}

std::string Configuration::GetItem(int displayId, const std::string &key) const
{
    if (key.empty()) {
        return ConfigurationInner::EMPTY_STRING;
    }

    std::string getKey;
    if (!MakeTheKey(getKey, displayId, key)) {
        return ConfigurationInner::EMPTY_STRING;
    }

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    auto iter = configParameter_.find(getKey);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return ConfigurationInner::EMPTY_STRING;
}

int Configuration::GetItemSize() const
{
    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    return configParameter_.size();
}

void Configuration::GetAllKey(std::vector<std::string> &keychain) const
{
    keychain.clear();

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    for (const auto &it :configParameter_) {
        keychain.push_back(it.first);
    }
}

std::string Configuration::GetValue(const std::string &key) const
{
    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    auto iter = configParameter_.find(key);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return ConfigurationInner::EMPTY_STRING;
}

void Configuration::CompareDifferent(std::vector<std::string> &diffKeyV, const Configuration &other)
{
    if (other.GetItemSize() == 0) {
        return;
    }

    diffKeyV.clear();
    std::vector<std::string> otherk;
    other.GetAllKey(otherk);

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    for (const auto &iter : otherk) {
        HILOG_DEBUG(" iter : [%{public}s] | Val: [%{public}s]", iter.c_str(), other.GetValue(iter).c_str());
        // Insert new content directly
        auto pair = configParameter_.insert(std::make_pair(iter, other.GetValue(iter)));
        if (pair.second) {
            diffKeyV.push_back(iter); // One of the changes this time
            continue;
        }
        // Compare what you already have
        if (!other.GetValue(iter).empty() && other.GetValue(iter) != GetValue(iter)) {
            diffKeyV.push_back(iter);
        }
    }
}

void Configuration::Merge(const std::vector<std::string> &diffKeyV, const Configuration &other)
{
    if (diffKeyV.empty()) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    for (const auto &mergeItemKey : diffKeyV) {
        auto myItem = GetValue(mergeItemKey);
        auto otherItem = other.GetValue(mergeItemKey);
        // myItem possible empty
        if (!otherItem.empty() && otherItem != myItem) {
            configParameter_[mergeItemKey] = otherItem;
        }
    }
}

int Configuration::RemoveItem(int displayId, const std::string &key)
{
    if (key.empty()) {
        return 0;
    }

    std::string getKey;
    if (!MakeTheKey(getKey, displayId, key)) {
        return 0;
    }

    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    return configParameter_.erase(getKey);
}

bool Configuration::AddItem(const std::string &key, const std::string &value)
{
    return AddItem(defaultDisplayId_, key, value);
}

std::string Configuration::GetItem(const std::string &key) const
{
    return GetItem(defaultDisplayId_, key);
}

int Configuration::RemoveItem(const std::string &key)
{
    return RemoveItem(defaultDisplayId_, key);
}

const std::string& Configuration::GetName() const
{
    std::lock_guard<std::recursive_mutex> lock(configParameterMutex_);
    // json configArray(configParameter_);
    // toStrintg_ = configArray.dump();
    return toStrintg_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
