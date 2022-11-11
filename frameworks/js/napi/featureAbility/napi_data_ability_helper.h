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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_H
#define OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_H
#include <memory>

#include "data_ability_observer_stub.h"
#include "feature_ability_common.h"
#include "js_napi_common_ability.h"

namespace OHOS {
namespace AppExecFwk {
class NAPIDataAbilityObserver : public AAFwk::DataAbilityObserverStub {
public:
    void OnChange() override;
    void SetEnv(NativeEngine *engine);
    void SetCallbackRef(NativeReference *ref);
    void ReleaseJSCallback();

    void CallJsMethod();

private:
    void SafeReleaseJSCallback();

    NativeEngine *engine_ = nullptr;
    NativeReference *nativeRef_ = nullptr;
    bool isCallingback_ = false;
    bool needRelease_ = false;
    std::mutex mutex_;
};

class JsDataAbilityHelper {
public:
    explicit JsDataAbilityHelper(const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) :
        dataAbilityHelper_(dataAbilityHelper) {}
    ~JsDataAbilityHelper() = default;

    struct JsNormalizeUriData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::string result = "";
    };
    typedef JsNormalizeUriData JsDenormalizeUriData;

    struct JsReleaseData {
        bool flag;
    };

    struct JsExecuteBatchData {
        int32_t errorVal = 0;
        std::string uri;
        std::vector<std::shared_ptr<DataAbilityOperation>> operations;
        std::vector<std::shared_ptr<DataAbilityResult>> results;
    };

    struct JsCallData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::string method;
        std::string arg;
        PacMap extras;
        std::shared_ptr<PacMap> result;
    };

    struct JsGetFileTypesData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::string mimeTypeFilter = "";
        std::vector<std::string> result;
    };

    struct JsGetTypeData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::string result;
    };

    struct JsOpenFileData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::string mode;
        int result = 0;
    };

    struct JsBatchInsertData {
        int32_t errorVal = 0;
        std::string uri = "";
        std::vector<NativeRdb::ValuesBucket> values;
        int result = 0;
    };

    struct JsUpdateData {
        int32_t errorVal = 0;
        std::string uri = "";
        NativeRdb::ValuesBucket valueBucket;
        NativeRdb::DataAbilityPredicates predicates;
        int result = 0;
    };

    struct JsQueryData {
        int32_t errorVal = 0;
        std::shared_ptr<NativeRdb::AbsSharedResultSet> result = nullptr;
        std::string uri = "";
        std::vector<std::string> columns;
        NativeRdb::DataAbilityPredicates predicates;
    };

    struct JsDeleteData {
        int32_t errorVal = 0;
        std::string uri = "";
        NativeRdb::DataAbilityPredicates predicates;
        int result = 0;
    };

    struct JsInsertData
    {
        int32_t errorVal = 0;
        NativeRdb::ValuesBucket valueBucket;
        std::string uri = "";
        int result = 0;
    };

    struct JsNotifyChangeData
    {
        int32_t errorVal = 0;
        std::string uri = "";
    };

    static NativeValue* JsNormalizeUri(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsDenormalizeUri(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsRelease(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsExecuteBatch(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsCall(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetFileTypes(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsGetType(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsOpenFile(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsBatchInsert(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsUpdate(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsQuery(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsDelete(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsNotifyChange(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsInsert(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsRegister(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* JsUnRegister(NativeEngine *engine, NativeCallbackInfo *info);
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    std::shared_ptr<DataAbilityHelper> GetDataAbilityHelper()
    {
        return dataAbilityHelper_;
    }

private:
    NativeValue* OnNormalizeUri(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnDenormalizeUri(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnRelease(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnExecuteBatch(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnCall(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnGetFileTypes(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnGetType(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnOpenFile(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnBatchInsert(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnUpdate(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnQuery(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnDelete(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnNotifyChange(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnInsert(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnRegister(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnUnRegister(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* CreateDataAbilityResult(NativeEngine &engine, const std::shared_ptr<DataAbilityResult> &data);
    NativeValue* CreateExecuteBatchResult(NativeEngine &engine, const std::shared_ptr<JsExecuteBatchData> &data);
    NativeValue* CreateCallResult(NativeEngine &engine, const std::shared_ptr<JsCallData> &data);
    bool UnWarpExecuteBatchParams(NativeEngine &engine, const NativeCallbackInfo &info,
        std::shared_ptr<JsExecuteBatchData> &data);
    bool UnWarpCallParams(NativeEngine &engine, const NativeCallbackInfo &info, std::shared_ptr<JsCallData> &data);
    void AnalysisPacMap(NativeEngine &engine, NativeValue *jsParam, AppExecFwk::PacMap &pacMap);
    void SetPacMapObject(NativeEngine &engine, NativeValue *jsParam, const std::string strKey,
        AppExecFwk::PacMap &pacMap);

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper_;
};

NativeValue* CreateJsDataAbilityHelper(NativeEngine &engine, const NativeCallbackInfo &info);

void FindRegisterObs(DAHelperOnOffCB *data);
std::vector<NativeRdb::ValuesBucket> NapiValueObject(napi_env env, napi_value param);
bool UnwrapArrayObjectFromJS(napi_env env, napi_value param, std::vector<NativeRdb::ValuesBucket> &value);
void AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg);
void SetValuesBucketObject(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env,
    std::string keyStr, napi_value value);
void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value);
void DeleteDAHelperOnOffCB(DAHelperOnOffCB *onCB);
bool NeedErase(std::vector<DAHelperOnOffCB*>::iterator& iter, const std::shared_ptr<DataAbilityHelper> &objectInfo);
void EraseMemberProperties(DAHelperOnOffCB* onCB);
void UnwrapDataAbilityPredicates(
    NativeEngine& engine, NativeValue* jsParam, std::shared_ptr<NativeRdb::DataAbilityPredicates> &predicates);
void AnalysisValuesBucket(NativeEngine& engine, NativeValue& jsParam, NativeRdb::ValuesBucket &valuesBucket);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_H */
