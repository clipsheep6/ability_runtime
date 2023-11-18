/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_auto_fill_extension_util.h"

#include "hilog_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *VIEW_DATA_BUNDLE_NAME = "bundleName";
constexpr const char *VIEW_DATA_MODULE_NAME = "moduleName";
constexpr const char *VIEW_DATA_ABILITY_NAME = "abilityName";
constexpr const char *VIEW_DATA_PAGEURL = "pageUrl";
constexpr const char *VIEW_DATA_PAGE_NODE_INFOS = "pageNodeInfos";
constexpr const char *VIEW_DATA_VIEW_DATA = "viewData";
constexpr const char *VIEW_DATA_TYPE = "type";
constexpr const char *PAGE_INFO_ID = "id";
constexpr const char *PAGE_INFO_DEPTH = "depth";
constexpr const char *PAGE_INFO_AUTOFILLTYPE = "autoFillType";
constexpr const char *PAGE_INFO_TAG = "tag";
constexpr const char *PAGE_INFO_VALUE = "value";
constexpr const char *PAGE_INFO_PLACEHOLDER = "placeholder";
constexpr const char *PAGE_INFO_PASSWORDRULES = "passwordRules";
constexpr const char *PAGE_INFO_ENABLEAUTOFILL = "enableAutoFill";
constexpr const char *WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
constexpr const char *WANT_PARAMS_AUTO_FILL_TYPE_KEY = "ability.want.params.AutoFillType";
constexpr uint32_t PAGE_NODE_COUNT_MAX = 100;
} // namespace

napi_value JsAutoFillExtensionUtil::WrapViewData(const napi_env env, const AbilityBase::ViewData &viewData)
{
    HILOG_DEBUG("Called.");
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    napi_value jsValue = nullptr;
    jsValue = WrapStringToJS(env, viewData.bundleName);
    SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_BUNDLE_NAME, jsValue);

    jsValue = WrapStringToJS(env, viewData.abilityName);
    SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_ABILITY_NAME, jsValue);

    jsValue = WrapStringToJS(env, viewData.moduleName);
    SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_MODULE_NAME, jsValue);

    jsValue = WrapStringToJS(env, viewData.pageUrl);
    SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_PAGEURL, jsValue);

    napi_value jsArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArray));
    napi_value jsSubValue = nullptr;
    uint32_t index = 0;
    for (auto element : viewData.nodes) {
        jsSubValue = WrapPageNodeInfo(env, element);
        if (jsSubValue != nullptr && napi_set_element(env, jsArray, index, jsSubValue) == napi_ok) {
            index++;
        } else {
            HILOG_ERROR("Set element fail.");
        }
    }

    SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_PAGE_NODE_INFOS, jsArray);
    return jsObject;
}

napi_value JsAutoFillExtensionUtil::WrapPageNodeInfo(const napi_env env, const AbilityBase::PageNodeInfo &pageNodeInfo)
{
    HILOG_DEBUG("Called.");
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    napi_value jsValue = nullptr;
    jsValue = AppExecFwk::WrapInt32ToJS(env, pageNodeInfo.id);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_ID, jsValue);

    jsValue = AppExecFwk::WrapInt32ToJS(env, pageNodeInfo.depth);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_DEPTH, jsValue);

    jsValue = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(pageNodeInfo.autoFillType));
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_AUTOFILLTYPE, jsValue);

    jsValue = WrapStringToJS(env, pageNodeInfo.tag);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_TAG, jsValue);

    jsValue = WrapStringToJS(env, pageNodeInfo.value);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_VALUE, jsValue);

    jsValue = WrapStringToJS(env, pageNodeInfo.passwordRules);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_PASSWORDRULES, jsValue);

    jsValue = WrapStringToJS(env, pageNodeInfo.placeholder);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_PLACEHOLDER, jsValue);

    jsValue = WrapBoolToJS(env, pageNodeInfo.enableAutoFill);
    SetPropertyValueByPropertyName(env, jsObject, PAGE_INFO_ENABLEAUTOFILL, jsValue);

    return jsObject;
}

void JsAutoFillExtensionUtil::UnwrapViewData(
    const napi_env env, const napi_value value, AbilityBase::ViewData &viewData)
{
    HILOG_DEBUG("Called.");
    napi_value jsViewData = GetPropertyValueByPropertyName(env, value, VIEW_DATA_VIEW_DATA, napi_object);
    if (jsViewData == nullptr) {
        HILOG_ERROR("Get ViewData from JS failed");
        return;
    }

    napi_value jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, jsViewData, VIEW_DATA_BUNDLE_NAME, napi_string);
    viewData.bundleName = UnwrapStringFromJS(env, jsValue, "");
    jsValue = GetPropertyValueByPropertyName(env, jsViewData, VIEW_DATA_MODULE_NAME, napi_string);
    viewData.moduleName = UnwrapStringFromJS(env, jsValue, "");
    jsValue = GetPropertyValueByPropertyName(env, jsViewData, VIEW_DATA_ABILITY_NAME, napi_string);
    viewData.abilityName = UnwrapStringFromJS(env, jsValue, "");
    jsValue = GetPropertyValueByPropertyName(env, jsViewData, VIEW_DATA_PAGEURL, napi_string);
    viewData.pageUrl = UnwrapStringFromJS(env, jsValue, "");
    jsValue = GetPropertyValueByPropertyName(env, jsViewData, VIEW_DATA_PAGE_NODE_INFOS, napi_object);
    if (jsValue != nullptr) {
        uint32_t jsProCount = 0;
        if (!IsArrayForNapiValue(env, jsValue, jsProCount)) {
            HILOG_ERROR("Get PAGE_NODE_INFOS from JS failed.");
            return;
        }

        for (uint32_t index = 0; index < jsProCount && index < PAGE_NODE_COUNT_MAX; index++) {
            napi_value jsNode = nullptr;
            napi_get_element(env, jsValue, index, &jsNode);
            AbilityBase::PageNodeInfo node;
            UnwrapPageNodeInfo(env, jsNode, node);
            viewData.nodes.emplace_back(node);
        }
    }
}

void JsAutoFillExtensionUtil::UnwrapPageNodeInfo(
    const napi_env env, const napi_value jsNode, AbilityBase::PageNodeInfo &node)
{
    HILOG_DEBUG("Called.");
    UnwrapInt32ByPropertyName(env, jsNode, PAGE_INFO_ID, node.id);
    UnwrapInt32ByPropertyName(env, jsNode, PAGE_INFO_DEPTH, node.depth);
    int32_t type;
    UnwrapInt32ByPropertyName(env, jsNode, PAGE_INFO_AUTOFILLTYPE, type);
    node.autoFillType = static_cast<AbilityBase::AutoFillType>(type);
    UnwrapStringByPropertyName(env, jsNode, PAGE_INFO_TAG, node.tag);
    UnwrapStringByPropertyName(env, jsNode, PAGE_INFO_VALUE, node.value);
    UnwrapStringByPropertyName(env, jsNode, PAGE_INFO_PASSWORDRULES, node.passwordRules);
    UnwrapStringByPropertyName(env, jsNode, PAGE_INFO_PLACEHOLDER, node.placeholder);
    UnwrapBooleanByPropertyName(env, jsNode, PAGE_INFO_ENABLEAUTOFILL, node.enableAutoFill);
}

napi_value JsAutoFillExtensionUtil::WrapFillRequest(const AAFwk::Want &want, const napi_env env)
{
    HILOG_DEBUG("Called.");
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    if (jsObject == nullptr) {
        HILOG_ERROR("Failed to create jsObject.");
        return nullptr;
    }

    if (want.HasParameter(WANT_PARAMS_AUTO_FILL_TYPE_KEY)) {
        auto type = want.GetIntParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, -1);
        HILOG_DEBUG("Auto fill request type: %{public}d", type);

        napi_value jsValue = AppExecFwk::WrapInt32ToJS(env, type);
        SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_TYPE, jsValue);
    }

    if (want.HasParameter(WANT_PARAMS_VIEW_DATA)) {
        std::string viewDataString = want.GetStringParam(WANT_PARAMS_VIEW_DATA);
        if (viewDataString.empty()) {
            HILOG_ERROR("View data is empty.");
            return jsObject;
        }

        AbilityBase::ViewData viewData;
        viewData.FromJsonString(viewDataString);
        napi_value viewDataValue = WrapViewData(env, viewData);
        SetPropertyValueByPropertyName(env, jsObject, VIEW_DATA_VIEW_DATA, viewDataValue);
    }
    return jsObject;
}

void JsAutoFillExtensionUtil::UnwrapFillResponse(const napi_env env, const napi_value value, FillResponse &response)
{
    HILOG_DEBUG("Called.");
    UnwrapViewData(env, value, response.viewData);
}
} // namespace AbilityRuntime
} // namespace OHOS