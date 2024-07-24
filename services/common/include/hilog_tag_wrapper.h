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

#ifndef OHOS_AAFWK_HILOG_TAG_WRAPPER_H
#define OHOS_AAFWK_HILOG_TAG_WRAPPER_H

#include <cinttypes>
#include <map>

#include "hilog/log.h"

#ifndef AAFWK_FUNC_FMT
#define AAFWK_FUNC_FMT "[%{public}s(%{public}s:%{public}d)]"
#endif

#ifndef AAFWK_FILE_NAME
#define AAFWK_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef AAFWK_FUNC_INFO
#define AAFWK_FUNC_INFO AAFWK_FILE_NAME, __FUNCTION__, __LINE__
#endif


namespace OHOS::AAFwk {
enum class AAFwkLogTag : uint32_t {
    DEFAULT = 0xD001300,        // 0XD001300
    ABILITY,
    TEST,
    AA_TOOL,
    ABILITY_SIM,

    APPDFR = DEFAULT + 0x10,    // 0xD001310
    APPMGR,
    DBOBSMGR,
    DIALOG,
    QUICKFIX,
    URIPERMMGR,
    BUNDLEMGRHELPER,
    APPKIT,

    JSENV = DEFAULT + 0x20,     // 0xD001320
    JSRUNTIME,
    FA,
    INTENT,
    JSNAPI,
    CJRUNTIME,

    DELEGATOR = DEFAULT + 0x30, // 0xD001330
    CONTEXT,
    UIABILITY,
    WANT,
    MISSION,
    CONNECTION,
    ABILITYMGR,
    ECOLOGICAL_RULE,
    DATA_ABILITY,

    EXT = DEFAULT + 0x40,       // 0xD001340
    AUTOFILL_EXT,
    SERVICE_EXT,
    FORM_EXT,
    SHARE_EXT,
    UI_EXT,
    ACTION_EXT,
    EMBEDDED_EXT,
    UISERVC_EXT,

    WANTAGENT = DEFAULT + 0x50, // 0xD001350
    AUTOFILLMGR,
    EXTMGR,
    SER_ROUTER,
    AUTO_STARTUP,
    STARTUP,
    RECOVERY,
    PROCESSMGR,
    CONTINUATION,
    DISTRIBUTED,
    FREE_INSTALL,

    LOCAL_CALL = DEFAULT + 0x60, // 0xD001360

    END = 256,               // N.B. never use it
};

inline uint32_t GetOffset(AAFwkLogTag tag, AAFwkLogTag base)
{
    return static_cast<uint32_t>(tag) - static_cast<uint32_t>(base);
}

inline const char* GetDomainName0(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwk", "AAFwkAbility", "AAFwkTest", "AAFwkAATool", "AAFwkAbilitySimulator" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName1(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkAppDfr", "AAFwkAppMgr", "AAFwkDbObsMgr", "AAFwkDialog", "AAFwkQuickfix",
        "AAFwkUriPermMgr", "AAFwkBundleMgrHelper", "AAFwkAppKit" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::APPDFR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName2(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkJsEnv", "AAFwkJsRuntime", "AAFwkFA", "AAFwkIntent", "AAFwkJsNapi" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::JSENV);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName3(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkDelegator", "AAFwkContext", "AAFwkUIAbility", "AAFwkWant", "AAFwkMission",
        "AAFwkConnection", "AAFwkAbilityMgr", "AAFwkEcologicalRule", "AAFwkDataAbility" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DELEGATOR);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName4(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkExt", "AAFwkAutoFillExt", "AAFwkServiceExt", "AAFwkFormExt", "AAFwkShareExt",
        "AAFwkUIExt", "AAFwkActionExt", "AAFwkEmbeddedExt", "AAFwkUIServiceExt" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::EXT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName5(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkWantAgent", "AAFwkAutoFillMgr", "AAFwkExtMgr", "AAFwkServiceRouter",
        "AAFwkAutoStartup", "AAFwkStartup", "AAFwkRecovery", "AAFwkProcessMgr", "AAFwkContinuation",
        "AAFwkDistributed", "AAFwkFreeInstall" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::WANTAGENT);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

inline const char* GetDomainName6(AAFwkLogTag tag)
{
    const char* tagNames[] = { "AAFwkLocalCall" };
    uint32_t offset = GetOffset(tag, AAFwkLogTag::LOCAL_CALL);
    if (offset >= sizeof(tagNames) / sizeof(const char*)) {
        return "AAFwkUN";
    }
    return tagNames[offset];
}

constexpr uint32_t BASE_DEFAULT = 0;
constexpr uint32_t BASE_APPDFR = 1;
constexpr uint32_t BASE_JSENV = 2;
constexpr uint32_t BASE_DELEGATOR = 3;
constexpr uint32_t BASE_EXT = 4;
constexpr uint32_t BASE_WANTAGENT = 5;
constexpr uint32_t BASE_LOCAL_CALL = 6;

static inline const char* GetTagInfoFromDomainId(AAFwkLogTag tag)
{
    uint32_t offset = GetOffset(tag, AAFwkLogTag::DEFAULT);
    uint32_t base = offset >> 4;
    switch (base) {
        case BASE_DEFAULT: return GetDomainName0(tag);
        case BASE_APPDFR: return GetDomainName1(tag);
        case BASE_JSENV: return GetDomainName2(tag);
        case BASE_DELEGATOR: return GetDomainName3(tag);
        case BASE_EXT: return GetDomainName4(tag);
        case BASE_WANTAGENT: return GetDomainName5(tag);
        case BASE_LOCAL_CALL: return GetDomainName6(tag);
        default: return "AAFwkUN";
    }
}
} // OHOS::AAFwk

using AAFwkTag = OHOS::AAFwk::AAFwkLogTag;

#define AAFWK_PRINT_LOG(level, tag, fmt, ...)                                                           \
    do {                                                                                                \
        AAFwkTag logTag = tag;                                                                          \
        ((void)HILOG_IMPL(LOG_CORE, level, static_cast<uint32_t>(logTag),                                  \
        OHOS::AAFwk::GetTagInfoFromDomainId(logTag), AAFWK_FUNC_FMT fmt, AAFWK_FUNC_INFO, ##__VA_ARGS__)); \
    } while (0)

#define TAG_LOGD(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGI(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_INFO,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGW(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_WARN,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGE(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGF(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_FATAL, tag, fmt, ##__VA_ARGS__)

#endif  // OHOS_AAFWK_HILOG_TAG_WRAPPER_H
