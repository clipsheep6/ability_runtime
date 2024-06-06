/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_error_observer.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

char *MallocCString(const std::string &origin)
{
    if (origin.empty()) {
        return nullptr;
    }
    auto len = origin.length() + 1;
    char* res = (char*)malloc(sizeof(char) * len);
    if (res == nullptr) {
        return nullptr;
    }
    return std::char_traits<char>::copy(res, origin.c_str(), len);
}

ErrorObserver::ErrorObserver() {};

void ErrorObserver::OnExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGI(AAFwkTag::APPKIT, "OnExceptionObject come.");
    std::weak_ptr<ErrorObserver> thisWeakPtr(shared_from_this());
    std::shared_ptr<ErrorObserver> observer = thisWeakPtr.lock();
    if (observer) {
        observer->HandleException(errorObj);
    }
}

void ErrorObserver::OnUnhandledException(const std::string errMsg)
{
    TAG_LOGI(AAFwkTag::APPKIT, "OnUnhandledException come.");
    std::weak_ptr<ErrorObserver> thisWeakPtr(shared_from_this());
    std::shared_ptr<ErrorObserver> observer = thisWeakPtr.lock();
    if (observer) {
        observer->HandleOnUnhandledException(errMsg);
    }
}

void ErrorObserver::HandleOnUnhandledException(const std::string &errMsg)
{
    TAG_LOGI(AAFwkTag::APPKIT, "HandleOnUnhandledException come.");
    auto tmpMap = observerObjectMap_;
    for (auto &item : tmpMap) {
        auto obj = item.second;
        char* cstr = MallocCString(errMsg);
        obj.callbackOnUnhandledException(cstr);
    }
}

void ErrorObserver::HandleException(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGI(AAFwkTag::APPKIT, "HandleException come.");
    auto tmpMap = observerObjectMap_;
    for (auto &item : tmpMap) {
        auto obj = item.second;
        if (obj.callbackOnException == nullptr) {
            return;
        }
        CErrorObject cjErrorObj;
        cjErrorObj.name = MallocCString(errorObj.name);
        cjErrorObj.message = MallocCString(errorObj.message);
        cjErrorObj.stack = MallocCString(errorObj.stack);
        obj.callbackOnException(cjErrorObj);
    }
}

void ErrorObserver::AddObserverObject(const int32_t observerId, CErrorObserver observer)
{
    MapErrorObserver mObserver;
    mObserver.callbackOnUnhandledException = CJLambda::Create(observer.callbackOnUnhandledException);
    if (observer.callbackOnException == nullptr) {
        mObserver.callbackOnException = nullptr;
    } else {
        mObserver.callbackOnException = CJLambda::Create(observer.callbackOnException);
    }
    observerObjectMap_.emplace(observerId, mObserver);
}

bool ErrorObserver::IsEmpty()
{
    bool isEmpty = observerObjectMap_.empty();
    return isEmpty;
}

bool ErrorObserver::RemoveObserverObject(const int32_t observerId)
{
    bool result = false;
    result = (observerObjectMap_.erase(observerId) == 1);
    return result;
}

}
}