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
    bool res = (parcel.WriteUint32(pid) && parcel.WriteUint32(tid)
        && parcel.WriteBool(needGc) && parcel.WriteBool(needSnapshot)
        && parcel.WriteUInt32Vector(fdVec) && parcel.WriteUInt32Vector(tidVec));
    HILOG_INFO("pid: %{public}d, tid: %{public}d, needGc: %{public}d, needSnapshot: %{public}d",
        pid, tid, needGc, needSnapshot);

    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Marshalling msgParcel==nullptr");
        return false;
    }
    for (auto &fd : fdVec){
        msgParcel->WriteFileDescriptor(fd);
        HILOG_ERROR("Dump WriteFileDescriptor fd=%{public}d", fd);
    }
    for (auto &tid : tidVec){
        HILOG_INFO("Dump tid=%{public}d", tid);
    }
    return res;
}

bool JsHeapDumpInfo::ReadFromParcel(Parcel &parcel)
{

    pid = parcel.ReadUint32();
    tid = parcel.ReadUint32();
    needGc = parcel.ReadBool();
    needSnapshot = parcel.ReadBool();
    HILOG_INFO("pid: %{public}d, tid: %{public}d, needGc: %{public}d, needSnapshot: %{public}d",
        pid, tid, needGc, needSnapshot);

    parcel.ReadUInt32Vector(&fdVec);
    parcel.ReadUInt32Vector(&tidVec);
    for (auto &fd : fdVec){
        HILOG_INFO("Dump1 Unmarshalling fd=%{public}d", fd);
    }
    for (auto &tid : tidVec){
        HILOG_INFO("Dump1 Unmarshalling tid=%{public}d", tid);
    }

    HILOG_INFO("Dump JsHeap info->tidVec.size()=%{public}zu, fdvecSize=%{public}zu", tidVec.size(), fdVec.size());
    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Unmarshalling msgParcel==nullptr");
        return false;
    }
    fdVec.clear();
    for (auto &tid : tidVec){
        uint32_t parcelFd = msgParcel->ReadFileDescriptor();
        HILOG_INFO("Dump JsHeapDumpInfo::Unmarshalling. tid=%{public}d, parcelFd=%{public}d", tid, parcelFd);
        fdVec.push_back(parcelFd);
    }
    return true;
}

JsHeapDumpInfo *JsHeapDumpInfo::Unmarshalling(Parcel &parcel)
{
    JsHeapDumpInfo *info = new (std::nothrow) JsHeapDumpInfo();
    if (info == nullptr) {
        HILOG_ERROR("info nullptr");
        return nullptr;
    }
    if (info && !info->ReadFromParcel(parcel)) {
        HILOG_INFO("Dump JsHeapDumpInfo failed, because ReadFromParcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS
