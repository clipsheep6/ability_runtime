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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H

#include "errors.h"

namespace OHOS {
enum {
    APPEXECFWK_MODULE_COMMON = 0x00,
    APPEXECFWK_MODULE_APPMGR = 0x01,
    APPEXECFWK_MODULE_BUNDLEMGR = 0x02,
    // Reserved 0x03 ~ 0x0f for new modules, Event related modules start from 0x10
    APPEXECFWK_MODULE_EVENTMGR = 0x10,
    APPEXECFWK_MODULE_HIDUMP = 0x11
};

// Error code for Common
constexpr ErrCode APPEXECFWK_COMMON_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_COMMON);
enum {
    ERR_APPEXECFWK_SERVICE_NOT_READY = APPEXECFWK_COMMON_ERR_OFFSET + 1,
    ERR_APPEXECFWK_SERVICE_NOT_CONNECTED,
    ERR_APPEXECFWK_INVALID_UID,
    ERR_APPEXECFWK_INVALID_PID,
    ERR_APPEXECFWK_PARCEL_ERROR,
    ERR_APPEXECFWK_FAILED_SERVICE_DIED,
    ERR_APPEXECFWK_OPERATION_TIME_OUT,
    ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR,
};

// Error code for AppMgr
constexpr ErrCode APPEXECFWK_APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_APPMGR);
enum {
    ERR_APPEXECFWK_ASSEMBLE_START_MSG_FAILED = APPEXECFWK_APPMGR_ERR_OFFSET + 1,
    ERR_APPEXECFWK_CONNECT_APPSPAWN_FAILED,
    ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT,
    ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET,
    ERR_APPEXECFWK_SOCKET_READ_FAILED,
    ERR_APPEXECFWK_SOCKET_WRITE_FAILED
};

// Error code for BundleMgr
constexpr ErrCode APPEXECFWK_BUNDLEMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_BUNDLEMGR);
enum {
    // the install error code from 0x0001 ~ 0x0020.
    ERR_APPEXECFWK_INSTALL_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0001, // 8519681
    ERR_APPEXECFWK_INSTALL_HOST_INSTALLER_FAILED = 8519682,
    ERR_APPEXECFWK_INSTALL_PARSE_FAILED = 8519683,
    ERR_APPEXECFWK_INSTALL_VERSION_DOWNGRADE = 8519684,
    ERR_APPEXECFWK_INSTALL_VERIFICATION_FAILED = 8519685,
    ERR_APPEXECFWK_INSTALL_PARAM_ERROR = 8519686,
    ERR_APPEXECFWK_INSTALL_PERMISSION_DENIED = 8519687,
    ERR_APPEXECFWK_INSTALL_ENTRY_ALREADY_EXIST = 8519688,
    ERR_APPEXECFWK_INSTALL_STATE_ERROR = 8519689,
    ERR_APPEXECFWK_INSTALL_FILE_PATH_INVALID = 8519690,
    ERR_APPEXECFWK_INSTALL_INVALID_HAP_NAME = 8519691,
    ERR_APPEXECFWK_INSTALL_INVALID_BUNDLE_FILE = 8519692,
    ERR_APPEXECFWK_INSTALL_INVALID_HAP_SIZE = 8519693,
    ERR_APPEXECFWK_INSTALL_GENERATE_UID_ERROR = 8519694,
    ERR_APPEXECFWK_INSTALL_INSTALLD_SERVICE_ERROR = 8519695,
    ERR_APPEXECFWK_INSTALL_BUNDLE_MGR_SERVICE_ERROR = 8519696,
    ERR_APPEXECFWK_INSTALL_ALREADY_EXIST = 8519697,
    ERR_APPEXECFWK_INSTALL_BUNDLENAME_NOT_SAME = 8519698,
    ERR_APPEXECFWK_INSTALL_VERSIONCODE_NOT_SAME = 8519699,
    ERR_APPEXECFWK_INSTALL_VERSIONNAME_NOT_SAME = 8519700,
    ERR_APPEXECFWK_INSTALL_MINCOMPATIBLE_VERSIONCODE_NOT_SAME = 8519701,
    ERR_APPEXECFWK_INSTALL_VENDOR_NOT_SAME = 8519702,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_TARGET_NOT_SAME = 8519703,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_NOT_SAME = 8519704,
    ERR_APPEXECFWK_INSTALL_RELEASETYPE_COMPATIBLE_NOT_SAME = 8519705,
    ERR_APPEXECFWK_INSTALL_VERSION_NOT_COMPATIBLE = 8519706,
    ERR_APPEXECFWK_INSTALL_APP_DISTRIBUTION_TYPE_NOT_SAME = 8519707,
    ERR_APPEXECFWK_INSTALL_APP_PROVISION_TYPE_NOT_SAME = 8519708,
    ERR_APPEXECFWK_INSTALL_INVALID_NUMBER_OF_ENTRY_HAP = 8519709,
    ERR_APPEXECFWK_INSTALL_DISK_MEM_INSUFFICIENT = 8519710,
    ERR_APPEXECFWK_INSTALL_GRANT_REQUEST_PERMISSIONS_FAILED = 8519711,
    ERR_APPEXECFWK_INSTALL_UPDATE_HAP_TOKEN_FAILED = 8519712,
    ERR_APPEXECFWK_INSTALL_SINGLETON_NOT_SAME = 8519713,
    ERR_APPEXECFWK_INSTALL_ZERO_USER_WITH_NO_SINGLETON = 8519714,
    ERR_APPEXECFWK_INSTALL_CHECK_SYSCAP_FAILED = 8519715,
    ERR_APPEXECFWK_INSTALL_APPTYPE_NOT_SAME = 8519716,
    ERR_APPEXECFWK_INSTALL_URI_DUPLICATE = 8519717,
    ERR_APPEXECFWK_INSTALL_TYPE_ERROR = 8519718,
    ERR_APPEXECFWK_INSTALL_SDK_INCOMPATIBLE = 8519719,
    ERR_APPEXECFWK_INSTALL_SO_INCOMPATIBLE = 8519720,
    ERR_APPEXECFWK_INSTALL_AN_INCOMPATIBLE = 8519721,
    ERR_APPEXECFWK_INSTALL_NOT_UNIQUE_DISTRO_MODULE_NAME = 8519722,
    ERR_APPEXECFWK_INSTALL_INCONSISTENT_MODULE_NAME = 8519723,
    ERR_APPEXECFWK_INSTALL_SINGLETON_INCOMPATIBLE = 8519724,
    ERR_APPEXECFWK_INSTALL_DEVICE_TYPE_NOT_SUPPORTED = 8519725,
    ERR_APPEXECFWK_INSTALL_COPY_HAP_FAILED = 8519726,
    ERR_APPEXECFWK_INSTALL_DEPENDENT_MODULE_NOT_EXIST = 8519727,
    ERR_APPEXECFWK_INSTALL_ASAN_ENABLED_NOT_SAME = 85197288,
    ERR_APPEXECFWK_INSTALL_ASAN_NOT_SUPPORT = 8519729,
    ERR_APPEXECFWK_BUNDLE_TYPE_NOT_SAME = 8519730,
    ERR_APPEXECFWK_INSTALL_SHARE_APP_LIBRARY_NOT_ALLOWED = 8519731,
    ERR_APPEXECFWK_INSTALL_COMPATIBLE_POLICY_NOT_SAME = 8519732,
    ERR_APPEXECFWK_INSTALL_FILE_IS_SHARED_LIBRARY = 8519733,
    ERR_APPEXECFWK_INSTALL_CHECK_PROXY_DATA_URI_FAILED = 8519734,
    ERR_APPEXECFWK_INSTALL_CHECK_PROXY_DATA_PERMISSION_FAILED = 8519735,
    ERR_APPEXECFWK_INSTALL_DEBUG_NOT_SAME = 8519736,
    ERR_APPEXECFWK_INSTALL_ISOLATION_MODE_FAILED = 8519737,
    ERR_BUNDLEMANAGER_INSTALL_CODE_SIGNATURE_FAILED = 8519738,
    ERR_BUNDLEMANAGER_INSTALL_CODE_SIGNATURE_FILE_IS_INVALID = 8519739,

    // signature errcode
    ERR_APPEXECFWK_INSTALL_FAILED_INVALID_SIGNATURE_FILE_PATH = 8519740,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE_FILE = 8519741,
    ERR_APPEXECFWK_INSTALL_FAILED_NO_BUNDLE_SIGNATURE = 8519742,
    ERR_APPEXECFWK_INSTALL_FAILED_VERIFY_APP_PKCS7_FAIL = 8519743,
    ERR_APPEXECFWK_INSTALL_FAILED_PROFILE_PARSE_FAIL = 8519744,
    ERR_APPEXECFWK_INSTALL_FAILED_APP_SOURCE_NOT_TRUESTED = 8519745,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_DIGEST = 8519746,
    ERR_APPEXECFWK_INSTALL_FAILED_BUNDLE_INTEGRITY_VERIFICATION_FAILURE = 8519747,
    ERR_APPEXECFWK_INSTALL_FAILED_FILE_SIZE_TOO_LARGE = 8519748,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_PUBLICKEY = 8519749,
    ERR_APPEXECFWK_INSTALL_FAILED_BAD_BUNDLE_SIGNATURE = 8519750,
    ERR_APPEXECFWK_INSTALL_FAILED_NO_PROFILE_BLOCK_FAIL = 8519751,
    ERR_APPEXECFWK_INSTALL_FAILED_BUNDLE_SIGNATURE_VERIFICATION_FAILURE = 8519752,
    ERR_APPEXECFWK_INSTALL_FAILED_VERIFY_SOURCE_INIT_FAIL = 8519753,
    ERR_APPEXECFWK_INSTALL_FAILED_INCOMPATIBLE_SIGNATURE = 8519754,
    ERR_APPEXECFWK_INSTALL_FAILED_INCONSISTENT_SIGNATURE = 8519755,
    ERR_APPEXECFWK_INSTALL_FAILED_MODULE_NAME_EMPTY = 8519756,
    ERR_APPEXECFWK_INSTALL_FAILED_MODULE_NAME_DUPLICATE = 8519757,
    ERR_APPEXECFWK_INSTALL_FAILED_CHECK_HAP_HASH_PARAM = 8519758,
    ERR_BUNDLEMANAGER_INSTALL_FAILED_SIGNATURE_EXTENSION_NOT_EXISTED = 8519759,

    ERR_APPEXECFWK_INSTALL_ENTERPRISE_BUNDLE_NOT_ALLOWED = 8519780,

    // sandbox app install
    ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR = 8519800,
    ERR_APPEXECFWK_SANDBOX_INSTALL_APP_NOT_EXISTED = 8519801,
    ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR = 8519802,
    ERR_APPEXECFWK_SANDBOX_INSTALL_WRITE_PARCEL_ERROR = 8519803,
    ERR_APPEXECFWK_SANDBOX_INSTALL_READ_PARCEL_ERROR = 8519804,
    ERR_APPEXECFWK_SANDBOX_INSTALL_SEND_REQUEST_ERROR = 8519805,
    ERR_APPEXECFWK_SANDBOX_INSTALL_USER_NOT_EXIST = 8519806,
    ERR_APPEXECFWK_SANDBOX_INSTALL_INVALID_APP_INDEX = 8519807,
    ERR_APPEXECFWK_SANDBOX_INSTALL_NOT_INSTALLED_AT_SPECIFIED_USERID = 8519808,
    ERR_APPEXECFWK_SANDBOX_INSTALL_NO_SANDBOX_APP_INFO = 8519809,
    ERR_APPEXECFWK_SANDBOX_INSTALL_UNKNOWN_INSTALL_TYPE = 8519810,
    ERR_APPEXECFWK_SANDBOX_INSTALL_DELETE_APP_INDEX_FAILED = 8519811,
    ERR_APPEXECFWK_SANDBOX_APP_NOT_SUPPORTED = 8519812,
    ERR_APPEXECFWK_SANDBOX_INSTALL_GET_PERMISSIONS_FAILED = 8519813,
    ERR_APPEXECFWK_SANDBOX_INSTALL_DATABASE_OPERATION_FAILED = 8519814,

    // sandbox app query
    ERR_APPEXECFWK_SANDBOX_QUERY_PARAM_ERROR = 8519815,
    ERR_APPEXECFWK_SANDBOX_QUERY_INTERNAL_ERROR = 8519816,
    ERR_APPEXECFWK_SANDBOX_QUERY_INVALID_USER_ID = 8519817,
    ERR_APPEXECFWK_SANDBOX_QUERY_NO_SANDBOX_APP = 85198188,
    ERR_APPEXECFWK_SANDBOX_QUERY_NO_MODULE_INFO = 8519819,
    ERR_APPEXECFWK_SANDBOX_QUERY_NO_USER_INFO = 8519820,

    ERR_APPEXECFWK_PARSE_UNEXPECTED = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x00c8, // 8519880
    ERR_APPEXECFWK_PARSE_MISSING_BUNDLE = 8519881,
    ERR_APPEXECFWK_PARSE_MISSING_ABILITY = 8519882,
    ERR_APPEXECFWK_PARSE_NO_PROFILE = 8519883,
    ERR_APPEXECFWK_PARSE_BAD_PROFILE = 8519884,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR = 8519885,
    ERR_APPEXECFWK_PARSE_PROFILE_MISSING_PROP = 8519886,
    ERR_APPEXECFWK_PARSE_PERMISSION_ERROR = 8519887,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_CHECK_ERROR = 8519888,
    ERR_APPEXECFWK_PARSE_PROFILE_PROP_SIZE_CHECK_ERROR = 8519889,
    ERR_APPEXECFWK_PARSE_RPCID_FAILED = 8519890,
    ERR_APPEXECFWK_PARSE_NATIVE_SO_FAILED = 8519891,
    ERR_APPEXECFWK_PARSE_AN_FAILED = 8519892,

    ERR_APPEXECFWK_INSTALLD_PARAM_ERROR = 8519893,
    ERR_APPEXECFWK_INSTALLD_GET_PROXY_ERROR = 8519894,
    ERR_APPEXECFWK_INSTALLD_CREATE_DIR_FAILED = 8519895,
    ERR_APPEXECFWK_INSTALLD_CREATE_DIR_EXIST = 8519896,
    ERR_APPEXECFWK_INSTALLD_CHOWN_FAILED = 8519897,
    ERR_APPEXECFWK_INSTALLD_REMOVE_DIR_FAILED = 8519898,
    ERR_APPEXECFWK_INSTALLD_EXTRACT_FILES_FAILED = 8519899,
    ERR_APPEXECFWK_INSTALLD_RNAME_DIR_FAILED = 8519900,
    ERR_APPEXECFWK_INSTALLD_CLEAN_DIR_FAILED = 8519901,
    ERR_APPEXECFWK_INSTALLD_MOVE_FILE_FAILED = 8519902,
    ERR_APPEXECFWK_INSTALLD_COPY_FILE_FAILED = 8519903,
    ERR_APPEXECFWK_INSTALLD_MKDIR_FAILED = 8519904,
    ERR_APPEXECFWK_INSTALLD_PERMISSION_DENIED = 8519905,
    ERR_APPEXECFWK_INSTALLD_SET_SELINUX_LABEL_FAILED = 8519906,
    ERR_APPEXECFWK_INSTALLD_AOT_EXECUTE_FAILED = 8519907,
    ERR_APPEXECFWK_INSTALLD_AOT_ABC_NOT_EXIST = 8519908,

    ERR_APPEXECFWK_UNINSTALL_SYSTEM_APP_ERROR = 8519909,
    ERR_APPEXECFWK_UNINSTALL_KILLING_APP_ERROR = 8519910,
    ERR_APPEXECFWK_UNINSTALL_INVALID_NAME = 8519911,
    ERR_APPEXECFWK_UNINSTALL_PARAM_ERROR = 8519912,
    ERR_APPEXECFWK_UNINSTALL_PERMISSION_DENIED = 8519913,
    ERR_APPEXECFWK_UNINSTALL_BUNDLE_MGR_SERVICE_ERROR = 8519914,
    ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_BUNDLE = 8519915,
    ERR_APPEXECFWK_UNINSTALL_MISSING_INSTALLED_MODULE = 8519916,
    ERR_APPEXECFWK_UNINSTALL_SHARE_APP_LIBRARY_IS_NOT_EXIST = 8519917,
    ERR_APPEXECFWK_UNINSTALL_SHARE_APP_LIBRARY_IS_RELIED = 8519918,
    ERR_APPEXECFWK_UNINSTALL_BUNDLE_IS_SHARED_LIBRARY = 8519919,

    ERR_APPEXECFWK_FAILED_GET_INSTALLER_PROXY = 8519920,
    ERR_APPEXECFWK_FAILED_GET_BUNDLE_INFO = 8519921,
    ERR_APPEXECFWK_FAILED_GET_ABILITY_INFO = 8519922,
    ERR_APPEXECFWK_FAILED_GET_RESOURCEMANAGER = 8519923,
    ERR_APPEXECFWK_FAILED_GET_REMOTE_PROXY = 8519924,
    ERR_APPEXECFWK_PERMISSION_DENIED = 8519925,
    ERR_APPEXECFWK_INPUT_WRONG_TYPE_FILE = 8519926,
    ERR_APPEXECFWK_ENCODE_BASE64_FILE_FAILED = 8519927,

    ERR_APPEXECFWK_RECOVER_GET_BUNDLEPATH_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0201, // 8520193
    ERR_APPEXECFWK_RECOVER_INVALID_BUNDLE_NAME = 8520194,
    ERR_APPEXECFWK_RECOVER_NOT_ALLOWED = 8520195,

    ERR_APPEXECFWK_USER_NOT_EXIST = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0301, // 8520449
    ERR_APPEXECFWK_USER_CREATE_FAILED = 8520450,
    ERR_APPEXECFWK_USER_REMOVE_FAILED = 8520451,
    ERR_APPEXECFWK_USER_NOT_INSTALL_HAP = 8520452,

    // error code in prebundle sacn
    ERR_APPEXECFWK_PARSE_FILE_FAILED = 8520453,

    // debug mode
    ERR_BUNDLEMANAGER_SET_DEBUG_MODE_INTERNAL_ERROR = 8520454,
    ERR_BUNDLEMANAGER_SET_DEBUG_MODE_PARCEL_ERROR = 8520455,
    ERR_BUNDLEMANAGER_SET_DEBUG_MODE_SEND_REQUEST_ERROR = 8520456,
    ERR_BUNDLEMANAGER_SET_DEBUG_MODE_UID_CHECK_FAILED = 8520457,
    ERR_BUNDLEMANAGER_SET_DEBUG_MODE_INVALID_PARAM = 8520458,

    // overlay installation errcode
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INTERNAL_ERROR = 8520600,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INVALID_BUNDLE_NAME = 8520601,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INVALID_MODULE_NAME = 8520602,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_ERROR_HAP_TYPE = 8520603,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_ERROR_BUNDLE_TYPE = 8520604,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_NAME_MISSED = 8520605,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_MODULE_NAME_MISSED = 8520606,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_NAME_NOT_SAME = 8520607,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INTERNAL_EXTERNAL_OVERLAY_EXISTED_SIMULTANEOUSLY = 8520608,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_PRIORITY_NOT_SAME = 8520609,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INVALID_PRIORITY = 8520610,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INCONSISTENT_VERSION_CODE = 8520611,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_SERVICE_EXCEPTION = 8520612,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_BUNDLE_NAME_SAME_WITH_TARGET_BUNDLE_NAME = 8520613,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_NO_SYSTEM_APPLICATION_FOR_EXTERNAL_OVERLAY = 8520614,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_DIFFERENT_SIGNATURE_CERTIFICATE = 8520615,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_IS_OVERLAY_BUNDLE = 8520616,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_MODULE_IS_OVERLAY_MODULE = 8520617,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_OVERLAY_TYPE_NOT_SAME = 8520618,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_INVALID_BUNDLE_DIR = 8520619,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_IS_NOT_STAGE_MODULE = 8520620,
    ERR_BUNDLEMANAGER_OVERLAY_INSTALLATION_FAILED_TARGET_BUNDLE_IS_SERVICE = 8520621,

    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_PARAM_ERROR = 8520622,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_MISSING_OVERLAY_BUNDLE = 8520623,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_MISSING_OVERLAY_MODULE = 8520624,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NON_OVERLAY_BUNDLE = 8520625,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NON_OVERLAY_MODULE = 8520626,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_BUNDLE_NOT_INSTALLED_AT_SPECIFIED_USERID = 8520627,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_TARGET_BUNDLE_NOT_EXISTED = 8520628,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_TARGET_MODULE_NOT_EXISTED = 8520629,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NO_OVERLAY_BUNDLE_INFO = 8520630,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_NO_OVERLAY_MODULE_INFO = 8520631,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_PERMISSION_DENIED = 8520632,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_TARGET_MODULE_IS_OVERLAY_MODULE = 8520633,
    ERR_BUNDLEMANAGER_OVERLAY_QUERY_FAILED_TARGET_BUNDLE_IS_OVERLAY_BUNDLE = 8520634,
    ERR_BUNDLEMANAGER_OVERLAY_SET_OVERLAY_PARAM_ERROR = 8520635,

    // quick fix errcode
    ERR_BUNDLEMANAGER_QUICK_FIX_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0401, // 8520705
    ERR_BUNDLEMANAGER_QUICK_FIX_PARAM_ERROR = 8520706,
    ERR_BUNDLEMANAGER_QUICK_FIX_PROFILE_PARSE_FAILED = 8520707,
    ERR_BUNDLEMANAGER_QUICK_FIX_BUNDLE_NAME_NOT_SAME = 8520708,
    ERR_BUNDLEMANAGER_QUICK_FIX_VERSION_CODE_NOT_SAME = 8520709,
    ERR_BUNDLEMANAGER_QUICK_FIX_VERSION_NAME_NOT_SAME = 8520710,
    ERR_BUNDLEMANAGER_QUICK_FIX_PATCH_VERSION_CODE_NOT_SAME = 8520711,
    ERR_BUNDLEMANAGER_QUICK_FIX_PATCH_VERSION_NAME_NOT_SAME = 8520712,
    ERR_BUNDLEMANAGER_QUICK_FIX_PATCH_TYPE_NOT_SAME = 8520713,
    ERR_BUNDLEMANAGER_QUICK_FIX_MODULE_NAME_SAME = 8520714,
    ERR_BUNDLEMANAGER_QUICK_FIX_UNKNOWN_QUICK_FIX_TYPE = 8520715,
    ERR_BUNDLEMANAGER_QUICK_FIX_SO_INCOMPATIBLE = 8520716,
    ERR_BUNDLEMANAGER_QUICK_FIX_BUNDLE_NAME_NOT_EXIST = 8520717,
    ERR_BUNDLEMANAGER_QUICK_FIX_MODULE_NAME_NOT_EXIST = 8520718,
    ERR_BUNDLEMANAGER_QUICK_FIX_SIGNATURE_INFO_NOT_SAME = 8520719,
    ERR_BUNDLEMANAGER_QUICK_FIX_ADD_HQF_FAILED = 8520720,
    ERR_BUNDLEMANAGER_QUICK_FIX_SAVE_APP_QUICK_FIX_FAILED = 8520721,
    ERR_BUNDLEMANAGER_QUICK_FIX_VERSION_CODE_ERROR = 8520722,
    ERR_BUNDLEMANAGER_QUICK_FIX_EXTRACT_DIFF_FILES_FAILED = 8520723,
    ERR_BUNDLEMANAGER_QUICK_FIX_APPLY_DIFF_PATCH_FAILED = 8520724,
    ERR_BUNDLEMANAGER_QUICK_FIX_NO_PATCH_IN_DATABASE = 8520725,
    ERR_BUNDLEMANAGER_QUICK_FIX_INVALID_PATCH_STATUS = 8520726,
    ERR_BUNDLEMANAGER_QUICK_FIX_NOT_EXISTED_BUNDLE_INFO = 8520727,
    ERR_BUNDLEMANAGER_QUICK_FIX_REMOVE_PATCH_PATH_FAILED = 8520728,
    ERR_BUNDLEMANAGER_QUICK_FIX_CREATE_PATCH_PATH_FAILED = 8520729,
    ERR_BUNDLEMANAGER_QUICK_FIX_MOVE_PATCH_FILE_FAILED = 8520730,
    ERR_BUNDLEMANAGER_QUICK_FIX_HOT_RELOAD_NOT_SUPPORT_RELEASE_BUNDLE = 8520731,
    ERR_BUNDLEMANAGER_QUICK_FIX_PATCH_ALREADY_EXISTED = 8520732,
    ERR_BUNDLEMANAGER_QUICK_FIX_HOT_RELOAD_ALREADY_EXISTED = 8520733,
    ERR_BUNDLEMANAGER_QUICK_FIX_NO_PATCH_INFO_IN_BUNDLE_INFO = 8520734,
    ERR_BUNDLEMANAGER_QUICK_FIX_OLD_PATCH_OR_HOT_RELOAD_IN_DB = 8520735,
    ERR_BUNDLEMANAGER_QUICK_FIX_SEND_REQUEST_FAILED = 8520736,
    ERR_BUNDLEMANAGER_QUICK_FIX_REAL_PATH_FAILED = 8520737,
    ERR_BUNDLEMANAGER_QUICK_FIX_INVALID_PATH = 8520738,
    ERR_BUNDLEMANAGER_QUICK_FIX_OPEN_SOURCE_FILE_FAILED = 8520739,
    ERR_BUNDLEMANAGER_QUICK_FIX_CREATE_FD_FAILED = 8520740,
    ERR_BUNDLEMANAGER_QUICK_FIX_INVALID_TARGET_DIR = 8520741,
    ERR_BUNDLEMANAGER_QUICK_FIX_CREATE_TARGET_DIR_FAILED = 8520742,
    ERR_BUNDLEMANAGER_QUICK_FIX_PERMISSION_DENIED = 8520743,
    ERR_BUNDLEMANAGER_QUICK_FIX_WRITE_FILE_FAILED = 8520744,

    ERR_BUNDLE_MANAGER_APP_CONTROL_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0501, // 8520961
    ERR_BUNDLE_MANAGER_APP_CONTROL_PERMISSION_DENIED = 8520962,
    ERR_BUNDLE_MANAGER_APP_CONTROL_RULE_TYPE_INVALID = 8520963,
    ERR_BUNDLE_MANAGER_BUNDLE_NOT_SET_CONTROL = 8520964,
    ERR_BUNDLE_MANAGER_APP_CONTROL_DISALLOWED_INSTALL = 8520965,
    ERR_BUNDLE_MANAGER_APP_CONTROL_DISALLOWED_UNINSTALL = 8520966,

    // query errcode
    ERR_BUNDLE_MANAGER_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0601, // 8521217
    ERR_BUNDLE_MANAGER_INVALID_PARAMETER = 8521218,
    ERR_BUNDLE_MANAGER_INVALID_USER_ID = 8521219,
    ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST = 8521220,
    ERR_BUNDLE_MANAGER_ABILITY_NOT_EXIST = 8521221,
    ERR_BUNDLE_MANAGER_MODULE_NOT_EXIST = 8521222,
    ERR_BUNDLE_MANAGER_ABILITY_DISABLED = 8521223,
    ERR_BUNDLE_MANAGER_APPLICATION_DISABLED = 8521224,
    ERR_BUNDLE_MANAGER_PARAM_ERROR = 8521225,
    ERR_BUNDLE_MANAGER_PERMISSION_DENIED = 8521226,
    ERR_BUNDLE_MANAGER_IPC_TRANSACTION = 8521227,
    ERR_BUNDLE_MANAGER_GLOBAL_RES_MGR_ENABLE_DISABLED = 8521228,
    ERR_BUNDLE_MANAGER_CAN_NOT_CLEAR_USER_DATA = 8521229,
    ERR_BUNDLE_MANAGER_QUERY_PERMISSION_DEFINE_FAILED = 8521230,
    ERR_BUNDLE_MANAGER_DEVICE_ID_NOT_EXIST = 8521231,
    ERR_BUNDLE_MANAGER_PROFILE_NOT_EXIST = 8521232,
    ERR_BUNDLE_MANAGER_INVALID_UID = 8521233,
    ERR_BUNDLE_MANAGER_INVALID_HAP_PATH = 8521234,
    ERR_BUNDLE_MANAGER_DEFAULT_APP_NOT_EXIST = 8521235,
    ERR_BUNDLE_MANAGER_INVALID_TYPE = 8521236,
    ERR_BUNDLE_MANAGER_ABILITY_AND_TYPE_MISMATCH = 8521237,
    ERR_BUNDLE_MANAGER_SYSTEM_API_DENIED = 8521238,
    ERR_BUNDLE_MANAGER_DUPLICATED_EXT_OR_TYPE = 8521239,

    // zlib errcode
    ERR_ZLIB_SRC_FILE_DISABLED = 8521240,
    ERR_ZLIB_DEST_FILE_DISABLED = 8521241,
    ERR_ZLIB_SERVICE_DISABLED = 8521242,
    ERR_ZLIB_SRC_FILE_FORMAT_ERROR = 8521243,
    // app jump interceptor
    ERR_BUNDLE_MANAGER_APP_JUMP_INTERCEPTOR_INTERNAL_ERROR = APPEXECFWK_BUNDLEMGR_ERR_OFFSET + 0x0701, // 8521473
    ERR_BUNDLE_MANAGER_APP_JUMP_INTERCEPTOR_PERMISSION_DENIED = 8521474,
    ERR_BUNDLE_MANAGER_APP_JUMP_INTERCEPTOR_RULE_TYPE_INVALID = 8521475,
    ERR_BUNDLE_MANAGER_BUNDLE_NOT_SET_JUMP_INTERCPTOR = 8521476,
    ERR_BUNDLE_MANAGER_INSTALL_FAILED_BUNDLE_EXTENSION_NOT_EXISTED = 8521477,

    // bms extension adapter
    ERR_BUNDLE_MANAGER_UNINSTALL_FROM_BMS_EXTENSION_FAILED = 8521478,
};

// Error code for Hidump
constexpr ErrCode APPEXECFWK_HIDUMP = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_HIDUMP);
enum {
    ERR_APPEXECFWK_HIDUMP_ERROR = APPEXECFWK_HIDUMP + 1,
    ERR_APPEXECFWK_HIDUMP_INVALID_ARGS,
    ERR_APPEXECFWK_HIDUMP_UNKONW,
    ERR_APPEXECFWK_HIDUMP_SERVICE_ERROR
};

} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_APPEXECFWK_ERROR_H
