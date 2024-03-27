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
#include<unistd.h> 
#include "app_jsheap_mem_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool JsHeapDumpInfo::Marshalling(Parcel& parcel) const
{
    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Marshalling msgParcel==nullptr");
        return false;
    }
    if (!parcel.WriteUInt32Vector(fdVec) && parcel.WriteUInt32Vector(tidVec)) {
        HILOG_ERROR("WriteUInt32Vector fdVec error");
        return false;
    }
    for (auto &fd : fdVec){
        msgParcel->WriteFileDescriptor(fd);
        HILOG_ERROR("Dump WriteFileDescriptor fd=%{public}d", fd);
    }
    for (auto &tid : tidVec){
        HILOG_INFO("Dump tid=%{public}d", tid);
    }
    HILOG_INFO("Dump JsHeapDumpInfo::Marshalling");
    return (parcel.WriteUint32(pid) && parcel.WriteUint32(tid)
        && parcel.WriteBool(needGc) && parcel.WriteBool(needSnapshot));
}

JsHeapDumpInfo *JsHeapDumpInfo::Unmarshalling(Parcel &parcel)
{
    JsHeapDumpInfo *info = new (std::nothrow) JsHeapDumpInfo();
    if (info == nullptr) {
        HILOG_ERROR("info nullptr");
        return nullptr;
    }
    info->pid = parcel.ReadUint32();
    info->tid = parcel.ReadUint32();
    info->needGc = parcel.ReadBool();
    info->needSnapshot = parcel.ReadBool();
    parcel.ReadUInt32Vector(&(info->tidVec));
    parcel.ReadUInt32Vector(&(info->fdVec));
    for (auto &fd : info->fdVec){
        HILOG_INFO("Dump Unmarshalling fd=%{public}d", fd);
    }
    for (auto &tid : info->tidVec){
        HILOG_INFO("Dump Unmarshalling tid=%{public}d", tid);
    }
    HILOG_INFO("pid: %{public}d, tid: %{public}d, needGc: %{public}d, needSnapshot: %{public}d",
        info->pid, info->tid, info->needGc, info->needSnapshot);
    HILOG_INFO("Dump JsHeap info->tidVec.size()=%{public}zu, fdvecSize=%{public}zu", info->tidVec.size(), info->fdVec.size());
    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Unmarshalling msgParcel==nullptr");
        return nullptr;
    }
    for (auto &tid : info->tidVec){
        uint32_t fd = msgParcel->ReadFileDescriptor();
        HILOG_INFO("Dump JsHeapDumpInfo::Unmarshalling. fd=%{public}d", fd);
        info->fdVec.push_back(msgParcel->ReadFileDescriptor());
    }
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS
