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
#include "ipc_file_descriptor.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

bool JsHeapDumpInfo::WriteFileDescriptor(Parcel& parcel, int fd) const
{
    if (fd < 0) {
        return false;
    }
    int dupFd = dup(fd);
    if (dupFd < 0) {
        return false;
    }
    sptr<IPCFileDescriptor> descriptor = new (std::nothrow)IPCFileDescriptor(dupFd);
    if (descriptor == nullptr) {
        HILOG_ERROR("Dump create IPCFileDescriptor object failed");
        return false;
    }
    return parcel.WriteObject<IPCFileDescriptor>(descriptor);
}



bool JsHeapDumpInfo::Marshalling(Parcel& parcel) const
{
    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Marshalling msgParcel==nullptr");
        return false;
    }
    return (parcel.WriteUint32(pid) && parcel.WriteUint32(tid)
        && parcel.WriteBool(needGc) && parcel.WriteBool(needSnapshot)) && msgParcel->WriteFileDescriptor(fds);
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
    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if(msgParcel==nullptr){
        HILOG_ERROR("Dump Unmarshalling msgParcel==nullptr");
        return nullptr;
    }
    info->fds = msgParcel->ReadFileDescriptor();
    HILOG_INFO("Dump JsHeapDumpInfo::Unmarshalling. info->fds=%{public}d", info->fds);
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS
