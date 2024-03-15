/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_HILOG_WRAPPER_H
#define OHOS_AAFWK_HILOG_WRAPPER_H

#include <cinttypes>
#include <map>

#include "hilog/log.h"

#ifdef HILOG_FATAL
#undef HILOG_FATAL
#endif

#ifdef HILOG_ERROR
#undef HILOG_ERROR
#endif

#ifdef HILOG_WARN
#undef HILOG_WARN
#endif

#ifdef HILOG_INFO
#undef HILOG_INFO
#endif

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

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
    BASE = 0xD001300,        // 0XD001300
    ABILITY,
    TEST,
    AA_TOOL,
    SIMULATOR,

    APPDFR = BASE + 0x10,    // 0xD001310
    APPMGR,
    DBOBSMGR,
    DIALOG,
    QUICKFIX,
    URIPERMMGR,

    JSENV = BASE + 0x20,     // 0xD001320
    JSRUNTIME,
    FA,
    INTENT,
    JSNAPI,

    DELEGATOR = BASE + 0x30, // 0xD001330
    CONTEXT,
    UIABILITY,
    WANT,
    MISSION,
    CONNECTION,
    ATOMIC_SERVICE,
    ABILITYMGR,
    ECOLOGICAL_RULE,

    EXT = BASE + 0x40,       // 0xD001340
    AUTOFILL_EXT,
    SERVICE_EXT,
    FORM_EXT,
    SHARE_EXT,
    UI_EXT,
    ACTION_EXT,

    END = 256,               // N.B. never use it
};

const std::map<AAFwkLogTag, const char*> DOMAIN_MAP = {
    { AAFwkLogTag::BASE,      "AAFwk" },
    { AAFwkLogTag::ABILITY,   "AAFwkAbility" },
    { AAFwkLogTag::TEST,      "AAFwkTest" },
    { AAFwkLogTag::AA_TOOL,   "AAFwkAATool" },
    { AAFwkLogTag::SIMULATOR, "AAFwkSimulator" },

    { AAFwkLogTag::APPMGR,     "AAFwkAppMgr" },
    { AAFwkLogTag::DBOBSMGR,   "AAFwkDbObsMgr" },
    { AAFwkLogTag::DIALOG,     "AAFwkDialog" },
    { AAFwkLogTag::QUICKFIX,   "AAFwkQuickfix" },
    { AAFwkLogTag::URIPERMMGR, "AAFwkUriPermMgr" },

    { AAFwkLogTag::JSENV,     "AAFwkJsEnv" },
    { AAFwkLogTag::JSRUNTIME, "AAFwkJsRuntime" },
    { AAFwkLogTag::FA,        "AAFwkFA" },
    { AAFwkLogTag::INTENT,    "AAFwkIntent" },

    { AAFwkLogTag::DELEGATOR,  "AAFwkDelegator" },
    { AAFwkLogTag::CONTEXT,    "AAFwkContext" },
    { AAFwkLogTag::UIABILITY,  "AAFwkUIAbility" },
    { AAFwkLogTag::WANT,       "AAFwkWant" },
    { AAFwkLogTag::MISSION,    "AAFwkMission" },
    { AAFwkLogTag::ABILITYMGR, "AAFwkAbilityMgr" },

    { AAFwkLogTag::EXT,          "AAFwkExt" },
    { AAFwkLogTag::AUTOFILL_EXT, "AAFwkAutoFillExt" },
    { AAFwkLogTag::SERVICE_EXT,  "AAFwkServiceExt" },
    { AAFwkLogTag::FORM_EXT,     "AAFwkFormExt" },
    { AAFwkLogTag::UI_EXT,       "AAFwkUIExt" },
};
} // OHOS::AAFwk

using AAFwkTag = OHOS::AAFwk::AAFwkLogTag;

#define AAFWK_PRINT_LOG(level, tag, fmt, ...)                                                           \
    do {                                                                                                \
        ((void)HILOG_IMPL(LOG_CORE, level, static_cast<uint32_t>(tag), OHOS::AAFwk::DOMAIN_MAP.at(tag), \
        AAFWK_FUNC_FMT fmt, AAFWK_FUNC_INFO, ##__VA_ARGS__));                                           \
    } while (0)

#define HILOG_DEBUG(fmt, ...) TAG_LOGD(AAFwkTag::BASE, fmt, ##__VA_ARGS__)
#define HILOG_INFO(fmt, ...)  TAG_LOGI(AAFwkTag::BASE, fmt, ##__VA_ARGS__)
#define HILOG_WARN(fmt, ...)  TAG_LOGW(AAFwkTag::BASE, fmt, ##__VA_ARGS__)
#define HILOG_ERROR(fmt, ...) TAG_LOGE(AAFwkTag::BASE, fmt, ##__VA_ARGS__)
#define HILOG_FATAL(fmt, ...) TAG_LOGF(AAFwkTag::BASE, fmt, ##__VA_ARGS__)

#define TAG_LOGD(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGI(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_INFO,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGW(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_WARN,  tag, fmt, ##__VA_ARGS__)
#define TAG_LOGE(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGF(tag, fmt, ...) AAFWK_PRINT_LOG(LOG_FATAL, tag, fmt, ##__VA_ARGS__)

#endif  // OHOS_AAFWK_HILOG_WRAPPER_H
