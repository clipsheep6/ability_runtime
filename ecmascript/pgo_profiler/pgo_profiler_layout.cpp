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

#include "ecmascript/pgo_profiler/pgo_profiler_layout.h"
#include "ecmascript/js_hclass.h"
#include "ecmascript/js_hclass-inl.h"
#include "ecmascript/js_tagged_value.h"

namespace panda::ecmascript::pgo {
void PGOHClassTreeDesc::Clear()
{
    IterateAll([] (HClassLayoutDesc *desc) {
        delete desc;
    });
    transitionLayout_.clear();
}

void PGOHClassTreeDesc::Merge(const PGOHClassTreeDesc &from)
{
    ASSERT(from.GetProfileType() == GetProfileType());
    from.IterateAll([this] (HClassLayoutDesc *fromDesc) {
        auto curLayoutDesc = GetHClassLayoutDesc(fromDesc->GetProfileType());
        if (curLayoutDesc == nullptr) {
            if (fromDesc->GetProfileType().IsRootType()) {
                RootHClassLayoutDesc *rootFromTreeDesc = reinterpret_cast<RootHClassLayoutDesc *>(fromDesc);
                curLayoutDesc = new RootHClassLayoutDesc(*rootFromTreeDesc);
            } else {
                ChildHClassLayoutDesc *childFromTreeDesc = reinterpret_cast<ChildHClassLayoutDesc *>(fromDesc);
                curLayoutDesc = new ChildHClassLayoutDesc(*childFromTreeDesc);
            }
            transitionLayout_.emplace(fromDesc->GetProfileType(), curLayoutDesc);
        } else {
            curLayoutDesc->Merge(fromDesc);
        }
    });
    elementTrackInfo_ = from.GetElementsTrackInfo();
}

HClassLayoutDesc *PGOHClassTreeDesc::GetHClassLayoutDesc(ProfileType type) const
{
    auto iter = transitionLayout_.find(type);
    if (iter != transitionLayout_.end()) {
        return iter->second;
    }
    return nullptr;
}

HClassLayoutDesc *PGOHClassTreeDesc::GetOrInsertHClassLayoutDesc(ProfileType type, bool root)
{
    auto iter = transitionLayout_.find(type);
    if (iter != transitionLayout_.end()) {
        return iter->second;
    } else {
        HClassLayoutDesc *layout;
        if (root) {
            layout = new RootHClassLayoutDesc(type);
        } else {
            layout = new ChildHClassLayoutDesc(type);
        }
        transitionLayout_.emplace(type, layout);
        return layout;
    }
}

bool PGOHClassTreeDesc::DumpForRoot(JSTaggedType root, ProfileType rootType)
{
    ASSERT(rootType.IsRootType());
    HClassLayoutDesc *rootLayout;
    auto iter = transitionLayout_.find(rootType);
    auto rootHClass = JSHClass::Cast(JSTaggedValue(root).GetTaggedObject());
    if (iter != transitionLayout_.end()) {
        rootLayout = iter->second;
    } else {
        rootLayout = new RootHClassLayoutDesc(rootType, rootHClass->GetObjectType(),
                                              rootHClass->GetObjectSizeExcludeInlinedProps());
        transitionLayout_.emplace(rootType, rootLayout);
    }

    return JSHClass::DumpForRootHClass(rootHClass, rootLayout);
}

bool PGOHClassTreeDesc::DumpForChild(JSTaggedType child, ProfileType childType)
{
    ASSERT(!childType.IsRootType());
    auto rootType = GetProfileType();
    auto rootIter = transitionLayout_.find(rootType);
    auto childHClass = JSHClass::Cast(JSTaggedValue(child).GetTaggedObject());
    if (rootIter != transitionLayout_.end()) {
        auto rootLayout = rootIter->second;
        JSHClass::UpdateRootLayoutDesc(childHClass, this, rootLayout);
    }

    HClassLayoutDesc *childLayout;
    auto iter = transitionLayout_.find(childType);
    if (iter != transitionLayout_.end()) {
        childLayout = iter->second;
    } else {
        childLayout = new ChildHClassLayoutDesc(childType);
        transitionLayout_.emplace(childType, childLayout);
    }

    return JSHClass::DumpForChildHClass(childHClass, childLayout);
}

bool PGOHClassTreeDesc::UpdateForChild(ProfileType rootType, JSTaggedType child, ProfileType childType)
{
    ASSERT(rootType.IsRootType());
    ASSERT(!childType.IsRootType());
    auto rootIter = transitionLayout_.find(rootType);
    auto hclass = JSHClass::Cast(JSTaggedValue(child).GetTaggedObject());
    if (rootIter != transitionLayout_.end()) {
        auto rootLayout = rootIter->second;
        JSHClass::UpdateRootLayoutDesc(hclass, this, rootLayout);
    }
    auto childIter = transitionLayout_.find(childType);
    if (childIter != transitionLayout_.end()) {
        auto childLayout = childIter->second;
        JSHClass::UpdateChildLayoutDesc(hclass, childLayout);
    }
    return true;
}

bool PGOHClassTreeDesc::UpdateLayout(ProfileType rootType, JSTaggedType curHClass, ProfileType curType)
{
    if (curType.IsRootType()) {
        return DumpForRoot(curHClass, curType);
    } else {
        return UpdateForChild(rootType, curHClass, curType);
    }
}

bool PGOHClassTreeDesc::DumpForTransition(
    JSTaggedType parent, ProfileType parentType, JSTaggedType child, ProfileType childType)
{
    if (parentType.IsRootType()) {
        if (!DumpForRoot(parent, parentType)) {
            return false;
        }
    } else {
        if (!DumpForChild(parent, parentType)) {
            return false;
        }
    }
    bool ret = DumpForChild(child, childType);
    auto parentLayoutDesc = transitionLayout_.find(parentType)->second;
    auto childLayoutDesc = transitionLayout_.find(childType)->second;
    parentLayoutDesc->AddChildHClassLayoutDesc(childLayoutDesc->GetProfileType());
    return ret;
}

void HClassLayoutDesc::Merge(const HClassLayoutDesc *from)
{
    from->IterateChilds([this] (const ProfileType &type) -> bool {
        AddChildHClassLayoutDesc(type);
        return true;
    });
}

void HClassLayoutDesc::InsertKeyAndDesc(const PGOHandler &handler, PropertyDesc &desc)
{
    PGOHandler oldHandler = desc.second;
    if (oldHandler == handler) {
        return;
    }
    auto oldTrackType = oldHandler.GetTrackType();
    auto newTrackType = handler.GetTrackType();
    if (oldTrackType == newTrackType) {
        desc.second.SetPropertyMeta(handler.GetPropertyMeta());
        return;
    }

    switch (oldTrackType) {
        case TrackType::TAGGED:
            desc.second.SetPropertyMeta(handler.GetPropertyMeta());
            break;
        case TrackType::NONE:
        case TrackType::INT:
        case TrackType::DOUBLE:
            if (newTrackType != TrackType::TAGGED) {
                newTrackType = static_cast<TrackType>(static_cast<uint8_t>(newTrackType) |
                    static_cast<uint8_t>(oldTrackType));
            }
            desc.second = PGOHandler(newTrackType, handler.GetPropertyMeta());
            break;
        default:
            break;
    }
}

void RootHClassLayoutDesc::Merge(const HClassLayoutDesc *from)
{
    ASSERT(from->GetProfileType() == GetProfileType());
    ASSERT(from->GetProfileType().IsRootType());
    auto fromDesc = reinterpret_cast<const RootHClassLayoutDesc *>(from);
    fromDesc->IterateProps([this] (const PropertyDesc &desc) {
        InsertKeyAndDesc(desc.first, desc.second);
    });
    HClassLayoutDesc::Merge(from);
}

void RootHClassLayoutDesc::InsertKeyAndDesc(const CString &key, const PGOHandler &handler)
{
    if (!UpdateKeyAndDesc(key, handler)) {
        layoutDesc_.emplace_back(key, handler);
    }
}

bool RootHClassLayoutDesc::UpdateKeyAndDesc(const CString &key, const PGOHandler &handler)
{
    for (auto &iter : layoutDesc_) {
        if (iter.first == key) {
            HClassLayoutDesc::InsertKeyAndDesc(handler, iter);
            return true;
        }
    }
    return false;
}

void ChildHClassLayoutDesc::Merge(const HClassLayoutDesc *from)
{
    ASSERT(from->GetProfileType() == GetProfileType());
    ASSERT(!from->GetProfileType().IsRootType());
    auto fromDesc = reinterpret_cast<const ChildHClassLayoutDesc *>(from);
    auto fromPropDesc = fromDesc->GetPropertyDesc();
    InsertKeyAndDesc(fromPropDesc.first, fromPropDesc.second);
    HClassLayoutDesc::Merge(from);
}

void ChildHClassLayoutDesc::InsertKeyAndDesc(const CString &key, const PGOHandler &handler)
{
    if (!UpdateKeyAndDesc(key, handler)) {
        propertyDesc_ = PropertyDesc(key, handler);
    }
}

bool ChildHClassLayoutDesc::UpdateKeyAndDesc(const CString &key, const PGOHandler &handler)
{
    if (propertyDesc_.first == key) {
        HClassLayoutDesc::InsertKeyAndDesc(handler, propertyDesc_);
        return true;
    }
    return false;
}
} // namespace panda::ecmascript::pgo
