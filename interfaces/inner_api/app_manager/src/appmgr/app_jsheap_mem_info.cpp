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

#include "app_jsheap_mem_info.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_TID_COUNT = 40;
}

JsHeapDumpInfo::~JsHeapDumpInfo()
{
    TAG_LOGI(AAFwkTag::APPMGR, "~JsHeapDumpInfo start");
    for (auto &fd : fdVec) {
        close(fd);
    }
    fdVec.clear();
    tidVec.clear();
}

bool JsHeapDumpInfo::Marshalling(Parcel &parcel) const
{
    bool res = (parcel.WriteUint32(pid) && parcel.WriteUint32(tid)
        && parcel.WriteBool(needGc) && parcel.WriteBool(needSnapshot)
        && parcel.WriteUInt32Vector(fdVec) && parcel.WriteUInt32Vector(tidVec));

    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if (msgParcel == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Dump Marshalling msgParcel==nullptr");
        return false;
    }
    for (auto &fd : fdVec) {
        msgParcel->WriteFileDescriptor(fd);
    }
    return res;
}

bool JsHeapDumpInfo::ReadFromParcel(Parcel &parcel)
{
    pid = parcel.ReadUint32();
    tid = parcel.ReadUint32();
    needGc = parcel.ReadBool();
    needSnapshot = parcel.ReadBool();
    if (fdVec.size() > MAX_TID_COUNT || tidVec.size() > MAX_TID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "fdVec or tidVec size more than 40.");
        return false;
    }
    parcel.ReadUInt32Vector(&fdVec);
    parcel.ReadUInt32Vector(&tidVec);

    auto msgParcel = static_cast<MessageParcel*>(&parcel);
    if (msgParcel == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadFromParcel failed.");
        return false;
    }
    fdVec.clear();
    for (auto &tid : tidVec) {
        uint32_t parcelFd = static_cast<uint32_t>(msgParcel->ReadFileDescriptor());
        fdVec.push_back(parcelFd);
    }
    return true;
}

JsHeapDumpInfo *JsHeapDumpInfo::Unmarshalling(Parcel &parcel)
{
    JsHeapDumpInfo *info = new (std::nothrow) JsHeapDumpInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "info nullptr");
        return nullptr;
    }
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::APPMGR, "JsHeapDumpInfo failed, because ReadFromParcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS
