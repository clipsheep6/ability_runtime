/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "napi_common_child_process_param.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AppExecFwk {
bool UnwrapChildProcessArgs(napi_env env, napi_value jsValue, AppExecFwk::ChildProcessArgs &args,
    std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Parameter error. The type of args must be ProcessArgs.");
        errorMsg =  "Parameter error. The type of args must be ProcessArgs.";
        return false;
    }

    if (IsExistsByPropertyName(env, jsValue, "entryParams") &&
        !UnwrapStringByPropertyName(env, jsValue, "entryParams", args.entryParams)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Parameter error. The type of args.entryParams must be string.");
        errorMsg = "Parameter error. The type of args.entryParams must be string.";
        return false;
    }

    if (IsExistsByPropertyName(env, jsValue, "fds")) {
        napi_value jsFds = GetPropertyValueByPropertyName(env, jsValue, "fds", napi_object);
        if (jsFds == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "The type of args.fds must be Record<string, number>.");
            errorMsg = "The type of args.fds must be Record<string, number>.";
            return false;
        }
        if (!UnwrapChildProcessFds(env, jsFds, args.fds, errorMsg)) {
            return false;
        }
    }

    return true;
}

bool UnwrapChildProcessFds(napi_env env, napi_value param, std::map<std::string, int32_t> &map, std::string &errorMsg)
{
    napi_value jsKeyList = nullptr;
    uint32_t keyCount = 0;
    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsKeyList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsKeyList, &keyCount), false);
    if (keyCount > CHILD_PROCESS_ARGS_FDS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "fds count must <= %{public}d.", CHILD_PROCESS_ARGS_FDS_MAX_COUNT);
        errorMsg = "fds count must <= " + std::to_string(CHILD_PROCESS_ARGS_FDS_MAX_COUNT);
        return false;
    }

    napi_value jsKey = nullptr;
    for (uint32_t index = 0; index < keyCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsKeyList, index, &jsKey), false);
        std::string key;
        if (!UnwrapStringFromJS2(env, jsKey, key)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "The type of args.fds must be Record<string, number>.");
            errorMsg = "The type of args.fds must be Record<string, number>.";
            return false;
        }
        if (!ChildProcessArgs::CheckFdKeyLength(key)) {
            errorMsg = "fd key length must <= " + std::to_string(CHILD_PROCESS_ARGS_FD_KEY_MAX_LENGTH);
            return false;
        }

        int32_t value;
        if (!UnwrapInt32ByPropertyName(env, param, key.c_str(), value)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "The type of args.fds must be Record<string, number>.");
            errorMsg = "The type of args.fds must be Record<string, number>.";
            return false;
        }
        map.emplace(key, value);
    }
    return true;
}

bool UnwrapChildProcessOptions(napi_env env, napi_value jsValue, AppExecFwk::ChildProcessOptions &options,
    std::string &errorMsg)
{
    if (!IsTypeForNapiValue(env, jsValue, napi_object)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Parameter error. The type of options must be ProcessOptions.");
        errorMsg =  "Parameter error. The type of options must be ProcessOptions.";
        return false;
    }

    if (IsExistsByPropertyName(env, jsValue, "isolationMode") &&
        !UnwrapBooleanByPropertyName(env, jsValue, "isolationMode", options.isolationMode)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Parameter error. The type of options.isolationMode must be boolean.");
        errorMsg = "Parameter error. The type of options.isolationMode must be boolean.";
        return false;
    }
    return true;
}

napi_value WrapChildProcessArgs(napi_env env, AppExecFwk::ChildProcessArgs &args)
{
    napi_value jsArgs = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsArgs));

    napi_value jsEntryParams = WrapStringToJS(env, args.entryParams);
    SetPropertyValueByPropertyName(env, jsArgs, "entryParams", jsEntryParams);

    napi_value jsFds = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsFds));
    if (!args.CheckFdsSize()) {
        return jsArgs;
    }
    auto &fds = args.fds;
    for (auto iter = fds.begin(); iter != fds.end(); iter++) {
        std::string key = iter->first;
        napi_value jsValue = WrapInt32ToJS(env, iter->second);
        SetPropertyValueByPropertyName(env, jsFds, key.c_str(), jsValue);
    }

    SetPropertyValueByPropertyName(env, jsArgs, "fds", jsFds);
    return jsArgs;
}
}  // namespace AppExecFwk
}  // namespace OHOS
