add_cxxflags("-Wno-format")

target("ability_runtime")
    set_kind("static")
    add_files("services/**/*.cpp", "frameworks/native/**.cpp")
    add_defines("DEBUG_REFBASE", "INCLUDE_SELF_DEFINE")
    add_includedirs("services/abilitymgr/include", "services/common/include", "services/appmgr/include",
        "services/dataobsmgr/include")
    add_includedirs("d:/repo", "interfaces")
    add_includedirs("interfaces/inner_api/app_manager/include", "interfaces/inner_api/app_manager/include/appmgr", 
        "interfaces/inner_api/ability_manager/include", "interfaces/inner_api/connectionobs_manager/include",
        "interfaces/inner_api/error_utils/include", "interfaces/inner_api/napi_base_context/include",
        "interfaces/inner_api/runtime/include", "interfaces/inner_api/deps_wrapper/include",
        "interfaces/inner_api/uri_permission/include", "interfaces/inner_api/wantagent/include",
        "interfaces/inner_api/dataobs_manager/include",
        "interfaces/kits/native/ability/ability_runtime", "interfaces/kits/native/ability/native",
        "interfaces/kits/native/ability/native/distributed_ability_runtime",
        "interfaces/kits/native/appkit/ability_runtime", "interfaces/kits/native/appkit/ability_runtime/context",
        "interfaces/kits/native/appkit/app", "interfaces/kits/native/ability/native/continuation/kits",
        "interfaces/kits/native/ability/native/continuation/distributed", 
        "interfaces/kits/native/ability/native/continuation/remote_register_service",
        "interfaces/kits/native/ability/native/ability_business_error", "frameworks/js/napi/inner/napi_common",
        "interfaces/kits/native/ability/native/recovery", "interfaces/kits/native/appkit/ability_delegator",
        "tools/aa/include", "tools/fm/include", "frameworks/js/napi/inner/napi_ability_common")
    add_includedirs("../communication_ipc/interfaces/innerkits/ipc_core/include", "../communication_ipc/interfaces/innerkits/ipc_napi_common/include")
    add_includedirs("../commonlibrary_c_utils/base/include")
    add_includedirs("D:/cache/include")
    add_includedirs("../bundlemanager_bundle_framework/interfaces/inner_api/appexecfwk_base/include", 
        "../bundlemanager_bundle_framework/interfaces/inner_api/appexecfwk_core/include",
        "../bundlemanager_bundle_framework/interfaces/inner_api/appexecfwk_core/include/bundlemgr")
    add_includedirs("../ability_ability_base/interfaces/kits/native/want/include", "../ability_ability_base/interfaces/inner_api/base/include", 
        "../ability_ability_base/interfaces/kits/native/uri/include", "../ability_ability_base/interfaces/kits/native/configuration/include",
        "../ability_ability_base/interfaces/kits/native/extractortool/include", "../ability_ability_base/interfaces/kits/native/session_info/include")
    add_includedirs("../third_party_json/single_include", "../third_party_json/include", "../third_party_jsoncpp/include")
    add_includedirs("../hiviewdfx_hitrace/interfaces/native/innerkits/include/hitrace_meter", "../hiviewdfx_hilog_lite/interfaces/native/innerkits")
    add_includedirs("../notification_eventhandler/interfaces/inner_api", "../notification_common_event_service/interfaces/inner_api",
        "../notification_common_event_service/interfaces/kits/napi/common_event/include", "../notification_common_event_service/frameworks/core/include")
    add_includedirs("../startup_appspawn/interfaces/innerkits/include")
    add_includedirs("../systemabilitymgr_safwk/services/safwk/include", "../systemabilitymgr_samgr/interfaces/innerkits/samgr_proxy/include")
    add_includedirs("../distributedhardware_device_manager/interfaces/inner_kits/native_cpp/include")
    add_includedirs("../arkui_napi/interfaces/kits", "../arkui_napi/interfaces/inner_api")
    add_includedirs("../other_include", "../arkui_napi")
    add_includedirs("../arkui_ace_engine/frameworks", "../arkui_ace_engine/frameworks/base/image", "../arkui_ace_engine/interfaces/inner_api/ace")
    add_includedirs("../global_resource_management/interfaces/inner_api/include")
    add_includedirs("../security_access_token/interfaces/innerkits/accesstoken/include")
    add_includedirs("../hiviewdfx_hisysevent/interfaces/native/innerkits/hisysevent/include")
    add_includedirs("../hiviewdfx_hicollie/interfaces/native/innerkits/include")
    add_includedirs("../communication_dsoftbus/interfaces/kits/bus_center", "../communication_dsoftbus/interfaces/kits/common")
    add_includedirs("../startup_syspara_lite/interfaces/kits")
    add_includedirs("../window_window_manager/interfaces/innerkits/wm", "../window_window_manager/utils/include",
        "../window_window_manager/window_scene", "../window_window_manager/previewer/mock")
    add_includedirs("../distributeddatamgr_relational_store/interfaces/inner_api/rdb/include", "../distributeddatamgr_relational_store/interfaces/inner_api/appdatafwk/include",
        "../distributeddatamgr_relational_store/interfaces/inner_api/dataability/include")
    add_includedirs("../ability_form_fwk/interfaces/inner_api/include", "../ability_form_fwk/interfaces/kits/native/include")
    add_includedirs("../arkcompiler_ets_runtime", "../arkcompiler_runtime_core/libpandabase")
    add_includedirs("../hiviewdfx_faultloggerd/interfaces/innerkits/dump_catcher/include", "../hiviewdfx_faultloggerd/common",
        "../hiviewdfx_faultloggerd/tools/process_dump", "../hiviewdfx_faultloggerd/interfaces/innerkits/faultloggerd_client/include")
    add_includedirs("../hiviewdfx_hichecker/interfaces/native/innerkits/include")
    add_includedirs("../startup_init_lite/interfaces/innerkits/include")
    add_includedirs("../third_include_dir")


set_languages("c11", "c++17")