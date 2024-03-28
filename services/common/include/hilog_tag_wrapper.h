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

    DELEGATOR = DEFAULT + 0x30, // 0xD001330
    CONTEXT,
    UIABILITY,
    WANT,
    MISSION,
    CONNECTION,
    ATOMIC_SERVICE,
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

    WANTAGENT = DEFAULT + 0x50, // 0xD001350
    AUTOFILLMGR,
    EXTMGR,
    SER_ROUTER,
    AUTO_STARTUP,
    RECOVERY,
    PROCESSMGR,
    CONTINUATION,
    DISTRIBUTED,
    FREE_INSTALL,

    LOCAL_CALL = DEFAULT + 0x60, // 0xD001360

    END = 256,               // N.B. never use it
};

const std::map<AAFwkLogTag, const char*> DOMAIN_MAP = {
    { AAFwkLogTag::DEFAULT,     "AAFwk" },
    { AAFwkLogTag::ABILITY,     "AAFwkAbility" },
    { AAFwkLogTag::TEST,        "AAFwkTest" },
    { AAFwkLogTag::AA_TOOL,     "AAFwkAATool" },
    { AAFwkLogTag::ABILITY_SIM, "AAFwkAbilitySimulator" },

    { AAFwkLogTag::APPDFR,          "AAFwkAppDfr"},
    { AAFwkLogTag::APPMGR,          "AAFwkAppMgr" },
    { AAFwkLogTag::DBOBSMGR,        "AAFwkDbObsMgr" },
    { AAFwkLogTag::DIALOG,          "AAFwkDialog" },
    { AAFwkLogTag::QUICKFIX,        "AAFwkQuickfix" },
    { AAFwkLogTag::URIPERMMGR,      "AAFwkUriPermMgr" },
    { AAFwkLogTag::BUNDLEMGRHELPER, "AAFwkBundleMgrHelper" },
    { AAFwkLogTag::APPKIT,          "AAFwkAppKit" },

    { AAFwkLogTag::JSENV,     "AAFwkJsEnv" },
    { AAFwkLogTag::JSRUNTIME, "AAFwkJsRuntime" },
    { AAFwkLogTag::FA,        "AAFwkFA" },
    { AAFwkLogTag::INTENT,    "AAFwkIntent" },
    { AAFwkLogTag::JSNAPI,    "AAFwkJsNapi" },

    { AAFwkLogTag::DELEGATOR,       "AAFwkDelegator" },
    { AAFwkLogTag::CONTEXT,         "AAFwkContext" },
    { AAFwkLogTag::UIABILITY,       "AAFwkUIAbility" },
    { AAFwkLogTag::WANT,            "AAFwkWant" },
    { AAFwkLogTag::MISSION,         "AAFwkMission" },
    { AAFwkLogTag::CONNECTION,      "AAFwkConnection" },
    { AAFwkLogTag::ATOMIC_SERVICE,  "AAFwkAtomicService" },
    { AAFwkLogTag::ABILITYMGR,      "AAFwkAbilityMgr" },
    { AAFwkLogTag::ECOLOGICAL_RULE, "AAFwkEcologicalRule" },
    { AAFwkLogTag::DATA_ABILITY,    "AAFwkDataAbility" },

    { AAFwkLogTag::EXT,          "AAFwkExt" },
    { AAFwkLogTag::AUTOFILL_EXT, "AAFwkAutoFillExt" },
    { AAFwkLogTag::SERVICE_EXT,  "AAFwkServiceExt" },
    { AAFwkLogTag::FORM_EXT,     "AAFwkFormExt" },
    { AAFwkLogTag::SHARE_EXT,    "AAFwkShareExt" },
    { AAFwkLogTag::UI_EXT,       "AAFwkUIExt" },
    { AAFwkLogTag::ACTION_EXT,   "AAFwkActionExt" },
    { AAFwkLogTag::EMBEDDED_EXT, "AAFwkEmbeddedExt" },

    { AAFwkLogTag::WANTAGENT,    "AAFwkWantAgent" },
    { AAFwkLogTag::AUTOFILLMGR,  "AAFwkAutoFillMgr" },
    { AAFwkLogTag::EXTMGR,       "AAFwkExtMgr" },
    { AAFwkLogTag::SER_ROUTER,   "AAFwkServiceRouter" },
    { AAFwkLogTag::AUTO_STARTUP, "AAFwkAutoStartup" },
    { AAFwkLogTag::RECOVERY,     "AAFwkRecovery" },
    { AAFwkLogTag::PROCESSMGR,   "AAFwkProcessMgr" },
    { AAFwkLogTag::CONTINUATION, "AAFwkContinuation" },
    { AAFwkLogTag::DISTRIBUTED,  "AAFwkDistributed" },
    { AAFwkLogTag::FREE_INSTALL, "AAFwkFreeInstall" },

    { AAFwkLogTag::LOCAL_CALL, "AAFwkLocalCall" },
};

static inline const char* GetTagInfoFromDomainId(AAFwkLogTag tag)
{
    if (DOMAIN_MAP.find(tag) == DOMAIN_MAP.end()) {
        tag = AAFwkLogTag::DEFAULT;
    }
    return DOMAIN_MAP.at(tag);
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
