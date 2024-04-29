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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INTERFACE_H

#include <vector>

#include <ipc_types.h>
#include <iremote_broker.h>

#include "data_ability_observer_interface.h"
#include "dataobs_mgr_errors.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;
constexpr const char* DATAOBS_MANAGER_SERVICE_NAME = "DataObsMgrService";
/**
 * @class IDataObsMgr
 * IDataObsMgr interface is used to access dataobs manager services.
 */
class IDataObsMgr : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.DataObsMgr")

    enum {
        TRANS_HEAD,
        REGISTER_OBSERVER = TRANS_HEAD,
        UNREGISTER_OBSERVER,
        NOTIFY_CHANGE,
        REGISTER_OBSERVER_EXT,
        UNREGISTER_OBSERVER_EXT,
        UNREGISTER_OBSERVER_ALL_EXT,
        NOTIFY_CHANGE_EXT,
        TRANS_BUTT,
    };
    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) = 0;

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) = 0;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyChange(const Uri &uri) = 0;

    /**
     * Registers an observer to DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     * @param isDescendants, Indicates the Whether to note the change of descendants.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, bool isDescendants) = 0;

    /**
     * Deregisters an observer used for DataObsMgr specified by the given Uri.
     *
     * @param uri, Indicates the path of the data to operate.
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver) = 0;

    /**
     * Deregisters dataObserver used for DataObsMgr specified
     *
     * @param dataObserver, Indicates the IDataAbilityObserver object.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver) = 0;

    /**
     * Notifies the registered observers of a change to the data resource specified by Uris.
     *
     * @param changeInfo Indicates the info of the data to operate.
     *
     * @return Returns SUCCESS on success, others on failure.
     */
    virtual Status NotifyChangeExt(const ChangeInfo &changeInfo) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INTERFACE_H
